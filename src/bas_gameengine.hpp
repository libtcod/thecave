/*
* Copyright (c) 2010 Jice
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
#include "screen.hpp"

class Dialog;

class GameEngine : public Screen {
public :
	GameEngine();
	virtual ~GameEngine() {}
	static GameEngine *instance;

	void init();

    Logger log;

	TCODConsole *pauseScreen;
	inline bool isGamePaused() { return pauseOn; }
	void pauseGame();
	void resumeGame();
	void onFontChange();
	bool update(float elapsed, TCOD_key_t k,TCOD_mouse_t mouse);

	Player player;
	Dungeon *dungeon; // current dungeon map
	int xOffset,yOffset; // coordinate of console cell 0,0 in dungeon
	int mousex,mousey; // cell under mouse cursor
	TCODImage *ground; // visible part of the ground

	LightMap *lightMap; // store light reaching each cell
	Packer *packer;

	inline float getFog(int x,int y) { return lightMap->getFog(x,y); }
	void hitFlash(); // when player is hit

	// fireballs
	void addFireball(FireBall *fb) { fireballs.push(fb); }

	// inventory
	void openCloseInventory();
	void openCloseLoot(Item *toLoot);

	// stats
	struct {
		int nbCreatureKilled;
		int nbSpellStandard;
		int nbSpellBurst;
		int nbSpellIncandescence;
		int nbSteps;
		int nbEaten[NB_ITEMS];
	} stats;
	inline void startRipple(float x, float y) { startRipple((int)x,(int)y);}
	void startRipple(int dungeonx, int dungeony);
	// base utilities. to be moved elsewhere
	static TCODColor setSepia(const TCODColor &col, float coef);
	void displayProgress(float prog); // renders a progress bar and flush

	Inventory guiInventory;
	Inventory guiLoot;
	float aspectRatio; // font char width / font char height
protected :
	TCODList <FireBall *>fireballs;

	float pauseCoef;
	bool pauseOn;
	bool lookOn; // shit pressed
	bool firstFrame;
	Descriptor descriptor;
	StatusPanel statusPanel;
	RippleManager *rippleManager;
	float hitFlashAmount;

	void activate();
	void computeAspectRatio();

};

