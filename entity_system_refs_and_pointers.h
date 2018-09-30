#ifndef GAME_ENGINE_ENTITY_SYSTEM_REFS_AND_POINTERS_H
#define GAME_ENGINE_ENTITY_SYSTEM_REFS_AND_POINTERS_H

#include <utility>
#include <cassert>
#include <functional>

#include "useful.h"

#include "entity_system_numbers.h"


namespace game_engine {

	class temporary_entity_pointer {
		friend class entity_system;
		friend class temporary_entity_reference;

		private:
			entity_id eid = placeholder_entity_id;

			constexpr temporary_entity_pointer(entity_id id) noexcept : eid(id) {}

		public:
			constexpr temporary_entity_pointer() noexcept = default;
			constexpr temporary_entity_pointer(const temporary_entity_pointer& other) noexcept = default;
			constexpr temporary_entity_pointer(temporary_entity_pointer&& other) noexcept = default;
			constexpr temporary_entity_pointer(std::nullptr_t) noexcept { }

			constexpr entity_id get_entity_id() const noexcept {
				return eid;
			}

			constexpr temporary_entity_pointer& operator=(const temporary_entity_pointer& other) noexcept {
				eid = other.eid;
				return *this;
			}
			constexpr temporary_entity_pointer& operator=(temporary_entity_pointer&& other) noexcept {
				eid = other.eid;
				return *this;
			}
			constexpr bool operator==(const temporary_entity_pointer& other) const noexcept {
				return eid == other.eid;
			}
			constexpr bool operator<(const temporary_entity_pointer& other) const noexcept {
				return eid < other.eid;
			}
			constexpr bool operator<=(const temporary_entity_pointer& other) const noexcept {
				return eid <= other.eid;
			}

			constexpr operator bool() const noexcept {
				return eid != placeholder_entity_id;
			}

			/*constexpr*/ void swap(temporary_entity_pointer& other) noexcept {
				using std::swap;
				swap(eid, other.eid);
			}
	};
	inline void swap(temporary_entity_pointer& a, temporary_entity_pointer& b) {
		a.swap(b);
	}
	constexpr bool operator>(const temporary_entity_pointer& a, const temporary_entity_pointer& b) noexcept {
		return !(a <= b);
	}
	constexpr bool operator>=(const temporary_entity_pointer& a, const temporary_entity_pointer& b) noexcept {
		return !(a < b);
	}
	constexpr bool operator!=(const temporary_entity_pointer& a, const temporary_entity_pointer& b) noexcept {
		return !(a == b);
	}

	class temporary_entity_reference {
		friend class entity_system;

		private:
			entity_id eid;

			constexpr temporary_entity_reference(entity_id id) noexcept : eid(id) {
				assert(eid != placeholder_entity_id);
			}

		public:
			constexpr temporary_entity_reference(const temporary_entity_reference& other) noexcept = default;
			constexpr temporary_entity_reference(temporary_entity_reference&& other) noexcept = default;
			temporary_entity_reference(temporary_entity_pointer tep) : temporary_entity_reference(tep.get_entity_id()) {
				if(eid == placeholder_entity_id) [[unlikely]] {
					throw std::runtime_error("(temporary_)entity_reference can not reference the placeholder entity.");
				}
			}

			constexpr entity_id get_entity_id() const noexcept {
				return eid;
			}


			constexpr temporary_entity_reference& operator=(const temporary_entity_reference& other) noexcept {
				eid = other.eid;
				return *this;
			}
			constexpr temporary_entity_reference& operator=(temporary_entity_reference&& other) noexcept {
				eid = other.eid;
				return *this;
			}
			constexpr bool operator==(const temporary_entity_reference& other) const noexcept {
				return eid == other.eid;
			}
			constexpr bool operator<(const temporary_entity_reference& other) const noexcept {
				return eid < other.eid;
			}
			constexpr bool operator<=(const temporary_entity_reference& other) const noexcept {
				return eid <= other.eid;
			}


			/*constexpr*/ void swap(temporary_entity_reference& other) noexcept {
				using std::swap;
				swap(eid, other.eid);
			}

			constexpr operator temporary_entity_pointer() const noexcept {
				return temporary_entity_pointer(get_entity_id());
			}
	};
	inline void swap(temporary_entity_reference& a, temporary_entity_reference& b) {
		a.swap(b);
	}
	constexpr bool operator>(const temporary_entity_reference& a, const temporary_entity_reference& b) noexcept {
		return !(a <= b);
	}
	constexpr bool operator>=(const temporary_entity_reference& a, const temporary_entity_reference& b) noexcept {
		return !(a < b);
	}
	constexpr bool operator!=(const temporary_entity_reference& a, const temporary_entity_reference& b) noexcept {
		return !(a == b);
	}



	class entity_pointer {
		private:
			entity_id eid = placeholder_entity_id;

		public:
			constexpr entity_pointer() noexcept = default;
			constexpr entity_pointer(const entity_pointer& other) noexcept = default;
			constexpr entity_pointer(entity_pointer&& other) noexcept = default;
			constexpr entity_pointer(std::nullptr_t) noexcept { }
			constexpr entity_pointer(entity_id id) noexcept : eid(id) { }
			constexpr entity_pointer(temporary_entity_pointer tep) noexcept : eid(tep.get_entity_id()) { };

			constexpr operator bool() const noexcept {
				return eid != placeholder_entity_id;
			}


			constexpr entity_pointer& operator=(const entity_pointer& other) noexcept {
				eid = other.eid;
				return *this;
			}
			constexpr entity_pointer& operator=(entity_pointer&& other) noexcept {
				eid = other.eid;
				return *this;
			}
			constexpr bool operator==(const entity_pointer& other) const noexcept {
				return eid == other.eid;
			}
			constexpr bool operator<(const entity_pointer& other) const noexcept {
				return eid < other.eid;
			}
			constexpr bool operator<=(const entity_pointer& other) const noexcept {
				return eid <= other.eid;
			}


			constexpr entity_id get_entity_id() const noexcept {
				return eid;
			}

			/*constexpr*/ void swap(entity_pointer& other) noexcept {
				using std::swap;
				swap(eid, other.eid);
			}
	};
	inline void swap(entity_pointer& a, entity_pointer& b) {
		a.swap(b);
	}
	constexpr bool operator>(const entity_pointer& a, const entity_pointer& b) noexcept {
		return !(a <= b);
	}
	constexpr bool operator>=(const entity_pointer& a, const entity_pointer& b) noexcept {
		return !(a < b);
	}
	constexpr bool operator!=(const entity_pointer& a, const entity_pointer& b) noexcept {
		return !(a == b);
	}



	class entity_pointer_with_deletion_point : public entity_pointer {
		private:
			deletion_point dp = 0;

		public:
			constexpr entity_pointer_with_deletion_point() noexcept = default;
			constexpr entity_pointer_with_deletion_point(const entity_pointer_with_deletion_point& id) noexcept = default;
			constexpr entity_pointer_with_deletion_point(entity_pointer_with_deletion_point&& id) noexcept = default;
			constexpr entity_pointer_with_deletion_point(const entity_pointer& epwdp, deletion_point d) noexcept : entity_pointer(epwdp), dp(d) { }
			constexpr entity_pointer_with_deletion_point(std::nullptr_t) noexcept : entity_pointer() { }
			constexpr entity_pointer_with_deletion_point(entity_id id, deletion_point d) noexcept : entity_pointer(id), dp(d) { }

			constexpr deletion_point get_deletion_point() const noexcept {
				return dp;
			}

			constexpr entity_pointer_with_deletion_point& operator=(const entity_pointer_with_deletion_point& other) noexcept {
				entity_pointer::operator=(other);
				dp = other.dp;
				return *this;
			}
			constexpr entity_pointer_with_deletion_point& operator=(entity_pointer_with_deletion_point&& other) noexcept {
				entity_pointer::operator=(other);
				dp = other.dp;
				return *this;
			}
			constexpr bool operator==(const entity_pointer_with_deletion_point& other) const noexcept {
				return dp == other.dp && entity_pointer::operator==(other);
			}
			constexpr bool operator<(const entity_pointer_with_deletion_point& other) const noexcept {
				return dp < other.dp || (dp == other.dp && entity_pointer::operator<(other));
			}
			constexpr bool operator<=(const entity_pointer_with_deletion_point& other) const noexcept {
				return dp < other.dp || (dp == other.dp && entity_pointer::operator<=(other));
			}

			/*constexpr*/ void swap(entity_pointer_with_deletion_point& other) noexcept {
				using std::swap;
				entity_pointer::swap(other);
				swap(dp, other.dp);
			}

	};
	inline void swap(entity_pointer_with_deletion_point& a, entity_pointer_with_deletion_point& b) {
		a.swap(b);
	}

	constexpr bool operator>(const entity_pointer_with_deletion_point& a, const entity_pointer_with_deletion_point& b) noexcept {
		return !(a <= b);
	}
	constexpr bool operator>=(const entity_pointer_with_deletion_point& a, const entity_pointer_with_deletion_point& b) noexcept {
		return !(a < b);
	}
	constexpr bool operator!=(const entity_pointer_with_deletion_point& a, const entity_pointer_with_deletion_point& b) noexcept {
		return !(a == b);
	}



	class entity_reference {
		private:
			entity_id eid;

		public:
			constexpr entity_reference(const entity_reference& other) noexcept = default;
			constexpr entity_reference(entity_reference&& other) noexcept = default;
			entity_reference(entity_id id) : eid(id) {
				if(eid == placeholder_entity_id) [[unlikely]] {
					throw std::runtime_error("(temporary_)entity_reference can not reference the placeholder entity.");
				}
			}
			entity_reference(entity_pointer ep) : entity_reference(ep.get_entity_id()) {}
			constexpr entity_reference(temporary_entity_reference tep) noexcept : eid(tep.get_entity_id()) { };

			constexpr entity_id get_entity_id() const noexcept {
				return eid;
			}


			constexpr entity_reference& operator=(const entity_reference& other) noexcept {
				eid = other.eid;
				return *this;
			}
			constexpr entity_reference& operator=(entity_reference&& other) noexcept {
				eid = other.eid;
				return *this;
			}
			constexpr bool operator==(const entity_reference& other) const noexcept {
				return eid == other.eid;
			}
			constexpr bool operator<(const entity_reference& other) const noexcept {
				return eid < other.eid;
			}
			constexpr bool operator<=(const entity_reference& other) const noexcept {
				return eid <= other.eid;
			}

			/*constexpr*/ void swap(entity_reference& other) noexcept {
				using std::swap;
				swap(eid, other.eid);
			}

			constexpr operator entity_pointer() const noexcept {
				return entity_pointer(get_entity_id());
			}
	};
	inline void swap(entity_reference& a, entity_reference& b) {
		a.swap(b);
	}
	constexpr bool operator>(const entity_reference& a, const entity_reference& b) noexcept {
		return !(a <= b);
	}
	constexpr bool operator>=(const entity_reference& a, const entity_reference& b) noexcept {
		return !(a < b);
	}
	constexpr bool operator!=(const entity_reference& a, const entity_reference& b) noexcept {
		return !(a == b);
	}


	class entity_reference_with_deletion_point : public entity_reference {
		private:
			deletion_point dp;

		public:

			constexpr entity_reference_with_deletion_point(const entity_reference_with_deletion_point& id) noexcept = default;
			constexpr entity_reference_with_deletion_point(entity_reference_with_deletion_point&& id) noexcept = default;
			constexpr entity_reference_with_deletion_point(const entity_reference& erwdp, deletion_point d) noexcept : entity_reference(erwdp), dp(d) { }
			entity_reference_with_deletion_point(entity_id id, deletion_point d) : entity_reference(id), dp(d) { }
			entity_reference_with_deletion_point(const entity_pointer_with_deletion_point& ep) : entity_reference_with_deletion_point(ep.get_entity_id(), ep.get_deletion_point()) { }

			constexpr deletion_point get_deletion_point() const noexcept {
				return dp;
			}

			constexpr entity_reference_with_deletion_point& operator=(const entity_reference_with_deletion_point& other) noexcept {
				((entity_pointer&) *this) = other;
				dp = other.dp;
				return *this;
			}
			constexpr entity_reference_with_deletion_point& operator=(entity_reference_with_deletion_point&& other) noexcept {
				((entity_pointer&)*this) = other;
				dp = other.dp;
				return *this;
			}
			constexpr bool operator==(const entity_reference_with_deletion_point& other) const noexcept {
				return dp == other.dp && entity_reference::operator==(other);
			}
			constexpr bool operator<(const entity_reference_with_deletion_point& other) const noexcept {
				return dp < other.dp || (dp == other.dp && entity_reference::operator<(other));
			}
			constexpr bool operator<=(const entity_reference_with_deletion_point& other) const noexcept {
				return dp < other.dp || (dp == other.dp && entity_reference::operator<=(other));
			}

			/*constexpr*/ void swap(entity_reference_with_deletion_point& other) noexcept {
				using std::swap;
				entity_reference::swap(other);
				swap(dp, other.dp);
			}

			constexpr operator entity_pointer_with_deletion_point() const noexcept {
				return entity_pointer_with_deletion_point(get_entity_id(), get_deletion_point());
			}
	};
	inline void swap(entity_reference_with_deletion_point& a, entity_reference_with_deletion_point& b) {
		a.swap(b);
	}
	constexpr bool operator>(const entity_reference_with_deletion_point& a, const entity_reference_with_deletion_point& b) noexcept {
		return !(a <= b);
	}
	constexpr bool operator>=(const entity_reference_with_deletion_point& a, const entity_reference_with_deletion_point& b) noexcept {
		return !(a < b);
	}
	constexpr bool operator!=(const entity_reference_with_deletion_point& a, const entity_reference_with_deletion_point& b) noexcept {
		return !(a == b);
	}

}

namespace std {
	template<> struct hash<game_engine::temporary_entity_pointer> {
		std::size_t operator()(const game_engine::temporary_entity_pointer& ent) const {
			return std::size_t(ent.get_entity_id());
		}
	};
	template<> struct hash<game_engine::entity_pointer> {
		std::size_t operator()(const game_engine::entity_pointer& ent) const {
			return std::size_t(ent.get_entity_id());
		}
	};
	template<> struct hash<game_engine::entity_pointer_with_deletion_point> {
		std::size_t operator()(const game_engine::entity_pointer_with_deletion_point& ent) const {
			// TODO: Instead of bit shifting, use a hash combiner function
			return (std::size_t(ent.get_deletion_point()) << 8) + std::size_t(ent.get_entity_id());
		}
	};

	template<> struct hash<game_engine::temporary_entity_reference> {
		std::size_t operator()(const game_engine::temporary_entity_reference& ent) const {
			return hash<game_engine::temporary_entity_pointer>()(ent);
		}
	};
	template<> struct hash<game_engine::entity_reference> {
		std::size_t operator()(const game_engine::entity_reference& ent) const {
			return hash<game_engine::entity_pointer>()(ent);
		}
	};
	template<> struct hash<game_engine::entity_reference_with_deletion_point> {
		std::size_t operator()(const game_engine::entity_reference_with_deletion_point& ent) const {
			return hash<game_engine::entity_pointer_with_deletion_point>()(ent);
		}
	};
}

#endif
