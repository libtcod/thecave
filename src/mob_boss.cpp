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

Boss::Boss() {
	static char bossChar=Config::getChar("config.creatures.boss.ch");
	static TCODColor bossColor=Config::getColor("config.creatures.boss.fg");
	static int bossLife=Config::getInt("config.creatures.boss.life");
	static float pathDelay=Config::getFloat("config.creatures.pathDelay");
	static float treasureLightRange=Config::getFloat("config.display.treasureLightRange");
	static TCODColor treasureLightColor=Config::getColor("config.display.treasureLightColor");
	static float treasureIntensityDelay=Config::getFloat("config.display.treasureIntensityDelay");
	static const char *treasureIntensityPattern=strdup(Config::getString("config.display.treasureIntensityPattern"));

	treasureLight=new ExtendedLight();
	treasureLight->color=treasureLightColor;
	treasureLight->range=treasureLightRange*2;
	treasureLight->setup(treasureLightColor,treasureIntensityDelay,treasureIntensityPattern,NULL);
	GameEngine::instance->dungeon->addLight(treasureLight);

	ch=bossChar;
	col=bossColor;
	life=bossLife;
	seen=false;
	speed=1.0f;
	pathTimer=TCODRandom::getInstance()->getFloat(0.0f,pathDelay);
	summonTimer=0.0f;
	ignoreCreatures=false;
}

void Boss::setSeen() {
	static float bossSpeed=Config::getFloat("config.creatures.boss.speed");

	seen=true;
	((Game *)(GameEngine::instance))->bossSeen=true;
	AiDirector::instance->bossSeen();
	speed=bossSpeed;
}

// boss can't be stunned
void Boss::stun(float delay) {}

void Boss::takeDamage(float amount) {
	Creature::takeDamage(amount);
	if ( life <= 0.0f ) {
		// the boss dies
		((Game *)(GameEngine::instance))->bossIsDead=true;
		AiDirector::instance->bossDead();
		GameEngine::instance->dungeon->stairx=(int)x;
		GameEngine::instance->dungeon->stairy=(int)y;
	}
}

bool Boss::update(float elapsed) {
	static float pathDelay=Config::getFloat("config.creatures.pathDelay");
	static float summonTime=Config::getFloat("config.creatures.boss.summonTime");
	static int minionCount=Config::getInt("config.creatures.boss.minionCount");
	static float burnDamage=Config::getFloat("config.creatures.burnDamage");

	GameEngine *game=GameEngine::instance;
	treasureLight->setPos(x*2,y*2);
	if ( life <= 0 ) {
		return false;
	}
	if ( burn ) {
		takeDamage(burnDamage*elapsed);
	}
	pathTimer+=elapsed;
	if ( !seen ) {
		if ( game->dungeon->isCellInFov(x,y) && game->dungeon->getMemory(x,y) ) {
			// creature is seen by player
			setSeen();
		}
		return true;
	}
	summonTimer+=elapsed;
	if ( summonTimer > summonTime ) {
		// summon some minions to protect the boss
		summonTimer=0.0f;
		for (int i=0; i< minionCount; i++ ) {
			Minion *cr=new Minion();
			int crx=(int)x;
			int cry=(int)y;
			game->dungeon->getClosestWalkable(&crx,&cry,true,false);
			cr->setPos(crx,cry);
			cr->setSeen();
			game->dungeon->addCreature(cr);
		}
	}
	if ( pathTimer > pathDelay ){
		if ( ! path || path->isEmpty() ) {
			// stay away from player
			// while staying in lair
			int destx=game->dungeon->stairx+TCODRandom::getInstance()->getInt(-15,15);
			int desty=game->dungeon->stairy+TCODRandom::getInstance()->getInt(-15,15);
			destx=CLAMP(0,game->dungeon->size-1,destx);
			desty=CLAMP(0,game->dungeon->size-1,desty);
			game->dungeon->getClosestWalkable(&destx,&desty,true,true);
			if (! path) {
				path=new TCODPath(game->dungeon->size,game->dungeon->size,this,game);
			}
			path->compute((int)x,(int)y,destx,desty);
			pathTimer=0.0f;
		} else walk(elapsed);
	} else {
		walk(elapsed);
	}
	return life > 0;
}

float Boss::getWalkCost( int xFrom, int yFrom, int xTo, int yTo, void *userData ) const {
	static int secureDist=Config::getInt("config.creatures.boss.secureDist");
	static float secureCoef=Config::getFloat("config.creatures.boss.secureCoef");

	// the boss don't like to be near player
	GameEngine *game=GameEngine::instance;
	if ( !game->dungeon->map->isWalkable(xTo,yTo)) return 0.0f;
	if ( ignoreCreatures ) return 1.0f;
	float pdist=SQRDIST(game->player.x,game->player.y,xTo,yTo);
	if ( pdist < secureDist ) return 1.0f + secureCoef*(secureDist - pdist);
	return 1.0f;
}

