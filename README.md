# triangler

Software renderer using libc and linux API.

## Features:
- IQE model loading
- TGA texture loading
- simple text rendering
- matrix/vector math
- mouse/keyboard input via /dev/input*

## TODO
1) clipping without breaking everything
2) sanitize camera movement
3) android (kitkat on Tegra 2 ventana)

## TODO longterm
- color palette/16-bit color
- extra modules:
	- audio (OSS not alsa or pulse because fuck that noise hee hee)
	- physics/collision detection+response
	- homemade memory allocation
