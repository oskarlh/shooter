#ifndef OSKAR_INTEGER_LOG2
#define OSKAR_INTEGER_LOG2


#include "compilation_target_info.hpp"
#include "useful.h"

#include <algorithm>
#include <type_traits>
#include <utility>
#include <type_traits>
#include <cstddef>

#if defined(_MSC_VER) && __has_include(<intrin.h>)
	#include <intrin.h>
#endif

#if defined(_MSC_VER) && __has_include(<intsafe.h>)
	#include <intsafe.h>
#endif

namespace oskar {




	template<class Signed> [[nodiscard]] constexpr std::make_unsigned_t<std::decay_t<Signed>> absolute_integer_value(Signed&& s) {
		using integer = std::decay_t<Signed>;
		using unsigned_int = std::make_unsigned_t<integer>;

		unsigned_int result;
		if constexpr(std::numeric_limits<integer>::is_signed && -std::numeric_limits<integer>::max() > std::numeric_limits<integer>::lowest()) {
			if(s != std::numeric_limits<integer>::lowest()) [[likely]] {
				result = s >= 0 ? std::forward<integer>(s) : -std::forward<integer>(s);
			} else {
				result = std::numeric_limits<integer>::max();
			}
		} else {
			result = s >= 0 ? std::forward<integer>(s) : -std::forward<integer>(s);
		}
		return result;
	}
	template<class ResultInt, class A, class B> [[nodiscard]] std::optional<ResultInt> multiply_integers_and_check_for_overflow(A&& a, B&& b) {
		using a_integer = std::decay_t<A>;
		using b_integer = std::decay_t<B>;
		static_assert(
			std::numeric_limits<ResultInt>::is_exact &&
			std::numeric_limits<a_integer>::is_exact &&
			std::numeric_limits<b_integer>::is_exact &&
			std::numeric_limits<a_integer>::is_integer &&
			std::numeric_limits<b_integer>::is_integer &&
			(std::numeric_limits<ResultInt>::is_signed || (!std::numeric_limits<a_integer>::is_signed && !std::numeric_limits<b_integer>::is_signed)) &&
			std::numeric_limits<ResultInt>::max() >= std::numeric_limits<a_integer>::max() &&
			std::numeric_limits<ResultInt>::max() >= std::numeric_limits<a_integer>::max() &&
			std::numeric_limits<ResultInt>::lowest() <= std::numeric_limits<a_integer>::lowest() &&
			std::numeric_limits<ResultInt>::lowest() <= std::numeric_limits<a_integer>::lowest()
		);


		// TODO: Consider putting in std::forward<a_integer> and std::forward<a_integer> in this function, to optimize for bigint arguments



		constexpr bool twosComplementResult = std::numeric_limits<ResultInt>::is_signed && -std::numeric_limits<ResultInt>::max() == std::numeric_limits<ResultInt>::lowest() + 1;
		static_assert(twosComplementResult || -std::numeric_limits<ResultInt>::max() == std::numeric_limits<ResultInt>::lowest());

		#if defined(_MSC_VER) && __has_include(<intsafe.h>)
			constexpr bool hasWindowsIntsafeFunctions = true;
		#else
			constexpr bool hasWindowsIntsafeFunctions = false;
		#endif

		std::optional<ResultInt> result;
		if constexpr(
			(ResultInt(std::numeric_limits<a_integer>::max()) * ResultInt(std::numeric_limits<b_integer>::max())) / ResultInt(std::numeric_limits<b_integer>::max()) == std::numeric_limits<a_integer>::max() &&
			(ResultInt(std::numeric_limits<a_integer>::lowest()) * ResultInt(std::numeric_limits<b_integer>::max())) / ResultInt(std::numeric_limits<b_integer>::max()) == std::numeric_limits<a_integer>::lowest() &&
			(ResultInt(std::numeric_limits<b_integer>::lowest()) * ResultInt(std::numeric_limits<a_integer>::max())) / ResultInt(std::numeric_limits<a_integer>::max()) == std::numeric_limits<b_integer>::lowest() &&
			(
				!std::numeric_limits<a_integer>::is_signed ||
				!std::numeric_limits<b_integer>::is_signed ||
				(ResultInt(std::numeric_limits<a_integer>::lowest()) * ResultInt(std::numeric_limits<b_integer>::lowest())) / ResultInt(std::numeric_limits<a_integer>::lowest()) == std::numeric_limits<b_integer>::lowest()
			) {
			// The result type is big enough to contain any possible result
			result = ResultInt(a) * b;
		} else if constexpr(oskar::compilation_target_infocompiler_gcc_compatible && std::is_fundamental_v<a_integer> && std::is_fundamental_v<b_integer> && std::is_fundamental_v<ResultInt>) {
			// For GCC/Clang when the integers are fundamental types
			ResultInt ri;
			if(!__builtin_mul_overflow(a, b, &ri)) [[likely]] {
				result = ri;
			}
		} else if constexpr(oskar::compilation_target_infocompiler_gcc_compatible &&
			(can_integers_of_type_a_fit_into_b<ResultInt, signed long long> || can_integers_of_type_a_fit_into_b<ResultInt, unsigned long long>)
		) {
			// For GCC/Clang when the integers must be small enough to be safely cast to (un)signed long long
			if constexpr(std::numeric_limits<ResultInt>::is_signed) {
				signed long long ri;
				if(!__builtin_mul_overflow((signed long long) a, (signed long long) b, &ri)) [[likely]] {
					result = ResultInt(ri);
				}
			}
			else {
				unsigned long long ri;
				if(!__builtin_mul_overflow((unsigned long long) a, (unsigned long long) b, &ri)) [[likely]] {
					result = ResultInt(ri);
				}
			}
		} else if constexpr(hasWindowsIntsafeFunctions &&
			(can_integers_of_type_a_fit_into_b<ResultInt, signed long long> || can_integers_of_type_a_fit_into_b<ResultInt, unsigned long long>)
		) {
			// For Windows when the integers are small enough to be safely cast to (un)signed long long
			if constexpr(std::numeric_limits<ResultInt>::is_signed) {
				signed long long ri;
				if(!LongLongMult((signed long long) a, (signed long long) b, &ri)) [[likely]] {
					result = ResultInt(ri);
				}
			} else {
				unsigned long long ri;
				if(!ULongLongMult((unsigned long long) a, (unsigned long long) b, &ri)) [[likely]] {
					result = ResultInt(ri);
				}
			}
		} else if constexpr(big_enough_fast_uint_or_sint_for_multiplication_result_exists<a_integer, b_integer>) {
			using mult_res_type = big_enough_fast_uint_or_sint_for_multiplication_result<a_integer, b_integer>;
			mult_res_type multiplied = mult_res_type(a) * mult_res_type(b);
			if((!std::numeric_limits<multiplied>::is_signed || multiplied >= std::numeric_limits<ResultInt>::lowest()) && multiplied <= std::numeric_limits<ResultInt>::max()) [[likely]] {
				result = ResultInt(multiplied);
			}
		} else if constexpr(std::numeric_limits<a_integer>::is_signed || std::numeric_limits<b_integer>::is_signed) {
			// Because signed overflow is UB, we use unsigned arithmetic instead

			auto unsignedResult = multiply_integers_and_check_for_overflow<std::make_unsigned_t<ResultInt>>(absolute_integer_value(a), absolute_integer_value(b));

			const bool positive = (a < 0) == (b < 0);

			if constexpr(!twosComplementResult) {
				if(unsignedResult.has_value() && *unsignedResult <= std::numeric_limits<ResultInt>::max()) [[likely]] {
					result = positive ? ResultInt(std::move(*unsignedResult)) : -ResultInt(std::move(unsignedResult));
				}
			} else if(unsignedResult.has_value()) [[likely]] {
				if(*unsignedResult <= std::numeric_limits<ResultInt>::max()) [[likely]] {
					result = positive ? ResultInt(std::move(*unsignedResult)) : -ResultInt(std::move(unsignedResult));
				} else if(*unsignedResult == std::make_unsigned_t<ResultInt>(std::numeric_limits<ResultInt>::max()) + 1) [[likely]] {
					result = std::numeric_limits<ResultInt>::lowest();
				}
			}
		} else {
			ResultInt ri = ResultInt(a) * b;
			if(!b || result.first / b == a) [[likely]] {
				result = ri;
			}
		}
		return result;
	}

	template<class Integer> [[nodiscard]] constexpr int constexpr_integer_log2(Integer&& i) {
		using integer = std::decay_t<Integer>;

		int result = 0;
		auto testValue = std::make_unsigned_t<integer>(i);
		int digitsRemaining = std::numeric_limits<std::make_unsigned_t<integer>>::digits;
		do {
			testValue /= 2;
			result += testValue != 0;
		} while(--digitsRemaining);
		return result;
	}

	template<class Integer> [[nodiscard]] int integer_log2(Integer&& iv) {
		constexpr bool has_builtin_clz = oskar::compilation_target_info::compiler_gcc || oskar::compilation_target_info::compiler_clang;
		constexpr bool has_long_BitScanReverse = oskar::compilation_target_info::compiler_msvc_compatible && (oskar::compilation_target_info::architecture_arm || oskar::compilation_target_info::architecture_x86_32);
		constexpr bool has_long_long_BitScanReverse = oskar::compilation_target_info::compiler_msvc_compatible && (oskar::compilation_target_info::architecture_arm || oskar::compilation_target_info::architecture_x86_64);

		using integer = std::decay_t<Integer>;

		int result;
		if constexpr(has_builtin_clz && std::numeric_limits<integer>::digits <= std::numeric_limits<unsigned int>::digits) {
			unsigned int i(iv);
			result = std::numeric_limits<unsigned int>::digits - 1 - __builtin_clz(i + !i);
		}
		else if constexpr(has_builtin_clz && std::numeric_limits<integer>::digits <= std::numeric_limits<unsigned long>::digits) {
			unsigned long i(iv);
			result = std::numeric_limits<unsigned long>::digits - 1 - __builtin_clzl(i + !i);
		}
		else if constexpr(has_long_BitScanReverse && std::numeric_limits<integer>::digits <= std::numeric_limits<unsigned long>::digits) {
			unsigned long i(iv);
			unsigned long bsrResult;
			_BitScanReverse(&bsrResult, i + !i);
			result = int(bsrResult);
		}
		else if constexpr(has_builtin_clz && std::numeric_limits<integer>::digits <= std::numeric_limits<unsigned long long>::digits) {
			unsigned long long i(iv);
			result = std::numeric_limits<unsigned long long>::digits - 1 - __builtin_clzll(i + !i);
		} else if constexpr(has_long_long_BitScanReverse && std::numeric_limits<integer>::digits <= std::numeric_limits<unsigned long long>::digits) {
			unsigned long long i(iv);
			unsigned long bsrResult;
			_BitScanReverse64(&bsrResult, i + !i);
			result = int(bsrResult);
		} else {
			result = constexpr_integer_log2(i);
		}
		return result;
	}



	template<class Integer> [[nodiscard]] constexpr int constexpr_count_trailing_zeroes(Integer&& i) {
		using integer = std::decay_t<Integer>;

		int result = 0;
		bool counting = !!i;
		auto testValue = std::make_unsigned_t<integer>(i);
		int digitsRemaining = std::numeric_limits<std::make_unsigned_t<integer>>::digits;
		do {
			counting = testValue % 2 == 0 && counting;
			result += counting;
			testValue /= 2;
		} while(--digitsRemaining);
		return result;
	}

	template<class Integer> [[nodiscard]] int count_trailing_zeroes(Integer&& iv) {
		constexpr bool has_builtin_ctz = oskar::compilation_target_info::compiler_gcc || oskar::compilation_target_info::compiler_clang;
		constexpr bool has_long_BitScanForward = oskar::compilation_target_info::compiler_msvc_compatible && (oskar::compilation_target_info::architecture_arm || oskar::compilation_target_info::architecture_x86_32);
		constexpr bool has_long_long_BitScanForward = oskar::compilation_target_info::compiler_msvc_compatible && (oskar::compilation_target_info::architecture_arm || oskar::compilation_target_info::architecture_x86_64);

		using integer = std::decay_t<Integer>;

		int result;
		if constexpr(has_builtin_ctz && std::numeric_limits<integer>::digits <= std::numeric_limits<unsigned int>::digits) {
			unsigned int i(iv);
			result = __builtin_ctz(i + !i);
		} else if constexpr(has_builtin_ctz && std::numeric_limits<integer>::digits <= std::numeric_limits<unsigned long>::digits) {
			unsigned long i(iv);
			result = __builtin_ctzl(i + !i);
		} else if constexpr(has_long_BitScanReverse && std::numeric_limits<integer>::digits <= std::numeric_limits<unsigned long>::digits) {
			unsigned long i(iv);
			unsigned long bsrResult;
			_BitScanForward(&bsrResult, i + !i);
			result = int(bsrResult);
		} else if constexpr(has_builtin_ctz && std::numeric_limits<integer>::digits <= std::numeric_limits<unsigned long long>::digits) {
			unsigned long long i(iv);
			result = __builtin_ctzll(i + !i);
		} else if constexpr(has_long_long_BitScanForward && std::numeric_limits<integer>::digits <= std::numeric_limits<unsigned long long>::digits) {
			unsigned long long i(iv);
			unsigned long bsrResult;
			_BitScanForward64(&bsrResult, i + !i);
			result = int(bsrResult);
		} else {
			result = constexpr_count_trailing_zeroes(i);
		}
		return result;
	}

}

#endif
