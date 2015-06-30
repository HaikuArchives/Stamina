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

/*	Nom :				afl_server.cpp
	Type :				Source C
	Auteur :			Sylvain Demongeot
	Date de Création:	Février 1998
	Projet :			afl_server
	Environnement :		BeOS
	Fonction :			vérouillage coopératif de fichiers (partie serveur)
	Remarques : 		-
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <afl_client.h>

#define ATTRTYPE 'afls'

status_t afl_main(void *data);

int main(void)
{
	char path[300];
	int code;
	thread_id tid;
	system_info sinfo;
	thread_info tinfo;
	int fd;
	int r;
	struct{
		int lock;
		bigtime_t sessionSN;
		thread_id tid;
	}lockdata;
	
	signal(SIGINT,SIG_IGN);	// protection anti-cons

	get_system_info(&sinfo);

	for(;;){
		code=receive_data(&tid,&path,sizeof path);
		path[sizeof path-1]=0;
		
		fd=open(path,B_READ_WRITE);
		if(fd<0){
			send_data(tid,errno,NULL,0);
			continue;
		}
		
		r=fs_read_attr(fd,AFL_SERVER_NAME,ATTRTYPE,
			0,&lockdata,sizeof lockdata);
		
		switch(code){
		case AFLMSG_LOCK:
			if(r==sizeof lockdata
			&& lockdata.lock
			&& lockdata.sessionSN==sinfo.boot_time
			&& !(get_thread_info(lockdata.tid,&tinfo))){
				// le fichier est déjà vérouillé !
				close(fd);
				send_data(tid,AFLMSG_ISLOCKED,&lockdata.tid,sizeof lockdata.tid);
				break;
			}

			lockdata.lock=1;
			lockdata.sessionSN=sinfo.boot_time;
			lockdata.tid=tid;
			r=fs_write_attr(fd,AFL_SERVER_NAME,ATTRTYPE,
				0,&lockdata,sizeof lockdata);
			close(fd);
			if(r<0)
				send_data(tid,errno,0,NULL);
			else
				send_data(tid,B_OK,0,NULL);
			break;
			
		case AFLMSG_UNLOCK:
			if(r!=sizeof lockdata
			|| !lockdata.lock
			|| lockdata.sessionSN!=sinfo.boot_time
			|| lockdata.tid!=tid){
				// le fichier n'est pas vérouillé par ce thread !
				close(fd);
				send_data(tid,B_PERMISSION_DENIED,0,NULL);
				break;
			}

			r=fs_remove_attr(fd,AFL_SERVER_NAME);
			close(fd);
			if(r<0)
				send_data(tid,errno,0,NULL);
			else
				send_data(tid,B_OK,0,NULL);
			break;
			
		default:
			close(fd);
			send_data(tid,B_BAD_VALUE,0,NULL);
		}
	}
	
	return 0;
}
