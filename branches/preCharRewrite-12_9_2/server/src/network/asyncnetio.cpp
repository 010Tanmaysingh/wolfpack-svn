//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
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

#include "asyncnetio.h"
#include "uorxpackets.h"
#include "uopacket.h"
#include "../srvparams.h"
#include "../globals.h"

// Library Includes
#include <qsocketdevice.h>
#include <qptrlist.h>
#include <qmap.h>

#undef CONST

// Twofish
#include "../encryption.h"

/*!
	\internal
	Table of packet lengths, automatically generated from
	Client Version: UO: LBR (Thrid Dawn) 3.0.8d

	0xFFFF - Packet not used
	0x0000 - Packet has dynamic length
*/
const Q_UINT16 packetLengths[256] =
{
0x0068, 0x0005, 0x0007, 0x0000, 0x0002, 0x0005, 0x0005, 0x0007, // 0x07
0x000e, 0x0005, 0x000b, 0x010a, 0x0000, 0x0003, 0x0000, 0x003d, // 0x0F
0x00d7, 0x0000, 0x0000, 0x000a, 0x0006, 0x0009, 0x0001, 0x0000, // 0x17
0x0000, 0x0000, 0x0000, 0x0025, 0x0000, 0x0005, 0x0004, 0x0008, // 0x1F
0x0013, 0x0008, 0x0003, 0x001a, 0x0007, 0x0014, 0x0005, 0x0002, // 0x27
0x0005, 0x0001, 0x0005, 0x0002, 0x0002, 0x0011, 0x000f, 0x000a, // 0x2F
0x0005, 0x0001, 0x0002, 0x0002, 0x000a, 0x028d, 0x0000, 0x0008, // 0x37
0x0007, 0x0009, 0x0000, 0x0000, 0x0000, 0x0002, 0x0025, 0x0000, // 0x3F
0x00c9, 0x0000, 0x0000, 0x0229, 0x02c9, 0x0005, 0x0000, 0x000b, // 0x47
0x0049, 0x005d, 0x0005, 0x0009, 0x0000, 0x0000, 0x0006, 0x0002, // 0x4F
0x0000, 0x0000, 0x0000, 0x0002, 0x000c, 0x0001, 0x000b, 0x006e, // 0x57
0x006a, 0x0000, 0x0000, 0x0004, 0x0002, 0x0049, 0x0000, 0x0031, // 0x5F
0x0005, 0x0009, 0x000f, 0x000d, 0x0001, 0x0004, 0x0000, 0x0015, // 0x67
0x0000, 0x0000, 0x0003, 0x0009, 0x0013, 0x0003, 0x000e, 0x0000, // 0x6F
0x001c, 0x0000, 0x0005, 0x0002, 0x0000, 0x0023, 0x0010, 0x0011, // 0x77
0x0000, 0x0009, 0x0000, 0x0002, 0x0000, 0x000d, 0x0002, 0x0000, // 0x7F
0x003e, 0x0000, 0x0002, 0x0027, 0x0045, 0x0002, 0x0000, 0x0000, // 0x87
0x0042, 0x0000, 0x0000, 0x0000, 0x000b, 0x0000, 0x0000, 0x0000, // 0x8F
0x0013, 0x0041, 0x0000, 0x0063, 0x0000, 0x0009, 0x0000, 0x0002, // 0x97
0x0000, 0x001a, 0x0000, 0x0102, 0x0135, 0x0033, 0x0000, 0x0000, // 0x9F
0x0003, 0x0009, 0x0009, 0x0009, 0x0095, 0x0000, 0x0000, 0x0004, // 0xA7
0x0000, 0x0000, 0x0005, 0x0000, 0x0000, 0x0000, 0x0000, 0x000d, // 0xAF
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0040, 0x0009, 0x0000, // 0xB7
0x0000, 0x0003, 0x0006, 0x0009, 0x0003, 0x0000, 0x0000, 0x0000, // 0xBF
0x0024, 0x0000, 0x0000, 0x0000, 0x0006, 0xFFFF, 0xFFFF, 0xFFFF, // 0xC7
0x0002, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, // 0xCF
0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
};

/*****************************************************************************
  cAsyncNetIOPrivate member functions
 *****************************************************************************/

using namespace ZThread;

enum eEncryption
{
	E_NONE = 0,
	E_LOGIN,
	E_GAME
};

class cAsyncNetIOPrivate
{
private:
	cAsyncNetIOPrivate( cAsyncNetIOPrivate& ); // Satisfy warning C4511
public:
	cAsyncNetIOPrivate();
	~cAsyncNetIOPrivate();

    QSocketDevice      *socket;			// connection socket
    QPtrList<QByteArray> rba, wba;		// list of read/write bufs
    Q_ULONG			rsize, wsize;		// read/write total buf size
    Q_ULONG			rindex, windex;		// read/write index
	FastMutex		wmutex;				// write mutex
	bool			skippedUOHeader;		// Skip crypt key junk

	LockedQueue<cUOPacket*, FastMutex>* packets; // Complete UOPackets
	FastMutex cryptMutex;

	int getch();
	int ungetch( int ch );
	Q_LONG readBlock( char *data, Q_ULONG maxlen );
	Q_LONG writeBlock( const char *data, Q_ULONG len );
	Q_LONG writeBlock( QByteArray data );
	bool consumeWriteBuf( Q_ULONG nbytes );
	bool consumeReadBuf( Q_ULONG nbytes, char *sink );

	bool login; // false = GameServer Protocol, true = LoginServer Protocol
	UINT32 seed;
	cClientEncryption *encryption;
};

cAsyncNetIOPrivate::cAsyncNetIOPrivate() 
	: socket(0), rsize(0), wsize(0), rindex(0), windex(0), skippedUOHeader(false)
{
    rba.setAutoDelete( TRUE );
    wba.setAutoDelete( TRUE );
	packets = new ZThread::LockedQueue<cUOPacket*, ZThread::FastMutex>;
	
	// Encryption
	encryption = 0;
}

cAsyncNetIOPrivate::~cAsyncNetIOPrivate()
{
	for ( int i = 0; i < packets->size(); ++i )
	{
		delete packets->next();
	}
	delete packets;

	if( encryption )
		delete encryption;
}

/*!
	\internal
    Reads \a maxlen bytes from the socket into \a data and returns the
    number of bytes read. Returns -1 if an error occurred.
*/
Q_LONG cAsyncNetIOPrivate::readBlock( char *data, Q_ULONG maxlen )
{
    if ( data == 0 && maxlen != 0 ) 
	{
		return -1;
    }
    if ( maxlen >= rsize )
		maxlen = rsize;
    consumeReadBuf( maxlen, data );
    return maxlen;
}

/*!
	\internal
    Writes \a len bytes to the socket from \a data and returns the
    number of bytes written. Returns -1 if an error occurred.
*/
Q_LONG cAsyncNetIOPrivate::writeBlock( const char *data, Q_ULONG len )
{
	// Invalid Socket -> Disconnected
	if( !socket->isValid() )
	{
		socket->close();
		return 0;
	}

    if ( len == 0 )
		return 0;
	
    QByteArray *a = wba.last();
	
    if ( a && a->size() + len < 128 ) 
	{
		// small buffer, resize
		int i = a->size();
		a->resize( i+len );
		memcpy( a->data()+i, data, len );
    } else {
		// append new buffer
		a = new QByteArray( len );
		memcpy( a->data(), data, len );
		wba.append( a );
    }
    wsize += len;
    return len;
}

/*!
	\internal
    Writes \a data and returns the number of bytes written. 
	Returns -1 if an error occurred.
*/
Q_LONG cAsyncNetIOPrivate::writeBlock( QByteArray data )
{
	return writeBlock( data.data(), data.size() );
}

/*!
	\internal
    Reads a single byte/character from the internal read buffer.
    Returns the byte/character read, or -1 if there is nothing to be
    read.

    \sa ungetch()
*/
int cAsyncNetIOPrivate::getch()
{
    if ( rsize > 0 ) 
	{
		uchar c;
		consumeReadBuf( 1, (char*)&c );
		return c;
    }
    return -1;
}

/*!
    Prepends the character \a ch to the read buffer so that the next
    read returns this character as the first character of the output.

	\sa getch()
*/
int cAsyncNetIOPrivate::ungetch( int ch )
{
    if ( rba.isEmpty() || rindex==0 ) {
		// we need a new QByteArray
		QByteArray *ba = new QByteArray( 1 );
		rba.insert( 0, ba );
		rsize++;
		ba->at( 0 ) = ch;
    } else {
		// we can reuse a place in the buffer
		QByteArray *ba = rba.first();
		rindex--;
		rsize++;
		ba->at( rindex ) = ch;
    }
    return ch;
}

/*!
  \internal
  Removes from internal read buffer \a nbytes.
  Returns true if successfull or false if there were not enought bytes in buffer to fullfill
  the request.
*/
bool cAsyncNetIOPrivate::consumeWriteBuf( Q_ULONG nbytes )
{
    if ( nbytes <= 0 || nbytes > wsize )
	return false;
    wsize -= nbytes;
    for ( ;; ) 
	{
		QByteArray *a = wba.first();
		if ( windex + nbytes >= a->size() ) 
		{
		    nbytes -= a->size() - windex;
			wba.remove();
		    windex = 0;
		    if ( nbytes == 0 )
				break;
		} else {
			windex += nbytes;
			break;
		}
    }
    return false;
}

/*!
	\internal
	Reads and removes from internal read buffer \a nbytes and place then into \a sink.
	Returns true if successfull or false if there were not enought bytes to fullfill
	the request.
*/
bool cAsyncNetIOPrivate::consumeReadBuf( Q_ULONG nbytes, char *sink )
{
    if ( nbytes <= 0 || nbytes > rsize )
		return false;
    rsize -= nbytes;
    for ( ;; ) 
	{
		QByteArray *a = rba.first();
		if ( rindex + nbytes >= a->size() ) 
		{
			// Here we skip the whole byte array and get the next later
			int len = a->size() - rindex;
			if ( sink ) 
			{
				memcpy( sink, a->data()+rindex, len );
				sink += len;
			}
			nbytes -= len;
			rba.remove();
			rindex = 0;
			if ( nbytes == 0 ) 
			{		// nothing more to skip
				break;
			}
		} else {
			// Here we skip only a part of the first byte array
			if ( sink )
				memcpy( sink, a->data()+rindex, nbytes );
			rindex += nbytes;
			break;
		}
    }
    return true;
}

/*****************************************************************************
  cAsyncNetIO member functions
 *****************************************************************************/

/*!
  \class cAsyncNetIO asyncnetio.h

  \brief The cAsyncNetIO class asyncronous low-level network handling, buffering,
  compressing, slicing and building higher level application packets.

  \ingroup network
  \ingroup mainclass
  \sa cUOPacket
*/

/*!
  Registers a \a socket for asyncronous services.
  \a login determines whether the connection has been established to
  the login server or not.
*/
bool cAsyncNetIO::registerSocket( QSocketDevice* socket, bool login )
{
	mapsMutex.acquire();
	cAsyncNetIOPrivate* d = new cAsyncNetIOPrivate;
	d->login = login;
	d->socket = socket;
	buffers.insert( socket, d );
	mapsMutex.release();
	return true;
}

/*!
  Unregisters a \a socket for asyncronous services.
  The \a socket parameter is not modified in any way.
*/
bool cAsyncNetIO::unregisterSocket( QSocketDevice* socket )
{
	mapsMutex.acquire();
	iterator it = buffers.find( socket );
	if( it != buffers.end() )
	{
		delete it.data();
		buffers.remove( it );
	}
	mapsMutex.release();
	return true;
}

/*!
  \internal
  The thread's execution loop for AsyncNetIO
*/
void cAsyncNetIO::run() throw()
{
	while ( !canceled() )
	{
		mapsMutex.acquire(); // do not disturb me here.
		for ( iterator it = buffers.begin(); it != buffers.end(); ++it )
		{
			// Read all avaliable data.
			char buf[4096];
			cAsyncNetIOPrivate* d = it.data();

			// Check if the socket is valid
			if( !d->socket->isValid() )
				continue; // Let it in the queue until it's taken out by the closed-collector

			// Try to get the UO Header somehow
			if( !d->skippedUOHeader )
			{
				int nread = d->socket->readBlock( buf, 4 - d->rsize );

				if( nread > 0 )
				{
					QByteArray* a = new QByteArray(nread);
					memcpy( a->data(), buf, nread );
					d->rba.append(a);
					d->rsize += nread;

					if( d->rsize == 4 )
					{
						char temp[4];
						d->consumeReadBuf( 4, temp );
						d->skippedUOHeader = true;
	
						d->seed = ( ( temp[0] & 0xFF ) << 24 ) | ( ( temp[1] & 0xFF ) << 16 ) | ( ( temp[2] & 0xFF ) << 8 ) | ( ( temp[3] & 0xFF ) );

						// Only 0xFFFFFFFF seed allowed for !d->login
						if( !d->login && d->seed != 0xFFFFFFFF )
						{
							d->writeBlock( "\x82\x04", 2 );
							flushWriteBuffer( d );
							d->socket->close();
							continue;
						}
					}
				}
				else if( nread == 0 )
				{
					d->socket->close();
				}
			}
			else
			{
				int nread = d->socket->readBlock( buf, sizeof(buf));

				if( nread > 0 )
				{
					// If we have an encryption object already
					// then decrypt the buffer before storing it
					if( d->encryption )
						d->encryption->clientDecrypt( &buf[0], nread );

					QByteArray* a = new QByteArray(nread);
					memcpy( a->data(), buf, nread );
					d->rba.append(a);
					d->rsize += nread;
					
					// We need to use the read buffer as a temporary buffer
					// for encrypted data if we didn't receive all we need
					if( !d->encryption )
					{
						// Gameserver Encryption
						if( !d->login && d->rsize >= 65 )
						{
							// The 0x91 packet is 65 byte
							nread = d->rsize;
							d->consumeReadBuf( d->rsize, &buf[0] );

							// This should be no encryption
							if( buf[0] == '\x91' && buf[1] == '\xFF' && buf[2] == '\xFF' && buf[3] == '\xFF' && buf[4] == '\xFF' )
							{
								// Is no Encryption allowed?
								if( !SrvParams->allowUnencryptedClients() )
								{
									// Send a communication problem message to this socket
									d->writeBlock( "\x82\x04", 2 );
									flushWriteBuffer( d );
									d->socket->close();
									continue;
								}

								d->encryption = new cNoEncryption;
							}
							else
							{
								cGameEncryption *crypt = new cGameEncryption;
								crypt->init( 0xFFFFFFFF ); // Seed is fixed anyway
								d->encryption = crypt;
							}							
						}
						// LoginServer Encryption
						else if( d->login && d->rsize >= 62 )
						{
							// The 0x80 packet is 62 byte, but we want to have everything
							nread = d->rsize;
							d->consumeReadBuf( d->rsize, &buf[0] );

							// Check if it could be *not* encrypted
							if( buf[0] == '\x80' && buf[30] == '\x00' && buf[60] == '\x00' )
							{
								// Is no Encryption allowed?
								if( !SrvParams->allowUnencryptedClients() )
								{
									// Send a communication problem message to this socket
									d->writeBlock( "\x82\x04", 2 );
									flushWriteBuffer( d );
									d->socket->close();
									continue;
								}

								d->encryption = new cNoEncryption;
							}
							else
							{
								cLoginEncryption *crypt = new cLoginEncryption;
								if( !crypt->init( d->seed, &buf[0], nread ) )
								{
									delete crypt;

									// Send a communication problem message to this socket
									d->writeBlock( "\x82\x04", 2 );
									flushWriteBuffer( d );
									d->socket->close();
									continue;
								}

								d->encryption = crypt;
							}

							// It gets a bit tricky now, try to decode it with all
							// possible keys
						}

                        // If we found an encryption let's decrypt what we got in our buffer
						if( d->encryption )
						{
							d->encryption->clientDecrypt( &buf[0], nread );

							QByteArray* a = new QByteArray(nread);
							memcpy( a->data(), buf, nread );
							d->rba.append(a);
							d->rsize += nread;
						}
					}					
				}

				if( nread == 0 )
				{
					d->socket->close();
				}

				buildUOPackets( d );
			}
			
			// Write data to socket
			flushWriteBuffer( d );
		}
		mapsMutex.release();		
		//if( buffers.empty() )
		// Disconnecting doesnt work for now
		try {
		sleep(40); // we've done our job, let's relax for a while.
		} catch ( ZThread::Interrupted_Exception& e )
		{ // Looks like we are going to finish this thread.
		}
	}
}

/*!
  Informs the amount of data valiable into internal buffers
*/
Q_ULONG cAsyncNetIO::bytesAvailable( QSocketDevice* socket ) const
{
	const_iterator it = buffers.find( socket );
	return it.data()->rsize;
}

/*!
  \internal
  This method will check the buffer for the next packet ID and try
  to build it if it have already been fully received. If the packet
  have arrived incomplete it will leave the data in buffer.
  Built packets are queued for core's retrieve.
*/
void cAsyncNetIO::buildUOPackets( cAsyncNetIOPrivate* d )
{
	bool keepExtracting = d->rsize > 1;
	while ( keepExtracting )
	{
		int packetID = d->getch();
		if ( packetID != -1 )
		{
			Q_UINT16 length = packetLengths[packetID];
			if ( length != 0x0000 )
			{// fixed size.
				d->ungetch( packetID );

				if( length == 0xFFFF )
				{
					QByteArray packetData( d->rsize );
					d->readBlock( packetData.data(), d->rsize );
					qWarning( cUOPacket::dump( packetData ) );
					continue;
				}

				if( d->rsize >= length )
				{
					QByteArray packetData(length);
					d->readBlock( packetData.data(), length );
					cUOPacket* packet = getUOPacket( packetData );
					if( !packet )
						d->socket->close();
					d->packets->add( packet );
				}
				else
					keepExtracting = false; // we have to wait some more.
			}
			else
			{ // variable length
				if ( d->rsize < 3 ) // Packet ID, size + 1 byte data.
				{
					keepExtracting = false;
					d->ungetch( packetID ); // byte was read, put back to buffer.
					continue;
				}
				Q_UINT16 length = 0;
				Q_UINT8* p = (Q_UINT8*)&length;
				*(p+1)     = (Q_UINT8) d->getch();
				*p         = (Q_UINT8) d->getch();
				d->ungetch( *p     );
				d->ungetch( *(p+1) );
				d->ungetch( packetID );
				if ( d->rsize < length )
				{
					keepExtracting = false;
					continue;
				}

				QByteArray packetData( length );
				d->readBlock( packetData.data(), length );
				cUOPacket* packet = getUOPacket( packetData );
				if( !packet )
					d->socket->close();
				d->packets->add( packet );
			}
		}
		else
			keepExtracting = false; // no more data in buffer.
	}
}

/*!
  \internal
  This method will check the buffer for data to be written and send
  it to socket.
  Lock is done in record level, should be safe to call from different
  threads.
*/
void cAsyncNetIO::flushWriteBuffer( cAsyncNetIOPrivate* d )
{
	bool osBufferFull = false;
	int consumed = 0;

	// Buffer has closed
	if( !d->socket->isValid() || !d->socket->isWritable() )
		return;

	// Before we continue, we should guarantee no one writes packets to the buffer.
	d->wmutex.acquire();
	while ( !osBufferFull && d->wsize > 0 )
	{
		QByteArray* a = d->wba.first();
		int nwritten;
		int i = 0;
		if ( (int)a->size() - d->windex < 1460 ) 
		{
			// Concatenate many smaller blocks.  the first may be
			// partial, but each subsequent block is copied entirely
			// or not at all.  the sizes here are picked so that we
			// generally won't trigger nagle's algorithm in the tcp
			// implementation: we concatenate if we'd otherwise send
			// less than PMTU bytes (we assume PMTU is 1460 bytes),
			// and concatenate up to the largest payload TCP/IP can
			// carry.  with these precautions, nagle's algorithm
			// should apply only when really appropriate.
			QByteArray out( 65536 );
			int j = d->windex;
			int s = a->size() - j;
			while ( a && i+s < (int)out.size() ) 
			{
				memcpy( out.data()+i, a->data()+j, s );
				j = 0;
				i += s;
				a = d->wba.next();
				s = a ? a->size() : 0;
			}

			// Encrypt the outgoing buffer
			if( d->encryption )
				d->encryption->serverEncrypt( out.data(), i );

			nwritten = d->socket->writeBlock( out.data(), i );
		} else {
			// Big block, write it immediately
			i = a->size() - d->windex;
			
			// Encrypt the outgoing buffer
			if( d->encryption )
				d->encryption->serverEncrypt( a->data() + d->windex, i );

			nwritten = d->socket->writeBlock( a->data() + d->windex, i );
		}

		if ( nwritten ) 
		{
			if ( d->consumeWriteBuf( nwritten ) )
				consumed += nwritten;
		}
		if ( nwritten < i )
			osBufferFull = TRUE;
	}
	d->wmutex.release(); // release this record
}

/*!
  Tries to receive a cUOPacket from \a socket if one is avaliable. 
  If no packet is avaliable at this time, the result will be 0.
  Packets retrieved from this method should be freed by
  the caller.
*/
cUOPacket* cAsyncNetIO::recvPacket( QSocketDevice* socket )
{
	iterator it = buffers.find( socket );
	if ( it.data()->packets->size() )
		return it.data()->packets->next();
	else
		return 0;
}

/*!
  Queues \a packet for sending to \a socket. UO Huffman compression will
  be applied if \a compress is true.
*/
void cAsyncNetIO::sendPacket( QSocketDevice* socket, cUOPacket* packet, bool compress )
{
	iterator it = buffers.find( socket );
	it.data()->wmutex.acquire();
	if( compress )
	{
		QByteArray data = packet->compressed();
		it.data()->writeBlock( data );
	}
	else
	{
		it.data()->writeBlock( packet->uncompressed() );
	}
	it.data()->wmutex.release();
}

/*!
  Sends to connected \a socket any packet writes still in buffer. This method 
  is usually called prior to disconnection to ensure all data, including error
  messages have been sent.
*/
void cAsyncNetIO::flush( QSocketDevice* socket)
{
	iterator it = buffers.find( socket );
	
	if( it != buffers.end() )
		flushWriteBuffer( it.data() );
}
