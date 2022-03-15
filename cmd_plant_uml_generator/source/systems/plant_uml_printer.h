#pragma once

#include <entity_manager.h>
#include <system_manager.h>

#include <filesystem>
#include <list>
#include <map>
#include <mutex>
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

    bool print_full{false};
    bool print_individual{false};
    bool print_individual_file{false};
    auto settings = emgr_component_r(Settings);
    if (settings->flags.find(Settings::Flag::kPrintFull) !=
        std::end(settings->flags))
      print_full = true;
    if (settings->flags.find(Settings::Flag::kPrintIndividual) !=
        std::end(settings->flags))
      print_individual = true;
    if (settings->flags.find(Settings::Flag::kPrintIndividualFile) !=
        std::end(settings->flags))
      print_individual_file = true;

    auto create_entries = [](auto& class_type, auto& dep_type, auto& class_name,
                             auto& dep_name, auto& types, auto& connections) {
      types.insert(dep_type + " " + dep_name + "\n");
      types.insert(class_type + " " + class_name + "\n");
      connections.insert(class_name + " ..> " + dep_name + "\n");
    };

    auto ignore_patterns = emgr_component_r(IgnorePatterns);
    auto check_ignore = [&](auto check) -> bool {
      if (!ignore_patterns->patterns.empty())
        for (auto& p : ignore_patterns->patterns)
          if (check.find(p) != std::string::npos) return false;
      return true;
    };

    if (print_full) {
      std::set<std::string> full_types;
      std::set<std::string> full_connections;
      for (const auto& [cls, cls_ent] : emgr_components_r(ClassDeclaration)) {
        for (auto& dep_ent : ent_components_w(cls_ent, Dependent<EntMgr>)) {
          for (auto& d :
               ent_components_r(dep_ent.dependent, ClassDeclaration)) {
            if (d.class_name != cls.class_name)
              if (check_ignore(cls.class_name + d.class_name))
                create_entries(cls.type, d.type, cls.class_name, d.class_name,
                               full_types, full_connections);
          }
        }
        for (auto& dep_ent : ent_components_w(cls_ent, Dependee<EntMgr>)) {
          for (auto& d : ent_components_r(dep_ent.dependee, ClassDeclaration)) {
            if (d.class_name != cls.class_name)
              if (check_ignore(cls.class_name + d.class_name))
                create_entries(d.type, cls.type, d.class_name, cls.class_name,
                               full_types, full_connections);
          }
        }
      }
      std::cout << header;
      for (auto& str : full_types) std::cout << str;
      std::cout << "\n";
      for (auto& str : full_connections) std::cout << str;
      std::cout << "@enduml\n";
    }

    if (print_individual || print_individual_file) {
      std::mutex print_mtx;
      auto type_declarations = emgr_components_r(ClassDeclaration);
      auto individual_processing = [&](auto i) {
        const auto& [cls, cls_ent] = type_declarations[i];
        std::string single = header;
        single += "title <using> " + cls.class_name + " <used by>\n\n";

        std::set<std::string> individual_types;
        std::set<std::string> individual_connections;

        std::list<std::pair<ecs::Entity<EntMgr>, ClassDeclaration>>
            open_list_dependent{{cls_ent, cls}};
        std::list<std::pair<ecs::Entity<EntMgr>, ClassDeclaration>>
            open_list_dependee{{cls_ent, cls}};
        std::unordered_set<std::string> visited{cls.class_name};
        for (size_t i = 0; i < settings->expansion_level; ++i) {
          for (size_t take = open_list_dependent.size(); take != 0; --take) {
            auto [focus_ent, focus] = open_list_dependent.front();
            open_list_dependent.pop_front();
            for (auto& dep_ent :
                 ent_components_w(focus_ent, Dependent<EntMgr>)) {
              for (auto& d :
                   ent_components_r(dep_ent.dependent, ClassDeclaration)) {
                if (d.class_name != focus.class_name &&
                    visited.find(d.class_name) == std::end(visited)) {
                  visited.emplace(d.class_name);
                  if (check_ignore(focus.class_name + d.class_name)) {
                    open_list_dependent.emplace_back(dep_ent.dependent, d);
                    create_entries(focus.type, d.type, focus.class_name,
                                   d.class_name, individual_types,
                                   individual_connections);
                  }
                }
              }
            }
          }

          for (size_t take = open_list_dependee.size(); take != 0; --take) {
            auto [focus_ent, focus] = open_list_dependee.front();
            open_list_dependee.pop_front();
            for (auto& dep_ent :
                 ent_components_w(focus_ent, Dependee<EntMgr>)) {
              for (auto& d :
                   ent_components_r(dep_ent.dependee, ClassDeclaration)) {
                if (d.class_name != focus.class_name &&
                    visited.find(d.class_name) == std::end(visited)) {
                  visited.emplace(d.class_name);
                  open_list_dependee.emplace_back(dep_ent.dependee, d);
                  create_entries(d.type, focus.type, d.class_name,
                                 focus.class_name, individual_types,
                                 individual_connections);
                }
              }
            }
          }
        }

        for (auto& str : individual_types) single += str;
        single += "\n";
        for (auto& str : individual_connections) single += str;
        single += "@enduml";
        if (print_individual) {
          std::lock_guard lock(print_mtx);
          std::cout << single << std::endl;
        }
        if (print_individual_file) {
          std::ofstream out("./" + cls.class_name + ".puml");
          out << single;
        }
      };

      for (size_t i = 0; i < type_declarations.size(); ++i)
        individual_processing(i);
      // tbb_templates::parallel_for(type_declarations, individual_processing);
    }

    auto& exit_code = emgr_add_component(int);
    exit_code = 0;

    smgr_remove_system(PlantUmlPrinter);
  }

  void Init() {}
  std::vector<std::type_index> Dependencies() { return {}; }
};
