;	Static Name Aliases
;
	TITLE   BITBLT.C
	NAME    BITBLT

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
EXTRN	_hspan:NEAR

incsrc  MACRO
        LOCAL   noincp
        shr	al,1	                ;srcmask
	or	al,al
	jnz	noincp
	mov	al,080h	                ;srcmask
	inc	bx	                ;srcspan
	mov     ah,BYTE PTR es:[bx]
noincp:
        ENDM


_TEXT      SEGMENT
	ASSUME	CS: _TEXT

        PUBLIC	_bitblt
_bitblt	PROC NEAR
	push	bp
	mov	bp,sp
	sub	sp,12
;	xoffset = 12
;	yoffset = 14
;	src = 16
;	pitch = 20
;	x = -6
;	xr = -12
;	lmask = -4
;	srcmask = -2
;	srcspan = -10
;	xl = 4
;	yt = 6
;	width = 8
;	height = 10
	push	di
	push	si
	mov	ax,WORD PTR [bp+14]	;yoffset
	mul	WORD PTR [bp+20]	;pitch
	mov	dx,WORD PTR [bp+12]	;xoffset
        mov     cl,07h
        and     cl,dl
	shr	dx,1
	shr	dx,1
	shr	dx,1
	add	ax,dx
	add	WORD PTR [bp+16],ax	;src
	mov	al,080h
	shr	al,cl
	mov	BYTE PTR [bp-4],al	;lmask
yloop:	mov	ax,WORD PTR [bp+16]	;src
	mov	dx,WORD PTR [bp+18]
	mov	WORD PTR [bp-10],ax	;srcspan
	mov	WORD PTR [bp-8],dx
	add	ax,WORD PTR [bp+20]	;pitch
	mov	WORD PTR [bp+16],ax	;src
	mov	al,BYTE PTR [bp-4]	;lmask
	mov	BYTE PTR [bp-2],al	;srcmask
	mov	si,WORD PTR [bp+4]	;xl
	mov	di,WORD PTR [bp+8]	;width
	les	bx,DWORD PTR [bp-10]	;srcspan
chksrc: mov     ah,BYTE PTR es:[bx]
        or      ah,ah
        jnz     bitscan
        inc     bx
        add     si,8
        sub     di,8
        jg      chksrc
        jmp     ynext
bitscan: mov	al,BYTE PTR [bp-2]	;srcmask
	test    al,ah
	jnz	fill
trans:	dec     di
	jz	ynext
	inc	si
        incsrc
        test    al,ah
        jz      trans
fill:   mov     cx,si                   ;startspan
        dec     di
        jz      fillpix
        incsrc
        test    al,ah
        jnz     floop
fillpix:
	mov	WORD PTR [bp-10],bx	;srcspan
	mov	BYTE PTR [bp-2],al	;srcmask
	push	WORD PTR [bp+6]	        ;yt
	push	cx
	call	WORD PTR _pixel
	add	sp,4
        or      di,di
        jz      ynext
        mov	al,BYTE PTR [bp-2]	;srcmask
	les	bx,DWORD PTR [bp-10]	;srcspan
	mov     ah,BYTE PTR es:[bx]
        inc     si
        jmp     SHORT trans
floop:  inc     si
        dec     di
        jz      fillspan
        incsrc
        test    al,ah
        jnz     floop
fillspan:
	mov	WORD PTR [bp-10],bx	;srcspan
	mov	BYTE PTR [bp-2],al	;srcmask
	push	WORD PTR [bp+6]	        ;yt
	push	si                      ;xr
	push	cx                      ;xl
	call	WORD PTR _hspan
	add	sp,6
        or      di,di
        jz      ynext
        mov	al,BYTE PTR [bp-2]	;srcmask
	les	bx,DWORD PTR [bp-10]	;srcspan
	mov     ah,BYTE PTR es:[bx]
        inc     si
        jmp     SHORT trans
ynext:  inc	WORD PTR [bp+6]	        ;yt
	dec	WORD PTR [bp+10]	;height
        jz      bbexit
	jmp	yloop
bbexit: pop	si
	pop	di
	mov	sp,bp
	pop	bp
	ret	

_bitblt	ENDP
_TEXT	ENDS
END
