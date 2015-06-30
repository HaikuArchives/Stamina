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

/*	Nom :				httpurl.h
	Type :				header C++
	Auteur :			Sylvain Demongeot
	Date de Création:	janvier 1998
	Projet :			Webgrabber
	Environnement :		HTTP, TCP/IP, HTML
	Fonction :			URL (Universal Resource Locator)
	Remarques : 		-
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#ifndef _HTTPURL_H_
#define _HTTPURL_H_

void simplifypath(char *path);

class CHttpURL{
public:
	CHttpURL(){m_valid=0;}
	CHttpURL(const char *host, int port, 
		const char *path, const char *fragment);
	CHttpURL(const char *string){set(string);}
	
	int set(const char *string);
	void get(char *string) const;
	void getsimple(char *string) const;
	void getuniquefile(const char* prefix, char *string) const;
	int setrel(const CHttpURL& base, const char *string);
	void sethost(const char *host);
	void simplifypath(void) {::simplifypath(m_path);}
	
	char m_host[80];
	int m_port;
	char m_path[B_PATH_NAME_LENGTH];
	char m_fragment[80];
	int m_valid;
};

#endif	// _HTTPURL_H_
