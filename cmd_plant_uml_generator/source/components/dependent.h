#pragma once

#include "entity.h"

template <typename Ent>
struct Dependent {
  ecs::Entity<Ent> entity;
};
