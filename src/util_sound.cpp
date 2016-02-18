#include <stdio.h>
#include "main.hpp"

Sound sound;

Sound::Sound() : on(false),possible(true), updateCalled(false) {
}

//initializes sound
void Sound::initialize (void) {
#ifndef NOSOUND
    result = FMOD_System_Create(&fmodsystem);
    if (result == FMOD_OK) result = FMOD_System_Init(fmodsystem, 20, FMOD_INIT_NORMAL, 0);
    if (result != FMOD_OK) {
		possible = false;
		printf ("Warning : could not initialize sound system : %s",FMOD_ErrorString(result));
	}
#else
	possible=false;
#endif
}

//sets the actual playing sound's volume
void Sound::setVolume (float v) {
	if (possible && on) {
		FMOD_Channel_SetVolume(channel,v);
	}
}

//loads a soundfile
void Sound::load (const char * filename) {
	currentSound = (char *)filename;
	if (possible) {
		result = FMOD_System_CreateStream(fmodsystem, currentSound, FMOD_DEFAULT, 0, &snd);
		if (result != FMOD_OK) {
			printf ("Warning : could not load sound %s : %s",filename,FMOD_ErrorString(result));
			possible = false;
		}
	}
}

//frees the sound object
void Sound::unload (void) {
	if (possible) {
		result = FMOD_Sound_Release(snd);
	}
}

//plays a sound (no argument to leave pause as dafault)
void Sound::play() {
	if (possible) {
		result = FMOD_System_PlaySound(fmodsystem, FMOD_CHANNEL_FREE, snd, false, &channel);
		on=true;
		//FMOD_Channel_SetMode(channel,FMOD_LOOP_NORMAL);
	}
}

void Sound::playLoop() {
	if (possible) {
		play();
		FMOD_Channel_SetMode(channel,FMOD_LOOP_NORMAL);
	}
}

//pause or unpause the sound
void Sound::setPause (bool pause) {
	if (possible) {
		FMOD_Channel_SetPaused(channel,pause);
	}
}

void Sound::update() {
	if (possible && ! updateCalled ) {
		FMOD_System_Update(fmodsystem);
		updateCalled=true;
	}
}

void Sound::endFrame() {
	updateCalled=false;
}

//toggle pause on and off
void Sound::togglePause (void) {
   	if (possible) {
		FMOD_BOOL p;
		FMOD_Channel_GetPaused(channel,&p);
		FMOD_Channel_SetPaused(channel,!p);
	}
}
