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
    std::cout << "<declarations>,";
    for (const auto& [cls, cls_ent] : emgr_components_r(ClassDeclaration))
      std::cout << cls.type << "," << cls.class_name << ",";

    std::cout << "<dependents>,";
    for (const auto& [cls, cls_ent] : emgr_components_r(ClassDeclaration))
      for (auto& dep : ent_components_r(cls_ent, Dependent<EntMgr>))
        for (auto& dep_cls :
             ent_components_r(const_cast<ecs::Entity<EntMgr>&>(dep.entity),
                              ClassDeclaration))
          std::cout << cls.class_name << "," << dep_cls.class_name << ",";

    std::cout << "<dependees>,";
    for (const auto& [cls, cls_ent] : emgr_components_r(ClassDeclaration))
      for (auto& dep : ent_components_r(cls_ent, Dependee<EntMgr>))
        for (auto& dep_cls :
             ent_components_r(const_cast<ecs::Entity<EntMgr>&>(dep.entity),
                              ClassDeclaration))
          std::cout << cls.class_name << "," << dep_cls.class_name << ",";

    auto& exit_code = emgr_add_component(int);
    exit_code = 0;
    smgr_remove_system(ModelExporter);
  }

  void Init() {}
  std::vector<std::type_index> Dependencies() { return {}; }
};
