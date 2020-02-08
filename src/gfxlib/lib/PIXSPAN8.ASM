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

EXTRN	_renderbuff:NEAR
EXTRN	_mapr:NEAR
EXTRN	_mapg:NEAR
EXTRN	_mapb:NEAR
EXTRN	_idx8:NEAR
EXTRN	_rgb8:NEAR
EXTRN   _brush8:NEAR
EXTRN   _amulr:NEAR
EXTRN   _amulg:NEAR
EXTRN   _amulb:NEAR

_TEXT   SEGMENT

	PUBLIC	_clear8
;void clear8(void)
_clear8         PROC NEAR
        push    di
        mov     dx,03C4h                ;plane enables
        mov     ax,0F02h
        out     dx,ax
        les     di,DWORD PTR _renderbuff ;video render page
        mov     cx,80*200/2
        mov     al,BYTE PTR _idx8       ;pattern
        mov     ah,al
        rep     stosw
        pop     di
        ret
_clear8         ENDP

        PUBLIC	_hspan8brush
_hspan8brush	PROC NEAR
	push	bp
	mov	bp,sp
;	xl = 4
;	xr = 6
;	y = 8
	push	di
        les     di,DWORD PTR _renderbuff ;video render page
        mov     dx,03CEh                ;clear pixmask
        mov     ax,0008h
        out     dx,ax
	mov	bx,WORD PTR [bp+8]	;y
        mov     ax,bx
        shl     ax,1
        shl     ax,1                    ;y*4
        add     ax,bx	                ;y*4+y
	mov	cl,4
        shl     ax,cl                   ;y*64+y*16=y*80
        add     di,ax
        and     bx,0003h
        mov     al,BYTE PTR es:[bx+80*200] ;latch brush
        mov     dx,0F0Fh
	mov	ax,WORD PTR [bp+4]	;xl
        mov     cl,03h
        and     cl,al
        shl     dl,cl                   ;lmask
        shr     ax,1
        shr     ax,1
	add	di,ax                   ;+x
        mov     bx,WORD PTR [bp+6]	;xr
        mov     cl,bl
        not     cl
        and     cl,03h
        shr     dh,cl                   ;rmask
        shr     bx,1
        shr     bx,1
        sub     bx,ax
        jnz     hbwide
;
; Narrow fill.        
;
        mov     ah,dl
        and     ah,dh
        mov     dx,03C4h                ;plane enables
        mov     al,02h
        out     dx,ax
        mov     BYTE PTR es:[di],al     ;dummy value, write latched brushes
        jmp     SHORT hbexit
;
; Wide fill.
;
hbwide: mov     ah,dl                   ;lmask
        mov     cl,dh
        mov     dx,03C4h                ;plane enables
        mov     al,02h
        out     dx,ax
        mov     BYTE PTR es:[di],al     ;dummy value, write latched brushes
        mov     al,cl                   ;rmask
        inc     dx
        out     dx,al                   ;plane enables
        mov     BYTE PTR es:[di][bx],al ;dummy value, write latched brushes
        dec     bx
        jz      hbexit
        mov     al,0FFh
        out     dx,al                   ;plane enables
        mov     cx,bx
        inc     di
        cld
        shr     cx,1
        jnc     hbfill
	stosb
hbfill:	rep     stosw
hbexit:	mov     dx,03CEh                ;reset pixmask
        mov     ax,0FF08h
        out     dx,ax
        pop	di
	pop	bp
	ret	
_hspan8brush	ENDP

        PUBLIC	_hspan8
_hspan8	        PROC NEAR
	push	bp
	mov	bp,sp
;	xl = 4
;	xr = 6
;	y = 8
	push	di
        les     di,DWORD PTR _renderbuff ;video render page
	mov	bx,WORD PTR [bp+8]	;y
        mov     ax,bx
        shl     ax,1
        shl     ax,1                    ;y*4
        add     ax,bx	                ;y*4+y
        mov     cl,4
        shl     ax,cl                   ;y*64+y*16=y*80
        add     di,ax
        mov     dx,0F0Fh
	mov	ax,WORD PTR [bp+4]	;xl
        mov     cl,03h
        and     cl,al
        shl     dl,cl                   ;lmask
        shr     ax,1
        shr     ax,1
	add	di,ax                   ;+x
        mov     bx,WORD PTR [bp+6]	;xr
        mov     cl,bl
        not     cl
        and     cl,03h
        shr     dh,cl                   ;rmask
        shr     bx,1
        shr     bx,1
        sub     bx,ax
        jnz     hswide
;
; Narrow fill.
;
        mov     ah,dl
        and     ah,dh
        mov     dx,03C4h                ;plane enables
        mov     al,02h
        out     dx,ax
	mov	al,BYTE PTR _idx8
        mov     BYTE PTR es:[di],al
        jmp     SHORT hsexit
;
; Wide fill.
;
hswide: mov     ah,dl                   ;lmask
        mov     cl,dh
        mov     dx,03C4h                ;plane enables
        mov     al,02h
        out     dx,ax
        mov	ch,BYTE PTR _idx8
        mov     BYTE PTR es:[di],ch
        mov     al,cl                   ;rmask
        inc     dx
        out     dx,al                   ;plane enables
        mov     BYTE PTR es:[di][bx],ch
        dec     bx
        jz      hsexit
        mov     al,0FFh
        out     dx,al                   ;plane enables
        mov     al,ch
        mov     cx,bx
        inc     di
        cld
        shr     cx,1
        jnc     hsfill
        stosb
hsfill: mov     ah,al
        rep     stosw
hsexit:	pop	di
	pop	bp
	ret	
_hspan8	        ENDP

        PUBLIC	_vspan8brush
_vspan8brush	PROC NEAR
	push	bp
	mov	bp,sp
;	x = 4
;	yt = 6
;	yb = 8
	push	di
	push	si
        les     di,DWORD PTR _renderbuff ;video render page
	mov	bx,WORD PTR [bp+6]      ;yt
        mov     ax,bx                   ;yt
        shl     ax,1
        shl     ax,1                    ;y*4
        add     ax,bx	                ;y*4+y
        mov     cl,4
        shl     ax,cl                   ;y*64+y*16=y*80
        add     di,ax                   ;pix
	mov	ax,WORD PTR [bp+4]	;x
        mov     cx,0003h
        and	cl,al
        shr     ax,1
        shr     ax,1
	add	di,ax                   ;+x
        mov     dx,03C4h                ;plane enables
        mov     ax,0102h
        shl     ah,cl
        out     dx,ax
        mov     si,OFFSET _brush8
        add     si,cx
        mov	cx,WORD PTR [bp+8]	;yb
        sub     cx,bx
        shl     bx,1
        shl     bx,1
	and	bx,000Ch
	mov	al,BYTE PTR [si][bx]
	mov	BYTE PTR es:[di],al
        jcxz    vbexit
	add	di,80
        add     bl,4
        and     bl,0Ch
	mov	ah,BYTE PTR [si][bx]
	mov	BYTE PTR es:[di],ah
        dec     cx
	jz      vbexit
	add	di,80
        add     bl,4
        and     bl,0Ch
	mov	dl,BYTE PTR [si][bx]
	mov	BYTE PTR es:[di],dl
        dec     cx
	jz      vbexit
	add	di,80
        add     bl,4
        and     bl,0Ch
	mov	dh,BYTE PTR [si][bx]
	mov	BYTE PTR es:[di],dh
        dec     cx
	jz      vbexit
vloopb:	add	di,80
	mov	BYTE PTR es:[di],al
        dec     cx
	jz      vbexit
	add	di,80
	mov	BYTE PTR es:[di],ah
        dec     cx
	jz      vbexit
	add	di,80
	mov	BYTE PTR es:[di],dl
        dec     cx
	jz      vbexit
	add	di,80
	mov	BYTE PTR es:[di],dh
	loop	vloopb
vbexit:	pop	si
	pop	di
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
        les     bx,DWORD PTR _renderbuff ;video render page
	mov	ax,WORD PTR [bp+4]	;x
        mov     cl,03h
        and     cl,al
        shr     ax,1
        shr     ax,1
	add	bx,ax                   ;+x
        mov     dx,03C4h                ;plane enables
        mov     ax,0102h
        shl     ah,cl
        out     dx,ax
	mov	ax,WORD PTR [bp+6]      ;yt
        mov     dx,ax                   ;yt
        shl     ax,1
        shl     ax,1                    ;y*4
        add     ax,dx	                ;y*4+y
        mov     cl,4
        shl     ax,cl                   ;y*64+y*16=y*80
	add	bx,ax
	mov	al,BYTE PTR _idx8
        mov	cx,WORD PTR [bp+8]	;yb
        sub     cx,dx                   ;yt
        inc     cx
vloopi:	mov	BYTE PTR es:[bx],al
	add	bx,80
	loop	vloopi
	pop	bp
	ret	
_vspan8	        ENDP

        PUBLIC	_pixel8brush
_pixel8brush    PROC NEAR
	push	bp
	mov	bp,sp
;	x = 4
;	y = 6
	push	di
        les     di,DWORD PTR _renderbuff ;video render page
        mov     bx, WORD PTR [bp+6]     ;y
        mov     ax,bx                   ;y
        shl     ax,1
        shl     ax,1                    ;y*4
        add     ax,bx	                ;y*4+y
        mov     cl,4
        shl     ax,cl                   ;y*64+y*16=y*80
        add     di,ax
	mov	ax,WORD PTR [bp+4]	;x
        mov     cl,03h
        and     cl,al
        shr     ax,1
        shr     ax,1
	add	di,ax                   ;+x
        mov     dx,03C4h                ;plane enables
        mov     ax,0102h
        shl     ah,cl
        out     dx,ax
        and     bx,0003h
        shl     bl,1
        shl     bl,1
        add     bl,cl
        mov     al,BYTE PTR _brush8[bx]
	mov	BYTE PTR es:[di],al
        pop     di
	pop	bp
	ret	
_pixel8brush    ENDP
        PUBLIC	_pixel8
_pixel8	        PROC NEAR
	push	bp
	mov	bp,sp
;	x = 4
;	y = 6
        les     bx,DWORD PTR _renderbuff ;video render page
	mov	ax,WORD PTR [bp+4]	;x
        mov     cl,03h
        and     cl,al
        shr     ax,1
        shr     ax,1
	add	bx,ax                   ;+x
        mov     dx,03C4h                ;plane enables
        mov     ax,0102h
        shl     ah,cl
        out     dx,ax
	mov	ax,WORD PTR [bp+6]      ;y
        mov     dx,ax                   ;y
        shl     ax,1
        shl     ax,1                    ;y*4
        add     ax,dx	                ;y*4+y
        mov     cl,4
        shl     ax,cl                   ;y*64+y*16=y*80
	add	bx,ax
	mov	al,BYTE PTR _idx8
	mov	BYTE PTR es:[bx],al
	pop	bp
	ret	
_pixel8	        ENDP

        PUBLIC	_pixel8argb
_pixel8argb	PROC NEAR
	push	bp
	mov	bp,sp
;	x     = 4
;	y     = 6
;       alpha = 8
	push	di
	push	si
        les     di,DWORD PTR _renderbuff ;video render page
	mov	bx,WORD PTR [bp+6]	;y
        mov     ax,bx
        shl     ax,1
        shl     ax,1                    ;y*4
        add     ax,bx	                ;y*4+y
        mov     cl,4
        shl     ax,cl                   ;y*64+y*16=y*80
        add     di,ax
	mov	ax,WORD PTR [bp+4]	;x
        mov     cl,03h
        and     cl,al
        shr     ax,1
        shr     ax,1
	add	di,ax                   ;+x
        mov     dx,03C4h                ;plane enables
        mov     ax,0102h
        shl     ah,cl
        out     dx,ax
        mov     si,WORD PTR [bp+8]      ;alpha
        cmp     si,00E0h
        jb      acmix
        mov     al,BYTE PTR _idx8
	mov	BYTE PTR es:[di],al
        jmp     SHORT acexit
acmix:  and     si,00E0h
        jz      acexit
        mov     dx,03CEh                ;read plane map
        mov     ah,cl
        mov     al,04h
        out     dx,ax
        shr     si,1
        shr     si,1
        mov     cl,5
	mov	dl,BYTE PTR es:[di]
	mov	bl,BYTE PTR _rgb8+2     ;red
        shr     bl,cl
	mov     al,BYTE PTR _amulr[bx][si]
        mov     bl,07h
        and     bl,dl
        xor     si,0038h
        add     al,BYTE PTR _amulr[bx][si]
	mov	bl,BYTE PTR _rgb8+1     ;grn
        shr     bl,cl
        xor     si,0038h
	or	al,BYTE PTR _amulg[bx][si]
        mov     bl,38h
        and     bl,dl
        shr     bl,1
        shr     bl,1
        shr     bl,1
        xor     si,0038h
        add     al,BYTE PTR _amulg[bx][si]
	mov	bl,BYTE PTR _rgb8+0     ;blu
        inc     cl
        shr     bl,cl
        shr     si,1
        xor     si,001Ch
	or	al,BYTE PTR _amulb[bx][si]
        mov     bl,dl
        shr     bl,cl
        xor     si,001Ch
        add     al,BYTE PTR _amulb[bx][si]
	mov	BYTE PTR es:[di],al
acexit:	pop	si
	pop	di
	pop	bp
	ret	
_pixel8argb	ENDP

        PUBLIC	_pixel8amono
_pixel8amono	PROC NEAR
	push	bp
	mov	bp,sp
;	x     = 4
;	y     = 6
;       alpha = 8
	push	di
        les     di,DWORD PTR _renderbuff ;video render page
	mov	bx,WORD PTR [bp+6]	;y
        mov     ax,bx
        shl     ax,1 
        shl     ax,1                    ;y*4
        add     ax,bx	                ;y*4+y
        mov     cl,4
        shl     ax,cl                   ;y*64+y*16=y*80
	add	di,ax                   ;+x
	mov	ax,WORD PTR [bp+4]	;x
        mov     cl,03h
        and     cl,al
        shr     ax,1
        shr     ax,1
        add     di,ax
        mov     dx,03C4h                ;plane enables
        mov     ax,0102h
        shl     ah,cl
        out     dx,ax
        mov     bl,BYTE PTR [bp+8]      ;alpha
        cmp     bl,0FCh
        jb      ammix
        mov     al,BYTE PTR _idx8
	mov	BYTE PTR es:[di],al
        jmp     SHORT amexit
ammix:  or      bl,bl
        jz      amexit
        mov     dx,03CEh                ;read plane map
        mov     ah,cl
        mov     al,04h
        out     dx,ax
        mov     al,BYTE PTR _idx8
        mul     bl
        mov     cl,ah
        not     bl
	mov	al,BYTE PTR es:[di]
        mul     bl
        add     ah,cl
	mov	BYTE PTR es:[di],ah
amexit:	pop	di
	pop	bp
	ret	
_pixel8amono	ENDP

_TEXT	ENDS
END

