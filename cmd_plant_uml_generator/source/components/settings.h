#pragma once

#include <limits>

struct Settings {
  enum class Flag {
    kPrintFull,
    kPrintIndividual,
    kPrintIndividualFile,
    kEcsAnalysis
  };
  std::unordered_set<Flag> flags;
  size_t expansion_level{1};
  size_t max_dependents{std::numeric_limits<size_t>::max()};
  std::string export_path;
  std::string import_path;
};
