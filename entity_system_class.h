#ifndef GAME_ENGINE_ENTITY_SYSTEM_CLASS_H
#define GAME_ENGINE_ENTITY_SYSTEM_CLASS_H

#include <memory>
#include <vector>
#include <cstdint>
#include <type_traits>
#include <limits>
#include <optional>
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

#include "nonstd_observer_ptr.hpp"
#include "boost/container/small_vector.hpp"

#include "shortmap.h"
#include "compilation_target_info.hpp"
#include "small_set.h"
#include "useful.h"
#include "integer_log2.h"
#include "over_aligned_heap_array.hpp"

#include "entity_system_numbers.h"
#include "entity_system_refs_and_pointers.h"

//#include "entity_system_worker.hpp"

namespace game_engine {



	class component_manager;
	class context;

	class entity_system {
		friend class worker_specific_entity_system_operations;
		private:

			// deletion step data
			//std::unique_ptr<component_instance_count[]> num_instance_ids_in_lists_by_manager_id; // Allocation size: numManagers
			//std::unique_ptr<oskar::big_enough_fast_uint<max_component_manager_ids_including_null * max_component_instances_per_manager>[]> offsets_to_next_by_manager_id; // Allocation size: numManagers
			//std::unique_ptr<component_instance_id[]> instances_to_delete; // Allocation size: numEntsAllocatedFor
				

			static constexpr std::size_t entity_deletion_flags_block_size_and_alignment = oskar::divide_integer_rounding_away_from_zero((std::size_t) entities_per_full_segment, (std::size_t) std::numeric_limits<unsigned char>::digits);
			using deletion_flags_block = std::array<std::byte, entity_deletion_flags_block_size_and_alignment>;
			std::unique_ptr<deletion_flags_block, oskar::over_aligned_heap_array_deleter<deletion_flags_block>> entity_deletion_flags;
			constexpr std::size_t entity_deletion_flags_blocks_per_worker() const {
				return oskar::divide_integer_rounding_away_from_zero(num_entities_allocated_for, entities_per_full_segment);
			}
			

			//std::vector<std::unique_ptr<entity_count>[]> deletion_step_ent_ids_by_manager_id;
			//std::unique_ptr<entity_count[]> deletion_step_num_ents_by_manager_id;
			//std::unique_ptr<entity_count[]> deletion_step_num_ents;
			//std::unique_ptr<entity_count[]> deletion_step_num_ents_by_ucc;


			struct worker_data {
				worker_id id = 0;
				entity_system& ent_sys;

				constexpr worker_data(worker_id i, entity_system& es) noexcept : id(i), ent_sys(es) { }
				constexpr nonstd::observer_ptr<deletion_flags_block> get_deletion_flags_row_ptr() noexcept {
					deletion_flags_block* dfbPtr = ent_sys.entity_deletion_flags.get() + id * ent_sys.num_blocks_per_entity_deletion_flags_row;

					#ifdef __has_builtin
						#if  __has_builtin(__builtin_assume_aligned)
							dfbPtr = __builtin_assume_aligned(dfbPtr, entity_deletion_flags_block_size_and_alignment);
						#endif
					#endif
					return nonstd::observer_ptr<deletion_flags_block>(dfbPtr);
				}
				constexpr std::span<deletion_flags_block> get_deletion_flags_span() noexcept {

				}
			};

			std::unique_ptr<worker_data, oskar::over_aligned_heap_array_deleter<worker_data>> worker_data_by_worker_number;
			worker_count num_workers = 0;

			std::vector<oskar::unowned<component_manager>> component_manager_pointers_by_id;
			//std::vector<oskar::unowned<component_manager*>> component_manager_pointers_by_slot_and_short_id;

			std::vector<deletion_point> deletion_points_by_entity_id;
			std::vector<unique_component_combination_id> ucc_ids_by_entity_id;
			std::vector<component_instance_id> ci_ids_by_slot_and_entity_id;

			std::vector<component_manager_id> component_manager_ids_by_slot_and_ucc_id;

			unique_component_combination_count num_uccs_allocated_for = 0;
			unique_component_combination_count num_uccs_used = 0;
			component_slot_count num_component_slots = 0;
			entity_count num_entities_allocated_for = 0; // Including the placeholder entity
			std::size_t total_number_of_component_instances;


			entity_id greatest_entity_id = 0;
			/*entity_id greatest_live_entity_id = 0;
			entity_count num_alive_entities = 0;
			entity_count num_corpse_entities = 0;*/
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
			void think_step_deletion();
		public:

			entity_system() {

				std::size_t numWorkerThreads = std::size_t(std::clamp(std::thread::hardware_concurrency(), (unsigned int) 1, (unsigned int) std::numeric_limits<std::size_t>::max()));



				worker_data_by_worker_number.reserve(numWorkerThreads);
				for(std::size_t wtNum = 0; wtNum != numWorkerThreads; ++wtNum) {
					worker_data_by_worker_number.push_back(worker_data(wtNum, *this));
				}


				
			}

			void think() {
				think_step_main();
				think_step_deletion();
			}
			void worker_think();



			constexpr deletion_point get_last_deletion_point() const {
				return last_deletion_point;
			}


			bool has_entity_been_deleted(entity_pointer_with_deletion_point ent) const {
				return deletion_points_by_entity_id[ent.get_entity_id()] > ent.get_deletion_point();
			}
			bool has_entity_been_deleted(temporary_entity_pointer ent) const = delete; // Temporary references and pointers ALWAYS point to existing entities, unless you're keeping the refs/ptrs past their lifetime


			bool entity_has_component(temporary_entity_pointer ent, component_slot_number slotNumber, component_manager_id cmId) {
				const unique_component_combination_id uccId = ucc_ids_by_entity_id[ent.get_entity_id()];
				const component_manager_id actualManagerId = component_manager_ids_by_slot_and_ucc_id[std::size_t(slotNumber) * num_uccs_allocated_for + uccId];
				return actualManagerId == cmId;
			}
			bool entity_has_component(entity_pointer_with_deletion_point ent, component_slot_number slotNumber, component_manager_id cmId) {
				const unique_component_combination_id uccId = ucc_ids_by_entity_id[ent.get_entity_id()];
				const component_manager_id actualManagerId = component_manager_ids_by_slot_and_ucc_id[std::size_t(slotNumber) * num_uccs_allocated_for + uccId];
				return !has_entity_been_deleted(ent) && actualManagerId == cmId;
			}

			component_instance_id get_component_instance_id(temporary_entity_reference ent, component_slot_number slotNumber) const {
				const component_instance_id ciId = ci_ids_by_slot_and_entity_id[std::size_t(slotNumber) * num_entities_allocated_for + ent.get_entity_id()];
				return ciId;
			}

			std::optional<component_instance_id> try_to_get_component_instance_id(temporary_entity_pointer ent, component_slot_number slotNumber, component_manager_id cmId) const {
				const unique_component_combination_id uccId = ucc_ids_by_entity_id[ent.get_entity_id()];
				const component_manager_id actualManagerId = component_manager_ids_by_slot_and_ucc_id[std::size_t(slotNumber) * num_uccs_allocated_for + uccId];
				std::optional<component_instance_id> ciId = ci_ids_by_slot_and_entity_id[std::size_t(slotNumber) * num_entities_allocated_for + ent.get_entity_id()];
				if(actualManagerId != cmId) [[unlikely]] {
					ciId = std::nullopt;
				}
				return ciId;
			}
			std::optional<component_instance_id> try_to_get_component_instance_id(entity_pointer_with_deletion_point ent, component_slot_number slotNumber, component_manager_id cmId) const {
				const unique_component_combination_id uccId = ucc_ids_by_entity_id[ent.get_entity_id()];
				const component_manager_id actualManagerId = component_manager_ids_by_slot_and_ucc_id[std::size_t(slotNumber) * num_uccs_allocated_for + uccId];
				std::optional<component_instance_id> ciId = ci_ids_by_slot_and_entity_id[std::size_t(slotNumber) * num_entities_allocated_for + ent.get_entity_id()];
				if(has_entity_been_deleted(ent) || actualManagerId != cmId) [[unlikely]] {
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

	class null_component_manager;
	class component_manager {
	private:
		friend class entity_system;
		friend class null_component_manager;

		component_slot_number slot_number = 0;
		component_manager_id id_number = 0;

		entity_system& ent_sys;

		component_instance_count num_instances = 0;

		std::unique_ptr<component_instance_id[]> instances_to_be_deleted; // Note: must be resized to be big enough to fit all component instance ids. Also, null_component_manager must have at least one element to facilitate branchless writes
		component_instance_count num_instances_to_be_deleted = 0;
		component_instance_count instances_to_be_deleted_allocated_size = 32;
		std::size_t cpu_time_per_deletion_for_15_or_fewer_deletions = 100;
		std::size_t cpu_time_per_deletion_for_16_or_more_deletions = 100;


		void enqueue_component_instance_deletion(component_instance_id instanceId) {
			assert(num_queued_deletions < num_instances);
			instance_deletion_queue[num_queued_deletions] = instanceId;
			++num_queued_deletions;
		}

		void process_deletion_queue() {
			if(num_queued_deletions) {
				computation_timer timer;
				do_delete_instances(&instance_deletion_queue[0], &instance_deletion_queue[num_queued_deletions - 1]);
				num_instances -= num_queued_deletions;
				num_queued_deletions = 0;
				std::size_t time = timer.time_elapsed();
				
			}
		}

		std::size_t estimate_time_required_for_deletions(std::size_t n) const {
			const std::size_t time_per_n = n < 15 ? cpu_time_per_deletion_for_15_or_fewer_deletions : cpu_time_per_deletion_for_16_or_more_deletions;
			std::size_t result = n;
			result *= time_per_n;
			if(result / time_per_n != n) { // Overflow test
				result = std::numeric_limits<std::size_t>::max();
			}
			return result;
		}

	protected:
		struct allocation_result {
			component_instance_id first_id;
			component_instance_count num_created;
		};

	private:
		void queue_deletion(component_instance_id instanceId) {
			assert(num_instances_to_be_deleted < instances_to_be_deleted_allocated_size);
			assert(num_instances_to_be_deleted < num_instances);

			instances_to_be_deleted[num_instances_to_be_deleted] = instanceId;
			num_instances_to_be_deleted += get_id() != null_component_manager_id;
		}

		void create_instances(component_instance_count numInstancesToCreate, component_instance_id* outPtr) {
			assert(num_instances_to_be_deleted == 0);
			assert(outPtr != nullptr);

			if(numInstancesToCreate && get_id() != null_component_manager_id) [[likely]] {
				component_instance_count newNumInstances = num_instances + numInstancesToCreate;

				if(newNumInstances < num_instances || newNumInstances > max_component_instances_per_manager) {
					throw std::runtime_error("Tried to create too many instances of the same component type");
				}

				if(newNumInstances > instances_to_be_deleted_allocated_size) {
					component_instance_count newInstancesToBeDeletedAllocatedSize = newNumInstances + newNumInstances / 2;
					if(newInstancesToBeDeletedAllocatedSize < newNumInstances) [[unlikely]] { // Overflow correction
						newInstancesToBeDeletedAllocatedSize = max_component_instances_per_manager;
					}
					instances_to_be_deleted.reset(new component_instance_id[newInstancesToBeDeletedAllocatedSize]);
					instances_to_be_deleted_allocated_size = newInstancesToBeDeletedAllocatedSize;
				}

				do_create_instances(numInstancesToCreate, outPtr);
				num_instances = newNumInstances;
			}
		}

		void carry_out_queued_deletions() {
			if(num_instances_to_be_deleted) {
				do_delete_instances(instances_to_be_deleted.get(), num_instances_to_be_deleted);
				num_instances -= num_instances_to_be_deleted;
				num_instances_to_be_deleted = 0;

				// Space saving measures:
				if(instances_to_be_deleted_allocated_size > 32 && num_instances < instances_to_be_deleted_allocated_size / 16) {
					const component_instance_count newInstancesToBeDeletedAllocatedSize = std::max(std::size_t(32), num_instances * 4);
					instances_to_be_deleted.reset(new component_instance_id[newInstancesToBeDeletedAllocatedSize]);
					instances_to_be_deleted_allocated_size = newInstancesToBeDeletedAllocatedSize;
				}
			}
		}

	protected:
		virtual void do_create_instances(component_instance_count numInstancesToCreate, component_instance_id* outPtr) = 0; // Called by at most one thread at a time

		virtual void do_delete_instances(const component_instance_id* listStart, std::size_t numToDelete) = 0; // Called by at most one thread at a time

		[[nodiscard]] virtual std::string_view get_unique_name_utf8() const = 0;

	public:
		component_manager(entity_system& es) : instances_to_be_deleted(std::make_unique<component_instance_id[]>(32)), ent_sys(es) {}


		[[nodiscard]] component_instance_count instance_count() const noexcept {
			return num_instances;
		}
		[[nodiscard]] component_manager_id get_id() const noexcept {
			return id_number;
		}

		[[nodiscard]] bool entity_has_component(temporary_entity_pointer ent) const {
			return ent_sys.entity_has_component(ent, slot_number, id_number);
		}

		[[nodiscard]] bool entity_has_component(entity_pointer_with_deletion_point ent) const {
			return ent_sys.entity_has_component(ent, slot_number, id_number);
		}

		[[nodiscard]] component_instance_id get_instance_id_for_entity(temporary_entity_reference ent) const {
			return ent_sys.get_component_instance_id(ent, slot_number);
		}


		[[nodiscard]] std::optional<component_instance_id> get_instance_id_for_entity_if_one_exists(temporary_entity_pointer ent) const {
			return ent_sys.try_to_get_component_instance_id(ent, slot_number, id_number);
		}
		[[nodiscard]] std::optional<component_instance_id> get_instance_id_for_entity_if_one_exists(temporary_entity_reference ent) const {
			return ent_sys.try_to_get_component_instance_id(ent, slot_number, id_number);
		}

	};


	class null_component_manager final : public component_manager {
		protected:
			virtual void do_create_instances(component_instance_count numInstancesToCreate, component_instance_id* outPtr) override {
				std::fill(outPtr, outPtr + numInstancesToCreate, component_instance_id(0));
			}

			virtual void do_delete_instances(const component_instance_id* listStart, std::size_t numToDelete) override {}

			[[nodiscard]] virtual std::string_view get_unique_name_utf8() const override {
				return "";
			}

		public:

	};
}




#endif
