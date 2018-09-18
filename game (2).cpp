#include <iostream>

#include "engine.h"

class game_rules : public game_engine::no_rules {
	public:
		virtual void beforeThink() {

		}
};
class game_ui : public game_engine::no_ui {

};

class game {
	private:
		game_engine::engine<game_rules, game_ui> ge;
		physics_component physComp;
		vehicle_component vehicleComp;

	public:
		game() {
			ge.registerComponent(physComp);
			ge.registerComponent(vehicleComp);
			ge.registerRules(gameRules);
		}
		bool work() {
			return ge.work();
		}
};

int main() {
	game g;
	while(g.work());
	game_engine::entity_system esys;
	std::cout << "Success";
	std::cin.get();
	return 0;
}






/*



template<class... Types> class static_entity_prototype {
	private:
		//std::array<oskar::unowned<component*>> 
};
class dynamic_entity_prototype {
	private:
		shortmap<oskar::unowned<component*>, std::function<void()>> components;

	public:
		template<class ComponentType, class... SpawnParams> std::enable_if_t<
			std::is_convertible_v<ComponentType*, component*>
		>
		add(std::remove_cv_t<ComponentType>* comp, SpawnParams&&... params) {
			auto cmpIt = std::find(components.begin(), components.end());
			components.insert(cmpIt, comp);
			try {
				spawnCallers.insert(spawnCallers.begin() + , std::bind(&ComponentType::spawn, std::forward(params)...));
			}
			catch(...) {
				components.pop_back();
				throw;
			}
		}
};

class physics : public component_base {
	std::vector<phys_data> data;
};


*/
