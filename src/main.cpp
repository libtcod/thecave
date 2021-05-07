/*
* Copyright (c) 2009 Jice
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * The name of Jice may not be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Jice ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Jice BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <time.h>
#include <stdio.h>
#include "main.hpp"

TCODNoise noise1d(1);
TCODNoise noise2d(2);
TCODNoise noise3d(3);
TCODRandom *rng=NULL;
bool mouseControl=false;
bool newGame=false;
SaveGame saveGame;
UserPref userPref;
UmbraEngine engine("./data/cfg/umbra.txt",true,true);
TCODImage background("./data/img/background.png");

int main (int argc, char *argv[]) {
	// read main configuration file
	Config::init();
	ConditionType::init();
	TextGenerator::setGlobalFunction("NUMBER_TO_LETTER",new NumberToLetterFunc());

	// load user preferences (mouse control mode, ...)
	userPref.load();
	Powerup::init();

	tutorial = new Tutorial();
	threadPool = new ThreadPool();

    engine.setWindowTitle("The cave");

	// initialise random number generator
	if ( ! saveGame.load(PHASE_INIT) ) {
		newGame=true;
		saveGame.init();
		if ( Config::getBool("config.debug") && argc ==2 ) {
			// use a user-defined seed for RNG
			saveGame.seed=(uint32)atoi(argv[1]);
		}
	}
	if ( Config::getBool("config.debug") ) {
		printf ("Random seed : %d\n",saveGame.seed);
	}
	userPref.nbLaunches++;
	rng=new TCODRandom(saveGame.seed, TCOD_RNG_CMWC);

	// initialise the engine and register the game screens
	UmbraModule *forest=new ForestScreen();
	SCREEN_MAIN_MENU = engine.registerModule(new MainMenu(), SCREEN_NONE);
	char tmp[128];
	sprintf(tmp,"data/cfg/chapter%d.txg",saveGame.chapter+1);
//	engine.registerModule(new Game(), SCREEN_NONE);
	SCREEN_GAME_OVER = engine.registerModule(new EndScreen("You're dead...\n\nEven worse, Zeepoh will probably go with Alena now that you turned into a pile of ashes..."),SCREEN_MAIN_MENU);
	SCREEN_GAME_WON = engine.registerModule(new EndScreen("Congratulations, you won!\n\nMore important, you'll be able to show off with Alena tomorrow ! But this is another story..."),SCREEN_MAIN_MENU);
	SCREEN_CHAPTER_1 = engine.registerModule(forest, SCREEN_GAME_WON);
	SCREEN_STORY = engine.registerModule(new PaperScreen(tmp,"title","text",saveGame.chapter),SCREEN_CHAPTER_1);
	// set fading parameters
 	float fadeTime=Config::getFloat("config.display.fadeTime");
	((Screen *)engine.getModule(SCREEN_MAIN_MENU))->setFadeIn((int)(fadeTime*1000));
	((Screen *)engine.getModule(SCREEN_MAIN_MENU))->setFadeOut((int)(fadeTime*500));
	((Screen *)engine.getModule(SCREEN_STORY))->setFadeIn((int)(fadeTime*1000));
	((Screen *)engine.getModule(SCREEN_STORY))->setFadeOut((int)(fadeTime*1000));
	((Screen *)engine.getModule(SCREEN_CHAPTER_1))->setFadeIn((int)(fadeTime*1000));
	((Screen *)engine.getModule(SCREEN_CHAPTER_1))->setFadeOut((int)(fadeTime*1000));
	((Screen *)engine.getModule(SCREEN_GAME_OVER))->setFadeIn((int)(fadeTime*1000),TCODColor::darkRed);
	((Screen *)engine.getModule(SCREEN_GAME_OVER))->setFadeOut((int)(fadeTime*1000));
	((Screen *)engine.getModule(SCREEN_GAME_WON))->setFadeIn((int)(fadeTime*1000),TCODColor::white);
	((Screen *)engine.getModule(SCREEN_GAME_WON))->setFadeOut((int)(fadeTime*1000));

	engine.activateModule(SCREEN_MAIN_MENU);

	sound.initialize();
    if (engine.initialise(TCOD_RENDERER_SDL2)) {
		engine.run();
		saveGame.save();
		userPref.save();
		return 0;
	}

	return 1;
}
