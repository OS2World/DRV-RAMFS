; $Id: 32util.asm,v 1.2 2003/11/07 07:46:20 root Exp $
;
; Utility routines

	OPTION	OLDSTRUCTS

SAS	STRUC
SAS_signature	DD	?
SAS_tables_data	DW	?
SAS_flat_sel	DW	?
SAS_config_data	DW	?
SAS_dd_data	DW	?
SAS_vm_data	DW	?
SAS_task_data	DW	?
SAS_RAS_data	DW	?
SAS_file_data	DW	?
SAS_info_data	DW	?
SAS	ENDS


SAS_vm_section	STRUC
SAS_vm_arena	DD	?
SAS_vm_object	DD	?
SAS_vm_context	DD	?
SAS_vm_krnl_mte	DD	?
SAS_vm_glbl_mte	DD	?
SAS_vm_pft	DD	?
SAS_vm_prt	DD	?
SAS_vm_swap	DD	?
SAS_vm_idle_head	DD	?
SAS_vm_free_head	DD	?
SAS_vm_heap_info	DD	?
SAS_vm_all_mte	DD	?
SAS_vm_section	ENDS

.386p
		
	public	_locate_krnl_mte
	public  _savenpx_override
	public  _get_cs
	public	_npx_tcb_base
	public	_flat_ds

_DATA32	SEGMENT WORD PUBLIC 'DATA32'
	assume	ds:FLAT
	_npx_tcb_base	dd ?
_DATA32	ENDS

CONST32	SEGMENT WORD PUBLIC 'CONST32'
	assume	ds:FLAT
CONST32	ENDS

_BSS32	SEGMENT WORD PUBLIC 'BSS32'
	assume	ds:FLAT
_BSS32	ENDS

_BSS SEGMENT WORD PUBLIC USE16 'BSS'
	_flat_ds dw ?
_BSS ENDS

CODE32 GROUP _TEXT32
DGRP32 GROUP _DATA32, CONST32, _BSS32
	
_TEXT32 SEGMENT	PARA PUBLIC USE32 'CODE'
	assume	cs:_TEXT32, ds:FLAT

; Locate the kernel MTE through System Anchor Segment
		
_locate_krnl_mte	PROC
		push	es
		mov	ax, 70h		; SAS_Selector
		mov	es, ax
		mov	bx, es:[SAS_vm_data]
		mov	eax, dword ptr es:[bx+SAS_vm_krnl_mte]
		pop	es
		retn
_locate_krnl_mte	endp

; NPX procedure.

_savenpx_override proc	far
		mov	ebx, dword ptr [eax+ebx*4]
		or	ebx, ebx
		jnz	add_it
dump_it:
		stc
		retf
add_it:
		push	eax
		mov	eax, ds:_npx_tcb_base ; DS is already flat
		mov	ebx, dword ptr [ebx+eax]
		pop	eax
		or	ebx, ebx
		jz	dump_it
		clc
		retf
_savenpx_override endp

; Returns the code segment.

_get_cs		proc
		mov	ax, cs
		ret
_get_cs		endp
	
_TEXT32 ENDS

        END
