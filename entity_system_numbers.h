#ifndef GAME_ENGINE_ENTITY_SYSTEM_NUMBERS_H
#define GAME_ENGINE_ENTITY_SYSTEM_NUMBERS_H

#include <cstdint>
#include <limits>

#include "useful.h"

namespace game_engine {
	constexpr std::size_t max_entities_including_placeholder = (1 << 16) - 1;
	constexpr std::size_t max_component_managers = 65025;
	constexpr std::size_t max_component_instances_per_manager_including_placeholder = (1 << 16) - 1;
	constexpr std::size_t max_unique_component_combinations = max_component_managers;


	constexpr std::size_t max_entities_excluding_placeholder = max_entities_including_placeholder - 1;
	constexpr std::size_t max_component_instances_per_manager_excluding_placeholder = max_component_instances_per_manager_including_placeholder - 1;
	constexpr std::size_t max_component_managers_per_slot = std::min(max_component_instances_per_manager_including_placeholder, (std::size_t) std::numeric_limits<std::uint_fast8_t>::max());
	constexpr std::size_t max_component_slots = std::uintmax_t(max_component_managers - 1 + max_component_managers_per_slot) / max_component_managers_per_slot;

	using entity_reference_count_nonatomic = std::size_t;
	using entity_reference_count_atomic = std::atomic<entity_reference_count_nonatomic>;

	using component_manager_id = oskar::big_enough_fast_uint<max_component_managers>;
	using component_instance_id = oskar::big_enough_fast_uint<max_component_instances_per_manager_including_placeholder>;
	using entity_count = oskar::big_enough_fast_uint<max_entities_including_placeholder>;
	using entity_id = oskar::big_enough_fast_uint<max_entities_including_placeholder - 1>;
	using component_per_slot_manager_id = oskar::big_enough_fast_uint<max_component_managers_per_slot>;
	using component_slot_number = oskar::big_enough_fast_uint<max_component_slots - 1 + 1>; // +1 for null_component_slot_number
	using unique_component_combination_id = oskar::big_enough_fast_uint<max_unique_component_combinations - 1>;
	using unique_component_combination_count = oskar::big_enough_fast_uint<max_unique_component_combinations>;
	constexpr component_instance_id placeholder_component_instance_id = 0; // Changing this value requires changes to a lot of code. 0 is a good value because it lets components store placeholder data at the beginning of vectors
	constexpr entity_id placeholder_entity_id = 0; // Changing this value requires changes to a lot of code. 0 is a good value because it lets us store placeholder data at the beginning of vectors
	constexpr component_slot_number null_component_slot_number = 0;
	constexpr component_manager_id null_component_manager_id = max_component_managers;
}


#endif
