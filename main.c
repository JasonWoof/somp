/* Open Content Radio copyright (C) 2008 Jason Woofenden
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

#include "skin_coords.h"

#ifndef SKIN_PREFIX
#define SKIN_PREFIX "skin/"
#endif

#define STATE_STARTING 0
#define STATE_PLAYING 1
#define STATE_PAUSED 2
#define STATE_STOPPED 3

#define OVER_PREV 1
#define OVER_PLAY 2
#define OVER_PAUSE 3
#define OVER_NEXT 4
#define OVER_SAVE 5

#define EVENT_MUSIC_FINISHED 1

// GLOBALS
TTF_Font *g_track_font, *g_artist_font;
Mix_Music *g_music = 0;
// Playlist* g_playlist defined later
int g_mouse_over = 0;
int g_mouse_x, g_mouse_y;
int g_dirty = 1;
int g_track = 0;
int g_state = STATE_STOPPED;
SDL_Surface *surf_screen;
SDL_Surface *i_background, *i_next, *i_prev, *i_art, *i_save, *i_star, *i_nostar, *i_trash, *i_bar, *i_slider, *i_pause, *i_play, *i_next_over, *i_prev_over, *i_save_over, *i_pause_over, *i_play_over;

void set_state(int state);
int get_state();
void text_draw();

void
music_finished() {
	SDL_Event e;
	fprintf(stderr, "Reached end of track.\n");

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

	fprintf(stderr, "filename: %s, id: %s, title: %s, artist: %s, duration: %i\n", t->filename, t->id, t->title, t->artist, t->duration);

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
playlist_get_progress(Playlist *p) {
	TrackInfo *track;
	uint32_t delta;
	uint32_t length;

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


	length = playlist_cur(p)->duration * 1000;

	if(delta >= length) {
		return 1.0;
	}
	
	return ((float)delta) / ((float) length);
}



// TODO consider making a memory structure to hold all this
void
draw() {
	int i;
	SDL_Rect dest;
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

	dest.x = SKIN_ART_LEFT; dest.y = SKIN_ART_TOP;
	SDL_BlitSurface(i_art, NULL, surf_screen, &dest);

	if(g_mouse_over == OVER_SAVE) {
		dest.x = SKIN_SAVE_OVER_LEFT; dest.y = SKIN_SAVE_OVER_TOP;
		SDL_BlitSurface(i_save_over, NULL, surf_screen, &dest);
	} else {
		dest.x = SKIN_SAVE_LEFT; dest.y = SKIN_SAVE_TOP;
		SDL_BlitSurface(i_save, NULL, surf_screen, &dest);
	}

	for(i = 0; i < 4; ++i) {
		dest.x = SKIN_STAR_LEFT - (i * (SKIN_NOSTAR_LEFT - SKIN_STAR_LEFT)); dest.y = SKIN_STAR_TOP;
		SDL_BlitSurface(i_star, NULL, surf_screen, &dest);
	}

	dest.x = SKIN_NOSTAR_LEFT; dest.y = SKIN_NOSTAR_TOP;
	SDL_BlitSurface(i_nostar, NULL, surf_screen, &dest);

	dest.x = SKIN_TRASH_LEFT; dest.y = SKIN_TRASH_TOP;
	SDL_BlitSurface(i_trash, NULL, surf_screen, &dest);

	dest.x = SKIN_BAR_LEFT; dest.y = SKIN_BAR_TOP;
	SDL_BlitSurface(i_bar, NULL, surf_screen, &dest);

#define SLIDER_SPAN (SKIN_BAR_LEFT + SKIN_BAR_WIDTH - 2 * (SKIN_SLIDER_LEFT - SKIN_BAR_LEFT) - SKIN_SLIDER_WIDTH)

	dest.x = SKIN_SLIDER_LEFT + (SLIDER_SPAN * playlist_get_progress(g_playlist)); dest.y = SKIN_SLIDER_TOP;
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
	i_art = load_image(SKIN_PREFIX"art.png");
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
	if(within_2d(g_mouse_x, g_mouse_y, SKIN_PREV_LEFT, SKIN_PREV_TOP, SKIN_PREV_WIDTH, SKIN_PREV_HEIGHT)) {
		mouse_over = OVER_PREV;
	} else if(within_2d(g_mouse_x, g_mouse_y, SKIN_NEXT_LEFT, SKIN_NEXT_TOP, SKIN_NEXT_WIDTH, SKIN_NEXT_HEIGHT)) {
		mouse_over = OVER_NEXT;
	} else if(within_2d(g_mouse_x, g_mouse_y, SKIN_SAVE_LEFT, SKIN_SAVE_TOP, SKIN_SAVE_WIDTH, SKIN_SAVE_HEIGHT)) {
		mouse_over = OVER_SAVE;
	} else {
		if(get_state() == STATE_PLAYING) {
			if(within_2d(g_mouse_x, g_mouse_y, SKIN_PAUSE_LEFT, SKIN_PAUSE_TOP, SKIN_PAUSE_WIDTH, SKIN_PAUSE_HEIGHT)) {
				mouse_over = OVER_PAUSE;
			}
		} else {
			if(within_2d(g_mouse_x, g_mouse_y, SKIN_PLAY_LEFT, SKIN_PLAY_TOP, SKIN_PLAY_WIDTH, SKIN_PLAY_HEIGHT)) {
				mouse_over = OVER_PLAY;
			}
		}
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
play_seek_delta(int delta) {
	// FIXME seek currently playing song by delta seconds
}

void
play_prev() {
	dump_playlist(g_playlist);
	playlist_play_next(g_playlist, -1);
	dump_playlist(g_playlist);
}

void
play_next() {
	dump_playlist(g_playlist);
	playlist_play_next(g_playlist, 1);
	dump_playlist(g_playlist);
}


//void
//play_stop() {
//	set_state(STATE_STOPPED);
//	Mix_HaltMusic();
//}

void
play_pause() {
	playlist_pause(g_playlist);
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
}


// this function assumes that the mouse was clicked at (g_mouse_x, g_mouse_y)
void
mouse_clicked(int button) {
	switch(g_mouse_over) {
		case OVER_PREV:
			play_prev();
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
			play_next();
		break;
		case OVER_SAVE:
			fprintf(stderr, "Sorry, saving isn't implemented yet.\n"); // FIXME
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

void
text_init() {
	if (TTF_Init() == -1) {
		printf("ERROR: TTF_Init() failed: %s\n", TTF_GetError());
		exit(6);
	}

	g_track_font = font_init("FreeSerif.ttf", 20);
	g_artist_font = font_init("FreeSerif.ttf", 18);
}

void
text_draw() {
	SDL_Color black = {0, 0, 0, 125};
	SDL_Rect dest;
	TrackInfo *t = playlist_cur(g_playlist);
	if(!t) { return; } // FIXME do something more clever?

	if(!t->title_tex) {
		t->title_tex = TTF_RenderText_Blended(g_track_font, t->title, black);
		if(!t->title_tex) {
			fprintf(stderr, "TTF_RenderText_Blended(title) failed\n");
			exit(8);
		}
	}
	dest.x = SKIN_TITLE_LEFT; dest.y = SKIN_TITLE_TOP;
	SDL_BlitSurface(t->title_tex, NULL, surf_screen, &dest);

	if(!t->artist_tex) {
		t->artist_tex = TTF_RenderText_Blended(g_artist_font, t->artist, black);
		if(!t->artist_tex) {
			fprintf(stderr, "TTF_RenderText_Blended(artist) failed\n");
			exit(8);
		}
	}
	dest.x = SKIN_ARTIST_LEFT; dest.y = SKIN_ARTIST_TOP;
	SDL_BlitSurface(t->artist_tex, NULL, surf_screen, &dest);
}

#define NSDUP(a) memcpy(malloc(sizeof(a)), a, sizeof(a))
void
add_testing_tracks() {
	playlist_append(g_playlist, track_new(NSDUP(
		"test_1.ogg\000id1\000First Test Song\000First Test Artist\000254")));
	playlist_append(g_playlist, track_new(NSDUP(
		"test_2.ogg\000id2\000Second Test Song\000Second Test Artist\000324")));
	playlist_append(g_playlist, track_new(NSDUP(
		"test_3.ogg\000id3\000Third Test Song\000Third Test Artist\000269")));
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
	SDL_WM_SetCaption("Open Music Radio", "OMR");

	load_skin();

	text_init();

	playlist_init();

	add_testing_tracks();

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
							play_pause();
						break;
						case SDLK_RIGHT:
							play_seek_delta(5);
						break;
						case SDLK_LEFT:
							play_seek_delta(-5);
						break;
						default:
						break;
					}
					break;
				case SDL_MOUSEMOTION:
					new_mouse_x = e.motion.x;
					new_mouse_y = e.motion.y;
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
				case SDL_USEREVENT:
					// fprintf(stderr, "got a user event!\n");
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
			for(i = 0; !have_event; ++i) {
				have_event = SDL_PollEvent(&e);
				if(!have_event) {
					SDL_Delay(50);
					if(i % 14) {
						g_dirty = 1;
						draw();
					}
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
