#pragma once

#include <entity_manager.h>
#include <system_manager.h>

#include <filesystem>
#include <typeindex>
#include <vector>

#include "dependee.h"
#include "dependent.h"
#include "plant_uml_printer.h"

class ModelImporter {
 public:
  system_step() {
    std::unordered_map<std::string, Ent> entity_map;
    auto capture_declarations = [&](auto& in) {
      std::string type, decl;
      while (std::getline(in, type, ',')) {
        if (type == "<dependents>") break;
        auto ent = ent_mgr.CreateEntity();
        if (std::getline(in, decl, ',')) {
          auto& cls = ent_add_component(ent, ClassDeclaration);
          cls.type = type;
          cls.class_name = decl;
          entity_map[decl] = ent;
        }
      }
    };
    auto capture_dependents = [&](auto& in) {
      std::string source, target;
      while (std::getline(in, source, ',')) {
        if (source == "<dependees>") break;
        if (std::getline(in, target, ',')) {
          auto& dep = ent_add_component(entity_map[source], Dependent<EntMgr>);
          dep.entity = entity_map[target];
        }
      }
    };
    auto capture_dependees = [&](auto& in) {
      std::string source, target;
      while (std::getline(in, source, ',')) {
        if (std::getline(in, target, ',')) {
          auto& dep = ent_add_component(entity_map[source], Dependee<EntMgr>);
          dep.entity = entity_map[target];
        }
      }
    };

    auto settings = emgr_component_r(Settings);
    std::ifstream in(settings->import_path);
    if (!in.fail()) {
      std::string line;
      while (std::getline(in, line, ',')) {
        if (line == "<declarations>") {
          capture_declarations(in);
          capture_dependents(in);
          capture_dependees(in);
        }
      }
    }

    smgr_remove_system(ModelImporter);
    smgr_add_system(PlantUmlPrinter);
  }

  void Init() {}
  std::vector<std::type_index> Dependencies() { return {}; }
};
