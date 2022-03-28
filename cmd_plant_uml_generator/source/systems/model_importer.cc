#include "model_importer.h"

#include <filesystem>
#include <fstream>

#include "dependee.h"
#include "dependent.h"
#include "plant_uml_printer.h"
#include "settings.h"

void ModelImporter::Init() {}

void ModelImporter::Step(EntityManager_t& ent_mgr, SystemManager_t& sys_mgr) {
  auto settings = ent_mgr.ComponentR<Settings>();
  std::ifstream in(settings->import_path);
  if (!in.fail()) {
    std::vector<Entity_t> class_entities;
    std::vector<std::string> type_declarations;
    std::vector<std::string> tag_declarations;

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
      tag_declarations.emplace_back(line);
    }

    in.read((char*)&size, sizeof(size));
    std::vector<uint16_t> tag_indices(size / sizeof(std::uint16_t));
    in.read((char*)&tag_indices[0], size);

    in.read((char*)&size, sizeof(size));
    for (size_t i = 0; i < size; ++i) {
      std::getline(in, line, ',');
      class_entities.emplace_back(ent_mgr.CreateEntity());
      auto& cls = ent_mgr.AddComponent<ClassDeclaration>(class_entities.back());
      cls.type = type_declarations[type_indices[i]];
      cls.tag = tag_declarations[tag_indices[i]];
      cls.class_name = line;
    }

    in.read((char*)&size, sizeof(size));
    std::vector<std::pair<std::uint16_t, std::uint16_t>> dependents(
        size / sizeof(std::pair<std::uint16_t, std::uint16_t>));
    in.read((char*)&dependents[0], size);

    for (auto [source, target] : dependents) {
      auto& dep = ent_mgr.AddComponent<Dependent>(class_entities[source]);
      dep.entity = class_entities[target];
    }

    in.read((char*)&size, sizeof(size));
    std::vector<std::pair<std::uint16_t, std::uint16_t>> dependees(
        size / sizeof(std::pair<std::uint16_t, std::uint16_t>));
    in.read((char*)&dependees[0], size);

    for (auto [source, target] : dependees) {
      auto& dep = ent_mgr.AddComponent<Dependee>(class_entities[source]);
      dep.entity = class_entities[target];
    }
  }

  sys_mgr.RemoveSystem<ModelImporter>();
  sys_mgr.AddSystem<PlantUmlPrinter>();
}

std::vector<std::type_index> ModelImporter::Dependencies() { return {}; }
