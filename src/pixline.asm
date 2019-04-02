;	Static Name Aliases
;
	TITLE   LINE.ASM
	NAME    LINE

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
EXTRN	_pixel:NEAR
_TEXT   SEGMENT
	ASSUME	CS: _TEXT
	PUBLIC	_pixline
_pixline        PROC NEAR
	push	bp
	mov	bp,sp
	sub	sp,20
	push	di
	push	si
;	x1 = 4
;	y1 = 6
;	x2 = 8
;	y2 = 10
;	dx2 = -14
;	dy2 = -20
;	err = -8
;	sx = -10
;	sy = -12
;	ps = -2
;	shorterr = -6
;	shortlen = -18
;	longerr = -4
;	longlen = -16
	mov	dx,1                    ;sx
        mov     cx,dx                   ;sy
	mov	ax,WORD PTR [bp+8]	;x2
	sub	ax,WORD PTR [bp+4]	;x1
	shl	ax,1
	or	ax,ax
	jge	x_abs
	neg	dx                      ;sx
        neg     ax
x_abs:
	mov	bx,WORD PTR [bp+10]	;y2
	sub	bx,WORD PTR [bp+6]	;y1
	shl	bx,1
	or	bx,bx
	jge	y_abs
	neg     cx	                ;sy
	neg	bx
y_abs:
	mov	WORD PTR [bp-14],ax	;dx2
	mov	WORD PTR [bp-20],bx	;dy2
        cmp     bx,ax                   ;dx2>=dy2?
	jg	y_major
;
;
;
x_major:
	mov	di,bx	                ;dy2
	or      dx,dx	                ;sx
	jge	leftright
	mov	dx,WORD PTR [bp+4]	;x1
	mov	bx,WORD PTR [bp+8]	;x2
	mov	WORD PTR [bp+4],bx	;x1
	mov	WORD PTR [bp+8],dx	;x2
	mov	dx,WORD PTR [bp+6]	;y1
	mov	bx,WORD PTR [bp+10]	;y2
	mov	WORD PTR [bp+6],bx	;y1
	mov	WORD PTR [bp+10],dx	;y2
	neg	cx                      ;sy
leftright:
	mov	WORD PTR [bp-12],cx	;sy
;	mov	ax,WORD PTR [bp-14]	;dx2
	shr	ax,1                    ;dx2/2
	sub	di,ax                   ;err
	mov	si,WORD PTR [bp+4]	;x1
	cmp	si,WORD PTR [bp+8]	;x2
	jge	exit
xloop:
	push	WORD PTR [bp+6]	        ;y1
	push	si                      ;x
	call	WORD PTR _pixel
	add	sp,4
	or	di,di
	jl	xstep
	sub	di,WORD PTR [bp-14]	;dx2
	mov	ax,WORD PTR [bp-12]	;sy
	add	WORD PTR [bp+6],ax	;y1
xstep:
	add	di,WORD PTR [bp-20]	;dy2
	inc	si
	cmp	si,WORD PTR [bp+8]	;x2
	jl	xloop
	jmp	exit
;
;
;
y_major:
	mov	di,ax	                ;dx2
        or      cx,cx	                ;sy
	jge	topbottom
	mov	ax,WORD PTR [bp+4]	;x1
	mov	cx,WORD PTR [bp+8]	;x2
	mov	WORD PTR [bp+4],cx	;x1
	mov	WORD PTR [bp+8],ax	;x2
	mov	ax,WORD PTR [bp+6]	;y1
	mov	cx,WORD PTR [bp+10]	;y2
	mov	WORD PTR [bp+6],cx	;y1
	mov	WORD PTR [bp+10],ax	;y2
	neg	dx                      ;sx
topbottom:
	mov	WORD PTR [bp-10],dx	;sx
;	mov	cx,WORD PTR [bp-20]	;dy2
        shr     bx,1                    ;dy2/2
	sub	di,bx
	mov	si,WORD PTR [bp+6]	;y1
	cmp	si,WORD PTR [bp+10]	;y2
	jge	exit
yloop:
	push	si                      ;y
	push	WORD PTR [bp+4]	        ;x1
	call	WORD PTR _pixel
	add	sp,4
	or	di,di
	jl	ystep
	sub	di,WORD PTR [bp-20]	;dy2
	mov	ax,WORD PTR [bp-10]	;sx
	add	WORD PTR [bp+4],ax	;x1
ystep:
	add	di,WORD PTR [bp-14]	;dx2
	inc	si                      ;y1
	cmp	si,WORD PTR [bp+10]	;y2
	jl	yloop
exit:
	push	WORD PTR [bp+10]	;y2
	push	WORD PTR [bp+8]	        ;x2
	call	WORD PTR _pixel
	add	sp,4
	pop	si
	pop	di
	mov	sp,bp
	pop	bp
	ret	
_pixline	ENDP
_TEXT	ENDS
END

