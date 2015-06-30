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

#include <errno.h>
#include <create_directory_.h>

status_t create_directory_(const char *path, mode_t mode)
{
	int r;
	int i,j;
	char path1[B_PATH_NAME_LENGTH];
	struct stat st;
	
	if(!stat(path,&st)){					// le chemin existe
		if(S_ISDIR(st.st_mode))				// et c'est un répertoire
			return 0;
		else{								// ce n'est pas un répertoire
			sprintf(path1,"%s:FILE",path);	// on change son nom
			r=rename(path,path1);
			if(r<0) return r;
		}
	}else{									// le chemin n'existe pas
		j=0;								// on calcule le chemin du parent
		for(i=0;path[i];i++)
			if(path[i]=='/') j=i;	
		
		if(j){
			memcpy(path1,path,j);
			path1[j]=0;
		
			r=create_directory_(path1,mode); // on crée le parent
			if(r<0) return r;
		}
	}
	
	r=mkdir(path,mode);						// on crée le répertoire	
	if(!r || errno==EEXIST)
		return 0;
	else
		return r;
}

