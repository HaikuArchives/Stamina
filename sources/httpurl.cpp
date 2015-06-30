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

/*	Nom :				httpurl.cpp
	Type :				Source C++
	Auteur :			Sylvain Demongeot
	Date de Création:	janvier 1998
	Projet :			Stamina, Charisma
	Environnement :		TCP/IP, HTTP, HTML
	Fonction :			URL (Universal Resource Locator)
	Remarques : 		-
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <httpurl.h>
#include <printf_patch.h>

CHttpURL::CHttpURL(const char *host, int port, 
	const char *path, const char *fragment)
{
	sethost(host);
	m_port=port;
	strcpy(m_path,path);
	strcpy(m_fragment,fragment);
	m_valid=1;
}

int CHttpURL::set(const char *string)
{
	int i,j,n;
	
	m_valid=0;
	
	// scheme
	for(i=0;isalpha(string[i]);i++);
	if(string[i]==':')	// scheme spécifié
		if(strncmp(string,"http",i)
		&& strncmp(string,"HTTP",i)) return -1;	// pas HTTP
		else{
			i++;
			if(string[i]!='/'||string[i+1]!='/') return -1; // "//" obligatoire !
		}
	else i=0;	// sinon: http sous-entendu
	
	// on passe "//"	
	if(string[i]=='/'&&string[i+1]=='/') i+=2;
	
	// host
	j=0;
	for(;string[i] && string[i]!=':' && string[i]!='/';i++)
		m_host[j++]=tolower(string[i]);
	if(j==0) return -1;	// 1 lettre au moins
	m_host[j]=0;
	
	// port
	if(string[i]==':'){
		if(1!=sscanf(string+i,":%d%n",&m_port,&n)) return -1;
		i+=n;
	}else
		m_port=80;
	
	// path#fragment
	m_fragment[0]=0;
	if(string[i]){
		sscanf(string+i,"%[^#]%s",m_path,m_fragment);
		simplifypath();
	}
	else strcpy(m_path,"/");
	
	m_valid=1;
	return 0;
}

void CHttpURL::get(char *string) const
{
	if(m_port==80)
		sprintf(string,"http://%s%s%s",m_host,m_path,m_fragment);
	else
		sprintf(string,"http://%s:%d%s%s",m_host,m_port,m_path,m_fragment);
}

void CHttpURL::getsimple(char *string) const
{
	if(m_port==80)
		sprintf(string,"//%s%s",m_host,m_path);
	else
		sprintf(string,"//%s:%d%s",m_host,m_port,m_path);
}

void CHttpURL::getuniquefile(const char* prefix, char *string) const
{
	int l;
	
	if(m_port==80)
		sprintf(string,"%s%s%s",prefix,m_host,m_path);
	else
		sprintf(string,"%s%s:%d%s",prefix,m_host,m_port,m_path);
	l=strlen(string);
	if(string[l-1]=='/')
		strcpy(string+l,"index.html");
}

int CHttpURL::setrel(const CHttpURL& base, const char *string)
{
	int i,j;
	
	m_valid=0;
	if(!base.m_valid) return -1;
	
	for(i=0;isalpha(string[i]);i++);
	if(string[i]==':')	// scheme spécifié
		return set(string);
		
	if(string[0]=='/' && string[1]=='/')
		return set(string);		// net path
		
	strcpy(m_host,base.m_host);
	m_port=base.m_port;
	
	if(string[0]=='#'){
		strcpy(m_path,base.m_path);
		strcpy(m_fragment,string);
		
	}else{
		m_fragment[0]=0;
		if(string[0]=='/')	// abs path
			sscanf(string,"%[^#]%s",m_path,m_fragment);	
			
		else{				// rel path
			j=0;
			for(i=0;base.m_path[i];i++)
				if(base.m_path[i]=='/') j=i+1;				
			memcpy(m_path,base.m_path,j);
			sscanf(string,"%[^#]%s",m_path+j,m_fragment);
			simplifypath();
		}
	}
		
	m_valid=1;
	return 0;
}

void CHttpURL::sethost(const char *host)
{
	int i;
	
	for(i=0;host[i];i++)
		m_host[i]=tolower(host[i]);
	m_host[i]=0;
}

// transforme /boot/toto/../titi en /boot/titi
void simplifypath(char *path)
{
	int i,j,k,j0;

	if(path[0]!='/') return;

	i=1;
	while(path[i]){
		if(path[i]=='.' && path[i+1]=='/'){
			for(j=i,k=i+2;path[k];j++,k++)
				path[j]=path[k];
			path[j]=0;
			continue;
		}
		if(path[i]=='.' && path[i+1]=='.' && path[i+2]=='/'){
			for(j0=i-1;j0>0;j0--)
				if(path[j0-1]=='/') break;
			if(j0<=0) j0=1;
			for(j=j0,k=i+3;path[k];j++,k++)
				path[j]=path[k];
			path[j]=0;
			i=j0;
			continue;
		}
		do i++;
		while(path[i] && path[i-1]!='/');
	}
}
