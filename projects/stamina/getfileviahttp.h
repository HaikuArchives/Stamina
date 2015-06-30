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

/*	Nom :				getfileviahttp.h
	Type :				Header C
	Auteur :			Sylvain Demongeot
	Date de Création:	1997
	Projet :			Stamina
	Environnement :		BeOS, TCP/IP, HTTP
	Fonction :			Capture d'un fichier via HTTP 1.0
	Remarques : 		-
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#ifndef _GETFILEVIAHTTP_H_
#define _GETFILEVIAHTTP_H_

/*
	retourne:
	0	OK
	<0	erreur
*/
int getFileViaHttp(	
	unsigned long ipaddress,// l'adresse IP où se connecter
	int port,				// le port où se connecter
	const char *path,		// le chemin d'accès du fichier
	const char *orghost,	// l'hôte d'origine (ou NULL)
	int orgport,			// le port d'origine
	const char *sdate,		// la date pour If-Modified-Since (ou NULL)
	size_t maxlength,		// taille à ne pas dépasser (ou 0)
	int fdout,				// le fichier de sortie
	int *reply,				// en retour, le code retourné par le serveur (NULL permis)
	char *es,				// en retour, une chaine d'erreur (80 car, NULL permis)
	char *mimetype,			// en retour, le type MIME (NULL permis)			
	int32 *totalvolume);	// nombre d'octets lus (à incrémenter, NULL permis)			

#endif	// _GETFILEVIAHTTP_H_
