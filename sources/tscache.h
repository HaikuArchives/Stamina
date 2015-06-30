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

/*	Nom :				tscache.h
	Type :				Header C++
	Auteur :			Sylvain Demongeot
	Date de Création:	Janvier 1998
	Projet :			-
	Environnement :		BeOS
	Fonction :			Version thread-safe de TCache
	Remarques : 		thread-safe
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#ifndef _TSCACHE_H_
#define _TSCACHE_H_

#include <cache.h>

class TSCache: public Cache{
public:
	TSCache(int cachesize, int keysize, int datasize, int maxclients); 
	~TSCache(); 

	// cherche les données dans le cache ou via getdata
	// puis les données sont copiées dans data
	void lookup(const void *key, void *data);
	
	// vide le cache
	void flush();
	
protected:	
	status_t loop();
	status_t fetch();
	int registerfetcher(thread_id fetcher, thread_id calling);
	void waitforall();
	
	static status_t loop_(void *this_){
		return ((TSCache*)this_)->loop();}
	static status_t fetch_(void *this_){
		return ((TSCache*)this_)->fetch();}

	int m_maxclients;
	thread_id m_loopthread;
	int32 *m_fetcher;
	thread_id *m_back_fetcher;
	thread_id *m_back_calling;
	BLocker m_lock;		// protège l'accès à m_back_fetcher et m_back_calling
};

#endif // _TSCACHE_H_
