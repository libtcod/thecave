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

#include <stdlib.h>
#include <string.h>
#include "main.hpp"

// old school fire routines

TCODColor fireColor[ 256 ];
static bool col_init=false;

#define UPDATE_DELAY 0.05f
#define FIRE_SET(x,y,v) buf[ (y) * (w+2) + (x) ] = (v)
#define FIRE_SET2(x,y,v) smoothedBuf[ (y) * (w+2) + (x) ] = (v)
#define FIRE_GET(x,y) buf[ (y) * (w+2) + (x) ]
#define FIRE_GET2(x,y) smoothedBuf[ (y) * (w+2) + (x) ]

Fire::Fire(int w, int h) :w(w),h(h),el(0.0f) {
	if (! col_init) {
		int i;
		for (i=0; i < 128; i++) {
			fireColor[i].r=MIN(i*3,255);
			fireColor[i].g=i*2;
			fireColor[i].b=i/2;
			fireColor[i+128].r=255;
			fireColor[i+128].g=255;
			fireColor[i+128].b=MIN(64+192*i/128,255);
		}
		col_init=true;
	}
	img = new TCODImage(w,h);
	buf = new uint8[(w+2)*h];
	smoothedBuf = new uint8[(w+2)*h];
	memset(buf,0,sizeof(uint8)*(w+2)*h);
	memset(smoothedBuf,0,sizeof(uint8)*(w+2)*h);
}

void Fire::generateImage() {
	for (int x=1; x <= w; x++ ) {
		for (int y=0; y < h; y++) {
			img->putPixel(x-1,y,fireColor[FIRE_GET2(x,y)]);
		}
	}
}

void Fire::spark(int x, int y) {
	softspark(x,y,48);
}

void Fire::antispark(int x, int y) {
	softspark(x,y,-48);
}

void Fire::softspark(int x, int y, int delta) {
	int v = (int)(FIRE_GET(x,y))+delta;
	v = CLAMP(0,255,v);
	FIRE_SET(x,y,(uint8)v);
}

void Fire::update(float elapsed) {
	el += elapsed;
	if (el < UPDATE_DELAY) return;
	el = 0.0f;
	/*
	int x,y;
	int off1 = (h-1)*(w+2);
	int off2 = (h-2)*(w+2);
	for (x=0; x < w+2; x++) {
		uint8 v = TCODRandom::getInstance()->getInt(0,255);
		buf[ off1 + x ] = v;
		buf[ off2 + x ] = MAX(v-4,0);
	}
	*/
	for (int y=1; y < h-1; y++) {
		for (int x=1; x <= w; x++) {
			int x2 = x + TCODRandom::getInstance()->getInt(-1,1);
			int v = (int)(FIRE_GET(x,y)) * 4
				+ (int)(FIRE_GET(x,y+1)) * 4
				+ (int)(FIRE_GET(x+1,y+1))
				+ (int)(FIRE_GET(x-1,y+1));
			v /= 10 ;
			v -= 4;
			if ( y < 10 ) v -= 4;
			if ( v < 0) v=0;
			FIRE_SET(x2,y-1,(uint8)v);
		}
	}
	
	for (int y=0; y < h-1; y++) {
		for (int x=1; x <= w; x++) {
			int v = (int)(FIRE_GET(x,y)) 
				+ (int)(FIRE_GET(x,y+1)) 
				+ (int)(FIRE_GET(x+1,y+1))
				+ (int)(FIRE_GET(x+1,y));
			v /= 4 ;
			FIRE_SET2(x,y,(uint8)v);
		}
	}
	
}

Fire::~Fire() {
	delete buf;
	delete smoothedBuf;
	delete img;
}
