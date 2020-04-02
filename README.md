# triangler

Software renderer using libc and linux API.

## Features:
- IQE model loading
- TGA texture loading
- simple text rendering
- matrix/vector math
- mouse/keyboard input via /dev/input*
- simple collision detection + response

## TODO
1) fix backface culling
3) homebrew memory allocator

## TODO longterm
- android (kitkat on Tegra 2 ventana)
- color palette/16-bit color
- extra modules:
	- audio (OSS not alsa or pulse because fuck that noise hee hee)
	- physics/collision detection+response
	- homemade memory allocation
