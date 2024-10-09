/*
Copyright (c) 2019 SChernykh

This file is part of DeeppredictoR OpenCL.

DeeppredictoR OpenCL is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

DeeppredictoR OpenCL is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with DeeppredictoR OpenCL. If not, see <http://www.gnu.org/licenses/>.
*/

//Dataset base size in bytes. Must be a power of 2.
#define DEEPPREDICTOR_DATASET_BASE_SIZE  2147483648

//Dataset extra size. Must be divisible by 64.
#define DEEPPREDICTOR_DATASET_EXTRA_SIZE 33554368

//Scratchpad L3 size in bytes. Must be a power of 2.
#define DEEPPREDICTOR_SCRATCHPAD_L3      262144

//Scratchpad L2 size in bytes. Must be a power of two and less than or equal to DEEPPREDICTOR_SCRATCHPAD_L3.
#define DEEPPREDICTOR_SCRATCHPAD_L2      131072

//Scratchpad L1 size in bytes. Must be a power of two (minimum 64) and less than or equal to DEEPPREDICTOR_SCRATCHPAD_L2.
#define DEEPPREDICTOR_SCRATCHPAD_L1      16384

//Jump condition mask size in bits.
#define DEEPPREDICTOR_JUMP_BITS          8

//Jump condition mask offset in bits. The sum of DEEPPREDICTOR_JUMP_BITS and DEEPPREDICTOR_JUMP_OFFSET must not exceed 16.
#define DEEPPREDICTOR_JUMP_OFFSET        8

//Integer instructions
#define DEEPPREDICTOR_FREQ_IADD_RS       16
#define DEEPPREDICTOR_FREQ_IADD_M         7
#define DEEPPREDICTOR_FREQ_ISUB_R        16
#define DEEPPREDICTOR_FREQ_ISUB_M         7
#define DEEPPREDICTOR_FREQ_IMUL_R        16
#define DEEPPREDICTOR_FREQ_IMUL_M         4
#define DEEPPREDICTOR_FREQ_IMULH_R        4
#define DEEPPREDICTOR_FREQ_IMULH_M        1
#define DEEPPREDICTOR_FREQ_ISMULH_R       4
#define DEEPPREDICTOR_FREQ_ISMULH_M       1
#define DEEPPREDICTOR_FREQ_IMUL_RCP       8
#define DEEPPREDICTOR_FREQ_INEG_R         2
#define DEEPPREDICTOR_FREQ_IXOR_R        15
#define DEEPPREDICTOR_FREQ_IXOR_M         5
#define DEEPPREDICTOR_FREQ_IROR_R         8
#define DEEPPREDICTOR_FREQ_IROL_R         2
#define DEEPPREDICTOR_FREQ_ISWAP_R        4

//Floating point instructions
#define DEEPPREDICTOR_FREQ_FSWAP_R        4
#define DEEPPREDICTOR_FREQ_FADD_R        16
#define DEEPPREDICTOR_FREQ_FADD_M         5
#define DEEPPREDICTOR_FREQ_FSUB_R        16
#define DEEPPREDICTOR_FREQ_FSUB_M         5
#define DEEPPREDICTOR_FREQ_FSCAL_R        6
#define DEEPPREDICTOR_FREQ_FMUL_R        32
#define DEEPPREDICTOR_FREQ_FDIV_M         4
#define DEEPPREDICTOR_FREQ_FSQRT_R        6

//Control instructions
#define DEEPPREDICTOR_FREQ_CBRANCH       25
#define DEEPPREDICTOR_FREQ_CFROUND        1

//Store instruction
#define DEEPPREDICTOR_FREQ_ISTORE        16

//No-op instruction
#define DEEPPREDICTOR_FREQ_NOP            0

#define DEEPPREDICTOR_DATASET_ITEM_SIZE 64

#define DEEPPREDICTOR_PROGRAM_SIZE 256

#define HASH_SIZE 64
#define ENTROPY_SIZE (128 + DEEPPREDICTOR_PROGRAM_SIZE * 8)
#define REGISTERS_SIZE 256
#define IMM_BUF_SIZE (DEEPPREDICTOR_PROGRAM_SIZE * 4 - REGISTERS_SIZE)
#define IMM_INDEX_COUNT ((IMM_BUF_SIZE / 4) - 2)
#define VM_STATE_SIZE (REGISTERS_SIZE + IMM_BUF_SIZE + DEEPPREDICTOR_PROGRAM_SIZE * 4)
#define ROUNDING_MODE (DEEPPREDICTOR_FREQ_CFROUND ? -1 : 0)

// Scratchpad L1/L2/L3 bits
#define LOC_L1 (32 - 14)
#define LOC_L2 (32 - 17)
#define LOC_L3 (32 - 18)
