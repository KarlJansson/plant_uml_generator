#include "argument_parser.h"

#include <file_system_utility.hpp>
#include <filesystem>
#include <typeindex>
#include <vector>

#include "arguments.h"
#include "directory_crawler.h"
#include "directory_list.h"
#include "ignore_patterns.h"
#include "model_exporter.h"
#include "model_importer.h"
#include "settings.h"

void ArgumentParser::Init() {}

std::vector<std::type_index> ArgumentParser::Dependencies() { return {}; }

void ArgumentParser::Step(EntityManager_t& ent_mgr, SystemManager_t& sys_mgr) {
  auto& dir_list = ent_mgr.AddComponent<DirectoryList>();
  auto& ignore_patterns = ent_mgr.AddComponent<IgnorePatterns>();
  auto& settings = ent_mgr.AddComponent<Settings>();

  ReadConfigFile(ent_mgr, ignore_patterns.tag_patterns,
                 ignore_patterns.header_lines);

  std::unordered_map<std::string, std::function<void(const std::string&)>>
      command_table{
          {"-d",
           [&](const std::string& arg) {
             if (!arg.empty()) dir_list.directories.emplace_back(arg);
           }},
          {"-ecs_analysis",
           [&](const std::string& arg) {
             settings.flags.insert(Settings::Flag::kEcsAnalysis);
           }},
          {"-i",
           [&](const std::string& arg) {
             if (!arg.empty())
               ignore_patterns.ignore_patterns.emplace_back(arg);
           }},
          {"-s",
           [&](const std::string& arg) {
             if (!arg.empty()) ignore_patterns.stop_patterns.emplace_back(arg);
           }},
          {"-if",
           [&](const std::string& arg) {
             if (!arg.empty()) ignore_patterns.file_patterns.emplace_back(arg);
           }},
          {"-md",
           [&](const std::string& arg) {
             if (!arg.empty()) settings.max_dependents = std::stoi(arg);
           }},
          {"-export",
           [&](const std::string& arg) {
             if (!arg.empty()) settings.export_path = arg;
           }},
          {"-import",
           [&](const std::string& arg) {
             if (!arg.empty()) settings.import_path = arg;
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

  auto arguments = ent_mgr.ComponentR<Arguments>();
  for (auto& arg : arguments->args) {
    if (auto it = command_table.find(arg); it != std::end(command_table)) {
      argument_distributer = it->second;
      try {
        argument_distributer("");
      } catch (...) {
        std::cout << "faulty input detected" << std::endl;
      }
    } else {
      try {
        argument_distributer(arg);
      } catch (...) {
        std::cout << "faulty input detected" << std::endl;
      }
    }
  }

  if ((arguments->args.empty() || dir_list.directories.empty() ||
       settings.flags.empty()) &&
      settings.export_path.empty() && settings.import_path.empty()) {
    std::cout << "Need at least one directory path and one print flag:\n"
                 "  -d <director_path> - Analyze files in folders\n"
                 "  -i <ignore_pattern> - Ignores types including patterns\n"
                 "  -if <ignore_pattern> - Ignores files including patterns\n"
                 "  -s <ignore_pattern> - Stops on types including patterns\n"
                 "  -md <max_dep> - Set max dependents to continue\n"
                 "  -export - Export model\n"
                 "  -import - Import model\n"
                 "  -el <expand_steps> - Expand individual diargrams\n"
                 "  -pi - Print individual collaboration diagrams for types\n"
                 "  -pif - Print individual collaboration diagrams to file\n"
                 "  -pf - Print full collaboration diagram for found types\n"
              << std::endl;
    auto& exit_code = ent_mgr.AddComponent<int>();
    exit_code = 0;
  } else if (!settings.import_path.empty())
    sys_mgr.AddSystem<ModelImporter>();
  else
    sys_mgr.AddSystem<DirectoryCrawler>();
  sys_mgr.RemoveSystem<ArgumentParser>();
}

void ArgumentParser::ReadConfigFile(
    EntityManager_t& ent_mgr,
    std::vector<std::pair<std::string, std::string>>& patterns,
    std::vector<std::string>& header) {
  std::ifstream in("./.plant_uml_generator_config");
  if (!in.fail()) {
    std::string line;
    while (std::getline(in, line))
      if (line.find(':') != std::string::npos)
        patterns.emplace_back(line.substr(0, line.find(':')),
                              line.substr(line.find(':') + 1));
      else
        header.emplace_back(line);
  }
}
