#pragma once

#include <entity_manager_simple.h>

using namespace ecss;

#include <system_manager.h>
#include <tbb/tbb.h>

#include <string>
#include <vector>

#include "argument_parser.h"
#include "arguments.h"
#include "directory_list.h"

class MainEntry {
 public:
  static int Main(std::vector<std::string>& args) {
    EntityManager_t ent_mgr;
    ecs::SystemManager<EntityManager_t> sys_mgr;

    ent_mgr.RegisterType<int>();
    ent_mgr.RegisterType<Arguments>();
    ent_mgr.RegisterType<ClassDeclaration>();
    ent_mgr.RegisterType<Dependee>();
    ent_mgr.RegisterType<Dependent>();
    ent_mgr.RegisterType<DirectoryList>();
    ent_mgr.RegisterType<IgnorePatterns>();
    ent_mgr.RegisterType<Settings>();
    ent_mgr.RegisterType<FilePath>();

    auto& arg_comp = ent_mgr.AddComponent<Arguments>();
    arg_comp.args = args;

    sys_mgr.AddSystem<ArgumentParser>();

    ent_mgr.SyncSwap();
    sys_mgr.SyncSystems();
    while (true) {
      sys_mgr.Step(ent_mgr);
      ent_mgr.SyncSwap();
      sys_mgr.SyncSystems();
      if (auto comp = ent_mgr.ComponentR<int>(); comp) return *comp;
    }
  }
};
