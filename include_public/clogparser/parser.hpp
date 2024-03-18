#pragma once

#include <span>
#include <vector>
#include <variant>
#include <string_view>
#include <unordered_set>
#include <chrono>
#include <optional>
#include <charconv>
#include <unordered_map>

#include <clogparser/types.hpp>
#include <clogparser/item.hpp>

namespace clogparser {
  using Period = std::chrono::milliseconds;

  struct Timestamp {
    std::uint8_t month;
    std::uint8_t day;
    std::uint8_t hour;
    std::uint8_t minute;
    std::uint8_t second;
    std::uint16_t millisecond;

    //assumes both timestamps are +- 24 hours
    Period operator-(Timestamp const& other) const noexcept;
  };

  constexpr bool is_invalid_guid(std::string_view in) {
    return in.empty() || in[0] == '0';
  }

  namespace events {
    //shared
    struct Unit {
      static constexpr std::size_t COLUMNS_COUNT = 4;
      std::string_view guid;
      std::string_view name;
      Unit_flags flags;
      Raid_flags raid_flags;
    };

    struct Combat_header {
      static constexpr std::size_t COLUMNS_COUNT = 2* Unit::COLUMNS_COUNT;
      Unit source;
      Unit dest;
    };

    struct Spell_info {
      static constexpr std::size_t COLUMNS_COUNT = 3;
      std::uint64_t id;
      std::string_view name;
      Spell_schools school;
    };

    struct Advanced_info {
      static constexpr std::size_t COLUMNS_COUNT = 17;
      std::string_view advanced_unit_guid;
      std::string_view owner_guid;
      std::uint64_t current_hp;
      std::uint64_t max_hp;
      std::int64_t attack_power;
      std::int64_t spell_power;
      std::int64_t armor;
      std::int64_t absorb;
      Power_types power_type;
      std::uint64_t current_power;
      std::uint64_t max_power;
      std::uint64_t power_cost;
      float position_x;
      float position_y;
      std::uint64_t map_id;
      float facing;
      std::uint16_t level;
    };

    struct Damage {
      static constexpr std::size_t COLUMNS_COUNT = 10;
      std::int64_t final;
      std::int64_t initial;
      std::int64_t overkill;
      Spell_schools school;
      std::int64_t resisted;
      std::uint64_t blocked;
      std::int64_t absorbed;
      bool crit;
      bool glancing;
      bool crushing;
    };

    struct Heal {
      static constexpr std::size_t COLUMNS_COUNT = 5;
      std::uint64_t final;
      std::uint64_t initial;
      std::uint64_t overhealing;
      std::uint64_t absorbed;
      bool crit;
    };

    struct Combat_log_version {
      static constexpr std::string_view NAME = "COMBAT_LOG_VERSION";
      static constexpr std::size_t COLUMNS_COUNT = 7;
      std::uint8_t version;
      bool advanced_log_enabled;
      struct Build_version {
        std::uint8_t expac;
        std::uint8_t patch;
        std::uint8_t minor;

        constexpr std::strong_ordering operator<=>(Build_version const&) const noexcept = default;
      } build_version;
      std::uint8_t project_id;
    };

    struct Spell_aura_applied {
      static constexpr std::string_view NAME = "SPELL_AURA_APPLIED";
      static constexpr std::size_t COLUMNS_COUNT = Combat_header::COLUMNS_COUNT + Spell_info::COLUMNS_COUNT + 2;
      Combat_header combat_header;
      Spell_info spell;

      Aura_type aura_type;
      std::optional<std::uint64_t> remaining_points;
    };
    struct Spell_aura_applied_dose {
      static constexpr std::string_view NAME = "SPELL_AURA_APPLIED_DOSE";
      static constexpr std::size_t COLUMNS_COUNT = Combat_header::COLUMNS_COUNT + Spell_info::COLUMNS_COUNT + 2;
      Combat_header combat_header;
      Spell_info spell;

      Aura_type aura_type;
      std::uint8_t new_dosage;
    };
    struct Spell_aura_refresh {
      static constexpr std::string_view NAME = "SPELL_AURA_REFRESH";
      static constexpr std::size_t COLUMNS_COUNT = Combat_header::COLUMNS_COUNT + Spell_info::COLUMNS_COUNT + 2;
      Combat_header combat_header;
      Spell_info spell;

      Aura_type aura_type;
      std::optional<std::uint64_t> remaining_points;
    };
    struct Spell_aura_removed {
      static constexpr std::string_view NAME = "SPELL_AURA_REMOVED";
      static constexpr std::size_t COLUMNS_COUNT = Combat_header::COLUMNS_COUNT + Spell_info::COLUMNS_COUNT + 2;
      Combat_header combat_header;
      Spell_info spell;

      std::string_view aura_type;
      std::optional<std::uint64_t> remaining_points;
    };
    struct Spell_aura_removed_dose {
      static constexpr std::string_view NAME = "SPELL_AURA_REMOVED_DOSE";
      static constexpr std::size_t COLUMNS_COUNT = Combat_header::COLUMNS_COUNT + Spell_info::COLUMNS_COUNT + 2;
      Combat_header combat_header;
      Spell_info spell;

      Aura_type aura_type;
      std::uint8_t new_dosage;
    };
    struct Spell_periodic_damage {
      static constexpr std::string_view NAME = "SPELL_PERIODIC_DAMAGE";
      static constexpr std::size_t COLUMNS_COUNT = 
        Combat_header::COLUMNS_COUNT 
        + Spell_info::COLUMNS_COUNT 
        + Advanced_info::COLUMNS_COUNT 
        + Damage::COLUMNS_COUNT;

      Combat_header combat_header;
      Spell_info spell;

      Advanced_info advanced;
      Damage damage;
    };
    struct Spell_periodic_damage_support {
      static constexpr std::string_view NAME = "SPELL_PERIODIC_DAMAGE_SUPPORT";
      static constexpr std::size_t COLUMNS_COUNT = 
        Combat_header::COLUMNS_COUNT 
        + Spell_info::COLUMNS_COUNT 
        + Advanced_info::COLUMNS_COUNT 
        + Damage::COLUMNS_COUNT 
        + 1;

      Combat_header combat_header;
      Spell_info spell;

      Advanced_info advanced;
      Damage damage;
      std::string_view supporter;
    };
    struct Spell_periodic_missed {
      static constexpr std::string_view NAME = "SPELL_PERIODIC_MISSED";
      static constexpr std::size_t COLUMNS_COUNT = Combat_header::COLUMNS_COUNT + Spell_info::COLUMNS_COUNT + 5;
      Combat_header combat_header;
      Spell_info spell;
      std::string_view type;
      bool unk1;
      std::uint64_t final;
      std::uint64_t initial;
      bool unk2;
    };
    struct Spell_periodic_heal {
      static constexpr std::string_view NAME = "SPELL_PERIODIC_HEAL";
      static constexpr std::size_t COLUMNS_COUNT = Combat_header::COLUMNS_COUNT + Spell_info::COLUMNS_COUNT + Advanced_info::COLUMNS_COUNT + Heal::COLUMNS_COUNT;
      Combat_header combat_header;
      Spell_info spell;
      Advanced_info advanced;
      Heal heal;
    };

    struct Spell_absorbed {
      static constexpr std::string_view NAME = "SPELL_ABSORBED";
      static constexpr std::size_t COLUMNS_COUNT = Combat_header::COLUMNS_COUNT + Spell_info::COLUMNS_COUNT + Unit::COLUMNS_COUNT + Spell_info::COLUMNS_COUNT + 3;
      Combat_header combat_header;

      std::optional<Spell_info> dmg_spell;

      Unit absorber;

      Spell_info absorber_spell;

      std::int64_t absorbed;
      std::uint64_t unmitigated;
      bool critical;
    };

    struct Spell_heal_absorbed {
      static constexpr std::string_view NAME = "SPELL_HEAL_ABSORBED";
      static constexpr std::size_t COLUMNS_COUNT = Combat_header::COLUMNS_COUNT + Spell_info::COLUMNS_COUNT + Unit::COLUMNS_COUNT + Spell_info::COLUMNS_COUNT + 2;
      Combat_header combat_header;
      Spell_info absorbing_spell;
      Unit absorbed;
      Spell_info absorbed_spell;
      std::int64_t absorbed_amount;
      std::uint64_t unmitigated;
    };

    struct Swing_damage {
      static constexpr std::string_view NAME = "SWING_DAMAGE";
      static constexpr std::size_t COLUMNS_COUNT = Combat_header::COLUMNS_COUNT + Advanced_info::COLUMNS_COUNT + Damage::COLUMNS_COUNT;
      Combat_header combat_header;
      Advanced_info advanced;
      Damage damage;
    };

    struct Swing_missed {
      static constexpr std::string_view NAME = "SWING_MISSED";
      static constexpr std::size_t COLUMNS_COUNT = Combat_header::COLUMNS_COUNT + 5;
      Combat_header combat_header;
      std::string_view type;
      bool unk1;
      std::uint64_t final;
      std::uint64_t initial;
      bool unk2;
    };

    struct Swing_damage_landed {
      static constexpr std::string_view NAME = "SWING_DAMAGE_LANDED";
      static constexpr std::size_t COLUMNS_COUNT = Combat_header::COLUMNS_COUNT + Advanced_info::COLUMNS_COUNT + Damage::COLUMNS_COUNT;
      Combat_header combat_header;
      Advanced_info advanced;
      Damage damage;
    };

    struct Swing_damage_landed_support {
      static constexpr std::string_view NAME = "SWING_DAMAGE_LANDED_SUPPORT";
      static constexpr std::size_t COLUMNS_COUNT =
        Combat_header::COLUMNS_COUNT
        + Spell_info::COLUMNS_COUNT
        + Advanced_info::COLUMNS_COUNT
        + Damage::COLUMNS_COUNT
        + 1;

      Combat_header combat_header;
      Spell_info spell;

      Advanced_info advanced;
      Damage damage;
      std::string_view supporter;
    };

    struct Spell_missed {
      static constexpr std::string_view NAME = "SPELL_MISSED";
      static constexpr std::size_t COLUMNS_COUNT = Combat_header::COLUMNS_COUNT + Spell_info::COLUMNS_COUNT + 4;
      Combat_header combat_header;
      Spell_info spell;
      std::string_view type;
      bool offhand;
      std::uint64_t final;
      std::uint64_t initial;
    };

    struct Spell_damage {
      static constexpr std::string_view NAME = "SPELL_DAMAGE";
      static constexpr std::size_t COLUMNS_COUNT = Combat_header::COLUMNS_COUNT + Spell_info::COLUMNS_COUNT + Advanced_info::COLUMNS_COUNT + Damage::COLUMNS_COUNT;
      Combat_header combat_header;
      Spell_info spell;
      Advanced_info advanced;
      Damage damage;
    };

    struct Spell_damage_support {
      static constexpr std::string_view NAME = "SPELL_DAMAGE_SUPPORT";
      static constexpr std::size_t COLUMNS_COUNT =
        Combat_header::COLUMNS_COUNT
        + Spell_info::COLUMNS_COUNT
        + Advanced_info::COLUMNS_COUNT
        + Damage::COLUMNS_COUNT
        + 1;

      Combat_header combat_header;
      Spell_info spell;

      Advanced_info advanced;
      Damage damage;
      std::string_view supporter;
    };

    struct Spell_heal {
      static constexpr std::string_view NAME = "SPELL_HEAL";
      static constexpr std::size_t COLUMNS_COUNT = Combat_header::COLUMNS_COUNT + Spell_info::COLUMNS_COUNT + Advanced_info::COLUMNS_COUNT + Heal::COLUMNS_COUNT;
      Combat_header combat_header;
      Spell_info spell;
      Advanced_info advanced;
      Heal heal;
    };

    struct Spell_cast_success {
      static constexpr std::string_view NAME = "SPELL_CAST_SUCCESS";
      static constexpr std::size_t COLUMNS_COUNT = Combat_header::COLUMNS_COUNT + Spell_info::COLUMNS_COUNT + Advanced_info::COLUMNS_COUNT;
      Combat_header combat_header;
      Spell_info spell;
      Advanced_info advanced;
    };

    struct Spell_summon {
      static constexpr std::string_view NAME = "SPELL_SUMMON";
      static constexpr std::size_t COLUMNS_COUNT =
        Unit::COLUMNS_COUNT
        + Unit::COLUMNS_COUNT
        + Spell_info::COLUMNS_COUNT;

      Unit summoner;
      Unit summoned;
      Spell_info spell;
    };

    struct Zone_change {
      static constexpr std::string_view NAME = "ZONE_CHANGE";
      static constexpr std::size_t COLUMNS_COUNT = 3;
      std::uint64_t instance_id;
      std::string_view zone_name;
      std::uint64_t difficulty_id;
    };

    struct Map_change {
      static constexpr std::string_view NAME = "MAP_CHANGE";
      static constexpr std::size_t COLUMNS_COUNT = 6;
      std::uint64_t map_id;
      std::string_view map_name;
      float x_min;
      float x_max;
      float y_min;
      float y_max;
    };

    struct Encounter_start {
      static constexpr std::string_view NAME = "ENCOUNTER_START";
      static constexpr std::size_t COLUMNS_COUNT = 5;
      std::int32_t encounter_id;
      std::string_view encounter_name;
      Difficulty difficulty_id;
      std::uint8_t instance_size;
      std::uint64_t instance_id;
    };

    struct Encounter_end {
      static constexpr std::string_view NAME = "ENCOUNTER_END";
      static constexpr std::size_t COLUMNS_COUNT = 6;
      std::int32_t encounter_id;
      std::string_view encounter_name;
      Difficulty difficulty_id;
      std::uint8_t instance_size;
      bool success;
      std::uint64_t instance_id;
    };

    struct Combatant_info {
      static constexpr std::string_view NAME = "COMBATANT_INFO";
      static constexpr std::size_t COLUMNS_COUNT = 2 + Stats::size + 9;
      std::string_view guid;
      FactionId faction;
      Stats stats;
      SpecId current_spec_id;
      struct Talent {
        std::uint32_t trait_node_entry_id;
        std::uint8_t rank;
      };
      std::vector<Talent> talents;
      std::vector<std::string_view> pvp_talents;
      std::vector<Item> items;
      struct Interesting_aura {
        std::string_view caster_guid;
        std::uint64_t spell_id;
      };
      std::vector<Interesting_aura> interesting_auras;
      //pvp
      std::uint32_t honor_level;
      std::uint32_t season;
      std::uint32_t rating;
      std::uint32_t tier;
    };

    struct Unit_died {
      static constexpr std::string_view NAME = "UNIT_DIED";
      static constexpr std::size_t COLUMNS_COUNT = Combat_header::COLUMNS_COUNT + 1;
      Combat_header combat_header;
      bool unconscious_on_death;
    };

    struct Spell_resurrect {
      static constexpr std::string_view NAME = "SPELL_RESURRECT";
      static constexpr std::size_t COLUMNS_COUNT = Combat_header::COLUMNS_COUNT + Spell_info::COLUMNS_COUNT;

      Combat_header combat_header;
      Spell_info spell;
    };

    using Type = std::variant<
      Combat_log_version,
      Spell_aura_applied,
      Spell_aura_applied_dose,
      Spell_aura_refresh,
      Spell_aura_removed,
      Spell_aura_removed_dose,
      Spell_periodic_damage,
      Spell_periodic_damage_support,
      Spell_periodic_missed,
      Spell_periodic_heal,
      Spell_absorbed,
      Spell_heal_absorbed,
      Swing_missed,
      Swing_damage,
      Swing_damage_landed,
      Swing_damage_landed_support,
      Spell_missed,
      Spell_damage,
      Spell_damage_support,
      Spell_heal,
      Spell_cast_success,
      Encounter_start,
      Encounter_end,
      Combatant_info,
      Spell_summon,
      Zone_change,
      Map_change,
      Unit_died,
      Spell_resurrect>;
  }

  namespace helpers {
    template<typename T>
    void parseInt(T& returning, std::string_view in) {
      std::from_chars_result res;
      if constexpr (std::is_integral_v<T>) {
        if (in.size() > 3 && (in.starts_with("0x") || in.starts_with("0X"))) {
          res = std::from_chars(in.data() + 2, in.data() + in.size(), returning, 16);
        } else {
          res = std::from_chars(in.data(), in.data() + in.size(), returning);
        }
      } else {
        res = std::from_chars(in.data(), in.data() + in.size(), returning);
      }
      if (res.ec != std::errc()) {
        throw std::exception("Couldn't parse string to int");
      }
    }

    void parseInt(bool& returning, std::string_view in);

    template<typename T>
    T parseInt(std::string_view in) {
      T returning = 0;
      parseInt(returning, in);
      return returning;
    }

    using Columns_span = std::span<std::string_view>;

    struct Parsed {
      std::string_view rest;
      bool found = false;
      std::string_view found_str;
    };

    struct Parser {
    public:
      template<char delim, char... quotes>
      Parsed parse_for(std::string_view in) {
        std::string_view::size_type start = 0;

        std::string_view::size_type found_char;

        std::array<char, 1 + sizeof...(quotes)> search_string = { quotes..., delim };

        Parsed returning;

        for (;;) {
          if (looking_for_quote_) {
            found_char = in.find(*looking_for_quote_, start);
          } else {
            found_char = in.find_first_of(std::string_view{ search_string.data(), search_string.size() }, start);
          }

          if (found_char == std::string_view::npos) {
            return returning;
          } else if (in[found_char] != delim) { //isn't delim, must be a quote char
            if (looking_for_quote_) {
              looking_for_quote_.reset();
            } else {
              looking_for_quote_ = in[found_char];
            }
            start = found_char + 1;
          } else { //is delim
            assert(in[found_char] == delim);
            assert(!looking_for_quote_);

            returning.found_str = in.substr(0, found_char);
            returning.rest = in.substr(found_char + 1);
            returning.found = true;

            return returning;
          }
        }
      }
    private:
      std::optional<char> looking_for_quote_;
    };

    Columns_span parse_array(Columns_span returning, std::string_view in);
    void parse_array(std::vector<std::string_view>& returning, std::string_view in);
    std::vector<std::string_view> parse_array(std::string_view in);
  }

  namespace internal {
    struct String_hash {
      using is_transparent = void;

      std::size_t operator()(std::string const& str) const noexcept;
      std::size_t operator()(std::string_view str) const noexcept;
    };

    struct String_eq {
      using is_transparent = void;

      template<typename T, typename U>
      bool operator()(T const& t, U const& u) const noexcept {
        return t == u;
      }
    };

    std::optional<Timestamp> parse_timestamp(std::string_view in);

    struct Partial_parse {
      std::string_view time;
      std::string_view type;
      std::string_view data;
    };

    std::optional<Partial_parse> parse_line(helpers::Parser& parser, std::string_view in);

    template<typename T>
    struct Parse {
      static T parse(helpers::Columns_span);
    };

    template<typename T>
    struct Switch_partial_parse;

    template<typename First, typename ...Rest>
    struct Switch_partial_parse<std::variant<First, Rest...>> {
      template<typename Cb>
      static void check(Partial_parse const& partial_parse, std::size_t start_of_line, Cb&& cb) noexcept {
        if (partial_parse.type == First::NAME) {
          if constexpr (std::is_invocable_v<Cb, Timestamp, const First, std::size_t>) {
            const auto timestamp = parse_timestamp(partial_parse.time);
            if (!timestamp) { //we couldn't parse timestamp, just ignore this entry?
              return;
            }
            std::array<std::string_view, First::COLUMNS_COUNT> columns;
            const auto parsed_columns = helpers::parse_array(columns, partial_parse.data);
            const First data = Parse<First>::parse(parsed_columns);
            cb(*timestamp, data, start_of_line);
          } else {
            //do nothing
          }
        } else {
          Switch_partial_parse<std::variant<Rest...>>::check(partial_parse, start_of_line, std::forward<Cb>(cb));
        }
      }
    };

    template<>
    struct Switch_partial_parse<std::variant<>> {
      template<typename Cb>
      static void check(Partial_parse const& partial_parse, std::size_t start_of_line, Cb&&) noexcept {

      }
    };
  }

  struct String_store {
  public:
    std::string_view get(std::string_view) noexcept;

    events::Combat_log_version get(events::Combat_log_version);
    events::Spell_aura_applied get(events::Spell_aura_applied);
    events::Spell_aura_applied_dose get(events::Spell_aura_applied_dose);
    events::Spell_aura_refresh get(events::Spell_aura_refresh);
    events::Spell_aura_removed get(events::Spell_aura_removed);
    events::Spell_aura_removed_dose get(events::Spell_aura_removed_dose);
    events::Spell_periodic_damage get(events::Spell_periodic_damage);
    events::Spell_periodic_missed get(events::Spell_periodic_missed);
    events::Spell_periodic_heal get(events::Spell_periodic_heal);
    events::Spell_absorbed get(events::Spell_absorbed);
    events::Swing_missed get(events::Swing_missed);
    events::Swing_damage_landed get(events::Swing_damage_landed);
    events::Spell_missed get(events::Spell_missed);
    events::Spell_damage get(events::Spell_damage);
    events::Spell_heal get(events::Spell_heal);
    events::Spell_cast_success get(events::Spell_cast_success);
    events::Encounter_start get(events::Encounter_start);
    events::Encounter_end get(events::Encounter_end);
    events::Combatant_info::Interesting_aura get(events::Combatant_info::Interesting_aura);
    events::Combatant_info get(events::Combatant_info);
    events::Spell_summon get(events::Spell_summon);
    events::Zone_change get(events::Zone_change);
    events::Map_change get(events::Map_change);
    events::Unit_died get(events::Unit_died);
    events::Spell_resurrect get(events::Spell_resurrect);

    template<typename T>
    std::vector<T> get(std::vector<T> in) noexcept {
      for (T& elem : in) {
        elem = this->get(elem);
      }

      return in;
    }

    void clear();
  private:
    std::unordered_set<std::string, internal::String_hash, internal::String_eq> store_;
  };

  struct Event {
    constexpr Event(Timestamp time, events::Type type) noexcept :
      time(std::move(time)),
      type(std::move(type)) {

    }

    Timestamp time;
    events::Type type;
  };

  struct Log {
    auto parsing_cb() noexcept {
      return [this](Timestamp time, auto const& event, std::size_t bytes_on) {
        this->events.push_back(Event{ time, store(this->store_, event) });
      };
    }
    std::vector<Event> events;
  private:
    String_store store_;
  };

  template<typename Cb>
  struct Parser {
  public:
    Parser(Cb cb) :
      cb_(std::forward<Cb>(cb)) {

    }

    void parse(std::string_view recved) {
      helpers::Parsed res;
      std::optional<internal::Partial_parse> partial_parse;

      while ((res = parser_.parse_for<'\n', '"'>(recved)).found) {
        auto found_str_size = 1; //start with 1 for \n
        if (!res.found_str.empty() && res.found_str.back() == '\r') {
          res.found_str = res.found_str.substr(0, res.found_str.size() - 1);
          found_str_size++;
        }
        if (saved_.empty()) {
          found_str_size += res.found_str.size();
          partial_parse = internal::parse_line(parser_, res.found_str);
        } else {
          saved_.append(res.found_str);
          found_str_size += saved_.size();
          partial_parse = internal::parse_line(parser_, saved_);
        }

        if (partial_parse) {
          internal::Switch_partial_parse<events::Type>::check(*partial_parse, bytes_parsed_, cb_);
          if (!saved_.empty()) {
            saved_.clear();
          }
        }

        bytes_parsed_ += found_str_size;
        recved = res.rest;
      }
      saved_.append(recved);
    }
  private:
    Cb cb_;
    helpers::Parser parser_;
    std::string saved_;
    std::size_t bytes_parsed_ = 0;
  };
}