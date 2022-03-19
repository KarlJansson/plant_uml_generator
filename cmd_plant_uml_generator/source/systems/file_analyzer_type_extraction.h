#pragma once

#include <entity_manager.h>
#include <system_manager.h>

#include <file_system_utility.hpp>
#include <fstream>
#include <tbb_templates.hpp>
#include <typeindex>
#include <vector>

#include "class_declaration.h"
#include "file_analyzer_depndency_extraction.h"
#include "file_path.h"

class FileAnalyzerTypeExtraction {
 public:
  system_step() {
    auto class_declarations = emgr_components_r(FilePath);
    tbb_templates::parallel_for(class_declarations, [&](size_t i) {
      auto& [c, ent] = class_declarations[i];
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
              auto& class_decl = ent_add_component(ent, ClassDeclaration);
              class_decl.class_name = class_name;
              class_decl.type = type;
            }
            p = file_content.find(type_str, p2);
          }
        }
      };
      find_types("class ", "class");
      find_types("struct ", "enum");
    });

    smgr_remove_system(FileAnalyzerTypeExtraction);
    smgr_add_system(FileAnalyzerDependencyExtraction);
  }

  void Init() {}
  std::vector<std::type_index> Dependencies() { return {}; }

 private:
  bool IsValid(const std::string& str, std::set<std::string>& found) {
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
};
