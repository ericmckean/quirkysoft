#ifndef SpellData_h_seen
#define SpellData_h_seen
#include <nds/jtypes.h>
typedef void (*FunctionPtr_t)(void);
struct SpellData {
  const char * spellName;
  s8 chaosRating;
  u8 castChance;
  u8 castRange;
  u8 castPriority;
  u8 palette;
  u8 combat;
  u8 rangedCombat;
  u8 rangedCombatRange;
  u8 defence;
  u8 movement;
  u8 manvr;
  u8 magicRes;
  FunctionPtr_t spellFunction;
  const u16 * gfxData;
  const u16 * mapData;
#include "SpellDataImpl.h"
};
extern const SpellData s_spellData[];
typedef enum {
  SPELL_DISBELIEVE,
  SPELL_KING_COBRA,
  SPELL_DIRE_WOLF,
  SPELL_GOBLIN,
  SPELL_CROCODILE,
  SPELL_TROLL,
  SPELL_FAUN,
  SPELL_LION,
  SPELL_ELF,
  SPELL_ORC,
  SPELL_BEAR,
  SPELL_GORILLA,
  SPELL_OGRE,
  SPELL_HYDRA,
  SPELL_GIANT_RAT,
  SPELL_GIANT,
  SPELL_HORSE,
  SPELL_UNICORN,
  SPELL_CENTAUR,
  SPELL_PEGASUS,
  SPELL_GRYPHON,
  SPELL_MANTICORE,
  SPELL_BAT,
  SPELL_GREEN_DRAGON,
  SPELL_RED_DRAGON,
  SPELL_GOLDEN_DRAGON,
  SPELL_HARPY,
  SPELL_EAGLE,
  SPELL_VAMPIRE,
  SPELL_GHOST,
  SPELL_SPECTRE,
  SPELL_WRAITH,
  SPELL_SKELETON,
  SPELL_ZOMBIE,
  SPELL_GOOEY_BLOB,
  SPELL_MAGIC_FIRE,
  SPELL_MAGIC_WOOD,
  SPELL_SHADOW_WOOD,
  SPELL_MAGIC_CASTLE,
  SPELL_DARK_CITADEL,
  SPELL_WALL,
  SPELL_MAGIC_BOLT,
  SPELL_LIGHTNING,
  SPELL_VENGEANCE,
  SPELL_DECREE,
  SPELL_DARK_POWER,
  SPELL_JUSTICE,
  SPELL_MAGIC_SHIELD,
  SPELL_MAGIC_ARMOUR,
  SPELL_MAGIC_SWORD,
  SPELL_MAGIC_KNIFE,
  SPELL_MAGIC_BOW,
  SPELL_MAGIC_WINGS,
  SPELL_LAW_1,
  SPELL_LAW_2,
  SPELL_CHAOS_1,
  SPELL_CHAOS_2,
  SPELL_SHADOW_FORM,
  SPELL_SUBVERSION,
  SPELL_RAISE_DEAD,
  SPELL_TURMOIL
} SpellID_t;
#endif