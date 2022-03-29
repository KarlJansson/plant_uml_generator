#pragma once

#include <entity_manager.h>
#include <system_manager.h>

using namespace ecss;

#include <typeindex>
#include <vector>

class ArgumentParser {
 public:
  void Init();
  std::vector<std::type_index> Dependencies();
  void Step(EntityManager_t& ent_mgr, SystemManager_t& sys_mgr);

 private:
  void ReadConfigFile(
      EntityManager_t& ent_mgr,
      std::vector<std::pair<std::string, std::string>>& patterns,
      std::vector<std::string>& header);
};
