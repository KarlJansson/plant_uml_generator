#include "file_analyzer_depndency_extraction.h"

#include <file_system_utility.hpp>
#include <fstream>
#include <tbb_templates.hpp>

#include "class_declaration.h"
#include "dependee.h"
#include "dependent.h"
#include "file_path.h"
#include "model_exporter.h"
#include "plant_uml_printer.h"
#include "settings.h"

void FileAnalyzerDependencyExtraction::Init() {}

std::vector<std::type_index> FileAnalyzerDependencyExtraction::Dependencies() {
  return {};
}

void FileAnalyzerDependencyExtraction::Step(EntityManager_t& ent_mgr,
                                            SystemManager_t& sys_mgr) {
  auto settings = ent_mgr.ComponentR<Settings>();
  bool ecs_analysis = settings->flags.find(Settings::Flag::kEcsAnalysis) !=
                      std::end(settings->flags);

  auto class_declarations = ent_mgr.ComponentsR<ClassDeclaration>();
  auto func = [&](auto i) {
    auto [c, ent] = class_declarations[i];
    auto fp = ent_mgr.ComponentR<FilePath>(ent);

    auto file_paths = ent_mgr.ComponentsR<FilePath>();
    tbb_templates::parallel_for(file_paths, [&](auto i) {
      auto [file, file_ent] = file_paths[i];
      auto file_content = fsu::FileReader::FileToString(file.file_path);
      if (fp->file_name == file.file_name) {
        for (auto [us, us_e] : ent_mgr.ComponentsR<ClassDeclaration>()) {
          if (us.class_name == c.class_name) continue;
          if (file_content.find(us.class_name) != std::string::npos) {
            auto& dependee = ent_mgr.AddComponent<Dependee>(ent);
            dependee.entity = us_e;
            if (ecs_analysis) {
              // Add extra searches for macro definitions
            }
          }
        }
      } else {
        if (file_content.find(c.class_name) != std::string::npos) {
          auto& dependent = ent_mgr.AddComponent<Dependent>(ent);
          dependent.entity = file_ent;
          if (ecs_analysis) {
            // Add extra searches for macro definitions
          }
        }
      }
    });
  };

  tbb_templates::parallel_for(class_declarations, func);

  sys_mgr.RemoveSystem<FileAnalyzerDependencyExtraction>();

  if (!settings->export_path.empty())
    sys_mgr.AddSystem<ModelExporter>();
  else
    sys_mgr.AddSystem<PlantUmlPrinter>();
}
