#pragma once

#include <entity_manager.h>
using namespace ecss;

#include <system_manager.h>

#include <set>
#include <string>
#include <typeindex>
#include <vector>

class FileAnalyzerTypeExtraction {
 public:
  void Init();
  void Step(EntityManager_t& ent_mgr, SystemManager_t& sys_mgr);
  std::vector<std::type_index> Dependencies();

 private:
  bool IsValid(const std::string& str, std::set<std::string>& found);
};
