#pragma once

#include <entity_manager.h>
#include <system_manager.h>

using namespace ecss;

#include <typeindex>
#include <vector>

class ModelImporter {
 public:
  void Init();
  std::vector<std::type_index> Dependencies();
  void Step(EntityManager_t& ent_mgr, SystemManager_t& sys_mgr);
};
