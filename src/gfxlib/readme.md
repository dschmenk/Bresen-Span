Here is an update to the x86 sample programs using the span-based and anti-alias line drawing routines. This is a work-in-progress for a graphics library I'm writing to support the VGA, EGA, and CGA adapters in a consistent way. I got tired of rewriting tests and demos for all my retro PCs with their different graphics cards. So this new library works by finding commonality between the graphics adapters and writing an RGB API to hide the color depth differences. I brought over the 16 color, 4BPP EGA dithering algorithm to supplement the 256 color, 8BPP VGA dithering algorithm used here previously. The CGA dithereing works similarly, but with even more constraints - I'll get to that shortly.

Finding commonality: The one graphics resolution in common with all three adapters that has the most color depth on each is 320x200. That is the only supported resolution for this library.  An attempt is made to map RGB colors to each graphics mode.

CGA: 320x200x2 maps RGB values to monochrome pixels. The reason is that the colors available on the CGA won't map to RGB values, so it is treated as a 4 level grey scale. On monochrome monitors such as the Compaq dual-mode monitor, the colors do map to a greyscale. When connected to an RGB monitor or composite video, the colors are combined to attempt a grey color.

EGA: 320x200x4 maps RGB to 16 IRGB colors. When connected to a monochrome monitor that maps colors to grey values, the driver will map the IRGB colors to the grey levels output by the monitor.

VGA: 320x200x8 maps RGB colors to a 3-3-2 RGB colormap. For monochrome, a 64 level grey ramp is programmed into the color registers.

The library will auto-detect the graphics adapter and select the mode with the most colors. The library has a few options to control the adapter selection and the color mapping mode. By default, the library wil dither the RGB colors to the best colors the adapter can disply. FOr CGA, this is monochrome ony. The EGA and VGA can output to either color or monochrome monitors. The dithering can be disabled to use a best-match color, greatly reducing color fidelity, but with a slight speed improvement for EGA and VGA. The library can also be forced to use a lesser color depth than what the hardware may support.

Page flipping is supported by allowing two pages in graphics memory to be rendered and displayed (except the CGA). The CGA will allocate a main memory buffer and do a copy to video memory during a page flip. Normally this is without issue unless you are expecting the contents of the previous front page to now be the contents of the back page. Look at LINETEST.EXE to see where this breaks down at the end of the program. [Update] CGA now does proper page flipping through the use of two shadow buffers. Thanks, Trixter!

Sample programs: These programs just demonstrate the abilities of the library. There are common command line options to control the libraries features.

    -m  : monochrome mode
    -n  : no dithering, use best match
    -d2 : use 2 BPP mode
    -d4 : use 4 BPP mode
    -d8 : use 8 BPP mode (default)
    
LINETEST.EXE:
![AALine](https://github.com/dschmenk/Bresen-Span/blob/master/images/aaline.png)

TRIFILL.EXE has additional flags to control filling and double buffering.

    -f0 : disable fill mode
    -f1 : enable fill mode (default)
    -a0 : disable anti-aliased outline
    -a1 : enable anti-aliased outline (default)
    -b  : buffer the rendering to back page and flip on VSYNC to display
    
SHOWPBM.EXE also takes a parameter, the PBM file to display. There are a couple .PNM images (24BPP RGB PBM images) included.

SPHERE.EXE displays different tesselations of the three Platonic Solids with triangluar faces. The solids are the Octahedron, Tetrahedron, and Icosahedron. The other two, the Cube and Dodecahedron, have square and hexagonal faces. The options are:

    -a0 : disable anti-aliased lines (default)
    -a1 : enable anti-aliased lines
    -h0 : disable hidden liness
    -h1 : enable hidden lines (default)
    -s<0..4> : tesselation level. 0 = no tesselation. One solid can't go to 4 :-)
    -p<0..2> : chose one of three objects to tesselate

SHOW3D.EXE display a 3D object defined in a file with the .ZOF extension and also has these additional flags:

    -f0 : disable fill mode
    -f1 : enable fill mode (default)
    -a0 : disable anti-aliased outline (default)
    -a1 : enable anti-aliased outline
    -h0 : disable hidden lines
    -h1 : enable hidden lines (default)

![NCC1701](https://github.com/dschmenk/Bresen-Span/blob/master/images/NCC1701.png)
