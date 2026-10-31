add_executable(ChapterBarLayoutTest
  ChapterBarLayoutTest.cpp
)

target_link_libraries(ChapterBarLayoutTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(ChapterBarLayoutTest)
