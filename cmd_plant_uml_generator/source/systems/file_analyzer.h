#pragma once

#include <entity_manager.h>
#include <system_manager.h>

#include <fstream>
#include <typeindex>
#include <vector>

#include "class_declaration.h"
#include "dependent.h"
#include "file_path.h"
#include "plant_uml_printer.h"

class FileAnalyzer {
 public:
  void Init() {}
  std::vector<std::type_index> Dependencies() { return {}; }

  template <typename Ent, typename EntMgr, typename SysMgr>
  void Step(EntMgr& ent_mgr, SysMgr& sys_mgr) {
    FindClassDeclarations<Ent>(ent_mgr);
    FindDependents<Ent>(ent_mgr, sys_mgr);
  }

 private:
  template <typename Ent, typename EntMgr, typename SysMgr>
  void FindDependents(EntMgr& ent_mgr, SysMgr& sys_mgr) {
    bool print{false};
    for (auto [c, ent] : emgr_components_r(ent_mgr, ClassDeclaration)) {
      auto fp = ent_component_r((*ent), FilePath);
      for (auto [file, file_ent] : emgr_components_r(ent_mgr, FilePath)) {
        if (fp->file_path == file->file_path) continue;
        std::string line;
        std::ifstream input(file->file_path);
        while (std::getline(input, line)) {
          if (line.find(c->class_name) != std::string::npos) {
            print = true;
            auto dependent = ent_add_component((*ent), Dependent<EntMgr>);
            dependent->dependent = (*file_ent);
          }
        }
      }
    }

    if (print) {
      smgr_remove_system(sys_mgr, FileAnalyzer);
      smgr_add_system(sys_mgr, PlantUmlPrinter);
    }
  }

  template <typename Ent, typename EntMgr>
  void FindClassDeclarations(EntMgr& ent_mgr) {
    std::set<std::string> found;
    for (auto [c, ent] : emgr_added_components_r(ent_mgr, FilePath)) {
      std::string line;
      std::ifstream input(c->file_path);
      while (std::getline(input, line)) {
        auto p = line.find("class ");
        if (p == std::string::npos) {
          p = line.find("struct ");
          if (p != std::string::npos) p += 7;
        } else if (p != std::string::npos)
          p += 6;
        if (p != std::string::npos) {
          auto p2 = line.find(" ", p);
          if (p2 != std::string::npos) {
            auto class_name = line.substr(p, p2 - p);
            if (IsValid(class_name, found)) {
              auto class_decl = ent_add_component((*ent), ClassDeclaration);
              class_decl->class_name = class_name;
            }
          }
        }
      }
    }
  }

  bool IsValid(const std::string& str, std::set<std::string>& found) {
    if (found.find(str) != std::end(found)) return false;
    found.emplace(str);
    if (str.size() < 5) return false;
    bool all_lower{true};
    for (auto c : str) {
      if (std::isupper(c)) all_lower = false;
      if (!std::isalpha(c)) return false;
    }
    return !all_lower;
  }
};
