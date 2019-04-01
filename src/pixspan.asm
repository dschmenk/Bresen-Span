;	Static Name Aliases
;
	TITLE   PIXSPAN.ASM
	NAME    PIXSPAN

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
EXTRN	_dithmatrix:BYTE
EXTRN	_mapr:BYTE
EXTRN	_mapg:BYTE
EXTRN	_mapb:BYTE
EXTRN	_red8:WORD
EXTRN	_grn8:WORD
EXTRN	_blu8:WORD
EXTRN	_idx8:WORD
_TEXT   SEGMENT
	ASSUME	CS: _TEXT

        PUBLIC	_hspan8rgb
_hspan8rgb	PROC NEAR
	push	bp
	mov	bp,sp
	push	di
	push	si
;	xl = 4
;	xr = 6
;	y = 8
        mov     cx,WORD PTR [bp+4]	;xl
        mov     ax,0A000h               ;video segment
        mov     es,ax
	mov	si,WORD PTR [bp+8]	;y
	mov	ax,320
	imul	si	                ;y
        mov     di,ax                   ;pix
        add     di,cx
        mov     dx,si
        and     dx,3
	shl	dx,1
	shl	dx,1
hloop:	mov	bx,cx
	and	bx,3
	add	bx,dx
	mov	bl,BYTE PTR _dithmatrix[bx]
        mov     si,bx
	mov	bl,BYTE PTR _red8
	mov     al,BYTE PTR _mapr[bx][si]
	mov	bl,BYTE PTR _grn8
	or	al,BYTE PTR _mapg[bx][si]
	mov	bl,BYTE PTR _blu8
	shl	si,1
	or	al,BYTE PTR _mapb[bx][si]
	mov	BYTE PTR es:[di],al
	inc	di
        inc     cx
	cmp	cx,WORD PTR [bp+6]	;xr
	jle	hloop
	pop	si
	pop	di
	mov	sp,bp
	pop	bp
	ret	
_hspan8rgb	ENDP
        PUBLIC	_hspan8
_hspan8	        PROC NEAR
	push	bp
	mov	bp,sp
	push	di
;	xl = 4
;	xr = 6
;	y = 8
        mov     ax,0A000h               ;video segment
        mov     es,ax
        mov     cx,WORD PTR [bp+6]	;xr
	mov	ax,320
	imul	WORD PTR [bp+8]	;y
        mov     di,ax                   ;pix
        mov     ax,WORD PTR [bp+4]	;xl
        add     di,ax
        sub     cx,ax                   ;xr-xl
	mov	al,BYTE PTR _idx8
        rep     stosb
	mov	BYTE PTR es:[di],al
	pop	di
	mov	sp,bp
	pop	bp
	ret	
_hspan8	        ENDP

        PUBLIC	_vspan8rgb
_vspan8rgb	PROC NEAR
	push	bp
	mov	bp,sp
	push	di
	push	si
;	x = 4
;	yt = 6
;	yb = 8
	mov	cx,WORD PTR [bp+6]      ;yt
        mov     ax,0A000h               ;video segment
        mov     es,ax
	mov	ax,320
	imul	cx	                ;yt
	mov	dx,WORD PTR [bp+4]	;x
        mov     di,ax                   ;pix
        add     di,dx
        and     dx,3
vloop:	mov	bx,cx
	and	bx,3
        shl     bx,1
        shl     bx,1
	add	bx,dx
	mov	bl,BYTE PTR _dithmatrix[bx]
        mov     si,bx
	mov	bl,BYTE PTR _red8
	mov     al,BYTE PTR _mapr[bx][si]
	mov	bl,BYTE PTR _grn8
	or	al,BYTE PTR _mapg[bx][si]
	mov	bl,BYTE PTR _blu8
	shl	si,1
	or	al,BYTE PTR _mapb[bx][si]
	mov	BYTE PTR es:[di],al
	add	di,320
        inc     cx
	cmp	cx,WORD PTR [bp+8]	;yb
	jle	vloop
	pop	si
	pop	di
	mov	sp,bp
	pop	bp
	ret	
_vspan8rgb	ENDP
        PUBLIC	_vspan8
_vspan8	        PROC NEAR
	push	bp
	mov	bp,sp
	push	di
;	x = 4
;	yt = 6
;	yb = 8
        mov     ax,0A000h               ;video segment
        mov     es,ax
	mov	bx,WORD PTR [bp+6]      ;yt
	mov	ax,320
	imul	bx	                ;yt
	mov	cx,WORD PTR [bp+8]	;yb
        mov     di,ax                   ;pix
	add	di,WORD PTR [bp+4]	;x
        sub     cx,bx                   ;yb-yt
	mov	al,BYTE PTR _idx8
        inc     cx
vloopi:	mov	BYTE PTR es:[di],al
	add	di,320
	loop	vloopi
	pop	di
	mov	sp,bp
	pop	bp
	ret	
_vspan8	        ENDP

        PUBLIC	_pixel8rgb
_pixel8rgb	PROC NEAR
	push	bp
	mov	bp,sp
	push	si
;	x = 4
;	y = 6
	mov	ax,320
	mov	si,WORD PTR [bp+6]	;y
        imul    si                      ;a = 320*y
	and	si,3
	shl	si,1
	shl	si,1
	mov	bx,WORD PTR [bp+4]	;x
	add	ax,bx                   ;a += x
	and	bx,3
	mov	bl,BYTE PTR _dithmatrix[bx][si]
	mov	si,bx
	mov	bl,BYTE PTR _red8
	mov     cl,BYTE PTR _mapr[bx][si]
	mov	bl,BYTE PTR _grn8
	or	cl,BYTE PTR _mapg[bx][si]
	mov	bl,BYTE PTR _blu8
	shl	si,1
	or	cl,BYTE PTR _mapb[bx][si]
	mov	bx,ax
        mov     ax, 0A000h              ;video segment
        mov     es, ax
	mov	es:[bx],cl
	pop	si
	mov	sp,bp
	pop	bp
	ret	
_pixel8rgb	ENDP
        PUBLIC	_pixel8
_pixel8	        PROC NEAR
	push	bp
	mov	bp,sp
;	x = 4
;	y = 6
	mov	ax,320
        imul    WORD PTR [bp+6]         ;y
        mov     bx,ax
	add	bx,WORD PTR [bp+4]	;x
        mov     ax, 0A000h              ;video segment
        mov     es, ax
	mov	al,BYTE PTR _idx8
	mov	es:[bx],al
	mov	sp,bp
	pop	bp
	ret	
_pixel8	        ENDP
_TEXT	ENDS
END
