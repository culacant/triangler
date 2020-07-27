# triangler

game engine using libc and linux API.

## Features:
- mouse/keyboard input via /dev/input*
- drawing via /dev/fb*
- IQE model loading
- TGA texture loading
- simple text rendering
- matrix/vector math
- simple collision detection + response
- homebrew memory allocator

## TODO
0) optimizations
1) guns guns gun
2) pathfinding and AI
3) fix backface culling
4) put everything into memory allocator
5) figure out how i made call graphs

## TODO longterm
- color palette/16-bit color
- extra modules:
	- audio (OSS not alsa or pulse because fuck that noise hee hee)
	- multithreading (once game loop takes ~30-50ms)
