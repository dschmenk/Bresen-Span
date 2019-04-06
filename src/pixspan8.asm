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
EXTRN   _amulr:BYTE
EXTRN   _amulg:BYTE
EXTRN   _amulb:BYTE
EXTRN	_idx8:WORD
EXTRN	_rgb8:BYTE
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
        mov     cl,6
        mov     bx,WORD PTR [bp+8]      ;y
        mov     ax,bx
        shl     ax,cl                   ;y*64
        add     ah,bl                   ;y*256
        mov     si,WORD PTR [bp+4]	;xl
        add     ax,si
        mov     di,ax                   ;pix
	mov	cx,WORD PTR [bp+6]	;xr
        and     bx,3
	shl	bx,1
	shl	bx,1
        add     bx,OFFSET _brush8
        sub     cx,si
	and	si,3
	mov	al,BYTE PTR [si][bx]
	mov	BYTE PTR es:[di],al
        or      cx,cx
        jz      hbexit
	inc	di
        inc     si
	and	si,3
	mov	ah,BYTE PTR [si][bx]
	mov	BYTE PTR es:[di],ah
        dec     cx
        jz      hbexit
	inc	di
        inc     si
	and	si,3
	mov	dl,BYTE PTR [si][bx]
	mov	BYTE PTR es:[di],dl
        dec     cx
        jz      hbexit
	inc	di
        inc     si
	and	si,3
	mov	dh,BYTE PTR [si][bx]
	mov	BYTE PTR es:[di],dh
        dec     cx
        jz      hbexit
hloopb:	inc	di
	mov	BYTE PTR es:[di],al
        dec     cx
        jz      hbexit
	inc	di
	mov	BYTE PTR es:[di],ah
        dec     cx
        jz      hbexit
	inc	di
	mov	BYTE PTR es:[di],dl
        dec     cx
        jz      hbexit
	inc	di
	mov	BYTE PTR es:[di],dh
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
        mov     cl,6
        mov     bx,WORD PTR [bp+8]      ;y
        mov     ax,bx
        shl     ax,cl                   ;y*64
        add     ah,bl                   ;y*256
        mov     bx,WORD PTR [bp+4]	;xl
        add     ax,bx
        mov     di,ax                   ;pix
        mov     cx,WORD PTR [bp+6]	;xr
        sub     cx,bx                   ;xr-xl
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
        mov     cl,6
        mov     ax,bx                   ;yt
        shl     ax,cl                   ;yt*64
        add     ah,bl                   ;yt*256
	mov	si,WORD PTR [bp+4]	;x
        add     ax,si
        mov     di,ax                   ;pix
        and     si,3
        add     si,OFFSET _brush8
	mov	cx,WORD PTR [bp+8]	;yb
        sub     cx,bx
	and	bx,3
        shl     bx,1
        shl     bx,1
	mov	al,BYTE PTR [si][bx]
	mov	BYTE PTR es:[di],al
        or      cx,cx
	jz      vbexit
	add	di,320
        add     bx,4
        and     bx,0Fh
	mov	ah,BYTE PTR [si][bx]
	mov	BYTE PTR es:[di],ah
        dec     cx
	jz      vbexit
	add	di,320
        add     bx,4
        and     bx,0Fh
	mov	dl,BYTE PTR [si][bx]
	mov	BYTE PTR es:[di],dl
        dec     cx
	jz      vbexit
	add	di,320
        add     bx,4
        and     bx,0Fh
	mov	dh,BYTE PTR [si][bx]
	mov	BYTE PTR es:[di],dh
        dec     cx
	jz      vbexit
vloopb:	add	di,320
	mov	BYTE PTR es:[di],al
        dec     cx
	jz      vbexit
	add	di,320
	mov	BYTE PTR es:[di],ah
        dec     cx
	jz      vbexit
	add	di,320
	mov	BYTE PTR es:[di],dl
        dec     cx
	jz      vbexit
	add	di,320
	mov	BYTE PTR es:[di],dh
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
;	x = 4
;	yt = 6
;	yb = 8
        mov     ax,0A000h               ;video segment
        mov     es,ax
        mov     cl,6
	mov	bx,WORD PTR [bp+6]      ;yt
        mov     ax,bx                   ;yt
        shl     bx,cl                   ;yt*64
        add     bh,al                   ;yt*256
	add	bx,WORD PTR [bp+4]	;x
	mov	cx,WORD PTR [bp+8]	;yb
        sub     cx,ax                   ;yb-yt
	mov	al,BYTE PTR _idx8
        inc     cx
vloopi:	mov	BYTE PTR es:[bx],al
	add	bx,320
	loop	vloopi
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
        mov     cl,6
        mov     bx, WORD PTR [bp+6]     ;y
        mov     ax,bx                   ;y
        shl     ax,cl                   ;y*64
        add     ah,bl                   ;y*256
        and     bx,3
        shl     bl,1
        shl     bl,1
	mov	cx,WORD PTR [bp+4]	;x
	add	ax,cx                   ;x
        and     cx, 3
        add     bx,cx
        mov     dl, BYTE PTR _brush8[bx]
        mov     bx,ax
        mov     ax,0A000h               ;video segment
        mov     es,ax
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
	mov	cl,6
        mov     bx,WORD PTR [bp+6]      ;y
        mov     ax,bx
        shl     bx,cl                   ;y*64
        add     bh,al                   ;y*256
	add	bx,WORD PTR [bp+4]	;x
        mov     ax,0A000h               ;video segment
        mov     es,ax
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
	mov	cl,6
	mov	bx,WORD PTR [bp+6]	;y
        mov     ax,bx
        shl     ax,cl                   ;y*64
        add     ah,bl	                ;y*256
	and	bx,3
	shl	bx,1
	shl	bx,1
	mov	si,WORD PTR [bp+4]	;x
	add	ax,si                   ;+x
	and	si,3
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
        mov     ax,0A000h               ;video segment
        mov     es,ax
	mov	es:[bx],cl
	pop	si
	mov	sp,bp
	pop	bp
	ret	
_pixel8rgb	ENDP

        PUBLIC	_pixel8alpha
_pixel8alpha	PROC NEAR
	push	bp
	mov	bp,sp
	push	di
	push	si
;	x     = 4
;	y     = 6
;       alpha = 8
        mov     ax,0A000h               ;video segment
        mov     es,ax
	mov	cl,6
	mov	bx,WORD PTR [bp+6]	;y
        mov     ax,bx
        shl     ax,cl                   ;y*64
        add     ah,bl	                ;y*256
	add	ax,WORD PTR [bp+4]	;x
	mov	di,ax                   ;+x
        mov     si,WORD PTR [bp+8]      ;alpha
        cmp     si,00E0h
        jl      aamix
        mov     al,BYTE PTR _idx8
	mov	es:[di],al
        jmp     SHORT aaexit
aamix:  and     si,00E0h
        jz      aaexit
        shr     si,1
        shr     si,1
        mov     cl,5
	mov	dl,es:[di]
	mov	bl,BYTE PTR _rgb8+0     ;red
        shr     bl,cl
	mov     al,BYTE PTR _amulr[bx][si]
        mov     bl,dl
        and     bl,07h
        xor     si,0038h
        add     al,BYTE PTR _amulr[bx][si]
	mov	bl,BYTE PTR _rgb8+1     ;grn
        shr     bl,cl
        xor     si,0038h
	or	al,BYTE PTR _amulg[bx][si]
        mov     bl,dl
        shr     bl,1
        shr     bl,1
        shr     bl,1
        and     bl,07h
        xor     si,0038h
        add     al,BYTE PTR _amulg[bx][si]
	mov	bl,BYTE PTR _rgb8+3     ;blu
        inc     cl
        shr     bl,cl
        shr     si,1
        xor     si,001Ch
	or	al,BYTE PTR _amulb[bx][si]
        mov     bl,dl
        shr     bl,cl
        xor     si,001Ch
        add     al,BYTE PTR _amulb[bx][si]
	mov	es:[di],al
aaexit:	pop	si
	pop	di
	mov	sp,bp
	pop	bp
	ret	
_pixel8alpha	ENDP
_TEXT	ENDS
END

