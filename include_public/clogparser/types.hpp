#pragma once

#include <cstdint>
#include <array>

namespace clogparser {
  struct Unit_flags {
  public:
    using Backing = std::uint32_t;

    constexpr Unit_flags(Backing val) noexcept :
      val_(val) {

    }


    enum class Affiliation : Backing {
      mine = 0x00000001,
      party = 0x00000002,
      raid = 0x00000004,
      outsider = 0x00000008
    };
    enum class Reaction : Backing {
      friendly = 0x00000010,
      neutral = 0x00000020,
      hostile = 0x00000040
    };
    enum class Ownership : Backing {
      player = 0x00000100,
      npc = 0x00000200
    };
    enum class Unit_type : Backing {
      player = 0x00000400,
      npc = 0x00000800,
      pet = 0x00001000,
      guardian = 0x00002000,
      object = 0x00004000
    };
    enum class Special_cases : Backing {
      target = 0x00010000,
      focus = 0x00020000,
      main_tank = 0x00040000,
      main_assist = 0x00080000,
      none = 0x80000000
    };

    constexpr bool is(Affiliation test) const noexcept {
      return (val_ & static_cast<Backing>(test)) != 0;
    }
    constexpr bool is(Reaction test) const noexcept {
      return (val_ & static_cast<Backing>(test)) != 0;
    }
    constexpr bool is(Ownership test) const noexcept {
      return (val_ & static_cast<Backing>(test)) != 0;
    }
    constexpr bool is(Unit_type test) const noexcept {
      return (val_ & static_cast<Backing>(test)) != 0;
    }
    constexpr bool is(Special_cases test) const noexcept {
      return (val_ & static_cast<Backing>(test)) != 0;
    }

  private:
    Backing val_;
  };

  struct Raid_flags {
  public:
    constexpr Raid_flags(std::uint32_t val) :
      val_(val) {

    }

    enum class Marker : std::uint32_t {
      star = 0x00000001,
      circle = 0x00000002,
      diamond = 0x00000004,
      triangle = 0x00000008,
      moon = 0x00000010,
      square = 0x00000020,
      cross = 0x00000040,
      skull = 0x00000080
    };

    constexpr bool is(Marker) const noexcept;

  private:
    std::uint32_t val_;
  };

  struct Spell_schools {
  public:
    constexpr Spell_schools(std::uint8_t val) :
      val_(val) {

    }

    enum class School : std::uint8_t {
      physical = 0x01,
      holy = 0x02,
      fire = 0x04,
      nature = 0x08,
      frost = 0x10,
      shadow = 0x20,
      arcane = 0x40
    };

    constexpr bool is(School) const noexcept;

  private:
    std::uint8_t val_;
  };

  enum class Power_types {
    health = -2,
    mana = 0,
    rage = 1,
    focus = 2,
    energy = 3,
    combo_points = 4,
    runes = 5,
    runic_power = 6,
    soul_shards = 7,
    lunar_power = 8,
    holy_power = 9,
    alternate_power = 10,
    maelstrom = 11,
    chi = 12,
    insanity = 13,
    arcane_charges = 16,
    fury = 17,
    pain = 18
  };

  enum class Instance_type {
    not_instanced = 0,
    party_dungeon = 1,
    raid_dungeon = 2,
    pvp_battlefield = 3,

    scenario = 5
  };

  enum class Difficulty : std::uint8_t {
    normal_dungeon_1 = 1,
    heroic_dungeon = 2,
    player_10_raid = 3,
    player_25_raid = 4,
    player_10_heroic_raid = 5,
    player_25_heroic_raid = 6,
    looking_for_raid_1 = 7,
    mythic_keystone_dungeon = 8,
    player_40_raid = 9,
    heroic_scenario_1 = 11,
    normal_scenario_1 = 12,
    normal_raid = 14,
    heroic_raid = 15,
    mythic_raid = 16,
    looking_for_raid_2 = 17,
    event_raid = 18,
    event_dungeon = 19,
    event_scenario_1 = 20,
    mythic_dungeon = 23,
    timewalking_dungeon = 24,
    world_pvp_scenario_1 = 25,
    pvevp_scenario = 29,
    event_scenario_2 = 30,
    timewalking_raid = 33,
    pvp_battlefield = 34,
    normal_scenario_2 = 38,
    heroic_scenario_2 = 39,
    mythic_scenario = 40,
    pvp_scenario = 45,
    normal_scenario_3 = 147,
    heroic_scenario_3 = 149,
    normal_dungeon_2 = 150,
    looking_for_raid_3 = 151,
    visions_of_nzoth_scenario = 152,
    teeming_island_scenario = 153,
    torghast = 167,
    path_of_ascension_courage = 168,
    path_of_ascension_loyalty = 169,
    path_of_ascension_wisdom = 170,
    path_of_ascension_humility = 171,
    world_boss = 172,
    challenge_level_1 = 192,
    follower = 205,
    world_pvp_scenario_2 = 32
  };

  enum class SpecId : std::uint16_t {
    dk_blood = 250,
    dk_frost = 251,
    dk_unholy = 252,
    dk_unspecced = 1455,

    dh_havoc = 577,
    dh_vengeance = 581,
    dh_unspecced = 1456,

    druid_balance = 102,
    druid_feral = 103,
    druid_guardian = 104,
    druid_restoration = 105,
    druid_unspecced = 1447,

    evoker_devastation = 1467,
    evoker_preservation = 1468,
    evoker_augmentation = 1473,
    evoker_unspecced = 1465,

    hunter_beast_mastery = 253,
    hunter_marksmanship = 254,
    hunter_survival = 255,
    hunter_unspecced = 1448,

    mage_arcane = 62,
    mage_fire = 63,
    mage_frost = 64,
    mage_unspecced = 1449,

    monk_brewmaster = 268,
    monk_windwalker = 269,
    monk_mistweaver = 270,
    monk_unspecced = 1450,

    paladin_holy = 65,
    paladin_protection = 66,
    paladin_retribution = 70,
    paladin_unspecced = 1451,

    priest_discipline = 256,
    priest_holy = 257,
    priest_shadow = 258,
    priest_unspecced = 1452,

    rogue_assassination = 259,
    rogue_outlaw = 260,
    rogue_subtlety = 261,
    rogue_unspecced = 1453,

    shaman_elemental = 262,
    shaman_enhancement = 263,
    shaman_restoration = 264,
    shaman_unspecced = 1444,

    warlock_afflication = 265,
    warlock_demonology = 266,
    warlock_destruction = 267,
    warlock_unspecced = 1454,

    warrior_arms = 71,
    warrior_fury = 72,
    warrior_protection = 73,
    warrior_unspecced = 1446
  };

  enum class FactionId : std::uint8_t {
    horde = 0,
    alliance = 1,
  };

  enum class Attribute_rating : std::uint8_t {
    strength = 0,
    agility,
    stamina,
    intelligence,
    dodge,
    parry,
    block,
    crit_melee,
    crit_ranged,
    crit_spell,
    speed,
    lifesteal,
    haste_melee,
    haste_ranged,
    haste_spell,
    avoidance,
    mastery,
    versatility_damage_done,
    versatility_healing_done,
    versatility_damage_taken,
    armor,
    COUNT
  };

  using Stats = std::array<std::int32_t, static_cast<std::uint8_t>(Attribute_rating::COUNT)>;
}