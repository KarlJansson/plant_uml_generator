#pragma once

#include <entity_manager.h>
#include <system_manager.h>

#include <filesystem>
#include <typeindex>
#include <vector>

#include "dependee.h"
#include "dependent.h"

class ModelExporter {
 public:
  system_step() {
    std::unordered_map<std::string, size_t> id_lookup;
    std::cout << "<declarations>,";
    for (const auto& [cls, cls_ent] : emgr_components_r(ClassDeclaration)) {
      std::cout << cls.type << "," << cls.class_name << ",";
      id_lookup[cls.class_name] = id_lookup.size();
    }

    std::cout << "<dependents>,";
    for (auto& [cls, cls_ent] : emgr_components_r(ClassDeclaration))
      for (auto& dep : ent_components_r(cls_ent, Dependent))
        for (auto& dep_cls : ent_components_r(dep.entity, ClassDeclaration))
          std::cout << id_lookup[cls.class_name] << ","
                    << id_lookup[dep_cls.class_name] << ",";

    std::cout << "<dependees>,";
    for (auto& [cls, cls_ent] : emgr_components_r(ClassDeclaration))
      for (auto& dep : ent_components_r(cls_ent, Dependee))
        for (auto& dep_cls : ent_components_r(dep.entity, ClassDeclaration))
          std::cout << id_lookup[cls.class_name] << ","
                    << id_lookup[dep_cls.class_name] << ",";

    auto& exit_code = emgr_add_component(int);
    exit_code = 0;
    smgr_remove_system(ModelExporter);
  }

  void Init() {}
  std::vector<std::type_index> Dependencies() { return {}; }
};
