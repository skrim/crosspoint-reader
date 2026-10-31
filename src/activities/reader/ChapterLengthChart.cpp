#include "ChapterLengthChart.h"

#include <Epub.h>
#include <Logging.h>
#include <Memory.h>

#include <algorithm>

#include "ChapterBarLayout.h"

namespace fui = freeink::ui;

namespace {

constexpr int NOTCH_HEIGHT = 3;

// Groups spine items into chapters, by TOC entry or one per spine file.
struct ChapterSplitter {
  bool bySpine = false;
  int count = 0;        // chapters started so far
  uint32_t length = 0;  // bytes in the current chapter so far
  uint32_t longest = 0;
  int previousToc = -1;

  // False for front matter, which belongs to no chapter.
  bool add(const uint32_t size, const int16_t tocIndex) {
    if (!bySpine && tocIndex < 0) return false;
    if (bySpine || tocIndex != previousToc) {
      count++;
      length = 0;
    }
    previousToc = tocIndex;
    length += size;
    longest = std::max(longest, length);
    return true;
  }
};

// Turns cumulative sizes into per-item sizes; a bad entry counts as empty.
struct SpineSizes {
  uint32_t previous = 0;
  uint32_t next(const uint32_t cumulative) {
    const uint32_t size = cumulative > previous ? cumulative - previous : 0;
    previous = std::max(previous, cumulative);
    return size;
  }
};

}  // namespace

void ChapterLengthChart::load(const Epub& epub, const int currentSpineIndex) {
  heights.reset();
  chapterCount = 0;
  currentChapter = -1;

  // Pass 1: chapter count and longest chapter for both groupings.
  ChapterSplitter byToc;
  ChapterSplitter bySpine;
  bySpine.bySpine = true;
  SpineSizes sizes;
  auto measure = [&](const uint32_t cumulative, const int16_t tocIndex) {
    const uint32_t size = sizes.next(cumulative);
    byToc.add(size, tocIndex);
    bySpine.add(size, tocIndex);
  };
  if (!epub.forEachSpineSize(measure)) {
    LOG_ERR("CLC", "Cannot read spine sizes");
    return;
  }
  const bool useSpine = byToc.count < 2 || byToc.longest == 0;
  const ChapterSplitter& stats = useSpine ? bySpine : byToc;
  if (stats.count == 0 || stats.longest == 0) return;

  auto buffer = makeUniqueNoThrow<uint8_t[]>(static_cast<size_t>(stats.count));
  if (!buffer) {
    LOG_ERR("CLC", "OOM: chapter chart %d bytes", stats.count);
    return;
  }

  // Pass 2: scaled heights. Each item rewrites its chapter's height as the chapter grows.
  ChapterSplitter splitter;
  splitter.bySpine = useSpine;
  sizes = SpineSizes{};
  int spineIndex = 0;
  int current = -1;
  auto fill = [&](const uint32_t cumulative, const int16_t tocIndex) {
    const uint32_t size = sizes.next(cumulative);
    if (splitter.add(size, tocIndex) && splitter.count <= stats.count) {
      const int chapter = splitter.count - 1;
      // Non-empty chapters keep at least one step so they stay visible.
      const auto scaled = static_cast<uint8_t>(static_cast<uint64_t>(splitter.length) * 255 / stats.longest);
      buffer[chapter] = splitter.length > 0 && scaled == 0 ? 1 : scaled;
      if (spineIndex == currentSpineIndex) current = chapter;
    }
    spineIndex++;
  };
  if (!epub.forEachSpineSize(fill) || splitter.count != stats.count) {
    LOG_ERR("CLC", "Spine sizes changed while reading");
    return;
  }

  heights = std::move(buffer);
  chapterCount = stats.count;
  currentChapter = current;
}

void ChapterLengthChart::draw(fui::DrawTarget& target, const fui::Rect rect) const {
  if (chapterCount == 0 || rect.empty()) return;
  const auto layout = ChapterBarLayout::fit(chapterCount, rect.width);
  const int left = rect.x + (rect.width - layout.totalWidth) / 2;

  for (int bar = 0; bar < layout.barCount; bar++) {
    const int first = ChapterBarLayout::firstChapter(bar, layout.barCount, chapterCount);
    const int end = ChapterBarLayout::firstChapter(bar + 1, layout.barCount, chapterCount);
    uint8_t height = 0;
    bool current = false;
    for (int chapter = first; chapter < end; chapter++) {
      height = std::max(height, heights[chapter]);
      current = current || chapter == currentChapter;
    }
    const int x = left + bar * (layout.barWidth + layout.gap);
    if (bar > 0 && layout.gap > 0) {
      const int notch = std::min<int>(NOTCH_HEIGHT, rect.height);
      target.fill(fui::Rect{static_cast<int16_t>(x - layout.gap), static_cast<int16_t>(rect.bottom() - notch),
                            static_cast<int16_t>(layout.gap), static_cast<int16_t>(notch)},
                  fui::Paint::solid(fui::Color::Black));
    }
    int pixels = height * rect.height / 255;
    if (height > 0 && pixels == 0) pixels = 1;
    if (pixels == 0) continue;

    const fui::Rect barRect{static_cast<int16_t>(x), static_cast<int16_t>(rect.bottom() - pixels),
                            static_cast<int16_t>(layout.barWidth), static_cast<int16_t>(pixels)};
    target.fill(barRect, current ? fui::Paint::solid(fui::Color::Black) : fui::Paint::dither(fui::Color::DarkGray));
  }
}
