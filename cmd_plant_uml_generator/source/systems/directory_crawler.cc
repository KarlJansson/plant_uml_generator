#include "directory_crawler.h"

#include <filesystem>

#include "directory_list.h"
#include "file_analyzer_type_extraction.h"
#include "file_path.h"
#include "ignore_patterns.h"

void DirectoryCrawler::Init() {}

std::vector<std::type_index> DirectoryCrawler::Dependencies() { return {}; }

void DirectoryCrawler::Step(EntityManager_t& ent_mgr,
                            SystemManager_t& sys_mgr) {
  auto patterns = ent_mgr.ComponentR<IgnorePatterns>();
  auto check_ignore = [&](auto& check) {
    for (auto& p : patterns->file_patterns)
      if (check.find(p) != std::string::npos) return false;
    return true;
  };

  for (auto [c, ent] : ent_mgr.ComponentsR<DirectoryList>()) {
    for (auto& str : c.directories) {
      if (!std::filesystem::exists(str)) continue;
      std::unordered_map<std::string, Entity> file_ents;
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
            auto& fp = ent_mgr.AddComponent<FilePath>(file_ents[file_name]);
            fp.file_path = file_path;
            fp.file_name = file_name;
          }
        }
      }
    }
  }

  sys_mgr.RemoveSystem<DirectoryCrawler>();
  sys_mgr.AddSystem<FileAnalyzerTypeExtraction>();
}
