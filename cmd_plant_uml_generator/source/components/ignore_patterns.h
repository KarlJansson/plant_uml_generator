#pragma once

#include <string>
#include <vector>

struct IgnorePatterns {
  std::vector<std::string> ignore_patterns;
  std::vector<std::string> file_patterns;
  std::vector<std::string> stop_patterns;
  std::vector<std::pair<std::string, std::string>> tag_patterns;
  std::vector<std::string> header_lines;
};
