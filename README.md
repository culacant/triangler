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

## TODO
0) optimizations
1) pathfinding and AI
2) guns guns gun
3) fix backface culling
4) homebrew memory allocator

## TODO longterm
- color palette/16-bit color
- extra modules:
	- audio (OSS not alsa or pulse because fuck that noise hee hee)
	- homemade memory allocation
	- multithreading
