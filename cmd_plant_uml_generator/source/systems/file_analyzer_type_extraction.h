#pragma once

#include <entity_manager.h>
#include <system_manager.h>

using namespace ecss;

#include <set>
#include <string>
#include <typeindex>
#include <vector>

class FileAnalyzerTypeExtraction {
 public:
  void Init();
  std::vector<std::type_index> Dependencies();
  void Step(EntityManager_t& ent_mgr, SystemManager_t& sys_mgr);

 private:
  bool IsValid(const std::string& str, std::set<std::string>& found);
};
