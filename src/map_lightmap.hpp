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
#pragma once

class LightMap {
public :
	LightMap(int width, int height);
	void clear(const TCODColor& col);
	void applyToImage(TCODImage *img,
		int minx2x=0,int miny2x=0,int maxx2x=0,int maxy2x=0);
	inline TCODColor &getColor2x(int x, int y) { return data2x[x+y*width]; }
	inline TCODColor &getColor(int x, int y) { return data[x+y*width/2]; }
	inline void setColor2x(int x, int y, TCODColor col) {
		data2x[x+y*width]=col;
		if ( (x&1)==0 && (y&1) == 0 ) data[x/2+y*width/4]=col;
	}

	float getFog(int x,int y);
	float getPlayerFog(int x,int y);
	void update(float elapsed);

	int width,height;
	float fogRange;

protected :
	TCODColor *data2x;
	TCODColor *data;

	float fogZ;
	TCODNoise *fogNoise;
};
