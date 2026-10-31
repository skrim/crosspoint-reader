#pragma once

#include <FreeInkUICore.h>

#include <cstdint>
#include <memory>

class Epub;

// Bar chart of chapter lengths for the reader menu. A chapter is a spine file
// that starts a TOC entry plus the files after it, so TOC entries sharing a file
// share a bar and front matter before the first entry is left out. Books with
// fewer than two such chapters get one bar per spine file. Length is
// uncompressed XHTML bytes.
class ChapterLengthChart {
 public:
  // Two sequential passes over book.bin; leaves the chart empty on failure.
  void load(const Epub& epub, int currentSpineIndex);
  bool empty() const { return chapterCount == 0; }
  int count() const { return chapterCount; }
  // Zero-based chart chapter holding the reading position, or -1 (front matter, no chart).
  int current() const { return currentChapter; }
  // Bars are centered and bottom-aligned; the longest chapter fills rect.height.
  // Notches rise from the bottom in the gaps so runs of tiny chapters stay countable.
  void draw(freeink::ui::DrawTarget& target, freeink::ui::Rect rect) const;

 private:
  std::unique_ptr<uint8_t[]> heights;  // per chapter, 255 = longest
  int chapterCount = 0;
  int currentChapter = -1;
};
