#pragma once

#include <cstdint>

// Horizontal layout of a chapter-length bar chart. Kept dependency-free for host tests.
struct ChapterBarLayout {
  static constexpr int MAX_BAR_WIDTH = 10;
  static constexpr int BAR_GAP = 1;

  int barCount = 0;
  int barWidth = 0;
  int gap = 0;
  int totalWidth = 0;

  // Bars are at most MAX_BAR_WIDTH wide with a 1px gap, narrowed to fit `width`.
  // Gaps disappear before bars go below 1px; past one chapter per pixel,
  // neighbouring chapters share a bar.
  static constexpr ChapterBarLayout fit(const int chapterCount, const int width) {
    ChapterBarLayout layout;
    if (chapterCount <= 0 || width <= 0) return layout;
    if (chapterCount > width) {
      layout.barCount = width;
      layout.barWidth = 1;
    } else {
      layout.barCount = chapterCount;
      // n bars and n-1 gaps fit when n * (bar + gap) <= width + gap.
      const int pitch = (width + BAR_GAP) / chapterCount;
      if (pitch > BAR_GAP) {
        layout.gap = BAR_GAP;
        layout.barWidth = pitch - BAR_GAP < MAX_BAR_WIDTH ? pitch - BAR_GAP : MAX_BAR_WIDTH;
      } else {
        layout.barWidth = 1;
      }
    }
    layout.totalWidth = layout.barCount * layout.barWidth + (layout.barCount - 1) * layout.gap;
    return layout;
  }

  // First chapter drawn in `bar`; bar b covers [firstChapter(b), firstChapter(b + 1)).
  static constexpr int firstChapter(const int bar, const int barCount, const int chapterCount) {
    return static_cast<int>(static_cast<int64_t>(bar) * chapterCount / barCount);
  }
};
