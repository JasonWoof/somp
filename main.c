/* radio copyright (C) 2008 Jason Woofenden
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
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

// GLOBALS
int g_done = 0;
int g_state = STATE_STARTING;
SDL_Surface *surf_screen;
SDL_Surface *i_background, *i_next, *i_prev, *i_art, *i_save, *i_title, *i_artist, *i_star, *i_nostar, *i_trash, *i_bar, *i_slider, *i_pause, *i_play, *i_next_over, *i_prev_over, *i_save_over, *i_pause_over, *i_play_over;

// make a music finished function
void
music_finished() {
	printf("Music stopped.\n");
	g_done = 1;
	g_state = STATE_PAUSED; // FIXME this should be STATE_STARTING... I just wanted to see the display change :)
}

void
draw() {
	int i;
	SDL_Rect dest;
	dest.x = SKIN_BACKGROUND_LEFT; dest.y = SKIN_BACKGROUND_TOP;
	SDL_BlitSurface(i_background, NULL, surf_screen, &dest);

	dest.x = SKIN_NEXT_LEFT; dest.y = SKIN_NEXT_TOP;
	SDL_BlitSurface(i_next, NULL, surf_screen, &dest);

	dest.x = SKIN_PREV_LEFT; dest.y = SKIN_PREV_TOP;
	SDL_BlitSurface(i_prev, NULL, surf_screen, &dest);

	dest.x = SKIN_ART_LEFT; dest.y = SKIN_ART_TOP;
	SDL_BlitSurface(i_art, NULL, surf_screen, &dest);

	dest.x = SKIN_SAVE_LEFT; dest.y = SKIN_SAVE_TOP;
	SDL_BlitSurface(i_save, NULL, surf_screen, &dest);

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

	if(g_state == STATE_PLAYING || g_state == STATE_STARTING) {
		dest.x = SKIN_PAUSE_LEFT; dest.y = SKIN_PAUSE_TOP;
		SDL_BlitSurface(i_pause, NULL, surf_screen, &dest);
	} else {
		dest.x = SKIN_PLAY_LEFT; dest.y = SKIN_PLAY_TOP;
		SDL_BlitSurface(i_play, NULL, surf_screen, &dest);
	}

	// Update the surface
	SDL_Flip(surf_screen);
}

SDL_Surface *
load_image(char *filename)
{
	SDL_Surface *tmp, *img = NULL;
	img = IMG_Load(filename);
	if(!img) {
		fputs("Couldn't find data files for skin\n", stderr);
		exit(3);
	}
	return img;
}

int main(int argc, char **argv) {
	
	// Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		fputs("SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) failed\n", stderr);
		SDL_Quit();
		return 1;
	}
	atexit(SDL_Quit);
	atexit(SDL_CloseAudio);

	// Initialise output with SDL_mixer
	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, AUDIO_S16, MIX_DEFAULT_CHANNELS, 4096) < 0) {
		fprintf(stderr, "Couldn't open SDL_mixer audio: %s\n", SDL_GetError());
		return 2;
	}

	// Open window
	surf_screen = SDL_SetVideoMode(SKIN_BACKGROUND_WIDTH,SKIN_BACKGROUND_HEIGHT,32,0);

	// Set the title bar text
	SDL_WM_SetCaption("Open Music Radio", "OMR");

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




	Mix_Music *music;
	music = Mix_LoadMUS("test_short.ogg");
	if(!music) {
		printf("Mix_LoadMUS(\"test_short.ogg\"): %s\n", Mix_GetError());
		return 3;
	}

	if(Mix_PlayMusic(music, 1) == -1) {
		printf("Mix_PlayMusic: %s\n", Mix_GetError());
		return 4;
	}


	draw();

	// let me know when the song is done.
	Mix_HookMusicFinished(music_finished);

	g_state = STATE_PLAYING;
	while(g_state == STATE_PLAYING) {
		SDL_Delay(60);
	}

	Mix_FreeMusic(music);

	draw();

	// delete this crap and make a loop or something
	music = Mix_LoadMUS("test_short2.ogg");
	if(!music) {
		printf("Mix_LoadMUS(\"test_short2.ogg\"): %s\n", Mix_GetError());
		return 3;
	}

	if(Mix_PlayMusic(music, 1) == -1) {
		printf("Mix_PlayMusic: %s\n", Mix_GetError());
		return 4;
	}

	g_state = STATE_PLAYING;
	while(g_state == STATE_PLAYING) {
		SDL_Delay(60);
	}

	return 0;
}
