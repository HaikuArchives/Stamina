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

/*	Nom :				afl_client.h
	Type :				Header C
	Auteur :			Sylvain Demongeot
	Date de Création:	Février 1998
	Projet :			afl_server
	Environnement :		BeOS
	Fonction :			vérouillage coopératif de fichiers (partie client)
	Remarques : 		"path" doit être absolu
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#ifndef _AFL_CLIENT_H
#define _AFL_CLIENT_H

#define AFL_SERVER_NAME "afl_server"

// les codes de message entre les clients et le serveur
enum{
	AFLMSG_LOCK='lock',
	AFLMSG_UNLOCK='ulck',
	AFLMSG_ISLOCKED='lckd'
};

int afl_init(void);
int afl_lock(const char *path);
int afl_unlock(const char *path);
int afl_talk(int code, const char *path, void *reply, int replysize);

#endif	// _AFL_CLIENT_H
