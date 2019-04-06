	TITLE   AALINE.ASM
	NAME    AALINE

	.8087
_TEXT	SEGMENT  WORD PUBLIC 'CODE'
_TEXT	ENDS
_DATA	SEGMENT  WORD PUBLIC 'DATA'
_DATA	ENDS
CONST	SEGMENT  WORD PUBLIC 'CONST'
CONST	ENDS
_BSS	SEGMENT  WORD PUBLIC 'BSS'
_BSS	ENDS
DGROUP	GROUP	CONST, _BSS, _DATA
	ASSUME  CS: _TEXT, DS: DGROUP, SS: DGROUP
EXTRN	_aapixel:NEAR
EXTRN	_aahspan:NEAR
EXTRN	_aavspan:NEAR
_TEXT      SEGMENT
	ASSUME	CS: _TEXT

	PUBLIC	_aaline
_aaline	PROC NEAR
	push	bp
	mov	bp,sp
	sub	sp,24
	push	di
	push	si
;	sy = -20
;	tmp = -2
;	alpha = -4
;	x1 = 4
;	y1 = 6
;	x2 = 8
;	y2 = 10
;	inc = -16
;	inc_minus_one = -8
;	err = -12
;	dx = -22
;	dy = -24
;	sx = -18
	mov	ax,1
	mov	WORD PTR [bp-20],ax	;sy
	mov	WORD PTR [bp-18],ax	;sx
	mov	ax,WORD PTR [bp+8]	;x2
	sub	ax,WORD PTR [bp+4]	;x1
	mov	WORD PTR [bp-22],ax	;dx
	or	ax,ax
	jge	$I323
	mov	WORD PTR [bp-18],-1	;sx
	neg	ax
	mov	WORD PTR [bp-22],ax	;dx
$I323:
	mov	ax,WORD PTR [bp+10]	;y2
	sub	ax,WORD PTR [bp+6]	;y1
	mov	WORD PTR [bp-24],ax	;dy
	or	ax,ax
	jge	$I324
	mov	WORD PTR [bp-20],-1	;sy
	neg	ax
	mov	WORD PTR [bp-24],ax	;dy
$I324:
	mov	ax,WORD PTR [bp-24]	;dy
	cmp	WORD PTR [bp-22],ax	;dx
	jge	$JCC69
	jmp	$I325
$JCC69:
	or	ax,ax
	jne	$I326
	cmp	WORD PTR [bp-18],0	;sx
	jle	$L20000
	push	WORD PTR [bp+6]	        ;y1
	push	WORD PTR [bp+8]	        ;x2
	push	WORD PTR [bp+4]	        ;x1
$L20033:
	call	WORD PTR _aahspan
	jmp	$L20030
$L20000:
	push	WORD PTR [bp+6]	        ;y1
	push	WORD PTR [bp+4]	        ;x1
	push	WORD PTR [bp+8]	        ;x2
	jmp	SHORT $L20033
	nop	
$I326:
        mov     dx,WORD PTR [bp-24]     ;dy
	xor	ax,ax
	mov	bx,WORD PTR [bp-22]	;dx
        cmp     dx,bx
        jne     notdiag
        mov     dx,1
        jmp     SHORT diag
notdiag:
	div	bx	                ;dx
        xor     dx,dx
diag:	mov	WORD PTR [bp-16],ax	;inc
	mov	WORD PTR [bp-14],dx
	sub	ax,0
	sbb	dx,1
	mov	WORD PTR [bp-8],ax	;inc_minus_one
	mov	WORD PTR [bp-6],dx
	mov	ax,WORD PTR [bp-16]	;inc
	mov	dx,WORD PTR [bp-14]
	sub	ax,-32768
	sbb	dx,0
	mov	WORD PTR [bp-12],ax	;err
	mov	WORD PTR [bp-10],dx
	or	dx,dx
	jl	$L20002
	sub	ax,ax
	jmp	SHORT $L20003
$L20002:
	mov	ax,127
$L20003:
	mov	di,ax
	mov	si,WORD PTR [bp+4]	;x1
	jmp	SHORT $L20006
$I330:
	mov	al,BYTE PTR [bp-11]
	sub	ah,ah
	mov	di,ax
	mov	ax,WORD PTR [bp-16]	;inc
	mov	dx,WORD PTR [bp-14]
	add	WORD PTR [bp-12],ax	;err
	adc	WORD PTR [bp-10],dx
$I331:
	add	si,WORD PTR [bp-18]	;sx
$L20006:
	cmp	si,WORD PTR [bp+8]	;x2
	je	$L20005
	mov	ax,di
	xor	al,255
	push	ax
	push	WORD PTR [bp+6]	        ;y1
	push	si
	call	WORD PTR _aapixel
	add	sp,6
	push	di
	mov	ax,WORD PTR [bp+6]	;y1
	add	ax,WORD PTR [bp-20]	;sy
	push	ax
	push	si
	call	WORD PTR _aapixel
	add	sp,6
	cmp	WORD PTR [bp-10],0
	jl	$I330
	sub	di,di
	mov	ax,WORD PTR [bp-8]	;inc_minus_one
	mov	dx,WORD PTR [bp-6]
	add	WORD PTR [bp-12],ax	;err
	adc	WORD PTR [bp-10],dx
	mov	ax,WORD PTR [bp-20]	;sy
	add	WORD PTR [bp+6],ax	;y1
	jmp	SHORT $I331
	nop	
$L20005:
	mov	WORD PTR [bp-4],di	;alpha
	mov	WORD PTR [bp+4],si	;x1
	mov	ax,di
	xor	al,255
	push	ax
	push	WORD PTR [bp+10]	;y2
	push	WORD PTR [bp+8]	        ;x2
	call	WORD PTR _aapixel
	add	sp,6
	push	di
	mov	ax,WORD PTR [bp+10]	;y2
	add	ax,WORD PTR [bp-20]	;sy
	push	ax
	push	WORD PTR [bp+8]	        ;x2
	jmp	$L20028
$I325:
	cmp	WORD PTR [bp-22],0	;dx
	jne	$I333
	cmp	WORD PTR [bp-20],0	;sy
	jle	$L20008
	push	WORD PTR [bp+10]	;y2
	push	WORD PTR [bp+6]	        ;y1
$L20034:
	push	WORD PTR [bp+4]	        ;x1
	call	WORD PTR _aavspan
	jmp	$L20030
$L20008:
	push	WORD PTR [bp+6]	        ;y1
	push	WORD PTR [bp+10]	;y2
	jmp	SHORT $L20034
$I333:
	mov	dx,WORD PTR [bp-22]	;dx
	xor	ax,ax
	div	WORD PTR [bp-24]        ;dx/dy
        xor     dx,dx
	mov	WORD PTR [bp-16],ax	;inc
	mov	WORD PTR [bp-14],dx
	sub	ax,0
	sbb	dx,1
	mov	WORD PTR [bp-8],ax	;inc_minus_one
	mov	WORD PTR [bp-6],dx
	mov	ax,WORD PTR [bp-16]	;inc
	mov	dx,WORD PTR [bp-14]
	sub	ax,-32768
	sbb	dx,0
	mov	WORD PTR [bp-12],ax	;err
	mov	WORD PTR [bp-10],dx
	or	dx,dx
	jl	$L20010
	sub	ax,ax
	jmp	SHORT $L20011
$L20010:
	mov	ax,127
$L20011:
	mov	di,ax
	mov	si,WORD PTR [bp+6]	;y1
	jmp	SHORT $L20014
$I337:
	mov	al,BYTE PTR [bp-11]
	sub	ah,ah
	mov	di,ax
	mov	ax,WORD PTR [bp-16]	;inc
	mov	dx,WORD PTR [bp-14]
	add	WORD PTR [bp-12],ax	;err
	adc	WORD PTR [bp-10],dx
$I338:
	add	si,WORD PTR [bp-20]	;sy
$L20014:
	cmp	si,WORD PTR [bp+10]	;y2
	je	$L20013
	mov	ax,di
	xor	al,255
	push	ax
	push	si
	push	WORD PTR [bp+4]	        ;x1
	call	WORD PTR _aapixel
	add	sp,6
	push	di
	push	si
	mov	ax,WORD PTR [bp+4]	;x1
	add	ax,WORD PTR [bp-18]	;sx
	push	ax
	call	WORD PTR _aapixel
	add	sp,6
	cmp	WORD PTR [bp-10],0
	jl	$I337
	sub	di,di
	mov	ax,WORD PTR [bp-8]	;inc_minus_one
	mov	dx,WORD PTR [bp-6]
	add	WORD PTR [bp-12],ax	;err
	adc	WORD PTR [bp-10],dx
	mov	ax,WORD PTR [bp-18]	;sx
	add	WORD PTR [bp+4],ax	;x1
	jmp	SHORT $I338
	nop	
$L20013:
	mov	WORD PTR [bp-4],di	;alpha
	mov	WORD PTR [bp+6],si	;y1
	mov	ax,di
	xor	al,255
	push	ax
	push	WORD PTR [bp+10]	;y2
	push	WORD PTR [bp+8]	        ;x2
	call	WORD PTR _aapixel
	add	sp,6
	push	di
	push	WORD PTR [bp+10]	;y2
	mov	ax,WORD PTR [bp+8]	;x2
	add	ax,WORD PTR [bp-18]	;sx
	push	ax
$L20028:
	call	WORD PTR _aapixel
$L20030:
	add	sp,6
	pop	si
	pop	di
	mov	sp,bp
	pop	bp
	ret	

_aaline	ENDP
_TEXT	ENDS
END
