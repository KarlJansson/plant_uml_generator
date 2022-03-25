#include "model_exporter.h"

#include <filesystem>
#include <fstream>
#include <typeindex>
#include <vector>

#include "class_declaration.h"
#include "dependee.h"
#include "dependent.h"
#include "settings.h"

void ModelExporter::Init() {}

void ModelExporter::Step(EntityManager_t& ent_mgr, SystemManager_t& sys_mgr) {
  std::vector<std::string> class_declarations;
  std::vector<std::string> type_declarations;
  std::vector<std::uint16_t> type_indices;

  std::unordered_map<std::string, std::uint16_t> id_types;
  std::unordered_map<std::string, std::uint16_t> id_lookup;

  for (auto [cls, cls_ent] : ent_mgr.ComponentsR<ClassDeclaration>()) {
    if (id_types.find(cls.type) == std::end(id_types)) {
      id_types[cls.type] = id_types.size();
      type_declarations.emplace_back(cls.type);
    }
    if (id_lookup.find(cls.class_name) == std::end(id_lookup)) {
      id_lookup[cls.class_name] = id_lookup.size();
      class_declarations.emplace_back(cls.class_name);
      type_indices.emplace_back(id_types[cls.type]);
    }
  }

  std::vector<std::pair<std::uint16_t, std::uint16_t>> dependents;

  for (auto [cls, cls_ent] : ent_mgr.ComponentsR<ClassDeclaration>())
    for (auto& dep : ent_mgr.ComponentsR<Dependent>(cls_ent))
      for (auto& dep_cls : ent_mgr.ComponentsR<ClassDeclaration>(dep.entity))
        dependents.emplace_back(std::make_pair(id_lookup[cls.class_name],
                                               id_lookup[dep_cls.class_name]));

  std::vector<std::pair<std::uint16_t, std::uint16_t>> dependees;

  for (auto [cls, cls_ent] : ent_mgr.ComponentsR<ClassDeclaration>())
    for (auto& dep : ent_mgr.ComponentsR<Dependee>(cls_ent))
      for (auto& dep_cls : ent_mgr.ComponentsR<ClassDeclaration>(dep.entity))
        dependees.emplace_back(std::make_pair(id_lookup[cls.class_name],
                                              id_lookup[dep_cls.class_name]));

  auto settings = ent_mgr.ComponentR<Settings>();
  std::ofstream out(settings->export_path);

  auto size = type_declarations.size();
  out.write((char*)&size, sizeof(size));
  for (auto& str : type_declarations) out << str << ",";

  size = type_indices.size() * sizeof(decltype(type_indices)::value_type);
  out.write((char*)&size, sizeof(size));
  out.write((char*)type_indices.data(), size);

  size = class_declarations.size();
  out.write((char*)&size, sizeof(size));
  for (auto& str : class_declarations) out << str << ",";

  size = dependents.size() * sizeof(decltype(dependents)::value_type);
  out.write((char*)&size, sizeof(size));
  out.write((char*)dependents.data(), size);

  size = dependees.size() * sizeof(decltype(dependees)::value_type);
  out.write((char*)&size, sizeof(size));
  out.write((char*)dependees.data(), size);

  auto& exit_code = ent_mgr.AddComponent<int>();
  exit_code = 0;
  sys_mgr.RemoveSystem<ModelExporter>();
}

std::vector<std::type_index> ModelExporter::Dependencies() { return {}; }
