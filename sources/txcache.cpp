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

/*	Nom :				txcache.cpp
	Type :				Source C++
	Auteur :			Sylvain Demongeot
	Date de Création:	Janvier 1998
	Projet :			-
	Environnement :		BeOS
	Fonction :			Mécanisme générique et simple de cache
	Remarques : 		thread-safe
	Bugs :				assert(m_fetcher[i]==0) failed !
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#include <string.h>
#include <assert.h>
#include <cache.h>
#include <txcache.h>

typedef struct{
	TXCache *txcache;
	const void *key;
	void *data;
}t_fetchdata_params;

TXCache::TXCache(int cachesize, int keysize, int datasize):
	Cache(cachesize,keysize,datasize)
{
	m_fetcher=new thread_id[cachesize];
	memset(m_fetcher,0,sizeof(thread_id)*m_cachesize);
	m_fetchers_sem=create_sem(cachesize,"TXCache::m_fetchers_sem");
}

TXCache::~TXCache()
{
	killall();
	delete[] m_fetcher;
}

void TXCache::lookup(const void *key, void *data)
{
	status_t r;
	int i;
	t_fetchdata_params p;
	thread_id tid;

restart:
	m_lock.Lock();
	i=findindex(key);
	if(i<0){
		acquire_sem(m_fetchers_sem);
		i=findoldest();
		assert(m_fetcher[i]==0);
		reward(i);	// ceci empêchera i d'être choisi par findoldest() 
					//  dans un autre thread.
		memcpy(getkey(i),key,m_keysize);
		p.txcache=this;
		p.key=key;
		p.data=getdata(i);
		tid=spawn_thread(fetchdata_,"TXCache::fetchdata",
			B_NORMAL_PRIORITY,&p);
		if(tid<B_NO_ERROR){
			printf("spawn_thread failed file=%s line %d\n",__FILE__,__LINE__);
			exit(-1);
		}
		m_fetcher[i]=tid;
		m_lock.Unlock();	// on déverouille. On est protégé par m_fetchers_sem
		wait_for_thread(tid,&r);
		m_lock.Lock();
		m_fetcher[i]=0;
		release_sem(m_fetchers_sem);
	}else{
		tid=m_fetcher[i];
		if(tid){
			m_lock.Unlock();
			wait_for_thread(tid,&r);
			goto restart;
		}
	}
	memcpy(data,getdata(i),m_datasize);
	reward(i);
	m_lock.Unlock();
}

void TXCache::flush()
{
	m_lock.Lock();
	waitforall();
	Cache::flush();
	memset(m_fetcher,0,sizeof(thread_id)*m_cachesize);
	m_lock.Unlock();
}

void TXCache::waitforall()
{
	int i;
	status_t status;

	for(i=0;i<m_cachesize;i++)
		if(m_fetcher[i])
			wait_for_thread(m_fetcher[i],&status);
}

void TXCache::killall()
{
	int i;
	status_t status;

	for(i=0;i<m_cachesize;i++)
		if(m_fetcher[i]){
			kill_thread(m_fetcher[i]);
			wait_for_thread(m_fetcher[i],&status);
		}
}

status_t TXCache::fetchdata_(void *params)
{
	t_fetchdata_params *p=(t_fetchdata_params*)params;
	p->txcache->fetchdata(p->key,p->data);
	return 0;
}
