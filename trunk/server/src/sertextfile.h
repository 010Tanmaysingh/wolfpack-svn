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

#if !defined(__SERTEXTFILE_H__)
#define __SERTEXTFILE_H__

#include "iserialization.h"

#include <fstream>

/*!
CLASS
    

    This class provides an implementation for Text file serialization implementation.


USAGE
    <More detailed description of the class and
        a summary of it's (public) operations>.

*/
//##ModelId=3C5D92D001A9
class serTextFile : public ISerialization
{
protected:
	//##ModelId=3C5D92D0025E
	std::fstream file;
	//##ModelId=3C5D92D00271
	unsigned int _version;
	//##ModelId=3C5D92D0028F
	unsigned int _count;
public:
	//##ModelId=3C5D92D002A3
	serTextFile() : _version(0), _count(0) {}
	//##ModelId=3C5D92D002AD
	virtual ~serTextFile() {}

	//##ModelId=3C5D92D002B7
	virtual void prepareReading(std::string ident);
	//##ModelId=3C5D92D002CB
	virtual void prepareWritting(std::string ident);
	//##ModelId=3C5D92D002DF
	virtual void close();	

	//##ModelId=3C5D92D002F3
	virtual unsigned int getVersion();
	//##ModelId=3C5D92D002FD
	virtual unsigned int size();
	//##ModelId=3C5D92D00308
	virtual void setVersion(unsigned int);

	// Write Methods
	//##ModelId=3C5D92D0031B
	virtual void writeObjectID(std::string data);

	//##ModelId=3C5D92D00339
	virtual void write(std::string Key, std::string &data);
	//##ModelId=3C5D92D0034E
	virtual void write(std::string Key, unsigned int data);
	//##ModelId=3C5D92D0036C
	virtual void write(std::string Key, signed int data);
	//##ModelId=3C5D92D00394
	virtual void write(std::string Key, signed short data);
	//##ModelId=3C5D92D1002E
	virtual void write(std::string Key, unsigned short data);
	//##ModelId=3C5D92D1004C
	virtual void write(std::string Key, unsigned char data);
	//##ModelId=3C5D92D100A6
	virtual void write(std::string Key, signed char data);
	//##ModelId=3C5D92D100C4
	virtual void write(std::string Key, bool data);

	//##ModelId=3C5D92D100E2
	virtual void doneWritting();

	// Read Methods
	//##ModelId=3C5D92D100ED
	virtual void readObjectID(std::string &data);

	//##ModelId=3C5D92D10101
	virtual void read(std::string Key, std::string    &data);
	//##ModelId=3C5D92D1013C
	virtual void read(std::string Key, unsigned int   &data);
	//##ModelId=3C5D92D1015A
	virtual void read(std::string Key, signed   int   &data);
	//##ModelId=3C5D92D10178
	virtual void read(std::string Key, unsigned short &data);
	//##ModelId=3C5D92D10196
	virtual void read(std::string Key, signed short   &data);
	//##ModelId=3C5D92D101B4
	virtual void read(std::string Key, unsigned char  &data);
	//##ModelId=3C5D92D101DC
	virtual void read(std::string Key, signed   char  &data);
	//##ModelId=3C5D92D101F1
	virtual void read(std::string Key, bool           &data);
};

#endif // __SERTEXTFILE_H__
