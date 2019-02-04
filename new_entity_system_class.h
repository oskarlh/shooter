

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

