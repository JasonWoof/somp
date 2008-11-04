# config
SKIN=skin.svg


sdl-cflags := $(shell sdl-config --cflags)
sdl-ldflags := $(shell sdl-config --libs)

ldflags := $(sdl-ldflags) -lSDL_image -lSDL_mixer -lSDL_ttf $(LDFLAGS)
cflags := $(sdl-cflags) $(CFLAGS) -Wall -DDEBUG



SKIN_PARTS= background next prev save title artist star nostar trash bar slider pause play next_over prev_over save_over pause_over play_over bubble_trash bubble_1 bubble_2 bubble_3 bubble_4 bubble_5

OBJECTS= somp skin_coords.h

all: $(OBJECTS) FreeSerif.ttf test_1.ogg test_2.ogg test_3.ogg test_4.ogg test_5.ogg

FreeSerif.ttf:
	wget 'http://ftp.gnu.org/gnu/freefont/freefont-ttf-20080912.tar.gz' -O - | tar -xzf - freefont-20080912/FreeSerif.ttf -O > FreeSerif.ttf || rm -f FreeSerif.ttf

test_1.ogg:
	wget 'http://upload.wikimedia.org/wikipedia/commons/e/e3/Gesualdo-moro_lasso_al_mio_duolo.ogg' -O test_1.ogg || rm -f test_1.ogg

test_2.ogg:
	wget 'http://upload.wikimedia.org/wikipedia/commons/0/09/I_Want_to_Go_Back_to_Michigan.ogg' -O test_2.ogg || rm -f test_2.ogg

# FIXME
test_3.ogg:
	ln -s test_1.ogg test_3.ogg
# FIXME
test_4.ogg:
	ln -s test_2.ogg test_4.ogg
# FIXME
test_5.ogg:
	ln -s test_2.ogg test_5.ogg

somp: main.c skin_coords.h
	gcc -o somp $(ldflags) $(cflags) main.c

skin_coords.h: read_skin_coords.rb $(SKIN)
	ruby read_skin_coords.rb $(SKIN) $(SKIN_PARTS)

SKIN_PNGS := $(SKIN_PARTS:%=skin/%.png)

data-clean:
	rm -f $(SKIN_PNGS)

clean:
	rm -f $(OBJECTS)

distclean: clean data-clean
