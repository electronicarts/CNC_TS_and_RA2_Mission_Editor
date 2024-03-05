/*
    FinalSun/FinalAlert 2 Mission Editor

    Copyright (C) 1999-2024 Electronic Arts, Inc.
    Authored by Matthias Wagner

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// This code determines the cell values for 
// StartX, StartY, Width & Height for the [Header] section

#include "stdafx.h"
#include "MapCode.h"
#include "mapdata.h"
#include "defines.h"




int calcXPos(int x, int y)
{
	int x1=x*256+128;
	
	int y1 = y*256 + 128;

	int x2 = (x1 - y1) * 30;

	int y2 = (x1 + y1) * 15;

	int x3 = (x2/256 + 512*30) / 60;

	int y3 = (y2/256) / 30;

	return x3;
}

int calcYPos(int x, int y)
{
	int x1=x*256+128;
	
	int y1 = y*256 + 128;

	int x2 = (x1 - y1) * 30;

	int y2 = (x1 + y1) * 15;

	int x3 = (x2/256 + 512*30) / 60;

	int y3 = (y2/256) / 30;

	return y3;
}

struct MRECT
{
	int X;
	int Y;
	int Width;
	int Height;
};	
	

void MC_GetHeaderRect(int& startx, int& starty, int& width, int& height)
{
	int leastx=10000;
	int leasty=10000;
	int mostx=0;
	int mosty=0;

	MRECT LocalRect, PlayRect;

	RECT r;
	Map->GetLocalSize(&r);
	LocalRect.X=r.left;
	LocalRect.Y=r.top;
	LocalRect.Width=r.right;
	LocalRect.Height=r.bottom;

	CIniFile& ini=Map->GetIniFile();
	PlayRect.X=0;
	PlayRect.Y=0;
	PlayRect.Width=Map->GetWidth();
	PlayRect.Height=Map->GetHeight();

	char c[50];
	char d[50];
	itoa(LocalRect.Width, c, 10);
	
	int x,y;
	int max=Map->GetIsoSize();
	for(x=0;x<max;x++)
	{
		for(y=0;y<max;y++)
		{
			int height=0;

			 if (TRUE) {		  

			  height = Map->GetHeightAt(y+x*max); // remember: x/y switched here, WS coordinate system
 
			  // fudge ramps at the top of the map so that they end up considered not in the local rect
			  //if (cell->ramp && x + y < PlayRect.Width + 2*LocalRect.Y + 4 + height) {
			   //height++;
			  //}
			 }


			if ((x + y > PlayRect.Width + 2*LocalRect.Y + height) &&
			(x + y <= PlayRect.Width + 2*(LocalRect.Y + LocalRect.Height + 1) + height) &&
			(x - y < 2*(LocalRect.X + LocalRect.Width) - PlayRect.Width) &&
			(y - x < PlayRect.Width - 2*LocalRect.X))
			//if ((x + y > PlayRect.Width) && (x - y < PlayRect.Width) && (y - x < PlayRect.Width) && (x + y <= PlayRect.Width + 2 * PlayRect.Height)) 
			 {
				int rx=calcXPos(x,y);
				int ry=calcYPos(x,y);

				if (rx < leastx)
					leastx = rx;
 
				   if (rx > mostx)
					mostx = rx;
 
				   if (ry < leasty)
					leasty = ry;
 
				   if (ry > mosty)
					mosty = ry;
			}

		}
	}

	startx=leastx;
	starty=leasty;
	width=mostx-leastx;
	height=mosty-leasty;
}


 