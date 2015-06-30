/*

[BSD-style license for Stamina and Charisma]

* Copyright (c) 1997-2009, Sylvain Demongeot
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY Sylvain Demongeot ''AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Sylvain Demongeot BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

/*	Nom :				datastorage.h
	Type :				Header C++
	Auteur :			Sylvain Demongeot
	Date de Création:	Novembre 1997
	Projet :			-
	Environnement :		-
	Fonction :			accès mémoire portable
	Remarques : 		-
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#ifndef _INC_DATASTORAGE_
#define _INC_DATASTORAGE_

#include <gtypes.h>

class Datastorage{
public:
	char *data;
	int size;
	int corrupt;
	
	Datastorage(){pointto(NULL,0);corrupt=0;}
	Datastorage(char *data, int size){pointto(data,size);corrupt=0;}
	void pointto(char *data, int size){this->data=data; this->size=size;}

    inline char getchar(int pos);
    inline uchar getuchar(int pos);
    inline int16 getint16(int pos);
    inline uint16 getuint16(int pos);
    inline int32 getint32(int pos);
    inline uint32 getuint32(int pos);
};

inline char Datastorage::getchar(int pos)
{
	return (uchar)data[pos];
}

inline uchar Datastorage::getuchar(int pos)
{
	return data[pos];
}

inline int16 Datastorage::getint16(int pos)
{
    uchar *p;
    uint16 r;
    
    p=(uchar*)data+pos;
    r=p[0];
    r<<=8;
    r|=p[1];

	return r;
}

inline uint16 Datastorage::getuint16(int pos)
{
    uchar *p;
    uint16 r;
    
    p=(uchar*)data+pos;
    r=p[0];
    r<<=8;
    r|=p[1];

	return (int16)r;
}

inline int32 Datastorage::getint32(int pos)
{
    uchar *p;
    uint32 r;
    
    p=(uchar*)data+pos;
    r=p[0];
    r<<=8;
    r|=p[1];
    r<<=8;
    r|=p[2];
    r<<=8;
    r|=p[3];

	return (int32)r;
}

inline uint32 Datastorage::getuint32(int pos)
{
    uchar *p;
    uint32 r;
    
    p=(uchar*)data+pos;
    r=p[0];
    r<<=8;
    r|=p[1];
    r<<=8;
    r|=p[2];
    r<<=8;
    r|=p[3];

	return r;
}

#endif	// _INC_DATASTORAGE_
