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

/**************************
	Defines.h
 Place any definitions here
**************************/

// set NOSURFACES to use palettized tile drawing - probably slower, but using less memory
#define NOSURFACES
#define NOSURFACES_EXTRACT // this will cause tiles to be extracted
// #undef NOSURFACES // enable this if display is too slow

// set NOSURFACES_OBJECTS to use palettized object & overlay drawing - slower, but better display
#define NOSURFACES_OBJECTS
// #undef NOSURFACES_OBJECTS

// MW 08/07/01: define SMUDGE_SUPP if you want to support smudges
// added this as switch because if there are problems with this new feature we can just disable it
#define SMUDGE_SUPP

// tiberium / ore
#ifdef RA2_MODE
#define RIPARIUS_BEGIN 102
#define RIPARIUS_END 121
#define CRUENTUS_BEGIN 27
#define CRUENTUS_END 38
#define VINIFERA_BEGIN 127
#define VINIFERA_END 146
#define ABOREUS_BEGIN 147
#define ABOREUS_END 166
#else
#define RIPARIUS_BEGIN 102
#define RIPARIUS_END 121
#define CRUENTUS_BEGIN 27
#define CRUENTUS_END 38
#define VINIFERA_BEGIN 127
#define VINIFERA_END 146
#define ABOREUS_BEGIN 147
#define ABOREUS_END 166
#endif

// picdata type
#define PICDATA_TYPE_BMP 0
#define PICDATA_TYPE_SHP 1
#define PICDATA_TYPE_VXL 2

#define SUBPOS_COUNT 3
#define REAL_SUBPOS_COUNT 3

#define SOUND_NONE 0
#define SOUND_POSITIVE 1
#define SOUND_NEGATIVE 2
#define SOUND_LAYDOWNTILE 3
#define SOUND_DELETETILE 4

// actiondata modes (isoview user interface)
#define ACTIONMODE_PLACE 1 // place any unit, building, tree or overlay
#define ACTIONMODE_ERASEFIELD 2 // delete any unit, building, or tree
#define ACTIONMODE_WAYPOINT 3 // any waypoint editing
#define ACTIONMODE_CELLTAG 4 // any celltag editing
#define ACTIONMODE_NODE 5 // any node editing
#define ACTIONMODE_MAPTOOL 6
#define ACTIONMODE_SETTILE 10 // setting ground tiles
#define ACTIONMODE_HEIGHTEN 11 // heighten ground
#define ACTIONMODE_LOWER 12 // lower ground
#define ACTIONMODE_LOWERTILE 13
#define ACTIONMODE_HEIGHTENTILE 14
#define ACTIONMODE_FLATTENGROUND 15
#define ACTIONMODE_HIDETILESET 16 // make specific tileset invisible
#define ACTIONMODE_HIDEFIELD 17 // make specific field invisible
#define ACTIONMODE_CLIFFFRONT 18 // add front cliff
#define ACTIONMODE_CLIFFBACK 19 // add back cliff
#define ACTIONMODE_COPY 20
#define ACTIONMODE_PASTE 21
#define ACTIONMODE_RANDOMTERRAIN 22
#define ACTIONMODE_SMUDGE 23

// paramtype list types
#define PARAMTYPE_NOTHING 0
#define PARAMTYPE_HOUSES 1
#define PARAMTYPE_TEAMTYPES 2
#define PARAMTYPE_UNITTYPES 3
#define PARAMTYPE_INFANTRYTYPES 4
#define PARAMTYPE_AIRCRAFTTYPES 5
#define PARAMTYPE_BUILDINGTYPES 6
#define PARAMTYPE_VIDEOS 7
#define PARAMTYPE_TUTORIALTEXTS 8
#define PARAMTYPE_TRIGGERS 9
#define PARAMTYPE_YESNO 10
#define PARAMTYPE_SOUNDS 11
#define PARAMTYPE_THEMES 12
#define PARAMTYPE_SPEECHES 13
#define PARAMTYPE_SPECIALWEAPONS 14
#define PARAMTYPE_ANIMATIONS 15
#define PARAMTYPE_PARTICLES 16
#define PARAMTYPE_WAYPOINTS 17
#define PARAMTYPE_CRATETYPES 18
#define PARAMTYPE_SPEECHBUBBLETYPES 19
#define PARAMTYPE_GLOBALS 20
#define PARAMTYPE_RULESGLOBALS 27
#define PARAMTYPE_BUILDINGTYPESINI 28
#define PARAMTYPE_TECHTYPES 29

#define THEATER0 "TEMPERATE"
#define THEATER1 "SNOW"
#define THEATER2 "URBAN"
#define THEATER3 "NEWURBAN"
#define THEATER4 "LUNAR"
#define THEATER5 "DESERT"


#define ra2_mode 1
#define ts_mode 0


#ifndef RA2_MODE

#define NO_URBAN
#define MAXHEIGHT 14

#define f_y 24
#define f_x 48

#define TERRAINTYPE_GROUND 0x0d
#define TERRAINTYPE_WATER 0x09
#define TERRAINTYPE_IMPASSABLE 0x0f
#define TERRAINTYPE_ROUGH 0x0e
#define TERRAINTYPE_PAVED 0x0b
#define TERRAINTYPE_BUILDING 0x07
#define TERRAINTYPE_DIRT_ROAD 0x0c
#define TERRAINTYPE_ICE 0x01

#define MAINMIX "TIBSUN.MIX"
#define HOUSES "Houses"
#define MAPHOUSES "Houses"

#endif
#ifdef RA2_MODE

#define MAINMIX "RA2.MIX"
#define HOUSES "Countries"
#define MAPHOUSES "Houses"

#define MAXHEIGHT 14

#define f_y 30
#define f_x 60

#define TERRAINTYPE_GROUND 0x0d
#define TERRAINTYPE_WATER 0x09
#define TERRAINTYPE_IMPASSABLE 0x0f
#define TERRAINTYPE_ROUGH 0x0e
#define TERRAINTYPE_PAVED 0x0b
#define TERRAINTYPE_BUILDING 0x07
#define TERRAINTYPE_DIRT_ROAD 0x0c
#define TERRAINTYPE_ICE 0x01

#endif

#define SLOPE_UP_RIGHT 1
#define SLOPE_UP_BOTTOM 2
#define SLOPE_UP_LEFT 3
#define SLOPE_UP_TOP 4
#define SLOPE_UP_RIGHTBOTTOM 5
#define SLOPE_UP_LEFTBOTTOM 6
#define SLOPE_UP_LEFTTOP 7
#define SLOPE_UP_RIGHTTOP 8
#define SLOPE_DOWN_LEFTTOP 9
#define SLOPE_DOWN_RIGHTTOP 10
#define SLOPE_DOWN_RIGHTBOTTOM 11
#define SLOPE_DOWN_LEFTBOTTOM 12
#define SLOPE_DOWN_TOP 13   // z difference of x-1/y-1 to x+1/y+1 is -2, to x+1/y 1, to x/y+1 1
#define SLOPE_DOWN_RIGHT 14
#define SLOPE_DOWN_BOTTOM 15
#define SLOPE_DOWN_LEFT 16
#define SLOPE_UP_LEFTBOTTOM_AND_RIGHTTOP 17
#define SLOPE_UP_LEFTTOP_AND_RIGHTBOTTOM 18
#define SLOPE_UP_LEFTBOTTOM_AND_RIGHTTOP2 19
#define SLOPE_UP_LEFTTOP_AND_RIGHTBOTTOM2 20

// max images loaded from one overlay graphic file (SHP)
#define max_ovrl_img 60 