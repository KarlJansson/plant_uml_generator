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
    max_dependents_ = settings->max_dependents;
    if (settings->flags.find(Settings::Flag::kPrintFull) !=
        std::end(settings->flags))
      print_full = true;
    if (settings->flags.find(Settings::Flag::kPrintIndividual) !=
        std::end(settings->flags))
      print_individual = true;
    if (settings->flags.find(Settings::Flag::kPrintIndividualFile) !=
        std::end(settings->flags))
      print_individual_file = true;

    auto ignore_patterns = emgr_component_r(IgnorePatterns);
    ignore_patterns_ = ignore_patterns->ignore_patterns;
    stop_patterns_ = ignore_patterns->stop_patterns;

    if (print_full) {
      std::string full;
      std::unordered_set<std::string> added;
      for (const auto& [cls, cls_ent] : emgr_components_r(ClassDeclaration)) {
        std::unordered_set<std::string> visited;
        Crawl<Ent, EntMgr, SysMgr, Dependent<EntMgr>>(
            cls_ent, &cls, ent_mgr, sys_mgr, visited, 0, 1, added, full,
            [](auto& str1, auto& str2) -> auto {
              return str2 + " ..> " + str1 + "\n";
            },
            cls_ent);

        visited.erase(cls.class_name);

        Crawl<Ent, EntMgr, SysMgr, Dependee<EntMgr>>(
            cls_ent, &cls, ent_mgr, sys_mgr, visited, 0, 1, added, full,
            [](auto& str1, auto& str2) -> auto {
              return str1 + " ..> " + str2 + "\n";
            },
            cls_ent);
      }
      std::cout << header << full << "@enduml\n";
    }

    if (print_individual || print_individual_file) {
      std::mutex print_mtx;
      auto type_declarations = emgr_components_r(ClassDeclaration);
      auto individual_processing = [&](auto i) {
        const auto& [cls, cls_ent] = type_declarations[i];
        std::string single = header;

        single += "title <used> " + cls.class_name + " <using>\n\n";

        std::unordered_set<std::string> added;
        std::unordered_set<std::string> visited;

        Crawl<Ent, EntMgr, SysMgr, Dependent<EntMgr>>(
            cls_ent, &cls, ent_mgr, sys_mgr, visited, 0,
            settings->expansion_level, added, single,
            [](auto& str1, auto& str2) -> auto {
              return str2 + " ..> " + str1 + "\n";
            },
            cls_ent);

        visited.erase(cls.class_name);

        Crawl<Ent, EntMgr, SysMgr, Dependee<EntMgr>>(
            cls_ent, &cls, ent_mgr, sys_mgr, visited, 0,
            settings->expansion_level, added, single,
            [](auto& str1, auto& str2) -> auto {
              return str1 + " ..> " + str2 + "\n";
            },
            cls_ent);
        single += "@enduml\n";

        if (print_individual) {
          std::lock_guard lock(print_mtx);
          std::cout << single;
        }

        if (print_individual_file) {
          std::ofstream out("./" + cls.class_name + ".puml");
          out << single;
        }
      };

      // for (size_t i = 0; i < type_declarations.size(); ++i)
      //   individual_processing(i);
      tbb_templates::parallel_for(type_declarations, individual_processing);
    }

    auto& exit_code = emgr_add_component(int);
    exit_code = 0;

    smgr_remove_system(PlantUmlPrinter);
  }

  void Init() {}
  std::vector<std::type_index> Dependencies() { return {}; }

 private:
  template <typename Ent, typename EntMgr, typename SysMgr, typename Dep>
  void Crawl(Ent& focus, const ClassDeclaration* cls, EntMgr& ent_mgr,
             SysMgr& sys_mgr, std::unordered_set<std::string>& visited,
             size_t depth, size_t max_depth,
             std::unordered_set<std::string>& added, std::string& out,
             std::function<std::string(const std::string&, const std::string&)>
                 connection_entry,
             Ent& origin) {
    if (depth == max_depth) return;
    if (visited.find(cls->class_name) != std::end(visited)) return;
    visited.insert(cls->class_name);

    auto dependencies = ent_components_r(focus, Dep);
    if (dependencies.size() > max_dependents_ && focus != origin) return;
    for (auto& dep_ent : dependencies) {
      auto& dep_ent_casted = const_cast<ecs::Entity<EntMgr>&>(dep_ent.entity);
      for (auto& d : ent_components_r(dep_ent_casted, ClassDeclaration)) {
        if (CheckIgnore(cls->class_name + d.class_name)) {
          Crawl<Ent, EntMgr, SysMgr, Dep>(dep_ent_casted, &d, ent_mgr, sys_mgr,
                                          visited, depth + 1, max_depth, added,
                                          out, connection_entry, origin);

          if (d.class_name != cls->class_name) {
            CheckAndAdd(added, d.type + " " + d.class_name + "\n", out);
            CheckAndAdd(added, cls->type + " " + cls->class_name + "\n", out);
            if (depth == 0 || dep_ent_casted != origin)
              CheckAndAdd(added,
                          connection_entry(cls->class_name, d.class_name), out);
          }
        } else if (CheckStop(cls->class_name + d.class_name)) {
          if (d.class_name != cls->class_name) {
            CheckAndAdd(added, d.type + " " + d.class_name + "\n", out);
            CheckAndAdd(added, cls->type + " " + cls->class_name + "\n", out);
            if (depth == 0 || dep_ent_casted != origin)
              CheckAndAdd(added,
                          connection_entry(cls->class_name, d.class_name), out);
          }
        }
      }
    }
  }

  void CheckAndAdd(std::unordered_set<std::string>& set, std::string str,
                   std::string& out) {
    if (set.find(str) == std::end(set)) {
      set.insert(str);
      out += str;
    }
  }

  bool CheckIgnore(std::string check) {
    for (auto& p : ignore_patterns_)
      if (check.find(p) != std::string::npos) return false;
    return true;
  }

  bool CheckStop(std::string check) {
    for (auto& p : stop_patterns_)
      if (check.find(p) != std::string::npos) return false;
    return true;
  }

  size_t max_dependents_;
  std::vector<std::string> ignore_patterns_;
  std::vector<std::string> stop_patterns_;
};
