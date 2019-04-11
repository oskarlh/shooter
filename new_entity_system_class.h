#include <new>
#include <functional>

#if __cpp_lib_experimental_parallel_simd >= 201803
	#include <experimental/simd>
#endif



let's divide into blocks:
we have the following steps:
base_start
physics_calc
car_gear_handling
and these can all be split up by segment. Then one block is
base_start:the_segments_for_my_worker
physics_calc:the_segments_for_my_worker(which is a subset of base_start:the_segments_for_my_worker)
car_and_gear_handling:the_segments_for_my_worker(which is a subset of base_start : the_segments_for_my_worker)
These can be grouped together without thread synchronization
BUT
We can also flag to the other threads after we are done with each step, so other threads can start working on the next part


HMMMM actually I don''t think we should go for optimisations: we should go for simplicity first.
What we need to do as a bare minimum is to figure out how to calculate:
	For each step, which other steps must be completed first. Those are the ones we need signals from. Any further optimisations (which are optional) can use this as a starting point
	


struct task {
	some_function_object_to_be_run;
	task*[] tasks_we_depend_on;
};



class process_list {
	 
	struct per_worker_list_of_things_to_do {
		struct node {
			run_these_in_any_order;
		};
	};
	struct node {
		system* sysPtr;
		std::atomic<std::size_t> dependency_completion_count;
		std::unique_ptr<node*[]> blocks;
		std::unique_ptr<node*[]> blocked_by;
	};

	nodes must be organised in such a way that writes happen after every other read of the same var
	We should try to put writes as early as possible


	std::vector<node> blocking_nodes;
	
	bool canBeMovedBeforeAdjacentNode(nodeA, nodeB) const {
		return !has_common_element(nodeA->writes, nodeB->reads) && !has_common_element(nodeA->reads, nodeB->writes);
	}
	bool canRunSimultaneously(nodeA, nodeB) const {
		return !has_common_element(nodeA->writes, nodeB->reads) && !has_common_element(nodeA->reads, nodeA->writes) && !test_every_element_in_common(nodeA->writes, nodeB->writes, column_writes_are_atomic);
	}
	bool canBeRunByMultipleThreads(nodeA) const {
		return all(nodeA->writes, [](const node&) { return node.isolated_to_self || column_writes_are_atomic(node); });
	}

};
addProcess(physicsSys.thinkProcess());



void register_my_mods_components(registry& reg) {
	carComp.registerComponent(reg);

}

struct car_component {
	name;

	columnA;
	columnB;
	columnC;
};

class car_module {
	component carComponent;
	//system carGearingSystem;
	process carThink;

	car_module(game& g) : carComponent(g) {
		carComponent.cGear.set();
	}
};
class tractor_module {
	component tractorComponent;
	think mainThink;

	void mainThinkFunc(physics_main_system physMain, car_gearing_system carGearing, ents) {
		carGearing.gearUp(randomEnt());

	}

	tractor_module(game& g) : tractorComponent(g) {
		mainThink = think(
			physicsMainSystemDescription,
			carGearingSystemDescription,

			mainThinkFunc,
			carMainThink
		);

	}
};

struct car_gearing_system {
	car_component& carComp;
	column_access_descriptor gearsStageAccess(physicsAccess, column_access_descriptor(car_component.gears, rw, 4), column_access_descriptor(car_component.lights, read));
	
	car_gearing_system() {
		gearsStageAccess.lock();
	}
};


class car_component {
	private:
		think_id mainThinkId;
	public:
		const think& mainThink() const {
			
			return mt;
		}
		void registerComponent(registry& reg, entity_component& entComp, physics_component& physComp) {
			mainThinkRef = addTink(std::move(mainThink));
			addThink(doMainThink);
			car_component& cc = reg.add([]() { return car_component() });
			reg.addThink(cc.think, entComp.getFirstThink());

			column_access_descriptor gearsStageAccess(physicsAccess, column_access_descriptor(car_component.gears, rw, 4), column_access_descriptor(car_component.lights, read));

			reg.addStage("gears", gearsStageFunc, gearsAccess);
		}
		virtual void think() {
			{
				market_after_
				auto access(co_await requireAccess(potato_write, blarg_rw_self));
				physics_object_mover physicsObjectMover(lock.get<physics_write_access>());
				physicsObjectMover.move(some_ents, 0, 0, 0);
				auto pos = physicsObjectMover.getPosition(some_ents);

			}
		}
		virtual void on_unload() const {

		}

};

class accessor_requirements final {
	addDependency(accessor_descriptor) {

	}
};
class accessor {
	accessor(accessor_requirements& ar) {
		ar.lock();
	}
};
class car_accessor : public accessor {
	physics_accessor phys_acc;
	basic_accessor basic_acc;

	car_accessor(accessor_requirements& ar) : accessor(acc) {
		phys_acc = get<physics_accessor>(0);
		basic_acc = get<basic_accessor>(1);
	}

	float getSpeed(carId) {

		phys_acc.getVelocity(carId);
	}
};

auto carPhys = car_physics_accessor;
carPhys.getSpeed(myCar)

class spawn_system {
	public:
		void call_after_first_spawn
};

class process_description {
	public:
		virtual std::optional<component_manager&> activated_by() {
		}
};

process A
	reads : (physics, self_only), (monsters, others)
	readsSelf: own
	readsSelf:

any reads can happen simultaneously with any other reads
reads cannot happen simulataneously as writes, and must be in order
writes cannot happen simultaneously as other writes, but dont have to be in order
mins / maxs / sums, other associative write operations are special : they can be unordered
Allowed restrictions:
	a certain write must happen before a certain read?
	Maybe we should implement this through "data versions": initial, after_recomputing, ...
	
	
class column {
	alters = const_after_spawn / updates_rarely / always_always_or_often;
	

	const_after_spawn <- no writesToX at all
	updates_rarely < -writesToOthers
	updates_always_or_often <- writesToSelf
};

class process_description {
	std::vector<column_ref> readsFromSelf;
	std::vector<column_ref> readsFromOthers;
	std::vector<column_ref> writesToSelf; every / rarely
	std::vector<column_ref> writesToOthers; every / rarely
	std::vector<process_description&> mustHappenAfter;
};


Column:
	Alters: const_after_spawn / updates_rarely / always_always_or_often
	UpdateMethod : exclusive_replace, exclusive_update, shared_replace(either through atomics or through a per - worker queue)

Update :
	Reads from the following columns : physics_storage.position(self), button_storage::state_read(other)
	Writes to the following columns : physics_storage.position(self, every), button_storage::state_write(rarely)
	Must happen after : position_system::humbug






// 32 is a good size because at the time of writing it looks like it'll be the minimum value of
// std::experimental::parallelism_v2::simd_abi::max_fixed_size<T> for any T,
// see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/n4793.pdf#section.9.3
// 32 bytes also means there's a possibility of doing 31 completely unnecessary calculations
// when the number of entities of a certain type is a multiple of 32 plus 1,
// and that's a reasonable amount of waste, considering that we want this game engine to
// be optimised for large numbers of entities
constexpr std::size_t elements_per_segment = 32;
constexpr static std::size_t segment_size_factor = std::hardware_destructive_interference_size;



template<class ArrayType> class component_array_accessor {
	private:
		std::reference_wrapper<const ArrayType> source;
		std::reference_wrapper<ArrayType> destination;
		std::reference_wrapper<bool> alterationFlag;

	public:
		component_array_accessor(const ArrayType& src, ArrayType& dst, bool& altFlg) : source(src), destionation(dst), alterationFlag(altFlg) { }

		const ArrayType& read() const {
			return source;
		}

		ArrayType& write() const {
			altFlg = true;
			return destination;
		}

		void advance() {
			source = *(&source.get() + 1);
			destination = *(&destination.get() + 1);
			alterationFlag = *(&alterationFlag.get() + 1);
			alterationFlag.get() = true;
		}
};

template<class ValueType, class ArrayType> class component_value_accessor {
	private:
		component_array_accessor<ArrayType> array_accessor;
		std::size_t element_offset; // TODO: Consider using a narrower type and benchmark / analyze code

	public:
		component_value_accessor(component_array_accessor<ArrayType> arrAcc, std::size_t elOffset) : array_accessor(arrAcc), element_offset(elOffset) { }

		const ValueType& read() const {
			return array_accessor.read()[element_offset];
		}

		ValueType& write() const {
			return array_accessor.write()[element_offset];
		}
};

template<class ArrayType> class component_array_accessor_iterator_base {
	public:
		using value_type = ArrayType;
		using reference = ArrayType&;
		using pointer = ArrayType*;
};


template<bool IsSigned, std::size_t Digits> class component_integer_column {
	static_assert(Digits >= 1 && Digits + IsSigned < 64); // 64 (or more) bit integers are the biggest we're promised by the standard (C++17)
	public:
		constexpr static std::size_t digits = Digits;
		using value_type = oskar::least_uint_with_digits<Digits>;
		using fast_intermediate_value_type = oskar::fast_uint_with_digits<Digits>;

		#if __cpp_lib_experimental_parallel_simd >= 201803
			using simd_abi = std::experimental::simd_abi::fixed_size<elements_per_segment>;
			using simd = std::experimental::simd<value_type, simd_abi>;
			using simd_mask = simd::mask_type;
			constexpr static std::size_t value_array_alignment = std::experimental::memory_alignment_v<value_type, simd>;
		#else
			constexpr static std::size_t value_array_alignment = alignof(std::array<value_type, elements_per_segment>);
		#endif

		using value_array_type = alignas(value_array_alignment) value_type[elements_per_segment];

		constexpr static std::size_t bytes_per_segment = sizeof(value_array_type);
		constexpr std::size_t bytes_between_segments = oskar::divide_integer_rounding_away_from_zero(bytes_per_segment, segment_size_factor) * segment_size_factor;
};

class component_storage {
	private:


		std::unique_ptr<std::byte[]> all_bytes;

		struct column_data {
			std::byte* first_byte = nullptr;
			std::size_t bytes_between_segments;
		};
		std::vector<column_data> cols;
	public:

};


auto seg = storage.segment(1);
position_column_type::simd positionSimd(storage.segment(1).get(positionColumn));


positionSimd.write_to(storage.segment(1).outAddress(positionColumn));









// Below might be nonsense, consider discarding





class vector_of_component_ref_1d {
	std::vector<component_instance_segment_id>;
	std::vector<component_instance_id>;
};

component_ref() {
	segment_id;
	instance_id;
}
component_ref
get_same_ent_ref(global_segment_id, instance_id) {

}



class ent_sys {
};

class ucc {
	struct ucc_specific_segment_info {

	};
	std::vector<ucc_specific_segment_info> my_segments;

	
};

class component_manager {
	entity_system& ent_sys;
	common_component_manager& common_manager;
	component_slot_number slot_number = 0;
	component_instance_count num_instances = 0;

	std::vector<common_component_manager::segment_specific_data*> common_segment_data_ptr_by_private_segment_id;
	
	std::optional<segment_id> get_segment_id_for_possible(segment_id localId, component_manager& otherComponentManager) const {
		...
	}
	segment_id get_segment_id_for_known_to_exist(segment_id localId, component_manager& otherComponentManager) const {
		return common_segment_data_ptr_by_private_segment_id[localId]->segment_ids_by_component_manager_slot[otherComponentManager.slot_number];
	}

	virtual void delete_instance(----) = 0;
	virtual void create_instance(id, segment_id) = 0;
	virtual segment_id create_segment();
	
	void getInfo() {
		return 4;
	}

	void defrag() {
		cccccccccc
	}
	void think_for_segments(a) {
		global_to_local[a];
	}
};

class common_component_manager {
	struct segment_specific_data {
		std::vector<segment_id> segment_ids_by_component_manager_slot;
	};
	std::vector<segment_specific_data> segment_spec;
};

