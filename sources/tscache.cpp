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

/*	Nom :				tscache.cpp
	Type :				Source C++
	Auteur :			Sylvain Demongeot
	Date de Création:	Janvier 1998
	Projet :			-
	Environnement :		BeOS
	Fonction :			Version thread-safe de TCache
	Remarques : 		thread-safe
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#include <string.h>
#include <cache.h>
#include <tscache.h>

enum{
	kMsg_Lookup='sdLU',
	kMsg_DataIs='sdDI',
	kMsg_Fetch='sdFT',
	kMsg_Flush='sdFL',
	kMsg_Stop='sdST',
	kMsg_Error='sdER'
};

TSCache::TSCache(int cachesize, int keysize, int datasize, int maxclients):
	Cache(cachesize,keysize,datasize)
{
	m_maxclients=maxclients;

	m_fetcher=new thread_id[cachesize];
	m_back_fetcher=new thread_id[maxclients];
	m_back_calling=new thread_id[maxclients];
	
	memset(m_fetcher,0,sizeof(thread_id)*m_cachesize);
	memset(m_back_fetcher,0,sizeof(thread_id)*maxclients);
	memset(m_back_calling,0,sizeof(thread_id)*maxclients);	// facultatif

	m_loopthread=spawn_thread(loop_,"TSCache::loop",
		B_NORMAL_PRIORITY,this);
	resume_thread(m_loopthread);
}

// nota: dans l'idéal, le destructeur devrait être appelé après retour
// de tous les appels au serveur.
TSCache::~TSCache()
{
	status_t status;

	send_data(m_loopthread,kMsg_Stop,NULL,0);
	wait_for_thread(m_loopthread,&status);
	waitforall();

	delete[] m_fetcher;
	delete[] m_back_fetcher;
	delete[] m_back_calling;
}

void TSCache::lookup(const void *key, void *data)
{
	thread_id st;
	
	send_data(m_loopthread,kMsg_Lookup,key,m_keysize);
	if(receive_data(&st,data,m_datasize)!=kMsg_DataIs)
		memset(data,0,m_datasize);
}

void TSCache::flush()
{
	send_data(m_loopthread,kMsg_Flush,NULL,0);
}

status_t TSCache::loop() 
{
	char *key;
	int32 code;
	int index;
	thread_id calling,newfetcher,tid;
	status_t status;

	key=new char[m_keysize];

	for(;;){
		code=receive_data(&calling,key,m_keysize);
		switch(code){
		case kMsg_Lookup:
			index=findindex(key);
			if(index>=0){
				if(m_fetcher[index]){
					if(registerfetcher(m_fetcher[index],calling))
						send_data(calling,kMsg_Error,NULL,0);
				}else
					send_data(calling,kMsg_DataIs,getdata(index),m_datasize);
				reward(index);
			}else{
				index=findoldest();
				tid=m_fetcher[index];
				if(tid) wait_for_thread(tid,&status);
				memcpy(getkey(index),key,m_keysize);
				newfetcher=spawn_thread(fetch_,"TSCache::fetch",
					B_NORMAL_PRIORITY,this);
				m_fetcher[index]=newfetcher;
				send_data(newfetcher,kMsg_Fetch,&index,sizeof index);
				if(registerfetcher(newfetcher,calling))
					send_data(calling,kMsg_Error,NULL,0);
				resume_thread(newfetcher);
				reward(index);
			}
			break;
			
		case kMsg_Flush:
			waitforall();
			Cache::flush();
			memset(m_fetcher,0,sizeof(thread_id)*m_cachesize);
			memset(m_back_fetcher,0,sizeof(thread_id)*m_maxclients);
			memset(m_back_calling,0,sizeof(thread_id)*m_maxclients);	// facultatif
			break;
		
		case kMsg_Stop:
			goto break1;
		}
	}
	
break1:
	delete[] key;
	
	return 0;
}

status_t TSCache::fetch()
{
	char *key,*data;
	thread_id st,fetcher;
	int index;
	int i;
	
	receive_data(&st,&index,sizeof index);
	fetchdata(getkey(index),getdata(index));

	fetcher=m_fetcher[index];
	m_lock.Lock();
	for(i=0;i<m_maxclients;i++)
		if(m_back_fetcher[i]==fetcher){
			send_data(m_back_calling[i],kMsg_DataIs,getdata(index),m_datasize);
				// ne devrait pas bloquer si TSCache est correctement utilisé
			m_back_fetcher[i]=0;
			m_back_calling[i]=0;	// facultatif
		}
	m_lock.Unlock();
	m_fetcher[index]=0;
	
	return 0;
}

int TSCache::registerfetcher(thread_id fetcher, thread_id calling)
{
	int i,r;

	r=0;
	m_lock.Lock();
	for(i=0;i<m_maxclients;i++)
		if(!m_back_fetcher[i]){
			m_back_fetcher[i]=fetcher;
			m_back_calling[i]=calling;
			goto out;
		}
	r=-1;
out:
	m_lock.Unlock();
	return r;
}

void TSCache::waitforall()
{
	int i;
	status_t status;

	for(i=0;i<m_cachesize;i++)
		if(m_fetcher[i])
			wait_for_thread(m_fetcher[i],&status);
}


