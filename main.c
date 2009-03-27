/* somp copyright (C) 2008 Jason Woofenden
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdlib.h>
#include <stdio.h>

///////////////////////  SETTINGS ///////////////////////
//
// You can change these settings by changing your CFLAGS. example:
//
// $ export CFLAGS="$CFLAGS -DSMALL_SEEK=3"
//
// number of seconds to seek for left/right keys and holding prev/next buttons
#ifndef SMALL_SEEK
#define SMALL_SEEK 5
#endif

// miliseconds between seeks when holding mouse button on prev/next buttons
#ifndef SEEK_INTERVAL
#define SEEK_INTERVAL 100
#endif

// path to directory containing skin images
#ifndef SKIN_PREFIX
#define SKIN_PREFIX "skin/"
#endif

// number of milliseconds to stall while waiting for events on a timeout
#ifndef EVENT_LAG
#define EVENT_LAG 50
#endif

// while playing, slider is updated every this many milliseconds
#ifndef SLIDER_LAG
#define SLIDER_LAG 700
#endif

// if the mouse button is held down longer than this then it's not considered a click
#ifndef CLICK_THRESHOLD
#define CLICK_THRESHOLD 200
#endif





#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#include "skin_coords.h"
#define SKIN_SLIDER_SPAN (SKIN_BAR_WIDTH - 2 * (SKIN_SLIDER_LEFT - SKIN_BAR_LEFT) - SKIN_SLIDER_WIDTH)
#define SKIN_SLIDER_0_X (SKIN_SLIDER_LEFT + (SKIN_SLIDER_WIDTH / 2))
#define SKIN_SLIDER_100_X (SKIN_SLIDER_0_X + SKIN_SLIDER_SPAN)

#define SKIN_STAR_SPACING (SKIN_NOSTAR_LEFT - SKIN_STAR_LEFT)
#define SKIN_STAR_1_LEFT (SKIN_STAR_LEFT - (3 * SKIN_STAR_SPACING))
#define SKIN_STAR_2_LEFT (SKIN_STAR_LEFT - (2 * SKIN_STAR_SPACING))
#define SKIN_STAR_3_LEFT (SKIN_STAR_LEFT - SKIN_STAR_SPACING)
#define SKIN_STAR_4_LEFT (SKIN_STAR_LEFT)
#define SKIN_STAR_5_LEFT (SKIN_STAR_LEFT + SKIN_STAR_SPACING)

#define SKIN_STAR_1_HEIGHT (SKIN_STAR_HEIGHT)
#define SKIN_STAR_2_HEIGHT (SKIN_STAR_HEIGHT)
#define SKIN_STAR_3_HEIGHT (SKIN_STAR_HEIGHT)
#define SKIN_STAR_4_HEIGHT (SKIN_STAR_HEIGHT)
#define SKIN_STAR_5_HEIGHT (SKIN_STAR_HEIGHT)
#define SKIN_STAR_1_WIDTH (SKIN_STAR_WIDTH)
#define SKIN_STAR_2_WIDTH (SKIN_STAR_WIDTH)
#define SKIN_STAR_3_WIDTH (SKIN_STAR_WIDTH)
#define SKIN_STAR_4_WIDTH (SKIN_STAR_WIDTH)
#define SKIN_STAR_5_WIDTH (SKIN_STAR_WIDTH)
#define SKIN_STAR_1_TOP (SKIN_STAR_TOP)
#define SKIN_STAR_2_TOP (SKIN_STAR_TOP)
#define SKIN_STAR_3_TOP (SKIN_STAR_TOP)
#define SKIN_STAR_4_TOP (SKIN_STAR_TOP)
#define SKIN_STAR_5_TOP (SKIN_STAR_TOP)

#define SKIN_BUBBLE_LEFT_WIDTH (SKIN_BUBBLE_MIDDLE_LEFT - SKIN_BUBBLE_LEFT)
#define SKIN_BUBBLE_RIGHT_WIDTH (SKIN_BUBBLE_WIDTH - SKIN_BUBBLE_LEFT_WIDTH - SKIN_BUBBLE_MIDDLE_WIDTH)
#define SKIN_BUBBLE_TEXT_INDENT_X (SKIN_BUBBLE_TEXT_LEFT - SKIN_BUBBLE_LEFT)
#define SKIN_BUBBLE_TEXT_INDENT_Y (SKIN_BUBBLE_TEXT_TOP - SKIN_BUBBLE_TOP)

// offset of bubble relative (middle,top) of the thing it's pointing at
#define SKIN_BUBBLE_REL_X (SKIN_BUBBLE_LEFT - (SKIN_NOSTAR_LEFT + (SKIN_NOSTAR_WIDTH / 2)))
#define SKIN_BUBBLE_REL_Y (SKIN_BUBBLE_TOP - (SKIN_NOSTAR_TOP))

#define SKIN_SAVE_LEFT_WIDTH (SKIN_SAVE_MIDDLE_LEFT - SKIN_SAVE_LEFT)
#define SKIN_SAVE_RIGHT_WIDTH (SKIN_SAVE_WIDTH - SKIN_SAVE_LEFT_WIDTH - SKIN_SAVE_MIDDLE_WIDTH)
#define SKIN_SAVE_TEXT_INDENT_X (SKIN_SAVE_TEXT_LEFT - SKIN_SAVE_LEFT)
#define SKIN_SAVE_TEXT_INDENT_Y (SKIN_SAVE_TEXT_TOP - SKIN_SAVE_TOP)

#define SKIN_SAVE_OVER_LEFT_WIDTH (SKIN_SAVE_OVER_MIDDLE_LEFT - SKIN_SAVE_OVER_LEFT)
#define SKIN_SAVE_OVER_RIGHT_WIDTH (SKIN_SAVE_OVER_WIDTH - SKIN_SAVE_OVER_LEFT_WIDTH - SKIN_SAVE_OVER_MIDDLE_WIDTH)
#define SKIN_SAVE_OVER_TEXT_INDENT_X (SKIN_SAVE_OVER_TEXT_LEFT - SKIN_SAVE_OVER_LEFT)
#define SKIN_SAVE_OVER_TEXT_INDENT_Y (SKIN_SAVE_OVER_TEXT_TOP - SKIN_SAVE_OVER_TOP)

#define STATE_STARTING 0
#define STATE_PLAYING 1
#define STATE_PAUSED 2
#define STATE_STOPPED 3

#define OVER_PREV 1
#define OVER_PLAY 2
#define OVER_PAUSE 3
#define OVER_NEXT 4
#define OVER_SAVE 5
#define OVER_BAR 6
#define OVER_TRASH 7
#define OVER_STAR_1 8
#define OVER_STAR_2 9
#define OVER_STAR_3 10
#define OVER_STAR_4 11
#define OVER_STAR_5 12
#define OVER_SLIDER 13

#define DRAGGING_PREV OVER_PREV
#define DRAGGING_NEXT OVER_NEXT
#define DRAGGING_SLIDER OVER_SLIDER

#define EVENT_MUSIC_FINISHED 1

// FIXME use gettext()
#define TRANSLATE(a) (a)


// GLOBALS
TTF_Font *g_artist_font;
Mix_Music *g_music = 0;
// Playlist* g_playlist defined later
int g_mouse_over = 0;
int g_mouse_x, g_mouse_y;
int g_dirty = 1;
int g_dragging = 0;
uint32_t g_seek_at = 0;
uint32_t g_dragging_start = 0;
int g_state = STATE_STOPPED;
SDL_Surface *surf_screen;
SDL_Surface *i_background, *i_next, *i_prev, *i_save, *i_star, *i_nostar, *i_trash, *i_bar, *i_slider, *i_pause, *i_play, *i_next_over, *i_prev_over, *i_save_over, *i_pause_over, *i_play_over, *i_bubble;
SDL_Surface *i_txt_bubble_trash, *i_txt_bubble_1, *i_txt_bubble_2, *i_txt_bubble_3, *i_txt_bubble_4, *i_txt_bubble_5, *i_txt_save;
int g_skin_save_left, g_skin_save_width;

// wait up to ms for an event. return TRUE if an event was received
uint8_t
wait_event_with_timeout(SDL_Event *e, uint32_t ms) {
	while(ms) {
		if(SDL_PollEvent(e)) {
			return 1;
		}
		SDL_Delay(MIN(ms, EVENT_LAG));
		ms -= MIN(ms, EVENT_LAG);
	}
	return SDL_PollEvent(e);
}


void set_state(int state);
int get_state();
void text_draw();

void
music_finished() {
	SDL_Event e;
#ifdef DEBUG
	fprintf(stderr, "Reached end of track.\n");
#endif

	// Get the main loop back into action by telling it that the user hit the "next song" button
	e.type = SDL_USEREVENT;
	e.user.type = SDL_USEREVENT;
	e.user.code = EVENT_MUSIC_FINISHED;
	if(SDL_PushEvent(&e)) {
		fprintf(stderr, "SDL_PushEvent() failed.\n");
	}
}


typedef struct {
	char *meta;
	char *filename, *id, *title, *artist;
	int duration, rating;
	SDL_Surface *artist_tex, *title_tex; // used by text rendering
	Mix_Music *audio;
} TrackInfo;

// Pass a pointer that should be free()ed when this TrackInfo is deleted
// Make sure there are at least 5 nulls in it
TrackInfo*
track_new(char *m) {
	TrackInfo *t;
	t = (TrackInfo*) malloc(sizeof(TrackInfo));
	if(!t) {
		fprintf(stderr, "Failed to allocate memory for a TrackInfo object");
		exit(9);
	}
	t->meta = m;
	t->filename = m;   m += strlen(m) + 1;
	t->id = m;         m += strlen(m) + 1;
	t->title = m;      m += strlen(m) + 1;
	t->artist = m;     m += strlen(m) + 1;
	t->duration = atoi(m);
	t->rating = 0;

#ifdef DEBUG
	fprintf(stderr, "filename: %s, id: %s, title: %s, artist: %s, duration: %i\n", t->filename, t->id, t->title, t->artist, t->duration);
#endif

	t->artist_tex = 0;
	t->title_tex = 0;
	t->audio = 0;

	return t;
}

// delete what can be re-created
void
track_cleanup(TrackInfo *t) {
	if(t->artist_tex) {
		SDL_FreeSurface(t->artist_tex);
		t->artist_tex = 0;
	}

	if(t->title_tex) {
		SDL_FreeSurface(t->title_tex);
		t->title_tex = 0;
	}

	if(t->audio) {
		Mix_FreeMusic(t->audio);
		t->audio = 0;
	}
}

void
track_delete(TrackInfo *t) {
	track_cleanup(t);
	free(t->meta);
	free(t);
}

void
track_play(TrackInfo *t) {
	if(!t->audio) {
		t->audio = Mix_LoadMUS(t->filename);
		if(!t->audio) {
			printf("ERROR: Mix_LoadMUS(\"%s\") failed with message: \"%s\"\n", t->filename, Mix_GetError());
			exit(4);
		}
	}

	if(Mix_PlayMusic(t->audio, 1) == -1) {
		printf("ERROR: Mix_PlayMusic() failed with message: \"%s\"\n", Mix_GetError());
		exit(5);
	}
}




// don't you dare use this outside playlist_*
#define PLAYLIST_SIZE 10

typedef struct {
	TrackInfo *tracks[PLAYLIST_SIZE];
	int cur;
	int length;
	uint32_t start_time;
	uint32_t paused_at;
} Playlist;

Playlist*
playlist_new() {
	int i;
	Playlist *p = (Playlist*) malloc(sizeof(Playlist));
	if(!p) {
		fprintf(stderr, "Failed to allocate memory for playlist structure\n");
		exit(11);
	}

	for(i = 0; i < PLAYLIST_SIZE; ++i) {
		p->tracks[i] = 0;
	}
	p->cur = 0;
	p->length = 0;
	p->start_time = 0;
	p->paused_at = 0;

	return p;
}

Playlist* g_playlist = 0;

void dump_track(TrackInfo *t) {
	fprintf(stderr, "filename: %s, id: %s, title: %s, artist: %s, duration: %i\n", t->filename, t->id, t->title, t->artist, t->duration);
}

#ifdef DEBUG
void dump_playlist(Playlist *p) {
	int i;

	fprintf(stderr, "Playlist: cur=%i, length=%i\n", p->cur, p->length);

	for(i = 0; i < PLAYLIST_SIZE; ++i) {
		if(p->tracks[i]) {
			fprintf(stderr, "    track %i: ", i);
			dump_track(p->tracks[i]);
		}
	}
}
#else
#define dump_playlist(a)
#endif


void
playlist_init() {
	g_playlist = playlist_new();
}

void
playlist_delete(Playlist *p) {
	int i;

	for(i = 0; i < PLAYLIST_SIZE; ++i) {
		if(p->tracks[i]) {
			track_delete(p->tracks[i]);
		}
	}

	free(p);
}

uint8_t
playlist_is_empty(Playlist *p) {
	if(p->length == 0) {
		return 1;
	}

	return 0;
}

void
playlist_append(Playlist *p, TrackInfo *t) {
	if(p->length == PLAYLIST_SIZE) {
		fprintf(stderr, "Tried to add more tracks than are supported.\n");
		exit(10);
	}
	p->tracks[p->length] = t;
	p->length += 1;
}

TrackInfo *
playlist_cur(Playlist *p) {
	return p->tracks[p->cur];
}

// pass delta=1 for "next" track, delta=-1 for "prev" track
void
playlist_play_next(Playlist *p, int delta) {
	if(playlist_is_empty(p)) {
		fprintf(stderr, "ERROR: playlist empty\n");
		return;
	}

	// stop playback, reclaim memory, etc
	track_cleanup(playlist_cur(g_playlist));

	p->cur += delta; // next/prev track
	p->cur = (p->cur + p->length) % p->length; // loop at the ends

#ifdef DEBUG
	if(p->cur == 0 && delta == 1) {
		fprintf(stderr, "End of playlist. Playing first track again.\n");
	}
#endif

	track_play(playlist_cur(g_playlist));

	p->start_time = SDL_GetTicks();

	set_state(STATE_PLAYING);

	// let me know when the song is done.
	Mix_HookMusicFinished(music_finished);

#ifdef DEBUG
	fprintf(stderr, "Started previous song.\n");
#endif

	g_dirty = 1;
}

void
playlist_pause(Playlist *p) {
	set_state(STATE_PAUSED);
	Mix_PauseMusic();
	p->paused_at = SDL_GetTicks();
}

void
playlist_resume(Playlist *p) {
	Mix_ResumeMusic();
	set_state(STATE_PLAYING);
	p->start_time += SDL_GetTicks() - p->paused_at;
}

// return fraction of current song that has been played (0..1)
float
playlist_get_progress_seconds(Playlist *p) {
	TrackInfo *track;
	uint32_t delta;
	uint32_t duration;

	track = playlist_cur(p);

	if(!track) {
		return 0;
	}

	switch(get_state()) {
		case STATE_PLAYING:
			delta = SDL_GetTicks() - p->start_time;
		break;
		case STATE_PAUSED:
			delta = p->paused_at - p->start_time;
		break;
		default:
			return 0.0;
	}


	duration = track->duration;

	if(delta >= duration * 1000) {
		return duration;
	}

	return ((float)delta) / 1000;
}

// return fraction of current song that has been played (0..1)
float
playlist_get_progress_fraction(Playlist *p) {
	TrackInfo *track;

	track = playlist_cur(p);

	if(!track) {
		return 0;
	}

	return playlist_get_progress_seconds(p) / track->duration;
}

void
playlist_seek_seconds(Playlist *p, double position) {
	TrackInfo *t;
	uint32_t now;

	t = playlist_cur(p);

	if(!t) {
		return;
	}

	if(position < 0.0) {
		position = 0.0;
	}

	if(position > t->duration - 1) {
		position = t->duration - 1;
	}

	Mix_SetMusicPosition(position);
	now = SDL_GetTicks();
	p->start_time = now - (position * 1000);
	p->paused_at = now; // in case we're paused
}

typedef struct {
	int top, left, width, height, left_width, middle_width, capacity, text_indent_x, text_indent_y;
	SDL_Surface *surface;
	void(*draw)(void*);
} sprite;

sprite g_bubble_sprite = {
	SKIN_BUBBLE_TOP,
	SKIN_BUBBLE_LEFT,
	SKIN_BUBBLE_WIDTH,
	SKIN_BUBBLE_HEIGHT,
	SKIN_BUBBLE_LEFT_WIDTH,
	SKIN_BUBBLE_MIDDLE_WIDTH,
	SKIN_BUBBLE_TEXT_WIDTH - SKIN_BUBBLE_MIDDLE_WIDTH,
	SKIN_BUBBLE_TEXT_INDENT_X,
	SKIN_BUBBLE_TEXT_INDENT_Y,
	NULL,
	NULL
};

sprite g_save_sprite = {
	SKIN_SAVE_TOP,
	SKIN_SAVE_LEFT,
	SKIN_SAVE_WIDTH,
	SKIN_SAVE_HEIGHT,
	SKIN_SAVE_LEFT_WIDTH,
	SKIN_SAVE_MIDDLE_WIDTH,
	SKIN_SAVE_TEXT_WIDTH - SKIN_SAVE_MIDDLE_WIDTH,
	SKIN_SAVE_TEXT_INDENT_X,
	SKIN_SAVE_TEXT_INDENT_Y,
	NULL,
	NULL
};

sprite g_save_over_sprite = {
	SKIN_SAVE_OVER_TOP,
	SKIN_SAVE_OVER_LEFT,
	SKIN_SAVE_OVER_WIDTH,
	SKIN_SAVE_OVER_HEIGHT,
	SKIN_SAVE_OVER_LEFT_WIDTH,
	SKIN_SAVE_OVER_MIDDLE_WIDTH,
	SKIN_SAVE_OVER_TEXT_WIDTH - SKIN_SAVE_OVER_MIDDLE_WIDTH,
	SKIN_SAVE_OVER_TEXT_INDENT_X,
	SKIN_SAVE_OVER_TEXT_INDENT_Y,
	NULL,
	NULL
};

sprite g_trash_sprite = {
	SKIN_TRASH_TOP,
	SKIN_TRASH_LEFT,
	SKIN_TRASH_WIDTH,
	SKIN_TRASH_HEIGHT,
	0, 0, 0, 0, 0, NULL, NULL
};

sprite g_star_1_sprite = {
	SKIN_STAR_1_TOP,
	SKIN_STAR_1_LEFT,
	SKIN_STAR_1_WIDTH,
	SKIN_STAR_1_HEIGHT,
	0, 0, 0, 0, 0, NULL, NULL
};

sprite g_star_2_sprite = {
	SKIN_STAR_2_TOP,
	SKIN_STAR_2_LEFT,
	SKIN_STAR_2_WIDTH,
	SKIN_STAR_2_HEIGHT,
	0, 0, 0, 0, 0, NULL, NULL
};

sprite g_star_3_sprite = {
	SKIN_STAR_3_TOP,
	SKIN_STAR_3_LEFT,
	SKIN_STAR_3_WIDTH,
	SKIN_STAR_3_HEIGHT,
	0, 0, 0, 0, 0, NULL, NULL
};

sprite g_star_4_sprite = {
	SKIN_STAR_4_TOP,
	SKIN_STAR_4_LEFT,
	SKIN_STAR_4_WIDTH,
	SKIN_STAR_4_HEIGHT,
	0, 0, 0, 0, 0, NULL, NULL
};

sprite g_star_5_sprite = {
	SKIN_STAR_5_TOP,
	SKIN_STAR_5_LEFT,
	SKIN_STAR_5_WIDTH,
	SKIN_STAR_5_HEIGHT,
	0, 0, 0, 0, 0, NULL, NULL
};


void
draw_sprite_at(int x, int y, sprite* s) {
	SDL_Rect dest;
	dest.x = x; dest.y = y;

	SDL_BlitSurface(s->surface, NULL, surf_screen, &dest);
}

void
draw_sprite(sprite *s) {
	draw_sprite_at(s->left, s->top, s);
}

void
draw_expanding_sprite(int x, int y, int capacity, sprite *s) {
	int width_left, w;
	SDL_Rect dest, src;
	dest.x = x; dest.y = y;


	// draw left end of the expandable sprite
	src.x = 0; src.y = 0;
	src.w = s->left_width;
	src.h = s->height;
	SDL_BlitSurface(s->surface, &src, surf_screen, &dest); // changes dest if clipping happens
	src.x += s->left_width;
	x += s->left_width;
	dest.x = x;

	// draw middle of the expandable sprite
	width_left = capacity - (s->capacity);
	while(width_left > 0) {
		w = MIN(width_left, s->middle_width);
		src.w = w;
		SDL_BlitSurface(s->surface, &src, surf_screen, &dest); // changes dest if clipping happens
		x += w;
		dest.x = x;
		width_left -= w;
	}

	// draw the right end of the expandable sprite
	src.x += s->middle_width;
	src.w = s->width - s->left_width - s->middle_width;
	SDL_BlitSurface(s->surface, &src, surf_screen, &dest);
}

void
draw_text_sprite(int x, int y, sprite* s, SDL_Surface* text) {
	SDL_Rect dest;

	draw_expanding_sprite(x, y, text->w, s);

	// draw the text
	dest.x = x + s->text_indent_x;
	dest.y = y + s->text_indent_y;
	SDL_BlitSurface(text, NULL, surf_screen, &dest);
}

void
draw_expand_left(sprite* s, SDL_Surface* text) {
	draw_text_sprite(
		s->left + s->capacity - text->w + s->middle_width,
		s->top,
		s,
		text);
}

void
draw_bubble(sprite *target, SDL_Surface* text) {
	draw_text_sprite(
		target->left + (target->width / 2) + SKIN_BUBBLE_REL_X,
		target->top + SKIN_BUBBLE_REL_Y,
		&g_bubble_sprite,
		text);
}


// TODO consider making a memory structure to hold all this
void
draw() {
	int i;
	SDL_Rect dest;
#ifdef DEBUG
	fprintf(stderr, "redrawing (ticks: %i)\n", SDL_GetTicks());
#endif

	dest.x = SKIN_BACKGROUND_LEFT; dest.y = SKIN_BACKGROUND_TOP;
	SDL_BlitSurface(i_background, NULL, surf_screen, &dest);

	if(g_mouse_over == OVER_NEXT) {
		dest.x = SKIN_NEXT_OVER_LEFT; dest.y = SKIN_NEXT_OVER_TOP;
		SDL_BlitSurface(i_next_over, NULL, surf_screen, &dest);
	} else {
		dest.x = SKIN_NEXT_LEFT; dest.y = SKIN_NEXT_TOP;
		SDL_BlitSurface(i_next, NULL, surf_screen, &dest);
	}

	if(g_mouse_over == OVER_PREV) {
		dest.x = SKIN_PREV_OVER_LEFT; dest.y = SKIN_PREV_OVER_TOP;
		SDL_BlitSurface(i_prev_over, NULL, surf_screen, &dest);
	} else {
		dest.x = SKIN_PREV_LEFT; dest.y = SKIN_PREV_TOP;
		SDL_BlitSurface(i_prev, NULL, surf_screen, &dest);
	}

	if(g_mouse_over == OVER_SAVE) {
		draw_expand_left(&g_save_over_sprite, i_txt_save);
	} else {
		draw_expand_left(&g_save_sprite, i_txt_save);
	}

	for(i = 0; i < 4; ++i) {
		dest.x = SKIN_STAR_LEFT - (i * SKIN_STAR_SPACING); dest.y = SKIN_STAR_TOP;
		SDL_BlitSurface(i_star, NULL, surf_screen, &dest);
	}

	dest.x = SKIN_NOSTAR_LEFT; dest.y = SKIN_NOSTAR_TOP;
	SDL_BlitSurface(i_nostar, NULL, surf_screen, &dest);

	dest.x = SKIN_TRASH_LEFT; dest.y = SKIN_TRASH_TOP;
	SDL_BlitSurface(i_trash, NULL, surf_screen, &dest);

	dest.x = SKIN_BAR_LEFT; dest.y = SKIN_BAR_TOP;
	SDL_BlitSurface(i_bar, NULL, surf_screen, &dest);

	dest.x = SKIN_SLIDER_LEFT + (SKIN_SLIDER_SPAN * playlist_get_progress_fraction(g_playlist)); dest.y = SKIN_SLIDER_TOP;
	SDL_BlitSurface(i_slider, NULL, surf_screen, &dest);

	if(get_state() == STATE_PLAYING || get_state() == STATE_STARTING) {
		if(g_mouse_over == OVER_PAUSE) {
			dest.x = SKIN_PAUSE_OVER_LEFT; dest.y = SKIN_PAUSE_OVER_TOP;
			SDL_BlitSurface(i_pause_over, NULL, surf_screen, &dest);
		} else {
			dest.x = SKIN_PAUSE_LEFT; dest.y = SKIN_PAUSE_TOP;
			SDL_BlitSurface(i_pause, NULL, surf_screen, &dest);
		}
	} else {
		if(g_mouse_over == OVER_PLAY) {
			dest.x = SKIN_PLAY_OVER_LEFT; dest.y = SKIN_PLAY_OVER_TOP;
			SDL_BlitSurface(i_play_over, NULL, surf_screen, &dest);
		} else {
			dest.x = SKIN_PLAY_LEFT; dest.y = SKIN_PLAY_TOP;
			SDL_BlitSurface(i_play, NULL, surf_screen, &dest);
		}
	}

	text_draw();


	switch(g_mouse_over) {
		case OVER_TRASH:
			draw_bubble(&g_trash_sprite, i_txt_bubble_trash);
		break;
		case OVER_STAR_1:
			draw_bubble(&g_star_1_sprite, i_txt_bubble_1);
		break;
		case OVER_STAR_2:
			draw_bubble(&g_star_2_sprite, i_txt_bubble_2);
		break;
		case OVER_STAR_3:
			draw_bubble(&g_star_3_sprite, i_txt_bubble_3);
		break;
		case OVER_STAR_4:
			draw_bubble(&g_star_4_sprite, i_txt_bubble_4);
		break;
		case OVER_STAR_5:
			draw_bubble(&g_star_5_sprite, i_txt_bubble_5);
		break;
	}


	// Update the surface
	SDL_Flip(surf_screen);

	g_dirty = 0;
}

SDL_Surface *
load_image(char *filename) {
	SDL_Surface *img = NULL;
	img = IMG_Load(filename);
	if(!img) {
		fputs("Couldn't find data files for skin\n", stderr);
		exit(3);
	}
	return img;
}

void
load_skin() {
	i_background = load_image(SKIN_PREFIX"background.png");
	i_next = load_image(SKIN_PREFIX"next.png");
	i_prev = load_image(SKIN_PREFIX"prev.png");
	i_save = load_image(SKIN_PREFIX"save.png");
	i_star = load_image(SKIN_PREFIX"star.png");
	i_nostar = load_image(SKIN_PREFIX"nostar.png");
	i_trash = load_image(SKIN_PREFIX"trash.png");
	i_bar = load_image(SKIN_PREFIX"bar.png");
	i_slider = load_image(SKIN_PREFIX"slider.png");
	i_pause = load_image(SKIN_PREFIX"pause.png");
	i_play = load_image(SKIN_PREFIX"play.png");
	i_next_over = load_image(SKIN_PREFIX"next_over.png");
	i_prev_over = load_image(SKIN_PREFIX"prev_over.png");
	i_save_over = load_image(SKIN_PREFIX"save_over.png");
	i_pause_over = load_image(SKIN_PREFIX"pause_over.png");
	i_play_over = load_image(SKIN_PREFIX"play_over.png");
	i_bubble = load_image(SKIN_PREFIX"bubble.png");

	g_bubble_sprite.surface = i_bubble;
	g_save_sprite.surface   = i_save;
	g_save_over_sprite.surface   = i_save_over;
	g_trash_sprite.surface = i_star;
	g_star_1_sprite.surface = i_star;
	g_star_2_sprite.surface = i_star;
	g_star_3_sprite.surface = i_star;
	g_star_4_sprite.surface = i_star;
	g_star_5_sprite.surface = i_star;
}

void
play_seek_fraction(double position) {
	TrackInfo *t = playlist_cur(g_playlist);
	if(!t) {
		fprintf(stderr, "ERROR: tried to seek, but there's no track\n");
		return;
	}
	playlist_seek_seconds(g_playlist, t->duration * position);
	g_dirty = 1;
}

void
play_seek_delta(int delta) {
	TrackInfo *t = playlist_cur(g_playlist);
	if(!t) {
		fprintf(stderr, "ERROR: tried to seek, but there's no track\n");
		return;
	}
	playlist_seek_seconds(g_playlist, playlist_get_progress_seconds(g_playlist) + delta);
	g_dirty = 1;
}

void
move_slider_to_mouse() {
	int x = g_mouse_x;
	if(g_mouse_x <= SKIN_SLIDER_0_X) {
		x = SKIN_SLIDER_0_X;
	} else if(g_mouse_x >= SKIN_SLIDER_100_X) {
		x = SKIN_SLIDER_100_X; // Note: seeking maxes out at duration-1 seconds
	}
#ifdef DEBUG
	fprintf(stderr, "Seeking to %f%%\n", (((float)x - SKIN_SLIDER_0_X) / ((float)SKIN_SLIDER_SPAN)) * 100);
#endif
	play_seek_fraction(((float)x - SKIN_SLIDER_0_X) / ((float)SKIN_SLIDER_SPAN));
}

int
within_2d(int x, int y, int left, int top, int width, int height) {
	if(x < left) return 0;
	if(y < top) return 0;
	if(x >= left + width) return 0;
	if(y >= top + height) return 0;
	return 1;
}

void
mouse_moved() {
	int mouse_over = 0;

	switch(g_dragging) {
		case 0:
		break;
		case DRAGGING_PREV:
		case DRAGGING_NEXT:
		return; // don't do mouse-overs while dragging
		case DRAGGING_SLIDER:
		move_slider_to_mouse();
		return; // don't do mouse-overs while dragging
	} // else {

	if(within_2d(g_mouse_x, g_mouse_y, SKIN_PREV_LEFT, SKIN_PREV_TOP,
	                                   SKIN_PREV_WIDTH, SKIN_PREV_HEIGHT)) {
		mouse_over = OVER_PREV;
	} else if(within_2d(g_mouse_x, g_mouse_y, SKIN_NEXT_LEFT, SKIN_NEXT_TOP,
	                                          SKIN_NEXT_WIDTH, SKIN_NEXT_HEIGHT)) {
		mouse_over = OVER_NEXT;
	} else if(within_2d(g_mouse_x, g_mouse_y, g_skin_save_left, SKIN_SAVE_TOP,
	                                          g_skin_save_width, SKIN_SAVE_HEIGHT)) {
		mouse_over = OVER_SAVE;
	} else if(within_2d(g_mouse_x, g_mouse_y, SKIN_BAR_LEFT, SKIN_SLIDER_TOP,
	                                          SKIN_BAR_WIDTH, SKIN_SLIDER_HEIGHT)) {
		mouse_over = OVER_BAR;
	} else if(get_state() == STATE_PLAYING &&
	          within_2d(g_mouse_x, g_mouse_y, SKIN_PAUSE_LEFT, SKIN_PAUSE_TOP,
			                                SKIN_PAUSE_WIDTH, SKIN_PAUSE_HEIGHT)) {
		mouse_over = OVER_PAUSE;
	} else if(get_state() != STATE_PLAYING &&
	          within_2d(g_mouse_x, g_mouse_y, SKIN_PLAY_LEFT, SKIN_PLAY_TOP,
			                                SKIN_PLAY_WIDTH, SKIN_PLAY_HEIGHT)) {
		mouse_over = OVER_PLAY;
	} else if(within_2d(g_mouse_x, g_mouse_y, SKIN_TRASH_LEFT, SKIN_TRASH_TOP,
			                                SKIN_TRASH_WIDTH, SKIN_TRASH_HEIGHT)) {
		mouse_over = OVER_TRASH;
	} else if(within_2d(g_mouse_x, g_mouse_y, SKIN_STAR_1_LEFT, SKIN_STAR_TOP,
			                                SKIN_STAR_WIDTH, SKIN_STAR_HEIGHT)) {
		mouse_over = OVER_STAR_1;
	} else if(within_2d(g_mouse_x, g_mouse_y, SKIN_STAR_2_LEFT, SKIN_STAR_TOP,
			                                SKIN_STAR_WIDTH, SKIN_STAR_HEIGHT)) {
		mouse_over = OVER_STAR_2;
	} else if(within_2d(g_mouse_x, g_mouse_y, SKIN_STAR_3_LEFT, SKIN_STAR_TOP,
			                                SKIN_STAR_WIDTH, SKIN_STAR_HEIGHT)) {
		mouse_over = OVER_STAR_3;
	} else if(within_2d(g_mouse_x, g_mouse_y, SKIN_STAR_4_LEFT, SKIN_STAR_TOP,
			                                SKIN_STAR_WIDTH, SKIN_STAR_HEIGHT)) {
		mouse_over = OVER_STAR_4;
	} else if(within_2d(g_mouse_x, g_mouse_y, SKIN_STAR_5_LEFT, SKIN_STAR_TOP,
			                                SKIN_STAR_WIDTH, SKIN_STAR_HEIGHT)) {
		mouse_over = OVER_STAR_5;
	}

	if(mouse_over != g_mouse_over) {
		g_mouse_over = mouse_over;
		g_dirty = 1;
	}
}

void
recalculate_mouseover() {
	mouse_moved();
}

void
play_prev() {
	dump_playlist(g_playlist);
	playlist_play_next(g_playlist, -1);
	dump_playlist(g_playlist);
	g_dirty = 1;
}

void
play_next() {
	dump_playlist(g_playlist);
	playlist_play_next(g_playlist, 1);
	dump_playlist(g_playlist);
	g_dirty = 1;
}


//void
//play_stop() {
//	set_state(STATE_STOPPED);
//	Mix_HaltMusic();
//}

void
play_pause() {
	playlist_pause(g_playlist);
	g_dirty = 1;
}

void
play_toggle_paused() {
	if(get_state() == STATE_PAUSED) {
		playlist_resume(g_playlist);
	} else {
		playlist_pause(g_playlist);
	}
	g_dirty = 1;
}

void
play() {
	dump_playlist(g_playlist);

	if(get_state() == STATE_PAUSED) {
		playlist_resume(g_playlist);
	} else {
		playlist_play_next(g_playlist, 0);
	}

	dump_playlist(g_playlist);
	g_dirty = 1;
}

// call this to perform a seek that's been scheduled for now
void
delayed_seek() {
#ifdef DEBUG
	fprintf(stderr, "delayed seek\n");
#endif
	switch(g_dragging) {
		case DRAGGING_PREV:
			play_seek_delta(-SMALL_SEEK);
		break;
		case DRAGGING_NEXT:
			play_seek_delta(SMALL_SEEK);
		break;
		default:
			return;
	}
	g_seek_at = SDL_GetTicks() + SEEK_INTERVAL;
}

// this function assumes that the mouse was clicked at (g_mouse_x, g_mouse_y)
void
mouse_clicked(int button) {
	switch(g_mouse_over) {
		case OVER_PREV:
			// play_prev();
			g_dragging = DRAGGING_PREV;
			g_dragging_start = SDL_GetTicks();
			g_seek_at = g_dragging_start + CLICK_THRESHOLD;
		break;
		case OVER_PAUSE:
			if(get_state() == STATE_PLAYING) {
				play_pause();
			}
		break;
		case OVER_PLAY:
			play();
		break;
		case OVER_NEXT:
			// play_next();
			g_dragging = DRAGGING_NEXT;
			g_dragging_start = SDL_GetTicks();
			g_seek_at = g_dragging_start + CLICK_THRESHOLD;
		break;
		case OVER_SAVE:
			fprintf(stderr, "Sorry, saving isn't implemented yet.\n"); // FIXME
		break;
		case OVER_BAR:
			move_slider_to_mouse();
			g_dragging = DRAGGING_SLIDER;
		break;
	}
}

// only call from within playlist_*()
void
set_state(int state) {
	if(g_state != state) {
		g_state = state;
		recalculate_mouseover();
		g_dirty = 1;
	}
}

int
get_state() {
	return g_state;
}

TTF_Font*
font_init(char* file, int ptsize) {
	TTF_Font* f;
	f = TTF_OpenFont(file, ptsize);
	if (!f){
		fprintf(stderr, "ERROR: TTF_OpenFont() failed: %s\n", TTF_GetError());
		exit(7);
	}
	return f;
}

SDL_Surface*
text_to_surface(const char* txt) {
	SDL_Color black = {0, 0, 0, 125};

	SDL_Surface* surf = TTF_RenderUTF8_Blended(g_artist_font, txt, black);
	if(!surf) {
		fprintf(stderr, "TTF_RenderText_Blended(%s) failed\n", txt);
		exit(8);
	}

	return surf;
}

void
text_init() {
	if (TTF_Init() == -1) {
		printf("ERROR: TTF_Init() failed: %s\n", TTF_GetError());
		exit(6);
	}

	g_artist_font = font_init("FreeSerif.ttf", 18);

	i_txt_bubble_trash = text_to_surface(TRANSLATE("Delete now."));
	i_txt_bubble_1 = text_to_surface(TRANSLATE("Feh!"));
	i_txt_bubble_2 = text_to_surface(TRANSLATE("OK."));
	i_txt_bubble_3 = text_to_surface(TRANSLATE("Nice."));
	i_txt_bubble_4 = text_to_surface(TRANSLATE("Good stuff!"));
	i_txt_bubble_5 = text_to_surface(TRANSLATE("I love it!"));

	i_txt_save = text_to_surface(TRANSLATE("Save"));
	g_skin_save_left = SKIN_SAVE_LEFT + SKIN_SAVE_TEXT_WIDTH - i_txt_save->w;
	g_skin_save_width = SKIN_SAVE_WIDTH + (SKIN_SAVE_LEFT - g_skin_save_left);
}

void
text_draw() {
	SDL_Rect dest;
	TrackInfo *t = playlist_cur(g_playlist);
	if(!t) { return; } // FIXME do something more clever?

	if(!t->title_tex) {
		t->title_tex = text_to_surface(t->title);
	}
	dest.x = SKIN_TITLE_LEFT + 3; dest.y = SKIN_TITLE_TOP;
	SDL_BlitSurface(t->title_tex, NULL, surf_screen, &dest);

	if(!t->artist_tex) {
		t->artist_tex = text_to_surface(t->artist);
	}
	dest.x = SKIN_TITLE_LEFT + 3; dest.y = SKIN_ARTIST_TOP;
	SDL_BlitSurface(t->artist_tex, NULL, surf_screen, &dest);
}

#define NSDUP(a) memcpy(malloc(sizeof(a)), a, sizeof(a))
void
add_testing_tracks() {
	playlist_append(g_playlist, track_new(NSDUP(
		"test_1.ogg\000id1\000﻿മലയാളം malayāḷaṁ\000by Testing unusual characters\000254")));
	playlist_append(g_playlist, track_new(NSDUP(
		"test_2.ogg\000id2\000Terse\000by Verse\000324")));
	playlist_append(g_playlist, track_new(NSDUP(
		"test_3.ogg\000id3\000Track name goes here\000by Band name goes here\000269")));
	playlist_append(g_playlist, track_new(NSDUP(
		"test_4.ogg\000id4\000Testing unusually long track name that might not fit\000by a realy long artist name that might not fit\000436")));
	playlist_append(g_playlist, track_new(NSDUP(
		"test_5.ogg\000id5\000The last track in the playlist\000by will loop after this\000271")));
}

int
main(int argc, char **argv) {
	int new_mouse_x, new_mouse_y;
	int have_event;
	int i;
	SDL_Event e;

	// Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		fputs("SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) failed\n", stderr);
		SDL_Quit();
		return 1;
	}
	atexit(SDL_Quit);
	atexit(SDL_CloseAudio);

	// Initialize output with SDL_mixer
	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, AUDIO_S16, MIX_DEFAULT_CHANNELS, 4096) < 0) {
		fprintf(stderr, "Couldn't open SDL_mixer audio: %s\n", SDL_GetError());
		return 2;
	}

	// Open window
	surf_screen = SDL_SetVideoMode(SKIN_BACKGROUND_WIDTH,SKIN_BACKGROUND_HEIGHT,32,0);

	// Set the title bar text
	SDL_WM_SetCaption("Simple Open Music Player", "somp");

	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	load_skin();

	text_init();

	playlist_init();

	add_testing_tracks();

#ifdef DEBUG
	fprintf(stderr, "SKIN_SLIDER_SPAN: %i\n", SKIN_SLIDER_SPAN);
	fprintf(stderr, "SKIN_SLIDER_0_X: %i\n", SKIN_SLIDER_0_X);
	fprintf(stderr, "SKIN_SLIDER_100_X: %i\n", SKIN_SLIDER_100_X);
#endif

	SDL_GetMouseState(&new_mouse_x, &new_mouse_y);

	play();

	have_event = 0;
	for(;;) {
		while(have_event) {
			switch(e.type) {
				case SDL_QUIT:
					return 0;
				case SDL_KEYDOWN:
					switch(e.key.keysym.sym) {
						case SDLK_q:
						case SDLK_ESCAPE:
							return 0;
						break;
						case SDLK_SPACE:
						case SDLK_p:
							play_toggle_paused();
						break;
						case SDLK_RIGHT:
							play_seek_delta(SMALL_SEEK);
						break;
						case SDLK_LEFT:
							play_seek_delta(-SMALL_SEEK);
						break;
						default:
						break;
					}
					break;
				case SDL_MOUSEMOTION:
					new_mouse_x = e.motion.x;
					new_mouse_y = e.motion.y;
					if(g_dragging && !e.motion.state) {
						// if we miss an event, it would be very bad to be stuck dragging the slider
						g_dragging = 0;
					}
				break;
				case SDL_MOUSEBUTTONDOWN:
					new_mouse_x = e.button.x;
					new_mouse_y = e.button.y;
					if(new_mouse_x != g_mouse_x || new_mouse_y != g_mouse_y) {
						g_mouse_x = new_mouse_x;
						g_mouse_y = new_mouse_y;
						mouse_moved();
					}
					mouse_clicked(e.button.button); // needs mouse_moved() to be called first
				break;
				case SDL_MOUSEBUTTONUP:
					if(g_dragging) {
						if(new_mouse_x != g_mouse_x || new_mouse_y != g_mouse_y) {
							g_mouse_x = new_mouse_x;
							g_mouse_y = new_mouse_y;
							mouse_moved(); // do the last of the drag
						}
						if(g_dragging == DRAGGING_PREV || g_dragging == DRAGGING_NEXT) {
							if(SDL_GetTicks() - g_dragging_start < CLICK_THRESHOLD) {
								if(g_dragging == DRAGGING_PREV) {
									play_prev();
								} else {
									play_next();
								}
							}
						}
						g_dragging = 0;
						mouse_moved(); // check for rollovers now that they're on again
					}
				break;
				case SDL_USEREVENT:
#ifdef DEBUG
					fprintf(stderr, "got a user event!\n");
#endif
					if(e.user.code == EVENT_MUSIC_FINISHED) {
						if(get_state() == STATE_PLAYING) {
							play_next();
						}
					}
				break;
				default:
				break;
			}
			have_event = SDL_PollEvent(&e);
		}

		if(new_mouse_x != g_mouse_x || new_mouse_y != g_mouse_y) {
			g_mouse_x = new_mouse_x;
			g_mouse_y = new_mouse_y;
			mouse_moved();
		}

		if(g_dirty) {
			draw();
		}

		// if we're playing, we need to be updating the progress bar
		if(get_state() == STATE_PLAYING) {
			have_event = 0;
			i = 0;

			while(!have_event) {
				if(g_dragging == DRAGGING_PREV || g_dragging == DRAGGING_NEXT) {
					uint32_t delta;
					delta = g_seek_at - SDL_GetTicks();
					if(delta == 0 || delta > 4000) {
						delayed_seek();
					} else {
						fprintf(stderr, "waiting for seek for %ims\n", delta);
						have_event = wait_event_with_timeout(&e, delta);
						if(!have_event) { // waited full time
							delayed_seek();
						}
					}
				} else {
					have_event = wait_event_with_timeout(&e, SLIDER_LAG);
					if(!have_event) {
						g_dirty = 1;
					}
				}

				// when holding mouse down on prev/next or playing, the
				// display must be updated even though there's no events
				// coming in
				if(g_dirty) {
					draw();
				}
			}
		} else {
			SDL_WaitEvent(&e);
		}

		have_event = 1;
	}

	// make the compiler happy
	return 0;
}
