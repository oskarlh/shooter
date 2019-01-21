#include "entity_system_class.h"
#include <execution>




template<class RAND_ACC_IT, class FUNC>
void combine_rows(RAND_ACC_IT startIt, std::size_t width, std::size_t paddedWidth, std::size_t height, FUNC combine_func, std::size_t threadNum = 0, numThreads = 1, multiple = 1) {
	ASSERT(multiple > 0);
	ASSERT(width % multiple == 0);
	ASSERT(paddedWidth >= width);
	ASSERT(threadNum < numThreads);
	if(height > 1) [[likely]] {

		RAND_ACC_IT resultCellsStart = it + oskar::multiply_and_divide(threadNum, width / multiple, numThreads) * multiple;
		RAND_ACC_IT resultCellsEnd = it + oskar::multiply_and_divide(threadNum + 1, width / multiple, numThreads) * multiple;
		if(resultCellsStart != resultCellsEnd) [[likely]] {
			RAND_ACC_IT sourceCellsStart = resultCellsStart + (height - 1) * paddedWidth;
			RAND_ACC_IT sourceCellsEnd = resultCellsEnd + (height - 1) * paddedWidth);
			while(sourceCellsStart != resultCellsStart) {

				combine_func(sourceCellsStart, sourceCellsEnd, resultCellsStart);

				sourceCellsStart -= paddedWidth;
				sourceCellsEnd -= paddedWidth;
			}
		}
	}
}


namespace game_engine {
	void entity_system::think_step_deletion() {

		// Zero the placeholder entity's deletion flag since we can't allow it to be deleted
		constexpr unsigned long long notPlaceHolderMask = ~(((unsigned long long) 1) << (placeholder_entity_id % std::numeric_limits<unsigned long long>::digits));
		entity_deletion_flags[placeholder_entity_id / std::numeric_limits<unsigned long long>::digits] &= notPlaceHolderMask;




		OBS!!!! DEN HÄR FUNTKIONEN (think_step_deletion) BEHÖVER DU KOLLA PÅ... VERKAR VARA RÄTT GAMMALT TÄNK


		entity_count numEntitiesBeingDeleted = 0;

		entity_id firstFlagEntityId = 0;
		for(std::atomic<unsigned long long>& atomicFlagInt : entity_deletion_flags) {
			const unsigned long long flags = atomicFlagInt.exchange(0, std::memory_order_relaxed);
			if(flags) [[unlikely]] {
				for(std::size_t i = 0; i != std::numeric_limits<unsigned long long>::digits; ++i) {
					if((flags & (((unsigned long long) 1) << i)) != 0) {
						const entity_id entityId = firstFlagEntityId + i;
						ids_of_ents_to_delete[numEntitiesBeingDeleted++] = entityId;

						offsets_to_next_by_manager_id
					}
				}
			}
			firstFlagEntityId += std::numeric_limits<unsigned long long>::digits;
		}
		if(numEntitiesBeingDeleted) {
			++last_deletion_point;
		}

		for(component_slot_count slotNumber = 0; slotNumber != num_component_slots; ++i) {
			std::fill(num_instance_ids_in_lists_by_manager_id.get(), num_instance_ids_in_lists_by_manager_id.get() + component_manager_pointers_by_id.size(), 0);
			for(entity_id entId : ids_of_ents_to_delete) {
				const unique_component_combination_id uccId = ucc_ids_by_entity_id[entityId];
				const component_manager_short_id componentManagerShortId = component_manager_short_ids_by_slot_and_ucc_id[std::size_t(slotNumber) * num_uccs_allocated_for + uccId];

			}
		}


		//
		deletion_points_by_entity_id[entityId] = thisDeletionPoint;
		ucc_ids_by_entity_id[entityId] = 0000000000000000000000000000000000000000000000000000000000000000;



		// Delete entities and zero entity deletion flags
		// This could probably be parallelized, but I'm not sure if it's worth the effort unless we see scenarios where hundreds of entities are being deleted simulatenously
		entity_id firstFlagEntityId = 0;
		deletion_point thisDeletionPoint = last_deletion_point + 1;
		for(std::atomic<unsigned long long>& atomicFlagInt : entity_deletion_flags) {
			const unsigned long long flags = atomicFlagInt.exchange(0, std::memory_order_relaxed);
			if(flags)[[unlikely]]{
				last_deletion_point = thisDeletionPoint;
				for(std::size_t i = 0; i != std::numeric_limits<unsigned long long>::digits; ++i) {
					if((flags & (((unsigned long long) 1) << i)) != 0) {

						entity_id entityId = firstFlagEntityId + i;
						deletion_points_by_entity_id[entityId] = thisDeletionPoint;
						ucc_ids_by_entity_id[entityId] = 0000000000000000000000000000000000000000000000000000000000000000;


						const unique_component_combination_id uccId = ucc_ids_by_entity_id[entityId];
						for(component_slot_count slotNumber = 0; slotNumber != num_component_slots; ++slotNumber) {

							const component_manager_short_id componentManagerShortId = component_manager_short_ids_by_slot_and_ucc_id[std::size_t(slotNumber) * num_uccs_allocated_for + uccId];
							//component_manager& componentManager = *component_manager_pointers_by_slot_and_short_id[std::size_t(slotNumber) * num_component_slots + componentManagerShortId];

							const component_instance_id componentInstanceId = ci_ids_by_slot_and_entity_id[std::size_t(slotNumber) * num_entities_allocated_for + entityId];
							componentManager.enqueue_deletion(componentInstanceId);
						}
					}
				}
			}
			firstFlagEntityId += std::numeric_limits<unsigned long long>::digits;



			for(component_manager& cm : component_manager_refs_by_long_id) {
				cm.process_deletion_queue();
			}
		}
	}


	void entity_system::worker_think() {

		{ // Deletion flag merging
			
			constexpr std::size_t flag_ints_per_iteration = std::max(1, entity_deletion_flags_row_byte_alignment / sizeof(unsigned long long));
			combine_rows(entity_deletion_flags.get(), num_ints_used_for_entity_deletion_flags_row, num_ints_used_for_entity_deletion_flags_row, num_workers, [](unsigned long long* sourceStart, unsigned long long* sourceEnd, unsigned long long* resultStart) {
				// ENABLE THIS FOR GCC: sourceStart = __builtin_assume_aligned(sourceStart, entity_deletion_flags_row_byte_alignment);
				while(sourceStart != sourceEnd) {
					bool containsSetFlags = false;
					for(std::size_t i = 0; i != flag_ints_per_iteration; ++i) {
						containsSetFlags = containsASetFlag || sourceStart[i] != 0;
					}
					if(containsSetFlags) {
						for(std::size_t i = 0; i != flag_ints_per_iteration; ++i) {
							resultStart[i] |= sourceStart[i];
						}
						for(std::size_t i = 0; i != flag_ints_per_iteration; ++i) {
							sourceStart[i] = 0;
						}
					}
					resultStart += flag_ints_per_iteration;
					sourceStart += flag_ints_per_iteration;
				}

			}, workerNum, numWorkers, flag_ints_per_iteration);
			
			mark_as_ready(myWorkerData.readyVar);
		}

		wait for all





			NEW CODE TO USE::::::::::: pseudo-code: should use std::experimental::simd (or Vc (Master branch))

			vector<simd> infoNums;
			vector<simd_mask> deletionBlocks;
			std::size_t dbNum = 0;
			simd_mask setMask = false;
			do {
				simd_mask db = deletionBlocks[dbNum] ^ setMask;
				if(db.none_set()) {
					++dbNum;
					setMask = false;
				} else {
					NJONONONONONOONONONONO WE NEED TO WRITE COMPONENT_IDs NOT FUCKING ENTITY_IDS
					int firstSet = db.find_first_set();
					setMask = db;
					setMask &= uccNums[dbNum] == uccNums[dbNum][firstSet];
					for(slot numbers this thread is responsible for) {
						manager_id manId = manager_ids_by_slot_and_ucc[slotNumber][uccNums[dbNum][firstSet]];
						simd_mask* output = managers[manId]->newDeletes;
						output = manId == placeholder_manager ? somebullshitarray : output;
						*output = setMask;
					}
				}
			} while(dbNum != deletionBlocks.size());






		{


			// divide work by slots here
			std::size_t minSlotNumber = multiply_and_divide(num_component_slots, worker_number, worker_count);
			std::size_t nextWorkersMinSlotNumber = multiply_and_divide(num_component_slots, worker_number + 1, worker_count);

			if(minSlotNumber != nextWorkersMinSlotNumber) {
				unsigned long long macroMask = 0;

				const unsigned long long* mergedFlagIntIt = entity_deletion_flags.get();
				// ENABLE THIS FOR GCC: mergedFlagIntsStart = __builtin_assume_aligned(entity_deletion_flags.get(), entity_deletion_flags_row_byte_alignment);
				const unsigned long long* const mergedFlagIntsEnd = entity_deletion_flags.get() + num_ints_used_for_entity_deletion_flags_row;

				while(mergedFlagIntIt != mergedFlagIntsEnd) {
					// TODO: Rewrite a little to enable autovectorization
					if(*mergedFlagIntIt & macroMask == 0) [[likely]] {
						macroMask = 0;
						++mergedFlagIntIt;
					} else {
						const std::size_t flagNumber = oskar::count_trailing_zeroes(*mergedFlagIntIt & macroMask);
						macroMask |= ((unsigned long long) 1) << flagNumber;
						const entity_id entId = (mergedFlagIntsEnd - mergedFlagIntIt) * std::numeric_limits<unsigned long long>::digits + flagNumber;

						const unique_component_combination_id uccId = ucc_ids_by_entity_id[entId];
						for(std::size_t slotNumber = minSlotNumber; slotNumber != nextWorkersMinSlotNumber; ++slotNumber) {
							const component_manager_id managerId = component_manager_ids_by_slot_and_ucc_id[slotNumber * num_uccs_allocated_for + uccId];
							component_manager& manager = component_manager_pointers_by_id[managerId];
							const component_instance_id instanceId = ci_ids_by_slot_and_entity_id[slotNumber * num_entities_allocated_for + entId];

							manager.queueDeletion(instanceId);
						}
					}
				}
			}
		}

		wait for all

		{
			use some shared atomic integer
			and loop
			{
				component_manager_pointers_by_id[cmId].carry_out_queued_deletions();
			}
			
		}
	}
	


}

