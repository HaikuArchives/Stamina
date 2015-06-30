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

/*	Nom :				etypes.h
	Type :				header C++
	Auteur :			Sylvain Demongeot
	Date de Création:	Janvier 1998
	Projet :			-
	Environnement :		-
	Fonction :			Types de base explicites
	Remarques : 		-
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#ifndef _ETYPES_H_
#define _ETYPES_H_

typedef signed char		schar;
#ifndef BEOS
typedef unsigned char	uchar;
typedef signed short	int16;
typedef unsigned short	uint16;
typedef signed long		int32;
typedef unsigned long	uint32;
#endif // BEOS

typedef int32			fix1616;
#define fix1616_1		0x00010000L

inline fix1616 int32tofix1616(int32 x)	{return x<<16;}
inline int32 fix1616toint32(fix1616 x)	{return x>>16;}
inline fix1616 floattofix1616(float x)	{return (fix1616)(x*65536.0f);}
inline float fix1616tofloat(fix1616 x)	{return x/65536.0f;}

#ifndef NULL
#define NULL 0L
#endif

#endif	// _ETYPES_H_
