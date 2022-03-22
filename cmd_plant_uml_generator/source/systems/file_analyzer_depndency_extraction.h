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
  system_step_default() {
    auto settings = emgr_component_r(Settings);
    bool ecs_analysis = settings->flags.find(Settings::Flag::kEcsAnalysis) !=
                        std::end(settings->flags);

    auto class_declarations = emgr_components_r(ClassDeclaration);
    auto func = [&](auto i) {
      auto [c, ent] = class_declarations[i];
      auto fp = ent_component_r(FilePath, ent);

      auto file_paths = emgr_components_r(FilePath);
      tbb_templates::parallel_for(file_paths, [&](auto i) {
        auto [file, file_ent] = file_paths[i];
        auto file_content = fsu::FileReader::FileToString(file.file_path);
        if (fp->file_name == file.file_name) {
          for (auto [us, us_e] : emgr_components_r(ClassDeclaration)) {
            if (us.class_name == c.class_name) continue;
            if (file_content.find(us.class_name) != std::string::npos) {
              auto& dependee = ent_add_component(Dependee, ent);
              dependee.entity = us_e;
              if (ecs_analysis) {
                // Add extra searches for macro definitions
              }
            }
          }
        } else {
          if (file_content.find(c.class_name) != std::string::npos) {
            auto& dependent = ent_add_component(Dependent, ent);
            dependent.entity = file_ent;
            if (ecs_analysis) {
              // Add extra searches for macro definitions
            }
          }
        }
      });
    };

    tbb_templates::parallel_for(class_declarations, func);

    smgr_remove_system(FileAnalyzerDependencyExtraction);

    if (!settings->export_path.empty())
      smgr_add_system(ModelExporter);
    else
      smgr_add_system(PlantUmlPrinter);
  }
};
