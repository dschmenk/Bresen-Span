;       Static Name Aliases
;
        TITLE   PIXSPAN4.ASM
        NAME    PIXSPAN4

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
EXTRN   _pixmask:NEAR
EXTRN   _brush4:NEAR
EXTRN   _idx4:NEAR
EXTRN   _packbrush4:NEAR
EXTRN   _amul4:NEAR

_TEXT      SEGMENT
	ASSUME	CS: _TEXT

        PUBLIC	_clear4
;void clear4(void)
_clear4         PROC NEAR
        push    di
        mov     dx,03C4h                ;plane enables
        mov     ax,0F02h
        out     dx,ax
        mov     dx,03CEh                ;write mode 2
        mov     ax,0205h
        out     dx,ax
        mov     ax,0FF08h               ;pixmask
        out     dx,ax
        les     di,DWORD PTR _renderbuff ;video render page
        mov     cx, 40*200/2
        mov     al,BYTE PTR _idx4       ;pattern
        mov     ah,al
        rep     stosw
        pop     di
        ret
_clear4         ENDP

	PUBLIC	_hspan4brush
;void hspan4brush(int xl, int xr, int y)
_hspan4brush	PROC NEAR
        push    bp
        mov     bp,sp
;	left = 4
;	right = 6
;	scan = 8
        push    di
        push    si
        les     di,DWORD PTR _renderbuff ;video render page
        mov     si,WORD PTR [bp+8]      ;y
        mov     ax,si                   ;y
        shl     ax,1                    ;(y*4+y)*8=y*32+y*8=y*40
        shl     ax,1
        add     ax,si
        shl     ax,1
        shl     ax,1
        shl     ax,1
        add     di,ax
        and     si,0003h
        mov     cx,WORD PTR [bp+4]      ;xl
        mov     ax,cx
        shr     ax,1
        shr     ax,1
        shr     ax,1
	add	di,ax                   ;xl>>3
        mov     bx,WORD PTR [bp+6]      ;xr
        cmp     bx,cx
        jne     hbwide
;
; Single pixel case.
;
        mov     dx,03C4h                ;plane enables
        mov     ax,0F02h
        out     dx,ax
        mov     dx,03CEh                ;write mode 2
        mov     ax,0205h
        out     dx,ax
        and     cx,0007h
        mov     bx,cx
        shl     bx,1
        mov     ax,WORD PTR _pixmask[bx]
        out     dx,ax
        mov     dl,080h
        shr     dl,cl                   ;mask
        shl     si,1
        shl     si,1
        and     cl,03h
        add     si,cx
        mov     al,BYTE PTR _packbrush4[si]
	mov	ah,BYTE PTR es:[di]    ;latches
	mov	es:[di],al
        pop     si
        pop     di
        pop     bp
        ret
hbwide: xor     dx,dx
        not     dx
        and     cl,07h                  ;xl
        shr     dl,cl                   ;lmask
        mov     cl,bl                   ;xr
        not     cl
        and     cl,07h
        shl     dh,cl                   ;rmask
        shr     bx,1
        shr     bx,1
        shr     bx,1                    ;xr>>3
;
; Register assignments:
;       BX = span byte width
;       CL = lmask
;       CH = rmask
;       ES:DI = vbuf
;       SI = y & 3
;
        mov     cx,dx
        sub     bx,ax                   ;xl>>3
        jz      hbbyte
        jmp     hbspan
;
; Single byte span fill
;
hbbyte: and     cl,ch
        mov     dx,03CEh                ;write mode 0
        mov     ax,0005h
        out     dx,ax
        mov     ax,0FF08h               ;pixmask
        out     dx,ax
; Plane 0
;       mov     dx,03CEh                ;read plane map
        mov     ax,0004h
        out     dx,ax
        mov     dx,03C4h                ;plane enables
        mov     ax,0102h
        out     dx,ax
        mov     al,cl                   ;mask
        not     al
        and     al,BYTE PTR es:[di]     ;vbuf
        mov     ah,cl
        and     ah,BYTE PTR _brush4+0[si] ;pattern
        or      al,ah
        mov     BYTE PTR es:[di],al
; Plane 1
;       mov     dx,03C4h                ;plane enables
;       mov     ax,0202h
        inc     dx                      ;plane enables
        mov     al,02h
        out     dx,al
;       mov     dx,03CEh                ;read plane map
;       mov     ax,0104h
        mov     dx,03CFh                ;read plane map
        mov     al,01h
        out     dx,al
        mov     al,cl                   ;mask
        not     al
        and     al,BYTE PTR es:[di]     ;vbuf
        mov     ah,cl
        and     ah,BYTE PTR _brush4+4[si] ;pattern
        or      al,ah
        mov     BYTE PTR es:[di],al
; Plane 2
;       mov     dx,03CEh                ;read plane map
;       mov     ax,0204h
        mov     al,02h
        out     dx,al
;       mov     dx,03C4h                ;plane enables
;       mov     ax,0402h
        mov     dx,03C5h                ;plane enables
        mov     al,04h
        out     dx,al
        mov     al,cl                   ;mask
        not     al
        and     al,BYTE PTR es:[di]     ;vbuf
        mov     ah,cl
        and     ah,BYTE PTR _brush4+8[si] ;pattern
        or      al,ah
        mov     BYTE PTR es:[di],al
; Plane 3
;       mov     dx,03C4h                ;plane enables
;       mov     ax,0802h
        mov     al,08h
        out     dx,al
;       mov     dx,03CEh                ;read plane map
;       mov     ax,0304h
        mov     dx,03CFh                ;read plane map
        mov     al,03h
        out     dx,al
        mov     al,cl                   ;mask
        not     al
        and     al,BYTE PTR es:[di]     ;vbuf
        and     cl,BYTE PTR _brush4+12[si] ;pattern
        or      al,cl
        mov     BYTE PTR es:[di],al
        pop     si
        pop     di
        pop     bp
        ret
;
; Multi-pixel span.
;
hbspan: mov     dx,03CEh                ;write mode 0
        mov     ax,0005h
        out     dx,ax
        mov     ax,0FF08h               ;pixmask
        out     dx,ax
; Plane 0
;       mov     dx,03CEh                ;read plane map
        mov     ax,0004h
        out     dx,ax
        mov     dx,03C4h                ;plane enables
        mov     ax,0102h
        out     dx,ax
        mov     al,cl                   ;lmask
        not     al
        and     al,BYTE PTR es:[di]     ;vbuf
        mov     ah,cl
        and     ah,BYTE PTR _brush4+0[si] ;pattern
        or      al,ah
        mov     BYTE PTR es:[di],al
        mov     al,ch                   ;rmask
        not     al
        and     al,BYTE PTR es:[di][bx] ;vbuf
        mov     ah,ch
        and     ah,BYTE PTR _brush4+0[si] ;pattern
        or      al,ah
        mov     BYTE PTR es:[di][bx],al
; Plane 1
;       mov     dx,03C4h                ;plane enables
;       mov     ax,0202h
        inc     dx
        mov     al,02h
        out     dx,al
;       mov     dx,03CEh                ;read plane map
;       mov     ax,0104h
        mov     dx,03CFh                ;read plane map
        mov     ax,01h
        out     dx,al
        mov     al,cl                   ;lmask
        not     al
        and     al,BYTE PTR es:[di]     ;vbuf
        mov     ah,cl
        and     ah,BYTE PTR _brush4+4[si] ;pattern
        or      al,ah
        mov     BYTE PTR es:[di],al
        mov     al,ch                   ;rmask
        not     al
        and     al,BYTE PTR es:[di][bx] ;vbuf
        mov     ah,ch
        and     ah,BYTE PTR _brush4+4[si] ;pattern
        or      al,ah
        mov     BYTE PTR es:[di][bx],al
; Plane 2
;       mov     dx,03CEh                ;read plane map
;       mov     ax,0204h
        mov     al,02h
        out     dx,al
;       mov     dx,03C4h                ;plane enables
;       mov     ax,0402h
        mov     dx,03C5h                ;plane enables
        mov     ax,04h
        out     dx,al
        mov     al,cl                   ;lmask
        not     al
        and     al,BYTE PTR es:[di]     ;vbuf
        mov     ah,cl
        and     ah,BYTE PTR _brush4+8[si] ;pattern
        or      al,ah
        mov     BYTE PTR es:[di],al
        mov     al,ch                   ;rmask
        not     al
        and     al,BYTE PTR es:[di][bx] ;vbuf
        mov     ah,ch
        and     ah,BYTE PTR _brush4+8[si] ;pattern
        or      al,ah
        mov     BYTE PTR es:[di][bx],al
; Plane 3
;       mov     dx,03C4h                ;plane enables
;       mov     ax,0802h
        mov     al,08h
        out     dx,al
;       mov     dx,03CEh                ;read plane map
;       mov     ax,0304h
        mov     dx,03CFh                ;read plane map
        mov     ax,03h
        out     dx,al
        mov     al,cl                   ;lmask
        not     al
        and     al,BYTE PTR es:[di]     ;vbuf
        and     cl,BYTE PTR _brush4+12[si] ;pattern
        or      al,cl
        mov     BYTE PTR es:[di],al
        mov     al,ch                   ;rmask
        not     al
        and     al,BYTE PTR es:[di][bx] ;vbuf
        and     ch,BYTE PTR _brush4+12[si] ;pattern
        or      al,ch
        mov     BYTE PTR es:[di][bx],al
;
; Fill span center
;
        dec     bx
        jz      hbexit
        mov     cx,bx
        inc     di
;       mov     dx,03CEh                ;write mode 1
        dec     dx
        mov     ax,0105h
        out     dx,ax
;       mov     dx,03C4h                ;plane enables
;       mov     ax,0F02h
        mov     dx,03C5h                ;plane enables
        mov     ax,0Fh
        out     dx,al
        mov     al,BYTE PTR es:[si+40*200] ;load latches
        cld
        shr     cx,1
        jnc     hbfill
        stosb
hbfill: rep     stosw
hbexit: pop     si
        pop     di
        pop     bp
        ret 
_hspan4brush    ENDP

	PUBLIC	_vspan4brush
;void vspan4brush(int x, int yt, int yb)
_vspan4brush	PROC NEAR
        push    bp
        mov     bp,sp
;	x = 4
;	yt = 6
;       yb = 8
        push    di
        push    si
        les     di,DWORD PTR _renderbuff ;video render page
        mov     dx,03C4h                ;plane enables
        mov     ax,0F02h
        out     dx,ax
        mov     dx,03CEh                ;write mode 2
        mov     ax,0205h
        out     dx,ax
	mov	cx,WORD PTR [bp+4]	;x
        mov     bx,0007h
        and     bx,cx
        shl     bx,1
        mov     ax,WORD PTR _pixmask[bx]
        out     dx,ax
        mov     bx,WORD PTR [bp+6]      ;yt
        mov     ax,bx                   ;yt
        shl     ax,1                    ;(yt*4+yt)*8=yt*32+yt*8=yt*40
        shl     ax,1
        add     ax,bx
        shl     ax,1
        shl     ax,1
        shl     ax,1
        add     di,ax
        mov     ax,cx
        shr     ax,1
        shr     ax,1
        shr     ax,1
	add	di,ax                   ;x>>3
        mov     si,0003h
        and     si,cx                   ;x
        add     si,OFFSET _packbrush4
        mov     cx,WORD PTR [bp+8]      ;yb
        sub     cx,bx                   ;yt
        inc     cx
        and     bl,03h
        shl     bl,1
        shl     bl,1
vloop:  mov     al,BYTE PTR [si][bx]
	mov	ah,BYTE PTR es:[di]     ;latches
	mov	BYTE PTR es:[di],al
        add     di,40
        add     bl,4
        and     bl,0Ch
        loop    vloop
        pop     si
        pop     di
        pop     bp
        ret
_vspan4brush    ENDP

        PUBLIC	_pixel4brush
;void pixel4brush(int x, int y)
_pixel4brush	PROC NEAR
        push    bp
        mov     bp,sp
;       x = 4
;	y = 6
        push    di
        les     di,DWORD PTR _renderbuff ;video render page
        mov     dx,03C4h                ;plane enables
        mov     ax,0F02h
        out     dx,ax
        mov     dx,03CEh                ;write mode 2
        mov     ax,0205h
        out     dx,ax
	mov	cx,WORD PTR [bp+4]	;x
        mov     bx,0007h
        and     bx,cx
        shl     bx,1
        mov     ax,WORD PTR _pixmask[bx]
        out     dx,ax
        mov     bx,WORD PTR [bp+6]      ;y
        mov     ax,bx                   ;y
        shl     ax,1                    ;(y*4+y)*8=y*32+y*8=y*40
        shl     ax,1
        add     ax,bx
        shl     ax,1
        shl     ax,1
        shl     ax,1
        add     di,ax
        mov     dx,cx
        shr     dx,1
        shr     dx,1
        shr     dx,1
	add	di,dx                   ;x
        and     bl,03h
        shl     bx,1
        shl     bx,1
        and     cl,03h
        add     bl,cl
        mov     al,BYTE PTR _packbrush4[bx]
	mov	ah,BYTE PTR es:[di]     ;latches
	mov	BYTE PTR es:[di],al
        pop     di
        pop     bp
        ret
_pixel4brush    ENDP

	PUBLIC	_hspan4
;void hspan4(int xl, int xr, int y)
_hspan4	        PROC NEAR
        push    bp
        mov     bp,sp
;	left = 4
;	right = 6
;	scan = 8
        push    di
        les     di,DWORD PTR _renderbuff ;video render page
        mov     bx,WORD PTR [bp+8]      ;y
        mov     ax,bx                   ;y
        shl     ax,1                    ;(y*4+y)*8=y*32+y*8=y*40
        shl     ax,1
        add     ax,bx
        shl     ax,1
        shl     ax,1
        shl     ax,1
        add     di,ax
        mov     cx,WORD PTR [bp+4]      ;xl
        mov     ax,cx
        shr     ax,1
        shr     ax,1
        shr     ax,1
	add	di,ax                   ;xl>>3
        mov     bx,WORD PTR [bp+6]      ;xr
        xor     dx,dx
        not     dx
        and     cl,07h                  ;xl
        shr     dl,cl                   ;lmask
        mov     cl,bl                   ;xr
        not     cl
        and     cl,07h
        shl     dh,cl                   ;rmask
        shr     bx,1
        shr     bx,1
        shr     bx,1                    ;xr>>3
;
; Register assignments:
;       BX = span byte width
;       CL = lmask
;       CH = rmask
;       ES:DI = vbuf
;
        mov     cx,dx
        sub     bx,ax                   ;xl>>3
        mov     dx,03C4h                ;plane enables
        mov     ax,0F02h
        out     dx,ax
        mov     dx,03CEh                ;write mode 2
        mov     ax,0205h
        out     dx,ax
        mov     al,08h                  ;pixmask index
        mov     ah,cl                   ;lmask
        jnz     hswide
;
; Single byte span fill
;
        and     ah,ch
        out     dx,ax                   ;mask
        mov     al,BYTE PTR _idx4
	mov	ah,BYTE PTR es:[di]     ;latches
	mov	BYTE PTR es:[di],al
        pop     di
        pop     bp
        ret
;
; Wide span.
;
hswide: out     dx,ax                   ;lmask
        mov     cl,BYTE PTR _idx4
	mov	ah,BYTE PTR es:[di]     ;latches
	mov	BYTE PTR es:[di],cl
        mov     al,ch                   ;rmask
        inc     dx
        out     dx,al
        mov     ah,BYTE PTR es:[di][bx] ;latches
        mov     BYTE PTR es:[di][bx],cl
;
; Fill span center
;
        dec     bx
        jz      hsexit
        mov     al,0FFh                 ;clear pixmask
        out     dx,al
        mov     al,cl                   ;_idx4
        mov     ah,al
        mov     cx,bx
        inc     di
        cld
        shr     cx,1
        jnc     hsfill
        stosb
hsfill: rep     stosw
hsexit: pop     di
        pop     bp
        ret 
_hspan4         ENDP

	PUBLIC	_vspan4
;void vspan4(int x, int yt, int yb)
_vspan4	        PROC NEAR
        push    bp
        mov     bp,sp
;	x = 4
;	yt = 6
;       yb = 8
        push    di
        les     di,DWORD PTR _renderbuff ;video render page
        mov     dx,03C4h                ;plane enables
        mov     ax,0F02h
        out     dx,ax
        mov     dx,03CEh                ;write mode 2
        mov     ax,0205h
        out     dx,ax
	mov	cx,WORD PTR [bp+4]	;x
        mov     bx,0007h
        and     bx,cx
        shl     bx,1
        mov     ax,WORD PTR _pixmask[bx] ;pixmask
        out     dx,ax
        mov     bx,WORD PTR [bp+6]      ;yt
        mov     ax,bx                   ;yt
        shl     ax,1                    ;(yt*4+yt)*8=yt*32+yt*8=yt*40
        shl     ax,1
        add     ax,bx
        shl     ax,1
        shl     ax,1
        shl     ax,1
        add     di,ax
        shr     cx,1
        shr     cx,1
        shr     cx,1
	add	di,cx                   ;x>>3
        mov     cx,WORD PTR [bp+8]      ;yb
        sub     cx,bx                   ;yt
        inc     cx
        mov     al,BYTE PTR _idx4
vsloop:	mov	ah,BYTE PTR es:[di]     ;latches
	mov	BYTE PTR es:[di],al
        add     di,40
        loop    vsloop
        pop     di
        pop     bp
        ret
_vspan4         ENDP

        PUBLIC	_pixel4
;void pixel4(int x, int y)
_pixel4	        PROC NEAR
        push    bp
        mov     bp,sp
;       x = 4
;	y = 6
        push    di
        les     di,DWORD PTR _renderbuff ;video render page
        mov     dx,03C4h                ;plane enables
        mov     ax,0F02h
        out     dx,ax
        mov     dx,03CEh                ;write mode 2
        mov     ax,0205h
        out     dx,ax
	mov	cx,WORD PTR [bp+4]	;x
        mov     bx,0007h
        and     bx,cx
        shl     bx,1
        mov     ax,WORD PTR _pixmask[bx]
        out     dx,ax
        mov     bx,WORD PTR [bp+6]      ;y
        mov     ax,bx                   ;y
        shl     ax,1                    ;(y*4+y)*8=y*32+y*8=y*40
        shl     ax,1
        add     ax,bx
        shl     ax,1
        shl     ax,1
        shl     ax,1
        add     di,ax
        shr     cx,1
        shr     cx,1
        shr     cx,1
	add	di,cx                   ;x
        mov     al,BYTE PTR _idx4
	mov	ah,BYTE PTR es:[di]     ;latches
	mov	BYTE PTR es:[di],al
        pop     di
        pop     bp
        ret
_pixel4         ENDP

        PUBLIC	_pixel4alpha
_pixel4alpha	PROC NEAR
	push	bp
	mov	bp,sp
;	x     = 4
;	y     = 6
;       alpha = 8
	push	di
        les     di,DWORD PTR _renderbuff ;video render page
        mov     dx,03C4h                ;plane enables
        mov     ax,0F02h
        out     dx,ax
        mov     dx,03CEh                ;write mode 2
        mov     ax,0205h
        out     dx,ax
	mov	cx,WORD PTR [bp+4]	;x
        mov     bx,0007h
        and     bx,cx
        shr     cx,1
        shr     cx,1
        shr     cx,1
	add	di,cx                   ;x
        shl     bx,1
        mov     ax,WORD PTR _pixmask[bx]
        out     dx,ax
        mov     bx,00C0h
        and     bx,WORD PTR [bp+8]      ;alpha
        jz      aaexit
        mov     cx,WORD PTR [bp+6]      ;y
        mov     ax,cx                   ;y
        shl     ax,1                    ;(y*4+y)*8=y*32+y*8=y*40
        shl     ax,1
        add     ax,cx
        shl     ax,1
        shl     ax,1
        shl     ax,1
        add     di,ax
        cmp     bl,0C0h
        jb      aamix
	mov	al,BYTE PTR es:[di]     ;latches
        mov     al,BYTE PTR _idx4
	mov	BYTE PTR es:[di],al
        jmp     SHORT aaexit
aamix:  mov     ax,0203h                ;src OR dst pixel data
        out     dx,ax
	mov	al,BYTE PTR es:[di]     ;latches
        shr     bl,1
        shr     bl,1
        or      bl,BYTE PTR _idx4
        mov     al,BYTE PTR _amul4[bx]
	mov	BYTE PTR es:[di],al
        mov     al,00h                  ;src pixel data
        inc     dx
        out     dx,al

aaexit:	pop	di
	pop	bp
	ret	
_pixel4alpha	ENDP

_TEXT	ENDS
END
