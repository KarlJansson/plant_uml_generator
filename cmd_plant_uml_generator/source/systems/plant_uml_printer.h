#pragma once

#include <entity_manager.h>
#include <system_manager.h>

#include <filesystem>
#include <typeindex>
#include <vector>

#include "class_declaration.h"
#include "dependent.h"

class PlantUmlPrinter {
 public:
  void Init() {}
  std::vector<std::type_index> Dependencies() { return {}; }

  template <typename Ent, typename EntMgr, typename SysMgr>
  void Step(EntMgr& ent_mgr, SysMgr& sys_mgr) {
    std::string output =
        "@startuml\n"
        "hide members\n"
        "left to right direction\n";

    std::unordered_map<std::string, std::unordered_set<std::string>> duplicates;
    for (auto [cls, cls_ent] : emgr_components_r(ent_mgr, ClassDeclaration)) {
      for (auto dep : ent_components_w((*cls_ent), Dependent<EntMgr>)) {
        std::string dep_str = "<unknown>";
        if (auto dep_cldecl = ent_component_r(dep->dependent, ClassDeclaration);
            dep_cldecl) {
          dep_str = dep_cldecl->class_name;
          if (duplicates[dep_str].find(cls->class_name) ==
                  std::end(duplicates[dep_str]) &&
              dep_str != cls->class_name) {
            output += dep_str + " ..> " + cls->class_name + "\n";
            duplicates[dep_str].emplace(cls->class_name);
          }
        }
      }
    }

    output += "@enduml";
    std::cout << output << std::endl;

    auto exit_code = emgr_add_component(ent_mgr, int);
    *exit_code = 0;

    smgr_remove_system(sys_mgr, PlantUmlPrinter);
  }
};
