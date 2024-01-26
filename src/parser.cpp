#include <clogparser/parser.hpp>

#include <array>
#include <string_view>
#include <cstdint>
#include <stdexcept>
#include <charconv>
#include <span>
#include <optional>
#include <unordered_map>

namespace events = clogparser::events;

namespace {
  events::Unit convert(clogparser::String_store& store, events::Unit in) {
    return {
      store.get(in.guid),
      store.get(in.name),
      in.flags,
      in.raid_flags
    };
  }
  events::Combat_header convert(clogparser::String_store& store, events::Combat_header in) {
    return {
      convert(store, in.source),
      convert(store ,in.dest)
    };
  }
  events::Spell_info convert(clogparser::String_store& store, events::Spell_info in) {
    return {
      in.id,
      store.get(in.name),
      in.school
    };
  }
  events::Advanced_info convert(clogparser::String_store& store, events::Advanced_info in) {
    return {
      store.get(in.advanced_unit_guid),
      store.get(in.owner_guid),
      in.current_hp,
      in.max_hp,
      in.attack_power,
      in.spell_power,
      in.armor,
      in.absorb,
      in.power_type,
      in.current_power,
      in.max_power,
      in.power_cost,
      in.position_x,
      in.position_y,
      in.map_id,
      in.facing,
      in.level
    };
  }
  events::Damage convert(clogparser::String_store&, events::Damage in) {
    return in;
  }
  events::Heal convert(clogparser::String_store&, events::Heal in) {
    return in;
  }

  std::optional<clogparser::Timestamp> parse_timestamp(std::string_view time) {
    try {
      clogparser::Timestamp returning{ 0 };

      const auto found_month = time.find('/');
      if (found_month == time.npos) {
        fprintf(stderr, "Timestamp is missing a month: %.*s\n", (std::uint32_t)time.size(), time.data());
        return std::nullopt;
      }
      returning.month = clogparser::internal::parseInt<std::uint8_t>(time.substr(0, found_month));
      time = time.substr(found_month + 1);

      const auto found_day = time.find(' ');
      if (found_day == time.npos) {
        fprintf(stderr, "Timestamp is missing a day: %.*s\n", (std::uint32_t)time.size(), time.data());
        return std::nullopt;
      }
      returning.day = clogparser::internal::parseInt<std::uint8_t>(time.substr(0, found_day));
      time = time.substr(found_day + 1);

      const auto found_hour = time.find(':');
      if (found_hour == time.npos) {
        fprintf(stderr, "Timestamp is missing an hour: %.*s\n", (std::uint32_t)time.size(), time.data());
        return std::nullopt;
      }
      returning.hour = clogparser::internal::parseInt<std::uint8_t>(time.substr(0, found_hour));
      time = time.substr(found_hour + 1);

      const auto found_minute = time.find(':');
      if (found_minute == time.npos) {
        fprintf(stderr, "Timestamp is missing a minute: %.*s\n", (std::uint32_t)time.size(), time.data());
        return std::nullopt;
      }
      returning.minute = clogparser::internal::parseInt<std::uint8_t>(time.substr(0, found_minute));
      time = time.substr(found_minute + 1);

      const auto found_second = time.find('.');
      if (found_second == time.npos) {
        fprintf(stderr, "Timestamp is missing a second: %.*s\n", (std::uint32_t)time.size(), time.data());
        return std::nullopt;
      }
      returning.second = clogparser::internal::parseInt<std::uint8_t>(time.substr(0, found_second));
      time = time.substr(found_second + 1);

      returning.millisecond = clogparser::internal::parseInt<std::uint16_t>(time);

      return returning;
    } catch (std::exception const& ex) {
      fprintf(stderr, "Couldn't parse timestamp due to exception: %s\n", ex.what());
      return std::nullopt;
    }
  }

  void parse_array(std::vector<std::string_view>& returning, std::string_view in) {
    std::string_view::size_type found = 0;
    bool quoted = false;

    while (in.size() > 0) {
      if (in[0] == '"') {
        const auto found = in.find("\",");
        if (found == in.npos) {
          break;
        }
        returning.push_back(in.substr(1, found - 1));
        in = in.substr(found + 2);
      } else if (in[0] == '[' || in[0] == '(') {
        bool found = false;
        bool in_quote = false;
        int bracket_open_count = 1;
        for (std::string_view::size_type i = 1; i < in.size(); ++i) {
          if (!in_quote && (in[i] == '[' || in[i] == '(')) {
            bracket_open_count++;
          } else if (!in_quote && (in[i] == ']' || in[i] == ')')) {
            bracket_open_count--;
          } else if (in[i] == '"') {
            in_quote = !in_quote;
          } else if (in[i] == ',' && !in_quote && bracket_open_count == 0) {
            returning.push_back(in.substr(1, i - 2));
            in = in.substr(i + 1);
            found = true;
            break;
          }
        }
        if (!found) {
          break;
        }
      } else {
        const auto found = in.find(',');
        if (found == in.npos) {
          break;
        }
        returning.push_back(in.substr(0, found));
        in = in.substr(found + 1);
      }
    }
    if (in.size() > 0 && in[0] == '"') {
      if (in.size() > 1 && in.back() == '"') {
        returning.push_back(in.substr(1, in.size() - 2));
      } else {
        throw std::exception("Couldn't find quote termiantor");
      }
    } else if (in.size() > 0 && in[0] == '[') {
      if (in.size() > 1 && in.back() == ']') {
        returning.push_back(in.substr(1, in.size() - 2));
      } else {
        throw std::exception("Couldn't find array terminator");
      }
    } else if (in.size() > 0 && in[0] == '(') {
      if (in.size() > 1 && in.back() == ')') {
        returning.push_back(in.substr(1, in.size() - 2));
      } else {
        throw std::exception("Couldn't find tuple terminator");
      }
    } else {
      returning.push_back(in);
    }
  }
  std::vector<std::string_view> parse_array(std::string_view in) {
    std::vector<std::string_view> returning;
    parse_array(returning, in);
    return returning;
  }

  std::vector<clogparser::Item> parse_items(std::string_view in) {
    std::vector<clogparser::Item> returning;
    const auto parsed_in = parse_array(in);

    std::vector<std::string_view> parsed_sub_in;

    std::vector<std::string_view> temp;

    for (auto const& sub_in : parsed_in) {
      parsed_sub_in.clear();
      parse_array(parsed_sub_in, sub_in);

      if (parsed_sub_in.size() != 5) {
        throw std::exception("Item didn't have 5 fields");
      }

      returning.emplace_back();
      returning.back().item_id = clogparser::internal::parseInt<std::uint64_t>(parsed_sub_in[0]);
      returning.back().ilvl = clogparser::internal::parseInt<std::uint16_t>(parsed_sub_in[1]);

      temp.clear();
      parse_array(temp, parsed_sub_in[2]);

      if (temp.size() == 0 || (temp.size() == 1 && temp.front().empty())) {
        //do nothing, they're all nullopt
      } else if (temp.size() == 3) {
        const auto e1 = clogparser::internal::parseInt<std::uint64_t>(temp[0]);
        if (e1 != 0) {
          returning.back().permanent_enchant_id = e1;
        }
        const auto e2 = clogparser::internal::parseInt<std::uint64_t>(temp[1]);
        if (e2 != 0) {
          returning.back().temp_enchant_id = e2;
        }
        const auto e3 = clogparser::internal::parseInt<std::uint64_t>(temp[2]);
        if (e3 != 0) {
          returning.back().on_use_spell_enchant_id = e3;
        }
      } else {
        throw std::exception("Item enchants didn't have 0 or 3 fields");
      }



      if (!parsed_sub_in[3].empty()) {
        temp.clear();
        parse_array(temp, parsed_sub_in[3]);
        for (auto const& bonus_id : temp) {
          returning.back().bonus_ids.push_back(clogparser::internal::parseInt<std::uint64_t>(bonus_id));
        }
      }

      

      if (!parsed_sub_in[4].empty()) {
        temp.clear();
        parse_array(temp, parsed_sub_in[4]);
        for (auto const& gem : temp) {
          returning.back().gem_ids.push_back(clogparser::internal::parseInt<std::uint64_t>(gem));
        }
      }
    }

    return returning;
  }

  std::vector<clogparser::events::Combatant_info::Talent> parse_talents(std::string_view in) {
    std::vector<clogparser::events::Combatant_info::Talent> returning;

    if (in.empty()) {
      return returning;
    }

    std::vector<std::string_view> parsed_in = parse_array(in);
    std::vector<std::string_view> parsed_talent;

    for (auto const& sub_in : parsed_in) {
      parsed_talent.clear();
      parse_array(parsed_talent, sub_in);

      if (parsed_talent.size() != 3) {
        throw std::exception("Talent didn't have 3 fields");
      }

      clogparser::events::Combatant_info::Talent adding{ 0 };
      //parsed_talent[0] is unknown
      adding.trait_node_entry_id = clogparser::internal::parseInt<std::uint32_t>(parsed_talent[1]);
      adding.rank = clogparser::internal::parseInt<std::uint8_t>(parsed_talent[2]);

      returning.push_back(adding);
    }

    return returning;
  }
}
std::chrono::milliseconds clogparser::Timestamp::operator-(Timestamp const& other) const noexcept {
  const std::chrono::milliseconds us_duration = std::chrono::hours{ hour }
    + std::chrono::minutes{ minute }
    + std::chrono::seconds{ second }
    + std::chrono::milliseconds{ millisecond };

  const std::chrono::milliseconds other_duration = std::chrono::hours{ other.hour }
    + std::chrono::minutes{ other.minute }
    + std::chrono::seconds{ other.second }
    + std::chrono::milliseconds{ other.millisecond };

  if (us_duration < other_duration) {
    return std::chrono::days{ 1 } + us_duration - other_duration;
  } else {
    return us_duration - other_duration;
  }
}

std::size_t clogparser::internal::String_hash::operator()(std::string const& str) const noexcept {
  std::hash<std::string_view> hasher;
  return hasher(str);
}
std::size_t clogparser::internal::String_hash::operator()(std::string_view str) const noexcept {
  std::hash<std::string_view> hasher;
  return hasher(str);
}

clogparser::internal::Columns clogparser::internal::parse_array(Columns returning, std::string_view in) {
  std::string_view::size_type found = 0;
  bool quoted = false;

  std::size_t on = 0;

  while (in.size() > 0) {
    if (in[0] == '"') {
      const auto found = in.find("\",");
      if (found == in.npos) {
        break;
      }
      returning[on] = in.substr(1, found - 1);
      ++on;
      if (on == returning.size()) {
        return returning;
      }
      in = in.substr(found + 2);
    } else if (in[0] == '[' || in[0] == '(') {
      bool found = false;
      bool in_quote = false;
      int bracket_open_count = 1;
      for (std::string_view::size_type i = 1; i < in.size(); ++i) {
        if (!in_quote && (in[i] == '[' || in[i] == '(')) {
          bracket_open_count++;
        } else if (!in_quote && (in[i] == ']' || in[i] == ')')) {
          bracket_open_count--;
        } else if (in[i] == '"') {
          in_quote = !in_quote;
        } else if (in[i] == ',' && !in_quote && bracket_open_count == 0) {
          returning[on] = in.substr(1, i - 2);
          ++on;
          if (on == returning.size()) {
            return returning;
          }
          in = in.substr(i + 1);
          found = true;
          break;
        }
      }
      if (!found) {
        break;
      }
    } else {
      const auto found = in.find(',');
      if (found == in.npos) {
        break;
      }
      returning[on] = in.substr(0, found);
      ++on;
      if (on == returning.size()) {
        return returning;
      }
      in = in.substr(found + 1);
    }
  }
  if (in.size() > 0 && in[0] == '"') {
    if (in.size() > 1 && in.back() == '"') {
      returning[on] = in.substr(1, in.size() - 2);
      ++on;
    } else {
      throw std::exception("Couldn't find quote termiantor");
    }
  } else if (in.size() > 0 && in[0] == '[') {
    if (in.size() > 1 && in.back() == ']') {
      returning[on] = in.substr(1, in.size() - 2);
      ++on;
    } else {
      throw std::exception("Couldn't find array terminator");
    }
  } else if (in.size() > 0 && in[0] == '(') {
    if (in.size() > 1 && in.back() == ')') {
      returning[on] = in.substr(1, in.size() - 2);
      ++on;
    } else {
      throw std::exception("Couldn't find tuple terminator");
    }
  } else {
    returning[on] = in;
    ++on;
  }

  return returning.subspan(0, on);
}

std::optional<clogparser::internal::Partial_parse> clogparser::internal::parse_line(Parse_state& state, std::string_view in) {
  const auto end_timestamp = in.find("  ");
  if (end_timestamp == std::string_view::npos) {
    fprintf(stdout, "Couldn't find timestamp in line: %.*s\n", (std::uint32_t)in.size(), in.data());
    return std::nullopt;
  }
  const std::string_view timestamp_str = in.substr(0, end_timestamp);
  in = in.substr(end_timestamp + 2);

  const auto end_type = in.find(',');
  if (end_type == std::string_view::npos) {
    fprintf(stdout, "Couldn't find type in line: %.*s\n", (std::uint32_t)in.size(), in.data());
    return std::nullopt;
  }
  const std::string_view type_str = in.substr(0, end_type);
  in = in.substr(end_type + 1);

  const std::optional<Timestamp> parsed_time = parse_timestamp(timestamp_str);
  if (!parsed_time) {
    return std::nullopt;
  }

  return Partial_parse{ *parsed_time, type_str, in };
}

void clogparser::internal::parseInt(bool& returning, std::string_view in) {
  if (in == "nil") {
    returning = false;
  } else {
    std::uint32_t checking = 0;
    parseInt(checking, in);
    returning = (checking != 0);
  }
}

events::Unit clogparser::internal::Parse<events::Unit>::parse(Columns columns) {
  if (columns.size() < events::Unit::COLUMNS_COUNT) {
    throw std::exception("Not enough columns for a unit");
  }

  return events::Unit{
    columns[0],
    columns[1],
    Unit_flags(parseInt<std::uint32_t>(columns[2])),
    Raid_flags(parseInt<std::uint32_t>(columns[3]))
  };
}
events::Combat_header clogparser::internal::Parse<events::Combat_header>::parse(Columns columns) {
  if (columns.size() < events::Combat_header::COLUMNS_COUNT) {
    throw std::exception("Not enough columns for a combat header");
  }

  return events::Combat_header{
    Parse<events::Unit>::parse(columns.subspan(0, 4)),
    Parse<events::Unit>::parse(columns.subspan(4, 4))
  };
}
events::Spell_info clogparser::internal::Parse<events::Spell_info>::parse(Columns columns) {
  if (columns.size() < events::Spell_info::COLUMNS_COUNT) {
    throw std::exception("Not enough columns for a spell info");
  }

  return events::Spell_info{
    parseInt<std::uint64_t>(columns[0]),
    columns[1],
    Spell_schools(parseInt<std::uint8_t>(columns[2]))
  };
}
events::Advanced_info clogparser::internal::Parse<events::Advanced_info>::parse(Columns columns) {
  if (columns.size() < events::Advanced_info::COLUMNS_COUNT) {
    throw std::exception("Not enough columns for advanced info");
  }

  return events::Advanced_info{
    columns[0],
    columns[1],
    parseInt<std::uint64_t>(columns[2]),
    parseInt<std::uint64_t>(columns[3]),
    parseInt<std::int64_t>(columns[4]),
    parseInt<std::int64_t>(columns[5]),
    parseInt<std::int64_t>(columns[6]),
    parseInt<std::int64_t>(columns[7]),
    Power_types::alternate_power, //NYI, need to handle multiple values
    0, //NYI, need to handle multiple values
    0, //NYI, need to handle multiple values
    0, //NYI, need to handle multiple values
    parseInt<float>(columns[12]),
    parseInt<float>(columns[13]),
    parseInt<std::uint64_t>(columns[14]),
    parseInt<float>(columns[15]),
    parseInt<std::uint16_t>(columns[16])
  };
}
events::Damage clogparser::internal::Parse<events::Damage>::parse(Columns columns) {
  if (columns.size() < events::Damage::COLUMNS_COUNT) {
    throw std::exception{"not enough columns for damage"};
  }

  return events::Damage{
    parseInt<std::int64_t>(columns[0]),
    parseInt<std::int64_t>(columns[1]),
    parseInt<std::int64_t>(columns[2]),
    Spell_schools(parseInt<std::uint8_t>(columns[3])),
    parseInt<std::int64_t>(columns[4]),
    parseInt<std::uint64_t>(columns[5]),
    parseInt<std::int32_t>(columns[6]),
    parseInt<bool>(columns[7]),
    parseInt<bool>(columns[8]),
    parseInt<bool>(columns[9])
  };
}
events::Heal clogparser::internal::Parse<events::Heal>::parse(Columns columns) {
  if (columns.size() < events::Heal::COLUMNS_COUNT) {
    throw std::exception{"Not enough columns for healing"};
  }

  return events::Heal{
    parseInt<std::uint64_t>(columns[0]),
    parseInt<std::uint64_t>(columns[1]),
    parseInt<std::uint64_t>(columns[2]),
    parseInt<std::uint64_t>(columns[3]),
    parseInt<bool>(columns[4])
  };
}
events::Combat_log_version clogparser::internal::Parse<events::Combat_log_version>::parse(Columns columns) {
  if (columns.size() < events::Combat_log_version::COLUMNS_COUNT) {
    throw std::exception{"Not enough columns for a combat log version"};
  }

  if (columns[1] != "ADVANCED_LOG_ENABLED"
    || columns[3] != "BUILD_VERSION"
    || columns[5] != "PROJECT_ID") {
    throw std::exception{"Unexpected variable names in combat log version"};
  }

  auto version = columns[4];
  const auto found_expac = version.find('.');
  if (found_expac == std::string_view::npos) {
    throw std::exception("Couldn't find delimiter between expac and patch in combat log version");
  }
  const auto expac = parseInt<std::uint8_t>(version.substr(0, found_expac));
  version = version.substr(found_expac + 1);
  const auto found_major = version.find('.');
  if (found_major == std::string_view::npos) {
    throw std::exception("Couldn't find delimiter between patch and minor in combat log version");
  }
  const auto patch = parseInt<std::uint8_t>(version.substr(0, found_major));
  const auto minor = parseInt<std::uint8_t>(version.substr(found_major + 1));

  return {
    parseInt<std::uint8_t>(columns[0]),
    parseInt<bool>(columns[2]),
    {
      expac,
      patch,
      minor,
    },
    parseInt<std::uint8_t>(columns[6])
  };
}
events::Spell_aura_applied clogparser::internal::Parse<events::Spell_aura_applied>::parse(Columns columns) {
  if (columns.size() == 12) {
    return {
      Parse<events::Combat_header>::parse(columns.subspan(0,8)),
      Parse<events::Spell_info>::parse(columns.subspan(8,3)),
      columns[11],
      std::nullopt
    };
  } else if (columns.size() > 12) {
    return {
      Parse<events::Combat_header>::parse(columns.subspan(0,8)),
      Parse<events::Spell_info>::parse(columns.subspan(8,3)),
      columns[11],
      parseInt<std::uint64_t>(columns[12])
    };
  } else {
    throw std::exception{"Not enough columns for spell aura applied"};
  }
}
events::Spell_aura_refresh clogparser::internal::Parse<events::Spell_aura_refresh>::parse(Columns columns) {
  if (columns.size() == 12) {
    return {
      Parse<events::Combat_header>::parse(columns.subspan(0,8)),
      Parse<events::Spell_info>::parse(columns.subspan(8,3)),
      columns[11],
      std::nullopt
    };
  } else if (columns.size() > 12) {
    return {
      Parse<events::Combat_header>::parse(columns.subspan(0,8)),
      Parse<events::Spell_info>::parse(columns.subspan(8,3)),
      columns[11],
      parseInt<std::uint64_t>(columns[12])
    };
  } else {
    throw std::exception{"Not enough columns for spell aura applied"};
  }
}
events::Spell_aura_removed clogparser::internal::Parse<events::Spell_aura_removed>::parse(Columns columns) {
  if (columns.size() == 12) {
    return {
      Parse<events::Combat_header>::parse(columns.subspan(0,8)),
      Parse<events::Spell_info>::parse(columns.subspan(8,3)),
      columns[11],
      std::nullopt
    };
  } else if (columns.size() > 12) {
    return {
      Parse<events::Combat_header>::parse(columns.subspan(0,8)),
      Parse<events::Spell_info>::parse(columns.subspan(8,3)),
      columns[11],
      parseInt<std::uint64_t>(columns[12])
    };
  } else {
    throw std::exception{"Not enough columns for spell aura applied"};
  }
}
events::Spell_periodic_damage clogparser::internal::Parse<events::Spell_periodic_damage>::parse(Columns columns) {
  if (columns.size() < events::Spell_periodic_damage::COLUMNS_COUNT) {
    throw std::exception{"Not enough columns for a spell periodic damage"};
  }

  return {
    Parse<events::Combat_header>::parse(columns.subspan(0,8)),
    Parse<events::Spell_info>::parse(columns.subspan(8,3)),
    Parse<events::Advanced_info>::parse(columns.subspan(11, 17)),
    Parse<events::Damage>::parse(columns.subspan(28, 10))
  };
}
events::Spell_periodic_damage_support clogparser::internal::Parse<events::Spell_periodic_damage_support>::parse(Columns columns) {
  if (columns.size() < events::Spell_periodic_damage_support::COLUMNS_COUNT) {
    throw std::exception{ "Not enough columns for a spell periodic damage support" };
  }

  return {
    Parse<events::Combat_header>::parse(columns.subspan(0, 8)),
    Parse<events::Spell_info>::parse(columns.subspan(8,3)),
    Parse<events::Advanced_info>::parse(columns.subspan(11,17)),
    Parse<events::Damage>::parse(columns.subspan(28,10)),
    columns[38]
  };
}
events::Spell_periodic_missed clogparser::internal::Parse<events::Spell_periodic_missed>::parse(Columns columns) {
  if (columns.size() < 12) {
    throw std::exception("Not enough columns for spell missed");
  }

  const std::string_view type = columns[11];

  if (type == "ABSORB") {
    return {
      Parse<events::Combat_header>::parse(columns.subspan(0, 8)), //combat header
      Parse<events::Spell_info>::parse(columns.subspan(8, 3)),
      type,
      parseInt<bool>(columns[12]),
      parseInt<std::uint64_t>(columns[13]),
      parseInt<std::uint64_t>(columns[14])
    };
  } else {
    return {
      Parse<events::Combat_header>::parse(columns.subspan(0, 8)), //combat header
      Parse<events::Spell_info>::parse(columns.subspan(8, 3)),
      type,
      parseInt<bool>(columns[12]),
      0,
      0
    };
  }
}
events::Spell_periodic_heal clogparser::internal::Parse<events::Spell_periodic_heal>::parse(Columns columns) {
  if (columns.size() < events::Spell_periodic_heal::COLUMNS_COUNT) {
    throw std::exception{"Not enough columns for a spell periodic heal"};
  }

  return {
    Parse<events::Combat_header>::parse(columns.subspan(0,8)),
    Parse<events::Spell_info>::parse(columns.subspan(8,3)),
    Parse<events::Advanced_info>::parse(columns.subspan(11, 17)),
    Parse<events::Heal>::parse(columns.subspan(28, 5))
  };
}
events::Spell_absorbed clogparser::internal::Parse<events::Spell_absorbed>::parse(Columns columns) {
  if (columns.size() == 18) {
    return {
      Parse<events::Combat_header>::parse(columns.subspan(0, 8)),
      std::nullopt,
      Parse<events::Unit>::parse(columns.subspan(8, 4)),
      Parse<events::Spell_info>::parse(columns.subspan(12, 3)),
      parseInt<std::int64_t>(columns[15]),
      parseInt<std::uint64_t>(columns[16]),
      parseInt<bool>(columns[17])
    };
  } else if (columns.size() == 21) {
    return {
      Parse<events::Combat_header>::parse(columns.subspan(0, 8)),
      Parse<events::Spell_info>::parse(columns.subspan(8, 3)),
      Parse<events::Unit>::parse(columns.subspan(11, 4)),
      Parse<events::Spell_info>::parse(columns.subspan(15, 3)),
      parseInt<std::int64_t>(columns[18]),
      parseInt<std::uint64_t>(columns[19]),
      parseInt<bool>(columns[20])
    };
  } else {
    throw std::exception{"Not enough columns for a spell absorbed"};
  }
}
events::Swing_missed clogparser::internal::Parse<events::Swing_missed>::parse(Columns columns) {
  if (columns.size() < 9) {
    throw std::exception("Not enough columns for swing misses");
  }

  const std::string_view type = columns[8];

  if (type == "ABSORB") {
    return {
      Parse<events::Combat_header>::parse(columns.subspan(0, 8)),
      type,
      parseInt<bool>(columns[9]),
      parseInt<std::uint64_t>(columns[10]),
      parseInt<std::uint64_t>(columns[11]),
      parseInt<bool>(columns[12])
    };
  } else {
    return {
      Parse<events::Combat_header>::parse(columns.subspan(0, 8)),
      type,
      parseInt<bool>(columns[9]),
      0,
      0,
      false
    };
  }
}
events::Swing_damage clogparser::internal::Parse<events::Swing_damage>::parse(Columns columns) {
  if (columns.size() < events::Swing_damage::COLUMNS_COUNT) {
    throw std::exception{ "Not enough columns for a swing damage" };
  }

  return {
    Parse<events::Combat_header>::parse(columns.subspan(0, 8)), //combat header
    Parse<events::Advanced_info>::parse(columns.subspan(8, 17)),
    Parse<events::Damage>::parse(columns.subspan(25, 10))
  };
}
events::Swing_damage_landed clogparser::internal::Parse<events::Swing_damage_landed>::parse(Columns columns) {
  if (columns.size() < events::Swing_damage_landed::COLUMNS_COUNT) {
    throw std::exception{"Not enough columns for a swing damage landed"};
  }

  return {
    Parse<events::Combat_header>::parse(columns.subspan(0, 8)), //combat header
    Parse<events::Advanced_info>::parse(columns.subspan(8, 17)),
    Parse<events::Damage>::parse(columns.subspan(25, 10))
  };
}
events::Swing_damage_landed_support clogparser::internal::Parse<events::Swing_damage_landed_support>::parse(Columns columns) {
  if (columns.size() < events::Swing_damage_landed_support::COLUMNS_COUNT) {
    throw std::exception{ "Not enough columns for a swing damage landed support" };
  }

  return {
    Parse<events::Combat_header>::parse(columns.subspan(0, 8)),
    Parse<events::Spell_info>::parse(columns.subspan(8,3)),
    Parse<events::Advanced_info>::parse(columns.subspan(11,17)),
    Parse<events::Damage>::parse(columns.subspan(28,10)),
    columns[38]
  };
}
events::Spell_missed clogparser::internal::Parse<events::Spell_missed>::parse(Columns columns) {
  if (columns.size() < 12) {
    throw std::exception("Not enough columns for spell missed");
  }

  const std::string_view type = columns[11];

  if (type == "ABSORB") {
    return {
      Parse<events::Combat_header>::parse(columns.subspan(0, 8)), //combat header
      Parse<events::Spell_info>::parse(columns.subspan(8, 3)),
      type,
      parseInt<bool>(columns[12]),
      parseInt<std::uint64_t>(columns[13]),
      parseInt<std::uint64_t>(columns[14])
    };
  } else {
    return {
      Parse<events::Combat_header>::parse(columns.subspan(0, 8)), //combat header
      Parse<events::Spell_info>::parse(columns.subspan(8, 3)),
      type,
      parseInt<bool>(columns[12]),
      0,
      0
    };
  }
}
events::Spell_damage clogparser::internal::Parse<events::Spell_damage>::parse(Columns columns) {
  if (columns.size() < events::Spell_damage::COLUMNS_COUNT) {
    throw std::exception{"Not enough columns for a spell periodic damage"};
  }

  return {
    Parse<events::Combat_header>::parse(columns.subspan(0, 8)), //combat header
    Parse<events::Spell_info>::parse(columns.subspan(8, 3)),
    Parse<events::Advanced_info>::parse(columns.subspan(11, 17)),
    Parse<events::Damage>::parse(columns.subspan(28, 10))
  };
}
events::Spell_damage_support clogparser::internal::Parse<events::Spell_damage_support>::parse(Columns columns) {
  if (columns.size() < events::Spell_damage_support::COLUMNS_COUNT) {
    throw std::exception{ "Not enough columns for a spell damage support" };
  }

  return {
    Parse<events::Combat_header>::parse(columns.subspan(0, 8)),
    Parse<events::Spell_info>::parse(columns.subspan(8,3)),
    Parse<events::Advanced_info>::parse(columns.subspan(11,17)),
    Parse<events::Damage>::parse(columns.subspan(28,10)),
    columns[38]
  };
}
events::Spell_heal clogparser::internal::Parse<events::Spell_heal>::parse(Columns columns) {
  if (columns.size() < events::Spell_heal::COLUMNS_COUNT) {
    throw std::exception{"Not enough columns for a spell heal"};
  }
  return {
    Parse<events::Combat_header>::parse(columns.subspan(0, 8)),
    Parse<events::Spell_info>::parse(columns.subspan(8, 3)),
    Parse<events::Advanced_info>::parse(columns.subspan(11, 17)),
    Parse<events::Heal>::parse(columns.subspan(28, 5))
  };
}
events::Spell_cast_success clogparser::internal::Parse<events::Spell_cast_success>::parse(Columns columns) {
  if (columns.size() < events::Spell_cast_success::COLUMNS_COUNT) {
    throw std::exception{"Not enough columns for a spell cast success"};
  }
  return {
    Parse<events::Combat_header>::parse(columns.subspan(0, 8)),
    Parse<events::Spell_info>::parse(columns.subspan(8, 3)),
    Parse<events::Advanced_info>::parse(columns.subspan(11, 17))
  };
}
events::Encounter_start clogparser::internal::Parse<events::Encounter_start>::parse(Columns columns) {
  if (columns.size() < events::Encounter_start::COLUMNS_COUNT) {
    throw std::exception("Not enough columns for an encounter start");
  }
  return {
    parseInt<std::int32_t>(columns[0]),
    columns[1],
    static_cast<Difficulty>(parseInt<std::uint8_t>(columns[2])),
    parseInt<std::uint8_t>(columns[3]),
    parseInt<std::uint64_t>(columns[4])
  };
}
events::Encounter_end clogparser::internal::Parse<events::Encounter_end>::parse(Columns columns) {
  if (columns.size() < events::Encounter_end::COLUMNS_COUNT) {
    throw std::exception("Not enough columns for an encounter end");
  }
  return {
    parseInt<std::int32_t>(columns[0]),
    columns[1],
    static_cast<Difficulty>(parseInt<std::uint8_t>(columns[2])),
    parseInt<std::uint8_t>(columns[3]),
    parseInt<bool>(columns[4]),
    parseInt<std::uint64_t>(columns[5])
  };
}
events::Combatant_info clogparser::internal::Parse<events::Combatant_info>::parse(Columns columns) {
  if (columns.size() < events::Combatant_info::COLUMNS_COUNT) {
    throw std::exception{"Not enough columns for a combatant info"};
  }
  
  return {
    columns[0],
    FactionId{ parseInt<std::underlying_type_t<FactionId>>(columns[1])},
    Stats{
      parseInt<std::int32_t>(columns[2]),
      parseInt<std::int32_t>(columns[3]),
      parseInt<std::int32_t>(columns[4]),
      parseInt<std::int32_t>(columns[5]),
      parseInt<std::int32_t>(columns[6]),
      parseInt<std::int32_t>(columns[7]),
      parseInt<std::int32_t>(columns[8]),
      parseInt<std::int32_t>(columns[9]),
      parseInt<std::int32_t>(columns[10]),
      parseInt<std::int32_t>(columns[11]),
      parseInt<std::int32_t>(columns[12]),
      parseInt<std::int32_t>(columns[13]),
      parseInt<std::int32_t>(columns[14]),
      parseInt<std::int32_t>(columns[15]),
      parseInt<std::int32_t>(columns[16]),
      parseInt<std::int32_t>(columns[17]),
      parseInt<std::int32_t>(columns[18]),
      parseInt<std::int32_t>(columns[19]),
      parseInt<std::int32_t>(columns[20]),
      parseInt<std::int32_t>(columns[21]),
      parseInt<std::int32_t>(columns[22]),
    },
    SpecId{ parseInt<std::underlying_type_t<SpecId>>(columns[23])},
    parse_talents(columns[24]),
    ::parse_array(columns[25]),
    parse_items(columns[26]),
    ::parse_array(columns[27]),
    parseInt<std::uint32_t>(columns[28]),
    parseInt<std::uint32_t>(columns[29]),
    parseInt<std::uint32_t>(columns[30]),
    parseInt<std::uint32_t>(columns[31])
  };
}
events::Spell_summon clogparser::internal::Parse<events::Spell_summon>::parse(Columns columns) {
  if (columns.size() < events::Spell_summon::COLUMNS_COUNT) {
    throw std::exception{ "Not enough columns for a spell summon" };
  }

  return {
    Parse<events::Unit>::parse(columns.subspan(0,4)),
    Parse<events::Unit>::parse(columns.subspan(4,4)),
    Parse<events::Spell_info>::parse(columns.subspan(8,3))
  };
}
events::Zone_change clogparser::internal::Parse<events::Zone_change>::parse(Columns columns) {
  if (columns.size() < events::Zone_change::COLUMNS_COUNT) {
    throw std::exception("Not enough columns for a zone change");
  }
  return {
    parseInt<std::uint64_t>(columns[0]),
    columns[1],
    parseInt<std::uint64_t>(columns[2])
  };
}
events::Map_change clogparser::internal::Parse<events::Map_change>::parse(Columns columns) {
  if (columns.size() < events::Map_change::COLUMNS_COUNT) {
    throw std::exception("Not enough columns for a map change");
  }
  return {
    parseInt<std::uint64_t>(columns[0]),
      columns[1],
      parseInt<float>(columns[2]),
      parseInt<float>(columns[3]),
      parseInt<float>(columns[4]),
      parseInt<float>(columns[5])
  };
}

std::string_view clogparser::String_store::get(std::string_view in) noexcept {
  return *store_.emplace(in).first;
}

events::Combat_log_version clogparser::String_store::get(events::Combat_log_version in) {
  return in;
}
events::Spell_aura_applied clogparser::String_store::get(events::Spell_aura_applied in) {
  return {
    ::convert(*this, in.combat_header),
    ::convert(*this, in.spell),
    get(in.aura_type),
    in.remaining_points
  };
}
events::Spell_aura_refresh clogparser::String_store::get(events::Spell_aura_refresh in) {
  return {
    ::convert(*this, in.combat_header),
    ::convert(*this, in.spell),
    get(in.aura_type),
    in.remaining_points
  };
}
events::Spell_aura_removed clogparser::String_store::get(events::Spell_aura_removed in) {
  return {
    ::convert(*this, in.combat_header),
    ::convert(*this, in.spell),
    get(in.aura_type),
    in.remaining_points
  };
}
events::Spell_periodic_damage clogparser::String_store::get(events::Spell_periodic_damage in) {
  return {
    ::convert(*this, in.combat_header),
    ::convert(*this, in.spell),
    ::convert(*this, in.advanced),
    ::convert(*this, in.damage)
  };
}
events::Spell_periodic_missed clogparser::String_store::get(events::Spell_periodic_missed in) {
  return {
    ::convert(*this, in.combat_header),
    ::convert(*this, in.spell),
    get(in.type),
    in.unk1,
    in.final,
    in.initial,
    in.unk2
  };
}
events::Spell_periodic_heal clogparser::String_store::get(events::Spell_periodic_heal in) {
  return {
    ::convert(*this, in.combat_header),
    ::convert(*this, in.spell),
    ::convert(*this, in.advanced),
    ::convert(*this, in.heal)
  };
}
events::Spell_absorbed clogparser::String_store::get(events::Spell_absorbed in) {
  if (in.dmg_spell.has_value()) {
    return {
      ::convert(*this, in.combat_header),
      ::convert(*this, in.dmg_spell.value()),
      ::convert(*this, in.absorber),
      ::convert(*this, in.absorber_spell),
      in.absorbed,
      in.unmitigated,
      in.critical
    };
  } else {
    return {
      ::convert(*this, in.combat_header),
      std::nullopt,
      ::convert(*this, in.absorber),
      ::convert(*this, in.absorber_spell),
      in.absorbed,
      in.unmitigated,
      in.critical
    };
  }
}
events::Swing_missed clogparser::String_store::get(events::Swing_missed in) {
  return {
    ::convert(*this, in.combat_header),
    get(in.type),
    in.unk1,
    in.final,
    in.initial,
    in.unk2
  };
}
events::Swing_damage_landed clogparser::String_store::get(events::Swing_damage_landed in) {
  return {
    ::convert(*this, in.combat_header),
    ::convert(*this, in.advanced),
    ::convert(*this, in.damage)
  };
}
events::Spell_missed clogparser::String_store::get(events::Spell_missed in) {
  return {
    ::convert(*this, in.combat_header),
    ::convert(*this, in.spell),
    get(in.type),
    in.offhand,
    in.final,
    in.initial
  };
}
events::Spell_damage clogparser::String_store::get(events::Spell_damage in) {
  return {
    ::convert(*this, in.combat_header),
    ::convert(*this, in.spell),
    ::convert(*this, in.advanced),
    ::convert(*this, in.damage)
  };
}
events::Spell_heal clogparser::String_store::get(events::Spell_heal in) {
  return {
    ::convert(*this, in.combat_header),
    ::convert(*this, in.spell),
    ::convert(*this, in.advanced),
    ::convert(*this, in.heal)
  };
}
events::Spell_cast_success clogparser::String_store::get(events::Spell_cast_success in) {
  return {
    ::convert(*this, in.combat_header),
    ::convert(*this, in.spell),
    ::convert(*this, in.advanced)
  };
}
events::Encounter_start clogparser::String_store::get(events::Encounter_start in) {
  return {
    in.encounter_id,
    get(in.encounter_name),
    in.difficulty_id,
    in.instance_size,
    in.instance_id
  };
}
events::Encounter_end clogparser::String_store::get(events::Encounter_end in) {
  return {
    in.encounter_id,
    get(in.encounter_name),
    in.difficulty_id,
    in.instance_size,
    in.success,
    in.instance_id
  };
}
events::Combatant_info clogparser::String_store::get(events::Combatant_info in) {
  return {
    get(in.guid),
    in.faction,
    in.stats,
    in.current_spec_id,
    in.talents,
    get(in.pvp_talents),
    in.items,
    get(in.interesting_auras),
    in.honor_level,
    in.season,
    in.rating,
    in.tier
  };
}
events::Spell_summon clogparser::String_store::get(events::Spell_summon in) {
  return {
    ::convert(*this, in.summoner),
    ::convert(*this, in.summoned),
    ::convert(*this, in.spell)
  };
}
events::Zone_change clogparser::String_store::get(events::Zone_change in) {
  return {
    in.instance_id,
    get(in.zone_name),
    in.difficulty_id
  };
}
events::Map_change clogparser::String_store::get(events::Map_change in) {
  return {
    in.map_id,
    get(in.map_name),
    in.x_min,
    in.x_max,
    in.y_min,
    in.y_max
  };
}

void clogparser::String_store::clear() {
  store_.clear();
}
