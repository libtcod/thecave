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
TCODParser Config::parser;

void Config::init() {
	TCODParserStruct *config=parser.newStructure("config");
	config->addProperty("debug",TCOD_TYPE_BOOL,false);
	config->addProperty("multithread",TCOD_TYPE_BOOL,false);
	config->addProperty("threadPoolSize",TCOD_TYPE_INT,false);

	TCODParserStruct *display=parser.newStructure("display");
	display->addProperty("wallColor",TCOD_TYPE_COLOR,true);
	display->addProperty("groundColor",TCOD_TYPE_COLOR,true);
	display->addProperty("memoryWallColor",TCOD_TYPE_COLOR,true);
	display->addProperty("playerLightRange",TCOD_TYPE_INT,true);
	display->addProperty("playerLightColor",TCOD_TYPE_COLOR,true);
	display->addProperty("playerLightColorEnd",TCOD_TYPE_COLOR,true);
	display->addProperty("messageLife",TCOD_TYPE_FLOAT,true);
	display->addProperty("debugColor",TCOD_TYPE_COLOR,true);
	display->addProperty("infoColor",TCOD_TYPE_COLOR,true);
	display->addProperty("warnColor",TCOD_TYPE_COLOR,true);
	display->addProperty("criticalColor",TCOD_TYPE_COLOR,true);
	display->addProperty("fadeTime",TCOD_TYPE_FLOAT,true);
	display->addProperty("fireSpeed",TCOD_TYPE_FLOAT,true);
	display->addProperty("hitFlashDelay",TCOD_TYPE_FLOAT,true);
	display->addProperty("flashColor",TCOD_TYPE_COLOR,true);
	display->addProperty("corpseColor",TCOD_TYPE_COLOR,true);
	display->addProperty("treasureLightColor",TCOD_TYPE_COLOR,true);
	display->addProperty("treasureLightRange",TCOD_TYPE_FLOAT,true);
	display->addProperty("treasureIntensityDelay",TCOD_TYPE_FLOAT,true);
	display->addProperty("treasureIntensityPattern",TCOD_TYPE_STRING,true);
	display->addProperty("finalExplosionTime",TCOD_TYPE_FLOAT,true);
	config->addStructure(display);

	TCODParserStruct *fog=parser.newStructure("fog");
	fog->addProperty("maxLevel",TCOD_TYPE_FLOAT,true);
	fog->addProperty("scale",TCOD_TYPE_FLOAT,true);
	fog->addProperty("octaves",TCOD_TYPE_FLOAT,true);
	fog->addProperty("speed",TCOD_TYPE_FLOAT,true);
	fog->addProperty("color",TCOD_TYPE_COLOR,true);
	config->addStructure(fog);

	TCODParserStruct *spells=parser.newStructure("spells");

		TCODParserStruct *fireball=parser.newStructure("fireball");
		fireball->addProperty("lightColor",TCOD_TYPE_COLOR,true);
		fireball->addProperty("trailLength",TCOD_TYPE_INT,true);
		fireball->addProperty("speed",TCOD_TYPE_FLOAT,true);
		fireball->addProperty("sparkLife",TCOD_TYPE_FLOAT,true);
		fireball->addProperty("sparkleLife",TCOD_TYPE_FLOAT,true);
		fireball->addProperty("standardLife",TCOD_TYPE_FLOAT,true);
		fireball->addProperty("sparkleSpeed",TCOD_TYPE_FLOAT,true);
		fireball->addProperty("baseRange",TCOD_TYPE_FLOAT,true);
		fireball->addProperty("baseDamage",TCOD_TYPE_FLOAT,true);
		fireball->addProperty("stunDelay",TCOD_TYPE_FLOAT,true);
		spells->addStructure(fireball);

	config->addStructure(spells);

	TCODParserStruct *gameplay=parser.newStructure("gameplay");
	gameplay->addProperty("timeScale",TCOD_TYPE_FLOAT,true);
	gameplay->addProperty("nbLevels",TCOD_TYPE_INT,true);
	gameplay->addProperty("dungeonMinSize",TCOD_TYPE_INT,true);
	gameplay->addProperty("dungeonMaxSize",TCOD_TYPE_INT,true);
	gameplay->addProperty("penumbraLevel",TCOD_TYPE_INT,true);
	gameplay->addProperty("darknessLevel",TCOD_TYPE_INT,true);
	config->addStructure(gameplay);

	TCODParserStruct *aidirector=parser.newStructure("aidirector");
	aidirector->addProperty("waveLength",TCOD_TYPE_FLOAT,true);
	aidirector->addProperty("lowLevel",TCOD_TYPE_FLOAT,true);
	aidirector->addProperty("medLevel",TCOD_TYPE_FLOAT,true);
	aidirector->addProperty("medRate",TCOD_TYPE_FLOAT,true);
	aidirector->addProperty("highRate",TCOD_TYPE_FLOAT,true);
	aidirector->addProperty("maxCreatures",TCOD_TYPE_INT,true);
	aidirector->addProperty("spawnSourceRange",TCOD_TYPE_INT,true);
	aidirector->addProperty("hordeDelay",TCOD_TYPE_INT,true);
	aidirector->addProperty("distReplace",TCOD_TYPE_INT,true);
	aidirector->addProperty("itemKillCount",TCOD_TYPE_INT,true);
	config->addStructure(aidirector);

	TCODParserStruct *creatures=parser.newStructure("creatures");
	creatures->addProperty("burnDamage",TCOD_TYPE_FLOAT,true);
	creatures->addProperty("pathDelay",TCOD_TYPE_FLOAT,true);

		TCODParserStruct *minion=parser.newStructure("minion");
		minion->addProperty("char",TCOD_TYPE_CHAR,true);
		minion->addProperty("color",TCOD_TYPE_COLOR,true);
		minion->addProperty("life",TCOD_TYPE_INT,true);
		minion->addProperty("speed",TCOD_TYPE_FLOAT,true);
		minion->addProperty("damage",TCOD_TYPE_FLOAT,true);
		creatures->addStructure(minion);

		TCODParserStruct *boss=parser.newStructure("boss");
		boss->addProperty("char",TCOD_TYPE_CHAR,true);
		boss->addProperty("color",TCOD_TYPE_COLOR,true);
		boss->addProperty("life",TCOD_TYPE_INT,true);
		boss->addProperty("speed",TCOD_TYPE_FLOAT,true);
		boss->addProperty("secureDist",TCOD_TYPE_INT,true);
		boss->addProperty("secureCoef",TCOD_TYPE_FLOAT,true);
		boss->addProperty("summonTime",TCOD_TYPE_FLOAT,true);
		boss->addProperty("minionCount",TCOD_TYPE_INT,true);
		creatures->addStructure(boss);

		TCODParserStruct *player=parser.newStructure("player");
		player->addProperty("char",TCOD_TYPE_CHAR,true);
		player->addProperty("color",TCOD_TYPE_COLOR,true);
		player->addProperty("speed",TCOD_TYPE_FLOAT,true);
		player->addProperty("sprintLength",TCOD_TYPE_FLOAT,true);
		player->addProperty("sprintRecovery",TCOD_TYPE_FLOAT,true);
		player->addProperty("rangeAccomodation",TCOD_TYPE_FLOAT,true);
		player->addProperty("maxPathFinding",TCOD_TYPE_INT,true);
		player->addProperty("healRate",TCOD_TYPE_FLOAT,true);
		player->addProperty("healIntensityDelay",TCOD_TYPE_FLOAT,true);
		player->addProperty("healIntensityPattern",TCOD_TYPE_STRING,true);
		player->addProperty("longButtonDelay",TCOD_TYPE_FLOAT,true);
		player->addProperty("longSpellDelay",TCOD_TYPE_FLOAT,true);
		player->addProperty("moveUpKey",TCOD_TYPE_CHAR,true);
		player->addProperty("moveDownKey",TCOD_TYPE_CHAR,true);
		player->addProperty("moveLeftKey",TCOD_TYPE_CHAR,true);
		player->addProperty("moveRightKey",TCOD_TYPE_CHAR,true);
		player->addProperty("quickslot1",TCOD_TYPE_CHAR,true);
		player->addProperty("quickslot2",TCOD_TYPE_CHAR,true);
		player->addProperty("quickslot3",TCOD_TYPE_CHAR,true);
		player->addProperty("quickslot4",TCOD_TYPE_CHAR,true);
		player->addProperty("quickslot5",TCOD_TYPE_CHAR,true);
		player->addProperty("quickslot6",TCOD_TYPE_CHAR,true);
		player->addProperty("quickslot7",TCOD_TYPE_CHAR,true);
		player->addProperty("quickslot8",TCOD_TYPE_CHAR,true);
		player->addProperty("quickslot9",TCOD_TYPE_CHAR,true);
		player->addProperty("quickslot10",TCOD_TYPE_CHAR,true);
		creatures->addStructure(player);

	config->addStructure(creatures);

	parser.run("data/cfg/config.txt",NULL);
}
bool Config::getBool(const char *name) {
	return parser.getBoolProperty(name);
}

int Config::getChar(const char *name) {
	return parser.getCharProperty(name);
}

int Config::getInt(const char *name) {
	return parser.getIntProperty(name);
}

float Config::getFloat(const char *name) {
	return parser.getFloatProperty(name);
}

TCODColor Config::getColor(const char *name) {
	return parser.getColorProperty(name);
}

TCOD_dice_t Config::getDice(const char *name) {
	return parser.getDiceProperty(name);
}

const char * Config::getString(const char *name) {
	return parser.getStringProperty(name);
}

void * Config::getCustom(const char *name) {
	return parser.getCustomProperty(name);
}


