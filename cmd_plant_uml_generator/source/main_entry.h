#pragma once

#include <ecs_macros.h>
#include <entity_manager_simple.h>
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
  template <typename EntMgr, typename SysMgr>
  static int Main(std::vector<std::string>& args) {
    EntMgr ent_mgr;
    SysMgr sys_mgr;

    auto& arg_comp = emgr_add_component(Arguments);
    arg_comp.args = args;

    smgr_add_system(ArgumentParser);

    while (true) {
      smgr_step_systems(ent_mgr);
      if (auto comp = emgr_component_r(int); comp) return *comp;
    }
  }
};
