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
#include <stdio.h>
#include "main.hpp"

#define NB_LINES 10
#define HISTORY_SIZE 256
#define LOG_WIDTH (CON_W-12)

static TCODColor sevColor[NB_SEVERITIES];

Logger::Logger() {
	con = new TCODConsole(CON_W-12,CON_H);
	con->setDefaultBackground(guiBackground);
	con->clear();
	sevColor[DEBUG]=Config::getColor("config.display.debugColor");
	sevColor[INFO]=Config::getColor("config.display.infoColor");
	sevColor[WARN]=Config::getColor("config.display.warnColor");
	sevColor[CRITICAL]=Config::getColor("config.display.criticalColor");
	nbActive=0;
	flags=DIALOG_MAXIMIZABLE|DIALOG_MULTIPOS|DIALOG_DRAGGABLE;
	focus=drag=false;
	offset=0;
	possiblePos.push(new UmbraRect(0,CON_H-NB_LINES-1,LOG_WIDTH,NB_LINES+1));
	possiblePos.push(new UmbraRect(0,0,LOG_WIDTH,NB_LINES+1));
	possiblePos.push(new UmbraRect(12,CON_H-NB_LINES-1,LOG_WIDTH,NB_LINES+1));

	possiblePos.push(new UmbraRect(0,CON_H-NB_LINES-1,LOG_WIDTH,NB_LINES+1));
	possiblePos.push(new UmbraRect(0,0,LOG_WIDTH,NB_LINES+1));
	possiblePos.push(new UmbraRect(12,0,LOG_WIDTH,NB_LINES+1));

	possiblePos.push(new UmbraRect(12,CON_H-NB_LINES-1,LOG_WIDTH,NB_LINES+1));
	possiblePos.push(new UmbraRect(12,0,LOG_WIDTH,NB_LINES+1));
	possiblePos.push(new UmbraRect(0,CON_H-NB_LINES-1,LOG_WIDTH,NB_LINES+1));

	possiblePos.push(new UmbraRect(12,CON_H-NB_LINES-1,LOG_WIDTH,NB_LINES+1));
	possiblePos.push(new UmbraRect(12,0,LOG_WIDTH,NB_LINES+1));
	possiblePos.push(new UmbraRect(0,0,LOG_WIDTH,NB_LINES+1));
	minimizedRect = *possiblePos.get(0);
	minimizedRect.x=userPref.logx;
	minimizedRect.y=userPref.logy;
	maximizedRect=minimizedRect;
	maximizedRect.y=0;
	maximizedRect.w=LOG_WIDTH;
	maximizedRect.h=CON_H;
	setMinimized();
	titleBarAlpha=0.0f;
	lookOn=false;
}


void Logger::render() {
	// render messages
	Message **it=NULL;
	int y=0;
	if ( !isMinimized ) {
		int nbMessages = messages.size();
		int nbDisplayed = MIN(CON_H-1,nbMessages-offset);
		if ( nbMessages > 0 ) {
			y = CON_H-nbDisplayed;
			blitTransparent(con,0,0,maximizedRect.w,maximizedRect.h,TCODConsole::root,maximizedRect.x,maximizedRect.y);
			// scrollbar
			if ( nbDisplayed < nbMessages ) {
				int firstDisplayed = nbMessages - offset - nbDisplayed;
				int start = ((CON_H-1) * firstDisplayed)/nbMessages;
				int end = (CON_H * (firstDisplayed + nbDisplayed))/nbMessages;
				end=MIN(CON_H-1,end);
				if ( start > 0 ) darken(maximizedRect.x+LOG_WIDTH-1,0,1,start,0.5f);
				if ( end+1 < CON_H ) darken(maximizedRect.x+LOG_WIDTH-1,end+1,1,CON_H-1-end,0.5f);
				lighten(maximizedRect.x+LOG_WIDTH-2,start,2,end-start+1,focus || drag ? 0.5f : 0.25f);
			}
		}
	} else if ( titleBarAlpha > 0.0f ) {
		blitSemiTransparent(con,0,0,rect.w,rect.h,TCODConsole::root,rect.x,rect.y,titleBarAlpha,titleBarAlpha);
	} else if ( nbActive > 0 ) {
	    int ry=0, dy=0,count=0;
	    if (rect.y == 0 ) {
	        y=0;
	        count=MIN(rect.h,nbActive);
	        it=messages.end()-1;
	        dy=-1;
	        ry=0;
	    } else {
            y = MAX(0,rect.h-nbActive);
            ry = rect.y+y;
            count=rect.h-y;
            dy=1;
            it=messages.end()-count;
	    }
		for (; count > 0; it+=dy,y++,ry++, count--) {
			float timer=(*it)->timer;
			if ( timer >= 0.5f ) timer=1.0f;
			else timer = timer*2;
			blitSemiTransparent(con,0,y,LOG_WIDTH,1,TCODConsole::root,rect.x,ry,timer,timer);
		}
	}
	if ( titleBarAlpha > 0.0f ) renderFrame(titleBarAlpha,"Message log");
}

bool Logger::update(float elapsed, TCOD_key_t &k, TCOD_mouse_t &mouse) {
	static float messageLife=Config::getFloat("config.display.messageLife");

	// update messages
	TCODList<Message *> messagesToRemove;
	while ( messages.size() > HISTORY_SIZE ) {
		messagesToRemove.push(messages.get(0));
		messages.remove(messages.begin());
	}

	if (! GameEngine::instance->isGamePaused()) {
		for (Message **it = messages.end()-nbActive; it!= messages.end(); it++) {
			(*it)->timer-= elapsed/messageLife;
			if ((*it)->timer < 0.0f) {
				nbActive--;
			}
		}
	}
	messagesToRemove.clearAndDelete();
	if ( !k.pressed && (toupper(k.c)=='M' || (!isMinimized && k.c==' ') 
		|| (!isMinimized && k.vk == TCODK_ESCAPE) ) && messages.size() > 0 ) {
		if (isMinimized) setMaximized();
		else setMinimized();
	}
	if ( ! isMinimized ) {
		if ( !GameEngine::instance->isGamePaused() ) {
			GameEngine::instance->pauseGame();
			focus=drag=false;
		}
	}
	if ( k.vk == TCODK_ALT || k.lalt ) lookOn=k.pressed;

	if ( !isMinimized ) {
		int nbMessages = messages.size();
		int nbDisplayed = MIN(CON_H-1,nbMessages-offset);
		int firstDisplayed = nbMessages - offset - nbDisplayed;
		// scrollbar focus
		if ( nbDisplayed < nbMessages ) {
			bool up=false,down=false,left=false,right=false;
			Player::getMoveKey(k,&up,&down,&left,&right);
			if ( ( up || left ) && offset < messages.size() - CON_H) {
				offset += 5;
				offset = MIN(messages.size()-CON_H,offset);
			} else if ( (down || right ) && offset > 0 ) {
				offset -= MIN(5,offset);
			}
			int start = ((CON_H-1) * firstDisplayed)/nbMessages;
			int end = ((CON_H-1) * (firstDisplayed + nbDisplayed))/nbMessages;
			focus = ( mouse.cx >= maximizedRect.x+LOG_WIDTH-2 && mouse.cx < maximizedRect.x+LOG_WIDTH && mouse.cy >= start && mouse.cy <= end );
			if ( ! drag && focus && mouse.lbutton ) {
				drag=true;
				startDrag=mouse.cy;
				startOffset=start;
			} else if (drag && mouse.lbutton) {
				int delta = mouse.cy - startDrag;
				int newStart=startOffset + delta;
				int newFirst = newStart * nbMessages / (CON_H-1);
				newFirst = MAX(0,newFirst);
				newFirst = MIN(nbMessages-(CON_H-1),newFirst);
				offset = nbMessages - nbDisplayed - newFirst;
			} else if (drag && ! mouse.lbutton ) {
				drag=false;
			}
		}
	}
	con->clear();

	if ( !isMinimized || (rect.mouseHover && ! lookOn 
		&& ! GameEngine::instance->guiLoot.isActive()
		&& ! GameEngine::instance->guiInventory.isActive()
		) ) {
		titleBarAlpha+=elapsed;
		titleBarAlpha=MIN(1.0f,titleBarAlpha);
	} else if ( !isDragging ) {
		titleBarAlpha-=elapsed;
		titleBarAlpha=MAX(0.0f,titleBarAlpha);
	}

	Message **it=NULL;
	int y=0, count=0,dy=0;

	if ( !isMinimized ) {
		y = MAX(1,rect.h-messages.size()-offset);
        count=rect.h-y;
	} else if ( titleBarAlpha > 0.0f ) {
	    if ( rect.y == 0 ) {
	        y = 1;
	        count=MIN(rect.h,messages.size());
	    } else {
            y = MAX(1,rect.h-messages.size());
           	count=rect.h-y;
	    }
	} else {
	    if ( rect.y == 0 ) {
	        y = 0;
	        count=MIN(rect.h,nbActive);
	    } else {
            y = MAX(0,rect.h-nbActive);
            count=rect.h-y;
	    }
	}
	if ( isMinimized && rect.y==0 ) {
	    it = messages.end()-1;
	    dy=-1;
	} else {
        it=messages.end()-count;
        if ( ! isMinimized ) it -= offset;
        dy=1;
	}
	for (; count > 0; it+=dy,y++,count--) {
		con->setDefaultForeground(sevColor[(*it)->severity]);
		con->printEx(rect.w/2,y,TCOD_BKGND_NONE,TCOD_CENTER,(*it)->txt);
	}
	return true;
}

void Logger::addMessage(MessageSeverity severity, const char *fmt, va_list ap) {
	char tmp[256];
	vsprintf(tmp,fmt,ap);
	Message *msg=new Message();
	msg->timer=1.0f;
	msg->txt=strdup(tmp);
	msg->severity=severity;
	messages.push(msg);
	nbActive++;
}

void Logger::debug(const char *fmt, ...) {
	va_list ap;
	va_start(ap,fmt);
	addMessage(DEBUG,fmt,ap);
	va_end(ap);
}

void Logger::info(const char *fmt, ...) {
	va_list ap;
	va_start(ap,fmt);
	addMessage(INFO,fmt,ap);
	va_end(ap);
}

void Logger::warn(const char *fmt, ...) {
	va_list ap;
	va_start(ap,fmt);
	addMessage(WARN,fmt,ap);
	va_end(ap);
}

void Logger::critical(const char *fmt, ...) {
	va_list ap;
	va_start(ap,fmt);
	addMessage(CRITICAL,fmt,ap);
	va_end(ap);
}

#define HIST_CHUNK_VERSION 2
void Logger::saveData(uint32 chunkId, TCODZip *zip) {
	saveGame.saveChunk(HIST_CHUNK_ID,HIST_CHUNK_VERSION);
	zip->putChar(isMinimized ? 1 : 0);
	zip->putInt(messages.size());
	zip->putInt(offset);
	for (int i=0; i < messages.size(); i++) {
		Message *msg=messages.get(i);
		zip->putFloat(msg->timer);
		zip->putString(msg->txt);
		zip->putInt(msg->severity);
	}
}

bool Logger::loadData(uint32 chunkId, uint32 chunkVersion, TCODZip *zip) {
	if ( chunkVersion != HIST_CHUNK_VERSION ) return false;
	isMinimized=zip->getChar() == 1;
	int nbMessages=zip->getInt();
	offset=zip->getInt();
	while ( nbMessages > 0 ) {
		nbMessages--;
		Message *msg=new Message();
		msg->timer=zip->getFloat();
		msg->txt=strdup(zip->getString());
		msg->severity=(MessageSeverity)zip->getInt();
		messages.push(msg);
	}
	if ( isMinimized ) setMinimized();
	else setMaximized();
	return true;
}

void Logger::setPos(int x,int y) {
	MultiPosDialog::setPos(x,y);
	userPref.logx=x;
	userPref.logy=y;
	maximizedRect=minimizedRect;
	maximizedRect.y=0;
	maximizedRect.h=CON_H;
}
