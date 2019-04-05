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
EXTRN	_idx8:WORD
EXTRN   _brush8:BYTE

_TEXT   SEGMENT
	ASSUME	CS: _TEXT

        PUBLIC	_hspan8brush
_hspan8brush	PROC NEAR
	push	bp
	mov	bp,sp
	push	di
	push	si
;	xl = 4
;	xr = 6
;	y = 8
        mov     ax,0A000h               ;video segment
        mov     es,ax
        mov     bx,WORD PTR [bp+4]	;xl
	mov	ax,320
	mov	si,WORD PTR [bp+8]	;y
	imul	si	                ;y
        mov     di,ax                   ;pix
        add     di,bx
	mov	cx,WORD PTR [bp+6]	;xr
        and     si,3
	shl	si,1
	shl	si,1
        add     si,OFFSET _brush8
        sub     cx,bx
	and	bx,3
	mov	al,BYTE PTR [si][bx]
	mov	BYTE PTR es:[di],al
	inc	di
        or      cx,cx
        jz      hbexit
        inc     bx
	and	bx,3
	mov	ah,BYTE PTR [si][bx]
	mov	BYTE PTR es:[di],ah
	inc	di
        dec     cx
        jz      hbexit
        inc     bx
	and	bx,3
	mov	dl,BYTE PTR [si][bx]
	mov	BYTE PTR es:[di],dl
	inc	di
        dec     cx
        jz      hbexit
        inc     bx
	and	bx,3
	mov	dh,BYTE PTR [si][bx]
	mov	BYTE PTR es:[di],dh
	inc	di
        dec     cx
        jz      hbexit
hloopb:	mov	BYTE PTR es:[di],al
	inc	di
        dec     cx
        jz      hbexit
	mov	BYTE PTR es:[di],ah
	inc	di
        dec     cx
        jz      hbexit
	mov	BYTE PTR es:[di],dl
	inc	di
        dec     cx
        jz      hbexit
	mov	BYTE PTR es:[di],dh
	inc	di
        loop    hloopb
hbexit:	pop	si
	pop	di
	mov	sp,bp
	pop	bp
	ret	
_hspan8brush	ENDP
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
        inc     cx
        rep     stosb
	pop	di
	mov	sp,bp
	pop	bp
	ret	
_hspan8	        ENDP

        PUBLIC	_vspan8brush
_vspan8brush	PROC NEAR
	push	bp
	mov	bp,sp
	push	di
	push	si
;	x = 4
;	yt = 6
;	yb = 8
        mov     ax,0A000h               ;video segment
        mov     es,ax
	mov	bx,WORD PTR [bp+6]      ;yt
	mov	ax,320
	imul	bx	                ;yt
	mov	si,WORD PTR [bp+4]	;x
        mov     di,si
        add     di,ax                   ;pix
        and     si,3
        add     si,OFFSET _brush8
	mov	cx,WORD PTR [bp+8]	;yb
        sub     cx,bx
	and	bx,3
        shl     bx,1
        shl     bx,1
	mov	al,BYTE PTR [si][bx]
	mov	BYTE PTR es:[di],al
	add	di,320
        or      cx,cx
	jz      vbexit
        add     bx,4
        and     bx,0Fh
	mov	ah,BYTE PTR [si][bx]
	mov	BYTE PTR es:[di],ah
	add	di,320
        dec     cx
	jz      vbexit
        add     bx,4
        and     bx,0Fh
	mov	dl,BYTE PTR [si][bx]
	mov	BYTE PTR es:[di],dl
	add	di,320
        dec     cx
	jz      vbexit
        add     bx,4
        and     bx,0Fh
	mov	dh,BYTE PTR [si][bx]
	mov	BYTE PTR es:[di],dh
	add	di,320
        dec     cx
	jz      vbexit
vloopb:	mov	BYTE PTR es:[di],al
	add	di,320
        dec     cx
	jz      vbexit
	mov	BYTE PTR es:[di],ah
	add	di,320
        dec     cx
	jz      vbexit
	mov	BYTE PTR es:[di],dl
	add	di,320
        dec     cx
	jz      vbexit
	mov	BYTE PTR es:[di],dh
	add	di,320
	loop	vloopb
vbexit:	pop	si
	pop	di
	mov	sp,bp
	pop	bp
	ret	
_vspan8brush	ENDP
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

        PUBLIC	_pixel8brush
_pixel8brush    PROC NEAR
	push	bp
	mov	bp,sp
;	x = 4
;	y = 6
	mov	ax,320
        mov     bx, WORD PTR [bp+6]     ;y
        imul    bx                      ;y
        and     bx,3
        shl     bl,1
        shl     bl,1
	mov	cx,WORD PTR [bp+4]	;x
	add	ax,cx                   ;x
        and     cx, 3
        add     bx,cx
        mov     dl, BYTE PTR _brush8[bx]
        mov     bx,ax
        mov     ax, 0A000h              ;video segment
        mov     es, ax
	mov	es:[bx],dl
	mov	sp,bp
	pop	bp
	ret	
_pixel8brush    ENDP
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

        PUBLIC	_pixel8rgb
_pixel8rgb	PROC NEAR
	push	bp
	mov	bp,sp
	push	si
;	x   = 4
;	y   = 6
;       red = 8
;       grn = 10
;       blu = 12
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
	mov	bl,BYTE PTR [bp+8]      ;red
	mov     cl,BYTE PTR _mapr[bx][si]
	mov	bl,BYTE PTR [bp+10]     ;grn
	or	cl,BYTE PTR _mapg[bx][si]
	mov	bl,BYTE PTR [bp+12]     ;blu
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
_TEXT	ENDS
END
