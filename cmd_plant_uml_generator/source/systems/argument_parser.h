#pragma once

#include <entity_manager.h>
#include <system_manager.h>

#include <filesystem>
#include <typeindex>
#include <vector>

#include "arguments.h"
#include "directory_crawler.h"
#include "ignore_patterns.h"
#include "settings.h"

class ArgumentParser {
 public:
  system_step() {
    auto& dir_list = emgr_add_component(DirectoryList);
    auto& ignore_patterns = emgr_add_component(IgnorePatterns);
    auto& settings = emgr_add_component(Settings);
    auto arguments = emgr_component_w(Arguments);

    std::unordered_map<std::string, std::function<void(const std::string&)>>
        command_table{
            {"-d",
             [&](const std::string& arg) {
               if (!arg.empty()) dir_list.directories.emplace_back(arg);
             }},
            {"-i",
             [&](const std::string& arg) {
               if (!arg.empty()) ignore_patterns.patterns.emplace_back(arg);
             }},
            {"-md",
             [&](const std::string& arg) {
               if (!arg.empty()) settings.max_dependents = std::stoi(arg);
             }},
            {"-el",
             [&](const std::string& arg) {
               if (!arg.empty()) settings.expansion_level = std::stoi(arg);
             }},
            {"-pi",
             [&](const std::string& arg) {
               settings.flags.insert(Settings::Flag::kPrintIndividual);
             }},
            {"-pif",
             [&](const std::string& arg) {
               settings.flags.insert(Settings::Flag::kPrintIndividualFile);
             }},
            {"-pf", [&](const std::string& arg) {
               settings.flags.insert(Settings::Flag::kPrintFull);
             }}};

    auto argument_distributer = command_table["-d"];

    for (auto& arg : arguments->args) {
      if (auto it = command_table.find(arg); it != std::end(command_table)) {
        argument_distributer = it->second;
        try {
          argument_distributer("");
        } catch (...) {
        }
      } else {
        try {
          argument_distributer(arg);
        } catch (...) {
        }
      }
    }

    if (arguments->args.empty() || dir_list.directories.empty() ||
        settings.flags.empty()) {
      std::cout << "Need at least one directory path and one print flag:\n"
                   "  -d <director_path> - Analyze files in folders\n"
                   "  -i <ignore_pattern> - Ignores types including patterns\n"
                   "  -md <max_dep> - Set max dependents to continue\n"
                   "  -el <expand_steps> - Expand individual diargrams\n"
                   "  -pi - Print individual collaboration diagrams for types\n"
                   "  -pif - Print individual collaboration diagrams to file\n"
                   "  -pf - Print full collaboration diagram for found types\n"
                << std::endl;
      auto& exit_code = emgr_add_component(int);
      exit_code = 0;
    } else
      smgr_add_system(DirectoryCrawler);
  }

  void Init() {}
  std::vector<std::type_index> Dependencies() { return {}; }
};
