---------------------------------------------
cvt(std::array<float, 4ul> const&):
	vmovups	xmm0, XMMWORD PTR [rdi]
	jmp	void g<std::basic_simd<float, std::_VecAbi<4> > >(std::basic_simd<float, std::_VecAbi<4> >)
---------------------------------------------
cvt():
	vmovdqa	ymm0, YMMWORD PTR .LC0[rip]
	jmp	void g<std::basic_simd<int, std::_VecAbi<5> > >(std::basic_simd<int, std::_VecAbi<5> >)
---------------------------------------------
cvt2():
	vmovdqa	ymm0, YMMWORD PTR .LC0[rip]
	jmp	void g<std::basic_simd<int, std::_VecAbi<5> > >(std::basic_simd<int, std::_VecAbi<5> >)
---------------------------------------------
load_ub(std::vector<float, std::allocator<float> > const&):
	mov	rax, QWORD PTR [rdi]
	vmovups	ymm0, YMMWORD PTR [rax]
	jmp	void g<std::basic_simd<float, std::_VecAbi<8> > >(std::basic_simd<float, std::_VecAbi<8> >)
---------------------------------------------
load_zero(std::vector<float, std::allocator<float> > const&):
	mov	rdx, QWORD PTR [rdi]
	mov	rax, QWORD PTR [rdi+8] 	#
	sub	rax, rdx
	cmp	rax, 28 	# 0x000000000000001c
	ja	.L26
	push	r13
	sar	rax, 2 	#
	mov	ecx, 8 	#
	vxorps	xmm0, xmm0, xmm0
	sub	rcx, rax
	mov	rax, rdx
	lea	r13, [rsp+16] 	#
	and	rsp, -32 	# 0xffffffffffffffe0
	push	QWORD PTR [r13-8] 	# 0xfffffffffffffff8
	push	rbp
	mov	rbp, rsp
	push	r13
	lea	rsi, [rbp-48] 	# 0xffffffffffffffd0
	vmovaps	YMMWORD PTR [rbp-48], ymm0 	# 0xffffffffffffffd0
	cmp	ecx, 8 	#
	jnb	.L27
---------------------------------------------
.L7:
	xor	edx, edx
	test	cl, 4 	#
	jne	.L28
	test	cl, 2 	#
	jne	.L29
---------------------------------------------
.L11:
	and	ecx, 1 	#
	jne	.L30
---------------------------------------------
.L12:
	vmovaps	ymm0, YMMWORD PTR [rbp-48] 	# 0xffffffffffffffd0
	mov	r13, QWORD PTR [rbp-8] 	# 0xfffffffffffffff8
	leave
	lea	rsp, [r13-16] 	# 0xfffffffffffffff0
	pop	r13
	jmp	void g<std::basic_simd<float, std::_VecAbi<8> > >(std::basic_simd<float, std::_VecAbi<8> >)
---------------------------------------------
.L26:
	vmovups	ymm0, YMMWORD PTR [rdx]
	jmp	void g<std::basic_simd<float, std::_VecAbi<8> > >(std::basic_simd<float, std::_VecAbi<8> >)
---------------------------------------------
.L30:
	movzx	eax, BYTE PTR [rax+rdx]
	mov	BYTE PTR [rsi+rdx], al
	jmp	.L12
---------------------------------------------
.L29:
	movzx	edi, WORD PTR [rax+rdx]
	mov	WORD PTR [rsi+rdx], di
	add	rdx, 2 	#
	and	ecx, 1 	#
	je	.L12
	jmp	.L30
---------------------------------------------
.L28:
	mov	edx, DWORD PTR [rax]
	mov	DWORD PTR [rsi], edx
	mov	edx, 4 	#
	test	cl, 2 	#
	je	.L11
	jmp	.L29
---------------------------------------------
.L27:
	mov	r8d, ecx
	xor	eax, eax
	and	r8d, -8 	# 0xfffffffffffffff8
---------------------------------------------
.L8:
	mov	esi, eax
	add	eax, 8 	#
	mov	rdi, QWORD PTR [rdx+rsi]
	mov	QWORD PTR [rbp-48+rsi], rdi 	# 0xffffffffffffffd0
	cmp	eax, r8d
	jb	.L8
	lea	rsi, [rbp-48] 	# 0xffffffffffffffd0
	add	rsi, rax
	add	rax, rdx
	jmp	.L7
---------------------------------------------
iter_ub1(std::vector<float, std::allocator<float> > const&):
	mov	rax, QWORD PTR [rdi]
	mov	rdx, QWORD PTR [rdi+8] 	#
	vxorps	xmm0, xmm0, xmm0
	cmp	rax, rdx
	jnb	.L32
---------------------------------------------
.L33:
	vaddps	ymm0, ymm0, YMMWORD PTR [rax]
	add	rax, 32 	# 0x0000000000000020
	cmp	rax, rdx
	jb	.L33
---------------------------------------------
.L32:
	jmp	void g<std::basic_simd<float, std::_VecAbi<8> > >(std::basic_simd<float, std::_VecAbi<8> >)
---------------------------------------------
iter_ub2(std::vector<float, std::allocator<float> > const&):
	mov	rax, QWORD PTR [rdi]
	mov	rdx, QWORD PTR [rdi+8] 	#
	vxorps	xmm0, xmm0, xmm0
	cmp	rax, rdx
	jnb	.L37
---------------------------------------------
.L38:
	vaddps	ymm0, ymm0, YMMWORD PTR [rax]
	add	rax, 32 	# 0x0000000000000020
	cmp	rax, rdx
	jb	.L38
---------------------------------------------
.L37:
	jmp	void g<std::basic_simd<float, std::_VecAbi<8> > >(std::basic_simd<float, std::_VecAbi<8> >)
---------------------------------------------
iter_zero1(std::vector<float, std::allocator<float> > const&):
	mov	rax, QWORD PTR [rdi]
	mov	rcx, QWORD PTR [rdi+8] 	#
	vxorps	xmm0, xmm0, xmm0
	cmp	rax, rcx
	jnb	.L68
	vmovaps	ymm2, ymm0
	mov	r10d, 8 	#
---------------------------------------------
.L65:
	mov	rdx, rcx
	sub	rdx, rax
	cmp	rdx, 28 	# 0x000000000000001c
	jle	.L69
	vmovups	ymm1, YMMWORD PTR [rax]
	add	rax, 32 	# 0x0000000000000020
	vaddps	ymm0, ymm0, ymm1
	cmp	rax, rcx
	jb	.L65
---------------------------------------------
.L68:
	jmp	void g<std::basic_simd<float, std::_VecAbi<8> > >(std::basic_simd<float, std::_VecAbi<8> >)
---------------------------------------------
.L69:
	push	r13
	lea	r13, [rsp+16] 	#
	and	rsp, -32 	# 0xffffffffffffffe0
	push	QWORD PTR [r13-8] 	# 0xfffffffffffffff8
	push	rbp
	mov	rbp, rsp
	push	r13
---------------------------------------------
.L43:
	sar	rdx, 2 	#
	mov	rsi, r10
	vmovaps	YMMWORD PTR [rbp-48], ymm2 	# 0xffffffffffffffd0
	lea	r8, [rbp-48] 	# 0xffffffffffffffd0
	sub	rsi, rdx
	mov	rdx, rax
	cmp	esi, 8 	#
	jnb	.L70
---------------------------------------------
.L45:
	xor	edi, edi
	test	sil, 4 	#
	je	.L48
	mov	edi, DWORD PTR [rdx]
	mov	DWORD PTR [r8], edi
	mov	edi, 4 	#
---------------------------------------------
.L48:
	test	sil, 2 	#
	je	.L49
	movzx	r9d, WORD PTR [rdx+rdi]
	mov	WORD PTR [r8+rdi], r9w
	add	rdi, 2 	#
---------------------------------------------
.L49:
	and	esi, 1 	#
	je	.L50
	movzx	edx, BYTE PTR [rdx+rdi]
	mov	BYTE PTR [r8+rdi], dl
---------------------------------------------
.L50:
	vmovaps	ymm1, YMMWORD PTR [rbp-48] 	# 0xffffffffffffffd0
	add	rax, 32 	# 0x0000000000000020
	vaddps	ymm0, ymm0, ymm1
	cmp	rax, rcx
	jnb	.L71
---------------------------------------------
.L51:
	mov	rdx, rcx
	sub	rdx, rax
	cmp	rdx, 28 	# 0x000000000000001c
	jle	.L43
	vmovups	ymm1, YMMWORD PTR [rax]
	add	rax, 32 	# 0x0000000000000020
	vaddps	ymm0, ymm0, ymm1
	cmp	rax, rcx
	jb	.L51
---------------------------------------------
.L71:
	mov	r13, QWORD PTR [rbp-8] 	# 0xfffffffffffffff8
	leave
	lea	rsp, [r13-16] 	# 0xfffffffffffffff0
	pop	r13
	jmp	void g<std::basic_simd<float, std::_VecAbi<8> > >(std::basic_simd<float, std::_VecAbi<8> >)
---------------------------------------------
.L70:
	mov	r9d, esi
	xor	edx, edx
	and	r9d, -8 	# 0xfffffffffffffff8
---------------------------------------------
.L46:
	mov	edi, edx
	add	edx, 8 	#
	mov	r8, QWORD PTR [rax+rdi]
	mov	QWORD PTR [rbp-48+rdi], r8 	# 0xffffffffffffffd0
	cmp	edx, r9d
	jb	.L46
	lea	rdi, [rbp-48] 	# 0xffffffffffffffd0
	lea	r8, [rdi+rdx]
	add	rdx, rax
	jmp	.L45
---------------------------------------------
iter_ub3(std::vector<float, std::allocator<float> > const&):
	mov	rax, QWORD PTR [rdi]
	mov	rdx, QWORD PTR [rdi+8] 	#
	vxorps	xmm0, xmm0, xmm0
	cmp	rax, rdx
	jnb	.L73
---------------------------------------------
.L74:
	vaddps	ymm0, ymm0, YMMWORD PTR [rax]
	add	rax, 32 	# 0x0000000000000020
	cmp	rax, rdx
	jb	.L74
---------------------------------------------
.L73:
	jmp	void g<std::basic_simd<float, std::_VecAbi<8> > >(std::basic_simd<float, std::_VecAbi<8> >)
---------------------------------------------
iter_zero_efficient1(std::vector<float, std::allocator<float> > const&):
	mov	rcx, QWORD PTR [rdi]
	mov	rdx, QWORD PTR [rdi+8] 	#
	lea	rsi, [rcx+32] 	# 0x0000000000000020
	cmp	rdx, rsi
	jb	.L88
	mov	rax, rsi
	vxorps	xmm0, xmm0, xmm0
---------------------------------------------
.L79:
	vaddps	ymm0, ymm0, YMMWORD PTR [rax-32] 	# 0xffffffffffffffe0
	add	rax, 32 	# 0x0000000000000020
	cmp	rdx, rax
	jnb	.L79
	lea	rax, [rdx-32] 	# 0xffffffffffffffe0
	sub	rax, rcx
	and	rax, -32 	# 0xffffffffffffffe0
	lea	rcx, [rsi+rax]
---------------------------------------------
.L78:
	sub	rdx, rcx
	cmp	rdx, 28 	# 0x000000000000001c
	ja	.L103
	push	r13
	sar	rdx, 2 	#
	mov	esi, 8 	#
	vxorps	xmm1, xmm1, xmm1
	sub	rsi, rdx
	mov	rax, rcx
	lea	r13, [rsp+16] 	#
	and	rsp, -32 	# 0xffffffffffffffe0
	push	QWORD PTR [r13-8] 	# 0xfffffffffffffff8
	push	rbp
	mov	rbp, rsp
	push	r13
	lea	rdi, [rbp-48] 	# 0xffffffffffffffd0
	vmovaps	YMMWORD PTR [rbp-48], ymm1 	# 0xffffffffffffffd0
	cmp	esi, 8 	#
	jnb	.L104
---------------------------------------------
.L82:
	xor	edx, edx
	test	sil, 4 	#
	jne	.L105
	test	sil, 2 	#
	jne	.L106
---------------------------------------------
.L86:
	and	esi, 1 	#
	jne	.L107
---------------------------------------------
.L87:
	vmovaps	ymm1, YMMWORD PTR [rbp-48] 	# 0xffffffffffffffd0
	vaddps	ymm0, ymm0, ymm1
	mov	r13, QWORD PTR [rbp-8] 	# 0xfffffffffffffff8
	leave
	lea	rsp, [r13-16] 	# 0xfffffffffffffff0
	pop	r13
	jmp	void g<std::basic_simd<float, std::_VecAbi<8> > >(std::basic_simd<float, std::_VecAbi<8> >)
---------------------------------------------
.L103:
	vmovups	ymm1, YMMWORD PTR [rcx]
	vaddps	ymm0, ymm0, ymm1
	jmp	void g<std::basic_simd<float, std::_VecAbi<8> > >(std::basic_simd<float, std::_VecAbi<8> >)
---------------------------------------------
.L107:
	movzx	eax, BYTE PTR [rax+rdx]
	mov	BYTE PTR [rdi+rdx], al
	jmp	.L87
---------------------------------------------
.L106:
	movzx	ecx, WORD PTR [rax+rdx]
	mov	WORD PTR [rdi+rdx], cx
	add	rdx, 2 	#
	and	esi, 1 	#
	je	.L87
	jmp	.L107
---------------------------------------------
.L105:
	mov	edx, DWORD PTR [rax]
	mov	DWORD PTR [rdi], edx
	mov	edx, 4 	#
	test	sil, 2 	#
	je	.L86
	jmp	.L106
---------------------------------------------
.L104:
	mov	r8d, esi
	xor	eax, eax
	and	r8d, -8 	# 0xfffffffffffffff8
---------------------------------------------
.L83:
	mov	edx, eax
	add	eax, 8 	#
	mov	rdi, QWORD PTR [rcx+rdx]
	mov	QWORD PTR [rbp-48+rdx], rdi 	# 0xffffffffffffffd0
	cmp	eax, r8d
	jb	.L83
	lea	rdi, [rbp-48] 	# 0xffffffffffffffd0
	add	rdi, rax
	add	rax, rcx
	jmp	.L82
---------------------------------------------
.L88:
	vxorps	xmm0, xmm0, xmm0
	jmp	.L78
---------------------------------------------
iter_zero_efficient2(std::vector<float, std::allocator<float> > const&):
	mov	rcx, QWORD PTR [rdi]
	mov	rdx, QWORD PTR [rdi+8] 	#
	vxorps	xmm0, xmm0, xmm0
	lea	rax, [rcx+32] 	# 0x0000000000000020
	cmp	rdx, rax
	jb	.L109
---------------------------------------------
.L110:
	vaddps	ymm0, ymm0, YMMWORD PTR [rax-32] 	# 0xffffffffffffffe0
	mov	rcx, rax
	add	rax, 32 	# 0x0000000000000020
	cmp	rdx, rax
	jnb	.L110
---------------------------------------------
.L109:
	sub	rdx, rcx
	cmp	rdx, 28 	# 0x000000000000001c
	jbe	.L111
	vmovups	ymm1, YMMWORD PTR [rcx]
	vaddps	ymm0, ymm0, ymm1
	jmp	void g<std::basic_simd<float, std::_VecAbi<8> > >(std::basic_simd<float, std::_VecAbi<8> >)
---------------------------------------------
.L111:
	push	r13
	sar	rdx, 2 	#
	mov	esi, 8 	#
	vxorps	xmm1, xmm1, xmm1
	sub	rsi, rdx
	mov	rax, rcx
	lea	r13, [rsp+16] 	#
	and	rsp, -32 	# 0xffffffffffffffe0
	push	QWORD PTR [r13-8] 	# 0xfffffffffffffff8
	push	rbp
	mov	rbp, rsp
	push	r13
	lea	rdi, [rbp-48] 	# 0xffffffffffffffd0
	vmovaps	YMMWORD PTR [rbp-48], ymm1 	# 0xffffffffffffffd0
	cmp	esi, 8 	#
	jnb	.L134
---------------------------------------------
.L113:
	xor	edx, edx
	test	sil, 4 	#
	jne	.L135
	test	sil, 2 	#
	jne	.L136
---------------------------------------------
.L117:
	and	esi, 1 	#
	jne	.L137
---------------------------------------------
.L118:
	vmovaps	ymm1, YMMWORD PTR [rbp-48] 	# 0xffffffffffffffd0
	vaddps	ymm0, ymm0, ymm1
	mov	r13, QWORD PTR [rbp-8] 	# 0xfffffffffffffff8
	leave
	lea	rsp, [r13-16] 	# 0xfffffffffffffff0
	pop	r13
	jmp	void g<std::basic_simd<float, std::_VecAbi<8> > >(std::basic_simd<float, std::_VecAbi<8> >)
---------------------------------------------
.L137:
	movzx	eax, BYTE PTR [rax+rdx]
	mov	BYTE PTR [rdi+rdx], al
	jmp	.L118
---------------------------------------------
.L136:
	movzx	ecx, WORD PTR [rax+rdx]
	mov	WORD PTR [rdi+rdx], cx
	add	rdx, 2 	#
	and	esi, 1 	#
	je	.L118
	jmp	.L137
---------------------------------------------
.L135:
	mov	edx, DWORD PTR [rax]
	mov	DWORD PTR [rdi], edx
	mov	edx, 4 	#
	test	sil, 2 	#
	je	.L117
	jmp	.L136
---------------------------------------------
.L134:
	mov	r8d, esi
	xor	eax, eax
	and	r8d, -8 	# 0xfffffffffffffff8
---------------------------------------------
.L114:
	mov	edx, eax
	add	eax, 8 	#
	mov	rdi, QWORD PTR [rcx+rdx]
	mov	QWORD PTR [rbp-48+rdx], rdi 	# 0xffffffffffffffd0
	cmp	eax, r8d
	jb	.L114
	lea	rdi, [rbp-48] 	# 0xffffffffffffffd0
	add	rdi, rax
	add	rax, rcx
	jmp	.L113
---------------------------------------------
iter_zero_sadly_not_efficient(std::vector<float, std::allocator<float> > const&):
	push	r13
	lea	r13, [rsp+16] 	#
	and	rsp, -32 	# 0xffffffffffffffe0
	push	QWORD PTR [r13-8] 	# 0xfffffffffffffff8
	push	rbp
	mov	rbp, rsp
	push	r14
	push	r13
	push	rbx
	mov	r9, QWORD PTR [rdi]
	mov	rdx, QWORD PTR [rdi+8] 	#
	lea	r10, [r9+32] 	# 0x0000000000000020
	cmp	rdx, r10
	jb	.L157
	vxorps	xmm0, xmm0, xmm0
	mov	rax, r10
	mov	r11d, 8 	#
	vmovaps	ymm2, ymm0
	jmp	.L148
---------------------------------------------
.L140:
	sar	rcx, 2 	#
	mov	rdi, r11
	vmovaps	YMMWORD PTR [rbp-80], ymm2 	# 0xffffffffffffffb0
	lea	r8, [rbp-80] 	# 0xffffffffffffffb0
	sub	rdi, rcx
	mov	rcx, rsi
	cmp	edi, 8 	#
	jnb	.L180
---------------------------------------------
.L142:
	xor	esi, esi
	test	dil, 4 	#
	je	.L145
	mov	esi, DWORD PTR [rcx]
	mov	DWORD PTR [r8], esi
	mov	esi, 4 	#
---------------------------------------------
.L145:
	test	dil, 2 	#
	je	.L146
	movzx	ebx, WORD PTR [rcx+rsi]
	mov	WORD PTR [r8+rsi], bx
	add	rsi, 2 	#
---------------------------------------------
.L146:
	and	edi, 1 	#
	je	.L147
	movzx	ecx, BYTE PTR [rcx+rsi]
	mov	BYTE PTR [r8+rsi], cl
---------------------------------------------
.L147:
	vmovaps	ymm1, YMMWORD PTR [rbp-80] 	# 0xffffffffffffffb0
	add	rax, 32 	# 0x0000000000000020
	vaddps	ymm0, ymm0, ymm1
	cmp	rdx, rax
	jb	.L181
---------------------------------------------
.L148:
	lea	rsi, [rax-32] 	# 0xffffffffffffffe0
	mov	rcx, rdx
	sub	rcx, rsi
	cmp	rcx, 28 	# 0x000000000000001c
	jbe	.L140
	vmovups	ymm1, YMMWORD PTR [rax-32] 	# 0xffffffffffffffe0
	add	rax, 32 	# 0x0000000000000020
	vaddps	ymm0, ymm0, ymm1
	cmp	rdx, rax
	jnb	.L148
---------------------------------------------
.L181:
	lea	rax, [rdx-32] 	# 0xffffffffffffffe0
	sub	rax, r9
	and	rax, -32 	# 0xffffffffffffffe0
	lea	r9, [r10+rax]
---------------------------------------------
.L139:
	sub	rdx, r9
	cmp	rdx, 28 	# 0x000000000000001c
	ja	.L182
	sar	rdx, 2 	#
	mov	ecx, 8 	#
	vxorps	xmm1, xmm1, xmm1
	mov	rax, r9
	sub	rcx, rdx
	vmovaps	YMMWORD PTR [rbp-80], ymm1 	# 0xffffffffffffffb0
	lea	rsi, [rbp-80] 	# 0xffffffffffffffb0
	cmp	ecx, 8 	#
	jnb	.L183
---------------------------------------------
.L151:
	xor	edx, edx
	test	cl, 4 	#
	jne	.L184
	test	cl, 2 	#
	jne	.L185
---------------------------------------------
.L155:
	and	ecx, 1 	#
	jne	.L186
---------------------------------------------
.L156:
	vmovaps	ymm1, YMMWORD PTR [rbp-80] 	# 0xffffffffffffffb0
	jmp	.L150
---------------------------------------------
.L180:
	mov	ebx, edi
	xor	ecx, ecx
	and	ebx, -8 	# 0xfffffff8
---------------------------------------------
.L143:
	mov	r8d, ecx
	add	ecx, 8 	#
	mov	r14, QWORD PTR [rsi+r8]
	mov	QWORD PTR [rbp-80+r8], r14 	# 0xffffffffffffffb0
	cmp	ecx, ebx
	jb	.L143
	lea	rbx, [rbp-80] 	# 0xffffffffffffffb0
	lea	r8, [rbx+rcx]
	add	rcx, rsi
	jmp	.L142
---------------------------------------------
.L182:
	vmovups	ymm1, YMMWORD PTR [r9]
---------------------------------------------
.L150:
	vaddps	ymm0, ymm0, ymm1
	pop	rbx
	pop	r13
	pop	r14
	pop	rbp
	lea	rsp, [r13-16] 	# 0xfffffffffffffff0
	pop	r13
	jmp	void g<std::basic_simd<float, std::_VecAbi<8> > >(std::basic_simd<float, std::_VecAbi<8> >)
---------------------------------------------
.L186:
	movzx	eax, BYTE PTR [rax+rdx]
	mov	BYTE PTR [rsi+rdx], al
	vmovaps	ymm1, YMMWORD PTR [rbp-80] 	# 0xffffffffffffffb0
	jmp	.L150
---------------------------------------------
.L185:
	movzx	edi, WORD PTR [rax+rdx]
	mov	WORD PTR [rsi+rdx], di
	add	rdx, 2 	#
	and	ecx, 1 	#
	je	.L156
	jmp	.L186
---------------------------------------------
.L184:
	mov	edx, DWORD PTR [rax]
	mov	DWORD PTR [rsi], edx
	mov	edx, 4 	#
	test	cl, 2 	#
	je	.L155
	jmp	.L185
---------------------------------------------
.L183:
	mov	edi, ecx
	xor	eax, eax
	and	edi, -8 	# 0xfffffff8
---------------------------------------------
.L152:
	mov	edx, eax
	add	eax, 8 	#
	mov	rsi, QWORD PTR [r9+rdx]
	mov	QWORD PTR [rbp-80+rdx], rsi 	# 0xffffffffffffffb0
	cmp	eax, edi
	jb	.L152
	lea	rbx, [rbp-80] 	# 0xffffffffffffffb0
	lea	rsi, [rbx+rax]
	add	rax, r9
	jmp	.L151
---------------------------------------------
.L157:
	vxorps	xmm0, xmm0, xmm0
	jmp	.L139
