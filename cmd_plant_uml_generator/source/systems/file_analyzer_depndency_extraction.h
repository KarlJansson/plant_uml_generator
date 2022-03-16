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
#include "model_exporter.h"
#include "plant_uml_printer.h"
#include "settings.h"

class FileAnalyzerDependencyExtraction {
 public:
  system_step() {
    auto class_declarations = emgr_components_r(ClassDeclaration);
    tbb_templates::parallel_for(class_declarations, [&](auto i) {
      const auto& [c, ent] = class_declarations[i];
      auto fp = ent_component_r(ent, FilePath);

      auto file_paths = emgr_components_r(FilePath);
      tbb_templates::parallel_for(file_paths, [&](auto i) {
        const auto& [file, file_ent] = file_paths[i];
        auto file_content = fsu::FileReader::FileToString(file.file_path);
        if (fp->file_name == file.file_name) {
          for (const auto& [us, us_e] : emgr_components_r(ClassDeclaration)) {
            if (us.class_name == c.class_name) continue;
            if (file_content.find(us.class_name) != std::string::npos) {
              auto& dependee = ent_add_component(ent, Dependee<EntMgr>);
              dependee.entity = us_e;
            }
          }
        } else {
          if (file_content.find(c.class_name) != std::string::npos) {
            auto& dependent = ent_add_component(ent, Dependent<EntMgr>);
            dependent.entity = file_ent;
          }
        }
      });
    });

    smgr_remove_system(FileAnalyzerDependencyExtraction);

    auto settings = emgr_component_r(Settings);
    if (settings->export_model)
      smgr_add_system(ModelExporter);
    else
      smgr_add_system(PlantUmlPrinter);
  }

  void Init() {}
  std::vector<std::type_index> Dependencies() { return {}; }
};
