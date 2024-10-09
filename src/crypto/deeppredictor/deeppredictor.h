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

#ifndef DEEPPREDICTOR_H
#define DEEPPREDICTOR_H

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include "crypto/deeppredictor/intrin_portable.h"

#define DEEPPREDICTOR_HASH_SIZE 32
#define DEEPPREDICTOR_DATASET_ITEM_SIZE 64

#ifndef DEEPPREDICTOR_EXPORT
#define DEEPPREDICTOR_EXPORT
#endif


enum deeppredictor_flags {
  DEEPPREDICTOR_FLAG_DEFAULT = 0,
  DEEPPREDICTOR_FLAG_LARGE_PAGES = 1,
  DEEPPREDICTOR_FLAG_HARD_AES = 2,
  DEEPPREDICTOR_FLAG_FULL_MEM = 4,
  DEEPPREDICTOR_FLAG_JIT = 8,
  DEEPPREDICTOR_FLAG_1GB_PAGES = 16,
  DEEPPREDICTOR_FLAG_AMD = 64,
};


struct deeppredictor_dataset;
struct deeppredictor_cache;
class deeppredictor_vm;


struct DeeppredictoR_ConfigurationBase
{
	DeeppredictoR_ConfigurationBase();

	void Apply();

	// Common parameters for all DeeppredictoR variants
	enum Params : uint64_t
	{
		ArgonMemory = 262144,
		CacheAccesses = 8,
		SuperscalarMaxLatency = 170,
		DatasetBaseSize = 2147483648,
		DatasetExtraSize = 33554368,
		JumpBits = 8,
		JumpOffset = 8,
		CacheLineAlignMask_Calculated = (DatasetBaseSize - 1) & ~(DEEPPREDICTOR_DATASET_ITEM_SIZE - 1),
		DatasetExtraItems_Calculated = DatasetExtraSize / DEEPPREDICTOR_DATASET_ITEM_SIZE,
		ConditionMask_Calculated = ((1 << JumpBits) - 1) << JumpOffset,
	};

	uint32_t ArgonIterations;
	uint32_t ArgonLanes;
	const char* ArgonSalt;
	uint32_t SuperscalarLatency;

	uint32_t ScratchpadL1_Size;
	uint32_t ScratchpadL2_Size;
	uint32_t ScratchpadL3_Size;

	uint32_t ProgramSize;
	uint32_t ProgramIterations;
	uint32_t ProgramCount;

	uint32_t DEEPPREDICTOR_FREQ_IADD_RS;
	uint32_t DEEPPREDICTOR_FREQ_IADD_M;
	uint32_t DEEPPREDICTOR_FREQ_ISUB_R;
	uint32_t DEEPPREDICTOR_FREQ_ISUB_M;
	uint32_t DEEPPREDICTOR_FREQ_IMUL_R;
	uint32_t DEEPPREDICTOR_FREQ_IMUL_M;
	uint32_t DEEPPREDICTOR_FREQ_IMULH_R;
	uint32_t DEEPPREDICTOR_FREQ_IMULH_M;
	uint32_t DEEPPREDICTOR_FREQ_ISMULH_R;
	uint32_t DEEPPREDICTOR_FREQ_ISMULH_M;
	uint32_t DEEPPREDICTOR_FREQ_IMUL_RCP;
	uint32_t DEEPPREDICTOR_FREQ_INEG_R;
	uint32_t DEEPPREDICTOR_FREQ_IXOR_R;
	uint32_t DEEPPREDICTOR_FREQ_IXOR_M;
	uint32_t DEEPPREDICTOR_FREQ_IROR_R;
	uint32_t DEEPPREDICTOR_FREQ_IROL_R;
	uint32_t DEEPPREDICTOR_FREQ_ISWAP_R;
	uint32_t DEEPPREDICTOR_FREQ_FSWAP_R;
	uint32_t DEEPPREDICTOR_FREQ_FADD_R;
	uint32_t DEEPPREDICTOR_FREQ_FADD_M;
	uint32_t DEEPPREDICTOR_FREQ_FSUB_R;
	uint32_t DEEPPREDICTOR_FREQ_FSUB_M;
	uint32_t DEEPPREDICTOR_FREQ_FSCAL_R;
	uint32_t DEEPPREDICTOR_FREQ_FMUL_R;
	uint32_t DEEPPREDICTOR_FREQ_FDIV_M;
	uint32_t DEEPPREDICTOR_FREQ_FSQRT_R;
	uint32_t DEEPPREDICTOR_FREQ_CBRANCH;
	uint32_t DEEPPREDICTOR_FREQ_CFROUND;
	uint32_t DEEPPREDICTOR_FREQ_ISTORE;
	uint32_t DEEPPREDICTOR_FREQ_NOP;

	rx_vec_i128 fillAes4Rx4_Key[8];

	uint8_t codeSshPrefetchTweaked[20];
	uint8_t codePrefetchScratchpadTweaked[28];
	uint32_t codePrefetchScratchpadTweakedSize;

	uint32_t AddressMask_Calculated[4];
	uint32_t ScratchpadL3Mask_Calculated;
	uint32_t ScratchpadL3Mask64_Calculated;

#	if (XMRIG_ARM == 8)
	uint32_t Log2_ScratchpadL1;
	uint32_t Log2_ScratchpadL2;
	uint32_t Log2_ScratchpadL3;
	uint32_t Log2_DatasetBaseSize;
	uint32_t Log2_CacheSize;
#	endif
};

struct DeeppredictoR_ConfigurationMonero : public DeeppredictoR_ConfigurationBase {};
struct DeeppredictoR_ConfigurationWownero : public DeeppredictoR_ConfigurationBase { DeeppredictoR_ConfigurationWownero(); };
struct DeeppredictoR_ConfigurationArqma : public DeeppredictoR_ConfigurationBase { DeeppredictoR_ConfigurationArqma(); };
struct DeeppredictoR_ConfigurationGraft : public DeeppredictoR_ConfigurationBase { DeeppredictoR_ConfigurationGraft(); };
struct DeeppredictoR_ConfigurationSafex : public DeeppredictoR_ConfigurationBase { DeeppredictoR_ConfigurationSafex(); };
struct DeeppredictoR_ConfigurationYada : public DeeppredictoR_ConfigurationBase { DeeppredictoR_ConfigurationYada(); };

extern DeeppredictoR_ConfigurationMonero DeeppredictoR_MoneroConfig;
extern DeeppredictoR_ConfigurationWownero DeeppredictoR_WowneroConfig;
extern DeeppredictoR_ConfigurationArqma DeeppredictoR_ArqmaConfig;
extern DeeppredictoR_ConfigurationGraft DeeppredictoR_GraftConfig;
extern DeeppredictoR_ConfigurationSafex DeeppredictoR_SafexConfig;
extern DeeppredictoR_ConfigurationYada DeeppredictoR_YadaConfig;

extern DeeppredictoR_ConfigurationBase DeeppredictoR_CurrentConfig;

template<typename T>
void deeppredictor_apply_config(const T& config)
{
	static_assert(sizeof(T) == sizeof(DeeppredictoR_ConfigurationBase), "Invalid DeeppredictoR configuration struct size");
	static_assert(std::is_base_of<DeeppredictoR_ConfigurationBase, T>::value, "Incompatible DeeppredictoR configuration struct");
	DeeppredictoR_CurrentConfig = config;
	DeeppredictoR_CurrentConfig.Apply();
}

void deeppredictor_set_scratchpad_prefetch_mode(int mode);
void deeppredictor_set_huge_pages_jit(bool hugePages);
void deeppredictor_set_optimized_dataset_init(int value);

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Creates a deeppredictor_cache structure and allocates memory for DeeppredictoR Cache.
 *
 * @param flags is any combination of these 2 flags (each flag can be set or not set):
 *        DEEPPREDICTOR_FLAG_LARGE_PAGES - allocate memory in large pages
 *        DEEPPREDICTOR_FLAG_JIT - create cache structure with JIT compilation support; this makes
 *                           subsequent Dataset initialization faster
 *
 * @return Pointer to an allocated deeppredictor_cache structure.
 *         NULL is returned if memory allocation fails or if the DEEPPREDICTOR_FLAG_JIT
 *         is set and JIT compilation is not supported on the current platform.
 */
DEEPPREDICTOR_EXPORT deeppredictor_cache *deeppredictor_create_cache(deeppredictor_flags flags, uint8_t *memory);

/**
 * Initializes the cache memory and SuperscalarHash using the provided key value.
 *
 * @param cache is a pointer to a previously allocated deeppredictor_cache structure. Must not be NULL.
 * @param key is a pointer to memory which contains the key value. Must not be NULL.
 * @param keySize is the number of bytes of the key.
*/
DEEPPREDICTOR_EXPORT void deeppredictor_init_cache(deeppredictor_cache *cache, const void *key, size_t keySize);

/**
 * Releases all memory occupied by the deeppredictor_cache structure.
 *
 * @param cache is a pointer to a previously allocated deeppredictor_cache structure.
*/
DEEPPREDICTOR_EXPORT void deeppredictor_release_cache(deeppredictor_cache* cache);

/**
 * Creates a deeppredictor_dataset structure and allocates memory for DeeppredictoR Dataset.
 *
 * @param flags is the initialization flags. Only one flag is supported (can be set or not set):
 *        DEEPPREDICTOR_FLAG_LARGE_PAGES - allocate memory in large pages
 *
 * @return Pointer to an allocated deeppredictor_dataset structure.
 *         NULL is returned if memory allocation fails.
 */
DEEPPREDICTOR_EXPORT deeppredictor_dataset *deeppredictor_create_dataset(uint8_t *memory);

/**
 * Gets the number of items contained in the dataset.
 *
 * @return the number of items contained in the dataset.
*/
DEEPPREDICTOR_EXPORT unsigned long deeppredictor_dataset_item_count(void);

/**
 * Initializes dataset items.
 *
 * Note: In order to use the Dataset, all items from 0 to (deeppredictor_dataset_item_count() - 1) must be initialized.
 * This may be done by several calls to this function using non-overlapping item sequences.
 *
 * @param dataset is a pointer to a previously allocated deeppredictor_dataset structure. Must not be NULL.
 * @param cache is a pointer to a previously allocated and initialized deeppredictor_cache structure. Must not be NULL.
 * @param startItem is the item number where intialization should start.
 * @param itemCount is the number of items that should be initialized.
*/
DEEPPREDICTOR_EXPORT void deeppredictor_init_dataset(deeppredictor_dataset *dataset, deeppredictor_cache *cache, unsigned long startItem, unsigned long itemCount);

/**
 * Returns a pointer to the internal memory buffer of the dataset structure. The size
 * of the internal memory buffer is deeppredictor_dataset_item_count() * DEEPPREDICTOR_DATASET_ITEM_SIZE.
 *
 * @param dataset is dataset is a pointer to a previously allocated deeppredictor_dataset structure. Must not be NULL.
 *
 * @return Pointer to the internal memory buffer of the dataset structure.
*/
DEEPPREDICTOR_EXPORT void *deeppredictor_get_dataset_memory(deeppredictor_dataset *dataset);

/**
 * Releases all memory occupied by the deeppredictor_dataset structure.
 *
 * @param dataset is a pointer to a previously allocated deeppredictor_dataset structure.
*/
DEEPPREDICTOR_EXPORT void deeppredictor_release_dataset(deeppredictor_dataset *dataset);

/**
 * Creates and initializes a DeeppredictoR virtual machine.
 *
 * @param flags is any combination of these 4 flags (each flag can be set or not set):
 *        DEEPPREDICTOR_FLAG_LARGE_PAGES - allocate scratchpad memory in large pages
 *        DEEPPREDICTOR_FLAG_HARD_AES - virtual machine will use hardware accelerated AES
 *        DEEPPREDICTOR_FLAG_FULL_MEM - virtual machine will use the full dataset
 *        DEEPPREDICTOR_FLAG_JIT - virtual machine will use a JIT compiler
 *        The numeric values of the flags are ordered so that a higher value will provide
 *        faster hash calculation and a lower numeric value will provide higher portability.
 *        Using DEEPPREDICTOR_FLAG_DEFAULT (all flags not set) works on all platforms, but is the slowest.
 * @param cache is a pointer to an initialized deeppredictor_cache structure. Can be
 *        NULL if DEEPPREDICTOR_FLAG_FULL_MEM is set.
 * @param dataset is a pointer to a deeppredictor_dataset structure. Can be NULL
 *        if DEEPPREDICTOR_FLAG_FULL_MEM is not set.
 *
 * @return Pointer to an initialized deeppredictor_vm structure.
 *         Returns NULL if:
 *         (1) Scratchpad memory allocation fails.
 *         (2) The requested initialization flags are not supported on the current platform.
 *         (3) cache parameter is NULL and DEEPPREDICTOR_FLAG_FULL_MEM is not set
 *         (4) dataset parameter is NULL and DEEPPREDICTOR_FLAG_FULL_MEM is set
*/
DEEPPREDICTOR_EXPORT deeppredictor_vm *deeppredictor_create_vm(deeppredictor_flags flags, deeppredictor_cache *cache, deeppredictor_dataset *dataset, uint8_t *scratchpad, uint32_t node);

/**
 * Reinitializes a virtual machine with a new Cache. This function should be called anytime
 * the Cache is reinitialized with a new key.
 *
 * @param machine is a pointer to a deeppredictor_vm structure that was initialized
 *        without DEEPPREDICTOR_FLAG_FULL_MEM. Must not be NULL.
 * @param cache is a pointer to an initialized deeppredictor_cache structure. Must not be NULL.
*/
DEEPPREDICTOR_EXPORT void deeppredictor_vm_set_cache(deeppredictor_vm *machine, deeppredictor_cache* cache);

/**
 * Reinitializes a virtual machine with a new Dataset.
 *
 * @param machine is a pointer to a deeppredictor_vm structure that was initialized
 *        with DEEPPREDICTOR_FLAG_FULL_MEM. Must not be NULL.
 * @param dataset is a pointer to an initialized deeppredictor_dataset structure. Must not be NULL.
*/
DEEPPREDICTOR_EXPORT void deeppredictor_vm_set_dataset(deeppredictor_vm *machine, deeppredictor_dataset *dataset);

/**
 * Releases all memory occupied by the deeppredictor_vm structure.
 *
 * @param machine is a pointer to a previously created deeppredictor_vm structure.
*/
DEEPPREDICTOR_EXPORT void deeppredictor_destroy_vm(deeppredictor_vm *machine);

/**
 * Calculates a DeeppredictoR hash value.
 *
 * @param machine is a pointer to a deeppredictor_vm structure. Must not be NULL.
 * @param input is a pointer to memory to be hashed. Must not be NULL.
 * @param inputSize is the number of bytes to be hashed.
 * @param output is a pointer to memory where the hash will be stored. Must not
 *        be NULL and at least DEEPPREDICTOR_HASH_SIZE bytes must be available for writing.
*/
DEEPPREDICTOR_EXPORT void deeppredictor_calculate_hash(deeppredictor_vm *machine, const void *input, size_t inputSize, void *output);

DEEPPREDICTOR_EXPORT void deeppredictor_calculate_hash_first(deeppredictor_vm* machine, uint64_t (&tempHash)[8], const void* input, size_t inputSize);
DEEPPREDICTOR_EXPORT void deeppredictor_calculate_hash_next(deeppredictor_vm* machine, uint64_t (&tempHash)[8], const void* nextInput, size_t nextInputSize, void* output);

#if defined(__cplusplus)
}
#endif

#endif
