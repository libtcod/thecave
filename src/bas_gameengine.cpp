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
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include "main.hpp"

GameEngine *GameEngine::instance = NULL;

GameEngine::GameEngine() : Screen(0), pauseOn(false), lookOn(false) {
	instance=this;
	lightMap=new LightMap(CON_W*2,CON_H*2);
	ground=new TCODImage(CON_W*2,CON_H*2);
	packer = new Packer(0,0,CON_W,CON_H);
	mousex=0;mousey=0;xOffset=0;yOffset=0;
	pauseCoef=0.0f;
	rippleManager=NULL;
	firstFrame=true;
}

void GameEngine::activate() {
    Screen::activate();
	hitFlashAmount=0.0f;
	firstFrame=true;
	computeAspectRatio();
}

void GameEngine::onFontChange() {
	computeAspectRatio();
}

void GameEngine::computeAspectRatio() {
	int charw,charh;
	TCODSystem::getCharSize(&charw, &charh);
	aspectRatio = (float)(charw)/charh;
}

void GameEngine::hitFlash() {
	static float hitFlashDelay=Config::getFloat("config.display.hitFlashDelay");
	hitFlashAmount=hitFlashDelay;
}

TCODColor GameEngine::setSepia(const TCODColor &col, float coef) {
	float h,s,v;
	col.getHSV(&h,&s,&v);
	TCODColor ret = col;
	ret.setHSV(28.0f,0.25f,v*0.7f);
	return TCODColor::lerp(col,ret,coef);
}


bool GameEngine::update(float elapsed, TCOD_key_t k,TCOD_mouse_t mouse) {
	static float hitFlashDelay=Config::getFloat("config.display.hitFlashDelay");
	static TCODColor flashColor=Config::getColor("config.display.flashColor");
	packer->clear();
	if ( fade == FADE_OFF ) {
		if ( hitFlashAmount > 0.0f ) {
			hitFlashAmount-=elapsed;
			if ( hitFlashAmount > 0.0f ) {
				int flashLvl=(int)(255-128*hitFlashAmount/hitFlashDelay);
				TCODConsole::setFade(flashLvl,flashColor);
			} else {
				TCODConsole::setFade(255,flashColor);
			}
		}
	}
	if ( pauseOn && pauseCoef != 1.0f ) {
		pauseCoef += elapsed;
		if ( pauseCoef > 1.0f ) pauseCoef=1.0f;
	} else if ( ! pauseOn && pauseCoef > 0.0f ) {
		pauseCoef -= elapsed;
		if ( pauseCoef < 0.0f ) pauseCoef=0.0f;
	}
	return true;
}

void GameEngine::init() {
	pauseScreen=NULL;
	GameEngine::instance = this;
    engine.setKeyboardMode( UMBRA_KEYBOARD_PRESSED_RELEASED );
	player.init();
}

void GameEngine::pauseGame() {
	pauseOn=true;
	pauseCoef=0.0f;
}

void GameEngine::resumeGame() {
	pauseOn=false;
	pauseScreen=NULL;
}


void GameEngine::startRipple(int dungeonx, int dungeony) {
	rippleManager->startRipple(dungeonx,dungeony);
}

void GameEngine::openCloseInventory() {
	if ( guiInventory.isActive() ) {
		engine.deactivateModule(&guiInventory);
	} else {
		guiInventory.initialize(&player);
		engine.activateModule(&guiInventory);
	}
}

void GameEngine::openCloseLoot(Item *toLoot) {
	if ( !guiLoot.isActive() ) {
		guiLoot.initialize(toLoot);
		engine.activateModule(&guiLoot);
		guiInventory.initialize(&player);
		engine.activateModule(&guiInventory);
	} else {
		engine.deactivateModule(&guiLoot);
	}
}

void GameEngine::displayProgress(float prog) {
//printf ("==> %g \n",prog);
	int l=(int)(CON_W/2*prog);
	if ( l > 0 ) {
		TCODConsole::root->setDefaultBackground(TCODColor::red);
		TCODConsole::root->rect(CON_W/4,CON_H/2-1,l,2,false,TCOD_BKGND_SET);
	}
	if (l < CON_W/2 ) {
		TCODConsole::root->setDefaultBackground(TCODColor::darkestRed);
		TCODConsole::root->rect(CON_W/4+l,CON_H/2-1,CON_W/2-l,2,false,TCOD_BKGND_SET);
	}
	TCODConsole::root->flush();
	TCODConsole::checkForKeypress();
}
