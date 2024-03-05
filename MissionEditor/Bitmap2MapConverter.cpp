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

// Bitmap2MapConverter.cpp: Implementierung der Klasse CBitmap2MapConverter.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "stdafx.h"
#include "Bitmap2MapConverter.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CBitmap2MapConverter::CBitmap2MapConverter()
{

}

CBitmap2MapConverter::~CBitmap2MapConverter()
{

}

/*
I originally intended this tool to support different theaters,
but currently it does only support temperate. People can use
copy/paste to convert those maps though!
*/
BOOL CBitmap2MapConverter::Convert(HBITMAP hBitmap, CMapData & mapdata)
{
	BITMAP bm;
		
	GetObject(hBitmap, sizeof(BITMAP), &bm);
	
	HBITMAP hUsed=hBitmap;
	
	
	if(bm.bmWidth+bm.bmHeight>255)
	{
		float scalex=(float)bm.bmWidth/(float)bm.bmHeight;
		int neededheight, neededwidth;
		neededheight=255.0f/(scalex+1.0f);
		neededwidth=255-neededheight;

		hUsed=CreateCompatibleBitmap(GetDC(NULL), neededwidth, neededheight);
		HDC hDC=CreateCompatibleDC(GetDC(NULL));
		SelectObject(hDC, hUsed);
		HDC hSrcDC=CreateCompatibleDC(GetDC(NULL));
		SelectObject(hSrcDC, hBitmap);

		StretchBlt(hDC, 0,0,neededwidth,neededheight, hSrcDC, 0,0,bm.bmWidth, bm.bmHeight, SRCCOPY);

		DeleteDC(hDC);
		DeleteDC(hSrcDC);
		
		GetObject(hUsed, sizeof(BITMAP), &bm);
	}

	HDC hDC;
	hDC=CreateCompatibleDC(GetDC(NULL));
	SelectObject(hDC, hUsed);
	

	srand(GetTickCount());

	int i;
	int e;
	int theater=0;
	mapdata.CreateMap(bm.bmWidth, bm.bmHeight, "TEMPERATE", 0);

	int isosize=mapdata.GetIsoSize();


	for(i=0;i<(*tiledata_count);i++)
		if((*tiledata)[i].wTileSet==waterset) break;

	int water_start=i+8; // to 12


	int sandset=atoi((*tiles).sections["General"].values["SandTile"]);
	int greenset=atoi((*tiles).sections["General"].values["GreenTile"]);

#ifdef RA2_MODE
	sandset=atoi((*tiles).sections["General"].values["GreenTile"]);
	greenset=atoi((*tiles).sections["General"].values["RoughTile"]);
#endif

	for(i=0;i<(*tiledata_count);i++)
		if((*tiledata)[i].wTileSet==sandset) break;

	int sand_start=i; 

	for(i=0;i<(*tiledata_count);i++)
		if((*tiledata)[i].wTileSet==greenset) break;

	int green_start=i; 


	for(i=0;i<bm.bmWidth;i++)
	{
		for(e=0;e<bm.bmHeight;e++)
		{
			COLORREF col=GetPixel(hDC, i, bm.bmHeight-e);

			int x=(i)+mapdata.GetHeight()+1;
			int y=(e)+mapdata.GetWidth();

			int xiso;
			int yiso;

			yiso=mapdata.GetIsoSize()-(y - x);
			xiso=mapdata.GetIsoSize()-(x + y);
			yiso-=mapdata.GetHeight();

			
			for(x=-1;x<2;x++)
			{
				for(y=-1;y<2;y++)
				{
					DWORD dwPos=xiso+x+(yiso+y)*isosize;
					
					if(dwPos>isosize*isosize) continue;
					
					FIELDDATA* fd=mapdata.GetFielddataAt(dwPos);
					
					int r=GetRValue(col);
					int g=GetGValue(col);
					int b=GetBValue(col);

					if(g>r && g>b)
					{
						if(theater!=1)
						{
							fd->wGround=0;
							fd->bSubTile=0;
						}
					}
					if(b>g && b>r)
					{
						int p=rand()*4/RAND_MAX;
						fd->wGround=water_start+p;
						fd->bSubTile=0;
					}
					if(g>b+25 && r>b+25 && g>120 && r>120)
					{
						if(theater!=1)
						{
							fd->wGround=sand_start;
							fd->bSubTile=0;
						}
					}
					if(b<20 && r<20 && g>20)
					{
#ifdef RA2_MODE
						if(g<140) // dark only
#endif
						{
							fd->wGround=green_start;
							fd->bSubTile=0;
						}
						
					}

				}
			}
		}
	}

	mapdata.CreateShore(0,0,isosize, isosize);

	for(i=0;i<isosize*isosize;i++)
	{
		mapdata.SmoothAllAt(i);
	}

	DeleteDC(hDC);

	if(hUsed!=hBitmap) DeleteObject(hUsed);

	return TRUE;
}
