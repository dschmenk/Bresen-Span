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
	mov	dx,1
        mov     cx,dx
	mov	ax,WORD PTR [bp+8]	;x2
	sub	ax,WORD PTR [bp+4]	;x1
	jge	x_abs
        neg     dx	                ;sx
	neg	ax                      ;dx
x_abs:
	mov	bx,WORD PTR [bp+10]	;y2
	sub	bx,WORD PTR [bp+6]	;y1
	jge	y_abs
        neg     cx	                ;sy
	neg	bx                      ;dy
y_abs:
	mov	WORD PTR [bp-22],ax	;dx
	mov	WORD PTR [bp-24],bx	;dy
	mov	WORD PTR [bp-18],dx	;sx
        mov     WORD PTR [bp-20],cx     ;sy
	cmp	ax,bx	                ;dx>=dy?
	jge	x_major
	jmp	y_major
;
;
;
x_major:        
        or      bx,bx                   ;dy
	jne	hsetup
	push	WORD PTR [bp+6]	        ;y1
	or	dx,dx	                ;sx
        jle     hrev
        push	WORD PTR [bp+8]	        ;x2
        push	WORD PTR [bp+4]	        ;x1
        jmp     SHORT hspan
hrev:   push	WORD PTR [bp+4]	        ;x1
        push	WORD PTR [bp+8]	        ;x2
hspan:	call	WORD PTR _aahspan
	jmp	exit
hsetup:
        mov     dx,WORD PTR [bp-24]     ;dy
	xor	ax,ax
	mov	bx,WORD PTR [bp-22]	;dx
        cmp     dx,bx
        jne     nodiag
        mov     dx,1
        jmp     SHORT diag
nodiag:	div	bx	                ;dx
        xor     dx,dx
diag:	mov	WORD PTR [bp-16],ax	;inc
	mov	WORD PTR [bp-14],dx
	sub	ax,0
	sbb	dx,1
	mov	WORD PTR [bp-8],ax	;inc_minus_one
	mov	WORD PTR [bp-6],dx
	mov	ax,WORD PTR [bp-16]	;inc
	mov	dx,WORD PTR [bp-14]
	sub	ax,08000h
	sbb	dx,0
        mov     si,ax                   ;errL
        mov     di,dx                   ;errH
	or	dx,dx
	jl	hhalf
	xor	ax,ax
	jmp	SHORT halpha
hhalf:	mov	ax,007Fh
halpha:	mov	bx,WORD PTR [bp+4]	;x1
	cmp	bx,WORD PTR [bp+8]	;x2
	je	hend
hloop:
	push	ax                      ;alpha for second call to aapix
	not	al
	push	ax                      ;alpha
	push	WORD PTR [bp+6]	        ;y1
        push    bx                      ;x1
	call	WORD PTR _aapixel
	add	sp,6
	mov	ax,WORD PTR [bp+6]	;y1
	add	ax,WORD PTR [bp-20]	;sy
	push	ax
	push    WORD PTR [bp+4]	        ;x1
	call	WORD PTR _aapixel
	add	sp,6
        or      di,di
	jge	hnext
hstep:  mov     ax,si
        mov     al,ah
        xor     ah,ah
	add	si,WORD PTR [bp-16]	;inc
	adc	di,WORD PTR [bp-14]
	mov	bx,WORD PTR [bp+4]	;x1
	add	bx,WORD PTR [bp-18]	;sx
	mov	WORD PTR [bp+4],bx	;x1
	cmp	bx,WORD PTR [bp+8]	;x2
	jne	hloop
        jmp     SHORT hend
hnext:	xor	ax,ax
	add	si,WORD PTR [bp-8]	;inc_minus_one
	adc	di,WORD PTR [bp-6]
	mov	bx,WORD PTR [bp-20]	;sy
	add	WORD PTR [bp+6],bx	;y1
	mov	bx,WORD PTR [bp+4]	;x1
	add	bx,WORD PTR [bp-18]	;sx
	mov	WORD PTR [bp+4],bx	;x1
	cmp	bx,WORD PTR [bp+8]	;x2
	jne	hloop
hend:
	push	ax                      ;alpha for second call to aapix
	not	al
	push	ax                      ;alpha
	push	WORD PTR [bp+10]	;y2
	push	WORD PTR [bp+8]	        ;x2
	call	WORD PTR _aapixel
	add	sp,6
	mov	ax,WORD PTR [bp+10]	;y2
	add	ax,WORD PTR [bp-20]	;sy
	push	ax
	push	WORD PTR [bp+8]	        ;x2
	jmp	aexit
;
;
;
y_major:
        or      ax,ax                   ;dx
	jne	vsetup
	or	cx,cx	                ;sy
        jle     vrev
	push	WORD PTR [bp+10]	;y2
	push	WORD PTR [bp+6]	        ;y1
        jmp     SHORT vspan
vrev:	push	WORD PTR [bp+6]	        ;y1
	push	WORD PTR [bp+10]	;y2
vspan:  push	WORD PTR [bp+4]	        ;x1
	call	WORD PTR _aavspan
	jmp	exit
vsetup:
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
	sub	ax,08000h
	sbb	dx,0
        mov     si,ax                   ;errL
        mov     di,dx                   ;errH
	or	dx,dx
	jl	vhalf
	xor	ax,ax
	jmp	SHORT valpha
vhalf:	mov	ax,007Fh
valpha:	mov	bx,WORD PTR [bp+6]	;y1
	cmp	bx,WORD PTR [bp+10]	;y2
	je	vend
vloop:
	push	ax                      ;alpha for second call to aapix
	not	al
	push	ax                      ;alpha
	push	bx                      ;y1
	push	WORD PTR [bp+4]	        ;x1
	call	WORD PTR _aapixel
	add	sp,6
	push	WORD PTR [bp+6]	        ;y1
	mov	ax,WORD PTR [bp+4]	;x1
	add	ax,WORD PTR [bp-18]	;sx
	push	ax
	call	WORD PTR _aapixel
	add	sp,6
        or      di,di
	jge	vnext
vstep:  mov     ax,si
        mov     al,ah
        xor     ah,ah
	add	si,WORD PTR [bp-16]	;inc
	adc	di,WORD PTR [bp-14]
	mov	bx,WORD PTR [bp+6]	;y1
	add	bx,WORD PTR [bp-20]	;sy
	mov	WORD PTR [bp+6],bx	;y1
	cmp	bx,WORD PTR [bp+10]	;y2
	jne	vloop
	jmp	SHORT vend
vnext:  xor	ax,ax
	add	si,WORD PTR [bp-8]	;inc_minus_one
	adc	di,WORD PTR [bp-6]
	mov	bx,WORD PTR [bp-18]	;sx
	add	WORD PTR [bp+4],bx	;x1
	mov	bx,WORD PTR [bp+6]	;y1
	add	bx,WORD PTR [bp-20]	;sy
	mov	WORD PTR [bp+6],bx	;y1
	cmp	bx,WORD PTR [bp+10]	;y2
	jne	vloop
vend:
	push	ax                      ;alpha for second call to aapix
	not	al
	push	ax                      ;alpha
	push	WORD PTR [bp+10]	;y2
	push	WORD PTR [bp+8]	        ;x2
	call	WORD PTR _aapixel
	add	sp,6
	push	WORD PTR [bp+10]	;y2
	mov	ax,WORD PTR [bp+8]	;x2
	add	ax,WORD PTR [bp-18]	;sx
	push	ax
aexit:	call	WORD PTR _aapixel
exit:	add	sp,6
	pop	si
	pop	di
	mov	sp,bp
	pop	bp
	ret	

_aaline	ENDP
_TEXT	ENDS
END
