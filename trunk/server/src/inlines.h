/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2005 by holders identified in AUTHORS.txt
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In addition to that license, if you are running this program or modified
 * versions of it on a public system you HAVE TO make the complete source of
 * the version used by you available or provide people with a location to
 * download it.
 *
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#if !defined(_INLINES_H__)
#define _INLINES_H__

// library Includes
#include <algorithm>
#include <qglobal.h>
#include <qstring.h>

// By Race Section to Check in Create an Elf Character

inline bool isHairsByRace( Q_UINT16 model, bool race )
{
	if (!race)	// Human's Hairs
	{
		return	( ( ( model >= 0x203B ) && ( model <= 0x203D ) ) || ( ( model >= 0x2044 ) && ( model <= 0x204A ) ) ) ? true : false;
	}
	else		// Elf's Hairs
	{
		return  ( ( ( model >= 0x2fbf ) && ( model <= 0x2fc2 ) ) || ( ( model >= 0x2fcc ) && ( model <= 0x2fd1 ) ) ) ? true : false;
	}
}

inline bool isHairsByRaceColor( Q_UINT16 color, bool race )
{
	if (!race)	// Human's Hairs
	{
		return ( ( color >= 0x44E ) && ( color <= 0x47D ) ) ? true : false;
	}
	else		// Elf's Hairs
	{
		// Line 1 of Colors
		if ( ( color >= 0x34 ) && ( color == 0x39 ) )
			return true;
		// Line 2 of Colors
		else if ( ( color == 0x101 ) || ( color = 0x6b8 ) || ( color = 0x207 ) || ( color = 0x211 ) || ( color = 0x26c ) || ( color = 0x2c3 ) )
			return true;
		// Line 3 of Colors
		else if ( ( color == 0x2c9 ) || ( color = 0x1e4 ) || ( color = 0x239 ) || ( color = 0x369 ) || ( color = 0x59d ) || ( color = 0x853 ) )
			return true;
		// Line 4 of Colors
		else if ( ( ( color >= 0x8e ) && ( color == 0x92 ) ) || ( color = 0x159 ) )
			return true;
		// Line 5 of Colors
		else if ( ( ( color >= 0x15a ) && ( color == 0x15e ) ) || ( color = 0x1bd ) )
			return true;
		// Line 6 of Colors
		else if ( ( color == 0x725 ) || ( color = 0x58 ) || ( color = 0x128 ) || ( color = 0x12f ) || ( color = 0x1f3 ) || ( color = 0x251 ) )
			return true;
		// Last Lines
		else if ( ( ( color >= 0x31d ) && ( color == 0x322 ) ) || ( ( color >= 0x323 ) && ( color == 0x326 ) ) || ( ( color >= 0x386 ) && ( color == 0x38a ) ) )
			return true;
		else
			return false;
	}
}

inline bool isHair( Q_UINT16 model )
{
	return	( ( ( model >= 0x203B ) && ( model <= 0x203D ) ) || ( ( model >= 0x2044 ) && ( model <= 0x204A ) ) ) ? true : false;
}

inline bool isBeard( Q_UINT16 model )
{
	return	( ( ( model >= 0x203E ) && ( model <= 0x2041 ) ) || ( ( model >= 0x204B ) && ( model <= 0x204D ) ) ) ? true : false;
}

inline bool isNormalColor( Q_UINT16 color )
{
	return ( ( color >= 2 ) && ( color < 0x3ea ) ) ? true : false;
}

inline bool isSkinColor( Q_UINT16 color, bool race )
{
	if (!race) // Human's Colors
	{
		return ( ( color >= 0x3EA ) && ( color <= 0x422 ) ) ? true : false;
	}
	else	   // Elf's Colors
	{
		// Line 1 of Colors
		if ( ( color == 852 ) || ( color == 906 ) || ( color == 1151 ) || ( color == 2102 ) )
			return true;
		// Line 2 of Colors
		else if ( ( color == 1247 ) || ( color == 1310 ) || ( color == 1901 ) || ( color == 1902 ) )
			return true;
		// Line 3 of Colors
		else if ( ( color == 192 ) || ( color == 590 ) || ( color == 591 ) || ( color == 592 ) )
			return true;
		// Line 4 of Colors
		else if ( ( color == 1344 ) || ( color == 1402 ) || ( color == 898 ) || ( color == 899 ) )
			return true;
		// Line 5 of Colors
		else if ( ( color == 900 ) || ( color == 901 ) || ( color == 902 ) || ( color == 998 ) )
			return true;
		// Line 6 of Colors
		else if ( ( color == 999 ) || ( color == 1000 ) || ( color == 1001 ) || ( color == 326 ) )
			return true;
		// Line 7 of Colors
		else if ( ( color == 1900 ) || ( color == 767 ) || ( color == 866 ) || ( color == 867 ) )
			return true;
		// Line 8 of Colors
		else if ( ( color == 872 ) || ( color == 885 ) || ( color == 886 ) || ( color == 887 ) )
			return true;
		else
			return false;
	}
}

inline bool isHairColor( Q_UINT16 color )
{
	return ( ( color >= 0x44E ) && ( color <= 0x47D ) ) ? true : false;
}

template <typename T>
inline T wpAbs( T a )
{
	return ( a >= 0 ? a : -a );
}

template <typename T>
inline T wpMin( T a, T b )
{
	return ( a > b ? b : a );
}

template <typename T>
inline T wpMax( T a, T b )
{
	return ( a < b ? b : a );
}

inline int roundInt( double n )
{
	return static_cast<int>( n + 0.5 );
}

inline bool isBetween( double n, int lower, int higher, double tolerance = 0.5 )
{
	// Swap the bounds if they are out of order
	if ( lower > higher )
	{
		std::swap( lower, higher );
	}

	return ( n > lower - tolerance ) && ( n < higher + tolerance );
}

inline QString makeAscii( const QString& input )
{
	QString result;

	for ( unsigned int i = 0; i < input.length(); ++i )
	{
		QChar c = input.at( i );

		// German umlauts can be represented differently
		switch ( c )
		{
		case 129:
			// ü
			result.append( "ue" );
			continue;
		case 132:
			// ä
			result.append( "ae" );
			continue;
		case 148:
			// ö
			result.append( "oe" );
			continue;
		case 225:
			// ß
			result.append( "ss" );
			continue;
		case 154:
			// Ü
			result.append( "Ue" );
			continue;
		case 142:
			// Ä
			result.append( "Ae" );
			continue;
		case 153:
			// Ö
			result.append( "Oe" );
			continue;
		}

		// Other non representable char
		if ( c.latin1() < 1 || ( c.latin1() > 122 && c.latin1() != 127 ) )
		{
			continue;
		}
		result.append( c );
	}

	return result;
}

#endif
