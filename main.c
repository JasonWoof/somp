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
Mix_Music *g_music = 0;
int g_mouse_over = 0;
int g_mouse_x, g_mouse_y;
int g_dirty = 1;
int g_track = 0;
int g_state = STATE_STOPPED;
SDL_Surface *surf_screen;
SDL_Surface *i_background, *i_next, *i_prev, *i_art, *i_save, *i_title, *i_artist, *i_star, *i_nostar, *i_trash, *i_bar, *i_slider, *i_pause, *i_play, *i_next_over, *i_prev_over, *i_save_over, *i_pause_over, *i_play_over;

void set_state(int state);
int get_state();

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

	dest.x = SKIN_TITLE_LEFT; dest.y = SKIN_TITLE_TOP;
	SDL_BlitSurface(i_title, NULL, surf_screen, &dest);

	dest.x = SKIN_ARTIST_LEFT; dest.y = SKIN_ARTIST_TOP;
	SDL_BlitSurface(i_artist, NULL, surf_screen, &dest);

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

	dest.x = SKIN_SLIDER_LEFT; dest.y = SKIN_SLIDER_TOP;
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
	i_title = load_image(SKIN_PREFIX"title.png");
	i_artist = load_image(SKIN_PREFIX"artist.png");
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
play_next() {
	if(g_music) {
		Mix_FreeMusic(g_music);
		g_music = 0;
	}

	g_track += 1;

	if(g_track % 2) {
		g_music = Mix_LoadMUS("test_short.ogg");
	} else {
		g_music = Mix_LoadMUS("test_short2.ogg");
	}
	if(!g_music) {
		printf("Mix_LoadMUS(\"test_short[2].ogg\"): %s\n", Mix_GetError());
		exit(3);
	}

	if(Mix_PlayMusic(g_music, 1) == -1) {
		printf("Mix_PlayMusic: %s\n", Mix_GetError());
		exit(4);
	}

	set_state(STATE_PLAYING);

	// let me know when the song is done.
	Mix_HookMusicFinished(music_finished);

	fprintf(stderr, "Starting next song.\n");
}

void
play_stop() {
	if(!g_music) { return; }

	set_state(STATE_STOPPED);
	Mix_HaltMusic();
}

void
play_pause() {
	if(!g_music) { return; }

	set_state(STATE_PAUSED);
	Mix_PauseMusic();
}

void
play_resume() {
	if(!g_music) { return; }

	set_state(STATE_PLAYING);
	Mix_ResumeMusic();
}


// this function assumes that the mouse was clicked at (g_mouse_x, g_mouse_y)
void
mouse_clicked(int button) {
	switch(g_mouse_over) {
		case OVER_PREV:
			play_next(); // FIXME
		break;
		case OVER_PAUSE:
			if(get_state() == STATE_PLAYING) {
				play_pause();
			}
		break;
		case OVER_PLAY:
			if(get_state() == STATE_PAUSED) {
				play_resume();
			} else if(get_state() == STATE_STOPPED) {
				play_next();
			}
		break;
		case OVER_NEXT:
			play_next();
		break;
		case OVER_SAVE:
			fprintf(stderr, "Sorry, saving isn't implemented yet.\n"); // FIXME
		break;
	}
}

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

int
main(int argc, char **argv) {
	int new_mouse_x, new_mouse_y;
	int have_event;
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

	SDL_GetMouseState(&new_mouse_x, &new_mouse_y);

	play_next();

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
						case SDLK_RIGHT:
							if(g_track < 4) {
								play_next();
							} else {
								set_state(STATE_STOPPED);
							}
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

		SDL_WaitEvent(&e);
		have_event = 1;
	}

	// make the compiler happy
	return 0;
}
