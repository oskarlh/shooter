#ifndef GAME_ENGINE_ENTITY_SYSTEM_WORKER_HPP
#define GAME_ENGINE_ENTITY_SYSTEM_WORKER_HPP


namespace game_engine {
	class entity_system_worker {
		private:
			entity_system& ent_sys;
		
		public:
			void work();
	};

}


#endif
