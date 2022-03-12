#include "main_entry.h"

TEST(MainEntry, main_test) {
  std::vector<std::string> args{"/home/enrkaja/repos/ecs_game_engine"};
  auto result = MainEntry::Main(args);
  EXPECT_EQ(result, 0);
}
