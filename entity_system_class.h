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
#include <optional>
#include <new>
#include <future>

#include "boost/container/small_vector.hpp"

#include "shortmap.h"
#include "compilation_target_info.h"
#include "small_set.h"
#include "useful.h"

#include "entity_system_numbers.h"
#include "entity_system_refs_and_pointers.h"
#include <atomic>

namespace game_engine {





	class component_manager;
	class context;

	class entity_system {
		friend class worker_specific_entity_system_operations;
		private:

			struct alignas(std::hardware_destructive_interference_size) worker_data {
				std::size_t worker_number = 0;
				entity_system& ent_sys;
				std::unique_ptr<unsigned long long []> macro_entity_deletion_flags;

				constexpr worker_data(std::size_t num, entity_system& es) noexcept : worker_number(num), ent_sys(es) { }
			};

			std::vector<worker_data> worker_data_by_worker_number;

			std::size_t macro_entity_deletion_flags_scale = 0;


			std::vector<deletion_point> deletion_points_by_entity_id;
			std::vector<unique_component_combination_id> ucc_ids_by_entity_id;
			std::vector<component_instance_id> ci_ids_by_slot_and_entity_id;
			std::vector<std::atomic<unsigned long long>[]> entity_deletion_flags;

			std::vector<component_manager_short_id> component_manager_short_ids_by_slot_and_ucc_id;

			unique_component_combination_count num_uccs_allocated_for = 0;
			unique_component_combination_count num_uccs_used = 0;
			component_slot_count num_component_slots = 0;

			entity_id greatest_live_entity_id = 0;
			entity_count num_alive_entities = 0;
			entity_count num_corpse_entities = 0;
			entity_count num_entities_allocated_for = 0;
			//entity_count num_doa_entities = 0;
			//entity_count num_zeroed_entities = 0;

			/*void queue_entity_creation(std::size_t num, component_set cset) {

			}
			*/
			deletion_point last_deletion_point = 0;


			// Remember!!! The placeholder entity needs to have a deletion point == max_deletion_points_including_last_one_reserved_for_placeholder_entity
			// And we need make sure we never reach that high


			void think_step_main() {

			}
			void think_step_deletion() {
				// This part could be parallelized



				//std::size_t numBlocks = 
				//std::unique_ptr<std::atomic<unsigned long long>[]> ptr(new unsigned long long[numBlocks])
				{ // Zero the macro flags... and fill the jump array???
					const std::size_t numMeaningfulMacroUintsPerWorker = (greatest_live_entity_id >> macro_entity_deletion_flags_scale) / std::numeric_limits<unsigned long long>::digits;

					unsigned long long* macro_entity_deletion_flags_first_combined = worker_data_by_worker_number[0].macro_entity_deletion_flags.get();
					for(auto workerDataIt = ++(worker_data_by_worker_number.begin()); workerDataIt != worker_data_by_worker_number.end(); ++workerDataIt) {
						for(std::size_t i = 0; i != numMeaningfulMacroUintsPerWorker; ++i) {
							macro_entity_deletion_flags_first_combined[i] |= workerDataIt->macro_entity_deletion_flags[i];
							workerDataIt->macro_entity_deletion_flags[i] = 0;
						}
					}
					for(std::size_t i = 0; i != numMeaningfulMacroUintsPerWorker; ++i) {
						macro_entity_deletion_flags_first_combined[i] = 0;
					}
				}

				std::atomic<unsigned long long>* search_point = 5;
				while(int i = 0) {
					++search_point;
					delete_ent(4040);
				}
			}
		public:

			entity_system() {

				std::size_t numWorkerThreads = std::size_t(std::clamp(std::thread::hardware_concurrency(), (unsigned int) 1, (unsigned int) std::numeric_limits<std::size_t>::max()));

				worker_data_by_worker_number.reserve(numWorkerThreads);
				for(std::size_t wtNum = 0; wtNum != numWorkerThreads; ++wtNum) {
					worker_data_by_worker_number.push_back(worker_data(wtNum, *this));
				}

				macro_entity_deletion_flags_scale = ([&]() {
					constexpr std::size_t number_of_threads_before_we_start_saving_memory = 64; // When we a huge number of threads, we don't want to use too much memory
					std::size_t macro_flags_scale = oskar::integer_log2(
						std::max(number_of_threads_before_we_start_saving_memory, numWorkerThreads) *
						std::hardware_constructive_interference_size * std::size_t(std::numeric_limits<unsigned char>::digits) // Cache line size in bits
					);
					macro_flags_scale -= oskar::constexpr_integer_log2(number_of_threads_before_we_start_saving_memory);

					if(macro_flags_scale > std::size_t(std::numeric_limits<entity_id>::digits - 1)) { // Prevent bitshift overflow that might otherwise happen with a ridiculous number of threads
						macro_flags_scale = std::numeric_limits<entity_id>::digits - 1;
					}
					return macro_flags_scale;
				})();

				
			}

			void think() {
				think_step_main();
				think_step_deletion();
			}



			constexpr deletion_point get_last_deletion_point() const {
				return last_deletion_point;
			}


			bool has_entity_been_deleted(entity_pointer_with_deletion_point ent) const {
				return deletion_points_by_entity_id[ent.get_entity_id()] > ent.get_deletion_point();
			}
			bool has_entity_been_deleted(temporary_entity_pointer ent) const = delete; // Temporary references and pointers ALWAYS point to existing entities, unless you're keeping the refs/ptrs past their lifetime


			bool entity_has_component(temporary_entity_pointer ent, component_slot_number slotNumber, component_manager_short_id cmShortId) {
				const unique_component_combination_id uccId = ucc_ids_by_entity_id[std::size_t(slotNumber) * num_entities_allocated_for + ent.get_entity_id()];
				const component_manager_short_id actualShortId = component_manager_short_ids_by_slot_and_ucc_id[std::size_t(slotNumber) * num_uccs_allocated_for + uccId];
				return actualShortId == cmShortId;
			}
			bool entity_has_component(entity_pointer_with_deletion_point ent, component_slot_number slotNumber, component_manager_short_id cmShortId) {
				const unique_component_combination_id uccId = ucc_ids_by_entity_id[std::size_t(slotNumber) * num_entities_allocated_for + ent.get_entity_id()];
				const component_manager_short_id actualShortId = component_manager_short_ids_by_slot_and_ucc_id[std::size_t(slotNumber) * num_uccs_allocated_for + uccId];
				return !has_entity_been_deleted(ent) && actualShortId == cmShortId;
			}

			component_instance_id get_component_instance_id(temporary_entity_reference ent, component_slot_number slotNumber) const {
				const component_instance_id ciId = ci_ids_by_slot_and_entity_id[std::size_t(slotNumber) * num_component_slots + ent.get_entity_id()];
				return ciId;
			}

			std::optional<component_instance_id> try_to_get_component_instance_id(temporary_entity_pointer ent, component_slot_number slotNumber, component_manager_short_id cmShortId) const {
				const unique_component_combination_id uccId = ucc_ids_by_entity_id[ent.get_entity_id()];
				const component_manager_short_id managerShortId = component_manager_short_ids_by_slot_and_ucc_id[std::size_t(slotNumber) * num_uccs_allocated_for + uccId];
				std::optional<component_instance_id> ciId = ci_ids_by_slot_and_entity_id[std::size_t(slotNumber) * num_component_slots + ent.get_entity_id()];
				if(managerShortId != cmShortId) [[unlikely]] {
					ciId = std::nullopt;
				}
				return ciId;
			}
			std::optional<component_instance_id> try_to_get_component_instance_id(entity_pointer_with_deletion_point ent, component_slot_number slotNumber, component_manager_short_id cmShortId) const {
				const unique_component_combination_id uccId = ucc_ids_by_entity_id[ent.get_entity_id()];
				const component_manager_short_id managerShortId = component_manager_short_ids_by_slot_and_ucc_id[std::size_t(slotNumber) * num_uccs_allocated_for + uccId];
				std::optional<component_instance_id> ciId = ci_ids_by_slot_and_entity_id[std::size_t(slotNumber) * num_component_slots + ent.get_entity_id()];
				if(has_entity_been_deleted(ent) || managerShortId != cmShortId) [[unlikely]] {
					ciId = std::nullopt;
				}
				return ciId;
			}

			temporary_entity_pointer create_temporary_entity_pointer(entity_pointer_with_deletion_point ent) const {
				return temporary_entity_pointer(has_entity_been_deleted(ent) ? placeholder_entity_id : ent.get_entity_id());
			}
			temporary_entity_reference create_temporary_entity_reference(entity_pointer_with_deletion_point ent) const {
				if(has_entity_been_deleted(ent)) [[unlikely]] {
					throw std::runtime_error("entity_system::create_temporary_entity_reference() - entity has been deleted (or ent == nullptr)");
				}
				return temporary_entity_reference(ent.get_entity_id());
			}


	};
	class worker_specific_entity_system_operations {
		private:
			entity_system::worker_data& worker_specific_data;

		public:
			

			void mark_entity_for_deletion(temporary_entity_pointer ent) {
				entity_id index = ent.get_entity_id();


				entity_id macroIndex = index >> worker_specific_data.ent_sys.macro_entity_deletion_flags_scale;
				unsigned long long macroMask = ((unsigned long long) 1) << (macroIndex % std::numeric_limits<unsigned long long>::digits);
				worker_specific_data.macro_entity_deletion_flags[macroIndex / std::numeric_limits<unsigned long long>::digits] |= macroMask;


				unsigned long long mask = ((unsigned long long) 1) << (index % std::numeric_limits<unsigned long long>::digits);
				worker_specific_data.ent_sys.entity_deletion_flags[index / std::numeric_limits<unsigned long long>::digits].fetch_or(mask, std::memory_order_relaxed);

			}
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
		component_manager_short_id short_id;
		component_manager_long_id long_id;

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



		bool entity_has_component(temporary_entity_pointer ent) const {
			return es.entity_has_component(ent, slot_number, short_id);
		}

		bool entity_has_component(entity_pointer_with_deletion_point ent) const {
			return es.entity_has_component(ent, slot_number, short_id);
		}

		component_instance_id get_component_instance_id_for_entity(temporary_entity_reference ent) const {
			return es.get_component_instance_id(ent, slot_number);
		}


		std::optional<component_instance_id> get_component_instance_id_for_entity_if_one_exists(temporary_entity_pointer ent) const {
			return es.try_to_get_component_instance_id(ent, slot_number, short_id);
		}
		std::optional<component_instance_id> get_component_instance_id_for_entity_if_one_exists(temporary_entity_reference ent) const {
			return es.try_to_get_component_instance_id(ent, slot_number, short_id);
		}

	};
	
}




#endif
