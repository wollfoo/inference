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
#include <vector>
#include <type_traits>
#include "crypto/deeppredictor/common.hpp"
#include "crypto/deeppredictor/superscalar_program.hpp"
#include "crypto/deeppredictor/allocator.hpp"

/* Global scope for C binding */
struct deeppredictor_dataset {
	uint8_t* memory = nullptr;
};

/* Global scope for C binding */
struct deeppredictor_cache {
	uint8_t* memory = nullptr;
	deeppredictor::JitCompiler* jit = nullptr;
	deeppredictor::CacheInitializeFunc* initialize;
	deeppredictor::DatasetInitFunc* datasetInit;
	deeppredictor::SuperscalarProgram programs[DEEPPREDICTOR_CACHE_MAX_ACCESSES];

	bool isInitialized() const {
		return programs[0].getSize() != 0;
	}
};

//A pointer to a standard-layout struct object points to its initial member
static_assert(std::is_standard_layout<deeppredictor_dataset>(), "deeppredictor_dataset must be a standard-layout struct");
static_assert(std::is_standard_layout<deeppredictor_cache>(), "deeppredictor_cache must be a standard-layout struct");

namespace deeppredictor {

	using DefaultAllocator = AlignedAllocator<CacheLineSize>;

	template<class Allocator>
	void deallocDataset(deeppredictor_dataset* dataset) {
		if (dataset->memory != nullptr)
			Allocator::freeMemory(dataset->memory, DEEPPREDICTOR_DATASET_MAX_SIZE);
	}

	template<class Allocator>
	void deallocCache(deeppredictor_cache* cache);

	void initCache(deeppredictor_cache*, const void*, size_t);
	void initCacheCompile(deeppredictor_cache*, const void*, size_t);
	void initDatasetItem(deeppredictor_cache* cache, uint8_t* out, uint64_t blockNumber);
	void initDataset(deeppredictor_cache* cache, uint8_t* dataset, uint32_t startBlock, uint32_t endBlock);
}
