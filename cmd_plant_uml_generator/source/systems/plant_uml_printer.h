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
#include "ignore_patterns.h"
#include "settings.h"

class PlantUmlPrinter {
 public:
  system_step() {
    std::string header =
        "@startuml\n"
        "hide members\n"
        "left to right direction\n\n";

    std::set<std::string> full_types;
    std::set<std::string> full_connections;
    std::set<std::string> individual_types;
    std::set<std::string> individual_connections;
    std::map<std::string, std::string> individual;

    auto ignore_patterns = emgr_component_r(IgnorePatterns);
    auto create_entries = [&](auto& class_type, auto& dep_type,
                              auto& class_name, auto& dep_name) {
      if (!ignore_patterns->patterns.empty()) {
        auto concat = class_name + dep_name;
        for (auto& p : ignore_patterns->patterns)
          if (concat.find(p) != std::string::npos) return;
      }
      individual_types.insert(dep_type + " " + dep_name + "\n");
      individual_types.insert(class_type + " " + class_name + "\n");
      individual_connections.insert(class_name + " ..> " + dep_name + "\n");
      full_types.insert(class_type + " " + class_name + "\n");
      full_types.insert(dep_type + " " + dep_name + "\n");
      full_connections.insert(dep_name + " ..> " + class_name + "\n");
    };

    auto settings = emgr_component_r(Settings);
    for (const auto& [cls, cls_ent] : emgr_components_r(ClassDeclaration)) {
      std::string single = header;
      single += "title <using> " + cls.class_name + " <used by>\n\n";

      individual_types.clear();
      individual_connections.clear();

      std::list<std::pair<ecs::Entity<EntMgr>, ClassDeclaration>>
          open_list_dependent{{cls_ent, cls}};
      std::list<std::pair<ecs::Entity<EntMgr>, ClassDeclaration>>
          open_list_dependee{{cls_ent, cls}};
      std::unordered_set<std::string> visited{cls.class_name};
      for (size_t i = 0; i < settings->expansion_level; ++i) {
        for (size_t take = open_list_dependent.size(); take != 0; --take) {
          auto [focus_ent, focus] = open_list_dependent.front();
          open_list_dependent.pop_front();
          for (auto& dep_ent : ent_components_w(focus_ent, Dependent<EntMgr>)) {
            for (auto& d :
                 ent_components_r(dep_ent.dependent, ClassDeclaration)) {
              if (d.class_name != focus.class_name &&
                  visited.find(d.class_name) == std::end(visited)) {
                visited.emplace(d.class_name);
                open_list_dependent.emplace_back(dep_ent.dependent, d);
                create_entries(focus.type, d.type, focus.class_name,
                               d.class_name);
              }
            }
          }
        }

        for (size_t take = open_list_dependee.size(); take != 0; --take) {
          auto [focus_ent, focus] = open_list_dependee.front();
          open_list_dependee.pop_front();
          for (auto& dep_ent : ent_components_w(focus_ent, Dependee<EntMgr>)) {
            for (auto& d :
                 ent_components_r(dep_ent.dependee, ClassDeclaration)) {
              if (d.class_name != focus.class_name &&
                  visited.find(d.class_name) == std::end(visited)) {
                visited.emplace(d.class_name);
                open_list_dependee.emplace_back(dep_ent.dependee, d);
                create_entries(d.type, focus.type, d.class_name,
                               focus.class_name);
              }
            }
          }
        }
      }

      for (auto& str : individual_types) single += str;
      single += "\n";
      for (auto& str : individual_connections) single += str;
      single += "@enduml";
      individual[cls.class_name] = single;
    }

    std::string output = header;
    for (auto& str : full_types) output += str;
    output += "\n";
    for (auto& str : full_connections) output += str;
    output += "@enduml";

    if (settings->flags.find(Settings::Flag::kPrintFull) !=
        std::end(settings->flags))
      std::cout << output << std::endl;
    if (settings->flags.find(Settings::Flag::kPrintIndividual) !=
        std::end(settings->flags))
      for (auto& [cls_name, out] : individual) std::cout << out << std::endl;

    auto& exit_code = emgr_add_component(int);
    exit_code = 0;

    smgr_remove_system(PlantUmlPrinter);
  }

  void Init() {}
  std::vector<std::type_index> Dependencies() { return {}; }
};
