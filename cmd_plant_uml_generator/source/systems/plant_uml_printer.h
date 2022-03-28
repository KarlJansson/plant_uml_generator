#pragma once

#include <entity_manager.h>
#include <system_manager.h>

using namespace ecss;

#include <typeindex>
#include <unordered_set>
#include <vector>

#include "class_declaration.h"

class PlantUmlPrinter {
 public:
  void Init();
  std::vector<std::type_index> Dependencies();
  void Step(EntityManager_t& ent_mgr, SystemManager_t& sys_mgr);

 private:
  template <typename Dep>
  void Crawl(EntityManager_t& ent_mgr, SystemManager_t& sys_mgr,
             const Entity& focus, const ClassDeclaration* cls,
             std::unordered_set<std::string>& visited, size_t depth,
             std::unordered_set<std::string>& added, std::string& out,
             std::function<std::string(const std::string&, const std::string&)>
                 conn_entry,
             const Entity& origin);

  void CheckAndAdd(std::unordered_set<std::string>& set, std::string str,
                   std::string& out);
  bool CheckIgnore(const std::string& check);
  bool CheckStop(const std::string& check);

  size_t max_depth_;
  size_t max_dependents_;
  std::vector<std::string> ignore_patterns_;
  std::vector<std::string> stop_patterns_;
};
