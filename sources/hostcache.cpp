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

/*	Nom :				hostcache.cpp
	Type :				Source C++
	Auteur :			Sylvain Demongeot
	Date de Création:	Novembre 1997
	Projet :			Stamina, Charisma
	Environnement :		BeOS, TCP/IP
	Fonction :			Résolution de nom de machine avec cache
	Remarques : 		thread-safe
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#include <stdio.h>
#include <string.h>
#include <net/netdb.h>
#include <net/socket.h>
#include <cache.h>
#include <txcache.h>
#include <hostcache.h>

#define KEYSIZE 65

Hostcache::Hostcache(int cachesize):
	TXCache(cachesize,KEYSIZE,4)	//cachesize,keysize,datasize
{
}

int Hostcache::comparekeys(const void *key1, const void *key2) const
{
	return strncmp((char*)key1,(char*)key2,KEYSIZE);
}

void Hostcache::fetchdata(const void *key, void *data)
{
	struct hostent* host;
	
	host=::gethostbyname((char*)key);
	if(host)
		memcpy(data,host->h_addr_list[0],4);
	else
		memset(data,0,4);
}

unsigned long Hostcache::gethostbyname(const char *hostname)
{
	unsigned int ipnumbers[4];
	char ctrash;
	unsigned long ipaddress;
	
	if(4==sscanf(hostname,"%d.%d.%d.%d%c",
		ipnumbers,ipnumbers+1,ipnumbers+2,ipnumbers+3,&ctrash)
	&& ipnumbers[0]<0x100
	&& ipnumbers[1]<0x100
	&& ipnumbers[2]<0x100
	&& ipnumbers[3]<0x100)
		return ipnumbers[0]<<24
			| ipnumbers[1]<<16
			| ipnumbers[2]<<8
			| ipnumbers[3];
	
	lookup(hostname,&ipaddress);
	return ipaddress;
}

