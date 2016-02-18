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
#include <stdio.h>
#include "main.hpp"

Building::Building(int w, int h) : Rect(0,0,w,h) {
	map = new int[w*h];
	memset(map,0,sizeof(int)*w*h);
	doorx=doory=0;
}

Building *Building::generate(int width, int height, int nbRooms, TCODRandom *rng) {
	Building *ret=new Building(width,height);
	int oldrx,oldry,oldrw,oldrh;
	// draw random rectangular rooms
	for (int room=0; room < nbRooms; room++) {
		int rx,ry,rw,rh;
		rw=rng->getInt(width/nbRooms,MIN(width,width*2/nbRooms));
		rh=rng->getInt(height/nbRooms,MIN(height,height*2/nbRooms));
		if ( room == 0 ) {
			rx=rng->getInt(0,width-rw);
			ry=rng->getInt(0,height-rh);
		} else {
			// rooms should not be disjoint
			rx=rng->getInt(oldrx-rw,oldrx+oldrw);
			ry=rng->getInt(oldry-rh,oldry+oldrh);
			rx = CLAMP(0,width-rw,rx);
			ry = CLAMP(0,height-rh,ry);
		}
		// fill room with floor
		for (int rry=ry; rry < ry+rh; rry++) {
			for (int rrx=rx; rrx < rx+rw; rrx++) {
				ret->map[rrx+rry*width]=BUILDING_FLOOR;
			}
		}
		oldrx=rx;oldry=ry;oldrw=rw;oldrh=rh;
	}
	ret->buildExternalWalls();
	ret->placeRandomDoor(rng);
	for (int room=0; room < nbRooms; room++) {
		ret->placeRandomWindow(rng);
	}
	return ret;
}

void Building::placeRandomDoor(TCODRandom *rng) {
	doorx=rng->getInt(0,w-1);
	doory=rng->getInt(0,h-1);
	while(map[doorx+doory*w] < BUILDING_WALL_N || map[doorx+doory*w] > BUILDING_WALL_W ) {
		// not a door-hosting wall
		doorx++;
		if ( doorx == w ) {
			doorx=0;doory++;
			if ( doory==h ) doory=0;
		}
	}
	map[doorx+doory*w]=BUILDING_DOOR;	
}

#define IS_FLAT_WALL(x) ((x) >= BUILDING_WALL_N && (x) <= BUILDING_WALL_W)
void Building::placeRandomWindow(TCODRandom *rng) {
	int winx=rng->getInt(0,w-1);
	int winy=rng->getInt(0,h-1);
	bool ok=false;
	int count=w*h;
	bool horiz=false;
	while (!ok && count > 0) {
		int cell=map[winx+winy*w];
		while(count > 0 && !IS_FLAT_WALL(cell)) {
			// not a window-hosting wall
			count --;
			winx++;
			if ( winx == w ) {
				winx=0;winy++;
				if ( winy==h ) winy=0;
			}
			cell=map[winx+winy*w];
		}
		// window should not be adjacent to door or corner
		horiz = ((cell & 1) == 0);
		int before=0;
		int after=0;
		if ( horiz ) {
			before=map[winx-1+winy*w];
			after=map[winx+1+winy*w];
		} else {
			before=map[winx+(winy-1)*w];
			after=map[winx+(winy+1)*w];
		}
		if ( IS_FLAT_WALL(before) && IS_FLAT_WALL(after) ) ok=true;
		else {
			count --;
			winx++;
			if ( winx == w ) {
				winx=0;winy++;
				if ( winy==h ) winy=0;
			}
		}
	}
	if ( !ok ) return; // no place found
	map[winx+winy*w]=horiz ? BUILDING_WINDOW_H : BUILDING_WINDOW_V;	
}

// scan the map. put walls at floor/none borders
void Building::buildExternalWalls() {
	for (int y=0; y < h; y++) {
		for (int x=0; x < w; x++) {
			if ( map[x+y*w] == BUILDING_FLOOR ) {
				// check if there are adjacent outdoor cells
				enum BuildDir {
					BD_NW=1,
					BD_N=2,
					BD_NE=4,
					BD_E=8,
					BD_SE=16,
					BD_S=32,
					BD_SW=64,
					BD_W=128
				};
				int bd=0;
				static int xdir[8]={-1,0,1,1,1,0,-1,-1};
				static int ydir[8]={-1,-1,-1,0,1,1,1,0};
				// find direction that contain outdoor cell
				for (int dir=0; dir < 8; dir++) {
					int dx=x+xdir[dir];
					int dy=y+ydir[dir];
					// flag outdoor directions
					if (! IN_RECTANGLE(dx,dy,w,h) || map[dx+dy*w] == BUILDING_NONE ) bd |= (1<<dir);
				}
				if ( bd != 0 ) {
					// oudoor cells found. create an external wall
					int wall=map[x+y*w];
					#define HAS_FLAG(f,f2) ( ((f)&(f2)) == (f2) )
					#define IS_NW(f) HAS_FLAG(f,BD_W|BD_NW|BD_N)
					#define IS_NE(f) HAS_FLAG(f,BD_N|BD_NE|BD_E)
					#define IS_SE(f) HAS_FLAG(f,BD_E|BD_SE|BD_S)
					#define IS_SW(f) HAS_FLAG(f,BD_S|BD_SW|BD_W)
					#define IS_N(f) HAS_FLAG(f,BD_N)
					#define IS_E(f) HAS_FLAG(f,BD_E)
					#define IS_S(f) HAS_FLAG(f,BD_S)
					#define IS_W(f) HAS_FLAG(f,BD_W)
					if (IS_NW(bd)) wall=BUILDING_WALL_NW;  
					else if (IS_NE(bd)) wall=BUILDING_WALL_NE;  
					else if (IS_SE(bd)) wall=BUILDING_WALL_SE;  
					else if (IS_SW(bd)) wall=BUILDING_WALL_SW;  
					else if ( bd == BD_NW ) wall = BUILDING_WALL_SE;
					else if ( bd == BD_NE ) wall = BUILDING_WALL_SW;
					else if ( bd == BD_SE ) wall = BUILDING_WALL_NW;
					else if ( bd == BD_SW ) wall = BUILDING_WALL_NE;
					else if (IS_N(bd)) wall=BUILDING_WALL_N;  
					else if (IS_E(bd)) wall=BUILDING_WALL_E;  
					else if (IS_S(bd)) wall=BUILDING_WALL_S;  
					else if (IS_W(bd)) wall=BUILDING_WALL_W;  
					map[x+y*w]=wall;
				}
			}			
		}
	}
}

bool Building::getFreeFloor(int *fx, int *fy) {
	int ffx=rng->getInt(0,w-1);
	int ffy=rng->getInt(0,h-1);
	int count=w*h;
	while(count > 0 && map[ffx+ffy*w] != BUILDING_FLOOR ) {
		ffx++;
		count --;
		if ( ffx == w ) {
			ffx=0;ffy++;
			if ( ffy==h ) ffy=0;
		}
	}
	if ( count == 0 ) return false;
	*fx=ffx;
	*fy=ffy;
	return true;
}

void Building::setHuntingHide(Dungeon *dungeon) {
	int ix,iy;
	/*
	if (getFreeFloor(&ix,&iy)) {
		Item *item=Item::getItem(ITEM_BAG,x+ix,y+iy);
		dungeon->addItem(item);
		map[ix+iy*w] = BUILDING_ITEM;
	}
	*/
	if (getFreeFloor(&ix,&iy)) {
		Item *chest=Item::getItem(ITEM_CHEST,x+ix,y+iy);
		dungeon->addItem(chest);
		map[ix+iy*w] = BUILDING_ITEM;
		// fill the chest
		Item *blade = Item::getItem(ITEM_SMALL_BRONZE_BLADE,0,0);
		//Item *knife=Item::getRandomWeapon(ITEM_KNIFE,ITEM_CLASS_STANDARD);
		//knife->name = strdup("hunting knife");
		blade->name=strdup("hunting knife blade");
		chest->putInside(blade);
	}
}

void Building::applyTo(Dungeon *dungeon, int dungeonDoorx, int dungeonDoory) {
	static TCODColor roofcol=TCODColor::darkOrange;
	x=dungeonDoorx-doorx;
	y=dungeonDoory-doory;
	for (int cy=0; cy < h; cy++) {
		for (int cx=0; cx < w; cx++) {
			int cell=map[cx+cy*w];
			switch(cell) {
				case BUILDING_NONE : break;
				case BUILDING_WALL_N :
				case BUILDING_WALL_E :
				case BUILDING_WALL_S :
				case BUILDING_WALL_W :
				case BUILDING_WALL_NW :
				case BUILDING_WALL_NE :
				case BUILDING_WALL_SE :
				case BUILDING_WALL_SW :
					{
						// walls
						static int wallToChar[] = {
							0,0,
							TCOD_CHAR_HLINE,TCOD_CHAR_VLINE,TCOD_CHAR_HLINE,TCOD_CHAR_VLINE,
							TCOD_CHAR_NW,TCOD_CHAR_NE,TCOD_CHAR_SE,TCOD_CHAR_SW
						};
						Item *wall=Item::getItem(ITEM_WALL,x+cx,y+cy);
						wall->ch=wallToChar[cell];
						dungeon->addItem(wall);
					}
				// no break!
				// concerns also walls
				case BUILDING_FLOOR :
				case BUILDING_WINDOW_H :
				case BUILDING_WINDOW_V :
				case BUILDING_DOOR :
					// floor
					dungeon->getCell(x+cx,y+cy)->terrain=TERRAIN_WOODEN_FLOOR;
					static int subcx[] = {0,1,0,1};
					static int subcy[] = {0,0,1,1};
					int dungeon2x=(int)(x+cx)*2;
					int dungeon2y=(int)(y+cy)*2;
					// subcell stuff
					for (int i=0; i < 4; i++) {
						int d2x=dungeon2x+subcx[i];
						int d2y=dungeon2y+subcy[i];
						dungeon->setGroundColor(d2x,d2y,TCODColor::darkerAmber);
						dungeon->setShadowHeight(d2x,d2y,1.0f);
						// roof
						dungeon->canopy->putPixel(d2x,d2y,cx*2+subcx[i] < w ? roofcol*0.7f : roofcol);
					}
					if ( cell == BUILDING_DOOR ) {
						dungeon->addItem(Item::getItem(ITEM_DOOR,x+cx,y+cy));
					} else if ( cell == BUILDING_WINDOW_H ) {
						Item *item=Item::getItem(ITEM_WINDOW,x+cx,y+cy);
						item->ch=TCOD_CHAR_HLINE;
						dungeon->addItem(item);
					} else if ( cell == BUILDING_WINDOW_V ) {
						Item *item=Item::getItem(ITEM_WINDOW,x+cx,y+cy);
						item->ch=TCOD_CHAR_VLINE;
						dungeon->addItem(item);
					}
				break;
			}
		}
	}
}
