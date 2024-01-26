#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace clogparser {
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