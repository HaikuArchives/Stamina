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

/*	Nom :				getfileviahttp_p.h
	Type :				Header C
	Auteur :			Sylvain Demongeot
	Date de Création:	avril 1998
	Projet :			Charisma
	Environnement :		BeOS, TCP/IP, HTTP
	Fonction :			Capture d'un fichier via HTTP 1.0, version Proxy
	Remarques : 		-
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#ifndef _GETFILEVIAHTTP_P_H_
#define _GETFILEVIAHTTP_P_H_

/*
	retourne:
	0	OK
	<0	erreur
*/
int getFileViaHttp_p(	
	unsigned long ipaddress,// l'adresse IP où se connecter
	int port,				// le port où se connecter
	const char *path,		// le chemin d'accès du fichier
	const char *orghost,	// l'hôte d'origine (ou NULL)
	int orgport,			// le port d'origine
	const char *sdate,		// la date pour If-Modified-Since (ou NULL)
	int headonly,			// non-nul si le corps du fichier ne doit pas être transmis
	int fd,					// le fichier de sortie (et entrée en cas de 304)
	int soout,				// le socket de sortie
	int *reply,				// en retour, le code retourné par le serveur (NULL permis)
	char *es);				// en retour, une chaine d'erreur (80 car, NULL permis)

#endif	// _GETFILEVIAHTTP_P_H_
