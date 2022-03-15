#pragma once

struct Settings {
  enum class Flag { kPrintFull, kPrintIndividual, kPrintIndividualFile };
  std::unordered_set<Flag> flags;
  size_t expansion_level{1};
};
