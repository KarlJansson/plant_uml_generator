#pragma once

#include <entity_manager.h>
#include <system_manager.h>

using namespace ecss;

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
    SystemManager_t sys_mgr;

    auto& arg_comp = ent_mgr.AddComponent<Arguments>();
    arg_comp.args = args;

    sys_mgr.AddSystem<ArgumentParser>();

    while (true) {
      sys_mgr.Step(ent_mgr);
      ent_mgr.SyncSwap();
      sys_mgr.SyncSystems();
      if (auto comp = ent_mgr.ComponentR<int>(); comp) return *comp;
    }
  }
};
