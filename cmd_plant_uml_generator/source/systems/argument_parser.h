#pragma once

#include <entity_manager.h>
using namespace ecss;

#include <system_manager.h>

#include <typeindex>
#include <vector>

class ArgumentParser {
 public:
  void Init();
  void Step(EntityManager_t& ent_mgr, SystemManager_t& sys_mgr);
  std::vector<std::type_index> Dependencies();
};
