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

/*	Nom :				afl_client.cpp
	Type :				Source C
	Auteur :			Sylvain Demongeot
	Date de Création:	Février 1998
	Projet :			afl_server
	Environnement :		BeOS
	Fonction :			vérouillage coopératif de fichiers (partie client)
	Remarques : 		-
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#include <loadbufimage.h>
#include <afl_client.h>

extern char afl_server[4531];

static thread_id server_thread=0;

int afl_init(void)
{
	server_thread=loadbufimage(afl_server,sizeof afl_server,AFL_SERVER_NAME);
	if(server_thread<0) return server_thread;
	
	resume_thread(server_thread);
	
	return 0;	
}

int afl_lock(const char *path)
{
	thread_id tid;
	status_t status;
	int r;
	
	do{
		r=afl_talk(AFLMSG_LOCK,path,&tid,sizeof tid);
		if(r==AFLMSG_ISLOCKED)
			wait_for_thread(tid,&status);
	}while(r==AFLMSG_ISLOCKED);
	
	return r;
}

int afl_unlock(const char *path)
{
	return afl_talk(AFLMSG_UNLOCK,path,NULL,0);
}

int afl_talk(int code, const char *path, void *reply, int replysize)
{
	thread_id tid;
	int r;

	if(server_thread<=0) return-1;

	r=send_data(server_thread,code,path,strlen(path)+1);
	if(r) return r;
	
	r=receive_data(&tid,reply,replysize);
//	printf("tid=%d\n",tid);
	return r;
}
