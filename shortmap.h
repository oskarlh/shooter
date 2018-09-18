#ifndef OSKAR_SHORTMAP_H
#define OSKAR_SHORTMAP_H

#include <utility>
#include <algorithm>
#include <memory>

template <class Key, class Value, class Allocator = std::allocator<std::pair<const Key, Value>>> class shortmap {
	private:
		std::vector<std::pair<const Key, Value>> vtr;

	public:
		using iterator = typename std::vector<std::pair<const Key, Value>>::iterator;
		using const_iterator = typename std::vector<std::pair<const Key, Value>>::const_iterator;

		template<class T> void insert_or_assign(const Key& key, T&& value) {
			auto it = find_key(key);
			if(it == end()) {
				vtr.push_back(std::pair<const Key, Value>(std::forward(key), std::forward(value)));
			} else {
				it->second = std::forward(value);
			}
		}
		template<class T> void insert_or_assign(Key&& key, T&& value) {
			auto it = find_key(key);
			if(it == end()) {
				vtr.push_back(std::pair<const Key, Value>(std::forward(key), std::forward(value)));
			} else {
				it->second = std::forward(value);
			}
		}

		const_iterator find_key(const Key& key) const {
			return std::find_if(vtr.begin(), vtr.end(), [](const std::pair<const Key, Value>& p) { return p.first == key; });
		}
		iterator find_key(const Key& key) {
			return std::find_if(vtr.begin(), vtr.end(), [](const std::pair<const Key, Value>& p) { return p.first == key; });
		}

		const_iterator begin() const {
			return vtr.begin();
		}
		iterator begin() {
			return vtr.begin();
		}

		const_iterator end() const {
			return vtr.end();
		}
		iterator end() {
			return vtr.end();
		}

		void shrink_to_fit() {
			vtr.shrink_to_fit();
		}
		void reserve(std::size_t newCap) {
			vtr.reserve(newCap);
		}
};

#endif OSKAR_SHORTMAP_H
