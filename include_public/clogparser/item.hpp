#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace clogparser {
  enum class Item_slot : std::uint8_t {
    non_equipable = 0,
    head = 1,
    neck = 2,
    shoulder = 3,
    shirt = 4,
    chest = 5,
    waist = 6,
    legs = 7,
    feet = 8,
    wrist = 9,
    hands = 10,
    finger = 11,
    trinket = 12,
    one_hand = 13,
    shield = 14,
    bow = 15,
    back = 16,
    two_hand = 17,
    bag = 18,
    tabard = 19,
    robe = 20,
    main_hand = 21,
    off_hand_weapon = 22,
    off_hand = 23,
    ammo = 24,
    thrown = 25,
    ranged_2 = 26
  };

  struct Item {
    std::uint64_t item_id;
    std::uint16_t ilvl;
    std::optional<std::uint64_t> permanent_enchant_id;
    std::optional<std::uint64_t> temp_enchant_id;
    std::optional<std::uint64_t> on_use_spell_enchant_id;
    std::vector<std::uint64_t> bonus_ids;
    std::vector<std::uint64_t> gem_ids;
  };
}