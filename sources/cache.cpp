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

/*	Nom :				cache.cpp
	Type :				Source C++
	Auteur :			Sylvain Demongeot
	Date de Création:	Novembre 1997
	Projet :			Stamina, Charisma
	Environnement :		-
	Fonction :			Mécanisme générique et simple de cache
	Remarques : 		non thread-safe
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#include <string.h>
#include <cache.h>

Cache::Cache(int cachesize, int keysize, int datasize)
{
	m_cachesize=cachesize;
	m_keysize=keysize;
	m_datasize=datasize;
	
	m_keys=new char[m_cachesize*m_keysize];
	m_data=new char[m_cachesize*m_datasize];
	m_time=new int[m_cachesize];
	
	memset(m_keys,0,m_cachesize*m_keysize);
	memset(m_time,0,sizeof(int)*m_cachesize);
	m_curtime=0;
}

Cache::~Cache()
{
	delete[] m_keys;
	delete[] m_data;
	delete[] m_time;
}

void Cache::lookup(const void *key, void *data)
{
	int i;

	i=findindex(key);
	if(i<0){
		i=findoldest();
		fetchdata(key,getdata(i));
		memcpy(getkey(i),key,m_keysize);
	}
	memcpy(data,getdata(i),m_datasize);
	reward(i);
}

void Cache::flush()
{
	memset(m_keys,0,m_cachesize*m_keysize);
	memset(m_time,0,sizeof(int)*m_cachesize);
	m_curtime=0;
}

int Cache::comparekeys(const void *key1, const void *key2) const
{
	return memcmp(key1,key2,m_keysize);
}

int Cache::findindex(const void *key) const
{
	int i;
	
	for(i=0;i<m_cachesize;i++)
		if(!comparekeys(key,getkey(i)))
			return i;
	return -1;
}

void Cache::reward(int index)
{
	m_time[index]=++m_curtime;
}

int Cache::findoldest() const
{
	int i,ioldest;
	
	ioldest=0;
	for(i=0;i<m_cachesize;i++){
		if(!m_time[i])
			return i;
		if(m_time[i]<m_time[ioldest])
			ioldest=i;
	}
	return ioldest;
}

