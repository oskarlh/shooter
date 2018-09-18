#include <iostream>

#include "engine.h"

class game_rules {
public:
	void beforeThink() {

	}
	void afterThink() {

	}
};

class game_ui {
public:
	virtual void beforeThink() {

	}
};


class simulation {
	game_engine::entity_system es;
};
class view {

};

#include <codecvt>
#include <stdexcept>

class utf8_codec {
	private:
		class destructible_codecvt_facet : public std::codecvt<char32_t, char, std::mbstate_t> { };
		destructible_codecvt_facet cf;
	public:
		struct decoding_result {
			char32_t cp;
			const char* nextIt;
			bool partial;
			bool badBytes;
			bool ok;
		};
		struct encoding_result {
			char* nextIt;
			bool partial;
			bool badBytes;
			bool ok;
		};

		[[nodiscard]] decoding_result try_to_decode_utf8_cp(const char* it, const char* endIt) {
			decoding_result res;
			char32_t* toNext;
			std::mbstate_t state{};
			auto inReturnValue = cf.in(state, it, endIt, res.nextIt, &res.cp, (&res.cp) + 1, toNext);
			res.partial = inReturnValue == destructible_codecvt_facet::partial;
			res.badBytes = inReturnValue == destructible_codecvt_facet::error;
			res.ok = inReturnValue == destructible_codecvt_facet::ok;
			return res;
		}

		[[nodiscard]] encoding_result try_to_encode_utf8_cp(char32_t cp, char* outIt, char* outEnd) {
			encoding_result res;
			const char32_t* fromNext;
			std::mbstate_t state{};
			auto outReturnValue = cf.out(state, &cp, (&cp) + 1, fromNext, outIt, outEnd, res.nextIt);
			res.partial = outReturnValue == destructible_codecvt_facet::partial;
			res.badBytes = outReturnValue == destructible_codecvt_facet::error;
			res.ok = outReturnValue == destructible_codecvt_facet::ok;
			return res;
		}
};

struct compress_utf8_result {
	const char* nextInIt;
	char* nextOutIt;
};

[[nodiscard]] compress_utf8_result compress_utf8(const char* inIt, const char* inEnd, char* outIt, char* outEnd)
{
	utf8_codec u8Codec;
	const char* safeOutEnd = outIt + std::min(inEnd - inIt, outEnd - outIt);
	while(outIt < safeOutEnd) [[likely]] {
		const unsigned char firstInputByte = (unsigned char) *inIt++;
		*outIt++ = (char) firstInputByte;
		if(firstInputByte >= 0b10000000) {
			--outIt;
			--inIt;
			auto [ cp, nextInIt, partial, badBytes, ok ] = u8Codec.try_to_decode_utf8_cp(inIt, inEnd);
			const bool needThreeBytes = cp >= (char32_t(1) << (6 + 8));
			if(ok && outEnd - outIt >= 2 + needThreeBytes) [[likely]] { // Enough space
				inIt = nextInIt;
				const unsigned char mostSignificant6Bits = cp >> (8 + 8 * needThreeBytes);
				*outIt++ = (char)(((((unsigned char) 0b11000000) << needThreeBytes) & 0xFF) | mostSignificant6Bits);
				*outIt = (char)(((unsigned char)(cp >> 8)) & 0xFF);
				outIt += needThreeBytes;
				*outIt++ = (char)(((unsigned char) cp) & 0xFF);
				safeOutEnd = outIt + std::min(inEnd - inIt, outEnd - outIt);
			} else [[unlikely]] {
				safeOutEnd = 0; // Force the loop to end
			}
		}
	}
	compress_utf8_result res;
	res.nextInIt = inIt;
	res.nextOutIt = outIt;
	return res;
}

struct decompress_utf8_result {
	const char* nextInIt;
	char* nextOutIt;
};
[[nodiscard]] decompress_utf8_result decompress_utf8(const char* inIt, const char* inEnd, char* outIt, char* outEnd) {
	utf8_codec u8Codec;
	const char* safeInEnd = inIt + std::min(inEnd - inIt, outEnd - outIt);
	while(inIt < safeInEnd) [[likely]] {
		const unsigned char firstInputByte = (unsigned char)*inIt++;
		*outIt++ = (char) firstInputByte;
		if(firstInputByte >= 0b10000000) {
			--outIt;
			bool inputIsThreeBytes = firstInputByte < 0b11000000;
			if(inEnd - inIt >= 1 + inputIsThreeBytes) [[likely]] { // Enough bytes
				char32_t cp = char32_t(firstInputByte & 0b00111111) << 8;
				cp |= char32_t(*inIt);
				inIt += inputIsThreeBytes;
				cp <<= inputIsThreeBytes * 8;
				cp |= char32_t(*inIt);
				++inIt;

				auto [ nextOutIt, partial, badBytes, ok ] = u8Codec.try_to_encode_utf8_cp(cp, outIt, outEnd);
				outIt = nextOutIt;
				if(!ok) [[unlikely]] {
					inIt -= 2 + inputIsThreeBytes;
					safeInEnd = 0; // Force the loop to end
				}
			} else [[unlikely]] {
				--inIt;
				safeInEnd = 0; // Force the loop to end
			}
		}
	}
	decompress_utf8_result res;
	res.nextInIt = inIt;
	res.nextOutIt = outIt;
	return res;
}

class physical_presence_component_manager : public game_engine::component_manager {
	public:

		virtual allocation_result allocate(game_engine::entity_count n) {

		}
		virtual void deallocate(game_engine::temporary_entity_reference ent) {

		}

		virtual std::string_view getUniqueNameUtf8() const {
			return u8"Oskar/PhysicalPresence/V0";
		}
};

class game {
	private:
		game_rules rules;
		game_ui ui;
		bool running = true;

		game_engine::simulation mainSim;

	public:
		game() {
			/*mainSim.registerComponent(physicalPresenceComponent);
			mainSim.registerComponent(graphicsComponent);
			mainSim.registerComponent(physicsComponent);*/
		}
		game(game&&) = delete;
		game(const game&) = delete;



		/*explicit engine(Rules&& r) : rules(r) {}
		explicit engine(Rules&& r, Ui&& u) : rules(r), ui(u) {}
		explicit engine(Rules&& r, const Ui& u) : rules(r), ui(u) {}
		explicit engine(const Rules& r) : rules(r) {}
		explicit engine(const Rules& r, Ui&& u) : rules(r), ui(u) {}
		explicit engine(const Rules& r, const Ui& u) : rules(r), ui(u) {}
		explicit engine(Ui&& u) : ui(u) {}
		explicit engine(const Ui& u) : ui(u) {}*/

		bool work() {
		/*	running = rules.preThink();

			physicsUpdates; ?
				stateChanges;
			events;
			countedEvents;

			if(!running) {
				shutDownGraphics();
			}*/
			return running;
		}
};

int main() {
	game g;
	g.work();
	//while(g.work());
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
