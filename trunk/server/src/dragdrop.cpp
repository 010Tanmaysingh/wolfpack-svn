//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001-2003 by holders identified in authors.txt
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
//	Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================

#include "basics.h"
#include "tilecache.h"
#include "speech.h"
#include "itemid.h"
#include "sectors.h"
#include "srvparams.h"
#include "skills.h"
#include "maps.h"
#include "network.h"
#include "network/uosocket.h"
#include "network/uorxpackets.h"
#include "network/uotxpackets.h"
#include "multis.h"
#include "dragdrop.h"
#include "player.h"
#include "npc.h"
#include "world.h"
#include "inlines.h"

#undef  DBGFILE
#define DBGFILE "dragdrop.cpp"


// New Class implementation
void DragAndDrop::grabItem( cUOSocket *socket, cUORxDragItem *packet )
{
	// Get our character
	P_PLAYER pChar = socket->player();
	if( !pChar )
		return;

	UINT32 weight = pChar->weight();

	// Fetch the grab information
	UI16 amount = packet->amount();
	if( !amount )
		amount = 1;

	P_ITEM pItem = FindItemBySerial( packet->serial() );

	// If it's an invalid pointer we can't even bounce
	if( !pItem )
		return;

	// Are we already dragging an item ?
	// Bounce it and reject the move
	// (Logged out while dragging an item)
	if( socket->dragging() )
	{
		socket->bounceItem( socket->dragging(), BR_ALREADY_DRAGGING );
		return;
	}

	if( pItem->onPickup( pChar ) )
		return;

	if( pChar->onPickup( pItem ) )
		return;

	// Do we really want to let him break his meditation
	// When he picks up an item ?
	// Maybe a meditation check here ?!?
	pChar->disturbMed(); // Meditation

	P_CHAR itemOwner = pItem->getOutmostChar();

	// Try to pick something out of another characters posessions
	if( !pChar->isGM() && itemOwner && ( itemOwner != pChar ) && ( itemOwner->objectType() == enNPC && dynamic_cast<P_NPC>(itemOwner)->owner() != pChar ) )
	{
		socket->bounceItem( pItem, BR_BELONGS_TO_SOMEONE_ELSE );
		return;
	}

	// Check if the user can grab the item
	if( !pChar->canPickUp( pItem ) )
	{
		socket->bounceItem( pItem, BR_CANNOT_PICK_THAT_UP );
		return;
	}

	// The user can't see the item
	// Basically thats impossible as the socket should deny moving the item
	// if it's not in line of sight but to prevent exploits
	/*if( !line_of_sight( socket->socket(), pChar->pos, pItem->pos, TREES_BUSHES|WALLS_CHIMNEYS|DOORS|ROOFING_SLANTED|FLOORS_FLAT_ROOFING|LAVA_WATER ) )
	{
		socket->sysMessage( "You can't see the item." );
		bounceItem( socket, pItem, true );
		return;
	}*/

	P_ITEM outmostCont = pItem->getOutmostItem();

	// If the top-most container ( thats important ) is a corpse 
	// and looting is a crime, flag the character criminal.
	if( !pChar->isGM() && outmostCont && outmostCont->corpse() )
	{
		// For each item we take out we loose carma
		// if the corpse is innocent and not in our guild
		bool sameGuild = true;//( GuildCompare( pChar, outmostCont->owner() ) != 0 );

		if( outmostCont->hasTag( "notority" ) && outmostCont->getTag( "notority" ).toInt() == 1 && 
			!pChar->Owns( outmostCont ) && !sameGuild )
		{
//			pChar->karma -= 5;
			pChar->setKarma( pChar->karma() - 5 );
			pChar->setCriminalTime( uiCurrentTime + SrvParams->crimtime() * MY_CLOCKS_PER_SEC );
			socket->sysMessage( tr("You lost some karma.") );
		}
	}

	// Check if the item is too heavy
	//if( !pc_currchar->isGMorCounselor() )
	//{
	//} << Deactivated (DarkStorm)

	// ==== Grabbing the Item is allowed here ====
	
	// Send the user a pickup sound if we're picking it up
	// From a container/paperdoll
	if( !pItem->isInWorld() )
		socket->soundEffect( 0x57, pItem );
	
	// If we're picking up a specific amount of what we got
	// Take that into account
	if( amount < pItem->amount() )
	{
		UI32 pickedAmount = QMIN( amount, pItem->amount() );

		// We only have to split if we're not taking it all
		if( pickedAmount != pItem->amount() )
		{
			P_ITEM splitItem = new cItem( *pItem ); // Create a new item to pick that up
			splitItem->setSerial( World::instance()->findItemSerial() );
			splitItem->setAmount( pItem->amount() - pickedAmount );
			P_ITEM pContainer = dynamic_cast<P_ITEM>(pItem->container());
			if ( pContainer )
				pContainer->addItem( splitItem, false );
			splitItem->SetOwnSerial( pItem->ownSerial() );
			splitItem->setSpawnRegion( pItem->spawnregion() );

			// He needs to see the new item
			splitItem->update();

			// If we're taking something out of a spawn-region it's spawning "flag" is removed isn't it?
			pItem->setSpawnRegion( (char*)0 );
			pItem->setAmount( pickedAmount );
		}
	}
	
	// *normally* we should exclude the dragging socket here. but it works so as well.
	pItem->removeFromView( true );

	// Remove it from the World if it is in world, otherwise remove it from it's current container
	if( pItem->isInWorld() )
		MapObjects::instance()->remove( pItem );
	else
		pItem->removeFromCont( true );

	// Remove eventual item-bonusses if we're unequipping something
	if( pItem->container() && pItem->container()->isChar() ) 
	{
		P_CHAR wearer = dynamic_cast<P_CHAR>( pItem->container() );

		// resend the stat window
		if( wearer && wearer->objectType() == enPlayer )
		{
			P_PLAYER pp = dynamic_cast<P_PLAYER>(wearer);
			if( pp->socket() )
				pp->socket()->sendStatWindow();
		}
	}

	// The item was in a multi
	if( pItem->multis() != INVALID_SERIAL )
	{
		cMulti* pMulti = dynamic_cast< cMulti* >( FindItemBySerial( pItem->multis() ) );
		if( pMulti )
			pMulti->removeItem( pItem );
	}
	
	pChar->addItem( cBaseChar::Dragging, pItem );

	if( weight != pChar->weight() )
		socket->sendStatWindow();
}

// Tries to equip an item
// if that fails it tries to put the item in the users backpack
// if *that* fails it puts it at the characters feet
// That works for NPCs as well
void equipItem( P_CHAR wearer, P_ITEM item )
{
	tile_st tile = TileCache::instance()->getTile( item->id() );

	// User cannot wear the item
	if( tile.layer == 0 )
	{
		if( wearer->objectType() == enPlayer )
		{
			P_PLAYER pp = dynamic_cast<P_PLAYER>(wearer);
			if( pp->socket() )
				pp->socket()->sysMessage( tr( "You cannot wear that item.") );
		}

		item->toBackpack( wearer );
		return;
	}

	cBaseChar::ItemContainer container = wearer->content();
	cBaseChar::ItemContainer::const_iterator it(container.begin());
	for( ; it != container.end(); ++it )
	{
		P_ITEM equip = *it; 
		
		// Unequip the item and free the layer that way
		if( equip && ( equip->layer() == tile.layer ) )
			equip->toBackpack( wearer );
	}

	// *finally* equip the item
	wearer->addItem( static_cast<cBaseChar::enLayer>(item->layer()), item );
}

void DragAndDrop::bounceItem( cUOSocket* socket, P_ITEM pItem, bool denyMove )
{
	// Reject the move of the item
	socket->bounceItem( pItem, BR_NO_REASON );

	// If the Client is *not* dragging the item we don't need to reset it to it's original location
	if( !socket->dragging() )
		return;

	// Sends the item to the backpack of char (client)
	pItem->toBackpack( socket->player() );

	// When we're dropping the item to the ground let's play a nice sound-effect
	// to all in-range sockets
	if( pItem->isInWorld() )
	{
		for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
			if( mSock->inRange( socket ) )
				mSock->soundEffect( 0x42, pItem );
	}
	else
		socket->soundEffect( 0x57, pItem );
}

void DragAndDrop::equipItem( cUOSocket *socket, cUORxWearItem *packet )
{
	P_ITEM pItem = FindItemBySerial( packet->serial() );
	P_CHAR pWearer = FindCharBySerial( packet->wearer() );

	if( !pItem || !pWearer )
		return;

	P_PLAYER pChar = socket->player();

	// We're dead and can't do that
	if( pChar->isDead() )
	{
		socket->clilocMessage( 0x7A4D5, "", 0x3b2 ); // You can't do that when you're dead.
		socket->bounceItem( pItem, BR_NO_REASON );
		return;
	}

	// No Special Layer Equipping
	if( ( packet->layer() > cBaseChar::InnerLegs || packet->layer() <= cBaseChar::TradeWindow ) && !pChar->isGM() )
	{
		socket->sysMessage( tr( "You can't equip on that layer." ) );
		socket->bounceItem( pItem, BR_NO_REASON );
		return;
	}

	// Our target is dead
	if( ( pWearer != pChar ) && pWearer->isDead() )
	{
		socket->sysMessage( tr( "You can't equip dead players." ) );
		socket->bounceItem( pItem, BR_NO_REASON );
		return;
	}

	// Only GM's can equip other People
	if( pWearer != pChar && !pChar->isGM() )
	{
		P_NPC pNpc = dynamic_cast< P_NPC >( pWearer );
		
		// But we are allowed to equip our own humans
		if( !pNpc || ( pNpc->owner() != pChar && pWearer->isHuman() ) )
			socket->sysMessage( tr( "You can't equip other players." ) );

		socket->bounceItem( pItem, BR_NO_REASON );
		return;
	}

	// Get our tile-information
	tile_st pTile = TileCache::instance()->getTile( pItem->id() );

	// Is the item wearable ? ( layer == 0 | equip-flag not set )
	// Multis are not wearable are they :o)
	if( pTile.layer == 0 || !( pTile.flag3 & 0x40 ) || pItem->isMulti() )
	{
		socket->sysMessage( tr( "This item cannot be equipped." ) );
		socket->bounceItem( pItem, BR_NO_REASON );
		return;
	}

	// Check the Script for it
	if( pItem->onWearItem( pChar, pWearer, packet->layer() ) )
	{
		socket->bounceItem( pItem, BR_NO_REASON );
		return;
	}

	// Males can't wear female armor
	if( ( pChar->bodyID() == 0x0190 ) && ( pItem->id() >= 0x1C00 ) && ( pItem->id() <= 0x1C0D ) )
	{
		socket->sysMessage( tr( "You cannot wear female armor." ) );
		socket->bounceItem( pItem, BR_NO_REASON );
		return;
	}

	// Needs a check (!)
	// Checks for equipment on the same layer
	// If there is any it tries to unequip it
	// If that fails it cancels
	// we also need to check if there is a twohanded weapon if we want to equip another weapon.
	UI08 layer = pTile.layer;

	bool twohanded = false;
	P_ITEM equippedLayerItem = pWearer->atLayer( static_cast<cBaseChar::enLayer>(layer) );
	if ( equippedLayerItem )
		twohanded = equippedLayerItem->twohanded();

	if( twohanded && ( layer == 1 || layer == 2 ) )
	{
		socket->sysMessage( tr("You can't hold another item while wearing a twohanded weapon!") );
		socket->bounceItem( pItem, BR_NO_REASON );
		return;
	}

	// we're equipping so we do the check
	if( equippedLayerItem )
	{ 
		if( pChar->canPickUp( equippedLayerItem ) )
		{
			equippedLayerItem->toBackpack( pWearer );
		}
		else
		{
			socket->sysMessage( tr( "You can't wear another item there!" ) );
			socket->bounceItem( pItem, BR_NO_REASON );
			return;
		}
	}

	// At this point we're certain that we can wear the item
	pWearer->addItem( static_cast<cBaseChar::enLayer>(pTile.layer), pItem );

	if( pWearer->objectType() == enPlayer )
	{
		P_PLAYER pp = dynamic_cast<P_PLAYER>(pWearer);
		if( pp->socket() )
			pp->socket()->sendStatWindow();
	}

	// I don't think we need to remove the item
	// as it's only visible to the current char
	// And he looses contact anyway

	// Build our packets
	cUOTxCharEquipment wearItem;
	wearItem.fromItem( pItem );

	cUOTxSoundEffect soundEffect;
	soundEffect.setSound( 0x57 );
	soundEffect.setCoord( pWearer->pos() );

	// Send to all sockets in range
	// ONLY the new equipped item and the sound-effect
	for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		if( mSock->player() && ( mSock->player()->dist( pWearer ) <= mSock->player()->visualRange() ) )
		{
			mSock->send( &wearItem );
			mSock->send( &soundEffect );
		}
	}
}

void DragAndDrop::dropItem( cUOSocket *socket, cUORxDropItem *packet )
{
	P_PLAYER pChar = socket->player();

	if( !pChar )
		return;

	// Get the data
	SERIAL contId = packet->cont();

	Coord_cl dropPos = pChar->pos(); // plane
	dropPos.x = packet->x();
	dropPos.y = packet->y();
	dropPos.z = packet->z();

	// Get possible containers
	P_ITEM pItem = FindItemBySerial( packet->serial() );
	
	if( !pItem )
		return;

	P_ITEM iCont = FindItemBySerial( packet->cont() );
	P_CHAR cCont = FindCharBySerial( packet->cont() );

	// >> SEE LORD BINARIES DROPFIX <<

	// A completely invalid Drop packet
	if( !iCont && !cCont && ( dropPos.x == 0xFFFF ) && ( dropPos.y == 0xFFFF ) && ( (unsigned char)dropPos.z == 0xFF ) )
	{
		socket->bounceItem( pItem, BR_NO_REASON );
		return;
	}

	UINT32 weight = pChar->weight();

	// Item dropped on Ground
	if( !iCont && !cCont )
		dropOnGround( socket, pItem, dropPos );

	// Item dropped on another item
	else if( iCont )
		dropOnItem( socket, pItem, iCont, dropPos );

	// Item dropped on char
	else if( cCont )
		dropOnChar( socket, pItem, cCont );

	// Handle the sound-effect
	if( pItem->id() == 0xEED )
		pChar->goldSound( pItem->amount() );

	// Update our weight.
	if( weight != pChar->weight() )
		socket->sendStatWindow();
}

void DragAndDrop::dropOnChar( cUOSocket *socket, P_ITEM pItem, P_CHAR pOtherChar )
{
	// Three possibilities:
	// If we're dropping it on ourself: packintobackpack
	// If we're dropping it on some other player: trade-window
	// If we're dropping it on some NPC: checkBehaviours
	// If not handeled: Equip the item if the NPC is owned by us

	P_CHAR pChar = socket->player();

	if( pItem->onDropOnChar( pOtherChar ) )
	{
		// Still dragging? Bounce!
		if( socket->dragging() == pItem )
			socket->bounceItem( pItem, BR_NO_REASON );

		return;
	}

	if( pOtherChar->onDropOnChar( pItem ) )
	{
		// Still dragging? Bounce!
		if( socket->dragging() == pItem )
			socket->bounceItem( pItem, BR_NO_REASON );

		return;
	}

	// Dropped on ourself
	if( pChar == pOtherChar )
	{
		pItem->toBackpack( pChar );
		return;
	}
	
	// Are we in range of our target
	if( !pChar->inRange( pOtherChar, 3 ) )
	{
		socket->bounceItem( pItem, BR_OUT_OF_REACH );
		return;
	}

	// Can wee see our target
	if( !pChar->pos().lineOfSight( pOtherChar->pos() ) )
	{
		socket->bounceItem( pItem, BR_OUT_OF_SIGHT );
		return;
	}

	// Open a secure trading window
	if( pOtherChar->objectType() == enPlayer && dynamic_cast<P_PLAYER>(pOtherChar)->socket() )
	{
		dynamic_cast<P_PLAYER>(pChar)->onTradeStart( dynamic_cast<P_PLAYER>(pOtherChar), pItem );
		// Check if we're already trading, 
		// if not create a new window
		/*
		P_ITEM tradeWindow = pChar->atLayer( cBaseChar::TradeWindow );

		//if( !tradeWindow )
		//	tradeWindow = Trade->tradestart( client->socket(), pOtherChar );
		socket->bounceItem( pItem, BR_NO_REASON );
		socket->sysMessage( "Trading is disabled" );
		return;

		tradeWindow->addItem( pItem, false, false );
		pItem->setPos( Coord_cl(rand() % 60, rand() % 60, 9) );
		pItem->removeFromView( false );
		pItem->update();
		*/
		return;

	}

	// Dropping based on AI Type
	/*switch( pOtherChar->npcaitype() )
	{
	case 4:
		dropOnGuard( client, pItem, pOtherChar );
		break;
	case 5:
		dropOnBeggar( client, pItem, pOtherChar );
		break;
	case 8:
		dropOnBanker( client, pItem, pOtherChar );
		break;
	case 19:
		dropOnBroker( client, pItem, pOtherChar );
		break;
	};

	// Try to train - works for any NPC
	if( pOtherChar->cantrain() )
		if( pChar->trainer() == pOtherChar->serial )
			dropOnTrainer( client, pItem, pOtherChar );
		else
			pOtherChar->talk( "You need to tell me what you want to learn first" );*/

	// Finally lets check if it is simple food
	if( pItem->type() == 14 )
	{
		dropFoodOnChar( socket, pItem, pOtherChar );
		return;
	}

	socket->sysMessage( tr("The character does not seem to want the item.") );
	socket->bounceItem( pItem, BR_NO_REASON );
	return;
}

void DragAndDrop::dropOnGround( cUOSocket *socket, P_ITEM pItem, const Coord_cl &pos )
{
	P_PLAYER pChar = socket->player();

	// Check if the destination is in line of sight
	if( !pChar->pos().lineOfSight( pos, false ) )
	{
		socket->bounceItem( pItem, BR_OUT_OF_SIGHT );
		return;
	}

	if( !pChar->canPickUp( pItem ) )
	{
		socket->bounceItem( pItem, BR_CANNOT_PICK_THAT_UP );
		return;
	}

	if( pItem->onDropOnGround( pos ) )
	{
		// We're still dragging something
		if( socket->dragging() )
			socket->bounceItem( socket->dragging(), BR_NO_REASON );

		return;
	}

	pItem->removeFromCont();
	pItem->moveTo( pos );
	pItem->update();

	// Multi handling
	// Has it been dropped into a multi
	cMulti* pMulti = cMulti::findMulti( pos );
	if( pMulti )
	{
		pMulti->addItem( pItem );
	}
}

inline char calcSpellId( cItem *item )
{
	tile_st tile = TileCache::instance()->getTile( item->id() );

	if( tile.unknown1 == 0 )
		return -1;
	else
		return tile.unknown1 - 1;
}

void DragAndDrop::dropOnItem( cUOSocket *socket, P_ITEM pItem, P_ITEM pCont, const Coord_cl &dropPos )
{
	P_PLAYER pChar = socket->player();
	
	if( pItem->isMulti() )
	{
		socket->sysMessage( tr( "You cannot put houses in containers" ) );
		cUOTxBounceItem bounce;
		bounce.setReason( BR_NO_REASON );
		socket->send( &bounce );
		pItem->remove();
		return;
	}
	
	if( pItem->onDropOnItem( pCont ) )
	{
		if( pItem->free )
			return;

		if( socket->dragging() )
			socket->bounceItem( socket->dragging(), BR_NO_REASON );

		return;
	}
	else if( pCont->onDropOnItem( pItem ) )
	{
		if( pItem->free )
			return;

		if( socket->dragging() )
			socket->bounceItem( socket->dragging(), BR_NO_REASON );

		return;
	}

	// If the target belongs to another character 
	// It needs to be our vendor or else it's denied
	P_CHAR packOwner = pCont->getOutmostChar();

	if( ( packOwner ) && ( packOwner != pChar ) && !pChar->isGM() )
	{
		// For each item someone puts into there 
		// He needs to do a snoop-check
		if( pChar->maySnoop() )
		{
			if( !pChar->checkSkill( SNOOPING, 0, 1000 ) )
			{

				socket->sysMessage( tr( "You fail to put that into %1's pack" ).arg( packOwner->name() ) );
				socket->bounceItem( pItem, BR_NO_REASON );
				return;
			}
		}

		if( packOwner->objectType() == enPlayer || 
			( packOwner->objectType() == enNPC && dynamic_cast<P_NPC>(packOwner)->owner() != pChar ) )
		{
			socket->sysMessage( tr("You cannot put that into the belongings of another player") );
			socket->bounceItem( pItem, BR_NO_REASON );
			return;
		}
	}
	
	if( !pChar->canPickUp( pItem ) )
	{
		socket->bounceItem( pItem, BR_CANNOT_PICK_THAT_UP );
		return;
	}

	// Trash can
	if( pCont->type()==87 )
	{
		pItem->remove();
		socket->sysMessage( tr( "As you let go of the item it disappears." ) );
		return;
	}

	// We drop something on the belongings of one of our playervendors
/*	if( ( packOwner != NULL ) && ( packOwner->npcaitype() == 17 ) && packOwner->owner() == pChar )
	{
		socket->sysMessage( tr( "You drop something into your playervendor (unimplemented)" ) );
		socket->bounceItem( pItem, BR_NO_REASON );
		return;
	}*/

	// Playervendors (chest equipped by the vendor - opened to the client)

	/*if( !( pCont->pileable() && pItem->pileable() && pCont->id() == pItem->id() || ( pCont->type() != 1 && pCont->type() != 9 ) ) )
	{
		P_CHAR pc_j = GetPackOwner(pCont);
		if (pc_j != NULL)
		{
			if (pc_j->npcaitype() == 17 && pc_j->isNpc() && pChar->Owns(pc_j))
			{
				pChar->inputitem = pItem->serial;
				pChar->inputmode = cChar::enPricing;
				sysmessage(s, "Set a price for this item.");
			}
		}
	*/

	// We may also drop into *any* locked chest
	// So we can have post-boxes ;o)
	if( pCont->type() == 1 || pCont->type() == 8 || pCont->type() == 63 || pCont->type() == 65 || pCont->type() == 66 )
	{
		// If we're dropping it onto the closed container
		if( dropPos.x == 0xFFFF && dropPos.y == 0xFFFF )
		{
			pCont->addItem( pItem );
		}
		else
		{
			pCont->addItem( pItem, false );
			pItem->setPos( dropPos );
		}

		// Dropped on another Container/in another Container
		pChar->soundEffect( 0x57 );
		pItem->update();
		return;
	}
	// Item matching needs to be extended !!! at least Color! (for certain types)
	else if ( pCont->isPileable() && pItem->isPileable() && ( pCont->id() == pItem->id() ) )
	{
		if( pCont->amount() + pItem->amount() <= 65535 )
		{
			pCont->setAmount( pCont->amount() + pItem->amount() );
			
			pItem->remove();
			pCont->update(); // Need to update the amount
			return;
		}
		// We have to *keep* our current item
		else
		{
			pCont->setAmount( 65535 ); // Max out the amount
			pCont->update();

			// The delta between 65535 and pCont->amount() sub our Amount is the
			// new amount
			pItem->setAmount( pItem->amount() - ( 65535 - pCont->amount() ) );
		}
	}

	// We dropped the item NOT on a container
	// And were *un*able to stack it (!)
	// >> Set it to the location of the item we dropped it on and stack it up by 2
	pItem->moveTo( pCont->pos() );
	pItem->setPos( pItem->pos() + Coord_cl(0, 0, 2) );
	pItem->update();

/*	// This needs to be checked
	// It annoyingly shows the spellbook
	// whenever you add a scroll
	// << could it be that addItemToContainer is enough?? >>
	if( pCont->type() == 9 )
		Magic->openSpellBook( pChar, pCont );*/
}

// Food was dropped on a pet
void DragAndDrop::dropFoodOnChar( cUOSocket* socket, P_ITEM pItem, P_CHAR pChar )
{
	// Feed our pets
	if( pChar->hunger() >= 6 || pItem->type2() == 0 || !( pChar->nutriment() & ( 1 << (pItem->type2()-1) ) ) )
	{
		socket->sysMessage( tr("It doesn't seem to want your item") );
		bounceItem( socket, pItem );
		return;
	}

	// We have three different eating-sounds (I don't like the idea as they sound too human)
	pChar->soundEffect( 0x3A + RandomNum( 1, 3 ) );

	// *You see Snowwhite eating some poisoned apples*
	// Color: 0x0026
	pChar->emote( tr( "*You see %1 eating %2*" ).arg( pChar->name() ).arg( pItem->getName() ) );

	// We try to feed it more than it needs
	if( pChar->hunger() + pItem->amount() > 6 )
	{
		pItem->setAmount( pItem->amount() - ( 6 - pChar->hunger() ) );
		pChar->setHunger( 6 );

		// Pack the rest into his backpack
		bounceItem( socket, pItem );
		return;
	}

	pChar->setHunger( pChar->hunger() + pItem->amount() );
	pItem->remove();
}

void DragAndDrop::dropOnBeggar( cUOSocket* socket, P_ITEM pItem, P_CHAR pBeggar )
{
	int tempint;
	
	if( ( pBeggar->hunger() < 6 ) && pItem->type() == 14 )
	{
		pBeggar->talk( tr("*cough* Thank thee!") );
		pBeggar->soundEffect( 0x3A + RandomNum( 1, 3 ) );

		// *You see Snowwhite eating some poisoned apples*
		// Color: 0x0026
		pBeggar->emote( tr( "*You see %1 eating %2*" ).arg( pBeggar->name() ).arg( pItem->getName() ) );

		// We try to feed it more than it needs
		if( pBeggar->hunger() + pItem->amount() > 6 )
		{
			
//			client->player()->karma += ( 6 - pBeggar->hunger() ) * 10;
			tempint = ( 6 - pBeggar->hunger() ) * 10;
			socket->player()->setKarma( socket->player()->karma() + tempint );

			pItem->setAmount( pItem->amount() - ( 6 - pBeggar->hunger() ) );
			pBeggar->setHunger( 6 );

			// Pack the rest into his backpack
			bounceItem( socket, pItem );
			return;
		}

		pBeggar->setHunger( pBeggar->hunger() + pItem->amount() );
//		client->player()->karma += pItem->amount() * 10;
		tempint = pItem->amount() * 10;
		socket->player()->setKarma( socket->player()->karma() + tempint );

		pItem->remove();
		return;
	}

	// No Food? Then it has to be Gold
	if( pItem->id() != 0xEED )
	{
		pBeggar->talk( tr("Sorry, but i can only use gold.") );
		bounceItem( socket, pItem );
		return;
	}

	pBeggar->talk( tr( "Thank you %1 for the %2 gold!" ).arg( socket->player()->name() ).arg( pItem->amount() ) );
	socket->sysMessage( tr("You have gained some karma!") );
	
	if( pItem->amount() <= 100 )
		socket->player()->setKarma( socket->player()->karma() + 10 );
	else
		socket->player()->setKarma( socket->player()->karma() + 50 );
	
	pItem->remove();
}

void DragAndDrop::dropOnBroker( cUOSocket* socket, P_ITEM pItem, P_CHAR pBroker )
{
	// For House and Boat deeds we should pay back 75% of the value
	if( pItem->id() == 0x14EF )
	{
		if( !pItem->sellprice() )
		{
			pBroker->talk( tr("I can only accept deeds with value!") );
			bounceItem( socket, pItem );
			return;
		}
		
		socket->player()->giveGold( pItem->sellprice(), true );
		pBroker->talk( tr( "Here you have your %1 gold, %2" ).arg( pItem->sellprice() ).arg( socket->player()->name() ) );
		pItem->remove();
		return;
	}

	bounceItem( socket, pItem );
}

void DragAndDrop::dropOnBanker( cUOSocket* socket, P_ITEM pItem, P_CHAR pBanker )
{
}

void DragAndDrop::dropOnTrainer( cUOSocket* socket, P_ITEM pItem, P_CHAR pTrainer )
{
}
