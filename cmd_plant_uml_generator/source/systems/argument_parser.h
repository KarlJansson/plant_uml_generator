#pragma once

#include <entity_manager.h>
#include <system_manager.h>

using namespace ecss;

#include <typeindex>
#include <vector>

class ArgumentParser {
 public:
  void Init();
  void Step(EntityManager_t& ent_mgr, SystemManager_t& sys_mgr);
  std::vector<std::type_index> Dependencies();

 private:
  void ReadConfigFile(
      EntityManager_t& ent_mgr,
      std::vector<std::pair<std::string, std::string>>& patterns);
};
