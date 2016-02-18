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

#include "main.hpp"

Minion::Minion() {
	static char minionChar=Config::getChar("config.creatures.minion.char");
	static TCODColor minionColor=Config::getColor("config.creatures.minion.color");
	static int minionLife=Config::getInt("config.creatures.minion.life");
	static float pathDelay=Config::getFloat("config.creatures.pathDelay");
	ch=minionChar;
	col=minionColor;
	life=minionLife;
	seen=false;
	speed=1.0f;
	pathTimer=TCODRandom::getInstance()->getFloat(0.0f,pathDelay);
}

void Minion::setSeen() {
	static float minionSpeed=Config::getFloat("config.creatures.minion.speed");
	seen=true;
	speed=minionSpeed;
}

bool Minion::update(float elapsed) {
	static float minionDamage=Config::getFloat("config.creatures.minion.damage");
	static float pathDelay=Config::getFloat("config.creatures.pathDelay");

	GameEngine *game=GameEngine::instance;
	if ( ! Creature::update(elapsed) ) return false;
	pathTimer+=elapsed;
	if ( !seen && game->dungeon->map->isInFov((int)x,(int)y) && game->dungeon->getMemory(x,y) ) {
		// creature is seen by player
		setSeen();
	}
	if ( burn || ! seen ) {
		randomWalk(elapsed);
	} else {
		// track player
		if (! path) {
			path=new TCODPath(game->dungeon->size,game->dungeon->size,this,game);
		} 
		if ( pathTimer > pathDelay ) {
			int dx,dy;
			path->getDestination(&dx,&dy);
			if (dx!=game->player.x || dy != game->player.y ) {
				// path is no longer valid (the player moved)
					path->compute((int)x,(int)y,(int)game->player.x,(int)game->player.y);
					pathTimer=0.0f;
			}
		}
		walk(elapsed);
	}
	float dx=ABS(game->player.x-x);
	float dy=ABS(game->player.y-y);
	if ( dx <= 1.0f && dy <= 1.0f ) {
		// at melee range. attack
		game->player.takeDamage(minionDamage*elapsed);
	}
	return true;
}

