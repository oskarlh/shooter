#ifndef GAME_ENGINE_ENTITY_SYSTEM_NUMBERS_H
#define GAME_ENGINE_ENTITY_SYSTEM_NUMBERS_H

#include <cstdint>
#include <limits>

#include "useful.h"

namespace game_engine {
	constexpr std::size_t max_entities_including_placeholder = (1ULL << 16) - 1;

	constexpr std::size_t max_component_manager_long_ids_including_null = (1ULL << 16) - 1;
	constexpr std::size_t max_component_instances_per_manager = (1ULL << 16) - 1;
	constexpr std::size_t max_unique_component_combinations = max_component_manager_long_ids_including_null;


	constexpr std::size_t max_entities_excluding_placeholder = max_entities_including_placeholder - 1;
	constexpr std::size_t max_component_manager_short_ids_including_null = std::min(max_component_manager_long_ids_including_null - 1, (std::size_t) std::numeric_limits<std::uint_fast8_t>::max()) + 1;
	constexpr std::size_t max_component_slots = max_component_manager_long_ids_including_null - 1;

	using entity_reference_count_nonatomic = std::size_t;
	using entity_reference_count_atomic = std::atomic<entity_reference_count_nonatomic>;

	using component_manager_long_id = oskar::big_enough_fast_uint<max_component_manager_long_ids_including_null - 1>;
	using component_instance_id = oskar::big_enough_fast_uint<max_component_instances_per_manager - 1>;
	using component_instance_count = oskar::big_enough_fast_uint<max_component_instances_per_manager>;
	using entity_count = oskar::big_enough_fast_uint<max_entities_including_placeholder>;
	using entity_id = oskar::big_enough_fast_uint<max_entities_including_placeholder - 1>;
	using component_manager_short_id = oskar::big_enough_fast_uint<max_component_manager_short_ids_including_null - 1>;
	using component_slot_number = oskar::big_enough_fast_uint<max_component_slots - 1>;
	using component_slot_count = oskar::big_enough_fast_uint<max_component_slots>;
	using unique_component_combination_id = oskar::big_enough_fast_uint<max_unique_component_combinations - 1>;
	using unique_component_combination_count = oskar::big_enough_fast_uint<max_unique_component_combinations>;
	constexpr entity_id placeholder_entity_id = 0; // Changing this value requires changes to a lot of code. 0 is a good value because it lets us store placeholder data at the beginning of vectors
	constexpr component_manager_long_id null_component_manager_long_id = 0;
	constexpr component_manager_short_id null_component_manager_short_id = 0;

	constexpr std::uintmax_t max_deletion_points_including_last_one_reserved_for_placeholder_entity = 1ULL << 32; // A "deletion point" is a point in time when one or more entities (and the component instances they own) are deleted. Simulation start also counts as a deletion point
	using deletion_point = oskar::big_enough_fast_uint<max_deletion_points_including_last_one_reserved_for_placeholder_entity - 1>;


}


#endif
