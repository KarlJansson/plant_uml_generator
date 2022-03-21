#include <string>
#include <vector>

#include "main_entry.h"

int main(int argc, char** argv) {
  std::vector<std::string> args;
  for (int i = 1; i < argc; ++i) args.emplace_back(argv[i]);
  return MainEntry::Main<EntityManager_t, SystemManager_t>(args);
}
