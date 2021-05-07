#include <cstdio>
#include "main.hpp"

#include <SDL.h>
#include <SDL_mixer.h>

Sound sound;

Sound::Sound() {}

//initializes sound
void Sound::initialize (void) {
	if(SDL_InitSubSystem(SDL_INIT_AUDIO)==-1) {
		printf("Warning : could not initialize sound system : %s", SDL_GetError());
		return;
	}
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024)==-1) {
		printf("Warning : Mix_OpenAudio: %s\n", Mix_GetError());
		return;
	}
	if (Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG) {
		printf("Warning : failed to load OGG support : %s", Mix_GetError());
		return;
	}
}

//sets the actual playing sound's volume
void Sound::setVolume (float v) {
	if (v < 0) v = 0.0f;
	Mix_VolumeMusic((int)(v * MIX_MAX_VOLUME));
}

//loads a soundfile
void Sound::load (const char * filename) {
	unload();
	snd = Mix_LoadMUS(filename);
	if (!snd) {
		printf("Warning : could not load sound %s : %s", filename, Mix_GetError());
	}
}

//frees the sound object
void Sound::unload (void) {
	if (snd) {
		Mix_FreeMusic(snd);
		snd = NULL;
	}
}

//plays a sound (no argument to leave pause as dafault)
void Sound::play() {
	if (snd) {
		Mix_PlayMusic(snd, 1);
	}
}

void Sound::playLoop() {
	if (snd) {
		Mix_PlayMusic(snd, -1);
	}
}

//pause or unpause the sound
void Sound::setPause (bool pause) {
	Mix_PauseMusic();
}

void Sound::update() {}
void Sound::endFrame() {}

//toggle pause on and off
void Sound::togglePause (void) {
	if (Mix_PausedMusic()){
		Mix_ResumeMusic();
	} else {
		Mix_PauseMusic();
	}
}
