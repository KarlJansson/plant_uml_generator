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
  system_step_default() {
    auto settings = emgr_component_r(Settings);
    std::ifstream in(settings->import_path);
    if (!in.fail()) {
      std::vector<Entity_t> class_entities;
      std::vector<std::string> type_declarations;

      size_t size;
      std::string line;
      in.read((char*)&size, sizeof(size));
      for (size_t i = 0; i < size; ++i) {
        std::getline(in, line, ',');
        type_declarations.emplace_back(line);
      }

      in.read((char*)&size, sizeof(size));
      std::vector<uint16_t> type_indices(size / sizeof(std::uint16_t));
      in.read((char*)&type_indices[0], size);

      in.read((char*)&size, sizeof(size));
      for (size_t i = 0; i < size; ++i) {
        std::getline(in, line, ',');
        class_entities.emplace_back(ent_mgr.CreateEntity());
        auto& cls = ent_add_component(ClassDeclaration, class_entities.back());
        cls.type = type_declarations[type_indices[i]];
        cls.class_name = line;
      }

      in.read((char*)&size, sizeof(size));
      std::vector<std::pair<std::uint16_t, std::uint16_t>> dependents(
          size / sizeof(std::pair<std::uint16_t, std::uint16_t>));
      in.read((char*)&dependents[0], size);

      for (auto [source, target] : dependents) {
        auto& dep = ent_add_component(Dependent, class_entities[source]);
        dep.entity = class_entities[target];
      }

      in.read((char*)&size, sizeof(size));
      std::vector<std::pair<std::uint16_t, std::uint16_t>> dependees(
          size / sizeof(std::pair<std::uint16_t, std::uint16_t>));
      in.read((char*)&dependees[0], size);

      for (auto [source, target] : dependees) {
        auto& dep = ent_add_component(Dependee, class_entities[source]);
        dep.entity = class_entities[target];
      }
    }

    smgr_remove_system(ModelImporter);
    smgr_add_system(PlantUmlPrinter);
  }
};
