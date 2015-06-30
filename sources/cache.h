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

/*	Nom :				cache.h
	Type :				Header C++
	Auteur :			Sylvain Demongeot
	Date de Création:	Novembre 1997
	Projet :			Webgrabber
	Environnement :		-
	Fonction :			Mécanisme générique et simple de cache
	Remarques : 		non thread-safe
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#ifndef _CACHE_H_
#define _CACHE_H_

class Cache{
public:
	Cache(int cachesize, int keysize, int datasize); 
	~Cache(); 
	
	// cherche les données dans le cache ou via fetchdata
	// puis les données sont copiées dans data
	// retourne 0 si OK
	void lookup(const void *key, void *data);
	
	// vide le cache
	void flush();

protected:
	// va chercher les données en fonction de la clé
	// les données sont copiées
	virtual void fetchdata(const void *key, void *data)=0;
	
	// compare deux clés, retourne 0 si elles sont identiques
	virtual int comparekeys(const void *key1, const void *key2) const;

	int findindex(const void *key) const;
	void reward(int index);
	int findoldest() const;
	
	void *getkey(int index) const {return m_keys+index*m_keysize;}
	void *getdata(int index) const {return m_data+index*m_datasize;}
	
	int m_cachesize;
	int m_keysize;
	int m_datasize;
	char *m_keys;
	char *m_data;
	int *m_time;
	int m_curtime;
};

#endif	// _CACHE_H_
