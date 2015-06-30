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

/*	Nom :				txcache.h
	Type :				Header c++
	Auteur :			Sylvain Demongeot
	Date de Création:	Janvier 1998
	Projet :			-
	Environnement :		BeOS
	Fonction :			Mécanisme générique et simple de cache
	Remarques : 		thread-safe
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#ifndef _TXCACHE_H_
#define _TXCACHE_H_

#include <cache.h>

class TXCache: public Cache{
public:
	TXCache(int cachesize, int keysize, int datasize); 
	~TXCache(); 

	// cherche les données dans le cache ou via getdata
	// puis les données sont copiées dans data
	void lookup(const void *key, void *data);
	
	// vide le cache
	void flush();
	
protected:	
	void waitforall();
	void killall();
	static status_t fetchdata_(void *params);
	
	thread_id *m_fetcher;
	sem_id m_fetchers_sem;
	BLocker m_lock;	
};

#endif // _TXCACHE_H_
