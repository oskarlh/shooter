#ifndef OSKAR_COMPILATION_TARGET_INFO_H
#define OSKAR_COMPILATION_TARGET_INFO_H

namespace oskar::compilation_target_info {
	constexpr bool compiler_gcc_compatible = false
		#if defined(__GNUC__)
			|| true
		#endif
	;
	constexpr bool compiler_msvc_compatible = false
		#if defined(_MSC_VER)
			|| true
		#endif
	;

	constexpr bool compiler_gcc = false
		#if defined(__GNUG__)
			|| true
		#endif
	;

	constexpr bool compiler_clang = false
		#if defined(__clang__)
			|| true
		#endif
	;

	constexpr bool compiler_icc = false
		#if defined(__INTEL_COMPILER)
			|| true
		#endif
	;

	constexpr bool compiler_msvc = compiler_msvc_compatible && !compiler_icc;


	// Macro source: https://sourceforge.net/p/predef/wiki/Architectures/
	// Macro source: https://msdn.microsoft.com/en-us/library/b0084kay.aspx

	constexpr bool architecture_arm64 = false
		#if defined(__aarch64__) || defined(_M_ARM64) || defined(__arm64)
			|| architecture_arm
		#endif
	;
	constexpr bool architecture_arm = architecture_arm64
		#if ( \
			defined(__arm__) || defined(__thumb__) || defined(_ARM) || defined(_M_ARM) \
			|| defined(_M_ARMT) || defined(__arm) \
		)
			|| true
		#endif
	;
	
	constexpr bool architecture_powerpc = false
		#if ( \
			defined(__powerpc) || defined(__powerpc__) || defined(__POWERPC__) || defined(__ppc__) \
			|| defined(__PPC__) || defined(_ARCH_PPC) || defined(_M_PPC) || defined(_ARCH_PPC64) \
			|| defined(__PPCGECKO__) || defined(__PPCBROADWAY__) || defined(_XENON) || defined(__ppc) \
		)
			|| true
		#endif
	;
	
	constexpr bool architecture_x86_64 = false
		#if defined(_M_AMD64) || defined(__x86_64__) || defined(__amd64__)
			|| true
		#endif
	;
	constexpr bool architecture_x86_32 = architecture_x86_64
		#if defined(__i386) || defined(_M_IX86) || defined(_X86_) || defined(__i386__)
			|| true
		#endif
	;

	constexpr bool memory_order_acq_rel_likely_generates_fence = architecture_arm || architecture_powerpc;
	constexpr bool memory_order_acq_rel_likely_generates_no_fence = architecture_x86_32;

	static_assert(
		architecture_arm + architecture_powerpc + architecture_x86_32 <= 1,
		"Architecture detection failure: multiple independent architectures were detected."
	);
}


/*
std::memory_order_relaxed

std::memory_order_acq_rel
*/
#endif
