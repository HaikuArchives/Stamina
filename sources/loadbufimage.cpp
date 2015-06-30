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

/*	Nom :				loadbufimage.cpp
	Type :				Source C
	Auteur :			Sylvain Demongeot
	Date de Création:	Mai 1998
	Projet :			Charisma, Stamina
	Environnement :		BeOS
	Fonction :			lancement d'un binaire contenu dans un buffer
	Remarques : 		Appeler resume_thread après loadbufimage
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#include <errno.h>
#include <unistd.h>
#include <loadbufimage.h>

extern char **environ;

thread_id loadbufimage(const char *buf, size_t bufsize, const char *imagename)
{
	thread_id tid;
	int fd;
	char fname[B_PATH_NAME_LENGTH];
	int r;
	char *argv[1];
	
	tid=find_thread(imagename);
	if(tid>0) return tid;
	
	r=find_directory(B_COMMON_TEMP_DIRECTORY,NULL,true,fname,sizeof fname);
	if(r) return r;
	strcat(fname,"/");
	strcat(fname,imagename);

	fd=open(fname,B_WRITE_ONLY|B_CREATE_FILE);
	if(fd<0) return errno;
	
	r=write(fd,buf,bufsize);
	if(r<0){
		close(fd);
		return errno;
	}
	
	close(fd);

	argv[0]=fname;	
	tid=load_image(1,argv,environ);
	return tid;
}