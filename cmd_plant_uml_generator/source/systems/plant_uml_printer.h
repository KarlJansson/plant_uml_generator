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
  template <typename Ent, typename EntMgr, typename SysMgr>
  void Step(EntMgr& ent_mgr, SysMgr& sys_mgr) {
    std::string header =
        "@startuml\n"
        "hide members\n"
        "left to right direction\n\n";

    std::set<std::string> full_types;
    std::set<std::string> full_connections;
    std::set<std::string> individual_types;
    std::set<std::string> individual_connections;
    std::map<std::string, std::string> individual;

    auto create_entries = [&](auto& class_type, auto& dep_type,
                              auto& class_name, auto& dep_name) {
      if (class_name != dep_name) {
        individual_types.insert(dep_type + " " + dep_name + "\n");
        individual_types.insert(class_type + " " + class_name + "\n");
        individual_connections.insert(class_name + " ..> " + dep_name + "\n");
        full_types.insert(class_type + " " + class_name + "\n");
        full_types.insert(dep_type + " " + dep_name + "\n");
        full_connections.insert(dep_name + " ..> " + class_name + "\n");
      }
    };

    for (auto [cls, cls_ent] : emgr_components_r(ent_mgr, ClassDeclaration)) {
      std::string single = header;
      single += "title <using> " + cls->class_name + " <used by>\n\n";

      individual_types.clear();
      individual_connections.clear();

      for (auto dep_ent : ent_components_w((*cls_ent), Dependent<EntMgr>))
        if (auto d = ent_component_r(dep_ent->dependent, ClassDeclaration); d)
          create_entries(cls->type, d->type, cls->class_name, d->class_name);

      for (auto dep_ent : ent_components_w((*cls_ent), Dependee<EntMgr>))
        if (auto d = ent_component_r(dep_ent->dependee, ClassDeclaration); d)
          create_entries(d->type, cls->type, d->class_name, cls->class_name);

      for (auto& str : individual_types) single += str;
      single += "\n";
      for (auto& str : individual_connections) single += str;
      single += "@enduml";
      individual[cls->class_name] = single;
    }

    std::string output = header;
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

  void Init() {}
  std::vector<std::type_index> Dependencies() { return {}; }
};
