
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

#include "main.hpp"

bool Dialog::update() {
    static float timeScale=Config::getFloat("config.gameplay.timeScale");
    float elapsed = TCODSystem::getLastFrameLength() * timeScale;
    if ( isMaximizable() && minimiseButton.mouseDown && ! waitRelease) {
  		if (isMinimized) setMaximized();
  		else setMinimized();
  		waitRelease=true;
    } else if (! minimiseButton.mouseDown) waitRelease=false;
    if ( isClosable() && closeButton.mouseDown ) {
    	return false;
	}
	if (!UmbraWidget::update()) return false;
    internalUpdate();
    return update(elapsed, key,ms);
}

void Dialog::activate() {
	UmbraWidget::activate();
	if ( isModal() ) GameEngine::instance->pauseGame();
}

void Dialog::deactivate() {
	UmbraWidget::deactivate();
	if ( isModal() ) GameEngine::instance->resumeGame();
}

void Dialog::setMaximized() {
	// if draggable, save position
	if ( isDraggable()) {
		minimizedRect.x=rect.x;
		minimizedRect.y=rect.y;
	}
	rect=maximizedRect;
	minimiseButton.set(rect.w-2,0);
	isMinimized=false;
	canDrag=false;
	if (GameEngine::instance && ! GameEngine::instance->isGamePaused()) GameEngine::instance->pauseGame();
}

void Dialog::setMinimized() {
	rect=minimizedRect;
	minimiseButton.set(rect.w-2,0);
	isMinimized=true;
	canDrag=isDraggable() || isMultiPos();
	if ( isDraggable() || isMultiPos()) {
		setDragZone(0,0,minimizedRect.w-3,1);
	}
	if (GameEngine::instance && GameEngine::instance->isGamePaused() ) GameEngine::instance->resumeGame();
}

void Dialog::renderFrame(float alpha, const char *title) {
    con->setDefaultBackground(guiBackground);
    con->setDefaultForeground(guiText);
    con->rect(0,0,rect.w,1,true,TCOD_BKGND_SET);
    con->printEx(rect.w/2,0,TCOD_BKGND_NONE, TCOD_CENTER, title);
    if ( isMinimized && (isDraggable() || isMultiPos()) ) {
    	// draw dragging handle
    	int l=strlen(title);
    	for (int x=0; x < rect.w/2-l/2-1; x++ ) con->putChar(x,0,TCOD_CHAR_BLOCK2,TCOD_BKGND_NONE);
    	for (int x=rect.w/2+l/2+1 + (l&1); x < rect.w; x++ ) con->putChar(x,0,TCOD_CHAR_BLOCK2,TCOD_BKGND_NONE);
    }
    if ( isClosable() ) {
		con->setDefaultForeground(closeButton.mouseHover ? guiHighlightedText : guiText);
		con->putChar(closeButton.x,closeButton.y,'x',TCOD_BKGND_NONE);
    }
    if (isMaximizable()) {
    	con->setDefaultForeground(minimiseButton.mouseHover ? guiHighlightedText : guiText);
    	con->putChar(minimiseButton.x,minimiseButton.y,
			isMinimized?TCOD_CHAR_ARROW2_N:TCOD_CHAR_ARROW2_S,TCOD_BKGND_NONE);
	}

    if ( alpha < 1.0f ) blitSemiTransparent(con,0,0,rect.w,1,TCODConsole::root,rect.x,rect.y, alpha,alpha);
    else TCODConsole::blit(con,0,0,rect.w,1,TCODConsole::root,rect.x,rect.y);
}

void Dialog::internalUpdate() {
    if ( isModal() && !GameEngine::instance->isGamePaused() ) {
			GameEngine::instance->pauseGame();
    }
}

void MultiPosDialog::renderFrame(float alpha, const char *title) {
    if (isMultiPos() && isDragging && isMinimized ) {
    	renderTargetFrame();
    }
   	Dialog::renderFrame(alpha,title);
}

void MultiPosDialog::internalUpdate() {
    Dialog::internalUpdate();
    if (isMultiPos() && isDragging && isMinimized ) {
    	targetx=rect.x;
    	targety=rect.y;
    	if (! isDraggable() ) {
	    	// cancel actual dragging
	    	rect.x=minimizedRect.x;
	    	rect.y=minimizedRect.y;
	    }
    	// find nearest position
    	int dist=10000,best=0;
    	for (int i=0; i < possiblePos.size(); i++ ) {
    		UmbraRect curRect=*possiblePos.get(i);
    		int curDist=(curRect.x-targetx)*(curRect.x-targetx)+(curRect.y-targety)*(curRect.y-targety);
    		if ( curDist < dist ) {
    			dist=curDist;
    			best=i;
    		}
    	}
    	targetx=possiblePos.get(best)->x;
    	targety=possiblePos.get(best)->y;
    }
}

void MultiPosDialog::endDragging(int mousex,int mousey) {
	minimizedRect.x=targetx;
	minimizedRect.y=targety;
	setPos(targetx,targety);
	// alter the dialogs of the set
	int posnum=0;
	int bestdist=100000;
	int bestpos=0;
	// find the best set position
   	for (posnum=0; posnum < possiblePos.size(); posnum++ ) {
   		UmbraRect curRect=*possiblePos.get(posnum);
   		if ( rect.x == curRect.x && rect.y == curRect.y ) {
   			int dist=0;
   			for ( MultiPosDialog **it=set.begin();it!=set.end(); it++) {
   				if ( *it != this ) {
   					UmbraRect *prect = (*it)->possiblePos.get(posnum);
   					int dx = prect->x - (*it)->rect.x;
   					int dy = prect->y - (*it)->rect.y;
   					dist += dx*dx+dy*dy;
				}
			}
			if ( dist < bestdist ) {
				bestdist=dist;
				bestpos=posnum;
			}
		}
   	}
	for ( MultiPosDialog **it=set.begin();it!=set.end(); it++) {
		if ( *it != this ) {
			UmbraRect *bestp=(*it)->possiblePos.get(bestpos);
			(*it)->rect = (*it)->minimizedRect = *bestp;
			(*it)->setPos(bestp->x,bestp->y);
		}
	}
}

void MultiPosDialog::renderTargetFrame() {
	TCODConsole::root->setDefaultForeground(guiText);
	TCODConsole::root->printFrame(targetx,targety,rect.w,rect.h,false,TCOD_BKGND_NONE,NULL);
}
