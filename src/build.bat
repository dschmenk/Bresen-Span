masm /Mx fastline.asm;
masm /Mx pixline.asm;
masm /Mx aaline.asm;
masm /Mx pixspan8.asm;
cl /c /Ox rgb8.c
lib ..\bin\rgb8.lib -+rgb8.obj -+fastline.obj -+pixline.obj -+aaline.obj -+pixspan8.obj;
cl /Ox testline.c ..\bin\rgb8.lib pixline.obj /o ..\bin\testline.exe
cl /Ox filltri.c ..\bin\rgb8.lib /o ..\bin\filltri.exe

