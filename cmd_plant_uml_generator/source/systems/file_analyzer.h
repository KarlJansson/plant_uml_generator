#pragma once

#include <entity_manager.h>
#include <system_manager.h>

#include <file_system_utility.hpp>
#include <fstream>
#include <typeindex>
#include <vector>

#include "class_declaration.h"
#include "dependee.h"
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
        auto file_content = fsu::FileReader::FileToString(file->file_path);
        if (fp->file_name == file->file_name) {
          for (auto [us, us_e] : emgr_components_r(ent_mgr, ClassDeclaration)) {
            if (us->class_name == c->class_name) continue;
            if (file_content.find(us->class_name) != std::string::npos) {
              auto dependee = ent_add_component((*ent), Dependee<EntMgr>);
              dependee->dependee = (*us_e);
            }
          }
        } else {
          if (file_content.find(c->class_name) != std::string::npos) {
            auto dependent = ent_add_component((*ent), Dependent<EntMgr>);
            dependent->dependent = (*file_ent);
          }
        }
      }
      print = true;
    }

    if (print) {
      smgr_remove_system(sys_mgr, FileAnalyzer);
      smgr_add_system(sys_mgr, PlantUmlPrinter);
    }
  }

  template <typename Ent, typename EntMgr>
  void FindClassDeclarations(EntMgr& ent_mgr) {
    std::set<std::string> found;
    for (auto [c, ent] : emgr_components_r(ent_mgr, FilePath)) {
      auto file_content = fsu::FileReader::FileToString(c->file_path);
      auto find_types = [&](const std::string& type_str,
                            const std::string& type) {
        auto p = file_content.find(type_str);
        while (p != std::string::npos) {
          p += type_str.size();
          auto p2 = file_content.find(" ", p);
          if (p2 != std::string::npos) {
            auto class_name = file_content.substr(p, p2 - p);
            if (IsValid(class_name, found)) {
              auto class_decl = ent_add_component((*ent), ClassDeclaration);
              class_decl->class_name = class_name;
              class_decl->type = type;
            }
            p = file_content.find(type_str, p2);
          }
        }
      };
      find_types("class ", "class");
      find_types("struct ", "enum");
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
