#ifndef OSKAR_GAME_ENGINE_ENGINE_H
#define OSKAR_GAME_ENGINE_ENGINE_H

#include "entity_system.h"
#include "useful.h"

namespace game_engine {

	class no_rules {
		void beforeThink() {

		}
		void afterThink() {

		}
	};

	class no_ui {
		void beforeThink() {

		}
		void afterThink() {

		}
	};

	template<class Rules, class Ui>
	class engine {
		private:
			Rules rules;
			Ui ui;
			entity_system es;
			bool running = true;

		public:
			engine() {
				
			}
			engine(engine&&) = delete;
			engine(const engine&) = delete;

			/*explicit engine(Rules&& r) : rules(r) {}
			explicit engine(Rules&& r, Ui&& u) : rules(r), ui(u) {}
			explicit engine(Rules&& r, const Ui& u) : rules(r), ui(u) {}
			explicit engine(const Rules& r) : rules(r) {}
			explicit engine(const Rules& r, Ui&& u) : rules(r), ui(u) {}
			explicit engine(const Rules& r, const Ui& u) : rules(r), ui(u) {}
			explicit engine(Ui&& u) : ui(u) {}
			explicit engine(const Ui& u) : ui(u) {}*/

			bool work() {
				running = rules.preThink();

				physicsUpdates;?
				stateChanges;
				events;
				countedEvents;

				if(!running) {
					shutDownGraphics();
				}
				return running;
			}
	};
}

#endif
