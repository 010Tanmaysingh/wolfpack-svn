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

 //Boats->cpp by Zippy Started on 7/12/99

// Mapregion stuff + boat-blocking corrected/touched by LB 7/24/99

#include "boats.h"
#include "regions.h"

#undef DBGFILE
#define DBGFILE "boats.cpp" 

#define X 0
#define Y 1

//============================================================================================
//UooS Item translations - You guys are the men! :o)

//[4]=direction of ship
//[4]=Which Item (PT Plank, SB Plank, Hatch, TMan)
//[2]=Coord (x,y) offsets
signed short int iSmallShipOffsets[4][4][2]=
// X  Y  X  Y  X  Y  X  Y
{ -2, 0, 2, 0, 0,-4, 1, 4,//Dir
   0,-2, 0, 2, 4, 0,-4, 0,
   2, 0,-2, 0, 0, 4, 0,-4,
   0, 2, 0,-2,-4, 0, 4, 0 };
//  P1    P2   Hold  Tiller
signed short int iMediumShipOffsets[4][4][2]=
// X  Y  X  Y  X  Y  X  Y
{ -2, 0, 2, 0, 0,-4, 1, 5,
   0,-2, 0, 2, 4, 0,-5, 0,
   2, 0,-2, 0, 0, 4, 0,-5,
   0, 2, 0,-2,-4, 0, 5, 0 };
signed short int iLargeShipOffsets[4][4][2]=
// X  Y  X  Y  X  Y  X  Y
{ -2,-1, 2,-1, 0,-5, 1, 5,
   1,-2, 1, 2, 5, 0,-5, 0,
   2, 1,-2, 1, 0, 5, 0,-5,
  -1, 2,-1,-2,-5, 0, 5, 0 };
//Ship Items
//[4] = direction
//[6] = Which Item (PT Plank Up,PT Plank Down, SB Plank Up, SB Plank Down, Hatch, TMan)
unsigned char cShipItems[4][6]=
{(unsigned char)0xB1,(unsigned char)0xD5,(unsigned char)0xB2,(unsigned char)0xD4,(unsigned char)0xAE,(unsigned char)0x4E,
 (unsigned char)0x8A,(unsigned char)0x89,(unsigned char)0x85,(unsigned char)0x84,(unsigned char)0x65,(unsigned char)0x53,
 (unsigned char)0xB2,(unsigned char)0xD4,(unsigned char)0xB1,(unsigned char)0xD5,(unsigned char)0xB9,(unsigned char)0x4B,
 (unsigned char)0x85,(unsigned char)0x84,(unsigned char)0x8A,(unsigned char)0x89,(unsigned char)0x93,(unsigned char)0x50 };
//============================================================================================


void sendinrange(P_ITEM pi)//Send this item to all online people in range
{//(Decided this was better than writting 1000 for loops to send an item.
	for(int a=0;a<now;a++)
	{
		if(perm[a] && iteminrange(a, pi, VISRANGE))
			senditem(a, pi);
	}
}

P_ITEM findmulti(Coord_cl pos) //Sortta like getboat() only more general... use this for other multi stuff!
{
	int lastdist = 30;
	P_ITEM multi = NULL;
	int ret;

	cRegion::RegionIterator4Items ri(pos);
	
	for (ri.Begin(); !ri.atEnd(); ri++)
	{
		P_ITEM mapitem = ri.GetData();
		if (mapitem != NULL)
		{
			if (mapitem->isMulti())
			{
				ret = pos.distance(mapitem->pos);
				if (ret <= lastdist)
				{
					lastdist = ret;
					if (inmulti(pos, mapitem))
						multi = mapitem;
				}
			}
		}
	}

	return multi;
}

bool inmulti(Coord_cl pos, P_ITEM pi)//see if they are in the multi at these chords (Z is NOT checked right now)
{
	int j;
	SI32 length;			// signed long int on Intel
	st_multi multi;
	UOXFile *mfile;
	Map->SeekMulti(pi->id()-0x4000, &mfile, &length);
	length=length/sizeof(st_multi);
	if (length == -1 || length>=17000000)//Too big...
	{
		sprintf((char*)temp,"inmulti() - Bad length in multi file. Avoiding stall. (Item Name: %s)\n", pi->name.c_str() );
		LogError( (char*)temp ); // changed by Magius(CHE) (1)
		length = 0;
	}
	for (j=0;j<length;j++)
	{
		mfile->get_st_multi(&multi);
		if ((multi.visible)&&(pi->pos.x+multi.x == pos.x) && (pi->pos.y+multi.y == pos.y))
		{
			return true;
		}
	}
	return false;
}

void cBoat::PlankStuff(UOXSOCKET s, P_ITEM pi_plank)//If the plank is opened, double click Will send them here
{
	P_CHAR pc_cs,pc_b;

	pc_cs = currchar[s];

	P_ITEM boat = GetBoat(pc_cs);
	if(boat == NULL)//They aren't on a boat, so put then on the plank.
	{
		// LB, bugfix for tillerman not reacting if the boat was entered via plank !

		// we need to get the boat again after beaming the character to the boat's plank
		// otherweise only -1's will be added to the boat hash-table 
        
		// khpae : commented 3 lines
		//pc_cs->moveTo(pi_plank->pos + Coord_cl(0,0,5));

		//pc_cs->multis=-3; // we have to trick getboat to start the search !!!
		                              // will be corrected automatically by setserial...

		//P_ITEM boat2 = GetBoat(pc_cs);
		P_ITEM boat2 = FindItemBySerial (calcserial (pi_plank->more1, pi_plank->more2, pi_plank->more3, pi_plank->more4));
		if (boat2 == NULL)
			return;

		unsigned int a;	
		vector<SERIAL> vecCown = cownsp.getData(pc_cs->serial);
		for(a = 0; a < vecCown.size(); a++)//Put all their Pets/Hirlings on the boat too
		{
			pc_b = FindCharBySerial(vecCown[a]);

			if (pc_b != NULL) // never log -1's that indicate non existance !!!
			{
			   if (pc_b->isNpc() && pc_cs->Owns(pc_b))
			   {
				  pc_b->moveTo(boat2->pos + Coord_cl(1, 1, 2));
                  pc_b->SetMultiSerial(boat2->serial);
				  teleport(pc_b);
			   }
			}
		}
	
		// khpae
		UI16 x, y;
		SI08 z;
		switch (boat2->dir & 0x0F) {
			case 0:
			case 4:
				x = (boat2->pos.x + pi_plank->pos.x) / 2;
				y = (boat2->pos.y + pi_plank->pos.y) / 2;
				break;
			case 2:
			case 6:
				x = (boat2->pos.x + pi_plank->pos.x) / 2;
				y = (boat2->pos.y + pi_plank->pos.y) / 2;
				break;
			default:
				return;
		}
		z = boat2->pos.z + 3;
		pc_cs->MoveTo (x, y, z);
		teleport (pc_cs);
      pc_cs->SetMultiSerial(boat2->serial); // set chars->multis value
		sysmessage(s,"You entered a boat");
	} else {
		int bser = calcserial (pi_plank->more1, pi_plank->more2, pi_plank->more3, pi_plank->more4);
		if (bser != boat->serial) {
			return;
		}
		if (LeaveBoat(s, pi_plank)) {	//They are on a boat, get off
			sysmessage (s, "You left the boat.");
		} else {
			sysmessage (s, "You cannot get off here!");
		}
	}

/*        OpenPlank(pi_plank); //lb

		if (boat2 != NULL) // now set the char coords to the boat !!!
		{
			pc_cs->moveTo(boat2->pos + Coord_cl(1, 1, 2));
		}

		sysmessage(s,"you entered a boat");
        pc_cs->SetMultiSerial(boat2->serial);
	} else {
		LeaveBoat(s, pi_plank);//They are on a boat, get off
	}
	teleport(pc_cs);//Show them they moved.*/
}

// khpae : rewritten

/*void cBoat::LeaveBoat(UOXSOCKET s, P_ITEM pi_plank)//Get off a boat (dbl clicked an open plank while on the boat.
{
	P_CHAR pc_cs,pc_b;
	
	pc_cs = currchar[s];

	//long int pos, pos2, length;
	int x,x2=pi_plank->pos.x;
	int y,y2=pi_plank->pos.y;
	signed char z = pi_plank->pos.z, mz, sz, typ;
	P_ITEM pBoat = GetBoat(pc_cs);
	
	if (pBoat == NULL) 
		return;
	
	for(x=x2-2;x<x2+3;x++)
	{
		for(y=y2-2;y<y2+3;y++)
		{
			sz = Map->StaticTop(Coord_cl(x,y,z, pi_plank->pos.map ) ); // MapElevation() doesnt work cauz we are in a multi !!
			
			mz = Map->MapElevation(Coord_cl(x,y, z, pi_plank->pos.map ) );
			if (sz == illegal_z) 
				typ=0;
			else 
				typ=1;
			
			if((typ==0 && mz!=5) || (typ==1 && sz!=-5))// everthing the blocks a boat is ok to leave the boat ... LB
			{
				unsigned int a;
				vector<SERIAL> vecCown = cownsp.getData(pc_cs->serial);
				for(a = 0; a < vecCown.size(); a++)//Put all their Pets/Hirlings on the boat too
				{
					pc_b = FindCharBySerial(vecCown[a]);

					if (pc_b != NULL) // never log -1's that indicate non existance !!!
					{
						if (pc_b->isNpc() && pc_cs->Owns(pc_b) && inrange1p(currchar[s], pc_b))
						{
							pc_b->MoveTo(x,y, typ ? sz : mz);
							
							if(pc_b->multis>-1)
							{
								if( pBoat->serial != INVALID_SERIAL ) 
									cmultisp.remove(pBoat->serial, pc_b->serial);
								pc_b->multis=-1;
							}
							
							teleport((pc_b));
						}
					}
				}
				
				pc_cs->SetMultiSerial( INVALID_SERIAL );
				
				if (typ) 
					pc_cs->MoveTo(x, y, sz); 
				else  
					pc_cs->MoveTo(x, y, mz);
				
				sysmessage(s,"You left the boat.");			
				return;
			}
		}//for y
	}//for x
	sysmessage(s,"You cannot get off here!");
}*/

void cBoat::OpenPlank(P_ITEM pi_p)//Open, or close the plank (called from keytarget() )
{
	switch(pi_p->id2)
	{
		//Open plank->
		case (unsigned char)0xE9: pi_p->id2=(unsigned char)0x84; break;
		case (unsigned char)0xB1: pi_p->id2=(unsigned char)0xD5; break;
		case (unsigned char)0xB2: pi_p->id2=(unsigned char)0xD4; break;
		case (unsigned char)0x8A: pi_p->id2=(unsigned char)0x89; break;
		case (unsigned char)0x85: pi_p->id2=(unsigned char)0x84; break;
		//Close Plank->
		case (unsigned char)0x84: pi_p->id2=(unsigned char)0xE9; break;
		case (unsigned char)0xD5: pi_p->id2=(unsigned char)0xB1; break;
		case (unsigned char)0xD4: pi_p->id2=(unsigned char)0xB2; break;
		case (unsigned char)0x89: pi_p->id2=(unsigned char)0x8A; break;
		default: { sprintf((char*)temp,"WARNING: Invalid plank ID called! Plank %x '%s' [%x %x]\n",pi_p->serial,pi_p->name.c_str(),pi_p->id1,pi_p->id2); LogWarning( (char*)temp ); break; }
	}
}

bool cBoat::Build(UOXSOCKET s, P_ITEM pBoat, char id2)//Build a boat! (Do stuff NESSICARY for boats, called from buildhouse() )
{
	P_CHAR pc_cs = currchar[s];

	int nid2=id2;
	tile_st tile;
	map_st map;
	land_st land;

	if( !pBoat ) 
	{
		sysmessage(s, "There was an error creating that boat.");
		return false;
	}
	
	if(id2!=0x00 && id2!=0x04 && id2!=0x08 && id2!=0x0C && id2!=0x10 && id2!=0x14)//Valid boat ids (must start pointing north!)
	{
		sysmessage(s, "The deed is broken, please contact a Game Master.");
		return false;
	}
	//Start checking for a valid possition:

	map = Map->SeekMap(pBoat->pos);
	switch(map.id)
	{
	//water tiles:
	case 0x00A8:
	case 0x00A9:
	case 0x00AA:
	case 0x00Ab:
	case 0x0136:
	case 0x0137:
	case 0x3FF0:
	case 0x3FF1:
	case 0x3FF2:
	case 0x2FF3:
		break;
	//Lava tiles:
	case 0x01F4:
	case 0x01F5:
	case 0x01F6:
	case 0x01F7:
		break;
	default:
		Map->SeekTile(((buffer[s][0x11]<<8)+buffer[s][0x12]), &tile);
		if(!(strstr((char *) tile.name, "water") || strstr((char *) tile.name, "lava")))
		{
			Map->SeekLand(map.id, &land);
			if (!(land.flag1&0x80))//not a "wet" tile
			{
				sysmessage(s, "You cannot place your boat there.");
				return false;
			}
		}
	}
	
	// Okay we found a good  place....

	pBoat->more1=id2;//Set min ID
	pBoat->more2=nid2+3;//set MAX id
	pBoat->type=117;//Boat type
	pBoat->pos.z=-5;//Z in water
	pBoat->name = "a mast";//Name is something other than "%s's house"
	
	P_ITEM pTiller=Items->SpawnItem(pc_cs,1,"a tiller man",0,0x3E4E,0,0);
	if( !pTiller ) return false;
	pTiller->pos.z=-5;
	pTiller->priv=0;
	// khpae
	pTiller->type = 117;
	pTiller->type2 = 1; // tiller man sub type
	pTiller->more1 = static_cast<unsigned char>((pBoat->serial&0xFF000000)>>24);
	pTiller->more2 = static_cast<unsigned char>((pBoat->serial&0x00FF0000)>>16);
	pTiller->more3 = static_cast<unsigned char>((pBoat->serial&0x0000FF00)>>8);
	pTiller->more4 = static_cast<unsigned char>((pBoat->serial&0x000000FF));
	pTiller->pos.z=-5;
	pTiller->priv=0;

	P_ITEM pPlankR=Items->SpawnItem(pc_cs,1,"#",0,0x3EB2,0,0);//Plank2 is on the RIGHT side of the boat
	if( !pPlankR ) return false;
	pPlankR->type=117;
	pPlankR->type2=3;
	pPlankR->more1 = static_cast<unsigned char>((pBoat->serial&0xFF000000)>>24);
	pPlankR->more2 = static_cast<unsigned char>((pBoat->serial&0x00FF0000)>>16);
	pPlankR->more3 = static_cast<unsigned char>((pBoat->serial&0x0000FF00)>>8);
	pPlankR->more4 = static_cast<unsigned char>((pBoat->serial&0x000000FF));
	pPlankR->pos.z=-5;
	pPlankR->priv=0;//Nodecay

	P_ITEM pPlankL=Items->SpawnItem(pc_cs,1,"#",0,0x3EB1,0,0);//Plank1 is on the LEFT side of the boat
	if( !pPlankL ) return false;
	pPlankL->type=117;//Boat type
	pPlankL->type2=3;//Plank sub type
	pPlankL->more1 = static_cast<unsigned char>((pBoat->serial&0xFF000000)>>24);
	pPlankL->more2 = static_cast<unsigned char>((pBoat->serial&0x00FF0000)>>16);
	pPlankL->more3 = static_cast<unsigned char>((pBoat->serial&0x0000FF00)>>8);
	pPlankL->more4 = static_cast<unsigned char>((pBoat->serial&0x000000FF));
	pPlankL->pos.z=-5;
	pPlankL->priv=0;

	P_ITEM pHold=Items->SpawnItem(pc_cs,1,"#",0,0x3EAE,0,0);
	if( !pHold ) return false;
	pHold->more1 = static_cast<unsigned char>((pBoat->serial&0xFF000000)>>24);
	pHold->more2 = static_cast<unsigned char>((pBoat->serial&0x00FF0000)>>16);
	pHold->more3 = static_cast<unsigned char>((pBoat->serial&0x0000FF00)>>8);
	pHold->more4 = static_cast<unsigned char>((pBoat->serial&0x000000FF));
	pHold->type=1;//Conatiner
	pHold->pos.z=-5;
	pHold->priv=0;
	
	pBoat->moreb1 = static_cast<unsigned char>((pTiller->serial&0xFF000000)>>24);
	pBoat->moreb2 = static_cast<unsigned char>((pTiller->serial&0x00FF0000)>>16);
	pBoat->moreb3 = static_cast<unsigned char>((pTiller->serial&0x0000FF00)>>8);
	pBoat->moreb4 = static_cast<unsigned char>((pTiller->serial&0x000000FF));
	pBoat->morex=pPlankL->serial;//Store the other stuff anywhere it will fit :-)
	pBoat->morey=pPlankR->serial;
	pBoat->morez=pHold->serial;
	
	switch(id2)//Give everything the right Z for it size boat
	{
	case 0x00:
	case 0x04:
		pTiller->pos.x=pBoat->pos.x+1;
		pTiller->pos.y=pBoat->pos.y+4;
		pPlankR->pos.x=pBoat->pos.x+2;
		pPlankR->pos.y=pBoat->pos.y;
		pPlankL->pos.x=pBoat->pos.x-2;
		pPlankL->pos.y=pBoat->pos.y;
		pHold->pos.x=pBoat->pos.x;
		pHold->pos.y=pBoat->pos.y-4;
		break;
	case 0x08:
	case 0x0C:
		pTiller->pos.x=pBoat->pos.x+1;
		pTiller->pos.y=pBoat->pos.y+5;
		pPlankR->pos.x=pBoat->pos.x+2;
		pPlankR->pos.y=pBoat->pos.y;
		pPlankL->pos.x=pBoat->pos.x-2;
		pPlankL->pos.y=pBoat->pos.y;
		pHold->pos.x=pBoat->pos.x;
		pHold->pos.y=pBoat->pos.y-4;
		break;
	case 0x10:
	case 0x14:
		pTiller->pos.x=pBoat->pos.x+1;
		pTiller->pos.y=pBoat->pos.y+5;
		pPlankR->pos.x=pBoat->pos.x+2;
		pPlankR->pos.y=pBoat->pos.y-1;
		pPlankL->pos.x=pBoat->pos.x-2;
		pPlankL->pos.y=pBoat->pos.y-1;
		pHold->pos.x=pBoat->pos.x;
		pHold->pos.y=pBoat->pos.y-5;
		break;
	}
	pBoat->type2 = 9;	// khpae : not moving 9, 1-8 : moving direction+1
	mapRegions->Add(pTiller);//Make sure everything is in da regions!
	mapRegions->Add(pPlankL);
	mapRegions->Add(pPlankR);
	mapRegions->Add(pHold);
	mapRegions->Add(pBoat);
	
	//their x pos is set by BuildHouse(), so just fix their Z...
	//pc_cs->pos.z = pc_cs->dispz = pBoat->pos.z+3;//Char Z, try and keep it right.  khpae : no need
    pc_cs->SetMultiSerial(pBoat->serial);
	return true;
}

P_ITEM cBoat::GetBoat(P_CHAR pcc_cs)//get the closest boat to the player and check to make sure they are on it
{	
	
	if (pcc_cs == NULL)
	{
		return NULL;
	}

	P_ITEM pi_boat = NULL;

    if (pcc_cs->multis > 0) 
		return FindItemBySerial( pcc_cs->multis );
	// khpae
	else return NULL;
/*    else if (pcc_cs->multis == -1) return NULL;
	else 
	{
		pi_boat = findmulti(pcc_cs->pos);
		if (pi_boat != NULL)//if we found a boat, make sure they are in it
			if(!inmulti(pcc_cs->pos, pi_boat)) 
				pi_boat = NULL;

		return pi_boat;
	}*/
}


// This  Boat-blocking method is now WATER PROOF :-)
// Please don't TOUCH !!!
// Lord Binary 21 August 1999

// it doesnt check against dynamics yet, especially against other ships.
// hopefully coming soon

// khpae : rewritten
/*bool cBoat::Block(P_ITEM pBoat, short int xmove, short int ymove, int dir)//Check to see if the boat is blocked in front of, behind, or next to it (Depending on direction)
// PARAM WARNING: xmove and ymove is unreferenced
{
	int ser, sz, zt, loopexit=0;
	short x = 0, y = 0, c;
	bool blocked = false;
	char type, size = 0, typ = 0;
	
	map_st map;
	land_st land;
	tile_st tile;

    ser = calcserial(pBoat->moreb1, pBoat->moreb2, pBoat->moreb3, pBoat->moreb4);
	P_ITEM t	= FindItemBySerial( ser );
	P_ITEM p1	= FindItemBySerial( pBoat->morex );
	P_ITEM p2	= FindItemBySerial( pBoat->morey );
	P_ITEM h	= FindItemBySerial( pBoat->morez );

	switch(dir)
	{
	case 6: // L
	case 7: // U & L
	case 0: // U
	case 1: // U & R
		x = min( t->pos.x, min( h->pos.x, min( p1->pos.x, p2->pos.x ) ) );
		y = min( t->pos.y, min( h->pos.y, min( p1->pos.y, p2->pos.y ) ) );
		if ( dir != 6 )
			type = 1;
		else if ( dir == 0 )
			type = 2;
		else 
			type = 3;
		break;

	case 2: // R
	case 3: // D & R
	case 4: // D
	case 5: // D & L
		x = max( t->pos.x, max( h->pos.x, max( p1->pos.x, p2->pos.x ) ) );
		y = min( t->pos.y, min( h->pos.y, min( p1->pos.y, p2->pos.y ) ) );
		if ( dir != 2 )
			type = 1;
		else if ( dir == 4 )
			type = 2;
		else 
			type = 3;
		break;
	}
	//small = 10x5, med = 11x5 large = 12x5
	switch(pBoat->more1)//Now set what size boat it is and move the specail items
	{
	case 0x00:
	case 0x04:
		if ( type == 1 )
			size = 10;
		else if ( type == 2 )
			size = 5;
		else
			size = 7;
	case 0x08:
	case 0x0C:
		if ( type == 1 )
			size = 11;
		else if ( type == 2 )
			size = 5;
		else
			size = 7;
	case 0x10:
	case 0x14:
		if ( type == 1 )
			size = 12;
		else if ( type == 2 )
			size = 5;
		else
			size = 7;
		break;
	}

	if ( type == 1)
		y -= (size/2)-1;
	else
		x -= (size/2)-1;

	for ( c=0 ; c<size ; c++ )
	{
		if ( type == 1 )
			y++;
		else if ( type == 2 )
			x++;
		else // type == 3
		{
			x++;
			y++;
		}

		sz = Map->StaticTop(Coord_cl(x,y, pBoat->pos.z, pBoat->pos.map ) );

		if (sz==illegal_z) 
			typ=0; //0: map-tile 
		else 
			typ=1; //1: static-tile
		
		if (typ==0)
		{
			map=Map->SeekMap( Coord_cl( x, y, 0, pBoat->pos.map ) );
			Map->SeekLand(map.id, &land);
			//clConsole.send("map type, water bit: %i\n",land.flag1&0x80);
			if (!(land.flag1&0x80)) 
				blocked = true;
		} else { // go through all statics of a given x,y...
			MapStaticIterator msi(Coord_cl( x, y, 0, pBoat->pos.map ));
			staticrecord *stat;

			while ( (stat = msi.Next()) && (++loopexit < MAXLOOPS) )
			{
				msi.GetTile(&tile);
				zt=stat->zoff+tile.height;
					
				//for this part...: Bridges can be shown not valid,
				//so we will keep setting false until we hit a valid point,
				//when we hit a valid place, we'll stop, leave block as it was, 
				//if all points are invalid, block is true and we exit as normal.
				if (!(tile.flag1&0x80) && zt<=70) blocked = true;
				else if (strcmp((char*)tile.name, "water")) blocked = true;
				//if (zt>70) water = 1; // every static til with z>70 (mast height?) doesnt block, no matter what water-bit is has
			}
		}//if type....
		if ( blocked )
			return true;
	}//for c=soze
	return false;
}*/

bool cBoat::Move(UOXSOCKET s, int dir, P_ITEM pBoat)
{//Move the boat and all it's items 1 square
	int tx=0,ty=0;
	int serial;
     
	if (pBoat == NULL)
		return false;

	serial = calcserial(pBoat->moreb1, pBoat->moreb2, pBoat->moreb3, pBoat->moreb4);
	if (serial == INVALID_SERIAL) return false;
	P_ITEM pTiller = FindItemBySerial( serial );
	if(pTiller == NULL)
		return false;
	
	P_ITEM pi_p1 = FindItemBySerial( pBoat->morex );
	if(pi_p1 == NULL) 
		return false;

	P_ITEM pi_p2 = FindItemBySerial( pBoat->morey );
	if(pi_p2 == NULL) 
		return false;

	P_ITEM pHold = FindItemBySerial( pBoat->morez );
	if(pHold == NULL) 
		return false;

	Xsend(s,wppause,2);

	switch(dir&0x0F)//Which DIR is it going in?
	{
	case '\x00' : 
		ty--;
		break;
	case '\x01' : 
		tx++; 
		ty--;
		break;
	case '\x02' :
		tx++;
		break;
	case '\x03' :
		tx++;
		ty++;
		break;
	case '\x04' : 
		ty++;
		break;
	case '\x05' :
		tx--;
		ty++;
		break;
	case '\x06' : 
		tx--;
		break;
	case '\x07' : 
		tx--; 
		ty--;
		break;
	default:
		{
		  sprintf((char*)temp,"warning: Boat direction error: %i int boat %i\n",pBoat->dir&0x0F,pBoat->serial);
		  LogWarning((char*)temp);
		  break;
		}
	}

	if((pBoat->pos.x+tx<=200 || pBoat->pos.x+tx>=6000) && (pBoat->pos.y+ty<=200 || pBoat->pos.y+ty>=4900)) //bugfix LB
	{
		pBoat->type2=9;
		itemtalk(s, pTiller, "Arr, Sir, we've hit rough waters!");
		Xsend(s,restart,2);
		return false;
	}
	if(Block(pBoat,tx,ty,dir))
	{
		pBoat->type2=9;
		itemtalk(s, pTiller, "Arr, somethings in the way!");
		Xsend(s,restart,2);
		return false;
	}

	//Move all the special items
	Coord_cl desloc(tx, ty, 0);
	pBoat->moveTo(pBoat->pos + desloc);
	pTiller->moveTo(pTiller->pos + desloc);
	pi_p1->moveTo(pi_p1->pos + desloc);
	pi_p2->moveTo(pi_p2->pos + desloc);
	pHold->moveTo(pHold->pos + desloc);

    serial = pBoat->serial;
	
	unsigned int a;
	vector<SERIAL> vecEntries = imultisp.getData(pBoat->serial);
	for (a = 0; a < vecEntries.size(); a++)
	{
		P_ITEM pi = FindItemBySerial(vecEntries[a]);
		if(pi != NULL)
		{
			pi->MoveTo(pi->pos.x+=tx, pi->pos.y+=ty, pi->pos.z);
			sendinrange(pi);
		}
	}

	vecEntries.clear();
	vecEntries = cmultisp.getData(pBoat->serial);
	for (a = 0; a < vecEntries.size(); a++)
	{
		P_CHAR pc_c = FindCharBySerial(vecEntries[a]);
		if (pc_c != NULL)
		{
			pc_c->moveTo(pc_c->pos + desloc);
			teleport((pc_c));
		}
	}
	Xsend(s,restart,2);
	return true;
}

// khpae : auto sail
void cBoat::Move (UOXSOCKET s, P_ITEM pBoat) {	
	if (pBoat==NULL) {
		return;
	}
	int dx;
	int dy;
	do {
		dx = pBoat->mapPinXY[0][0] - pBoat->pos.x;
		dy = pBoat->mapPinXY[0][1] - pBoat->pos.y;
		if ((dx==0) && (dy==0)) {
			if (pBoat->mapNumPin == 1) {
				int tser = calcserial (pBoat->moreb1, pBoat->moreb2, pBoat->moreb3, pBoat->moreb4);
				P_ITEM pTiller = FindItemBySerial (tser);
				if (pTiller != NULL) {
					itemtalk (s, pTiller, "We are here, Sir.");
				}
				pBoat->autoSail = false;
				pBoat->mapNumPin = 0;
				pBoat->type2 = 9;
				return;
			}
			pBoat->mapNumPin --;
			int i;
			for (i=0; i<(pBoat->mapNumPin); i++) {
				pBoat->mapPinXY[i][0] = pBoat->mapPinXY[i+1][0];
				pBoat->mapPinXY[i][1] = pBoat->mapPinXY[i+1][1];
			}
		}
	} while ((dx==0) && (dy==0));
	int dir;
	if (dx == 0) {
		if (dy < 0) {
			dir = 0;
		} else {
			dir = 4;
		}
		if (!Move (s, dir, pBoat)) {
			pBoat->autoSail = false;
			pBoat->mapNumPin = 0;
			pBoat->type2 = 9;
		}
		return;
	} else if (dy == 0) {
		if (dx < 0) {
			dir = 6;
		} else {
			dir = 2;
		}
		if (!Move (s, dir, pBoat)) {
			pBoat->autoSail = false;
			pBoat->mapNumPin = 0;
			pBoat->type2 = 9;
		}
		return;
	}
	float slength = sqrt (static_cast<float>(dx*dx + dy*dy)); // gcc 3.x have issues doing this implicit conversion
	float fdx = static_cast<float>(dx);
	fdx /= slength;
	float fdy = static_cast<float>(dy);
	fdy /= slength;
	if (fdx < -.25) {
		dx = -1;
	} else if (fdx > .25) {
		dx = 1;
	} else {
		dx = 0;
	}
	if (fdy < -.25) {
		dy = -1;
	} else if (fdy > .25) {
		dy = 1;
	} else {
		dy = 0;
	}
	if ((dx==0) && (dy<0)) {
		dir = 0;
	} else if ((dx>0) && (dy<0)) {
		dir = 1;
	} else if ((dx>0) && (dy==0)) {
		dir = 2;
	} else if ((dx>0) && (dy>0)) {
		dir = 3;
	} else if ((dx==0) && (dy>0)) {
		dir = 4;
	} else if ((dx<0) && (dy>0)) {
		dir = 5;
	} else if ((dx<0) && (dy==0)) {
		dir = 6;
	} else {
		dir = 7;
	}
	if (!Move (s, dir, pBoat)) {
		pBoat->autoSail = false;
		pBoat->mapNumPin = 0;
		pBoat->type2 = 9;
	}
}

void cBoat::TurnStuff(P_ITEM pBoat, P_CHAR pc_i, int dir)//Turn an item that was on the boat when the boat was turned.
{
	if (pc_i == NULL) 
		return;

	int dx = pc_i->pos.x - pBoat->pos.x;
	int dy = pc_i->pos.y - pBoat->pos.y;

	mapRegions->Remove(pc_i);
        
	pc_i->pos.x = pBoat->pos.x;
	pc_i->pos.y = pBoat->pos.y;
		
	if(dir)
	{
		pc_i->pos.x+=dy*-1;
		pc_i->pos.y+=dx;
	} else {
		pc_i->pos.x+=dy;
		pc_i->pos.y+=dx*-1;
	}
	//Set then in their new cell

	mapRegions->Add(pc_i);
	teleport((pc_i));
}

void cBoat::TurnStuff(P_ITEM pBoat, P_ITEM pi, int dir)//Turn an item that was on the boat when the boat was turned.
{
	int dx = pi->pos.x - pBoat->pos.x;//get their distance x
	int dy = pi->pos.y - pBoat->pos.y;//and distance Y
    mapRegions->Remove(pi);

	pi->pos.x = pBoat->pos.x;
	pi->pos.y = pBoat->pos.y;
	if(dir)//turning right
	{
		pi->pos.x+=dy*-1;
		pi->pos.y+=dx;
	} else {//turning left
		pi->pos.x+=dy;
		pi->pos.y+=dx*-1;
	}
	
	mapRegions->Add(pi);
	sendinrange(pi);
}

void cBoat::Turn(P_ITEM pBoat, int turn)//Turn the boat item, and send all the people/items on the boat to turnboatstuff()
{
	if (pBoat == NULL) return; 
	
	int id2 = pBoat->id2 ,olddir = pBoat->dir;
	unsigned short int Send[MAXCLIENT];
	SERIAL serial;
	int a,dir, d=0;
	int oldid2 = pBoat->id2;	// khpae added for restore if turn fail !

	for (a = 0; a < now; ++a)
	{
		if (iteminrange(a, pBoat, BUILDRANGE) && perm[a])
		{
			Send[d]=a;
			Xsend(a,wppause,2);
			d++; 
		} 
		
	}
	
	//Of course we need the boat items!
	serial = calcserial(pBoat->moreb1,pBoat->moreb2,pBoat->moreb3,pBoat->moreb4);
	if(serial == INVALID_SERIAL) return;
	P_ITEM pTiller = FindItemBySerial( serial );
	if(pTiller == NULL) return;
	P_ITEM pi_p1 = FindItemBySerial( pBoat->morex );
	if(pi_p1 == NULL) return;
	P_ITEM pi_p2 = FindItemBySerial( pBoat->morey );
	if(pi_p2 == NULL) return;
	P_ITEM pi_hold = FindItemBySerial( pBoat->morez );
	if(pi_hold == NULL) return;
	
	if(turn)//Right
	{
		pBoat->dir+=2;
		id2++;
	} else {//Left
		pBoat->dir-=2;
		id2--;
	}
	if(pBoat->dir>7) pBoat->dir-=4;//Make sure we dont have any DIR errors
	//if(pBoat->dir<0) pBoat->dir+=4;
	if(id2<pBoat->more1) id2+=4;//make sure we don't have any id errors either
	if(id2>pBoat->more2) id2-=4;//Now you know what the min/max id is for :-)
	
	pBoat->id2=id2;//set the id
	
	if(pBoat->id2==pBoat->more1) pBoat->dir=0;//extra DIR error checking
	if(pBoat->id2==pBoat->more2) pBoat->dir=6;
	
	
	if( Block( pBoat, 0, 0, pBoat->dir ) )
	{
		pBoat->dir = olddir;
		pBoat->id2 = oldid2;	// khpae : restore old id2
		for( a = 0; a < d; a++ )
		{
			Xsend( Send[a], restart, 2 );
			itemtalk( Send[a], pTiller, "Arr, something's in the way!" );
		}
		return;
	}
	pBoat->id2=id2;//set the id
	
	if(pBoat->id2==pBoat->more1) pBoat->dir=0;//extra DIR error checking
	if(pBoat->id2==pBoat->more2) pBoat->dir=6;    
	
	
	
	
    serial=pBoat->serial; // lb !!!

	unsigned int ci;	
	vector<SERIAL> vecEntries = imultisp.getData(serial);
    for (ci = 0; ci < vecEntries.size(); ci++)
	{
		P_ITEM pi = FindItemBySerial(vecEntries[ci]);
		if (pi != NULL)
			TurnStuff(pBoat, pi, turn);
	}
	
	vecEntries.clear();
	vecEntries = cmultisp.getData(serial);
	for (ci = 0; ci < vecEntries.size(); ci++)
	{
		P_CHAR pc = FindCharBySerial(vecEntries[ci]);
		if (pc != NULL)
			TurnStuff(pBoat, pc, turn);
	}
	
	//Set the DIR for use in the Offsets/IDs array
	dir = (pBoat->dir&0x0F)/2;
	
	//set it's Z to 0,0 inside the boat
	
	pi_p1->MoveTo(pBoat->pos.x,pBoat->pos.y,pi_p1->pos.z);
	pi_p1->id2 = cShipItems[dir][PORT_P_C];//change the ID
	
	pi_p2->MoveTo(pBoat->pos.x,pBoat->pos.y,pi_p2->pos.z);
	pi_p2->id2=cShipItems[dir][STAR_P_C];
	
	pTiller->MoveTo(pBoat->pos.x,pBoat->pos.y, pTiller->pos.z);
	pTiller->id2=cShipItems[dir][TILLERID];
	
	pi_hold->MoveTo(pBoat->pos.x,pBoat->pos.y, pi_hold->pos.z);
	pi_hold->id2=cShipItems[dir][HOLDID];
	
	switch(pBoat->more1)//Now set what size boat it is and move the specail items
	{
	case 0x00:
	case 0x04:
		pi_p1->moveTo(pi_p1->pos + Coord_cl(iSmallShipOffsets[dir][PORT_PLANK][X], iSmallShipOffsets[dir][PORT_PLANK][Y], 0));
		pi_p2->moveTo(pi_p2->pos + Coord_cl(iSmallShipOffsets[dir][STARB_PLANK][X], iSmallShipOffsets[dir][STARB_PLANK][Y], 0));
		pTiller->moveTo(pTiller->pos + Coord_cl(iSmallShipOffsets[dir][TILLER][X], iSmallShipOffsets[dir][TILLER][Y], 0));
		pi_hold->moveTo(pi_hold->pos + Coord_cl(iSmallShipOffsets[dir][HOLD][X], iSmallShipOffsets[dir][HOLD][Y], 0));
		break;
	case 0x08:
	case 0x0C:
		pi_p1->moveTo(pi_p1->pos + Coord_cl(iMediumShipOffsets[dir][PORT_PLANK][X], iMediumShipOffsets[dir][PORT_PLANK][Y], 0) );
		pi_p2->moveTo(pi_p2->pos + Coord_cl(iMediumShipOffsets[dir][STARB_PLANK][X], iMediumShipOffsets[dir][STARB_PLANK][Y], 0) );
		pTiller->moveTo(pTiller->pos + Coord_cl(iMediumShipOffsets[dir][TILLER][X], iMediumShipOffsets[dir][TILLER][Y], 0) );
		pi_hold->moveTo(pi_hold->pos + Coord_cl(iMediumShipOffsets[dir][HOLD][X], iMediumShipOffsets[dir][HOLD][Y], 0) );
		
		break;
	case 0x10:
	case 0x14:
		pi_p1->moveTo(pi_p1->pos + Coord_cl(iLargeShipOffsets[dir][PORT_PLANK][X], iLargeShipOffsets[dir][PORT_PLANK][Y], 0) );
        pi_p2->moveTo(pi_p2->pos + Coord_cl(iLargeShipOffsets[dir][STARB_PLANK][X], iLargeShipOffsets[dir][STARB_PLANK][Y], 0 ) );
		pTiller->moveTo(pTiller->pos + Coord_cl(iLargeShipOffsets[dir][TILLER][X], iLargeShipOffsets[dir][TILLER][Y], 0 ) );
		pi_hold->moveTo(pi_hold->pos + Coord_cl(iLargeShipOffsets[dir][HOLD][X], iLargeShipOffsets[dir][HOLD][Y], 0 ) );
		
		break;
		
	default: 
		{ 
			sprintf((char*)temp,"Turnboatstuff() more1 error! more1 = %c not found!\n",pBoat->more1); 
			LogWarning((char*)temp);
		}  
	}
	
	sendinrange(pi_p1);
	sendinrange(pi_p2);
	sendinrange(pi_hold);
	sendinrange(pTiller);
	
	for (a = 0; a < d; ++a) 
	{ 
		Xsend(Send[a],restart,2);
	}
}

char cBoat::Speech(UOXSOCKET s, const QString& msg)//See if they said a command. msg must already be capitalized
{
	P_CHAR pc_currchar = currchar[s];
	P_ITEM boat = GetBoat(pc_currchar);
	if(boat == NULL) 
		return 0;
	int dir = boat->dir&0x0F;
	int serial;
	char msg2[512];

	if (s == INVALID_UOXSOCKET) 
		return 0;

	//get the tiller man's item #
	serial=calcserial(boat->moreb1, boat->moreb2, boat->moreb3, boat->moreb4);
	if ( serial == INVALID_SERIAL ) 
		return 0;
	P_ITEM tiller = FindItemBySerial(serial);
	if ( tiller == NULL ) 
		return 0;

	// khpae - command add
	if((msg.find("ONE")!= string::npos) || (msg.find("DRIFT")!=string::npos))
	{
		if (msg.find ("FORWARD LEFT") != string::npos) {
			dir -= 1;
			if (dir < 0) {
				dir += 8;
			}
			if (Move (s, dir, boat)) {
				itemtalk (s, tiller, "Aye, sir.");
			}
			boat->type2 = 9;
			return 1;
		} else if (msg.find ("FORWARD RIGHT") != string::npos) {
			dir += 1;
			if (dir > 7) {
				dir -= 8;
			}
			if (Move (s, dir, boat)) {
				itemtalk (s, tiller, "Aye, sir.");
			}
			boat->type2 = 9;
			return 1;
		} else if (msg.find ("BACKWARD RIGHT") != string::npos) {
			dir += 3;
			if (dir > 7) {
				dir -= 8;
			}
			if (Move (s, dir, boat)) {
				itemtalk (s, tiller, "Aye, sir.");
			}
			boat->type2 = 9;
			return 1;
		} else if (msg.find ("BACKWARD LEFT") != string::npos) {
			dir += 5;
			if (dir > 7) {
				dir -= 8;
			}
			if (Move (s, dir, boat)) {
				itemtalk (s, tiller, "Aye, sir.");
			}
			boat->type2 = 9;
			return 1;
		} else
		if (msg.find ("FORWARD") != string::npos) {
			if (Move (s, dir, boat)) {
				itemtalk (s, tiller, "Aye, sir.");
			}
			boat->type2 = 9;
		} else if (msg.find ("BACKWARD") != string::npos) {
			dir -= 4;
			if (dir < 0) {
				dir += 8;
			}
			if (Move (s, dir, boat)) {
				itemtalk (s, tiller, "Aye, sir.");
			}
			boat->type2 = 9;
		} else 
		if(msg.find("LEFT")!=string::npos)
		{
			dir-=2;
			if(dir<0) dir+=8;			
			if (Move(s, dir, boat)) {
				itemtalk(s, tiller, "Aye, sir.");
			}
			// khpae
			boat->type2 = 9; // stop
			return 1;

		} else if(msg.find("RIGHT")!=string::npos)
		{
			dir+=2;
			if(dir>=8) dir-=8; 			
			if (Move(s,dir,boat)) {
				itemtalk(s, tiller, "Aye, sir.");
			}
			// khpae
			boat->type2 = 9;
			return 1;
		}
	} else if (msg.find ("FORWARD RIGHT") != string::npos) {
		dir = boat->dir + 1;
		if (dir > 7) {
			dir -= 8;
		}
		if (Move (s, dir, boat)) {
			itemtalk (s, tiller, "Aye, sir.");
			boat->type2 = 2;
		} else {
			boat->type2 = 9;
		}
		return 1;
	} else if (msg.find ("FORWARD LEFT") != string::npos) {
		dir = boat->dir - 1;
		if (dir < 0) {
			dir += 8;
		}
		if (Move (s, dir, boat)) {
			itemtalk (s, tiller, "Aye, sir.");
			boat->type2 = 8;
		} else {
			boat->type2 = 9;
		}
		return 1;
	} else if (msg.find ("BACKWARD RIGHT") != string::npos) {
		dir = boat->dir + 3;
		if (dir > 7) {
			dir -= 8;
		}
		if (Move (s, dir, boat)) {
			itemtalk (s, tiller, "Aye, sir.");
			boat->type2 = 4;
		} else {
			boat->type2 = 9;
		}
		return 1;
	} else if (msg.find ("BACKWARD LEFT") != string::npos) {
		dir = boat->dir - 3;
		if (dir < 0) {
			dir += 8;
		}
		if (Move (s, dir, boat)) {
			itemtalk (s, tiller, "Aye, sir.");
			boat->type2 = 6;
		} else {
			boat->type2 = 9;
		}
		return 1;
	} else
	if((msg.find("FORWARD")!= string::npos) || (msg.find("UNFURL SAIL")!=string::npos))
	{
		if (Move(s,dir, boat)) {
			itemtalk(s, tiller, "Aye, sir.");
			boat->type2=1;//Moving : khpae - moving the same direction of the boat
		} else {
			boat->type2 = 9;	// stop
		}
		return 1;
	} else if(msg.find("BACKWARD")!= string::npos)
	{
		if(dir >= 4) dir-=4;
		else dir+=4;
		if (Move(s,dir, boat)) {
			itemtalk(s, tiller, "Aye, sir.");
			boat->type2=5;//Moving backward // khpae : changed from 2 to 5
		} else {
			boat->type2 = 9;
		}
		return 1;
	}
	else if((msg.find("STOP")!=string::npos) || (msg.find("FURL SAIL")!=string::npos))
	{ 
		boat->type2=9;
		if (boat->autoSail) {
			boat->autoSail = false;
		}
		itemtalk(s, tiller, "Aye, sir."); 
		return 1;
	}//Moving is type2 1 and 2, so stop is 0 :-)
	
	else if(((msg.find("TURN")!=string::npos) && ((msg.find("AROUND")!=string::npos) || (msg.find("LEFT")!=string::npos) || (msg.find("RIGHT")!=string::npos)))
		|| (msg.find("PORT")!=string::npos) || (msg.find("STARBOARD")!=string::npos) || (msg.find("COME ABOUT")!=string::npos))
	{
		if((msg.find("RIGHT")!=string::npos) || (msg.find("STARBOARD")!=string::npos)) 
		{
			// khpae
			dir+=2; if(dir>7) dir-=8;
			int tx=0,ty=0;
/*
	        switch(dir&0x0F) // little reminder for myself: move this swtich to a function to have less code ... LB
			{
	           case '\x00' : 
		       ty--;
		       break;
	           case '\x01' : 
		       tx++; 
		       ty--;
		       break;
	           case '\x02' :
		       tx++;
		       break;
	           case '\x03' :
		       tx++;
		       ty++;
		       break;
	           case '\x04' : 
		       ty++;
		       break;
	           case '\x05' :
		       tx--;
		       ty++;
		       break;
	           case '\x06' : 
		       tx--;
		       break;
	           case '\x07' : 
		       tx--; 
		       ty--;
		       break;
			}
*/

//			if (!Block(boat,tx,ty,dir))
//			{
			  Turn(boat,1);
//			  itemtalk(s, tiller, "Aye, sir.");
//			  return 1;
//			} else {
				boat->type2 = 9;
//			    itemtalk(s, tiller, "Arr,somethings in the way");
				return 1;
//			}
		}
		else if((msg.find("LEFT")!=string::npos) || (msg.find("PORT")!=string::npos)) 
		{
			// khpae
			dir-=2; if(dir<0) dir+=8;
			int tx=0,ty=0;
/*
	        switch(dir&0x0F)
			{
	           case '\x00' : 
		       ty--;
		       break;
	           case '\x01' : 
		       tx++; 
		       ty--;
		       break;
	           case '\x02' :
		       tx++;
		       break;
	           case '\x03' :
		       tx++;
		       ty++;
		       break;
	           case '\x04' : 
		       ty++;
		       break;
	           case '\x05' :
		       tx--;
		       ty++;
		       break;
	           case '\x06' : 
		       tx--;
		       break;
	           case '\x07' : 
		       tx--; 
		       ty--;
		       break;
			}
*/

//			if (!Block(boat,tx,ty,dir))
//			{
			  Turn(boat,0);
//			  itemtalk(s, tiller, "Aye, sir.");
//			  return 1;
//			} else
//			{
				boat->type2 = 9;
//				itemtalk(s, tiller, "Arr,somethings in the way");
				return 1;
//			}
		}
		else if((msg.find("COME ABOUT")!=string::npos) || (msg.find("AROUND")!=string::npos))
		{
			Turn(boat, 1);
			Turn(boat, 1);
			itemtalk(s, tiller, "Aye, sir.");
			return 1;
		}
	}
	else if(msg.find("SET NAME")!=string::npos)
	{
		tiller->name = "a ship named ";
		tiller->name += msg2+8;
		return 1;
	}
	// khpae - bugfix
	else if (msg.find ("LEFT") != string::npos) {
		dir-=2;
		if (dir<0) dir+=8;  
		if (Move (s, dir, boat)) {
			itemtalk(s, tiller, "Aye, sir.");
			boat->type2 = 7;
		} else {
			boat->type2 = 9;
		}
		return 1;
	} else if (msg.find ("RIGHT") != string::npos) {
		dir+=2;
		if (dir>=8) dir-=8;
		if (Move (s,dir,boat)) {
			itemtalk (s, tiller, "Aye, sir.");
			boat->type2 = 3;
		} else {
			boat->type2 = 9;
		}
		return 1;
	}
	return 0;
}

// khpae - new blocking check : boat-boat collision correction will be added soon
bool cBoat::Block (P_ITEM pBoat, short int xmove, short int ymove, int dir) {
	int ser = calcserial (pBoat->moreb1, pBoat->moreb2, pBoat->moreb3, pBoat->moreb4);
	if (ser == INVALID_SERIAL) {
		return true;
	}
	P_ITEM t = FindItemBySerial (ser);
	P_ITEM p1 = FindItemBySerial (pBoat->morex);
	P_ITEM p2 = FindItemBySerial (pBoat->morey);
	P_ITEM h = FindItemBySerial (pBoat->morez); 
	if ((t==NULL) || (p1==NULL) || (p2==NULL) || (h==NULL)) {
		return true;
	}
	int length = getLength (pBoat->more1);
	if (length < 0) {
		return true;
	}
	int x0, y0, dx, dy, x1, y1;
	int width = 5;
	bool blocked = false;
	switch (pBoat->dir & 0x0F) {
		case 0:
			x0 = h->pos.x-2;
			y0 = h->pos.y-1;
			dx = width;
			dy = length;
			break;
		case 4:
			x0 = t->pos.x-2;
			y0 = t->pos.y-1;
			dx = width;
			dy = length;
			break;
		case 2:
			x0 = t->pos.x-1;
			y0 = t->pos.y-2;
			dx = length;
			dy = width;
			break;
		case 6:
			x0 = h->pos.x-1;
			y0 = h->pos.y-2;
			dx = length;
			dy = width;
			break;
		default:
			blocked = true;
			break;
	}
	if ((xmove==0) && (ymove==0)) {
		switch (dir) {
			case 2:
				x0 = pBoat->pos.x - length / 2 + 1;
				y0 = pBoat->pos.y - 2;
				dx = length;
				dy = width;
				break;
			case 6:
				x0 = pBoat->pos.x - length / 2;// - 1;
				y0 = pBoat->pos.y - 2;
				dx = length;
				dy = width;
				break;
			case 0:
				x0 = pBoat->pos.x - 2;
				y0 = pBoat->pos.y - length / 2;// - 1;
				dx = width;
				dy = length;
				break;
			case 4:
				x0 = pBoat->pos.x - 2;
				y0 = pBoat->pos.y - length / 2 + 1;
				dx = width;
				dy = length;
				break;
			default:
				blocked = true;
				break;
		}
	}
	if (blocked) {
		return true;
	}
	x1 = x0 + xmove;
	y1 = y0 + ymove;

	int sz, type;
	map_st map;
	land_st land;
	tile_st tile;
	int x, y;
	int zt;
	int loopexit = 0;

	for (x=x1; x<(x1+dx); x++) {
		for (y=y1; y<(y1+dy); y++) {
			if ((x>=x0) && (x<(x0+dx)) && (y>=y0) && (y<(y0+dy)) && ((xmove!=0) || (ymove!=0))) {
				continue;
			}
			if ((xmove==0) && (ymove==0) && (abs (x-pBoat->pos.x)<3) && (abs (y-pBoat->pos.y)<3)) {
				continue;
			}
			sz = Map->StaticTop (Coord_cl (x, y, pBoat->pos.z, pBoat->pos.map));
			if (sz == illegal_z) {
				type = 0;
			} else {
				type = 1;
			}
			if (type == 0) {
				map = Map->SeekMap (Coord_cl (x, y, 0, pBoat->pos.map));
				Map->SeekLand (map.id, &land);
				if (!(land.flag1 & 0x80)) {
					blocked = true;
				}
			} else {
				MapStaticIterator msi(Coord_cl( x, y, 0, pBoat->pos.map ));
				staticrecord *stat;
				while ((stat = msi.Next()) && (++loopexit < MAXLOOPS)) {
					msi.GetTile (&tile);
					zt = stat->zoff + tile.height;
					if ((!(tile.flag1 & 0x80)) && (zt <= 70)) {
						blocked = true;
					} else if (strcmp((char*)tile.name, "water") != 0) {
						blocked = true;
					}
				}
			}
			cRegion::RegionIterator4Items ri (Coord_cl (x, y, 0, pBoat->pos.map));
			for (ri.Begin (); !ri.atEnd (); ri++) {
				P_ITEM pi = ri.GetData ();
				if ((pi != NULL) && (pi->pos.x==x) && (pi->pos.y==y)) {
					blocked = true;
					break;
				}
			}
			if (blocked) {
				break;
			}
		}
		if (blocked) {
			break;
		}
	}
	return blocked;
}


int cBoat::getLength (unsigned char t) {
	int length = -1;
	switch (t) {
		case 0x00:
		case 0x04:
			length = 11;
			break;
		case 0x08:
		case 0x0C:
			length = 13;
			break;
		case 0x10:
		case 0x14:
			length = 15;
			break;
	}
	return length;
}

bool cBoat::LeaveBoat (UOXSOCKET s, P_ITEM pi_plank) {
	P_CHAR pc_cs = currchar[s];
	P_ITEM pBoat = GetBoat (pc_cs);
	if ((pc_cs == NULL) || (pi_plank == NULL) || (pBoat == NULL)) {
		return false;
	}
	UI16 x, y, x0, y0, x1, y1, dx, dy;
	switch (pBoat->dir & 0x0F) {
		case 0:
		case 4:
			x0 = (pBoat->pos.x > pi_plank->pos.x) ? pi_plank->pos.x-1 : pi_plank->pos.x+1;
			y0 = pi_plank->pos.y - 2;
			x1 = (pBoat->pos.x > pi_plank->pos.x) ? pi_plank->pos.x-2 : pi_plank->pos.x+2;
			y1 = y0 + 5;
			break;
		case 2:
		case 6:
			x0 = pi_plank->pos.x - 2;
			y0 = (pBoat->pos.y > pi_plank->pos.y) ? pi_plank->pos.y-1 : pi_plank->pos.y+1;
			x1 = x0 + 5;
			y1 = (pBoat->pos.y > pi_plank->pos.y) ? pi_plank->pos.y-2 : pi_plank->pos.y+2;
			break;
		default:
			return false;
	}
	UI16 tmp;
	if (x0 > x1) {
		tmp = x0;
		x0 = x1;
		x1 = tmp;
	}
	if (y0 > y1) {
		tmp = y0;
		y0 = y1;
		y1 = tmp;
	}
	signed char sz, mz, z;
	bool check = false;
	land_st landt;
	map_st mapt;
	tile_st tilet;
	int loopexit = 0;
	for (x=x0; x<=x1; x++) {
		for (y=y0; y<=y1; y++) {
			sz = Map->StaticTop (Coord_cl (x,y,0, 0));
			mz = Map->MapElevation (Coord_cl (x,y,0, 0));
			if ((sz == illegal_z) && (mz != -5)) {
				z = mz;
				check = true;
				break;
			} else if ((sz != illegal_z) && (sz != -5)) {
				z = sz;
				check = true;
				break;
			}
		}
		if (check) {
			break;
		}
	}
	if (!check) {
		return false;
	}
	UI16 a;
	vector<SERIAL> vecCown = cownsp.getData (pc_cs->serial);
	for (a=0; a<vecCown.size (); a++) {
		P_CHAR pc_b = FindCharBySerial (vecCown[a]);
		if (pc_b != NULL) {
			if (pc_b->isNpc () && pc_cs->Owns (pc_b) && inrange1p (pc_cs, pc_b)) {
				pc_b->MoveTo (x, y, z);
				pc_b->multis = INVALID_SERIAL;
			}
			teleport (pc_b);
		}
	}
	pc_cs->MoveTo (x, y, z);
	teleport (pc_cs);
	cmultisp.remove (pc_cs->multis, pc_cs->serial);
	pc_cs->multis = INVALID_SERIAL;
	return true;
}

// khpae - make deed from a boat
void cBoat::deedBoat (UOXSOCKET s, P_ITEM pii) {
	P_CHAR pc = currchar[s];
	if (pc == NULL) {
		return;
	}
	P_ITEM pBoat = FindItemBySerial (calcserial (pii->more1, pii->more2, pii->more3, pii->more4));
	if (pBoat == NULL) {
		return;
	}
	// if player is in boat
	if (pc->multis != INVALID_SERIAL) {
		sysmessage (s, "You must leave the boat to deed it.");
		return;
	}
	// check the player has the boat key
	P_ITEM bpack = Packitem (pc);
	if (bpack == NULL) {
		return;
	}
	vector<SERIAL> vpack = contsp.getData (bpack->serial);
	P_ITEM pi = NULL;
	bool found = false;
	int in;
	for (in=0; in<vpack.size (); in++) {
		pi = FindItemBySerial (vpack[in]);
		if (pi == NULL) {
			contsp.remove (bpack->serial, vpack[in]);
			continue;
		}
		if (pi->type == 7) {
			SERIAL si = calcserial (pi->more1, pi->more2, pi->more3, pi->more4);
			if (si == pBoat->serial) {
				found = true;
				break;
			}
		}
	}
	if ((!found) || (pi==NULL)) {
		sysmessage (s, "You don't have the boat key.");
		return;
	}
	// if any pcs / npcs / items are in the boat, it cannot be deed.
	vector<SERIAL> vitem = imultisp.getData (pBoat->serial);
	if (vitem.size () > 0) {
		sysmessage (s, "You can only deed with empty boat (remove items).");
		return;
	}
	vector<SERIAL> vchar = cmultisp.getData (pBoat->serial);
	if (vchar.size () > 0) {
		sysmessage (s, "You can only deed with empty boat (remove pc/npcs.");
		return;
	}
	// add deed
	P_ITEM bdeed = Items->SpawnItemBackpack2 (s, pBoat->madewith, 0);
	if (bdeed == NULL) {
		sysmessage (s, "There's problem with deed boat. Please contact Game Master.");
		return;
	}
	// remove key
	Items->DeleItem (pi);
	// remove all other keys for this ship
	AllItemsIterator iter_items;
	for (iter_items.Begin (); !iter_items.atEnd (); ++iter_items) {
		P_ITEM boatKey = iter_items.GetData();
		if ((boatKey->type==7) && (calcserial (boatKey->more1, boatKey->more2, boatKey->more3, boatKey->more4)==pBoat->serial)) {
			--iter_items;
			Items->DeleItem (boatKey);
		}
	}
	// tiller
	Items->DeleItem (pii);
	// left plank
	P_ITEM pi2 = FindItemBySerial (pBoat->morex);
	if (pi2 != NULL) {
		Items->DeleItem (pi2);
	}
	// right plank
	pi2 = FindItemBySerial (pBoat->morey);
	if (pi2 != NULL) {
		Items->DeleItem (pi2);
	}
	// hold
	pi2 = FindItemBySerial (pBoat->morez);
	if (pi2 != NULL) {
		Items->DeleItem (pi2);
	}
	Items->DeleItem (pBoat);
	pc->multis = INVALID_SERIAL;
	sysmessage (s, "You deed the boat.");
}

// khpae : initial setup for auto sailing
void cBoat::setAutoSail (UOXSOCKET s, P_ITEM pMap, P_ITEM pTiller) {
	P_CHAR pc = currchar[s];
	if (pc == NULL) {
		return;
	}
	if (!pMap->mapNumPin) {
		itemtalk (s, pTiller, "Sir, there's no ship cource.");
		return;
	}
	if (pc->multis == INVALID_SERIAL) {
		sysmessage (s, "You must on the boat to do that.");
		return;
	}
	int bserial = calcserial (pTiller->more1, pTiller->more2, pTiller->more3, pTiller->more4);
	if (bserial != pc->multis) {
		sysmessage (s, "You must on the boat to do that.");
		return;
	}
	P_ITEM pBoat = FindItemBySerial (bserial);
	if (pBoat == NULL) {
		return;
	}
	int x0 = (pMap->more1<<8) | pMap->more2;
	int y0 = (pMap->more3<<8) | pMap->more4;
	int x1 = (pMap->moreb1<<8) | pMap->moreb2;
	int y1 = (pMap->moreb3<<8) | pMap->moreb4;
	int width = 134 * (pMap->morez + 1);
	int i, posx, posy;
	for (i=0; i<pMap->mapNumPin; i++) {
		posx = x0 + pMap->mapPinXY[i][0]*(x1-x0) / width;
		posy = y0 + pMap->mapPinXY[i][1]*(y1-y0) / width;
		pBoat->mapPinXY[i][0] = (unsigned short)posx;
		pBoat->mapPinXY[i][1] = (unsigned short)posy;
	}
	itemtalk (s, pTiller, "Aye, Sir.");
	pBoat->autoSail = true;
	pBoat->mapNumPin = pMap->mapNumPin;
}
