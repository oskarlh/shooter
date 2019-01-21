#ifndef OSKAR_USEFUL_H
#define OSKAR_USEFUL_H

#include <cstdint>
#include <limits>
#include <bitset>
#include <chrono>
#include <optional>


#ifdef _MSC_VER
	#if defined(WIN32_LEAN_AND_MEAN) && defined(NOMINMAX)
		#include <windows.h>
		//#include <processthreadsapi.h>
		//#include <realtimeapiset.h>
	#elif defined(WIN32_LEAN_AND_MEAN)
		#define NOMINMAX
		#include <windows.h>
		//#include <processthreadsapi.h>
		//#include <realtimeapiset.h>
		#undef NOMINMAX
	#elif defined(NOMINMAX)
		#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
		//#include <processthreadsapi.h>
		//#include <realtimeapiset.h>
		#undef WIN32_LEAN_AND_MEAN
	#else
		#define NOMINMAX
		#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
		//#include <processthreadsapi.h>
		//#include <realtimeapiset.h>
		#undef WIN32_LEAN_AND_MEAN
		#undef NOMINMAX
	#endif
#elif defined(__unix__) && __has_include(<sys/resource.h>)
	#include <sys/resource.h>
#endif

#include "boost/multiprecision/cpp_int.hpp"

namespace oskar {


	template<class Struct, std::size_t MinSize, bool needs_padding = (sizeof(Struct) < MinSize)> class pad_if_smaller_than : public Struct {};

	template<class Struct, std::size_t MinSize>
	class pad_if_smaller_than<Struct, MinSize, true> : public Struct {
		private:
			char oskar_pad_if_smaller_than_padding_r6jq98i4jra4rOXl89q34j89gn5pdfg8B3r[MinSize - sizeof(Struct)];
	};


	template<class Struct, std::size_t SizeMultiple> using pad_so_size_is_multiple_of = pad_if_smaller_than<Struct,
		SizeMultiple * ((sizeof(Struct) / SizeMultiple) + ((sizeof(Struct) % SizeMultiple) > 0))
	>;


	template<class A, class B> constexpr bool can_integers_of_type_a_fit_into_b = (
		std::numeric_limits<std::decay_t<A>>::is_integer && std::numeric_limits<std::decay_t<B>>::is_integer &&
		std::numeric_limits<std::decay_t<A>>::is_exact && std::numeric_limits<std::decay_t<B>>::is_exact &&
		std::numeric_limits<std::decay_t<A>>::is_signed <= std::numeric_limits<std::decay_t<B>>::is_signed &&
		std::numeric_limits<std::decay_t<A>>::lowest() >= std::numeric_limits<std::decay_t<B>>::lowest() &&
		std::numeric_limits<std::decay_t<A>>::max() <= std::numeric_limits<std::decay_t<B>>::max()
	);

	template<std::intmax_t max_val, std::intmax_t lowest_val = -1 - max_val> using big_enough_fast_sint = std::conditional_t<
		max_val <= std::numeric_limits<std::int_fast8_t>::max() && lowest_val <= std::numeric_limits<std::int_fast8_t>::lowest(), std::int_fast8_t, std::conditional_t<
		max_val <= std::numeric_limits<std::int_fast16_t>::max() && lowest_val <= std::numeric_limits<std::int_fast16_t>::lowest(), std::int_fast16_t, std::conditional_t<
		max_val <= std::numeric_limits<std::int_fast32_t>::max() && lowest_val <= std::numeric_limits<std::int_fast32_t>::lowest(), std::int_fast32_t, std::conditional_t<
		max_val <= std::numeric_limits<std::int_fast64_t>::max() && lowest_val <= std::numeric_limits<std::int_fast64_t>::lowest(), std::int_fast64_t, std::conditional_t<
		max_val <= std::numeric_limits<signed short>::max() && lowest_val <= std::numeric_limits<signed short>::lowest(), signed short, std::conditional_t<
		max_val <= std::numeric_limits<signed int>::max() && lowest_val <= std::numeric_limits<signed int>::lowest(), signed int, std::conditional_t<
		max_val <= std::numeric_limits<signed long>::max() && lowest_val <= std::numeric_limits<signed long>::lowest(), signed long, std::conditional_t<
		max_val <= std::numeric_limits<signed long long>::max() && lowest_val <= std::numeric_limits<signed long long>::lowest(), signed long long, std::sintmax_t
		>>>>>>>>;
	template<std::uintmax_t digits> using fast_sint_with_digits = std::enable_if_t<digits <= std::numeric_limits<std::intmax_t>::digits, big_enough_fast_sint<std::intmax_t(1) << int(digits)>>;
	

	namespace detail {
		template<class A, class B> constexpr bool big_enough_fast_sint_for_multiplication_result_exists_no_cvr = (
			std::numeric_limits<A>::is_integer && std::numeric_limits<B>::is_integer &&
			std::numeric_limits<A>::is_exact && std::numeric_limits<B>::is_exact &&
			std::numeric_limits<A>::max() < std::numeric_limits<std::intmax_t>::max() &&
			std::numeric_limits<B>::max() < std::numeric_limits<std::intmax_t>::max() &&
			std::numeric_limits<A>::lowest() > std::numeric_limits<std::intmax_t>::lowest() &&
			std::numeric_limits<B>::lowest() > std::numeric_limits<std::intmax_t>::lowest() &&
			std::intmax_t(std::numeric_limits<A>::max()) * std::intmax_t(std::numeric_limits<B>::max()) / std::intmax_t(std::numeric_limits<A>::max()) == std::numeric_limits<B>::max() &&
			std::intmax_t(std::numeric_limits<A>::lowest()) * std::intmax_t(std::numeric_limits<B>::max()) / std::intmax_t(std::numeric_limits<B>::max()) == std::numeric_limits<A>::lowest() &&
			std::intmax_t(std::numeric_limits<B>::lowest()) * std::intmax_t(std::numeric_limits<A>::max()) / std::intmax_t(std::numeric_limits<A>::max()) == std::numeric_limits<B>::lowest() &&
			(!std::numeric_limits<A>::is_signed || !std::numeric_limits<B>::is_signed || std::intmax_t(std::numeric_limits<A>::lowest()) * std::intmax_t(std::numeric_limits<B>::lowest()) / std::intmax_t(std::numeric_limits<B>::lowest()) == std::numeric_limits<A>::lowest())
		);
	}

	template<class A, class B> constexpr bool big_enough_fast_sint_for_multiplication_result_exists = detail::big_enough_fast_sint_for_multiplication_result_exists_no_cvr<std::decay_t<A>, std::decay_t<B>>;

	template<class A, class B> using big_enough_fast_sint_for_multiplication_result = std::enable_if <
		big_enough_fast_sint_for_multiplication_result_exists<A, B>,
		big_enough_fast_sint<
			std::max(
				std::intmax_t(std::numeric_limits<std::decay_t<A>>::max()) * std::intmax_t(std::numeric_limits<std::decay_t<B>>::max()),
				std::intmax_t(std::numeric_limits<std::decay_t<A>>::lowest()) * std::intmax_t(std::numeric_limits<std::decay_t<B>>::lowest())
			), std::min(
				std::intmax_t(std::numeric_limits<std::decay_t<A>>::max()) * std::intmax_t(std::numeric_limits<std::decay_t<B>>::lowest()),
				std::intmax_t(std::numeric_limits<std::decay_t<A>>::lowest()) * std::intmax_t(std::numeric_limits<std::decay_t<B>>::max())
			)
		>
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

	namespace detail {
		template<class A, class B> constexpr bool big_enough_fast_uint_for_multiplication_result_exists_no_cvr = (
			std::numeric_limits<A>::is_integer && std::numeric_limits<B>::is_integer &&
			std::numeric_limits<A>::is_exact && std::numeric_limits<B>::is_exact &&
			!std::numeric_limits<A>::is_signed && !std::numeric_limits<B>::is_signed &&
			std::numeric_limits<A>::max() < std::numeric_limits<std::uintmax_t>::max() &&
			std::numeric_limits<B>::max() < std::numeric_limits<std::uintmax_t>::max() &&
			std::uintmax_t(std::numeric_limits<A>::max()) * std::intmax_t(std::numeric_limits<B>::max()) / std::uintmax_t(std::numeric_limits<A>::max()) == std::numeric_limits<B>::max()
		);
	}

	template<class A, class B> constexpr bool big_enough_fast_uint_for_multiplication_result_exists = detail::big_enough_fast_uint_for_multiplication_result_exists_no_cvr<std::decay_t<A>, std::decay_t<B>>;


	template<class A, class B> using big_enough_fast_uint_for_multiplication_result = std::enable_if<
		big_enough_fast_uint_for_multiplication_result_exists<A, B>,
		big_enough_fast_uint<
			std::max(
				std::uintmax_t(std::numeric_limits<std::decay_t<A>>::max()) * std::uintmax_t(std::numeric_limits<std::decay_t<B>>::max()),
				std::uintmax_t(std::numeric_limits<std::decay_t<A>>::lowest()) * std::uintmax_t(std::numeric_limits<std::decay_t<B>>::lowest())
			), std::min(
				std::uintmax_t(std::numeric_limits<std::decay_t<A>>::max()) * std::uintmax_t(std::numeric_limits<std::decay_t<B>>::lowest()),
				std::uintmax_t(std::numeric_limits<std::decay_t<A>>::lowest()) * std::uintmax_t(std::numeric_limits<std::decay_t<B>>::max())
			)
		>
	>;

	namespace detail {
		template<class A, class B, bool is_signed = std::numeric_limits<A>::is_signed || std::numeric_limits<B>::is_signed>
		struct big_enough_fast_uint_or_sint_for_multiplication_result_helper {
			using type = big_enough_fast_sint_for_multiplication_result<A, B>;
		};
		template<class A, class B>
		struct big_enough_fast_uint_or_sint_for_multiplication_result_helper <A, B, false> {
			using type = big_enough_fast_uint_for_multiplication_result<A, B>;
		};

		template<class A, class B, bool is_signed = std::numeric_limits<A>::is_signed || std::numeric_limits<B>::is_signed>
		struct big_enough_fast_uint_or_sint_for_multiplication_result_exists_helper {
			static constexpr bool value = big_enough_fast_sint_for_multiplication_result_exists<A, B>;
		};
		template<class A, class B>
		struct big_enough_fast_uint_or_sint_for_multiplication_result_exists_helper <A, B, false> {
			static constexpr bool type = big_enough_fast_uint_for_multiplication_result_exists<A, B>;
		};
	}

	template<class A, class B>
	constexpr bool big_enough_fast_uint_or_sint_for_multiplication_result_exists =
		detail::big_enough_fast_uint_or_sint_for_multiplication_result_exists_helper<std::decay_t<A>, std::decay_t<B>>
	;
	template<class A, class B>
	using big_enough_fast_uint_or_sint_for_multiplication_result =
		detail::big_enough_fast_uint_or_sint_for_multiplication_result_helper<std::decay_t<A>, std::decay_t<B>>
	;



	template<class UnsignedInt> constexpr std::decay_t<UnsignedInt> round_up_to_power_of_two(UnsignedInt&& val) {
		static_assert(
			std::numeric_limits<std::decay_t<UnsignedInt>>::is_integer &&
			std::numeric_limits<std::decay_t<UnsignedInt>>::is_exact &&
			!std::numeric_limits<std::decay_t<UnsignedInt>>::is_signed
		);

		std::decay_t<UnsignedInt> result(std::forward(val));
		result += !result;
		--result;
		for(unsigned int shift = 1; shift < std::numeric_limits<std::decay_t<UnsignedInt>>::digits; shift <<= 1) {
			result |= (result >> shift);
		}
		++result;
		return result;
	}


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


	// Avoids overflow during the multiplication part of ((a * b) / divisor)
	template<class IntType> [[nodiscard]] constexpr std::decay_t<IntType> multiply_and_divide(const IntType& a, const IntType& b, const IntType& divisor) {

		IS THIS FUNCTION WELL-WRITTEN?!?!!?!?! PLEASE REREAD IT AND CONSIDER THAT SIGNED OVERFLOW IS UB, AND THAT lowest() IS -max() - 1
		USE big_enough_fast_uint_or_sint_for_multiplication_result in some places to reduce code!!
		

		using integer = std::decay_t<IntType>;

		static_assert(
			std::numeric_limits<integer>::is_integer && 
			std::numeric_limits<integer>::max() == (((integer(1) << (std::numeric_limits<integer>::digits - 1)) - 1) * 2 + 1
		);

		integer result;
		if constexpr(std::numeric_limits<integer>::is_signed && std::numeric_limits<integer>::digits * 2 <= std::numeric_limits<std::intmax_t>::digits) {
			using twice_the_digits_type = big_enough_fast_sint<std::intmax_t(std::numeric_limits<integer>::max()) * std::intmax_t(std::numeric_limits<integer>::max())>;
			result = (twice_the_digits_type(a) * twice_the_digits_type(b)) / twice_the_digits_type(divisor);
		} else if constexpr(std::numeric_limits<integer>::digits * 2 <= std::numeric_limits<std::uintmax_t>::digits) {
			bool negate = false;
			if constexpr(std::numeric_limits<integer>::is_signed) {
				if(a < 0) {
					a = -a;
					negate = !negate;
				}
				if(b < 0) {
					b = -b;
					negate = !negate;
				}
				if(c < 0) {
					c = -c;
					negate = !negate;
				}
			}

			using twice_the_digits_type = big_enough_fast_uint<std::uintmax_t(std::numeric_limits<integer>::max()) * std::uintmax_t(std::numeric_limits<integer>::max())>;
			result = (twice_the_digits_type(a) * twice_the_digits_type(b)) / twice_the_digits_type(divisor);

			if constexpr(std::numeric_limits<integer>::is_signed) {
				if(negate) {
					result = -result;
				}
			}
		}
		else {
			using twice_the_digits_type = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<
				std::numeric_limits<integer>::digits * 2,
				std::numeric_limits<integer>::digits * 2,
				std::numeric_limits<integer>::is_signed ? boost::multiprecision::signed_magnitude : boost::multiprecision::unsigned_magnitude,
				unchecked,
				void
			>>;
			twice_the_digits_type bigResult;
			boost::multiprecision::multiply(bigResult, a, b);
			bigResult /= divisor;
			result = integer(std::move(bigResult));
		}

		return result;
	}



	namespace division_of_labour_for_threads {
		class linear_1d {
			private:
				std::size_t threadCount = 1;
				std::size_t alignment = 1;
			public:
				constexpr linear_1d() noexcept = default;
				constexpr linear_1d(const linear_1d&) noexcept = default;
				constexpr linear_1d(linear_1d&&) noexcept = default;
				constexpr linear_1d(std::size_t threadC, std::size_t align = 1) noexcept : threadCount(threadC), alignment(std::max(std::size_t(1), align)) { }

				constexpr linear_1d& operator=(const linear_1d&) noexcept = default;
				constexpr linear_1d& operator=(linear_1d&&) noexcept = default;

				constexpr linear_1d& setThreadCount(std::size_t threadC) noexcept {
					threadCount = threadC;
					return *this;
				}

				constexpr linear_1d& setAlignment(std::size_t align) noexcept {
					alignment = std::max(std::size_t(1), align);
					return *this;
				}

				[[nodiscard]] constexpr std::size_t getThreadCount() const noexcept {
					return threadCount;
				}
				[[nodiscard]] constexpr std::size_t getAlignment() const noexcept {
					return alignment;
				}

				[[nodiscard]] constexpr std::pair<std::size_t /*start*/, std::size_t /*end*/>
				operator()(std::size_t threadNumber, std::size_t numTasks) const {
					if(threadNumber >= threadCount) {
						throw std::runtime_error("Bad args");
					}
					std::size_t numTasksDivAlignment = (numTasks / alignment) + (numTasks % alignment != 0);

					std::size_t start = multiply_and_divide(numTasksDivAlignment, threadNumber, threadCount);
					std::size_t end = multiply_and_divide(numTasksDivAlignment, threadNumber + 1, threadCount);
					
					start *= alignment;
					end *= alignment;
					
					start = std::min(start, numTasks);
					end = std::min(end, numTasks);

					return std::pair(start, end);
				}

		};
		class middle_out_1d {
		};
		class measured_1d {
			private:
				struct per_thread_data {
					std::atomic<std::size_t> leftTime;
					std::atomic<std::size_t> rightTime;
					std::atomic<std::size_t> betweenTime;
					std::atomic<std::size_t> leftIndex;
					std::atomic<std::size_t> rightIndex;
				};

		};
	}
	class multithreaded_task_splitter_2d {
		public:

			std::pair<std::pair<std::size_t, std::size_t>, std::pair<std::size_t, std::size_t>> divide_parallel_2dimensional_work(std::size_t threadNumber, std::size_t threadCount, std::size_t width, std::size_t height, std::size_t columnsMustBeMultipleOf = 1, std::size_t rowsMustBeMultipleOf = 1) {
				if(!columnsMustBeMultipleOf || !rowsMustBeMultipleOf || threadNumber >= threadCount) {
					throw std::runtime_error("Bad args");
				}
				const std::size_t totalSize = width * height;
				const std::size_t sizePerThread = totalSize / threadCount;
				

				std::hardware_destructive_interference_size;
				aSize * bSize / 4;
			}

	};


	class computation_timer {
		private:
			#ifdef _MSC_VER
				static constexpr bool useQueryThreadCycleTime = true;
				static constexpr bool useGetrusage = false;
				using cpu_time_point = ULONG64;
			#elif defined(__unix__) && __has_include(<sys/resource.h>) && defined(RUSAGE_THREAD)
				static constexpr bool useQueryThreadCycleTime = false;
				static constexpr bool useGetrusage = true;
				using cpu_time_point = std::uintmax_t;
			#else
				static constexpr bool useQueryThreadCycleTime = false;
				static constexpr bool useGetrusage = false;
				static std::chrono::high_resolution_clock::rep startValue;
				using cpu_time_point = std::chrono::high_resolution_clock::rep;
			#endif

			std::optional<cpu_time_point> startTime;

			static std::optional<cpu_time_point> current_cpu_time() {
				std::optional<cpu_time_point> result;

				if constexpr(useQueryThreadCycleTime) { // Windows
					ULONG64 threadCycleTime = 0;
					if(::QueryThreadCycleTime(::GetCurrentThread(), &threadCycleTime)) [[likely]] {
						result = threadCycleTime;
					}
				} else if constexpr(useGetrusage) { // Linux and a few other UNIX(-like) systems
					rusage usage;
					usage.ru_utime.tv_sec = usage.ru_utime.tv_usec = usage.ru_stime.tv_sec = usage.ru_stime.tv_usec = 0;
					if(!getrusage(RUSAGE_THREAD, &usage)) [[likely]] {
						cpu_time_point fullSecs = cpu_time_point(usage.ru_utime.tv_sec) + cpu_time_point(usage.ru_stime.tv_sec);
						if(fullSecs < usage.ru_utime.tv_sec) [[unlikely]] { // Addition overflowed
							fullSecs = std::numeric_limits<cpu_time_point>::max();
						}
						cpu_time_point fullSecsAsMicrosecs = fullSecs * 1'000'000;
						if(fullSecsAsMicrosecs / 1'000'000 != fullSecs) [[unlikely]] { // Multiplication overflowed
							fullSecsAsMicrosecs = std::numeric_limits<cpu_time_point>::max();
						}

						cpu_time_point restOfTheMicrosecs = cpu_time_point(usage.ru_utime.tv_usec) + cpu_time_point(usage.su_utime.tv_usec);
						cpu_time_point everythingSummed = fullSecsAsMicrosecs + restOfTheMicrosecs;
						if(everythingSummed < fullSecsAsMicrosecs) [[unlikely]] { // Addition overflowed
							everythingSummed = std::numeric_limits<cpu_time_point>::max();
						}
						result = everythingSummed;
					}
				} else { // Other platforms
					result = std::chrono::high_resolution_clock::now().time_since_epoch().count();
				}
				return result;
			}

		public:
			explicit computation_timer(bool initialize) : startTime(initialize ? current_cpu_time() : std::nullopt) {}
			computation_timer() : startTime(current_cpu_time()) {}
			computation_timer(const computation_timer&) = default;
			computation_timer(computation_timer&&) = default;
			computation_timer& operator=(const computation_timer&) = default;
			computation_timer& operator=(computation_timer&&) = default;

			std::size_t time_passed(std::size_t minValue = 1) const {
				std::size_t result = minValue;
				if(startTime.has_value()) [[likely]] {
					std::optional<cpu_time_point> now = current_cpu_time();
					if(now.has_value() && now.value() >= startTime.value()) [[likely]] {
						auto diff = now.value() - startTime.value();
						if(diff > (cpu_time_point) std::numeric_limits<std::size_t>::max()) {
							diff = (cpu_time_point) std::numeric_limits<std::size_t>::max();
						}
						result = std::max(minValue, (std::size_t) diff);
					}
				}
				return result;
			}
			std::size_t measure_and_reset(std::size_t minValue = 1) {
				std::size_t result = minValue;
				std::optional<cpu_time_point> now = current_cpu_time();
				if(startTime.has_value() && now.has_value() && now.value() >= startTime.value()) [[likely]] {
					result = std::max(minValue, (std::size_t) (now.value() - startTime.value()));
				}
				startTime = now;
				return result;
			}


			void reset() {
				startTime = current_cpu_time();
			}

	};

}




#endif
