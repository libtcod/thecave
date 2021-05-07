#pragma once

#include <SDL_mixer.h>

class Sound {
private:
	Mix_Music *snd;
public:
    Sound ();
	void initialize ();

	//sound control
	void setVolume(float v);
	void load(const char * filename);
	void unload();
	void play();
	void playLoop();
	void update();
	void endFrame();

    //setters
    void setPause(bool pause);

    //toggles
    void togglePause(void);
};


