#include "file_analyzer_type_extraction.h"

#include <file_system_utility.hpp>
#include <fstream>
#include <tbb_templates.hpp>

#include "class_declaration.h"
#include "file_analyzer_depndency_extraction.h"
#include "file_path.h"
#include "ignore_patterns.h"

void FileAnalyzerTypeExtraction::Init() {}

std::vector<std::type_index> FileAnalyzerTypeExtraction::Dependencies() {
  return {};
}

void FileAnalyzerTypeExtraction::Step(EntityManager_t& ent_mgr,
                                      SystemManager_t& sys_mgr) {
  auto ignore_patterns = ent_mgr.ComponentR<IgnorePatterns>();
  auto class_declarations = ent_mgr.ComponentsR<FilePath>();
  tbb_templates::parallel_for(class_declarations, [&](size_t i) {
    auto [c, ent] = class_declarations[i];
    std::set<std::string> found;
    auto file_content = fsu::FileReader::FileToString(c.file_path);
    auto find_types = [&](const std::string& type_str,
                          const std::string& type) {
      auto p = file_content.find(type_str);
      while (p != std::string::npos && p < file_content.size()) {
        p += type_str.size();
        auto p2 = file_content.find(" ", p);
        if (p2 != std::string::npos && p2 < file_content.size()) {
          auto class_name = file_content.substr(p, p2 - p);
          if (IsValid(class_name, found)) {
            auto& class_decl = ent_mgr.AddComponent<ClassDeclaration>(ent);
            class_decl.class_name = class_name;
            class_decl.type = type;

            if (ignore_patterns)
              for (auto& [str, tag] : ignore_patterns->tag_patterns)
                if (c.file_path.find(str) != std::string::npos)
                  class_decl.tag = tag;
          }
          p = file_content.find(type_str, p2);
        }
      }
    };
    find_types("class ", "class");
    find_types("struct ", "enum");
  });

  sys_mgr.RemoveSystem<FileAnalyzerTypeExtraction>();
  sys_mgr.AddSystem<FileAnalyzerDependencyExtraction>();
}

bool FileAnalyzerTypeExtraction::IsValid(const std::string& str,
                                         std::set<std::string>& found) {
  if (found.find(str) != std::end(found)) return false;
  found.emplace(str);
  if (str.size() < 5) return false;
  bool all_lower{true};
  for (auto c : str) {
    if (std::isupper(c)) all_lower = false;
    if (!std::isalpha(c)) return false;
  }
  return !all_lower;
}
