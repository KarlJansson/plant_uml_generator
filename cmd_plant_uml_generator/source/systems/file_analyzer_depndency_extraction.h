#pragma once

#include <entity_manager.h>
#include <system_manager.h>

#include <file_system_utility.hpp>
#include <fstream>
#include <tbb_templates.hpp>
#include <typeindex>
#include <vector>

#include "class_declaration.h"
#include "dependee.h"
#include "dependent.h"
#include "file_path.h"
#include "plant_uml_printer.h"

class FileAnalyzerDependencyExtraction {
 public:
  void Init() {}
  std::vector<std::type_index> Dependencies() { return {}; }

  template <typename Ent, typename EntMgr, typename SysMgr>
  void Step(EntMgr& ent_mgr, SysMgr& sys_mgr) {
    auto class_declarations = emgr_components_r(ent_mgr, ClassDeclaration);
    tbb_templates::parallel_for(class_declarations, [&](auto i) {
      auto [c, ent] = class_declarations[i];
      auto fp = ent_component_r((*ent), FilePath);

      auto file_paths = emgr_components_r(ent_mgr, FilePath);
      tbb_templates::parallel_for(file_paths, [&](auto i) {
        auto [file, file_ent] = file_paths[i];
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
      });
    });

    smgr_remove_system(sys_mgr, FileAnalyzerDependencyExtraction);
    smgr_add_system(sys_mgr, PlantUmlPrinter);
  }
};
