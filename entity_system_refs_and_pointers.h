#ifndef GAME_ENGINE_ENTITY_SYSTEM_REFS_AND_POINTERS_H
#define GAME_ENGINE_ENTITY_SYSTEM_REFS_AND_POINTERS_H

#include "useful.h"

#include "entity_system_numbers.h"
#include "entity_system_class.h"
#include "entity_system_temp_refs_and_pointers.h"

namespace game_engine {

	class entity_pointer {
		friend class entity_reference;

		private:
			entity_id eid = placeholder_entity_id;
			oskar::unowned<entity_system*> ent_sys = nullptr;

			entity_pointer(entity_id id, entity_system& esys) : ent_sys(&esys), eid(id) {
				ent_sys->increase_reference_counter(id);
			}

		public:


			constexpr entity_pointer() noexcept = default;
			constexpr entity_pointer(entity_pointer&& other) noexcept {
				ent_sys = other.ent_sys;
				eid = other.eid;
				other.eid = placeholder_entity_id;
			}
			entity_pointer(const entity_pointer& other) : ent_sys(other.ent_sys), eid(other.eid) {
				ent_sys->increase_reference_counter(eid);
			}
			entity_pointer(const temporary_entity_pointer& tmp, entity_system& esys) : ent_sys(&esys), eid(tmp.eid) {
				ent_sys->increase_reference_counter(eid);
			}
			constexpr entity_pointer(std::nullptr_t) noexcept { }

			constexpr entity_pointer& operator=(entity_pointer&& other) noexcept {
				auto old_ent_sys = ent_sys;
				auto old_eid = eid;
				ent_sys = other.ent_sys;
				eid = other.eid;
				other.eid = old_eid;
				other.ent_sys = old_ent_sys;
				return *this;
			}
			entity_pointer& operator=(const entity_pointer& other) {
				if(eid != placeholder_entity_id) {
					ent_sys->decrease_reference_counter(eid);
				}
				other.ent_sys->increase_reference_counter(other.eid);
				ent_sys = other.ent_sys;
				eid = other.eid;
				return *this;
			}

			void swap(entity_pointer& other) {
				std::swap(ent_sys, other.ent_sys);
				std::swap(eid, other.eid);
			}


			entity_id get_entity_id() const noexcept {
				return eid;
			}

			constexpr operator bool() const noexcept {
				return eid != placeholder_entity_id;
			}

			operator temporary_entity_reference() const {
				return temporary_entity_reference(eid);
			}
			constexpr operator temporary_entity_pointer() const noexcept {
				return temporary_entity_pointer(eid);
			}

			~entity_pointer() {
				if(eid != placeholder_entity_id) {
					ent_sys->decrease_reference_counter(eid);
				}
			}
	};
	inline void swap(entity_pointer& a, entity_pointer& b) {
		a.swap(b);
	}




	class entity_reference {
		private:
			entity_id eid;
			oskar::unowned<entity_system*> ent_sys;

		public:


			entity_reference() = delete;
			entity_reference(entity_reference&& other) noexcept {
				ent_sys = other.ent_sys;
				eid = other.eid;
				other.eid = placeholder_entity_id; // <- this puts the original entity reference in an invalid state
			}
			entity_reference(entity_pointer&& ptr) {
				ent_sys = ptr.ent_sys;
				eid = ptr.eid;
				ptr.eid = placeholder_entity_id;

				if(eid == placeholder_entity_id) {
					throw std::runtime_error("entity_reference can not reference the placeholder entity.");
				}
			}
			entity_reference(const entity_reference& other) : ent_sys(other.ent_sys), eid(other.eid) {
				ent_sys->increase_reference_counter(eid);
			}
			entity_reference(const entity_pointer& other) : ent_sys(other.ent_sys), eid(other.eid) {
				ent_sys->increase_reference_counter(eid);
				if(eid == placeholder_entity_id) {
					throw std::runtime_error("entity_reference can not reference the placeholder entity.");
				}
			}
			entity_reference(const temporary_entity_reference& tmp, entity_system& esys) noexcept : ent_sys(&esys), eid(tmp.eid) {
				ent_sys->increase_reference_counter(eid);
			}
			entity_reference(const temporary_entity_pointer& tmp, entity_system& esys) : ent_sys(&esys), eid(tmp.eid) {
				ent_sys->increase_reference_counter(eid);
				if(eid == placeholder_entity_id) {
					throw std::runtime_error("entity_reference can not reference the placeholder entity.");
				}
			}

			constexpr entity_reference& operator=(entity_reference&& other) noexcept {
				auto old_ent_sys = ent_sys;
				auto old_eid = eid;
				ent_sys = other.ent_sys;
				eid = other.eid;
				other.eid = old_eid;
				other.ent_sys = old_ent_sys;
				return *this;
			}
			entity_reference& operator=(const entity_reference& other) {
				if(eid != placeholder_entity_id) {
					ent_sys->decrease_reference_counter(eid);
				}
				other.ent_sys->increase_reference_counter(other.eid);
				ent_sys = other.ent_sys;
				eid = other.eid;
				return *this;
			}

			void swap(entity_reference& other) {
				std::swap(ent_sys, other.ent_sys);
				std::swap(eid, other.eid);
			}


			entity_id get_entity_id() const noexcept {
				return eid;
			}

			constexpr operator bool() const noexcept {
				return eid != placeholder_entity_id;
			}

			operator temporary_entity_reference() const {
				return temporary_entity_reference(eid);
			}
			operator entity_pointer() const& noexcept {
				return entity_pointer(eid, *ent_sys);
			}
			operator entity_pointer() && noexcept {
				entity_pointer result;
				result.eid = eid;
				result.ent_sys = ent_sys;
				eid = placeholder_entity_id;
				return result;
			}
			/*constexpr operator entity_pointer() && noexcept {
			return temporary_entity_pointer(eid);
			}*/

			~entity_reference() {
				if(eid != placeholder_entity_id) {
					ent_sys->decrease_reference_counter(eid);
				}
			}
		};
		inline void swap(entity_reference& a, entity_reference& b) {
			a.swap(b);
		}

}

#endif
