#ifndef OSKAR_INTEGER_LOG2
#define OSKAR_INTEGER_LOG2

#include <algorithm>
#include "compilation_target_info.h"

#if defined(_MSC_VER) && __has_include(<intrin.h>)
	#include <intrin.h>
#endif


namespace oskar {

	template<class Integer> constexpr int constexpr_integer_log2(Integer i) {
		int result = 0;
		auto testValue = std::make_unsigned_t<Integer>(i);
		int digitsRemaining = std::numeric_limits<std::make_unsigned_t<Integer>>::digits;
		do {
			testValue /= 2;
			result += testValue != 0;
		} while(--digitsRemaining);
		return result;
	}

	template<class Integer> int integer_log2(Integer iv) {
		constexpr bool has_builtin_clz = oskar::compilation_target_info::compiler_gcc || oskar::compilation_target_info::compiler_clang;
		constexpr bool has_long_BitScanReverse = oskar::compilation_target_info::compiler_msvc_compatible && (oskar::compilation_target_info::architecture_arm || oskar::compilation_target_info::architecture_x86_32);
		constexpr bool has_long_long_BitScanReverse = oskar::compilation_target_info::compiler_msvc_compatible && (oskar::compilation_target_info::architecture_arm || oskar::compilation_target_info::architecture_x86_64);

		int result;
		if constexpr(has_builtin_clz && std::numeric_limits<Integer>::digits <= std::numeric_limits<unsigned int>::digits) {
			unsigned int i(iv);
			result = std::numeric_limits<unsigned int>::digits - 1 - __builtin_clz(i + !i)
		}
		else if constexpr(has_builtin_clz && std::numeric_limits<Integer>::digits <= std::numeric_limits<unsigned long>::digits) {
			unsigned long i(iv);
			result = std::numeric_limits<unsigned long>::digits - 1 - __builtin_clzl(i + !i);
		}
		else if constexpr(has_long_BitScanReverse && std::numeric_limits<Integer>::digits <= std::numeric_limits<unsigned long>::digits) {
			unsigned long i(iv);
			unsigned long bsrResult;
			_BitScanReverse(&bsrResult, i + !i);
			result = int(bsrResult);
		}
		else if constexpr(has_builtin_clz && std::numeric_limits<Integer>::digits <= std::numeric_limits<unsigned long long>::digits) {
			unsigned long long i(iv);
			result = std::numeric_limits<unsigned long long>::digits - 1 - __builtin_clzll(i + !i);
		} else if constexpr(has_long_long_BitScanReverse && std::numeric_limits<Integer>::digits <= std::numeric_limits<unsigned long long>::digits) {
			unsigned long long i(iv);
			unsigned long bsrResult;
			_BitScanReverse64(&bsrResult, i + !i);
			result = int(bsrResult);
		} else {
			result = constexpr_integer_log2(i);
		}
		return result;
	}
}

#endif
