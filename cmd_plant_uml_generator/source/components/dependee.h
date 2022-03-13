
#pragma once

#include "entity.h"

template <typename Ent>
struct Dependee {
  ecs::Entity<Ent> dependee;
};
