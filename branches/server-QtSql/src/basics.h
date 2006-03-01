/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2006 by holders identified in AUTHORS.txt
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

// basics.h: interface for the basics functions.
#if !defined(__BASICS_H__)
#define __BASICS_H__

// Platfrom specifics
#include "platform.h"

#include <functional>
#include <algorithm>
#include <QMap>
#include <QFile>
#include <QByteArray>

// Forward definitions
class Coord;
class QString;

// sereg : roll dices d&d style
int rollDice( const QString& dicePattern );

bool parseCoordinates( const QString& input, Coord& coord, bool ignoreZ = false );

int RandomNum( int nLowNum, int nHighNum );

QString hex2dec( const QString& value );

float RandomFloatNum( float nLowNum, float nHighNum );
unsigned int getNormalizedTime();
uint elfHash(const char * name);

// Swap the value in place
inline void swapBytes( unsigned int& data )
{
	data = ( ( data & 0xFF ) << 24 ) | ( ( data & 0xFF00 ) << 8 ) | ( ( data & 0xFF0000 ) >> 8 ) | ( ( data & 0xFF000000 ) >> 24 );
}

inline void swapBytes( double& value )
{
	unsigned char * ptr = ( unsigned char * ) &value;
	std::swap( ptr[0], ptr[7] );
	std::swap( ptr[1], ptr[6] );
	std::swap( ptr[2], ptr[5] );
	std::swap( ptr[3], ptr[4] );
}

inline void swapBytes( int& data )
{
	data = ( ( data & 0xFF ) << 24 ) | ( ( data & 0xFF00 ) << 8 ) | ( ( data & 0xFF0000 ) >> 8 ) | ( ( data & 0xFF000000 ) >> 24 );
}

inline void swapBytes( unsigned short& data )
{
	data = ( ( data & 0xFF00 ) >> 8 ) | ( ( data & 0xFF ) << 8 );
}

inline void swapBytes( short& data )
{
	data = ( ( data & 0xFF00 ) >> 8 ) | ( ( data & 0xFF ) << 8 );
}

class cBufferedWriter
{
private:
	class cBufferedWriterPrivate *d;
	int buffersize;

public:
	cBufferedWriter( const QByteArray& magic, unsigned int version );
	~cBufferedWriter();

	void open( const QString& filename );
	void close();
	void flush();

	inline void writeInt( unsigned int data, bool unbuffered = false );
	inline void writeShort( unsigned short data, bool unbuffered = false );
	inline void writeByte( unsigned char data, bool unbuffered = false );
	inline void writeBool( bool data, bool unbuffered = false );
	inline void writeUtf8( const QString& data, bool unbuffered = false );
	inline void writeAscii( const QByteArray& data, bool unbuffered = false );
	inline void writeRaw( const void* data, unsigned int size, bool unbuffered = false );
	inline void writeDouble( double data, bool unbuffered = false );

	unsigned int position();
	unsigned int version();
	void setSkipSize( unsigned char type, unsigned int skipsize );
	void setObjectCount( unsigned int count );
	unsigned int objectCount();
};

class cBufferedWriterPrivate
{
public:
	QFile file;
	unsigned int version;
	QByteArray magic;
	bool needswap;
	char *buffer;
	unsigned int bufferpos;
	QMap<QByteArray, unsigned int> dictionary;
	QMap<unsigned char, unsigned int> skipmap;
	QMap<unsigned char, QString> typemap;
	unsigned int lastStringId;
	unsigned int objectCount;
};

class cBufferedReader
{
private:
	class cBufferedReaderPrivate *d;
	QString error_;

public:
	cBufferedReader( const QByteArray& magic, unsigned int version );
	~cBufferedReader();

	void open( const QString& filename );
	void close();
	unsigned int version();

	unsigned int readInt();
	unsigned short readShort();
	unsigned char readByte();
	bool readBool();
	double readDouble();
	QString readUtf8();
	QByteArray readAscii( bool nodictionary = false );
	void readRaw( void* data, unsigned int size );

	unsigned int position();
	const QMap<unsigned char, QByteArray>& typemap();
	unsigned int getSkipSize( unsigned char type );

	unsigned int objectCount();

	inline void setError( const QString& error )
	{
		error_ = error;
	}

	inline bool hasError()
	{
		return !error_.isNull();
	}

	inline const QString& error()
	{
		return error_;
	}
};

inline void cBufferedWriter::writeInt( unsigned int data, bool unbuffered )
{
	// Inplace Swapping (data is a copy anyway)
	if ( d->needswap )
	{
		swapBytes( data );
	}

	if ( unbuffered )
	{
		flush();
		d->file.write( ( char * ) &data, sizeof( data ) );
	}
	else
	{
		if ( d->bufferpos > buffersize - sizeof( data ) )
		{
			flush();
		}

		d->buffer[d->bufferpos++] = ( ( char * ) &data )[0];
		d->buffer[d->bufferpos++] = ( ( char * ) &data )[1];
		d->buffer[d->bufferpos++] = ( ( char * ) &data )[2];
		d->buffer[d->bufferpos++] = ( ( char * ) &data )[3];
	}
}

inline void cBufferedWriter::writeShort( unsigned short data, bool unbuffered )
{
	// Inplace Swapping (data is a copy anyway)
	if ( d->needswap )
	{
		swapBytes( data );
	}

	if ( unbuffered )
	{
		flush();
		d->file.write( ( char * ) &data, sizeof( data ) );
	}
	else
	{
		if ( d->bufferpos > buffersize - sizeof( data ) )
		{
			flush();
		}

		d->buffer[d->bufferpos++] = ( ( char * ) &data )[0];
		d->buffer[d->bufferpos++] = ( ( char * ) &data )[1];
	}
}

inline void cBufferedWriter::writeBool( bool data, bool unbuffered )
{
	writeByte( data ? 1 : 0, unbuffered );
}

inline void cBufferedWriter::writeByte( unsigned char data, bool unbuffered )
{
	if ( unbuffered )
	{
		flush();
		d->file.write( ( const char * ) &data, sizeof( data ) );
	}
	else
	{
		if ( d->bufferpos >= 4096 )
		{
			flush();
		}

		d->buffer[d->bufferpos++] = ( char ) data;
	}
}

inline void cBufferedWriter::writeUtf8( const QString& data, bool unbuffered )
{
	writeAscii( data.toUtf8(), unbuffered );
}

inline void cBufferedWriter::writeAscii( const QByteArray& data, bool unbuffered )
{
	QMap<QByteArray, unsigned int>::iterator it = d->dictionary.find( data );

	if ( it != d->dictionary.end() )
	{
		writeInt( it.value(), unbuffered );
	}
	else
	{
		d->dictionary.insert( data, ++d->lastStringId );
		writeInt( d->lastStringId, unbuffered );
	}
}

inline void cBufferedWriter::writeRaw( const void* data, unsigned int size, bool unbuffered )
{
	if ( unbuffered )
	{
		flush();
		d->file.write( ( const char * ) data, size );
	}
	else
	{
		// Flush out entire blocks if neccesary until we dont
		// overflow the buffer anymore, then just append
		unsigned int pos = 0;

		while ( d->bufferpos + size >= ( unsigned int ) buffersize )
		{
			unsigned int bspace = buffersize - d->bufferpos;

			// Try putting in some bytes of the remaining data
			if ( bspace != 0 )
			{
				memcpy( d->buffer + d->bufferpos, ( unsigned char * ) data + pos, bspace );
				d->bufferpos = buffersize;
				pos += bspace;
				size -= bspace;
			}

			flush();
		}

		// There are still some remaining bytes of our data
		if ( size != 0 )
		{
			memcpy( d->buffer + d->bufferpos, ( unsigned char * ) data + pos, size );
			d->bufferpos += size;
		}
	}
}

inline void cBufferedWriter::writeDouble( double value, bool unbuffered )
{
	if ( d->needswap )
	{
		swapBytes( value );
	}

	writeRaw( &value, sizeof( value ), unbuffered );
}

#endif
