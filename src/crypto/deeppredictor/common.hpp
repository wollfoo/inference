/*
Copyright (c) 2018-2019, tevador <tevador@gmail.com>

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
	* Redistributions of source code must retain the above copyright
	  notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright
	  notice, this list of conditions and the following disclaimer in the
	  documentation and/or other materials provided with the distribution.
	* Neither the name of the copyright holder nor the
	  names of its contributors may be used to endorse or promote products
	  derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <cstdint>
#include <iostream>
#include <climits>
#include "crypto/deeppredictor/blake2/endian.h"
#include "crypto/deeppredictor/configuration.h"
#include "crypto/deeppredictor/deeppredictor.h"

namespace deeppredictor {

	//static_assert(DEEPPREDICTOR_ARGON_MEMORY > 0, "DEEPPREDICTOR_ARGON_MEMORY must be greater than 0.");
	//static_assert((DEEPPREDICTOR_ARGON_MEMORY & (DEEPPREDICTOR_ARGON_MEMORY - 1)) == 0, "DEEPPREDICTOR_ARGON_MEMORY must be a power of 2.");
	//static_assert(DEEPPREDICTOR_DATASET_BASE_SIZE >= 64, "DEEPPREDICTOR_DATASET_BASE_SIZE must be at least 64.");
	//static_assert((DEEPPREDICTOR_DATASET_BASE_SIZE & (DEEPPREDICTOR_DATASET_BASE_SIZE - 1)) == 0, "DEEPPREDICTOR_DATASET_BASE_SIZE must be a power of 2.");
	//static_assert(DEEPPREDICTOR_DATASET_BASE_SIZE <= 4294967296ULL, "DEEPPREDICTOR_DATASET_BASE_SIZE must not exceed 4294967296.");
	//static_assert(DEEPPREDICTOR_DATASET_EXTRA_SIZE % 64 == 0, "DEEPPREDICTOR_DATASET_EXTRA_SIZE must be divisible by 64.");
	//static_assert((uint64_t)DEEPPREDICTOR_DATASET_BASE_SIZE + DEEPPREDICTOR_DATASET_EXTRA_SIZE <= 17179869184, "Dataset size must not exceed 16 GiB.");
	//static_assert(DEEPPREDICTOR_PROGRAM_SIZE > 0, "DEEPPREDICTOR_PROGRAM_SIZE must be greater than 0");
	//static_assert(DEEPPREDICTOR_PROGRAM_ITERATIONS > 0, "DEEPPREDICTOR_PROGRAM_ITERATIONS must be greater than 0");
	//static_assert(DEEPPREDICTOR_PROGRAM_COUNT > 0, "DEEPPREDICTOR_PROGRAM_COUNT must be greater than 0");
	//static_assert((DEEPPREDICTOR_SCRATCHPAD_L3 & (DEEPPREDICTOR_SCRATCHPAD_L3 - 1)) == 0, "DEEPPREDICTOR_SCRATCHPAD_L3 must be a power of 2.");
	//static_assert(DEEPPREDICTOR_SCRATCHPAD_L3 >= DEEPPREDICTOR_SCRATCHPAD_L2, "DEEPPREDICTOR_SCRATCHPAD_L3 must be greater than or equal to DEEPPREDICTOR_SCRATCHPAD_L2.");
	//static_assert((DEEPPREDICTOR_SCRATCHPAD_L2 & (DEEPPREDICTOR_SCRATCHPAD_L2 - 1)) == 0, "DEEPPREDICTOR_SCRATCHPAD_L2 must be a power of 2.");
	//static_assert(DEEPPREDICTOR_SCRATCHPAD_L2 >= DEEPPREDICTOR_SCRATCHPAD_L1, "DEEPPREDICTOR_SCRATCHPAD_L2 must be greater than or equal to DEEPPREDICTOR_SCRATCHPAD_L1.");
	//static_assert(DEEPPREDICTOR_SCRATCHPAD_L1 >= 64, "DEEPPREDICTOR_SCRATCHPAD_L1 must be at least 64.");
	//static_assert((DEEPPREDICTOR_SCRATCHPAD_L1 & (DEEPPREDICTOR_SCRATCHPAD_L1 - 1)) == 0, "DEEPPREDICTOR_SCRATCHPAD_L1 must be a power of 2.");
	//static_assert(DEEPPREDICTOR_CACHE_ACCESSES > 1, "DEEPPREDICTOR_CACHE_ACCESSES must be greater than 1");
	//static_assert(DEEPPREDICTOR_SUPERSCALAR_LATENCY > 0, "DEEPPREDICTOR_SUPERSCALAR_LATENCY must be greater than 0");
	//static_assert(DEEPPREDICTOR_JUMP_BITS > 0, "DEEPPREDICTOR_JUMP_BITS must be greater than 0.");
	//static_assert(DEEPPREDICTOR_JUMP_OFFSET >= 0, "DEEPPREDICTOR_JUMP_OFFSET must be greater than or equal to 0.");
	//static_assert(DEEPPREDICTOR_JUMP_BITS + DEEPPREDICTOR_JUMP_OFFSET <= 16, "DEEPPREDICTOR_JUMP_BITS + DEEPPREDICTOR_JUMP_OFFSET must not exceed 16.");

	/*constexpr int wtSum = DEEPPREDICTOR_FREQ_IADD_RS + DEEPPREDICTOR_FREQ_IADD_M + DEEPPREDICTOR_FREQ_ISUB_R + \
		DEEPPREDICTOR_FREQ_ISUB_M + DEEPPREDICTOR_FREQ_IMUL_R + DEEPPREDICTOR_FREQ_IMUL_M + DEEPPREDICTOR_FREQ_IMULH_R + \
		DEEPPREDICTOR_FREQ_IMULH_M + DEEPPREDICTOR_FREQ_ISMULH_R + DEEPPREDICTOR_FREQ_ISMULH_M + DEEPPREDICTOR_FREQ_IMUL_RCP + \
		DEEPPREDICTOR_FREQ_INEG_R + DEEPPREDICTOR_FREQ_IXOR_R + DEEPPREDICTOR_FREQ_IXOR_M + DEEPPREDICTOR_FREQ_IROR_R + DEEPPREDICTOR_FREQ_IROL_R + DEEPPREDICTOR_FREQ_ISWAP_R + \
		DEEPPREDICTOR_FREQ_FSWAP_R + DEEPPREDICTOR_FREQ_FADD_R + DEEPPREDICTOR_FREQ_FADD_M + DEEPPREDICTOR_FREQ_FSUB_R + DEEPPREDICTOR_FREQ_FSUB_M + \
		DEEPPREDICTOR_FREQ_FSCAL_R + DEEPPREDICTOR_FREQ_FMUL_R + DEEPPREDICTOR_FREQ_FDIV_M + DEEPPREDICTOR_FREQ_FSQRT_R + DEEPPREDICTOR_FREQ_CBRANCH + \
		DEEPPREDICTOR_FREQ_CFROUND + DEEPPREDICTOR_FREQ_ISTORE + DEEPPREDICTOR_FREQ_NOP;*/

	//static_assert(wtSum == 256,	"Sum of instruction frequencies must be 256.");


	constexpr uint32_t ArgonBlockSize = 1024;
	constexpr int SuperscalarMaxSize = 3 * DEEPPREDICTOR_SUPERSCALAR_MAX_LATENCY + 2;
	constexpr size_t CacheLineSize = DEEPPREDICTOR_DATASET_ITEM_SIZE;
	#define ScratchpadSize DeeppredictoR_CurrentConfig.ScratchpadL3_Size
	#define CacheLineAlignMask DeeppredictoR_ConfigurationBase::CacheLineAlignMask_Calculated
	#define DatasetExtraItems DeeppredictoR_ConfigurationBase::DatasetExtraItems_Calculated
	constexpr int StoreL3Condition = 14;

	//Prevent some unsafe configurations.
#ifndef DEEPPREDICTOR_UNSAFE
	//static_assert((uint64_t)ArgonBlockSize * DEEPPREDICTOR_CACHE_ACCESSES * DEEPPREDICTOR_ARGON_MEMORY + 33554432 >= (uint64_t)DEEPPREDICTOR_DATASET_BASE_SIZE + DEEPPREDICTOR_DATASET_EXTRA_SIZE, "Unsafe configuration: Memory-time tradeoffs");
	//static_assert((128 + DEEPPREDICTOR_PROGRAM_SIZE * DEEPPREDICTOR_FREQ_ISTORE / 256) * (DEEPPREDICTOR_PROGRAM_COUNT * DEEPPREDICTOR_PROGRAM_ITERATIONS) >= DEEPPREDICTOR_SCRATCHPAD_L3, "Unsafe configuration: Insufficient Scratchpad writes");
	//static_assert(DEEPPREDICTOR_PROGRAM_COUNT > 1, "Unsafe configuration: Program filtering strategies");
	//static_assert(DEEPPREDICTOR_PROGRAM_SIZE >= 64, "Unsafe configuration: Low program entropy");
	//static_assert(DEEPPREDICTOR_PROGRAM_ITERATIONS >= 400, "Unsafe configuration: High compilation overhead");
#endif

#ifdef TRACE
	constexpr bool trace = true;
#else
	constexpr bool trace = false;
#endif

#ifndef UNREACHABLE
#ifdef __GNUC__
#define UNREACHABLE __builtin_unreachable()
#elif _MSC_VER
#define UNREACHABLE __assume(false)
#else
#define UNREACHABLE
#endif
#endif

#if defined(XMRIG_FEATURE_ASM) && (defined(_M_X64) || defined(__x86_64__))
	#define DEEPPREDICTOR_HAVE_COMPILER 1
	class JitCompilerX86;
	using JitCompiler = JitCompilerX86;
#elif defined(__aarch64__)
	#define DEEPPREDICTOR_HAVE_COMPILER 1
	class JitCompilerA64;
	using JitCompiler = JitCompilerA64;
#else
	#define DEEPPREDICTOR_HAVE_COMPILER 0
	class JitCompilerFallback;
	using JitCompiler = JitCompilerFallback;
#endif

	using addr_t = uint32_t;

	using int_reg_t = uint64_t;

	struct fpu_reg_t {
		double lo;
		double hi;
	};

	#define AddressMask DeeppredictoR_CurrentConfig.AddressMask_Calculated
	#define ScratchpadL3Mask DeeppredictoR_CurrentConfig.ScratchpadL3Mask_Calculated
	#define ScratchpadL3Mask64 DeeppredictoR_CurrentConfig.ScratchpadL3Mask64_Calculated
	constexpr int RegistersCount = 8;
	constexpr int RegisterCountFlt = RegistersCount / 2;
	constexpr int RegisterNeedsDisplacement = 5; //x86 r13 register
	constexpr int RegisterNeedsSib = 4; //x86 r12 register

	inline bool isZeroOrPowerOf2(uint64_t x) {
		return (x & (x - 1)) == 0;
	}

	constexpr int mantissaSize = 52;
	constexpr int exponentSize = 11;
	constexpr uint64_t mantissaMask = (1ULL << mantissaSize) - 1;
	constexpr uint64_t exponentMask = (1ULL << exponentSize) - 1;
	constexpr int exponentBias = 1023;
	constexpr int dynamicExponentBits = 4;
	constexpr int staticExponentBits = 4;
	constexpr uint64_t constExponentBits = 0x300;
	constexpr uint64_t dynamicMantissaMask = (1ULL << (mantissaSize + dynamicExponentBits)) - 1;

	struct MemoryRegisters {
		addr_t mx, ma;
		uint8_t* memory = nullptr;
	};

	//register file in little-endian byte order
	struct RegisterFile {
		int_reg_t r[RegistersCount];
		fpu_reg_t f[RegisterCountFlt];
		fpu_reg_t e[RegisterCountFlt];
		fpu_reg_t a[RegisterCountFlt];
	};

	typedef void(ProgramFunc)(RegisterFile&, MemoryRegisters&, uint8_t* /* scratchpad */, uint64_t);
	typedef void(DatasetInitFunc)(deeppredictor_cache* cache, uint8_t* dataset, uint32_t startBlock, uint32_t endBlock);

	typedef void(CacheInitializeFunc)(deeppredictor_cache*, const void*, size_t);
}
