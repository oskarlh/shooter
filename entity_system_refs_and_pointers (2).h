#ifndef GAME_ENGINE_ENTITY_SYSTEM_REFS_AND_POINTERS_H
#define GAME_ENGINE_ENTITY_SYSTEM_REFS_AND_POINTERS_H

#include "useful.h"

#include "entity_system_numbers.h"
#include "entity_system_entity_storage.h"
#include "entity_system_class.h"
#include "entity_system_temp_refs_and_pointers.h"

namespace game_engine {

	class entity_pointer {
		friend class entity_reference;

		private:
			entity_id eid = null_entity_id;
			oskar::unowned<entity_storage*> ent_storage_ptr = nullptr;

			entity_pointer(entity_id id, entity_system& esys) : ent_storage_ptr(oskar::unowned<entity_storage*>(&esys.ent_storage)), eid(id) {
				ent_storage_ptr->increase_reference_counter(id);
			}
			entity_pointer(entity_id id, entity_storage& estor) : ent_storage_ptr(oskar::unowned<entity_storage*>(&estor)), eid(id) {
				ent_storage_ptr->increase_reference_counter(id);
			}

		public:


			constexpr entity_pointer() noexcept = default;
			constexpr entity_pointer(entity_pointer&& other) noexcept {
				ent_storage_ptr = other.ent_storage_ptr;
				eid = other.eid;
				other.eid = null_entity_id;
			}
			entity_pointer(const entity_pointer& other) : ent_storage_ptr(other.ent_storage_ptr), eid(other.eid) {
				ent_storage_ptr->increase_reference_counter(eid);
			}
			entity_pointer(const temporary_entity_pointer& tmp, entity_system& esys) : ent_storage_ptr(oskar::unowned<entity_storage*>(&esys.ent_storage)), eid(tmp.eid) {
				ent_storage_ptr->increase_reference_counter(eid);
			}
			constexpr entity_pointer(std::nullptr_t) noexcept { }

			constexpr entity_pointer& operator=(entity_pointer&& other) noexcept {
				auto old_ent_storage_ptr = ent_storage_ptr;
				auto old_eid = eid;
				ent_storage_ptr = other.ent_storage_ptr;
				eid = other.eid;
				other.eid = old_eid;
				other.ent_storage_ptr = old_ent_storage_ptr;
				return *this;
			}
			entity_pointer& operator=(const entity_pointer& other) {
				if(eid != null_entity_id) {
					ent_storage_ptr->decrease_reference_counter(eid);
				}
				other.ent_storage_ptr->increase_reference_counter(other.eid);
				ent_storage_ptr = other.ent_storage_ptr;
				eid = other.eid;
				return *this;
			}

			void swap(entity_pointer& other) {
				std::swap(ent_storage_ptr, other.ent_storage_ptr);
				std::swap(eid, other.eid);
			}



			constexpr operator bool() const noexcept {
				return eid != null_entity_id;
			}

			operator temporary_entity_reference() const {
				return temporary_entity_reference(eid);
			}
			constexpr operator temporary_entity_pointer() const noexcept {
				return temporary_entity_pointer(eid);
			}

			~entity_pointer() {
				if(eid != null_entity_id) {
					ent_storage_ptr->decrease_reference_counter(eid);
				}
			}
	};
	inline void swap(entity_pointer& a, entity_pointer& b) {
		a.swap(b);
	}




	class entity_reference {
		private:
			entity_id eid;
			oskar::unowned<entity_storage*> ent_storage_ptr;

		public:


			entity_reference() = delete;
			entity_reference(entity_reference&& other) noexcept {
				ent_storage_ptr = other.ent_storage_ptr;
				eid = other.eid;
				other.eid = null_entity_id; // <- this puts the original entity reference in an invalid state
			}
			entity_reference(entity_pointer&& ptr) {
				ent_storage_ptr = ptr.ent_storage_ptr;
				eid = ptr.eid;
				ptr.eid = null_entity_id;

				if(eid == null_entity_id) {
					throw std::runtime_error("entity_reference can not reference the null entity.");
				}
			}
			entity_reference(const entity_reference& other) : ent_storage_ptr(other.ent_storage_ptr), eid(other.eid) {
				ent_storage_ptr->increase_reference_counter(eid);
			}
			entity_reference(const entity_pointer& other) : ent_storage_ptr(other.ent_storage_ptr), eid(other.eid) {
				ent_storage_ptr->increase_reference_counter(eid);
				if(eid == null_entity_id) {
					throw std::runtime_error("entity_reference can not reference the null entity.");
				}
			}
			entity_reference(const temporary_entity_reference& tmp, entity_system& esys) noexcept : ent_storage_ptr(oskar::unowned<entity_storage*>(&esys.ent_storage)), eid(tmp.eid) {
				ent_storage_ptr->increase_reference_counter(eid);
			}
			entity_reference(const temporary_entity_pointer& tmp, entity_system& esys) : ent_storage_ptr(oskar::unowned<entity_storage*>(&esys.ent_storage)), eid(tmp.eid) {
				ent_storage_ptr->increase_reference_counter(eid);
				if(eid == null_entity_id) {
					throw std::runtime_error("entity_reference can not reference the null entity.");
				}
			}

			constexpr entity_reference& operator=(entity_reference&& other) noexcept {
				auto old_ent_storage_ptr = ent_storage_ptr;
				auto old_eid = eid;
				ent_storage_ptr = other.ent_storage_ptr;
				eid = other.eid;
				other.eid = old_eid;
				other.ent_storage_ptr = old_ent_storage_ptr;
				return *this;
			}
			entity_reference& operator=(const entity_reference& other) {
				if(eid != null_entity_id) {
					ent_storage_ptr->decrease_reference_counter(eid);
				}
				other.ent_storage_ptr->increase_reference_counter(other.eid);
				ent_storage_ptr = other.ent_storage_ptr;
				eid = other.eid;
				return *this;
			}

			void swap(entity_reference& other) {
				std::swap(ent_storage_ptr, other.ent_storage_ptr);
				std::swap(eid, other.eid);
			}



			constexpr operator bool() const noexcept {
				return eid != null_entity_id;
			}

			operator temporary_entity_reference() const {
				return temporary_entity_reference(eid);
			}
			operator entity_pointer() const& noexcept {
				return entity_pointer(eid, *ent_storage_ptr);
			}
			operator entity_pointer() && noexcept {
				entity_pointer result;
				result.eid = eid;
				result.ent_storage_ptr = ent_storage_ptr;
				eid = null_entity_id;
				return result;
			}
			/*constexpr operator entity_pointer() && noexcept {
			return temporary_entity_pointer(eid);
			}*/

			~entity_reference() {
				if(eid != null_entity_id) {
					ent_storage_ptr->decrease_reference_counter(eid);
				}
			}
		};
		inline void swap(entity_reference& a, entity_reference& b) {
			a.swap(b);
		}

}

#endif
