#ifndef Entity_h_seen
#define Entity_h_seen

#include <string>

struct Entity {
  const char * const name;
  const unsigned int value;
};

#define ENTITY_COUNT 259
extern const Entity s_entity[ENTITY_COUNT];

#endif
