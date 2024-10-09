; Copyright (c) 2018-2019, tevador <tevador@gmail.com>
;
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
; 	* Redistributions of source code must retain the above copyright
; 	  notice, this list of conditions and the following disclaimer.
; 	* Redistributions in binary form must reproduce the above copyright
; 	  notice, this list of conditions and the following disclaimer in the
; 	  documentation and/or other materials provided with the distribution.
; 	* Neither the name of the copyright holder nor the
; 	  names of its contributors may be used to endorse or promote products
; 	  derived from this software without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
; ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
; FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
; DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
; SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
; CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
; OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
; OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

IFDEF RAX

_DEEPPREDICTOR_JITX86_STATIC SEGMENT PAGE READ EXECUTE

PUBLIC deeppredictor_prefetch_scratchpad
PUBLIC deeppredictor_prefetch_scratchpad_bmi2
PUBLIC deeppredictor_prefetch_scratchpad_end
PUBLIC deeppredictor_program_prologue
PUBLIC deeppredictor_program_prologue_first_load
PUBLIC deeppredictor_program_imul_rcp_store
PUBLIC deeppredictor_program_loop_begin
PUBLIC deeppredictor_program_loop_load
PUBLIC deeppredictor_program_loop_load_xop
PUBLIC deeppredictor_program_start
PUBLIC deeppredictor_program_read_dataset
PUBLIC deeppredictor_program_read_dataset_sshash_init
PUBLIC deeppredictor_program_read_dataset_sshash_fin
PUBLIC deeppredictor_dataset_init
PUBLIC deeppredictor_dataset_init_avx2_prologue
PUBLIC deeppredictor_dataset_init_avx2_loop_end
PUBLIC deeppredictor_dataset_init_avx2_epilogue
PUBLIC deeppredictor_dataset_init_avx2_ssh_load
PUBLIC deeppredictor_dataset_init_avx2_ssh_prefetch
PUBLIC deeppredictor_program_loop_store
PUBLIC deeppredictor_program_loop_end
PUBLIC deeppredictor_program_epilogue
PUBLIC deeppredictor_sshash_load
PUBLIC deeppredictor_sshash_prefetch
PUBLIC deeppredictor_sshash_end
PUBLIC deeppredictor_sshash_init
PUBLIC deeppredictor_program_end
PUBLIC deeppredictor_reciprocal_fast

DEEPPREDICTOR_SCRATCHPAD_MASK     EQU 2097088
DEEPPREDICTOR_DATASET_BASE_MASK   EQU 2147483584
DEEPPREDICTOR_CACHE_MASK          EQU 4194303

deeppredictor_prefetch_scratchpad PROC
	mov rdx, rax
	and eax, DEEPPREDICTOR_SCRATCHPAD_MASK
	prefetcht0 [rsi+rax]
	ror rdx, 32
	and edx, DEEPPREDICTOR_SCRATCHPAD_MASK
	prefetcht0 [rsi+rdx]
deeppredictor_prefetch_scratchpad ENDP

deeppredictor_prefetch_scratchpad_bmi2 PROC
	rorx rdx, rax, 32
	and eax, DEEPPREDICTOR_SCRATCHPAD_MASK
	prefetcht0 [rsi+rax]
	and edx, DEEPPREDICTOR_SCRATCHPAD_MASK
	prefetcht0 [rsi+rdx]
deeppredictor_prefetch_scratchpad_bmi2 ENDP

deeppredictor_prefetch_scratchpad_end PROC
deeppredictor_prefetch_scratchpad_end ENDP

ALIGN 64
deeppredictor_program_prologue PROC
	include asm/program_prologue_win64.inc
	movapd xmm13, xmmword ptr [mantissaMask]
	movapd xmm14, xmmword ptr [exp240]
	movapd xmm15, xmmword ptr [scaleMask]
deeppredictor_program_prologue ENDP

deeppredictor_program_prologue_first_load PROC
	mov rdx, rax
	and eax, DEEPPREDICTOR_SCRATCHPAD_MASK
	ror rdx, 32
	and edx, DEEPPREDICTOR_SCRATCHPAD_MASK
	sub rsp, 40
	mov dword ptr [rsp], 9FC0h
	mov dword ptr [rsp+4], 0BFC0h
	mov dword ptr [rsp+8], 0DFC0h
	mov dword ptr [rsp+12], 0FFC0h
	mov dword ptr [rsp+32], -1
	nop
	nop
	nop
	jmp deeppredictor_program_imul_rcp_store
deeppredictor_program_prologue_first_load ENDP

ALIGN 64
	include asm/program_xmm_constants.inc

deeppredictor_program_imul_rcp_store PROC
	include asm/program_imul_rcp_store.inc
	jmp deeppredictor_program_loop_begin
deeppredictor_program_imul_rcp_store ENDP

ALIGN 64
deeppredictor_program_loop_begin PROC
	nop
deeppredictor_program_loop_begin ENDP

deeppredictor_program_loop_load PROC
	include asm/program_loop_load.inc
deeppredictor_program_loop_load ENDP

deeppredictor_program_loop_load_xop PROC
	include asm/program_loop_load_xop.inc
deeppredictor_program_loop_load_xop ENDP

deeppredictor_program_start PROC
	nop
deeppredictor_program_start ENDP

deeppredictor_program_read_dataset PROC
	include asm/program_read_dataset.inc
deeppredictor_program_read_dataset ENDP

deeppredictor_program_read_dataset_sshash_init PROC
	include asm/program_read_dataset_sshash_init.inc
deeppredictor_program_read_dataset_sshash_init ENDP

deeppredictor_program_read_dataset_sshash_fin PROC
	include asm/program_read_dataset_sshash_fin.inc
deeppredictor_program_read_dataset_sshash_fin ENDP

deeppredictor_program_loop_store PROC
	include asm/program_loop_store.inc
deeppredictor_program_loop_store ENDP

deeppredictor_program_loop_end PROC
	nop
deeppredictor_program_loop_end ENDP

ALIGN 64
deeppredictor_dataset_init PROC
	push rbx
	push rbp
	push rdi
	push rsi
	push r12
	push r13
	push r14
	push r15
	mov rdi, qword ptr [rcx] ;# cache->memory
	mov rsi, rdx ;# dataset
	mov rbp, r8  ;# block index
	push r9      ;# max. block index
init_block_loop:
	prefetchw byte ptr [rsi]
	mov rbx, rbp
	db 232 ;# 0xE8 = call
	dd 32768 - distance
	distance equ $ - offset deeppredictor_dataset_init
	mov qword ptr [rsi+0], r8
	mov qword ptr [rsi+8], r9
	mov qword ptr [rsi+16], r10
	mov qword ptr [rsi+24], r11
	mov qword ptr [rsi+32], r12
	mov qword ptr [rsi+40], r13
	mov qword ptr [rsi+48], r14
	mov qword ptr [rsi+56], r15
	add rbp, 1
	add rsi, 64
	cmp rbp, qword ptr [rsp]
	jb init_block_loop
	pop r9
	pop r15
	pop r14
	pop r13
	pop r12
	pop rsi
	pop rdi
	pop rbp
	pop rbx
	ret
deeppredictor_dataset_init ENDP

ALIGN 64
deeppredictor_dataset_init_avx2_prologue PROC
	include asm/program_sshash_avx2_save_registers.inc

	mov rdi, qword ptr [rcx]		;# cache->memory
	mov rsi, rdx					;# dataset
	mov rbp, r8						;# block index
	push r9							;# max. block index
	sub rsp, 40

	jmp loop_begin
	include asm/program_sshash_avx2_constants.inc

ALIGN 64
loop_begin:
	include asm/program_sshash_avx2_loop_begin.inc

	;# init integer registers (lane 0)
	lea r8, [rbp+1]
	imul r8, qword ptr [r0_avx2_mul]
	mov r9, qword ptr [r1_avx2_add]
	xor r9, r8
	mov r10, qword ptr [r2_avx2_add]
	xor r10, r8
	mov r11, qword ptr [r3_avx2_add]
	xor r11, r8
	mov r12, qword ptr [r4_avx2_add]
	xor r12, r8
	mov r13, qword ptr [r5_avx2_add]
	xor r13, r8
	mov r14, qword ptr [r6_avx2_add]
	xor r14, r8
	mov r15, qword ptr [r7_avx2_add]
	xor r15, r8

	;# init AVX registers (lanes 1-4)
	mov qword ptr [rsp+32], rbp
	vbroadcastsd ymm0, qword ptr [rsp+32]
	vpaddq ymm0, ymm0, ymmword ptr [r0_avx2_increments]

	;# ymm0 *= r0_avx2_mul
	vbroadcastsd ymm1, qword ptr [r0_avx2_mul]
	vpsrlq ymm8, ymm0, 32
	vpsrlq ymm9, ymm1, 32
	vpmuludq ymm10, ymm0, ymm1
	vpmuludq ymm11, ymm9, ymm0
	vpmuludq ymm0, ymm8, ymm1
	vpsllq ymm11, ymm11, 32
	vpsllq ymm0, ymm0, 32
	vpaddq ymm10, ymm10, ymm11
	vpaddq ymm0, ymm10, ymm0

	vbroadcastsd ymm1, qword ptr [r1_avx2_add]
	vpxor ymm1, ymm0, ymm1
	vbroadcastsd ymm2, qword ptr [r2_avx2_add]
	vpxor ymm2, ymm0, ymm2
	vbroadcastsd ymm3, qword ptr [r3_avx2_add]
	vpxor ymm3, ymm0, ymm3
	vbroadcastsd ymm4, qword ptr [r4_avx2_add]
	vpxor ymm4, ymm0, ymm4
	vbroadcastsd ymm5, qword ptr [r5_avx2_add]
	vpxor ymm5, ymm0, ymm5
	vbroadcastsd ymm6, qword ptr [r6_avx2_add]
	vpxor ymm6, ymm0, ymm6
	vbroadcastsd ymm7, qword ptr [r7_avx2_add]
	vpxor ymm7, ymm0, ymm7

	vbroadcastsd ymm15, qword ptr [mul_hi_avx2_data] ;# carry_bit (bit 32)
	vpsllq ymm14, ymm15, 31                          ;# sign64 (bit 63)
deeppredictor_dataset_init_avx2_prologue ENDP

	;# generated SuperscalarHash code goes here

deeppredictor_dataset_init_avx2_loop_end PROC
	include asm/program_sshash_avx2_loop_end.inc
deeppredictor_dataset_init_avx2_loop_end ENDP

deeppredictor_dataset_init_avx2_epilogue PROC
	include asm/program_sshash_avx2_epilogue.inc
deeppredictor_dataset_init_avx2_epilogue ENDP

deeppredictor_dataset_init_avx2_ssh_load PROC
	include asm/program_sshash_avx2_ssh_load.inc
deeppredictor_dataset_init_avx2_ssh_load ENDP

deeppredictor_dataset_init_avx2_ssh_prefetch PROC
	include asm/program_sshash_avx2_ssh_prefetch.inc
deeppredictor_dataset_init_avx2_ssh_prefetch ENDP

deeppredictor_program_epilogue PROC
	include asm/program_epilogue_store.inc
	include asm/program_epilogue_win64.inc
deeppredictor_program_epilogue ENDP

ALIGN 64
deeppredictor_sshash_load PROC
	include asm/program_sshash_load.inc
deeppredictor_sshash_load ENDP

deeppredictor_sshash_prefetch PROC
	include asm/program_sshash_prefetch.inc
deeppredictor_sshash_prefetch ENDP

deeppredictor_sshash_end PROC
	nop
deeppredictor_sshash_end ENDP

ALIGN 64
deeppredictor_sshash_init PROC
	lea r8, [rbx+1]
	include asm/program_sshash_prefetch.inc
	imul r8, qword ptr [r0_mul]
	mov r9, qword ptr [r1_add]
	xor r9, r8
	mov r10, qword ptr [r2_add]
	xor r10, r8
	mov r11, qword ptr [r3_add]
	xor r11, r8
	mov r12, qword ptr [r4_add]
	xor r12, r8
	mov r13, qword ptr [r5_add]
	xor r13, r8
	mov r14, qword ptr [r6_add]
	xor r14, r8
	mov r15, qword ptr [r7_add]
	xor r15, r8
	jmp deeppredictor_program_end
deeppredictor_sshash_init ENDP

ALIGN 64
	include asm/program_sshash_constants.inc

ALIGN 64
deeppredictor_program_end PROC
	nop
deeppredictor_program_end ENDP

deeppredictor_reciprocal_fast PROC
	include asm/deeppredictor_reciprocal.inc
deeppredictor_reciprocal_fast ENDP

_DEEPPREDICTOR_JITX86_STATIC ENDS

ENDIF

END