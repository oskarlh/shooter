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


	void entity_system::worker_think(worker_data& workerData) {

		{
			// Deletion flag merging
			// Done vertically in order to prevent problems when two workers have flagged the same entity for deletion
			
			const deletion_flags_block* currentBlockOfFirstRowIt = entity_deletion_flags.get() + oskar::multiply_and_divide(workerData.worker_number, num_blocks_per_entity_deletion_flags_row, num_workers);
			const deletion_flags_block* const endBlockOfFirstRowIt = entity_deletion_flags.get() + oskar::multiply_and_divide(workerData.worker_number + 1, num_blocks_per_entity_deletion_flags_row, num_workers);
//			const deletion_flags_block* const lastIt = firstRowCurrentBlockIt + num_blocks_per_entity_deletion_flags_row * (num_workers - 1);
			
			
			entity_id firstPossibleEntIdThisIteration = (currentBlockOfFirstRowIt - entity_deletion_flags.get()) * entity_deletion_flags_block_size_and_alignment * std::numeric_limits<unsigned char>::digits;
			while(currentBlockOfFirstRowIt != endBlockOfFirstRowIt) {
				alignas(std::max(alignof(deletion_flags_block), alignof(unsigned long long))) deletion_flags_block sumOfFlags{};
				const deletion_flags_block* currentBlockIt = currentBlockOfFirstRowIt;
				const deletion_flags_block* const currentBlockOfLastRowIt = currentBlockOfFirstRowIt + num_blocks_per_entity_deletion_flags_row * num_workers;

				#ifdef __has_builtin
					#if  __has_builtin(__builtin_assume_aligned)
						currentBlockIt = __builtin_assume_aligned(currentBlockIt, entity_deletion_flags_block_size_and_alignment);
					#endif
				#endif
				while(currentBlockIt <= currentBlockOfLastRowIt) {
					const deletion_flags_block& currentBlock = *currentBlockIt;
					for(std::size_t i = 0; i != currentBlock.size(); ++i) {
						sumOfFlags[i] |= currentBlock[i];
					}
					currentBlockIt += num_blocks_per_entity_deletion_flags_row;
				}
				


				for(std::size_t i = 0; i != sumOfFlags.size(); ++i) {
					std::byte part = sumOfFlags[i];
					while(part != std::byte(0)) [[unlikely]] {
						const std::size_t bitNum = oskar::count_trailing_zeroes(part);
						part ^= deletion_flags_block::value_type(1) << bitNum;
						const entity_id entId = bitNum + firstPossibleEntIdThisIteration + i * std::numeric_limits<unsigned char>::digits;
						const unique_component_combination_id uccId = ucc_ids_by_entity_id[entId];
						entity_count& num_deletes = workerData.num_deletes_by_ucc[uccId];
						const entity_count deletes_offset = workerData.deletes_offset_by_ucc[uccId];
						
						workerData.ents_to_delete[deletes_offset + num_deletes] = entId;
						num_deletes++;
						
						OPKWEPOKROPWKER FORTSÄTT HÄR!!!! NU HAR VI ETT ENTITY_ID SOM DU VET BEHÖVS TAS BORT!!!
					}
				}

				++currentBlockOfFirstRowIt;
				firstPossibleEntIdThisIteration += entity_deletion_flags_block_size_and_alignment * std::numeric_limits<unsigned char>::digits;
			}
		}
		{
			// Zero this worker's deletion flags
			deletion_flags_block* blockToZeroIt = entity_deletion_flags.get() + num_blocks_per_entity_deletion_flags_row * threadNum;
			deletion_flags_block* endIt = blockToZeroIt + num_blocks_per_entity_deletion_flags_row;

			#ifdef __has_builtin
				#if  __has_builtin(__builtin_assume_aligned)
					blockToZeroIt = __builtin_assume_aligned(blockToZeroIt, entity_deletion_flags_block_size_and_alignment);
				#endif
			#endif
			while(blockToZeroIt != endIt) {

				deletion_flags_block& block = *blockToZeroIt;

				// Before we zero, we find out if the flags are not already zero
				// We do this because we don't want our flags bytes to be invalidated in other threads' cache unnecessarily
				bool needsZeroing = false;
				for(std::size_t i = 0; i != block.size(); ++i) {
					needsZeroing = needsZeroing || block[i] != 0;
				}
				if(needsZeroing) {
					for(std::size_t i = 0; i != block.size(); ++i) {
						block[i] = 0;
					}
				}
				++blockToZeroIt;
			}
		}

		{
			combine_rows(entity_deletion_flags.get(), num_ints_used_for_entity_deletion_flags_row, num_ints_used_for_entity_deletion_flags_row, num_workers, [](unsigned long long* sourceStart, unsigned long long* sourceEnd, unsigned long long* resultStart) {
				
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

