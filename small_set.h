#ifndef OSKAR_SMALL_SET_H
#define OSKAR_SMALL_SET_H
#include <memory>
#include <type_traits>
#include <functional>
#include <optional>

#include "boost/container/small_vector.hpp"

#include "integer_log2.h"

namespace oskar {
	template<class Value, std::size_t NumStackElements, class Equ = std::equal_to<Value>, class Allocator = std::allocator<Value>>
	class small_set {
		private:
			using storage = typename boost::container::small_vector<Value, NumStackElements, Allocator>;
			storage vec;

		public:
			using const_iterator = typename storage::const_iterator;
			using iterator = typename storage::iterator;
			using const_reference = typename storage::const_reference;
			using reference = typename storage::reference;
			using size_type = std::size_t;



			iterator find(const Value& value) {
				auto result = end();
				for(auto it = begin(), itEnd = end(); it != itEnd; ++it) {
					result = Equ()(*it, value) ? it : result;
				}
				return result;
			}
			const_iterator find(const Value& value) const {
				auto result = end();
				for(auto it = begin(), itEnd = end(); it != itEnd; ++it) {
					result = Equ()(*it, value) ? it : result;
				}
				return result;
			}

			bool contains(const Value& value) const {
				bool result = false;
				for(auto it = begin(), itEnd = end(); it != itEnd; ++it) {
					result = result || Equ()(*it, value);
				}
				return result;
			}

			void clear() {
				vec.clear();
			}

			bool erase(const Value& value) {
				iterator location = end();
				for(auto it = begin(), itEnd = end(); it != itEnd; ++it) {
					location = Equ()(*it, value) ? it : location;
				}
				if(location != end()) {
					if constexpr(std::is_trivially_assignable_v<Value>) {
						location = back();
					} else {
						std::swap(location, back());
					}
					vec.pop_back();
				}
				return location != end();
			}

			bool insert(const Value& value) {
				const bool shouldInsert = !contains(value);
				if(shouldInsert) {
					vec.push_back(value);
				}
				return shouldInsert;
			}

			bool insert(Value&& value) {
				const bool shouldInsert = !contains(value);
				if(shouldInsert) {
					vec.push_back(std::forward(value));
				}
				return shouldInsert;
			}

			std::size_t size() const noexcept {
				return vec.size();
			}
			bool empty() const noexcept {
				return vec.empty();
			}

			iterator begin() noexcept {
				return storage.begin();
			}
			const_iterator begin() const noexcept {
				return storage.begin();
			}

			iterator end() noexcept {
				return storage.end();
			}
			const_iterator end() const noexcept {
				return storage.end();
			}

			Value* data() noexcept {
				return storage.data();
			}
			const Value* data() const noexcept {
				return storage.data();
			}

			reference operator[](std::size_t index) {
				return storage[index];
			}
			const_reference operator[](std::size_t index) const {
				return storage[index];
			}
			reference at(std::size_t index) {
				return storage.at(index);
			}
			const_reference at(std::size_t index) const {
				return storage.at(index); 
			}

			std::size_t capacity() const noexcept {
				return storage.capacity();
			}
			void shrink_to_fit() {
				storage.shrink_to_fit();
			}

	};
}

#endif
