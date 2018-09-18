#ifndef OSKAR_USEFUL_H
#define OSKAR_USEFUL_H

#include <cstdint>
#include <limits>

namespace oskar {


	template<class Struct, std::size_t MinSize, bool needs_padding = (sizeof(Struct) < MinSize)> class pad_if_smaller_than : public Struct {};

	template<class Struct, std::size_t MinSize>
	class pad_if_smaller_than<Struct, MinSize, true> : public Struct {
		private:
			char oskar__pad_if_smaller__padding_rjq9834jr89q34j89gn5pdfg8aer[MinSize - sizeof(Struct)];
	};


		template<class Struct, std::size_t SizeMultiple> using pad_so_size_is_multiple_of = pad_if_smaller_than<Struct,
			SizeMultiple * ((sizeof(Struct) / SizeMultiple) + ((sizeof(Struct) % SizeMultiple) > 0))
	>;


	template<std::uintmax_t max_val> using big_enough_fast_uint = std::conditional_t<
		max_val <= std::numeric_limits<std::uint_fast8_t>::max(), std::uint_fast8_t, std::conditional_t<
		max_val <= std::numeric_limits<std::uint_fast16_t>::max(), std::uint_fast16_t, std::conditional_t<
		max_val <= std::numeric_limits<std::uint_fast32_t>::max(), std::uint_fast32_t, std::conditional_t<
		max_val <= std::numeric_limits<std::uint_fast64_t>::max(), std::uint_fast64_t, std::conditional_t<
		max_val <= std::numeric_limits<unsigned short>::max(), unsigned short, std::conditional_t<
		max_val <= std::numeric_limits<unsigned int>::max(), unsigned int, std::conditional_t<
		max_val <= std::numeric_limits<unsigned long>::max(), unsigned long, std::conditional_t<
		max_val <= std::numeric_limits<unsigned long long>::max(), unsigned long long, std::uintmax_t
		>>>>>>>>;
	template<std::uintmax_t digits> using fast_uint_with_digits = big_enough_fast_uint<std::uintmax_t(1) << (std::max(0, int(std::min((std::uintmax_t) std::numeric_limits<std::uintmax_t>::digits, digits)) - 1))>;


	template<class ptr> using unowned = ptr;


	template<class F> class call_on_destruction {
		private:
			F func;
			bool enabled = true;
		public:
			constexpr call_on_destruction(F&& f) : func(std::forward(f)) {}
			call_on_destruction(const call_on_destruction&) = delete;
			call_on_destruction& operator=(const call_on_destruction&) = delete;

			constexpr void disable() noexcept {
				enabled = false;
			}

			constexpr void enable() noexcept {
				enabled = true;
			}

			~call_on_destruction() noexcept(noexcept(func())) {
				if(enabled) {
					func();
				}
			}
	};

	template<class A, class B> auto if_throwing(A&& a, B&& b) {
		call_on_destruction cod(std::forward(b));
		auto result(a());
		cod.disable();
		return result;
	}
}





#endif
