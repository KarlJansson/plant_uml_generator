#pragma once

#include <entity_manager.h>
#include <system_manager.h>

#include <filesystem>
#include <typeindex>
#include <vector>

#include "directory_list.h"
#include "file_analyzer_type_extraction.h"
#include "file_path.h"
#include "ignore_patterns.h"

class DirectoryCrawler {
 public:
  system_step() {
    auto patterns = emgr_component_r(IgnorePatterns);
    auto check_ignore = [&](auto& check) {
      for (auto& p : patterns->file_patterns)
        if (check.find(p) != std::string::npos) return false;
      return true;
    };

    for (const auto& [c, ent] : emgr_components_r(DirectoryList)) {
      for (auto& str : c.directories) {
        if (!std::filesystem::exists(str)) continue;
        std::unordered_map<std::string, Ent> file_ents;
        for (auto& p : std::filesystem::recursive_directory_iterator(str)) {
          if (std::filesystem::is_regular_file(p)) {
            std::string file_path = p.path();
            if (!check_ignore(file_path)) continue;
            if (file_path.find(".cc") != std::string::npos ||
                file_path.find(".h") != std::string::npos) {
              if (file_path.find(".o") != std::string::npos) continue;
              std::string file_name = p.path().filename();
              file_name = file_name.substr(0, file_name.find_last_of('.'));
              if (file_ents.find(file_name) == std::end(file_ents))
                file_ents[file_name] = ent_mgr.CreateEntity();
              auto& fp = ent_add_component(file_ents[file_name], FilePath);
              fp.file_path = file_path;
              fp.file_name = file_name;
            }
          }
        }
      }
    }

    smgr_remove_system(DirectoryCrawler);
    smgr_add_system(FileAnalyzerTypeExtraction);
  }

  void Init() {}
  std::vector<std::type_index> Dependencies() { return {}; }
};
