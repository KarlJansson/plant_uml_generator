#include "plant_uml_printer.h"

#include <fstream>
#include <mutex>

#include "dependee.h"
#include "dependent.h"
#include "ignore_patterns.h"
#include "settings.h"

using sf = Settings::Flag;

void PlantUmlPrinter::Init() {}

void PlantUmlPrinter::Step(EntityManager_t& ent_mgr, SystemManager_t& sys_mgr) {
  std::string header =
      "@startuml\n"
      "hide members\n"
      "left to right direction\n\n";

  auto settings = ent_mgr.ComponentR<Settings>();
  auto check_flag = [settings](Settings::Flag flag) -> bool {
    if (settings->flags.find(flag) != std::end(settings->flags)) return true;
    return false;
  };

  max_depth_ = settings->expansion_level;
  max_dependents_ = settings->max_dependents;
  auto print_full = check_flag(sf::kPrintFull);
  auto print_individual = check_flag(sf::kPrintIndividual);
  auto print_individual_file = check_flag(sf::kPrintIndividualFile);

  auto ignore_patterns = ent_mgr.ComponentR<IgnorePatterns>();
  ignore_patterns_ = ignore_patterns->ignore_patterns;
  stop_patterns_ = ignore_patterns->stop_patterns;

  auto dependent_print = [](auto& str1, auto& str2) -> auto {
    return str2 + " ..> " + str1 + "\n";
  };
  auto dependee_print = [](auto& str1, auto& str2) -> auto {
    return str1 + " ..> " + str2 + "\n";
  };

  if (print_full) {
    max_depth_ = 1;
    std::string full;
    std::unordered_set<std::string> added;
    for (auto [cls, cls_ent] : ent_mgr.ComponentsR<ClassDeclaration>()) {
      std::unordered_set<std::string> visited;
      Crawl<Dependent>(ent_mgr, sys_mgr, cls_ent, &cls, visited, 0, added, full,
                       dependent_print, cls_ent);

      visited.erase(cls.class_name);

      Crawl<Dependee>(ent_mgr, sys_mgr, cls_ent, &cls, visited, 0, added, full,
                      dependee_print, cls_ent);
    }
    std::cout << header << full << "@enduml\n";
  }

  if (print_individual || print_individual_file) {
    std::mutex print_mtx;
    auto type_declarations = ent_mgr.ComponentsR<ClassDeclaration>();
    auto individual_processing = [&](auto i) {
      auto [cls, cls_ent] = type_declarations[i];
      std::string single = header;

      single += "title <used> " + cls.class_name + " <using>\n\n";

      std::unordered_set<std::string> added;
      std::unordered_set<std::string> visited;

      Crawl<Dependent>(ent_mgr, sys_mgr, cls_ent, &cls, visited, 0, added,
                       single, dependent_print, cls_ent);

      visited.erase(cls.class_name);

      Crawl<Dependee>(ent_mgr, sys_mgr, cls_ent, &cls, visited, 0, added,
                      single, dependee_print, cls_ent);
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

    tbb_templates::parallel_for(type_declarations, individual_processing);
  }

  auto& exit_code = ent_mgr.AddComponent<int>();
  exit_code = 0;

  sys_mgr.RemoveSystem<PlantUmlPrinter>();
}

std::vector<std::type_index> PlantUmlPrinter::Dependencies() { return {}; }

template <typename Dep>
void PlantUmlPrinter::Crawl(
    EntityManager_t& ent_mgr, SystemManager_t& sys_mgr, const Entity& focus,
    const ClassDeclaration* cls, std::unordered_set<std::string>& visited,
    size_t depth, std::unordered_set<std::string>& added, std::string& out,
    std::function<std::string(const std::string&, const std::string&)>
        conn_entry,
    const Entity& origin) {
  if (depth == max_depth_) return;
  if (visited.find(cls->class_name) != std::end(visited)) return;
  visited.insert(cls->class_name);
  if (CheckStop(cls->class_name) && depth != 0) return;

  auto dependents = ent_mgr.ComponentsR<Dependent>(focus);
  auto dependees = ent_mgr.ComponentsR<Dependee>(focus);
  if (dependents.size() + dependees.size() > max_dependents_ && depth != 0)
    return;
  for (auto& dep_ent : ent_mgr.ComponentsR<Dep>(focus)) {
    for (auto& d : ent_mgr.ComponentsR<ClassDeclaration>(dep_ent.entity)) {
      if (CheckIgnore(cls->class_name + d.class_name)) {
        Crawl<Dep>(ent_mgr, sys_mgr, dep_ent.entity, &d, visited, depth + 1,
                   added, out, conn_entry, origin);

        if (d.class_name != cls->class_name) {
          CheckAndAdd(added, d.type + " " + d.class_name + "\n", out);
          CheckAndAdd(added, cls->type + " " + cls->class_name + "\n", out);
          if (depth == 0 || dep_ent.entity != origin)
            CheckAndAdd(added, conn_entry(cls->class_name, d.class_name), out);
        }
      }
    }
  }
}

void PlantUmlPrinter::CheckAndAdd(std::unordered_set<std::string>& set,
                                  std::string str, std::string& out) {
  if (set.find(str) == std::end(set)) {
    set.insert(str);
    out += str;
  }
}

bool PlantUmlPrinter::CheckIgnore(const std::string& check) {
  for (auto& p : ignore_patterns_)
    if (check.find(p) != std::string::npos) return false;
  return true;
}

bool PlantUmlPrinter::CheckStop(const std::string& check) {
  for (auto& p : stop_patterns_)
    if (check.find(p) != std::string::npos) return true;
  return false;
}
