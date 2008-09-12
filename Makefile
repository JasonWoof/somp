# config
SKIN=skin.svg


SKIN_PARTS= background next prev art save title artist star nostar trash bar slider pause

all: skin_coords.h

skin_coords.h: read_skin_coords.rb $(SKIN)
	ruby read_skin_coords.rb $(SKIN) $(SKIN_PARTS)
