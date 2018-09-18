#ifndef GAME_ENGINE_ENTITY_SYSTEM_CLASS_H
#define GAME_ENGINE_ENTITY_SYSTEM_CLASS_H

#include <memory>
#include <vector>
#include <cstdint>
#include <type_traits>
#include <limits>
#include <optional>
#include <atomic>
#include <algorithm>
#include <thread>
#include <stdexcept>
#include <any>
#include <cstddef>
#include <utility>
#include <functional>

#include "boost/container/small_vector.hpp"

#include "shortmap.h"
#include "compilation_target_info.h"
#include "small_set.h"
#include "useful.h"

#include "entity_system_numbers.h"
#include "entity_system_temp_refs_and_pointers.h"


namespace game_engine {





	class component_manager;
	class context;

	class entity_system {
		private:
			friend class entity_pointer;
			friend class entity_reference;
			friend class component_manager;

			union component_instance_id_or_ucc_id {
				component_instance_id ci_id;
				unique_component_combination_id ucc_id;
			};

			struct alignas(std::hardware_destructive_interference_size) thread_data {
				std::size_t thread_number = 0;
				entity_system& ent_sys;
				std::vector<entity_id> entsToKill;
			};

			std::vector<thread_data> per_thread_data;


			std::vector<entity_reference_count_atomic> entity_refrence_counts;
			
			std::vector<component_instance_id_or_ucc_id> ucc_id_and_component_instance_ids_by_entity_and_slot;
			//std::vector<component_instance_id> entity_component_instance_ids;
			//std::vector<component_per_slot_manager_id> entity_component_per_slot_manager_ids;
			//std::vector<unique_component_combination_id> entity_ucc_numbers;
			std::vector<component_per_slot_manager_id> component_per_slot_manager_ids_by_slot_and_ucc;

			unique_component_combination_count num_uccs_allocated_log2 = 0; // TODO: Better (smaller) type
			unique_component_combination_count num_uccs_used = 0;
			oskar::big_enough_fast_uint<max_component_slots + 1> num_component_slots_plus_one = 0; // plus one for null

			entity_count num_alive_entities = 0;
			entity_count num_corpse_entities = 0;
			entity_count num_entities_allocated_for = 0;
			//entity_count num_doa_entities = 0;
			//entity_count num_zeroed_entities = 0;
			
			/*void queue_entity_creation(std::size_t num, component_set cset) {

			}
			*/

			void increase_reference_counter(entity_id entityId) {
				entity_refrence_counts[entityId].fetch_add(1, std::memory_order_relaxed);
			}
			void decrease_reference_counter(entity_id entityId) {
				entity_refrence_counts[entityId].fetch_sub(1, std::memory_order_relaxed);
			}

			component_per_slot_manager_id get_component_per_slot_manager_id(temporary_entity_pointer ent, component_slot_number slotNumber) const {
				unique_component_combination_id ucc_id = ucc_id_and_component_instance_ids_by_entity_and_slot[std::size_t(num_component_slots_plus_one) * ent.get_entity_id()].ucc_id;
				return component_per_slot_manager_ids_by_slot_and_ucc[std::size_t(slotNumber) << num_uccs_allocated_log2 + ucc_id];
			}
			component_instance_id get_component_instance_id(temporary_entity_pointer ent, component_slot_number slotNumber) const {
				return ucc_id_and_component_instance_ids_by_entity_and_slot[std::size_t(num_component_slots_plus_one) * ent.get_entity_id() + slotNumber].ci_id;
			}

			std::pair<component_per_slot_manager_id, component_instance_id> get_component_per_slot_manager_id_and_instance_id(temporary_entity_pointer ent, component_slot_number slotNumber) const {
				const component_instance_id_or_ucc_id* entityData = &ucc_id_and_component_instance_ids_by_entity_and_slot[std::size_t(num_component_slots_plus_one) * ent.get_entity_id()];
				return std::pair(entityData->ucc_id, (entityData + slotNumber)->ci_id);
			}

		public:

	};











	/*class context {
		private:
			entity_system::thread_data& this_threads_entity_system_data;

		public:
			constexpr context(entity_system::thread_data& td) noexcept : this_threads_entity_system_data(td) {}

	};*/

	/*

	class system {
		
		lolSystem::think();
		lolSystem::think();
		dependent systems
	};

	struct component_data_interface_descriptor {
		//oskar::unowned<const component*> comp;
		bool disallow_when_being_worked_on = true;
		bool one_accessed_component_instance_at_a_time = true;
		bool one_thread_at_a_time = false;

	};
	struct component_data_interface_access_needs {
		oskar::small_set<std::reference_wrapper<const component_data_interface_descriptor>> access_needs_on_first_pass;
		oskar::small_set<std::reference_wrapper<const component_data_interface_descriptor>> access_needs_on_second_pass;
	};
	template<class ComponentClass> class component_accessor {
	private:
		ComponentClass & comp;
		context ctx;
		enum component_access_level {
			none,
			read,
			write
		};
		enum access_restrictions {
			all_at_once,
			one_at_a_time,
			all_at_once_in_front,
			all_at_once_behind,
			one_at_a_time_in_front,
			one_at_a_time_behind,
		};
		struct access_restrictions {
			bool disallow_when_being_worked_on = true;
			bool disallow_on_first_pass = false;
			bool disallow_on_second_pass = false;
		};

	public:
		constexpr component_accessor(ComponentClass& co, const context& ct) noexcept : comp(co), ctx(ct) {}
		//physics.for_ent(eid, read_only, bubble, do_throw_if_null).queue(physics::kill);

	};*/

	class component_manager {
	private:
		component_slot_number slot_number;
		component_per_slot_manager_id per_slot_id;
		entity_system& es;

	public:
		struct allocation_result {
			entity_count remaining;
			component_instance_id start;
			component_instance_id end;
		};
		virtual allocation_result allocate(entity_count n) = 0;
		virtual void deallocate(temporary_entity_reference ent) = 0;

		virtual std::string_view getUniqueNameUtf8() const = 0;



		bool has_entity_component_instance(temporary_entity_pointer ent) const {
			return es.get_component_per_slot_manager_id(ent, slot_number) == per_slot_id;
		}

		component_instance_id get_component_instance_id_for_entity(temporary_entity_pointer ent) const {
			return es.get_component_instance_id(ent, slot_number);
		}


		std::optional<component_instance_id> get_component_instance_id_for_entity_if_one_exists(temporary_entity_pointer ent) const {
			auto [perSlotId, instanceId] = es.get_component_per_slot_manager_id_and_instance_id(ent, slot_number);
			std::optional<component_instance_id> result;
			if(perSlotId == per_slot_id) {
				result = instanceId;
			}
			return result;
		}

		component_instance_id get_component_instance_id_or_placeholder(temporary_entity_pointer ent) const {
			auto[perSlotId, instanceId] = es.get_component_per_slot_manager_id_and_instance_id(ent, slot_number);
			component_instance_id result = placeholder_component_instance_id;
			if(perSlotId == per_slot_id) {
				result = instanceId;
			}
			return result;
		}
	};
	
}




#endif
