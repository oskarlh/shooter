#ifndef OSKAR_OVER_ALIGNED_HEAP_ARRAY_HPP
#define OSKAR_OVER_ALIGNED_HEAP_ARRAY_HPP

#include <type_traits>
#include <memory>
#include <utility>
#include <new>
#include <cstdlib>

#include "useful.h"

namespace oskar {

	template<class Type, class Enable = void> class over_aligned_heap_array_deleter;

	template<class Type, class Enable = std::enable_if_t<!std::is_trivially_destructible_v<Type>>>
	class over_aligned_heap_array_deleter {
		static_assert(std::is_nothrow_destructible_v<Type>);
		private:
			Type* actual_allocation_start = nullptr;
			std::size_t num_constructed_objects = 0;

			constexpr over_aligned_heap_array_deleter(Type* ptr, std::size_t numObjects) noexcept : actual_allocation_start(ptr), num_constructed_objects(numObjects) {}

		public:
			over_aligned_heap_array_deleter(const over_aligned_heap_array_deleter&) = delete;
			constexpr over_aligned_heap_array_deleter() noexcept {}

			constexpr over_aligned_heap_array_deleter(over_aligned_heap_array_deleter&& other) noexcept
				: actual_allocation_start(other.actual_allocation_start), num_constructed_objects(other.num_constructed_objects) {
				other.actual_allocation_start = nullptr;
				other.num_constructed_objects = 0;
			}

			over_aligned_heap_array_deleter& operator=(const over_aligned_heap_array_deleter&) = delete;
			over_aligned_heap_array_deleter& operator=(over_aligned_heap_array_deleter&& other) noexcept {
				using std::swap;
				swap(actual_allocation_start, other.actual_allocation_start);
				swap(num_constructed_objects, other.num_constructed_objects);
			}

			void operator()(Type* objs) {
				while(num_constructed_objects > 0) {
					objs[--num_constructed_objects].~Type();
				}
				std::free(actual_allocation_start); // std::free(nullptr) does nothing, so calling std::free unconditionally is fine
				actual_allocation_start = nullptr;
			}
	};

	template<class Type, class Enable = std::enable_if_t<std::is_trivially_destructible_v<Type>>>
	class trivial_over_aligned_heap_array_deleter {
		private:
			Type* actual_allocation_start = nullptr;

			constexpr over_aligned_heap_array_deleter(Type* ptr, [[maybe_unused]] std::size_t numObjects = 0) noexcept : actual_allocation_start(ptr) {}

		public:
			over_aligned_heap_array_deleter(const over_aligned_heap_array_deleter&) = delete;
			constexpr over_aligned_heap_array_deleter() noexcept {}

			constexpr over_aligned_heap_array_deleter(over_aligned_heap_array_deleter&& other) noexcept
				: actual_allocation_start(other.actual_allocation_start) {
				other.actual_allocation_start = nullptr;
			}

			over_aligned_heap_array_deleter& operator=(const over_aligned_heap_array_deleter&) = delete;
			over_aligned_heap_array_deleter& operator=(over_aligned_heap_array_deleter&& other) noexcept {
				using std::swap;
				swap(actual_allocation_start, other.actual_allocation_start);
			}

			void operator()(Type* objs) {
				std::free(actual_allocation_start); // std::free(nullptr) does nothing, so calling std::free unconditionally is fine
				actual_allocation_start = nullptr;
			}
	};


	// Note: It's the start of the array that is aligned, not each individual element
	template<class Type, class ... ConstructorArguments>
	std::unique_ptr<Type[], over_aligned_heap_array_deleter<Type>>
	create_over_aligned_heap_array(std::size_t alignment, std::size_t numItems, ConstructorArguments&& ... constructorArguments) {
		static_assert(std::is_nothrow_destructible_v<Type>); // Not supported... yet

		if(alignment < alignof(Type)) [[unlikely]] {
			alignment = alignof(Type);
		}

		if(alignment & (alignment - 1)) { // Non-power-of-two alignment is not supported
			throw std::bad_alloc();
		}

		Type* unaligned = nullptr;
		Type* aligned = nullptr;

		oskar::call_on_destruction free_in_case_of_failure([&]() {
			std::free(unaligned); // std::free(nullptr) does nothing, so calling std::free unconditionally is fine
		});

		{
			std::size_t numBytes = numItems * sizeof(Type);
			const std::size_t bytesAwayFromAlignmentMultiple = numBytes & (alignment - 1);
			numBytes += (alignment - bytesAwayFromAlignmentMultiple) & (alignment - 1);
			if(numBytes / sizeof(Type) < numItems) { // This is extremely unlikely... it means an overflow has happened and that this number of items can't be allocated
				throw std::bad_alloc();
			}

			unaligned = (Type*)std::aligned_alloc(alignment, numBytes);
			aligned = unaligned;
		}

		if(!unaligned) [[unlikely]] {
			const std::size_t numBytes = numItems * sizeof(Type);
			const std::size_t numBytesPlusExtraAlignmentBytes = numBytes + alignment - 1;

			unaligned = (Type*) std::malloc(numBytesPlusExtraAlignmentBytes);

			if(!unaligned) {
				throw std::bad_alloc();
			}

			aligned = unaligned;
			std::align(alignment, numBytes, aligned, numBytesPlusExtraAlignmentBytes);
		}

		if constexpr(sizeof...(ConstructorArguments)) {

			Type* currentObjectToConstructPtr = aligned;

			oskar::call_on_destruction in_case_a_constructor_throws([&]() {
				if constexpr(!std::is_trivially_destructible_v<Type>) {
					while(currentObjectToConstructPtr-- > aligned) {
						currentObjectToConstructPtr->~Type();
					}
				}
			});

			for(Type* end = aligned + numItems; currentObjectToConstructPtr != end; ++currentObjectToConstructPtr) {
				new (currentObjectToConstructPtr) Type (constructorArguments...);
			}

			in_case_a_constructor_throws.disable();
		} else {
			new (aligned) Type[numItems];
		}

		std::unique_ptr<Type[], over_aligned_array_deleter>(aligned, over_aligned_heap_array_deleter(unaligned, numItems)) result;

		free_in_case_of_failure.disable();
		return std::move(result);
	}



}



#endif
