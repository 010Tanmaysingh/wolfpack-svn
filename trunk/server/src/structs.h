//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001 by holders identified in authors.txt
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================

#if !defined(__STRUCTS_H__)
#define __STRUCTS_H__

// Platform Specifics
#include "platform.h"

// System Headers
#include <string>
#include <map>

// Library Headers
#include "qstring.h"

// Wolfpack Headers
#include "defines.h"
#include "typedefs.h"
#include "coord.h"

// Library includes
#include "qstring.h"

struct move_st
{
	int effect[18];
};

struct stat_st
{
	int effect[17];
};

struct sound_st
{
	int effect[2];
};

struct path_st 
{
	unsigned short x;
	unsigned short y;
};

struct creat_st
{
	int basesound;
	unsigned char soundflag;
	unsigned char who_am_i;
	int icon;
	unsigned char type;
};

#include "start_pack.h"
struct versionrecord
{
	SI32 file;
	SI32 block;
	SI32 filepos;
	SI32 length;
	SI32 unknown;
} /*PACK*/;
#include "end_pack.h"

// XYZZY
#include "start_pack.h"
struct unitile_st
{
	signed char basez;
	unsigned char type; // 0=Terrain, 1=Item
	unsigned short int id;
	unsigned char flag1;
	unsigned char flag2;
	unsigned char flag3;
	unsigned char flag4;
	signed char height;
	unsigned char weight;
}/* PACK*/;
#include "end_pack.h"

#include "start_pack.h"
struct st_multiidx
{
	SI32 start;
	SI32 length;
	SI32 unknown;
}/* PACK*/;
#include "end_pack.h"

#include "start_pack.h"
struct st_multi
{
	SI32 visible;  // this needs to be first so it is word aligned to avoid bus errors - fur
	short int tile;
	signed short int x;
	signed short int y;
	signed char z;
	signed char empty;
}/* PACK*/;
#include "end_pack.h"

struct location_st
{
	int x1;
	int y1;
	int x2;
	int y2;
	// char region;
	QString region;
};

struct logout_st// Instalog
{
	unsigned int x1;
	unsigned int y1;
	unsigned int x2;
	unsigned int y2;
};

struct skill_st
{
	UINT16 st;
	UINT16 dx;
	UINT16 in;
	UINT16 advance_index;
	QString madeword;
};

struct advance_st
{
	UINT8 skill;
	UINT16 base;
	UINT16 success;
	UINT16 failure;
};

struct make_st
{
	int has;
	int has2;
	int needs;
	int minskill;
	int maxskill;
	unsigned short newcolor; // This color is setted if coloring>-1 // Magius(CHE) �
	int coloring; // Color modification activated by trigger token!  // Magius(CHE) �
	short Mat1id;		// id of material 1 used to make item
	short Mat1color;	// color of material 1 used to make item
	short Mat2id;		// id of material 2 used to make item
	short Mat2color;	// color of material 2 used to make item
	int minrank; // value of minum rank level of the item to create! - Magius(CHE)
	int maxrank; // value of maximum rank level of the item to create! - Magius(CHE)
	int number; // Store Script Number used to Rank System by Magius(CHE)
};

struct gmpage_st
{
	std::string reason;
	SERIAL serial;
	char timeofcall[9];
	std::string name;
	int handled;
};

struct jail_st
{
	Coord_cl oldpos;
	Coord_cl pos;
	bool occupied;
};

struct title_st // For custom titles
{
	QString fame;
	QString skill;
	QString prowess;
	QString other;
};

struct reag_st
{
	int ginseng;
	int moss;
	int drake;
	int pearl;
	int silk;
	int ash;
	int shade;
	int garlic;
};

struct tele_locations_st 
{
	Coord_cl destination, origem;
};

struct ServerList_st
{
	UI16 uiIndex;
	QString sServer;
	UI08 uiFull;
	UI08 uiTime;
	UI32 ip;
	QString sIP;
	UI16 uiPort;
};

#endif

