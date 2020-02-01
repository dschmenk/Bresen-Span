;	Static Name Aliases
;
	TITLE   FASTLINE.ASM
	NAME    FASTLINE

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

EXTRN	_hspan:NEAR
EXTRN	_vspan:NEAR

_TEXT   SEGMENT

        PUBLIC	_line
_line	PROC NEAR
	push	bp
	mov	bp,sp
	sub	sp,22
	push	di
	push	si
;	longlen = -18
;	halflen = -10
;	x1 = 4
;	y1 = 6
;	x2 = 8
;	y2 = 10
;	dx2 = -16
;	dy2 = -22
;	err = -8
;	sx = -12
;	sy = -14
;	ps = -2
;	shorterr = -6
;	shortlen = -20
;	longerr = -4
	mov	dx,1
        mov     cx,dx
	mov	ax,WORD PTR [bp+8]	;x2
	sub	ax,WORD PTR [bp+4]	;x1
	shl	ax,1
	jge	x_abs
        neg     dx	                ;sx
	neg	ax                      ;dx2
x_abs:
	mov	bx,WORD PTR [bp+10]	;y2
	sub	bx,WORD PTR [bp+6]	;y1
	shl	bx,1
	jge	y_abs
        neg     cx	                ;sy
	neg	bx                      ;dy2
y_abs:
	mov	WORD PTR [bp-16],ax	;dx2
	mov	WORD PTR [bp-22],bx	;dy2
	cmp	ax,bx	                ;dx2>=dy2?
	jge	x_major
	jmp	y_major
;
;
;
x_major:
        or      dx,dx                   ;sx
	jge	leftright
	mov	si,WORD PTR [bp+4]	;x1
	mov	di,WORD PTR [bp+8]	;x2
	mov	WORD PTR [bp+4],di	;x1
	mov	WORD PTR [bp+8],si	;x2
	mov	si,WORD PTR [bp+6]	;y1
	mov	di,WORD PTR [bp+10]	;y2
	mov	WORD PTR [bp+6],di	;y1
	mov	WORD PTR [bp+10],si	;y2
	neg	cx                      ;sy
leftright:
        or      bx,bx                   ;dy2
	jne	hsetup
	push	WORD PTR [bp+6]	        ;y1
	push	WORD PTR [bp+8]	        ;x2
	push	WORD PTR [bp+4]	        ;x1
	call	WORD PTR _hspan
	jmp	exit
hsetup:
	mov	WORD PTR [bp-14],cx	;sy
        mov	si,WORD PTR [bp+4]	;x1
	mov	WORD PTR [bp-2],si	;ps
        shr     ax,1                    ;dx2/2
;	mov	cx,ax
	xor     dx,dx
	div	bx                      ;dy2
	mov	cx,bx	                ;err=dy2-rem
        sub     cx,dx
        add	si,ax	                ;x1+=halflen
;	inc	ax
;	mul	bx                      ;dy2
;       sub	ax,cx                   ;dx2/2
;	mov	cx,ax	                ;err
	mov	dx,si	                ;x1
	sub	dx,WORD PTR [bp-2]	;ps
        inc     dx
	shl	dx,1                    ;longlen=(x1-ps+1)*2
	mov	di,cx	                ;err
	shl	di,1                    ;longerr=err*2
	cmp	di,bx	                ;longerr>=dy2?
	jl	nohround
	sub	di,bx	                ;longerr-dy2
	dec	dx	                ;longlen--
nohround:
	mov	WORD PTR [bp-18],dx	;longlen
	dec	dx
	mov	WORD PTR [bp-20],dx	;shortlen
	mov	WORD PTR [bp-4],di	;longerr
	sub	di,bx	                ;dy2
        mov	WORD PTR [bp-6],di	;shorterr
        add	di,cx	                ;err
	cmp	si,WORD PTR [bp+8]	;x1<x2?
	jge	hlast
hloop:
	push	WORD PTR [bp+6]	        ;y1
	push	si                      ;x1
	push	WORD PTR [bp-2]	        ;ps
	call	WORD PTR _hspan
	add	sp,6
	mov	ax,WORD PTR [bp-14]	;sy
	add	WORD PTR [bp+6],ax	;y1
	lea	ax,WORD PTR [si+1]
	mov	WORD PTR [bp-2],ax	;ps
	or	di,di                   ;err
	jl	hlong
hshort:
        add	di,WORD PTR [bp-6]	;shorterr
	add	si,WORD PTR [bp-20]	;shortlen
	cmp	si,WORD PTR [bp+8]	;x2
	jl	hloop
	jmp	SHORT hlast
hlong:
	add	di,WORD PTR [bp-4]	;longerr
	add	si,WORD PTR [bp-18]	;longlen
	cmp	si,WORD PTR [bp+8]	;x2
	jl	hloop
hlast:
	push	WORD PTR [bp+10]	;y2
	push	WORD PTR [bp+8]	        ;x2
	push	WORD PTR [bp-2]	        ;ps
	call	WORD PTR _hspan
	jmp	exit
;
;
;
y_major:
	or	cx,cx	                ;sy
	jge	topbottom
	mov	si,WORD PTR [bp+4]	;x1
	mov	di,WORD PTR [bp+8]	;x2
	mov	WORD PTR [bp+4],di	;x1
	mov	WORD PTR [bp+8],si	;x2
	mov	si,WORD PTR [bp+6]	;y1
	mov	di,WORD PTR [bp+10]	;y2
	mov	WORD PTR [bp+6],di	;y1
	mov	WORD PTR [bp+10],si	;y2
	neg	dx                      ;sx
topbottom:
        or      ax,ax                   ;dx2
	jne	vsetup
	push	WORD PTR [bp+10]	;y2
	push	WORD PTR [bp+6]	        ;y1
	push	WORD PTR [bp+4]	        ;x1
	jmp	vexit
vsetup:
	mov	WORD PTR [bp-12],dx	;sx
        mov	si,WORD PTR [bp+6]	;y1
	mov	WORD PTR [bp-2],si	;ps
        mov	cx,ax	                ;dx2
        shr     bx,1                    ;dy2/2
	mov	ax,bx
	xor     dx,dx
	div	cx
	add	si,ax	                ;y1+=halflen
	mov	bx,cx	                ;err=dy2-rem
        sub     bx,dx
;	inc	ax
;	mul	cx
;	sub	ax,bx
;	mov	bx,ax	                ;err
	mov	dx,si	                ;y1
	sub	dx,WORD PTR [bp-2]	;ps
        inc     dx
	shl	dx,1                    ;longlen=(y1-ps+1)*2
	mov	di,bx	                ;err
	shl	di,1                    ;err*2
	cmp	di,cx	                ;longerr>=dx2?
	jl	novround
	sub	di,cx	                ;longerr-dx2
	dec	dx	                ;longlen--
novround:
	mov	WORD PTR [bp-18],dx	;longlen
	dec	dx
	mov	WORD PTR [bp-20],dx	;shortlen
	mov	WORD PTR [bp-4],di	;longerr
	sub	di,cx	                ;dx2
	mov	WORD PTR [bp-6],di	;shorterr
	add	di,bx	                ;err
	cmp	si,WORD PTR [bp+10]	;y1<y2?
	jge	vlast
vloop:
	push	si                      ;y1
	push	WORD PTR [bp-2]	        ;ps
	push	WORD PTR [bp+4]	        ;x1
	call	WORD PTR _vspan
	add	sp,6
	mov	ax,WORD PTR [bp-12]	;sx
	add	WORD PTR [bp+4],ax	;x1
	lea	ax,WORD PTR [si+1]
	mov	WORD PTR [bp-2],ax	;ps
	or	di,di                   ;err
	jl	vlong
vshort:
	add	di,WORD PTR [bp-6]	;shorterr
	add	si,WORD PTR [bp-20]	;shortlen
	cmp	si,WORD PTR [bp+10]	;y2
	jl	vloop
        jmp     SHORT vlast
vlong:
	add	di,WORD PTR [bp-4]	;longerr
	add	si,WORD PTR [bp-18]	;longlen
	cmp	si,WORD PTR [bp+10]	;y2
	jl	vloop
vlast:
	push	WORD PTR [bp+10]	;y2
	push	WORD PTR [bp-2]	        ;ps
	push	WORD PTR [bp+8]	        ;x2
vexit:
	call	WORD PTR _vspan
exit:
	add	sp,6
	pop	si
	pop	di
	mov	sp,bp
	pop	bp
	ret	
_line	ENDP
_TEXT	ENDS
END

