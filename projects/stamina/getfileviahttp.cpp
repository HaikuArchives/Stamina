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

/*	Nom :				getfileviahttp.cpp
	Type :				Source C
	Auteur :			Sylvain Demongeot
	Date de Création:	1997
	Projet :			Stamina
	Environnement :		BeOS, TCP/IP, HTTP
	Fonction :			Capture d'un fichier via HTTP 1.0
	Remarques : 		-
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <net/netdb.h>
#include <net/socket.h>
#include <strcmp_ic.h>
#include <httptime.h>
#include <tools.beos.h>
#include <getfileviahttp.h>
#include <progversion.h>
#include <printf_patch.h>

#if 0
	#define DBG(A) A
#else
	#define DBG(A)
#endif

int removehttpheaders(int fd);

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
	int32 *totalvolume)		// nombre d'octets lus (à incrémenter, NULL permis)			
{
	int so;					// le socket utilisé pour la transaction
	sockaddr_in sa;			// le socket distant
	char buf[4096];			// le tampon pour les échanges
	int cb;					// le nombre d'octets lus ou écrits
	int size;				// nombre de caracteres dans buf
	int reply_;				// code de réponse HTTP
	int firstline;			// flag; vaut 1 pour la première ligne
	int header;				// flag; vaut 1 dans le header
	char *nextline;			// pointeur vers la ligne suivante
	int linesize;			// taille de la ligne
	long contentlength;		// taille du document
	int knowLength;			// non-nul si on connait la taille du fichier
	long loadedlength;		// nombre d'octets chargés
	char attrname[80];		// nom de l'attribut (y compris partie fixe)
	char *attrshortname;	// nom de l'attribut (sans partie fixe)
	char attrdata[80];		// valeur de l'attribut
	int r;					// code de retour
	int i,b;
	int transferbegun;
	char snow[40];
		
	strcpy(attrname,"HTTP:");
	attrshortname=attrname+strlen(attrname);
	if(reply) *reply=0;
	if(mimetype) *mimetype=0;
	
	so=socket(AF_INET,SOCK_STREAM,0);
	if(so==-1){
		if(es) sprintf(es,"Could not open internet connection: %s",strerror(errno));
		r=errno;
		goto err3;
	}	
	
	sa.sin_family=AF_INET;
	sa.sin_addr.s_addr=ipaddress;	// pas de htonl, curieusement
	sa.sin_port=htons(port);
	memset(sa.sin_zero,0,sizeof sa.sin_zero);
	
DBG(printf("Connecting to %s:%d\n",inet_ntoa(sa.sin_addr),ntohs(sa.sin_port));)
	if(connect(so,(sockaddr*)&sa,sizeof sa)<0){
		if(es) sprintf(es,"Could not connect to host: %s",strerror(errno));
		r=errno;
		goto err2;
	}
	
	sprintf(buf,
		"GET %s HTTP/1.0\r\n"
		"User-Agent: Stamina/" SPROGVERSION "\r\n",
		path);
		
	if(orghost)
		if(orgport==80)
			sprintf(buf+strlen(buf),
				"Host: %s\r\n",orghost);
		else
			sprintf(buf+strlen(buf),
				"Host: %s:%d\r\n",orghost,orgport);

	if(sdate)
		sprintf(buf+strlen(buf),
			"If-Modified-Since: %s\r\n",sdate);

	strcat(buf,"\r\n");
	
	if(send(so,buf,strlen(buf),0)<=0){
		if(es) strcpy(es,"Connection with host was broken.");
		r=errno;
		goto err2;
	}
DBG(printf("SEND> %s",buf);	)
		
	size=0;
	contentlength=0;
	firstline=1;
	header=1;
	knowLength=0;
	transferbegun=0;
	
	while(header){
	
		cb=recv(so,buf+size,sizeof buf-size-1,0);
		if(cb<=0){
			if(es) strcpy(es,"Connection with host was broken.");
			r=errno;
			goto err1;
		}
DBG(	printf("RECV> %.*s",cb,buf+size);	)
		size+=cb;	// cb octets de plus dans buf

		if(!transferbegun){
			write_attr_str(fdout,"HTTP:STATUS","busy");	// en cours de transfert
			removehttpheaders(fdout);
			transferbegun=1;
		}

		while(header && size){			
			buf[size]=0;					// on ferme la chaîne
			nextline=strchr(buf,'\n');		// on cherche le LF suivant
			if(!nextline) break;			// plus de LF dans buf
			nextline++;						// on se place après le LF		
			linesize=nextline-buf;		
DBG(		printf("LINE> %.*s",linesize,buf);	)

			if(firstline){
				if(1!=sscanf(buf,"HTTP/%*d.%*d %d",&reply_)){
					if(es) strcpy(es,"Unexpected reply from host. Not an HTTP server!");
					r=-1;
					goto err1;
				}
				if(reply)*reply=reply_;
				
				buf[linesize-2]=0;	// on remplace CRLF par 0				
				fs_write_attr(fdout,"HTTP:REPLY",B_STRING_TYPE,0,buf,linesize-1);

				firstline=0;
				goto next;
			}
		
			if(*buf=='\r'){
				header=0;	// ligne vide : fin du header
				goto next;
			}
			
			if(2==sscanf(buf,"%40[^:]:%*[ \t]%80[^\r\n]",attrshortname,attrdata)
			&& strcmp_ic(attrshortname,"connection")){
				for(i=0;attrshortname[i];i++)
					attrshortname[i]=tolower(attrshortname[i]);
					
				b=!strcmp(attrshortname,"content-type");
				write_attr_str(fdout,b?"BEOS:TYPE":attrname,attrdata);
				if(b && mimetype) strcpy(mimetype,attrdata);
					
				if(!strcmp(attrshortname,"content-length")
				&& 1==sscanf(attrdata,"%d",&contentlength)){
					knowLength=1;
					if(maxlength && contentlength>maxlength){
						if(es) sprintf(es,"Document is more than %d bytes in size.",maxlength);
						r=-1;
						goto err1;
					}
				}
			}

		next:		// ligne suivante
			memmove(buf,nextline,size-linesize);
			size-=linesize;	// on supprime cette ligne de buf
		}
	}
	
	if(reply_==304)	// not modified
		goto noerr;
	
	if(knowLength && contentlength<=0){
		if(es) strcpy(es,"Document is empty.");
		r=-1;
		goto err1;
	}
	
	ftruncate(fdout,0);
	loadedlength=0;
	for(;;){
		if(size){
			if(write(fdout,buf,size)<0){
				if(es) sprintf(es,"Could not write to disk: %s",strerror(errno));
				r=errno;
				goto err1;
			}
			if(totalvolume) atomic_add(totalvolume,size);
			loadedlength+=size;
			if(maxlength && loadedlength>maxlength){
				if(es) sprintf(es,"Document is more than %d bytes in size.",maxlength);
				r=-1;
				goto err1;
			}
			if(knowLength && loadedlength>=contentlength) break;
		}
		
		size=recv(so,buf,sizeof buf,0);
		if(size<=0)
			if(knowLength){
				if(es) strcpy(es,"Transfer interrupted");
				r=errno;
				goto err1;
			}else
				break;
DBG(	printf("RECV> %.*s",size,buf);	)
	}
	
noerr:	
	// transfert effectué avec succès
	http_timetostring(time(NULL),snow,sizeof snow);
	write_attr_str(fdout,"HTTP:LOCALDATE",snow);
	if(es)*es=0;
	r=0;
err1:
	if(transferbegun)
		write_attr_str(fdout,"HTTP:STATUS",r?"error":"OK");
err2:
	closesocket(so);
err3:
	return r;
}	

int removehttpheaders(int fd)
{
	DIR *dir;
	struct dirent *ent;
	char buf[1024];
	int pos,l;
	int i,n;

	dir=fs_fopen_attr_dir(fd);
	if(!dir){
DBG(	printf("fs_fopen_attr_dir failed: %s\n",strerror(errno));)
		return errno;
	}

	pos=0;
	n=0;
	while(ent=fs_read_attr_dir(dir))
		if(!memcmp(ent->d_name,"HTTP:",5)
		&& islower(ent->d_name[5])){
			l=strlen(ent->d_name)+1;
			if(pos+l>sizeof buf) break;
			memcpy(buf+pos,ent->d_name,l);
			pos+=l;
			n++;
		}
	
	pos=0;
	for(i=0;i<n;i++){
		fs_remove_attr(fd,buf+pos);
		pos+=strlen(buf+pos)+1;
	}
	
	fs_close_attr_dir(dir);	
	
	return 0;
}

