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
#include <stdlib.h>
#include <stdio.h>

int g_done = 0;

// make a music finished function
void music_finished()
{
	printf("Music stopped.\n");
	g_done = 1;
}

int main(int argc, char **argv) {
	
	// Initialize SDL
	if(SDL_Init(/* FIXMESDL_INIT_VIDEO | */ SDL_INIT_AUDIO) != 0) {
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



	// let me know when the song is done.
	Mix_HookMusicFinished(music_finished);

	while(!g_done) {
		SDL_Delay(60);
	}

	Mix_FreeMusic(music);


	// delete this crap and make a loop or something
	g_done = 0;
	music = Mix_LoadMUS("test_short2.ogg");
	if(!music) {
		printf("Mix_LoadMUS(\"test_short2.ogg\"): %s\n", Mix_GetError());
		return 3;
	}

	if(Mix_PlayMusic(music, 1) == -1) {
		printf("Mix_PlayMusic: %s\n", Mix_GetError());
		return 4;
	}

	while(!g_done) {
		SDL_Delay(60);
	}

	return 0;
}
