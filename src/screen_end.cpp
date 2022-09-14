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
#include <SDL.h>
#include <libtcod/libtcod_int.h> // to get fullscreen margins. There should be an API for that...
#include "main.hpp"

EndScreen::EndScreen(const char *txt,float fadeLvl, bool stats)
	: Screen(fadeLvl),txt(strdup(txt)),noiseZ(0.0f),stats(stats) {
}

void EndScreen::onInitialise() {
	img=new TCODImage(CON_W*2,CON_H*2);
	onFontChange();
}

void EndScreen::onActivate() {
    // set keyboard mode to RELEASED only
    engine.setKeyboardMode( UMBRA_KEYBOARD_RELEASED );
    Screen::onActivate();
}

void EndScreen::render() {
	static TCODNoise noise(3);
	img->clear(TCODColor::black);
	for (int x=0; x < CON_W*2; x++) {
		for (int y=0; y < CON_H*2; y++) {
			float f[3]= { (float)(x)/CON_W,(float)(y)/(2*CON_H)+2*noiseZ,noiseZ};
			float v=0.5f*(1.0f+noise.getFbm(f, 5.0f, TCOD_NOISE_SIMPLEX)) * y/(CON_H*2);
			img->putPixel(x,y,TCODColor::red*v);
		}
	}
	img->blit2x(TCODConsole::root,0,0);
	//TCODConsole::root->setDefaultBackground(TCODColor::black);
	//TCODConsole::root->clear();
	TCODConsole::root->setDefaultForeground(TCODColor::lightRed);
	TCODConsole::root->printRectEx(CON_W/2,CON_H/2,70,0,TCOD_BKGND_NONE,TCOD_CENTER,txt);

	if ( GameEngine::instance && ((Game *)GameEngine::instance)->bossIsDead ) {
		int score = GameEngine::instance->stats.nbCreatureKilled - 10 * GameEngine::instance->stats.nbEaten[ITEM_HEALTH_POTION];
		TCODConsole::root->setDefaultForeground(TCODColor::white);
		TCODConsole::root->printEx(CON_W/2,CON_H/2+5,TCOD_BKGND_NONE,TCOD_CENTER,
			"Score : %5d",score);
	}

	if ( stats ) {
		TCODConsole::root->setDefaultForeground(TCODColor::red);
		TCODConsole::root->printEx(CON_W/2-5,CON_H-10,TCOD_BKGND_NONE,TCOD_CENTER,
			"   creatures killed :%4d\n"
			"      spells casted :%4d\n"
			"           fireball :%4d\n"
			"         fire burst :%4d\n"
			"      incandescence :%4d\n"
			"health potions used :%4d\n"
			"  distance traveled :%4d",
			GameEngine::instance->stats.nbCreatureKilled,
			GameEngine::instance->stats.nbSpellStandard
				+GameEngine::instance->stats.nbSpellBurst
				+GameEngine::instance->stats.nbSpellIncandescence,
			GameEngine::instance->stats.nbSpellStandard,
			GameEngine::instance->stats.nbSpellBurst,
			GameEngine::instance->stats.nbSpellIncandescence,
			GameEngine::instance->stats.nbEaten[ITEM_HEALTH_POTION],
			GameEngine::instance->stats.nbSteps
		);
	}
}

bool EndScreen::update(float elapsed, TCOD_key_t k,TCOD_mouse_t mouse) {
	if( fade == FADE_DOWN ) {
		if (fadeLvl<=0.0f) {
           	sound.setVolume(0.0f);
//           	sound.unload();
			return false;
		}
		else sound.setVolume(fadeLvl);
	}
	noiseZ+=elapsed*0.1f;
	if ( (k.vk != TCODK_NONE && k.vk != TCODK_ALT )|| mouse.lbutton_pressed || mouse.rbutton_pressed ) {
		fade=FADE_DOWN;
	}
	return true;
}

void EndScreen::onFontChange() {
	int charw,charh;
	TCODSystem::getCharSize(&charw, &charh);
	if ( charw == charh ) {
		// secondary font mappings
		TCODConsole::mapAsciiCodesToFont(128,28,0,5); // 128 - 155 : A-Z,. upper part
		TCODConsole::mapAsciiCodesToFont(233,28,0,6); // 233 - 260 : A-Z,. lower part
		TCODConsole::mapAsciiCodesToFont(233+28,28,0,7); // 261 - 288 : a-z-' upper part
		TCODConsole::mapAsciiCodesToFont(233+28*2,28,0,8); // 289 - 317 : a-z-' lower part
	}
}

// render text with secondary font (non square font on square console)
void EndScreen::renderText(int x,int y, int w, const char *txt) {
	int curx=x;
	while ( *txt ) {
		char c=*txt;
		int ascii=c, ascii2=0;
		if ( c == '\n' ) {
			curx=x;
			y+=2;
		} else {
			if ( c >= 'A' && c <= 'Z' ) {
				ascii=128 + (*txt -'A');
				ascii2=ascii+233-128;
			} else if ( c>= 'a' && c <= 'z' ) {
				ascii = 233+28 + (*txt - 'a');
				ascii2 = ascii+28;
			} else if (c == ',') {
				ascii = 128+26;
				ascii2=ascii+233-128;
			} else if (c == '.') {
				ascii = 128+27;
				ascii2=ascii+233-128;
			} else if (c == '-') {
				ascii = 233+28+26;
				ascii2 = ascii+28;
			} else if (c == '\'') {
				ascii = 233+28+27;
				ascii2 = ascii+28;
			}
			TCODConsole::root->putChar(curx,y,ascii);
			if ( ascii2 ) TCODConsole::root->putChar(curx,y+1,ascii2);
			curx++;
			if ( curx >= CON_W || curx >= x+w ) {
				while (*txt != ' ') {
					curx--;
					TCODConsole::root->setChar(curx,y,' ');
					TCODConsole::root->setChar(curx,y+1,' ');
					txt--;
				}
				curx = x; y+=2;
			}
		}
		txt ++;
	}
}

PaperScreen::PaperScreen(const char *txgfile, const char  *titlegen, const char *textgen, int chapter)
	: EndScreen("",0.0f,false),chapter(chapter) {
	title=NULL;
	TCODRandom tmpRng(saveGame.seed);
	TextGenerator txtgen(txgfile,&tmpRng);
	txtgen.setLocalFunction("RANDOM_INT",new RandomIntFunc(&tmpRng));
	txtgen.setLocalFunction("RANDOM_NAME",new RandomNameFunc(&tmpRng));
	title=(const char *)strdup(txtgen.generate(titlegen,"${OUTPUT}"));
	txt=(const char *)strdup(txtgen.generate(textgen,"${OUTPUT}"));
	pix=NULL;
}

void PaperScreen::onInitialise() {
	tcodpix=loadChapterPicture(false);
	onFontChange();
}

void PaperScreen::onFontChange() {
	EndScreen::onFontChange();
	int pixw,pixh;
	tcodpix->getSize(&pixw,&pixh);
	SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormat(0, pixw, pixh, 24, SDL_PIXELFORMAT_RGB24);

	int charw,charh;
	float ratio=(float)(pixh)/pixw;
	TCODSystem::getCharSize(&charw, &charh);
	int ridx=surf->format->Rshift/8;
	int gidx=surf->format->Gshift/8;
	int bidx=surf->format->Bshift/8;
	for (int y=0; y < pixh; y++) {
		for (int x=0; x <pixw;x++) {
			TCODColor col=tcodpix->getPixel(x,y);
			Uint8 *p = (Uint8 *)surf->pixels + x * surf->format->BytesPerPixel + y * surf->pitch;
			p[ridx]=col.r;
			p[gidx]=col.g;
			p[bidx]=col.b;
		}
	}
	SDL_Surface *surf2 = SDL_CreateRGBSurfaceWithFormat(
		0, CON_W*charw/2, (int)(CON_W*charw/2*ratio), 24, SDL_PIXELFORMAT_RGB24);
	SDL_BlitScaled(surf, NULL, surf2, NULL);
	SDL_FreeSurface(surf);
	if ( pix ) SDL_FreeSurface(pix);
	pix=surf2;
}

void PaperScreen::render() {
	static TCODImage *paper=NULL;
	if (!paper) {
		paper=new TCODImage("data/img/paper.png");
	}
	paper->blit2x(TCODConsole::root,0,0);
	TCODConsole::root->setDefaultForeground(TCODColor::darkestRed);
	int charw,charh;
	TCODSystem::getCharSize(&charw, &charh);
	if ( charw != charh ) {
		TCODConsole::root->printEx(CON_W/2,3,TCOD_BKGND_NONE,TCOD_CENTER,title);
		TCODConsole::root->printRect(13,7,50,0,txt);
	} else {
		// wall of text looks ugly with square font.
		// use secondary, non square font
		renderText(CON_W/3,1,CON_W,title);
		renderText(13,5,50,txt);
	}
}

void PaperScreen::render(void *sdlSurface) {
	int charw,charh;
	TCODSystem::getCharSize(&charw, &charh);
	int offx=0,offy=0;

	if ( TCODConsole::isFullscreen()) TCODSystem::getFullscreenOffsets(&offx,&offy);
	SDL_Rect dst = {offx + CON_W*charw/4-18,
		offy + charh*CON_H/2-charh/2,0,0};
	SDL_SetSurfaceAlphaMod(pix, TCODConsole::getFade());
	SDL_BlitSurface(pix, NULL, (SDL_Surface*)sdlSurface, &dst);
}

bool PaperScreen::update(float elapsed, TCOD_key_t k,TCOD_mouse_t mouse) {
	return EndScreen::update(elapsed,k,mouse);
}

void PaperScreen::onActivate() {
	saveGame.chapter=chapter;
	TCODSystem::registerSDLRenderer(this);
}

void PaperScreen::onDeactivate() {
	TCODSystem::registerSDLRenderer(NULL);
	TCODConsole::root->setDirty(0,0,CON_W,CON_H);
}
