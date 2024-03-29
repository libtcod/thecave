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
#include <math.h>
#include <stdio.h>
#include "main.hpp"

#define FOREST_W 400
#define FOREST_H 400
#define WATER_START -0.87f

ForestScreen *ForestScreen::instance=NULL;

#define MAX_ENTITY_PROB 5

// probability for an entity (item or creature) to be on some terrain type
struct EntityProb {
	// item or creature type. -1 = end of list
	int entityType;
	// item or creature ?
	bool item;
	// how many entities (1.0 = on every cell, 0.0= never)
	float density;
	// height threshold on this terrain layer (0.0 - 1.0)
	float minThreshold, maxThreshold;
};

struct TerrainGenData {
	TerrainId terrain;
    float threshold;
	EntityProb itemData[MAX_ENTITY_PROB];
};

struct LayeredTerrain {
	const char *name;
	TerrainGenData info[5];
};

enum ForestId {
    FOREST_PINE,
    FOREST_NORTHERN,
    FOREST_OAK,
    NB_FORESTS
};

static LayeredTerrain forestTypes[NB_FORESTS] = {
	{"pine forest",
    {{TERRAIN_GRASS_LUSH,0.5f,
		{{ITEM_PINE,true,36,0.0f,1.0f},
		{CREATURE_SUEDE,false,1,0.0f,1.0f},
		{-1}}},
    {TERRAIN_GRASS_NORMAL,0.0f,
		{{ITEM_PINE,true,28,0.0f,1.0f},
		{-1}}},
    {TERRAIN_GRASS_SPARSE,-0.9f,
		{{ITEM_PINE,true,10,0.17f,1.0f},
		{-1}}},
    {TERRAIN_GROUND,-1.25f,{{-1}}},
    {TERRAIN_GROUND,-1.5f,{{-1}}},
	}},
	{"northern forest",
	{{TERRAIN_GRASS_NORMAL,0.66f,
		{{ITEM_OAK,true,0.0f,1.0f},
		{ITEM_APPLE_TREE,true,10,0.0f,1.0f},
		{-1}}},
    {TERRAIN_GRASS_SPARSE,0.33f,
		{{ITEM_OAK,true,0.0f,1.0f},
		{ITEM_APPLE_TREE,true,5,0.0f,1.0f},
		{-1}}},
    {TERRAIN_GRASS_WITHERED,0.0f,
		{{ITEM_OAK,true,7,0.0f,1.0f},
		{ITEM_APPLE_TREE,true,3,0.0f,1.0f},
		{-1}}},
    {TERRAIN_GRASS_DRIED,-0.5f,
		{{ITEM_OAK,true,4,0.0f,1.0f},
		{-1}}},
    {TERRAIN_GROUND,-1.0f,
		{{ITEM_OAK,true,2,0.1f,1.0f},
		{-1}}},
	}},
	{"oak forest",
	{{TERRAIN_GRASS_LUSH,0.1f,
		{{ITEM_OAK,true,36,0.0f,1.0f},
		{-1}}},
    {TERRAIN_GRASS_NORMAL,-0.4f,
		{{ITEM_OAK,true,14,0.0f,1.0f},
		{-1}}},
    {TERRAIN_GRASS_SPARSE,-0.8f,
		{{ITEM_OAK,true,6,0.12f,1.0f},
		{ITEM_STONE,true,10,0.0f,0.1f},
		{-1}}},
    {TERRAIN_SHALLOW_WATER,WATER_START,
		{
		{ITEM_STONE,true,25,0.2f,1.0f},
		{-1}}},
    {TERRAIN_DEEP_WATER,-1.0f,{{-1}}},
//    {TERRAIN_DIRT,0,-1.0f,-0.75f},
//    {TERRAIN_GRASS_SPARSE,0,-1.25f,-0.75f},
	}},
};

enum { DBG_HEIGHTMAP, DBG_SHADOWHEIGHT, DBG_FOV, DBG_NORMALMAP, DBG_CLOUDS, DBG_WATERCOEF, NB_DEBUGMAPS };
static const char *debugMapNames[] = {
	"heightmap","shadowheight","fov","normalmap","clouds", "waterCoef"
};

ForestScreen::ForestScreen() {
	instance=this;
	forestRng=NULL;
	debugMap=0;
}

void ForestScreen::render() {
	static bool debug=Config::getBool("config.debug");
	// draw subcell ground
	int squaredFov=(int)(player.fovRange*player.fovRange*4);
	int minx,maxx,miny,maxy;
	bool showDebugMap=false;
	ground->clear(TCODColor::black);
	Rect r1(xOffset*2,yOffset*2,CON_W*2,CON_H*2);
	Rect r2(0,0,dungeon->size2x,dungeon->size2x);
	r1.intersect(r2);
	minx = (int)(r1.x - xOffset*2);
	maxx = (int)(r1.x + r1.w - xOffset*2);
	miny = (int)(r1.y - yOffset*2);
	maxy = (int)(r1.y + r1.h - yOffset*2);
	float fovRatio=1.0f/(aspectRatio*aspectRatio);
	for (int x=minx; x < maxx; x++) {
		for (int y=miny; y < maxy; y++) {
			int dungeon2x=x+xOffset*2;
			int dungeon2y=y+yOffset*2;
			TCODColor col;
			int dx=(int)(dungeon2x-player.x*2);
			int dy=(int)(dungeon2y-player.y*2);
/*
			// in fov range, you see under the tree tops
			// out of range, you see the tree tops
			if ( dx*dx+dy*dy <= squaredFov ) {
				col=dungeon->getShadedGroundColor(dungeon2x,dungeon2y);
				if ( ! dungeon->map2x->isInFov(dungeon2x,dungeon2y) ) col = col * 0.8;
*/
			if ( dx*dx+dy*dy*fovRatio <= squaredFov && dungeon->map2x->isInFov(dungeon2x,dungeon2y) ) {
				col=dungeon->getShadedGroundColor(dungeon2x,dungeon2y);
			} else {
				col=dungeon->canopy->getPixel(dungeon2x,dungeon2y);
				if (col.r == 0) {
					col=dungeon->getShadedGroundColor(dungeon2x,dungeon2y);
				} else {
					col = col * dungeon->getInterpolatedCloudCoef(dungeon2x,dungeon2y);
				}
			}

			// debug maps
			if ( debug && TCODConsole::isKeyPressed(TCODK_TAB) && TCODConsole::isKeyPressed(TCODK_SHIFT) ) {
				switch (debugMap) {
					case DBG_HEIGHTMAP : {
						float h = dungeon->hmap->getValue(dungeon2x,dungeon2y);
						col = h*TCODColor::white;
						} break;
					case DBG_SHADOWHEIGHT : {
						float h = dungeon->getShadowHeight(dungeon2x,dungeon2y);
						col = h*TCODColor::white;
						} break;
					case DBG_FOV : {
						col = dungeon->map2x->isInFov(dungeon2x,dungeon2y) ? TCODColor::lightGrey : TCODColor::darkGrey;
						} break;
					case DBG_NORMALMAP : {
						float n[3];
						dungeon->hmap->getNormal(dungeon2x,dungeon2y,n);
						col=TCODColor((int)(128+n[0]*128),(int)(128+n[1]*128),(int)(128+n[2]*128));
						} break;
					case DBG_CLOUDS : {
						float h = dungeon->getInterpolatedCloudCoef(dungeon2x, dungeon2y);
						h = (h-0.5f)/1.2f;
						col = h*TCODColor::white;
					} break;
					case DBG_WATERCOEF : {
						float h = dungeon->getWaterCoef(dungeon2x, dungeon2y);
						col = h*TCODColor::white;
					} break;
				}
				showDebugMap=true;
			}

			ground->putPixel(x,y,col);
		}
	}
	// render the subcell creatures
	dungeon->renderSubcellCreatures(lightMap);
	// draw ripples
	if (! showDebugMap ) rippleManager->renderRipples(ground);
	// render the fireballs
	for (FireBall **it=fireballs.begin();it!=fireballs.end(); it++) {
		(*it)->render(ground);
	}

	// blit it on console
	ground->blit2x(TCODConsole::root,0,0);
	// render the items
	dungeon->renderItems(lightMap);
	// render the creatures
	dungeon->renderCreatures(lightMap);
	// render the player
	player.render(lightMap);

	descriptor.render();

	if ( pauseOn ) {
		TCODConsole::root->setDefaultForeground(TCODColor::lighterGrey);
		TCODConsole::root->print(CON_W-10,CON_H-1,"= pause =");
	}
	// apply sepia post-processing
	if ( pauseCoef != 0.0f ) {
		for (int x=0; x < CON_W; x++) {
			for (int y=0; y < CON_H; y++) {
				TCODColor bk=TCODConsole::root->getCharBackground(x,y);
				TCODColor fore=TCODConsole::root->getCharForeground(x,y);
				TCODConsole::root->setCharBackground(x,y,setSepia(bk,pauseCoef));
				TCODConsole::root->setCharForeground(x,y,setSepia(fore,pauseCoef));
			}
		}
	}

	// render messages
	//log.render();

	if ( debug && TCODConsole::isKeyPressed(TCODK_TAB) && TCODConsole::isKeyPressed(TCODK_SHIFT) ) {
		TCODConsole::root->setDefaultBackground(TCODColor::grey);
		TCODConsole::root->setDefaultForeground(TCODColor::white);
		TCODConsole::root->printEx(CON_W/2,0,TCOD_BKGND_MULTIPLY,TCOD_CENTER,debugMapNames[debugMap]);
	}

//TCODConsole::root->print(0,2,"player pos %d %d\nfriend pos %d %d\n",player.x,player.y,fr->x,fr->y);
	if ( player.name[0] == 0 ) textInput.render(CON_W/2,2);

}

bool ForestScreen::update(float elapsed, TCOD_key_t k,TCOD_mouse_t mouse) {
	static bool debug=Config::getBool("config.debug");

	mousex=mouse.cx;
	mousey=mouse.cy;
	if ( firstFrame ) elapsed=0.1f;
	firstFrame=false;

	GameEngine::update(elapsed,k,mouse);

	if ( player.name[0] == 0 ) {
		if (!textInput.update(elapsed,k)) {
			strcpy(player.name,textInput.getText());
			TextGenerator::addGlobalValue("PLAYER_NAME",player.name);
			resumeGame();
		} else return true;
	}

	if ( (k.c == 'm' || k.c =='M') && ! k.pressed && k.lalt) {
			// ALT-M : switch control type
			userPref.mouseOnly=!userPref.mouseOnly;
			log.warn(userPref.mouseOnly ? "Mouse only" : "Mouse + keyboard");
			k.c=0;k.vk=TCODK_NONE;
	} else if ( k.c ==' ' && ! k.pressed) {
		if (pauseOn) resumeGame();
		else pauseGame();
	} else if ( ! k.pressed && (k.c == 'i' || k.c =='I' ) ) {
		openCloseInventory();
	}
	// non player related keyboard handling
	if ( debug ) {
		// debug/cheat shortcuts
		if ( k.c == 'd' && k.lalt && ! k.pressed) {
			// debug mode : Alt-d = player takes 'd'amages
			player.takeDamage(20);
		} else if ( k.vk == TCODK_TAB && ! k.pressed ) {
			debugMap = (debugMap+1) % NB_DEBUGMAPS;
		} else if ( k.c == 'i' && k.lalt && ! k.pressed) {
			// debug mode : Alt-i = item
			dungeon->addItem(Item::getItem(ITEM_SMALL_BRONZE_BLADE,player.x,player.y-1));
		}
	}
	if ( k.vk == TCODK_ALT || k.lalt ) lookOn=k.pressed;

	// update messages
	//log.update(k,mouse,elapsed);

	if (pauseOn ) {
		descriptor.setFocus(mousex,mousey,mousex+xOffset,mousey+yOffset,lookOn);
		return true;
	}

	// update player
	player.update(elapsed,k,&mouse);
	xOffset=(int)(player.x-CON_W/2);
	yOffset=(int)(player.y-CON_H/2);

	// update items
	dungeon->updateItems(elapsed,k,&mouse);

	// calculate player fov
	dungeon->computeFov((int)player.x,(int)player.y);

	// update monsters
	if ( fade != FADE_DOWN ) {
		dungeon->updateCreatures(elapsed);
		// ripples must be after creatures because of shoal updates
		rippleManager->updateRipples(elapsed);
	}
	dungeon->updateClouds(elapsed);

	HerdBehavior::updateScarePoints(elapsed);

	// update fireballs
	TCODList<FireBall *> fireballsToRemove;
	for ( FireBall **it = fireballs.begin(); it != fireballs.end(); it++) {
		if (! (*it)->update(elapsed)) {
			fireballsToRemove.push(*it);
			it=fireballs.removeFast(it);
		}
	}
	fireballsToRemove.clearAndDelete();

	descriptor.setFocus(mousex,mousey,mousex+xOffset,mousey+yOffset,lookOn);

	return true;
}

void ForestScreen::onInitialise() {
	engine.registerModule(tutorial);
    engine.registerModule(&log);
    engine.registerModule(&statusPanel);
    engine.registerModule(&guiInventory);
    engine.registerModule(&guiLoot);
}

void ForestScreen::placeHouse(Dungeon *dungeon,int doorx, int doory,Entity::Direction dir) {
	Building *building=Building::generate(9,7,2,forestRng);
	building->applyTo(dungeon,doorx,doory);
	building->setHuntingHide(dungeon);
}

void ForestScreen::setCanopy(int x, int y, const ItemType * treeType) {
	static TCODColor green1=TCODColor::darkChartreuse;
	static TCODColor green2=TCODColor::green*0.7f;
	static const int treeRadiusW=4;
	for (int tx=-treeRadiusW; tx <= treeRadiusW; tx++) {
		if ( x+tx >= 0 && x+tx < FOREST_W*2 ) {
			// we want round trees even with non square fonts
			int dy=(int)(sqrtf(treeRadiusW*treeRadiusW - tx*tx) * aspectRatio);
			for (int ty=-dy; ty <= dy; ty++) {
				if ( y+ty >= 0 && y+ty < 2*FOREST_H ) {
					if ( dungeon->getShadowHeight(x+tx,y+ty)< 2.0f ) {
						TCODColor treecol=TCODColor::lerp(green1,green2,forestRng->getFloat(0.0f,1.0f));
						if ( treeType->id == ITEM_PINE ) treecol = treecol * 0.75f;
						treecol = treecol * ( 0.6f+0.4f*(tx+treeRadiusW)/(2*treeRadiusW) );
						if ( treeType->id == ITEM_APPLE_TREE && forestRng->getInt(0,80)==0 ) treecol =TCODColor::darkOrange;
						dungeon->canopy->putPixel(x+tx,y+ty,treecol);
						if ( x+tx >= 2 ) {
							// cast shadow
							dungeon->setShadowHeight(x+tx,y+ty,2.0f);
							float shadow=dungeon->getShadow(x+tx-2,y+ty)*0.95f;
							dungeon->setShadow(x+tx-2,y+ty,shadow);
							if (dungeon->getShadowHeight(x+tx-2,y+ty)< 2.0f) {
								TCODColor col=dungeon->canopy->getPixel(x+tx-2,y+ty);
								if ( col.r != 0 ) {
									col = col*shadow;
									dungeon->canopy->putPixel(x+tx-2,y+ty,col);
								}
							}
						}
					}
				}
			}
		}
	}
}

void ForestScreen::placeTree(Dungeon *dungeon,int x, int y, const ItemType * treeType) {
	// trunk
	int dx=x/2;
	int dy=y/2;
	// no tree against a door
	if (dungeon->hasItemTag(dx-1,dy,ITEM_BUILD_NOT_BLOCK)
		|| dungeon->hasItemTag(dx+1,dy,ITEM_BUILD_NOT_BLOCK)
		|| dungeon->hasItemTag(dx,dy-1,ITEM_BUILD_NOT_BLOCK)
		|| dungeon->hasItemTag(dx,dy+1,ITEM_BUILD_NOT_BLOCK)) return;

	dungeon->addItem(Item::getItem(treeType,x/2,y/2));
	// folliage
	setCanopy(x,y,treeType);
	ItemFeature *feat=treeType->getFeature(ITEM_FEAT_PRODUCES);
	if (feat && forestRng->getFloat(0.0,1.0) < feat->produce.chance/10.0f) {
		// drop some fruit/twig
		int dx=x/2;
		int dy=y/2;
		dungeon->getClosestWalkable(&dx,&dy);
		Item *it = Item::getItem(feat->produce.type,dx,dy);
		dungeon->addItem(it);
	}

}

int housex,housey;
void ForestScreen::generateMap(uint32 seed) {
	static TCODColor sunColor=TCODColor(250,250,255);
	DBG(("Forest generation start\n"));
	forestRng = new TCODRandom(seed);
	dungeon = new Dungeon(FOREST_W,FOREST_H);

	saveGame.registerListener(CHA1_CHUNK_ID,PHASE_START,this);
    saveGame.registerListener(DUNG_CHUNK_ID,PHASE_START,dungeon);
	saveGame.registerListener(PLAY_CHUNK_ID,PHASE_START,&player);
	saveGame.registerListener(TUTO_CHUNK_ID,PHASE_START,tutorial);
	saveGame.registerListener(HIST_CHUNK_ID,PHASE_START,&log);
	lightMap->clear(sunColor);
	for (int x=1; x < FOREST_W-1; x++ ) {
		if ( x % 40 == 0 ) displayProgress(0.4f+(float)(x) / FOREST_W*0.1f);
		for (int y=1; y < FOREST_H-1; y++ ) {
			dungeon->map->setProperties(x,y,true,true);
		}
	}
	for (int x=2; x < 2*FOREST_W-2; x++ ) {
		if ( x % 40 == 0 ) displayProgress(0.5f + (float)(x) / (2*FOREST_W)*0.1f);
		for (int y=2; y < 2*FOREST_H-2; y++ ) {
			dungeon->map2x->setProperties(x,y,true,true);
		}
	}
	displayProgress(0.6f);
	dungeon->hmap->addFbm(new TCODNoise(2,forestRng),2.20*FOREST_W/400,2.20*FOREST_W/400,0,0,4.0f,1.0,2.05);
	dungeon->hmap->normalize();
	TCODNoise terrainNoise(2,0.5f,2.0f,forestRng);
#ifndef NDEBUG
	float t0=TCODSystem::getElapsedSeconds();
#endif
	housex=forestRng->getInt(20,dungeon->size-20);
	housey=forestRng->getInt(20,dungeon->size-20);
	// don't put the house on water
	while ( dungeon->hasWater(housex,housey) ) {
		housex += 4;
		if ( housex > dungeon->size-20 ) {
			housex=20;
			housey += 4;
			if ( housey > dungeon->size-20 ) housey = 20;
		}
	}
	placeHouse(dungeon,housex,housey,Entity::NORTH);
	dungeon->saveShadowBeforeTree();

	for (int x=2*FOREST_W-1; x >=0 ; x--) {
		float f[2];
		f[0] = 2.5f * x / FOREST_W;
		if ( x % 40 == 0 ) displayProgress(0.6f + (float)(2*FOREST_W-1-x) / (2*FOREST_W)*0.4f);
		for (int y=0; y < 2*FOREST_H-1; y++ ) {
			if ( dungeon->getCell(x/2,y/2)->terrain == TERRAIN_WOODEN_FLOOR ) continue;
			f[1] = 2.5f * y / FOREST_H;
			float height = terrainNoise.getFbm(f, 5.0f, TCOD_NOISE_SIMPLEX);
			float forestTypeId = (dungeon->hmap->getValue(x,y) * NB_FORESTS);
			forestTypeId=MIN(NB_FORESTS-1,forestTypeId);
			LayeredTerrain *forestType1 = &forestTypes[(int)forestTypeId];
			LayeredTerrain *forestType2 = forestType1;
			if ( (int)forestTypeId < forestTypeId ) forestType2++;
            TerrainGenData *info1 = &forestType1->info[0];
            TerrainGenData *info2 = &forestType2->info[0];
            float maxThreshold1=2.0f;
            float maxThreshold2=2.0f;
            TCODColor nextColor1;
            TCODColor nextColor2;
			bool swimmable1=false;
			bool swimmable2=false;
            while (height<=info1->threshold) {
            	nextColor1 = terrainTypes[info1->terrain].color;
            	maxThreshold1 = info1->threshold;
				swimmable1=terrainTypes[info1->terrain].swimmable;
            	info1++;
            }
            while (height<=info2->threshold) {
            	nextColor2 = terrainTypes[info2->terrain].color;
            	maxThreshold2 = info2->threshold;
				swimmable2=terrainTypes[info2->terrain].swimmable;
            	info2++;
            }
			float terrainTypeCoef=forestTypeId-(int)forestTypeId;
			float layer1Height = (height - info1->threshold) / (maxThreshold1 - info1->threshold);
			float layer2Height = (height - info2->threshold) / (maxThreshold2 - info2->threshold);
			float waterCoef=0.0f;
			if ( terrainTypes[info1->terrain].swimmable || swimmable1
				|| terrainTypes[info2->terrain].swimmable || swimmable2 ) {
				waterCoef = (WATER_START-height) / (WATER_START +1);
			}
			TerrainGenData *info=NULL;
			if ( (terrainTypeCoef < 0.25f && !swimmable2 && !terrainTypes[info2->terrain].swimmable)  || swimmable1 || terrainTypes[info1->terrain].swimmable) {
		      	TCODColor groundCol1=TCODColor::lerp(terrainTypes[info1->terrain].color,
					nextColor1,
					layer1Height);
		      	dungeon->setGroundColor(x,y,groundCol1);
		      	/*
				if ( terrainTypes[info1->terrain].swimmable && swimmable1 ) waterCoef=1.0f;
				else if ( terrainTypes[info1->terrain].swimmable ) waterCoef=1.0f-layer1Height;
				else if ( swimmable1 ) waterCoef=layer1Height;
				*/
				info=info1;
			} else if ( terrainTypeCoef > 0.75f || swimmable2 || terrainTypes[info2->terrain].swimmable) {
		      	TCODColor groundCol2=TCODColor::lerp(terrainTypes[info2->terrain].color,
					nextColor2,
					layer2Height);
		      	dungeon->setGroundColor(x,y,groundCol2);
		      	/*
				if ( terrainTypes[info2->terrain].swimmable && swimmable2 ) waterCoef=1.0f;
				else if ( terrainTypes[info2->terrain].swimmable ) waterCoef=1.0f-layer2Height;
				else if ( swimmable2 ) waterCoef=layer2Height;
				*/
				info=info2;
			} else {
		      	TCODColor groundCol1=TCODColor::lerp(terrainTypes[info1->terrain].color,
					nextColor1,
					layer1Height);
					/*
				float waterCoef1=0.0f,waterCoef2=0.0f;
				if ( terrainTypes[info1->terrain].swimmable && swimmable1 ) waterCoef1=1.0f;
				else if ( terrainTypes[info1->terrain].swimmable ) waterCoef1=1.0f-layer1Height;
				else if ( swimmable1 ) waterCoef1=layer1Height;
				*/

		      	TCODColor groundCol2=TCODColor::lerp(terrainTypes[info2->terrain].color,
					nextColor2,
					layer2Height);
					/*
				if ( terrainTypes[info2->terrain].swimmable && swimmable2 ) waterCoef2=1.0f;
				else if ( terrainTypes[info2->terrain].swimmable ) waterCoef2=1.0f-layer2Height;
				else if ( swimmable2 ) waterCoef2=layer2Height;
				*/

				float coef=(terrainTypeCoef-0.25f)*2;
				if ( terrainTypes[info1->terrain].swimmable && swimmable1 ) coef = 1.0f-waterCoef;
				else if ( terrainTypes[info2->terrain].swimmable && swimmable2 ) coef=waterCoef;
		      	dungeon->setGroundColor(x,y,TCODColor::lerp(groundCol1,groundCol2,coef));
				//waterCoef=waterCoef2*coef + waterCoef1*(1.0f-coef);
				info = ( terrainTypeCoef <= 0.5f ? info1 : info2 );
			}
			if (terrainTypes[info->terrain].ripples) waterCoef=MAX(0.01f,waterCoef);
			dungeon->getSubCell(x,y)->waterCoef=waterCoef;
			if ( (x&1) == 0 && (y&1) == 0 ) {
				dungeon->setTerrainType(x/2,y/2,info->terrain);
				EntityProb *itemData=info->itemData;
				int count=MAX_ENTITY_PROB;
				while (count > 0 && itemData->entityType != -1) {
					if (layer1Height >= itemData->minThreshold && layer1Height < itemData->maxThreshold
						&& forestRng->getFloat(0,100) < itemData->density) {
						if ( itemData->item ) {
							ItemType *type=Item::getType((ItemTypeId)itemData->entityType);
							if ( type->isA(ITEM_TREE) ) placeTree(dungeon,x,y,type);
							else dungeon->addItem(Item::getItem(type, x/2, y/2));
						} else {
							Creature *cr=Creature::getCreature((CreatureTypeId)itemData->entityType);
							cr->setPos(x/2,y/2);
							dungeon->addCreature(cr);
						}
					}
					itemData++;
					count--;
				}
			}

		}
	}

//	static float lightDir[3]={0.2f,0.0f,1.0f};
//	dungeon->computeOutdoorLight(lightDir, sunColor);
	dungeon->smoothShadow();
	dungeon->computeSpawnSources();
//	dungeon->applyShadowMap();
#ifndef NDEBUG
	float t1=TCODSystem::getElapsedSeconds();
	DBG(("Forest generation end. %g sec\n",t1-t0));
#endif
}

// SaveListener
#define CHA1_CHUNK_VERSION 1
bool ForestScreen::loadData(uint32 chunkId, uint32 chunkVersion, TCODZip *zip) {
	if ( chunkVersion != CHA1_CHUNK_VERSION ) return false;
	return true;
}

void ForestScreen::saveData(uint32 chunkId, TCODZip *zip) {
	saveGame.saveChunk(CHA1_CHUNK_ID,CHA1_CHUNK_VERSION);
}

void ForestScreen::loadMap(uint32 seed) {
	DBG(("Forest loading start\n"));
	static TCODColor sunColor=TCODColor(250,250,255);
	lightMap->clear(sunColor);
	forestRng = new TCODRandom(seed);
	dungeon = new Dungeon(FOREST_W,FOREST_H);

	saveGame.registerListener(CHA1_CHUNK_ID,PHASE_START,this);
    saveGame.registerListener(DUNG_CHUNK_ID,PHASE_START,dungeon);
	saveGame.registerListener(PLAY_CHUNK_ID,PHASE_START,&player);
	saveGame.registerListener(TUTO_CHUNK_ID,PHASE_START,tutorial);
	saveGame.registerListener(HIST_CHUNK_ID,PHASE_START,&log);

	for (int x=1; x < FOREST_W-1; x++ ) {
		for (int y=1; y < FOREST_H-1; y++ ) {
			dungeon->map->setProperties(x,y,true,true);
		}
	}
	for (int x=2; x < 2*FOREST_W-2; x++ ) {
		for (int y=2; y < 2*FOREST_H-2; y++ ) {
			dungeon->map2x->setProperties(x,y,true,true);
		}
	}
#ifndef NDEBUG
	float t0=TCODSystem::getElapsedSeconds();
#endif
	saveGame.load(PHASE_START);

	dungeon->computeSpawnSources();
	fr = (Friend *)dungeon->getCreature(CREATURE_FRIEND);
#ifndef NDEBUG
	float t1=TCODSystem::getElapsedSeconds();
	DBG(("Forest loading end. %g sec\n",t1-t0));
#endif

}

void ForestScreen::onActivate() {
	TCODConsole::root->setDefaultBackground(TCODColor::black);
	TCODConsole::root->clear();
	// disable fading (to see the progress bar)
	TCODConsole::setFade(255,TCODColor::black);
    TCODConsole::setColorControl(TCOD_COLCTRL_1,TCODColor(255,255,240),TCODColor::black);
    TCODConsole::setColorControl(TCOD_COLCTRL_2,guiHighlightedText,TCODColor::black);
	GameEngine::onActivate();
	tutorial->init();
	init();
	log.set.push(&statusPanel);
	statusPanel.set.push(&log);
	engine.activateModule(tutorial);
	engine.activateModule(&log);
	engine.activateModule(&statusPanel);
	MainMenu::instance->waitForForestGen();

	if ( newGame ) {
		generateMap(saveGame.seed);
		dungeon->setPlayerStartingPosition();
		int fx,fy;
		fr = new Friend();
		dungeon->getClosestSpawnSource(player.x,player.y, &fx, &fy);
		dungeon->getClosestWalkable(&fx,&fy,true,false);
		fr->setPos(fx,fy);
		dungeon->addCreature(fr);
		player.addToInventory(Item::getRandomWeapon(ITEM_KNIFE,ITEM_CLASS_STANDARD));
		player.x=housex;
		player.y=housey+10;
	} else {
		displayProgress(0.02f);
		loadMap(saveGame.seed);
	}
	// re-enable fading
	TCODConsole::setFade(0,TCODColor::black);
	player.maxFovRange=player.fovRange=8;
	timefix=1.0f;
	if ( newGame ) log.critical ("Welcome to the Cave v" VERSION " ! %c?%c for help.",TCOD_COLCTRL_2,TCOD_COLCTRL_STOP);
	lookOn=false;
	rippleManager=new RippleManager(dungeon);
	if ( player.name[0] == 0 ) {
		if ( userPref.nbLaunches == 1 ) {
			textInput.init("Welcome to The Cave !","PageUp/PageDown to change font size\nPlease enter your name :",60);
		} else {
			textInput.init("Welcome to The Cave !","Please enter your name :",60);
		}

		pauseGame();
	}
}

void ForestScreen::onDeactivate() {
	engine.deactivateModule(tutorial);
	engine.deactivateModule(&log);
}

void ForestScreen::onFontChange() {
	float oldAspectRatio=aspectRatio;
	GameEngine::onFontChange();
	// recompute canopy if aspect ratio has changed (we want round trees!)
	if ( dungeon->canopy && oldAspectRatio != aspectRatio ) {
		dungeon->canopy->clear(TCODColor::black);
		dungeon->restoreShadowBeforeTree();
		for (int x=FOREST_W-1; x >=0 ; x--) {
			for (int y=0; y < FOREST_H-1; y++ ) {
				Item *tree=dungeon->getItemTag(x,y,ITEM_TREE);
				if ( tree ) {
					setCanopy(x*2,y*2,tree->typeData);
				}
			}
		}
	}
}
