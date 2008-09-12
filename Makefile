# config
SKIN=skin.svg


sdl-cflags := $(shell sdl-config --cflags)
sdl-ldflags := $(shell sdl-config --libs)

ldflags := $(sdl-ldflags) -lSDL_image -lSDL_mixer $(LDFLAGS)
cflags := $(sdl-cflags) $(CFLAGS)



SKIN_PARTS= background next prev art save title artist star nostar trash bar slider pause

all: radio skin_coords.h

radio: main.c .sdl_flags
	gcc -o radio $(ldflags) $(cflags) main.c

.sdl_flags:
	sdl_config --libs --cflags > .sdl_flags || rm .sdl_flags

skin_coords.h: read_skin_coords.rb $(SKIN)
	ruby read_skin_coords.rb $(SKIN) $(SKIN_PARTS)
