
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
#include "shortmap.h"
#include "compilation_target_info.h"
#include "small_set.h"
#include "boost/container/small_vector.hpp"

template<class ptr> using unowned = ptr;

template<std::uintmax_t max_val> using big_enough_fast_uint = std::conditional_t<
	max_val <= std::numeric_limits<std::uint_fast8_t>::max(), std::uint_fast8_t, std::conditional_t<
	max_val <= std::numeric_limits<std::uint_fast16_t>::max(), std::uint_fast16_t, std::conditional_t<
	max_val <= std::numeric_limits<std::uint_fast32_t>::max(), std::uint_fast32_t, std::conditional_t<
	max_val <= std::numeric_limits<std::uint_fast64_t>::max(), std::uint_fast64_t, std::conditional_t<
	max_val <= std::numeric_limits<unsigned short>::max(), unsigned short, std::conditional_t<
	max_val <= std::numeric_limits<unsigned int>::max(), unsigned int, std::conditional_t<
	max_val <= std::numeric_limits<unsigned long>::max(), unsigned long, std::conditional_t<
	max_val <= std::numeric_limits<unsigned long long>::max(), unsigned long long, std::uintmax_t
>>>>>>>>;
template<std::uintmax_t digits> using fast_uint_with_digits = big_enough_fast_uint<std::uintmax_t(1) << (int(std::min((std::uintmax_t) std::numeric_limits<std::uintmax_t>::digits, digits)) - 1)>;


constexpr std::size_t max_entities = (1 << 16) - 1 - 7; // -7 is so component_combination_id can fit into two bytes on common architectures
constexpr std::size_t max_component_combinations = max_entities;

constexpr std::size_t max_references_to_an_entity = (1 << 16) - 1;
using entity_reference_count_nonatomic = big_enough_fast_uint<max_references_to_an_entity>;

using component_instance_id = big_enough_fast_uint<max_entities>;
using entity_count = big_enough_fast_uint<max_entities>;
using entity_id = big_enough_fast_uint<max_entities>;
using component_combination_id = big_enough_fast_uint<max_component_combinations + std::numeric_limits<unsigned char>::digits - 1>; // The + digits - 1 is for the sake of avoiding (id + component_combination_test_bit_offset) overflowing
constexpr component_combination_id null_component_combination_id = 0;

struct alignas(std::max({alignof(component_combination_id), alignof(std::atomic<entity_reference_count_nonatomic>), alignof(component_instance_id), alignof(entity_id) }))
	entity_prologue
	{
	component_combination_id combination_id = null_component_combination_id;
	std::atomic<entity_reference_count_nonatomic> reference_count = 0;
};


template<class F> class call_on_destruction {
	private:
		F func;
		bool enabled = true;
	public:
		constexpr call_on_destruction(F&& f) : func(std::forward(f)) {}
		call_on_destruction(const call_on_destruction&) = delete;
		call_on_destruction& operator=(const call_on_destruction&) = delete;

		constexpr void disable() noexcept {
			enabled = false;
		}

		constexpr void enable() noexcept {
			enabled = true;
		}

		constexpr ~call_on_destruction() noexcept(noexcept(func())) {
			if(enabled) {
				func();
			}
		}
};

template<class A, class B> auto if_throwing(A&& a, B&& b) {
	call_on_destruction cod(std::forward(b));
	auto result = a();
	cod.disable();
	return result;
}


class component;


class too_many_references_to_an_entity : public std::runtime_error {
	private:
		entity_id which_entity_id;
	public:
		explicit too_many_references_to_an_entity(entity_id entityId) : std::runtime_error("Too many references to an entity."), which_entity_id(entityId) {}

		entity_id whichEntity() const noexcept {
			return which_entity_id;
		}
};

class context;


class component_set {
	public:
		
};
class normalised_component_set { // With all dependencies and no duplicates

};

class entity_system {
	friend class context;
	private:
		using entity_raw_storage = std::aligned_storage<std::hardware_destructive_interference_size, std::hardware_destructive_interference_size>::type;
		std::vector<std::unique_ptr<component>> components;
		std::vector<entity_raw_storage> entities_data;
		entity_count num_entities;
		entity_count num_entities_allocated_for;
		std::size_t bytes_per_entity = 0;

		struct alignas(std::hardware_destructive_interference_size) thread_data {
			entity_count free_entities = 0;
			entity_id first_free_entity_id = -1;
		};
		std::vector<thread_data> per_thread_data;
	
		static constexpr std::size_t calculate_bytes_per_entity(std::size_t numComponentSlots) {
			constexpr std::size_t prologueSize = sizeof(entity_prologue);
			
			constexpr std::size_t nextFreeEntityIdSize = sizeof(entity_id); // Size requirement for unused entity space (minus prologue)
			const std::size_t componentInstanceIdsSize = numComponentSlots * sizeof(component_instance_id); // Size requirement for in-use entity (minus prologue)

			return prologueSize + std::max(nextFreeEntityIdSize, componentInstanceIdsSize);
		}

		unowned<entity_prologue*> get_prologue_pointer(entity_id entityId) {
			char* firstByteOfPrologue = ((char*) entities_data.data()) + entityId * bytes_per_entity;
			return unowned<entity_prologue*>((entity_prologue*) firstByteOfPrologue);
		}
		unowned<const entity_prologue*> get_prologue_pointer(entity_id entityId) const {
			const char* firstByteOfPrologue = ((const char*) entities_data.data()) + entityId * bytes_per_entity;
			return unowned<const entity_prologue*>((const entity_prologue*) firstByteOfPrologue);
		}
		
		component_combination_id get_component_combination_id(entity_id entityId) const {
			get_prologue_pointer(entityId)->combination_id;
		}

		void increase_reference_counter(entity_id entityId, std::vector<entity_id>& deletionSuggestionQueue) {
			auto& refCount = get_prologue_pointer(entityId)->reference_count;
			if(refCount.fetch_add(1, std::memory_order_relaxed) >= max_references_to_an_entity) {
				decrease_reference_counter(entityId, deletionSuggestionQueue);
				throw too_many_references_to_an_entity(entityId);
			}
		}
		void decrease_reference_counter(entity_id entityId, std::vector<entity_id>& deletionSuggestionQueue) {
			unowned<entity_prologue*> prologue = get_prologue_pointer(entityId);

			bool done = false;

			// Optimised section for architechtures where memory_order_relaxed operations are likely to be faster than
			// memory_order_acq_rel operations
			// TODO: Measure performance impact
			if constexpr(oskar::compilation_target_info::memory_order_acq_rel_likely_generates_fence) {
				if(prologue->combination_id != null_component_combination_id) {
					prologue->reference_count.fetch_sub(1, std::memory_order_relaxed);
					done = true;
				}
			}

			if(!done && prologue->reference_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
				deletionSuggestionQueue.push_back(entityId);
			}
		}

		component_combination_id find_component_combination_id_for_set() const {
			
		}

		void queue_entity_creation(std::size_t num, component_set cset) {
			if(component_combination_id ccid = find_component_combination_id_for_set(cset)) {
				
			} else {

			}
		}
};



template<class... Types> class static_entity_prototype {
	private:
		std::vector<unowned<component*>>
};
class dynamic_entity_prototype {
	private:
		shortmap<unowned<component*>, std::function<void()>> components;

	public:
		template<class ComponentType, class... SpawnParams> std::enable_if_t<
			std::is_convertible_v<ComponentType*, component*>
		>
		add(std::remove_cv_t<ComponentType>* comp, SpawnParams&&... params) {
			auto cmpIt = std::find(components.begin(), components.end());
			components.insert(cmpIt, comp);
			if_throwing(
				[&] {
					spawnCallers.insert(spawnCallers.begin() + , std::bind(&ComponentType::spawn, std::forward(params)...));
				},
				[&] { components.pop_back(); }
			);
		}
};

class context {
	private:
		entity_system& ent_sys;
		entity_system::thread_data& this_threads_entity_system_data;
		std::size_t thread_num;

	public:
		
};
template<class component> class component_accessor {
	private:
		std::observer_ptr<const char[]> entities;
		std::observer_ptr<const char[]> entities_offset_by_component_offset;
		entity_count num_entities_allocated_for;
		std::size_t bytes_per_entity = 0;
		const char* component_varieties_table_ptr;

	public:

		std::optional<component_instance_id> get_component_instance_id_for_entity(entity_id eid) {
			std::optional<component_instance_id> result;
			if(eid < num_entities_allocated_for) {
				const char* variet235ij234j5Bytes = entities_offset_by_component_offset[bytes_per_entity * eid];
				const char* componentInstanceIdBytes = entities_offset_by_component_offset[bytes_per_entity * eid];
				component_varieties_table_ptr
			}
		}
		
};


class component_base_base {
	private:
		oskar::small_set<component_instance_id, 4> combination_ids;
		
	public:
		virtual struct { entity_count remaining; component_instance_id start; component_instance_id end; } allocate(entity_count n);
		virtual void deallocate(ents_list e);
		virtual void deallocate(components_list e);


		
		unowned<const char[]> component_combination_set;
		big_enough_fast_uint<std::numeric_limits<unsigned char>::digits - 1> component_combination_test_bit_offset; // An optimisation so we can reuse test bytes about 8 times as often

		bool is_part_of_combination(component_combination_id id) const {
			return combination_ids.contains(id);
		}
		/*bool is_part_of_combination(component_combination_id id) const {
			constexpr std::size_t bpb = std::numeric_limits<unsigned char>::digits;

			component_combination_id idPlusBitOffset = id + component_combination_test_bit_offset;

			char testByteValue = *(component_combination_test_bytes_ptr + (idPlusBitOffset / bpb));

			return ((testByteValue >> (idPlusBitOffset + component_combination_test_bit_offset) % bpb) & 1) == 1;
		}*/
};
template<class component> class component_base : public component_base_base {
	public:
		
};

class physics : public component_base {
	std::vector<phys_data> data;
};


