; $Id: vmheap.asm,v 1.4 2003/12/02 17:23:26 root Exp $
;
; VM block transfer routines. ALP v 4.00.007 or later required.

	.586p
	.MMX
	.XMM

	extern	DOS32FLATDS:abs
	extern	_fix_kernel_npx:FAR32
	extern	_fix_kernel:FAR32
	public	_threednow, _verify_memory
	OPTION  SEGMENT:USE16

_DATA	segment public 'DATA'
	_threednow dw 0FFFFh
	_verify_memory dw 1
_DATA	ends

_BSS	segment public 'BSS'
	extern	_DevHlp: dword
	extern  _alloc_flags: dword
	extern	_flat_ds: word
	far_gate fword	?

ifdef MAX_HEAP
    public _gcbHeapUsed
    _gcbHeapUsed    dd 0
    public _gcbHeapMax
    _gcbHeapMax     dd 0c0000000h
    public _gcVMBlocks
    _gcVMBlocks     dd 0
    public _gcVMFrees
    _gcVMFrees      dd 0
    public _gcVMAllocs
    _gcVMAllocs     dd 0
endif
_BSS	ends

DGROUP  GROUP _DATA, _BSS

_TEXT32 segment para public use32 'CODE'
		assume cs:_TEXT32
		assume ds:FLAT, es:FLAT

; Workhorse procedure for SIMD data transfer. Derived from:
;
; AMD Athlon<TM> Processor x86 Code Optimization 22007J  August 2001
; Chapter 10: "3DNow!<TM> and MMX<TM> Optimizations"
;
; AMD Athlon Processor-Specific Code
;
; The following memory copy example is written with Microsoft
; Visual C++ in-line assembler syntax, and assumes that the
; Microsoft Processor Pack is installed (available from
; Microsoft's web site). This is a general purpose memcpy()
; routine, which can efficiently copy any size block, small or
; large. Data alignment is strongly recommended for good
; performance, but this code can handle non-aligned blocks.
;
; Example 2: Optimized memcpy() for Any Data Size or Alignment
;
TINY_BLOCK_COPY 	EQU	64  ; upper limit for movsd type copy
; The smallest copy uses the X86 "movsd" instruction, in an optimized
; form which is an "unrolled loop".
IN_CACHE_COPY 		EQU	64 * 1024 ; upper limit for movq/movq copy w/SW prefetch
; Next is a copy that uses the MMX registers to copy 8 bytes at a time,
; also using the "unrolled loop" optimization. This code uses
; the software prefetch instruction to get the data into the cache.
UNCACHED_COPY 		EQU	197 * 1024 ; upper limit for movq/movntq w/SW prefetch
; For larger blocks, which will spill beyond the cache, it's faster to
; use the Streaming Store instruction MOVNTQ. This write instruction
; bypasses the cache and writes straight to main memory. This code also
; uses the software prefetch instruction to pre-read the data.
; USE 64 * 1024 FOR THIS VALUE IF YOU'RE ALWAYS FILLING A "CLEAN CACHE"
CACHEBLOCK 	EQU	80h ; # of 64-byte blocks (cache lines) for block prefetch
; For the largest size blocks, a special technique called Block Prefetch
; can be used to accelerate the read operations. Block Prefetch reads
; one address per cache line, for a series of cache lines, in a short loop.
; This is faster than using software prefetch. The technique is great for
; getting maximum read bandwidth, especially in DDR memory systems.

ramfs_3dnow_transfer proc far
	push	ebp
	mov	ebp, esp
	sub	esp, 108
 	fsave	[ebp-108]
	mov	ebx, ecx		; keep a copy of count
	cld

	cmp	ecx, TINY_BLOCK_COPY
	jb	$memcpy_ic_3		; tiny? skip mmx copy
	cmp	ecx, 32*1024		; don't align between 32k-64k because
	jbe	$memcpy_do_align	; it appears to be slower
	cmp	ecx, 64*1024
	jbe	$memcpy_align_done
$memcpy_do_align:
	mov	ecx, 8			; a trick that's faster than rep movsb...
	sub	ecx, edi		; align destination to qword
	and	ecx, 111b		; get the low bits
	sub	ebx, ecx		; update copy count
	neg	ecx			; set up to jump into the array
	add	ecx, offset FLAT:$memcpy_align_done
	jmp	ecx			; jump to array of movsb`s
	align	4
	movsb
	movsb
	movsb
	movsb
	movsb
	movsb
	movsb
	movsb
$memcpy_align_done:			; destination is dword aligned
	mov	ecx, ebx		; number of bytes left to copy
	shr	ecx, 6			; get 64-byte block count
	jz	$memcpy_ic_2		; finish the last few bytes
	cmp	ecx, IN_CACHE_COPY/64	; too big 4 cache? use uncached copy
	jae	$memcpy_uc_test
; This is small block copy that uses the MMX registers to copy 8 bytes
; at a time. It uses the "unrolled loop" optimization, and also uses
; the software prefetch instruction to get the data into the cache.
	ALIGN	16
$memcpy_ic_1:				; 64-byte block copies, in-cache copy
	prefetchnta ds:[esi+(200*64/34+192)] ; start reading ahead
	movq	mm0, ds:[esi+0]		; read 64 bits
	movq	mm1, ds:[esi+8]
	movq	es:[edi+0], mm0		; write 64 bits
	movq	es:[edi+8], mm1		; note: the normal movq writes the
	movq	mm2, ds:[esi+16]	; data to cache; a cache line will be
	movq	mm3, ds:[esi+24]	; allocated as needed, to store the data
	movq	es:[edi+16], mm2
	movq	es:[edi+24], mm3
	movq	mm0, ds:[esi+32]
	movq	mm1, ds:[esi+40]
	movq	es:[edi+32], mm0
	movq	es:[edi+40], mm1
	movq	mm2, ds:[esi+48]
	movq	mm3, ds:[esi+56]
	movq	es:[edi+48], mm2
	movq	es:[edi+56], mm3
	add	esi, 64			; update source pointer
	add	edi, 64			; update destination pointer
	dec	ecx			; count down
	jnz	$memcpy_ic_1		; last 64-byte block?
$memcpy_ic_2:
	mov	ecx, ebx		; has valid low 6 bits of the byte count
$memcpy_ic_3:
	shr	ecx, 2			; dword count
	and	ecx, 1111b		; only look at the "remainder" bits
	neg	ecx			; set up to jump into the array
	add	ecx, offset FLAT:$memcpy_last_few
	jmp	ecx			; jump to array of movsd`s
$memcpy_uc_test:
	cmp	ecx, UNCACHED_COPY/64	; big enough? use block prefetch copy
	jae	$memcpy_bp_1
$memcpy_64_test:
	or	ecx, ecx		; tail end of block prefetch will jump here
	jz	$memcpy_ic_2		; no more 64-byte blocks left
; For larger blocks, which will spill beyond the cache, it's faster to
; use the Streaming Store instruction MOVNTQ. This write instruction
; bypasses the cache and writes straight to main memory. This code also
; uses the software prefetch instruction to pre-read the data.
	align	16
$memcpy_uc_1:				; 64-byte blocks, uncached copy
	prefetchnta ds:[esi+(200*64/34+192)] ; start reading ahead
	movq	mm0, ds:[esi+0]		; read 64 bits
	add	edi,64			; update destination pointer
	movq	mm1, ds:[esi+8]
	add	esi,64			; update source pointer
	movq	mm2, ds:[esi-48]
	movntq	es:[edi-64], mm0		; write 64 bits, bypassing the cache
	movq	mm0, ds:[esi-40]		; note: movntq also prevents the CPU
	movntq	es:[edi-56], mm1		; from READING the destination address
	movq	mm1, ds:[esi-32]		; into the cache, only to be over-written
	movntq	es:[edi-48], mm2		; so that also helps performance
	movq	mm2, ds:[esi-24]
	movntq	es:[edi-40], mm0
	movq	mm0, ds:[esi-16]
	movntq	es:[edi-32], mm1
	movq	mm1, ds:[esi-8]
	movntq	es:[edi-24], mm2
	movntq	es:[edi-16], mm0
	dec	ecx
	movntq	es:[edi-8], mm1
	jnz	$memcpy_uc_1		; last 64-byte block?
	jmp	$memcpy_ic_2		; almost dont
; For the largest size blocks, a special technique called Block Prefetch
; can be used to accelerate the read operations. Block Prefetch reads
; one address per cache line, for a series of cache lines, in a short loop.
; This is faster than using software prefetch. The technique is great for
; getting maximum read bandwidth, especially in DDR memory systems.
$memcpy_bp_1:				; large blocks, block prefetch copy
	cmp	ecx, CACHEBLOCK		; big enough to run another prefetch loop?
	jl	$memcpy_64_test		; no, back to regular uncached copy
	mov	eax, CACHEBLOCK / 2	; block prefetch loop, unrolled 2X
	add	esi, CACHEBLOCK * 64	; move to the top of the block
	align	16
$memcpy_bp_2:
	mov	edx, ds:[esi-64]	; grab one address per cache line
	mov	edx, ds:[esi-128]	; grab one address per cache line
	sub	esi, 128		; go reverse order to suppress HW prefetcher
	dec	eax			; count down the cache lines
	jnz	$memcpy_bp_2		; keep grabbing more lines into cache
	mov	eax, CACHEBLOCK		; now that it's in cache, do the copy
	align	16
$memcpy_bp_3:
	movq	mm0, ds:[esi]		; read 64 bits
	movq	mm1, ds:[esi+8]
	movq	mm2, ds:[esi+16]
	movq	mm3, ds:[esi+24]
	movq	mm4, ds:[esi+32]
	movq	mm5, ds:[esi+40]
	movq	mm6, ds:[esi+48]
	movq	mm7, ds:[esi+56]
	add	esi, 64			; update source pointer
	movntq	es:[edi], mm0		; write 64 bits, bypassing cache
	movntq	es:[edi+8], mm1		; note: movntq also prevents the CPU
	movntq	es:[edi+16], mm2	; from READING the destination address
	movntq	es:[edi+24], mm3	; into the cache, only to be over-written,
	movntq	es:[edi+32], mm4	; so that also helps performance
	movntq	es:[edi+40], mm5
	movntq	es:[edi+48], mm6
	movntq	es:[edi+56], mm7
	add	edi, 64			; update dest pointer
	dec	eax			; count down
	jnz	$memcpy_bp_3		; keep copying
	sub	ecx, CACHEBLOCK		; update the 64-byte block count
	jmp	$memcpy_bp_1		; keep processing blocks
; The smallest copy uses the X86 "movsd" instruction, in an optimized
; form which is an "unrolled loop". Then it handles the last few bytes.
	align	4
	movsd
	movsd	; perform last 1-15 dword copies
	movsd
	movsd
	movsd
	movsd
	movsd
	movsd
	movsd
	movsd	; perform last 1-7 dword copies
	movsd
	movsd
	movsd
	movsd
	movsd
	movsd
$memcpy_last_few:			; dword aligned from before movsd`s
	mov	ecx, ebx		; has valid low 2 bits of the byte count
	and	ecx, 11b		; the last few cows must come home
	jz	$memcpy_final		; no more, let's leave
	rep	movsb			; the last 1, 2, or 3 bytes
$memcpy_final:
	emms				; clean up the MMX state
	sfence				; flush the write buffer

  	frstor	[ebp-108]
	mov	esp, ebp
	pop	ebp
	retf

ramfs_3dnow_transfer	endp

; The old yet reliable DWORD copy procedure

ramfs_dword_transfer	proc far
	mov	ebx, ecx
	cld
	shr	ecx, 2
	and	ebx, 3
	rep	movsd
	mov	ecx, ebx
	rep	movsb
	retf
ramfs_dword_transfer	endp
			
_TEXT32 ends

_TEXT	segment public 'CODE'

	assume es:nothing, ss:nothing, ds:DGROUP, fs:nothing, gs:nothing

	public	VMVIRTTOFLAT
	public	VMALLOC
	public	VMFREE
	public	VMREADUCHAR
	public	VMREADUSHORT
	public	VMREADBLK
	public	VMWRITEBLK
	public	VMREAD
	public	VMWRITE
	public	VMCOPY


;-----------------------------------------------------------------------------
; FLAT _pascal VMVirtToFlat (void *p);
;-----------------------------------------------------------------------------

VMVIRTTOFLAT	proc near
	push	bp
	mov	bp, sp
	push	si

	movzx	esi, word ptr [bp+4]	; offset of p
	mov	ax, word ptr [bp+6]	; selector of p
	mov	dl, 5Bh			; _DevHlp_VirtToLin
	call	[_DevHlp]
	jnc	vvtf_end
	xor	eax, eax
vvtf_end:

	shld	edx, eax, 16
	pop	si
	pop	bp
	ret	4
VMVIRTTOFLAT	endp




;-----------------------------------------------------------------------------
; FLAT _pascal VMAlloc (int fForce, ULONG cbSize);
;-----------------------------------------------------------------------------

VMALLOC	proc near
	push	bp
	mov	bp, sp

ifdef MAX_HEAP
    mov cx, [bp+8]      ; fForce
    or  cx, cx
    jnz va_ok
	mov	ecx, [bp+4]		; cbSize
    add ecx, _gcbHeapUsed
    cmp ecx, _gcbHeapMax
    jbe va_ok
    xor eax, eax
    jmp va_end
va_ok:
endif

	mov	ecx, [bp+4]		; cbSize
	mov	eax, _alloc_flags	; flags
	mov	dl, 57h			; _DevHlp_VMAlloc
	call	[_DevHlp]
    jnc	va_alloced
	xor	eax, eax
    jmp va_end
va_alloced:
ifdef MAX_HEAP
	mov	ecx, [bp+4]		; cbSize
    add _gcbHeapUsed, ecx
    inc _gcVMAllocs
    inc _gcVMBlocks
endif
va_end:

	shld	edx, eax, 16
	pop	bp
	ret	6
VMALLOC	endp




;-----------------------------------------------------------------------------
; void _pascal VMFree (FLAT flatBlock, ULONG cbSize);
;-----------------------------------------------------------------------------

VMFREE	proc near
	push	bp
	mov	bp, sp

	mov	eax, [bp+8]		; flatBlock
    or eax, eax
    jz vf_end

	mov	dl, 58h			; _DevHlp_VMFree
	call	[_DevHlp]
	jnc	vf_ok
ifdef DEBUG
; Bugs in error handling procedures means we shouldn't do this.
    int 3
endif
    jmp vf_end
vf_ok:
ifdef MAX_HEAP
    inc _gcVMFrees
    dec _gcVMBlocks
    mov ecx, [bp+4]
    cmp ecx, _gcbHeapUsed
    jg  vf_size_nok
    sub _gcbHeapUsed, ecx
vf_size_nok:
endif
vf_end:
	pop	bp
	ret 8
VMFREE	endp




;-----------------------------------------------------------------------------
; UCHAR _pascal VMReadUChar (FLAT flatSrc);
;-----------------------------------------------------------------------------

VMREADUCHAR	proc near
	push	bp
	mov	bp, sp
	mov	ax, DOS32FLATDS
	mov	es, ax

	mov	eax, [bp+4]		; flatSrc
	mov	al, es:[eax]

	pop	bp
	ret	4
VMREADUCHAR	endp




;-----------------------------------------------------------------------------
; USHORT _pascal VMReadUShort (FLAT flatSrc);
;-----------------------------------------------------------------------------

VMREADUSHORT	proc near
	push	bp
	mov	bp, sp
	mov	ax, DOS32FLATDS
	mov	es, ax

	mov	eax, [bp+4]		; flatSrc
	mov	ax, es:[eax]

	pop	bp
	ret	4
VMREADUSHORT	endp




;-----------------------------------------------------------------------------
; void _pascal VMReadBlk (BLOCK _ss *pBlk, FLAT flatBlk);
;-----------------------------------------------------------------------------

VMREADBLK	proc near
	push	bp
	mov	bp, sp
	push	ds
	mov	ax, DOS32FLATDS
	mov	es, ax

	mov	ebx, [bp+4]		; flatBlk
	mov	eax, es:[ebx+0]
	mov	edx, es:[ebx+4]
	lds	bx, dword ptr [bp+8]	; pBlk
	mov	ds:[bx+0], eax
	mov	ds:[bx+4], edx

	pop	ds
	pop	bp
	ret	8
VMREADBLK	endp




;-----------------------------------------------------------------------------
; void _pascal VMWriteBlk (FLAT flatBlk, BLOCK _ss *pBlk);
;-----------------------------------------------------------------------------

VMWRITEBLK	proc near
	push	bp
	mov	bp, sp
	push	ds
	mov	ax, DOS32FLATDS
	mov	es, ax

	lds	bx, dword ptr [bp+4]	; pBlk
	mov	eax, ds:[bx+0]
	mov	edx, ds:[bx+4]
	mov	ebx, [bp+8]		; flatBlk
	mov	es:[ebx+0], eax
	mov	es:[ebx+4], edx

	pop	ds
	pop	bp
	ret	8
VMWRITEBLK	endp




;-----------------------------------------------------------------------------
; void _pascal VMRead (void *pDest, FLAT flatSrc, USHORT cbLen);
;-----------------------------------------------------------------------------

VMREAD	proc near
	push	bp
	mov	bp, sp
	push    si
	push	di
	push	ds
	push	es

	sub	ecx, ecx
	mov	cx, [bp+4]		; number of bytes to copy
	xor	edi, edi
	mov	esi, [bp+6]		; source
	les	di, [bp+10]		; destination

	cmp	_verify_memory, 0
	jz	@F
	jcxz	@F
	mov	ax, es
	mov	dx, 127h		; DH_VerifyAccess && write access
	call	[_DevHlp]
	jc	read_failed
@@:
	mov	ax, DOS32FLATDS
	mov	ds, ax
	push	fs
	mov	ax, seg	DGROUP
	mov	fs, ax
	call	fword ptr fs:far_gate
	pop	fs

read_failed:
	pop	es	
	pop	ds
	pop	di
	pop	si
	pop	bp
	ret	10
VMREAD	endp



;-----------------------------------------------------------------------------
; void _pascal VMWrite (FLAT flatDest, void *pSrc, USHORT cbLen);
;-----------------------------------------------------------------------------

VMWRITE	proc near
	push	bp
	mov	bp, sp
	push	si
	push	di
	push	ds
	push	es
	mov	ax, DOS32FLATDS
	mov	es, ax

	xor	esi, esi
	xor	ecx, ecx
	mov	cx, word ptr [bp+4]	; cbLen

	cmp	_verify_memory, 0
	jz	@F
	jcxz	@F
	mov	ax, [bp+8]		; pSrc/segment
	mov	di, [bp+6]		; pSrc/offset
	mov	dx, 27h			; DH_VerifyAccess && read access
	call	[_DevHlp]
	jc	write_failed

@@:
	lds	si, [bp+6]		; pSrc
	mov	edi, [bp+10]		; flatDest
	push	fs
	mov	ax, seg	DGROUP
	mov	fs, ax
	call	fword ptr fs:far_gate
	pop	fs

write_failed:
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	bp
	ret     10
VMWRITE	endp




;-----------------------------------------------------------------------------
; void _pascal VMCopy (FLAT flatDest, FLAT flatSrc, ULONG cbLen);
;-----------------------------------------------------------------------------

VMCOPY	proc near
	push	bp
	mov	bp, sp
	push	es
	push	ds
	push	si
	push	di
	mov	ax, DOS32FLATDS
	mov	es, ax
	mov	ds, ax

	mov	edi, [bp+12]		; flatDest
	mov	esi, [bp+8]		; flatSrc
	mov	ecx, [bp+4]     	; cbLen
	mov	edx, ecx
	cmp	edi, esi
	je	vc_end
	ja	vc_backwards

	; move forwards
	push	fs
	mov	ax, seg	DGROUP
	mov	fs, ax
	call	fword ptr fs:far_gate
	pop	fs
	jmp	vc_end

vc_backwards:
	; move backwards
	lea	esi, [esi+ecx-1]
	lea	edi, [edi+ecx-1]
	and	ecx, 0003h
	std
	rep	movs byte ptr es:[edi], byte ptr es:[esi]
	sub	esi, 3
	sub	edi, 3
	mov	ecx, edx
	shr	ecx, 2
	rep	movs dword ptr es:[edi], dword ptr es:[esi]
	cld

vc_end:
	pop	di
	pop	si
	pop	ds
	pop	es
	pop	bp
	ret	12
VMCOPY	endp

;
; Weird initialization routine. _fix_kernel can't be issued at the init time,
; so it's deferred as we plant init_cont to sit in place of real transfer
; subroutine.
;

VMINIT	proc	near
	push	ds
	mov	ax, seg	DGROUP
	mov	ds, ax
	mov	dword ptr ds:far_gate, 0
	mov	word ptr ds:far_gate, offset init_cont
	mov	word ptr ds:far_gate+4, seg init_cont
	pop	ds
	ret
; The following runs once on the first access to VM core
init_cont:
	pushad
	push	ds
	push	es
	push	fs
	mov	ax, seg	DGROUP
	mov	ds, ax
	mov	fs, ax
	mov	al, 9			; This is an undocumented DosEnv entry
	mov	dl, 24h
	call	[_DevHlp]
        mov     es, ax
	mov	cx, seg	DGROUP
        movzx   ax, byte ptr es:[bx]
        shl     ax, 2
	mov	ds, cx
        add     bx, ax
        mov     ax, word ptr es:[bx+26h] ; Flat CS
        mov     word ptr ds:far_gate+4, ax
        mov     ax, word ptr es:[bx+2Ah] ; Flat DS
        mov     word ptr ds:_flat_ds, ax
; Apply miscellaneous fixes
	push	es
	push	ds
	mov	dword ptr fs:far_gate, offset FLAT:_fix_kernel
	mov	ax, fs:_flat_ds
	mov	ds, ax
	mov	es, ax
 	call	fword ptr fs:far_gate
	pop	ds
	pop	es
; Check if 3DNow! was forced
	mov	ax, ds:_threednow
	or	ax, ax
	jz	no_3dnow
	cmp	al, 1
	je	ok_3dnow
; 3DNow!<TM> detection - AMD technote #21928G/0 March 2000
	pushfd
	pushfd
	pop	eax
	mov	ebx, eax
	xor	eax, 00200000h		; Flip CPUID bit
	push	eax
	popfd
	pushfd
	pop	eax
	popfd
	cmp	eax, ebx
	je	no_3dnow
	mov	eax, 80000000h
	cpuid
	cmp	eax, 80000000h
	jbe	no_3dnow
	mov	eax, 80000001h
	cpuid
	and	edx, 80400000h
	cmp	edx, 80400000h
	jnz	no_3dnow
; Now try to apply the NPX patch for 3DNow. Compose the address manually using
; Flat CS provided by kernel.
ok_3dnow:
	push	es
	push	ds
	mov	dword ptr fs:far_gate, offset FLAT:_fix_kernel_npx
	mov	ax, fs:_flat_ds
	mov	ds, ax
	mov	es, ax
 	call	fword ptr fs:far_gate
	pop	ds
	pop	es
	or	eax, eax
	jz	kernel_fixed
	mov	ax, fs:_threednow
	cmp	al, 1			; If 3DNow! was forced, let it stay
	jne	no_3dnow
kernel_fixed:
; Install the appropriate handler
	mov	dword ptr ds:far_gate, offset FLAT:ramfs_3dnow_transfer
	jmp	short far_gate_done
no_3dnow:
	mov	dword ptr ds:far_gate, offset FLAT:ramfs_dword_transfer
far_gate_done:
	pop	fs
	pop	es
	pop	ds
	popad
	jmp	fword ptr fs:far_gate
VMINIT	endp

; Flips the stacks

__chkstk proc	far
	pop	cx
	pop	dx
	sub	sp, ax
	push	dx
	push	cx
	retf
__chkstk endp


_TEXT	ends

	end
