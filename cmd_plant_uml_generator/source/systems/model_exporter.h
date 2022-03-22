#pragma once

#include <entity_manager.h>
#include <system_manager.h>

#include <filesystem>
#include <typeindex>
#include <vector>

#include "dependee.h"
#include "dependent.h"
#include "settings.h"

class ModelExporter {
 public:
  system_step_default() {
    std::vector<std::string> class_declarations;
    std::vector<std::string> type_declarations;
    std::vector<std::uint16_t> type_indices;

    std::unordered_map<std::string, std::uint16_t> id_types;
    std::unordered_map<std::string, std::uint16_t> id_lookup;

    for (auto [cls, cls_ent] : emgr_components_r(ClassDeclaration)) {
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

    for (auto [cls, cls_ent] : emgr_components_r(ClassDeclaration))
      for (auto& dep : ent_components_r(Dependent, cls_ent))
        for (auto& dep_cls : ent_components_r(ClassDeclaration, dep.entity))
          dependents.emplace_back(std::make_pair(
              id_lookup[cls.class_name], id_lookup[dep_cls.class_name]));

    std::vector<std::pair<std::uint16_t, std::uint16_t>> dependees;

    for (auto [cls, cls_ent] : emgr_components_r(ClassDeclaration))
      for (auto& dep : ent_components_r(Dependee, cls_ent))
        for (auto& dep_cls : ent_components_r(ClassDeclaration, dep.entity))
          dependees.emplace_back(std::make_pair(id_lookup[cls.class_name],
                                                id_lookup[dep_cls.class_name]));

    auto settings = emgr_component_r(Settings);
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

    auto& exit_code = emgr_add_component(int);
    exit_code = 0;
    smgr_remove_system(ModelExporter);
  }
};
