;       Static Name Aliases
;
        TITLE   PIXSPAN2.ASM
        NAME    PIXSPAN2

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

EXTRN   _page_addr:NEAR
EXTRN   _renderbuff:NEAR
EXTRN   _scanaddr:NEAR
EXTRN   _brush2:NEAR
EXTRN   _idx2:NEAR
EXTRN   _amul2:NEAR

_TEXT      SEGMENT
	ASSUME	CS: _TEXT
	PUBLIC	_clear2
;void clear2(void)
_clear2         PROC NEAR
        push    di
        les     di,DWORD PTR _renderbuff ;video render page
        mov     cx,02000h
        mov     al,BYTE PTR _idx2       ;pattern
        mov     ah,al
        rep     stosw
        pop     di
        ret
_clear2         ENDP

        PUBLIC	_flip2
;void flip2(int sync)
_flip2          PROC NEAR
        push    bp
        mov     bp,sp
;	sync = 4
        push    di
        push    si
        push    ds
        les     di,DWORD PTR _page_addr   ;video front page
        lds     si,DWORD PTR _page_addr+4 ;video back  page
        mov     cx,02000h
        rep     movsw
        pop     ds
        pop     si
        pop     di
        pop     bp
        ret
_flip2          ENDP

        PUBLIC	_hspan2brush
;void hspan2brush(int xl, int xr, int y)
_hspan2brush	PROC NEAR
        push    bp
        mov     bp,sp
;	left = 4
;	right = 6
;	scan = 8
        push    di
        push    si
        les     di,DWORD PTR _renderbuff ;video render page
        mov     bx,WORD PTR [bp+8]      ;y
        mov     si,bx
        and     si,3
        shl     bx,1
        add     di,WORD PTR _scanaddr[bx] ;vbuf
        mov     bx,WORD PTR [bp+4]      ;xl
        xor     dx,dx
        not     dx
        mov     cl,bl                   ;xl
        and     cl,3
        shl     cl,1
        shr     dl,cl                   ;lmask
        mov     ax,WORD PTR [bp+6]      ;xr
        mov     cl,al                   ;xr
        not     cl
        and     cl,3
        shl     cl,1
        shl     dh,cl                   ;rmask
        shr     bx,1                    ;xl
        shr     ax,1                    ;xr
        shr     bx,1                    ;xl
        shr     ax,1                    ;xr
;
; Register assignments:
;       AX = xr >> 2
;       BX = xl >> 2
;       CX = span width in bytes - 1
;       DL = lmask
;       DH = rmask
;       ES:DI = vbuf
;       SI = y & 3
;
        mov     cx,ax
        sub     cx,bx
        jnz     lbrush
;
; Single byte span brush fill
;
        and     dl,dh
        mov     al,dl                   ;mask
        not     al
        and     al,BYTE PTR es:[di][bx] ;vbuf
        and     dl,BYTE PTR _brush2[si] ;pattern
        or      al,dl
        mov     BYTE PTR es:[di][bx],al
        pop     si
        pop     di
        pop     bp
        ret
;
; Left edge of span
;
lbrush: add     di,bx
        mov     al,dl                   ;lmask
        not     al
        and     al,BYTE PTR es:[di]     ;vbuf
        mov     ah,BYTE PTR _brush2[si] ;pattern
        and     dl,ah                   ;lmask
        or      al,dl
        cld
        stosb
        mov     al,ah
        dec     cx
        jz      rbrush
;
; Fill span center
;
        shr     cx,1
        jnc     wbrush
        stosb
wbrush: rep     stosw
;
; Right edge of span
;
rbrush: mov     ah,dh                   ;rmask
        not     ah
        and     ah,BYTE PTR es:[di]     ;vbuf
        and     al,dh                   ;rmask
        or      al,ah
        mov     BYTE PTR es:[di],al     ;vbuf
        pop     si
        pop     di
        pop     bp
        ret 
_hspan2brush    ENDP

	PUBLIC	_vspan2brush
;void vspan2brush(int x, int yt, int yb)
_vspan2brush	PROC NEAR
        push    bp
        mov     bp,sp
;	x = 4
;	yt = 6
;       yb = 8
        push    di
        push    si
        les     di,DWORD PTR _renderbuff ;video render page
        mov     ax,WORD PTR [bp+4]      ;x
        mov     ch,0C0h
        mov     cl,al                   ;x
        and     cl,3
        shl     cl,1
        shr     ch,cl                   ;mask
        shr     ax,1                    ;xl
        shr     ax,1                    ;xl
        add     di,ax
        mov     si,WORD PTR [bp+6]      ;yt
        mov     ax,WORD PTR [bp+8]      ;yb
        sub     ax,si
        mov     cl,al
        inc     cl
vbloop: mov     bx,si
        and     bx,0003h
        mov     ah,ch
        and     ah,BYTE PTR _brush2[bx] ;pattern
        shl     si,1
        mov     bx,WORD PTR _scanaddr[si] ;vbuf
        shr     si,1
        mov     al,ch                   ;mask
        not     al
        and     al,BYTE PTR es:[di][bx] ;vbuf
        or      al,ah
        mov     BYTE PTR es:[di][bx],al
        inc     si
        dec     cl
        jnz     vbloop
        pop     si
        pop     di
        pop     bp
        ret
_vspan2brush    ENDP

        PUBLIC	_pixel2brush
;void pixel2brush(int x, int y)
_pixel2brush	PROC NEAR
        push    bp
        mov     bp,sp
;       x = 4
;	y = 6
        push    di
        les     di,DWORD PTR _renderbuff ;video render page
        mov     bx,WORD PTR [bp+6]      ;y
        shl     bx,1
        add     di,WORD PTR _scanaddr[bx] ;vbuf
        shr     bx,1
        and     bx,3
        mov     ax,WORD PTR [bp+4]      ;x
        mov     dl,0C0h
        mov     cl,al                   ;x
        and     cl,3
        shl     cl,1
        shr     dl,cl                   ;mask
        shr     ax,1                    ;xl
        shr     ax,1                    ;xl
        add     di,ax
        mov     al,dl                   ;mask
        not     al
        and     al,BYTE PTR es:[di]     ;vbuf
        and     dl,BYTE PTR _brush2[bx] ;pattern
        or      al,dl
        mov     BYTE PTR es:[di],al
        pop     di
        pop     bp
        ret
_pixel2brush    ENDP

        PUBLIC	_hspan2
;void hspan2(int xl, int xr, int y)
_hspan2	        PROC NEAR
        push    bp
        mov     bp,sp
;	left = 4
;	right = 6
;	scan = 8
        push    di
        les     di,DWORD PTR _renderbuff ;video render page
        mov     bx,WORD PTR [bp+8]      ;y
        shl     bx,1
        add     di,WORD PTR _scanaddr[bx] ;vbuf
        mov     bx,WORD PTR [bp+4]      ;xl
        xor     dx,dx
        not     dx
        mov     cl,bl                   ;xl
        and     cl,3
        shl     cl,1
        shr     dl,cl                   ;lmask
        mov     ax,WORD PTR [bp+6]      ;xr
        mov     cl,al                   ;xr
        not     cl
        and     cl,3
        shl     cl,1
        shl     dh,cl                   ;rmask
        shr     bx,1                    ;xl
        shr     ax,1                    ;xr
        shr     bx,1                    ;xl
        shr     ax,1                    ;xr
;
; Register assignments:
;       AX = xr >> 2
;       BX = xl >> 2
;       CX = span width in bytes - 1
;       DL = lmask
;       DH = rmask
;       ES:DI = vbuf
;
        mov     cx,ax
        sub     cx,bx
        jnz     lsolid
;
; Single byte span solid fill
;
        and     dl,dh
        mov     al,dl                   ;mask
        not     al
        and     al,BYTE PTR es:[di][bx] ;vbuf
        and     dl,BYTE PTR _idx2       ;pattern
        or      al,dl
        mov     BYTE PTr es:[di][bx],al
        pop     di
        pop     bp
        ret
;
; Left edge of span
;
lsolid: add     di,bx
        mov     al,dl                   ;lmask
        not     al
        and     al,BYTE PTR es:[di]     ;vbuf
        mov     ah,BYTE PTR _idx2       ;pattern
        and     dl,ah                   ;lmask
        or      al,dl
        cld
        stosb
        mov     al,ah
        dec     cx
        jz      rsolid
;
; Fill span center
;
        shr     cx,1
        jnc     wsolid
        stosb
wsolid: rep     stosw
;
; Right edge of span
;
rsolid: mov     ah,dh                   ;rmask
        not     ah
        and     ah,BYTE PTR es:[di]     ;vbuf
        and     al,dh                   ;rmask
        or      al,ah
        mov     BYTE PTR es:[di],al     ;vbuf
        pop     di
        pop     bp
        ret 
_hspan2         ENDP

	PUBLIC	_vspan2
;void vspan2(int x, int yt, int yb)
_vspan2	        PROC NEAR
        push    bp
        mov     bp,sp
;	x = 4
;	yt = 6
;       yb = 8
        push    di
        push    si
        les     di,DWORD PTR _renderbuff ;video render page
        mov     ax,WORD PTR [bp+4]      ;x
        mov     ch,0C0h
        mov     cl,al                   ;x
        and     cl,3
        shl     cl,1
        shr     ch,cl                   ;mask
        shr     ax,1                    ;xl
        shr     ax,1                    ;xl
        add     di,ax
        mov     si,WORD PTR [bp+6]      ;yt
        mov     ax,WORD PTR [bp+8]      ;yb
        sub     ax,si
        mov     cl,al
        inc     cl
        shl     si,1
        mov     dl,BYTE PTR _idx2       ;pattern
vsloop: mov     bx,WORD PTR _scanaddr[si] ;vbuf
        mov     al,ch                   ;mask
        not     al
        and     al,BYTE PTR es:[di][bx] ;vbuf
        mov     ah,ch
        and     ah,dl                   ;pattern
        or      al,ah
        mov     BYTE PTR es:[di][bx],al
        add     si,2
        dec     cl
        jnz     vsloop
        pop     si
        pop     di
        pop     bp
        ret
_vspan2         ENDP

        PUBLIC	_pixel2
;void pixel2(int x, int y)
_pixel2	        PROC NEAR
        push    bp
        mov     bp,sp
;       x = 4
;	y = 6
        push    di
        les     di,DWORD PTR _renderbuff ;video render page
        mov     bx,WORD PTR [bp+6]      ;y
        shl     bx,1
        add     di,WORD PTR _scanaddr[bx] ;vbuf
        mov     bx,WORD PTR [bp+4]      ;x
        mov     dl,0C0h
        mov     cl,bl                   ;x
        and     cl,3
        shl     cl,1
        shr     dl,cl                   ;mask
        shr     bx,1                    ;xl
        shr     bx,1                    ;xl
        mov     al,dl                   ;mask
        not     al
        and     al,BYTE PTR es:[di][bx] ;vbuf
        and     dl,BYTE PTR _idx2       ;pattern
        or      al,dl
        mov     BYTE PTR es:[di][bx],al
        pop     di
        pop     bp
        ret
_pixel2         ENDP

        PUBLIC	_pixel2alpha
_pixel2alpha	PROC NEAR
        push    bp
        mov     bp,sp
;	x     = 4
;	y     = 6
;       alpha = 8
        mov     al,BYTE PTR [bp+8]      ;alpha
        and     ax,00C0h
        jz      aaexit
        push    di
        les     di,DWORD PTR _renderbuff ;video render page
        mov     bx,WORD PTR [bp+6]      ;y
        shl     bx,1
        add     di,WORD PTR _scanaddr[bx] ;vbuf
        mov     bx,ax
        mov     ax,WORD PTR [bp+4]      ;x
        mov     dl,0C0h
        mov     cl,al                   ;x
        and     cl,3
        shl     cl,1
        shr     dl,cl                   ;mask
        shr     ax,1                    ;xl
        shr     ax,1                    ;xl
        add     di,ax
        mov     al,BYTE PTR _idx2       ;pattern
        xor     cl,6
        shr     al,cl
        and     al,3
        mov     cl,4
        shr     bl,cl
        or      bl,al
        mov     al,BYTE PTR _amul2[bx]
        mov     ah,BYTE PTR es:[di]
        or      al,ah
        and     al,dl                   ;mask
        not     dl
        and     ah,dl                   ;~mask
        or      al,ah
        mov     BYTE PTR es:[di],al
        pop     di
aaexit: pop     bp
        ret
_pixel2alpha	ENDP

_TEXT	ENDS
END
