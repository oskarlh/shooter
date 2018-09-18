#ifndef GAME_ENGINE_ENTITY_SYSTEM_TEMP_REFS_AND_POINTERS_H
#define GAME_ENGINE_ENTITY_SYSTEM_TEMP_REFS_AND_POINTERS_H

#include "useful.h"

#include "entity_system_numbers.h"

namespace game_engine {

	class temporary_entity_reference {
		friend class entity_system;
		friend class entity_pointer;
		friend class entity_reference;
		friend class temporary_entity_pointer;

	private:
		entity_id eid;

		explicit temporary_entity_reference(entity_id id) : eid(id) {
			if(id == placeholder_entity_id) {
				throw std::runtime_error("temporary_entity_reference is not allowed to refer to the placeholder entity.");
			}
		}

	public:
		temporary_entity_reference() = delete;
		temporary_entity_reference(const temporary_entity_reference&) noexcept = default;
		temporary_entity_reference(temporary_entity_reference&&) noexcept = default;

		temporary_entity_reference& operator=(const temporary_entity_reference& other) noexcept = default;
		temporary_entity_reference& operator=(temporary_entity_reference&& other) noexcept = default;



		void swap(temporary_entity_reference& other) {
			std::swap(eid, other.eid);
		}

		entity_id get_entity_id() const noexcept {
			return eid;
		}
	};
	inline void swap(temporary_entity_reference& a, temporary_entity_reference& b) {
		a.swap(b);
	}

	class temporary_entity_pointer {
		friend class entity_system;
		friend class entity_pointer;
		friend class entity_reference;
		friend constexpr bool operator <(const temporary_entity_pointer& a, const temporary_entity_pointer& b) noexcept;
		friend constexpr bool operator ==(const temporary_entity_pointer& a, const temporary_entity_pointer& b) noexcept;

	private:
		entity_id eid = placeholder_entity_id;

		explicit constexpr temporary_entity_pointer(entity_id id) noexcept : eid(id) { }

	public:
		constexpr temporary_entity_pointer() noexcept = default;
		temporary_entity_pointer(const temporary_entity_reference& r) noexcept : eid(r.eid) {}
		constexpr temporary_entity_pointer(const temporary_entity_pointer&) noexcept = default;
		constexpr temporary_entity_pointer(temporary_entity_pointer&&) noexcept = default;
		constexpr temporary_entity_pointer(std::nullptr_t) noexcept { }

		constexpr temporary_entity_pointer& operator=(const temporary_entity_pointer& other) noexcept = default;
		constexpr temporary_entity_pointer& operator=(temporary_entity_pointer&& other) noexcept = default;
		constexpr temporary_entity_pointer& operator=(nullptr_t) {
			*this = temporary_entity_pointer();
			return *this;
		}


		constexpr operator bool() noexcept {
			return eid != placeholder_entity_id;
		}

		void swap(temporary_entity_pointer& other) {
			std::swap(eid, other.eid);
		}

		operator temporary_entity_reference() const {
			return temporary_entity_reference(eid);
		}

		entity_id get_entity_id() const noexcept {
			return eid;
		}
	};
	inline void swap(temporary_entity_pointer& a, temporary_entity_pointer& b) {
		a.swap(b);
	}
	constexpr bool operator <(const temporary_entity_pointer& a, const temporary_entity_pointer& b) noexcept {
		return a.eid < b.eid;
	}
	constexpr bool operator ==(const temporary_entity_pointer& a, const temporary_entity_pointer& b) noexcept {
		return a.eid == b.eid;
	}
	constexpr bool operator <=(const temporary_entity_pointer& a, const temporary_entity_pointer& b) noexcept {
		return !(b < a);
	}
	constexpr bool operator >(const temporary_entity_pointer& a, const temporary_entity_pointer& b) noexcept {
		return b < a;
	}
	constexpr bool operator >=(const temporary_entity_pointer& a, const temporary_entity_pointer& b) noexcept {
		return !(a < b);
	}
	constexpr bool operator !=(const temporary_entity_pointer& a, const temporary_entity_pointer& b) noexcept {
		return !(a == b);
	}


}

#endif
