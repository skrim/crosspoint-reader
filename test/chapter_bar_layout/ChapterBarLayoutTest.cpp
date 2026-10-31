#include <gtest/gtest.h>

#include "src/activities/reader/ChapterBarLayout.h"

TEST(ChapterBarLayout, FewChaptersUseMaxWidth) {
  const auto layout = ChapterBarLayout::fit(5, 450);
  EXPECT_EQ(layout.barCount, 5);
  EXPECT_EQ(layout.barWidth, 10);
  EXPECT_EQ(layout.gap, 1);
  EXPECT_EQ(layout.totalWidth, 54);
}

TEST(ChapterBarLayout, ExactFitAtMaxWidth) {
  // 41 bars * 10 + 40 gaps = 450.
  const auto layout = ChapterBarLayout::fit(41, 450);
  EXPECT_EQ(layout.barWidth, 10);
  EXPECT_EQ(layout.totalWidth, 450);
}

TEST(ChapterBarLayout, NarrowsToFit) {
  const auto layout = ChapterBarLayout::fit(100, 450);
  EXPECT_EQ(layout.barWidth, 3);
  EXPECT_EQ(layout.gap, 1);
  EXPECT_EQ(layout.totalWidth, 399);
  EXPECT_LE(layout.totalWidth, 450);
}

TEST(ChapterBarLayout, DropsGapsBeforeOnePixelBars) {
  const auto layout = ChapterBarLayout::fit(300, 450);
  EXPECT_EQ(layout.barCount, 300);
  EXPECT_EQ(layout.barWidth, 1);
  EXPECT_EQ(layout.gap, 0);
  EXPECT_EQ(layout.totalWidth, 300);
}

TEST(ChapterBarLayout, TwoPixelPitchKeepsGap) {
  // 225 bars + 224 gaps = 449.
  const auto layout = ChapterBarLayout::fit(225, 450);
  EXPECT_EQ(layout.barWidth, 1);
  EXPECT_EQ(layout.gap, 1);
  EXPECT_EQ(layout.totalWidth, 449);
}

TEST(ChapterBarLayout, MergesChaptersPastOnePerPixel) {
  const auto layout = ChapterBarLayout::fit(1000, 450);
  EXPECT_EQ(layout.barCount, 450);
  EXPECT_EQ(layout.barWidth, 1);
  EXPECT_EQ(layout.totalWidth, 450);
  // Every chapter lands in exactly one bar, and every bar has a chapter.
  EXPECT_EQ(ChapterBarLayout::firstChapter(0, 450, 1000), 0);
  EXPECT_EQ(ChapterBarLayout::firstChapter(450, 450, 1000), 1000);
  for (int bar = 0; bar < 450; bar++) {
    EXPECT_LT(ChapterBarLayout::firstChapter(bar, 450, 1000), ChapterBarLayout::firstChapter(bar + 1, 450, 1000));
  }
}

TEST(ChapterBarLayout, EmptyInputs) {
  EXPECT_EQ(ChapterBarLayout::fit(0, 450).barCount, 0);
  EXPECT_EQ(ChapterBarLayout::fit(10, 0).barCount, 0);
}
