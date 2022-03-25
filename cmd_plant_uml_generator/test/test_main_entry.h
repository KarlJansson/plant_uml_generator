#include "main_entry.h"

TEST(MainEntry, main_test) {
  std::vector<std::string> args{"../", "-pi", "-el", "2"};
  auto result = MainEntry::Main(args);
  EXPECT_EQ(result, 0);
}

TEST(MainEntry, main_test_faulty_call) {
  std::vector<std::string> args{"../"};
  auto result = MainEntry::Main(args);
  EXPECT_EQ(result, 0);
}

TEST(MainEntry, main_test_full_call) {
  std::vector<std::string> args{"../", "-pf"};
  auto result = MainEntry::Main(args);
  EXPECT_EQ(result, 0);
}

TEST(MainEntry, main_test_full_export) {
  std::vector<std::string> args{"../", "-pf", "-export", "test"};
  auto result = MainEntry::Main(args);
  EXPECT_EQ(result, 0);
}
