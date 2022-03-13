#pragma once

#include <entity_manager.h>
#include <system_manager.h>

#include <filesystem>
#include <map>
#include <set>
#include <typeindex>
#include <vector>

#include "class_declaration.h"
#include "dependee.h"
#include "dependent.h"

class PlantUmlPrinter {
 public:
  void Init() {}
  std::vector<std::type_index> Dependencies() { return {}; }

  template <typename Ent, typename EntMgr, typename SysMgr>
  void Step(EntMgr& ent_mgr, SysMgr& sys_mgr) {
    std::set<std::string> full_types;
    std::set<std::string> full_connections;

    std::string header =
        "@startuml\n"
        "hide members\n"
        "left to right direction\n\n";
    std::string output = header;

    std::set<std::string> individual_types;
    std::set<std::string> individual_connections;
    std::map<std::string, std::string> individual;
    for (auto [cls, cls_ent] : emgr_components_r(ent_mgr, ClassDeclaration)) {
      std::string single = header;
      single += "title <using> " + cls->class_name + " <used by>\n\n";

      individual_types.clear();
      individual_connections.clear();

      full_types.insert(cls->type + " " + cls->class_name + "\n");
      individual_types.insert(cls->type + " " + cls->class_name + "\n");

      for (auto dep : ent_components_w((*cls_ent), Dependent<EntMgr>)) {
        std::string dep_str = "<unknown>";
        if (auto dep_cldecl = ent_component_r(dep->dependent, ClassDeclaration);
            dep_cldecl) {
          dep_str = dep_cldecl->class_name;

          full_types.insert(dep_cldecl->type + " " + dep_str + "\n");
          full_connections.insert(dep_str + " ..> " + cls->class_name + "\n");

          individual_types.insert(dep_cldecl->type + " " + dep_str + "\n");
          individual_connections.insert(cls->class_name + " ..> " + dep_str +
                                        "\n");
        }
      }

      for (auto dep : ent_components_w((*cls_ent), Dependee<EntMgr>)) {
        std::string dep_str = "<unknown>";
        if (auto dep_cldecl = ent_component_r(dep->dependee, ClassDeclaration);
            dep_cldecl) {
          dep_str = dep_cldecl->class_name;
          if (cls->class_name != dep_str) {
            individual_types.insert(dep_cldecl->type + " " + dep_str + "\n");
            individual_connections.insert(dep_str + " ..> " + cls->class_name +
                                          "\n");
            full_types.insert(dep_cldecl->type + " " + dep_str + "\n");
            full_connections.insert(cls->class_name + " ..> " + dep_str + "\n");
          }
        }
      }

      for (auto& str : individual_types) single += str;
      single += "\n";
      for (auto& str : individual_connections) single += str;
      single += "@enduml";
      individual[cls->class_name] = single;
    }

    for (auto& str : full_types) output += str;
    output += "\n";
    for (auto& str : full_connections) output += str;
    output += "@enduml";

    std::cout << output << std::endl;
    for (auto& [cls_name, out] : individual) std::cout << out << std::endl;

    auto exit_code = emgr_add_component(ent_mgr, int);
    *exit_code = 0;

    smgr_remove_system(sys_mgr, PlantUmlPrinter);
  }
};
