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

extern "C" {
	void deeppredictor_prefetch_scratchpad();
	void deeppredictor_prefetch_scratchpad_bmi2();
	void deeppredictor_prefetch_scratchpad_end();
	void deeppredictor_program_prologue();
	void deeppredictor_program_prologue_first_load();
	void deeppredictor_program_imul_rcp_store();
	void deeppredictor_program_loop_begin();
	void deeppredictor_program_loop_load();
	void deeppredictor_program_loop_load_xop();
	void deeppredictor_program_start();
	void deeppredictor_program_read_dataset();
	void deeppredictor_program_read_dataset_sshash_init();
	void deeppredictor_program_read_dataset_sshash_fin();
	void deeppredictor_program_loop_store();
	void deeppredictor_program_loop_end();
	void deeppredictor_dataset_init();
	void deeppredictor_dataset_init_avx2_prologue();
	void deeppredictor_dataset_init_avx2_loop_end();
	void deeppredictor_dataset_init_avx2_epilogue();
	void deeppredictor_dataset_init_avx2_ssh_load();
	void deeppredictor_dataset_init_avx2_ssh_prefetch();
	void deeppredictor_program_epilogue();
	void deeppredictor_sshash_load();
	void deeppredictor_sshash_prefetch();
	void deeppredictor_sshash_end();
	void deeppredictor_sshash_init();
	void deeppredictor_program_end();
}
