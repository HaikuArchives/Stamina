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

/*	Nom :				proxy.cpp
	Type :				Source c++
	Auteur :			Sylvain Demongeot
	Date de Création:	Février 1998
	Projet :			-
	Environnement :		BeOS, HTTP
	Fonction :			serveur HTTP de "Charisma"
	Remarques : 		-
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <tools.beos.h>
#include <getfileviahttp_p.h>
#include <httpurl.h>
#include <strcmp_ic.h>
#include <httptime.h>
#include <create_directory_.h>
#include <afl_client.h>
#include <hostcache.h>
#include <messages.h>
#include <fatal.h>
#include <progversion.h>
#include <proxy.h>
#include <printf_patch.h>

#if 0
	#define DBG(A) A
#else
	#define DBG(A)
#endif

#define MAXCLIENTS 30

enum{kmeth_none,kmeth_get,kmeth_head};

extern char bkgif[10982];
extern char gogif[273];

int g_mode=k_disabled;
int g_extcontrol=0;
time_t g_refreshdate=0;
char g_webdir[B_PATH_NAME_LENGTH]="/boot/home/theWWW/";

sem_id fd_sem;
Hostcache ghostcache(100);	// on peut faire meilleur choix...
thread_id tid_proxy;

// main.cpp
void go_online();
extern int32 hitcount;

status_t proxythread(void *data);
status_t serveclient(void *data);
int sendfile(int so, int fd, int replycode, int headonly);
int sendnotonline(int so, const char *url);
int sendresponseheader(int so, const char *response, 
	const char *contenttype="text/html", int contentlength=0);
int senderrorbody(int so, const char *title, const char *text);

void startproxy(void)
{
	tid_proxy=spawn_thread(proxythread,"proxythread",
		B_NORMAL_PRIORITY,NULL);
	if(tid_proxy<B_NO_ERROR)
		fatal(__FILE__,__LINE__,"spawn_thread failed");
	resume_thread(tid_proxy);
}

void stopproxy(void)
{
	thread_id tid;
	status_t r;

	kill_thread(tid_proxy);
	wait_for_thread(tid_proxy,&r);

	for(;;){
		tid=find_thread("Charisma/serveclient");
		if(tid<0) break;
		kill_thread(tid);
		wait_for_thread(tid,&r);
	}
}

status_t proxythread(void *data)
{
	int listenso;
	int port;
	struct sockaddr_in sa;
	int so;
	int size;
	thread_id tid;
		
	port=8080;
	
	// création su sémaphore fd_sem
	fd_sem=create_sem(128,"fd_sem");
	if(fd_sem<B_NO_ERROR)
		fatal(__FILE__,__LINE__,"create_sem failed");
	
	// création du socket
	listenso=socket(AF_INET,SOCK_STREAM,0);
	if(listenso<0)
		fatal(__FILE__,__LINE__,"socket failed");
	
	// on s'attache à l'adresse
	sa.sin_family=AF_INET;
	sa.sin_addr.s_addr=INADDR_ANY;
	sa.sin_port=htons(port);
	memset(sa.sin_zero,0,sizeof sa.sin_zero);
	if(bind(listenso, (sockaddr*)&sa, sizeof sa)<0)
		fatal(__FILE__,__LINE__,"Could not bind to port %d",port);
	
	// on écoute les requêtes
	if(listen(listenso,MAXCLIENTS)<0)
		fatal(__FILE__,__LINE__,"listen failed");
DBG(printf("Listening on port %d.\n",port);)
	
	for(;;){
		// on accepte une requête
		size=sizeof sa;
		so=accept(listenso, (sockaddr*)&sa, &size);
		if(so<0)
			fatal(__FILE__,__LINE__,"accept failed");
DBG(	printf("Connected to client %s:%d\n",inet_ntoa(sa.sin_addr),ntohs(sa.sin_port));)
		
		// on démarre un thread pour la traiter
		tid=spawn_thread(serveclient,"Charisma/serveclient",B_NORMAL_PRIORITY,(void*)so);
		if(tid<B_NO_ERROR)
			fatal(__FILE__,__LINE__,"spawn_thread failed");
		resume_thread(tid);
		
		atomic_add(&hitcount,1);
	}
	
	return 0;
}

status_t serveclient(void *data)
{
	int so;					// le socket connecté au client
	char buf[1024];			// le tampon pour les échanges
	int cb;					// le nombre d'octets lus ou écrits
	int size;				// nombre de caracteres dans buf
	char *nextline;			// pointeur vers la ligne suivante
	int linesize;			// taille de la ligne
	char url[B_PATH_NAME_LENGTH];
	int n;
	CHttpURL httpurl;
	char localfile[B_PATH_NAME_LENGTH],localdir[B_PATH_NAME_LENGTH];
	int fd;
	char sstatus[20];
	int firstline;
	int clientvmaj,clientvmin;
	char smethod[10];
	int method;
	char headertype[40],headerdata[80];
	time_t ifmodifiedsince;
	int i,j,l;
	status_t r;
	ulong ipaddress;
	char es[80];
	char sdate[80],slocaldate[80];
	time_t localdate;
	time_t refreshdate;
	char sreply[100];
	int reply;
	struct stat st;
	int mode;
	
	so=(int)data;	// le socket client

	mode=g_mode;	// le mode pour cette requête (k_offline ou k_online)
	if(mode==k_disabled){
		closesocket(so);
		return 0;
	}

	if(g_refreshdate<-2)
		refreshdate=time(NULL)+g_refreshdate; 		// (voir main.cpp)
	else 
		refreshdate=g_refreshdate;
		
	// on déchiffre la requête
	
	firstline=1;
	size=0;
	url[0]=0;
	ifmodifiedsince=0;

	for(;;){
		cb=recv(so,buf+size,sizeof buf-size-1,0);
//DBG(	printf("RECV> %.*s",cb,buf+size);	)
		if(cb<=0){
DBG(		printf("Connection with host was broken.");)
			r=errno;
			goto err3;
		}
		size+=cb;	// cb octets de plus dans buf

		while(size){			
			buf[size]=0;					// on ferme la chaîne
			nextline=strchr(buf,'\n');		// on cherche le LF suivant
			if(!nextline) break;			// plus de LF dans buf
			nextline++;						// on saute le LF		
			linesize=nextline-buf;		
DBG(		printf("LINE> %.*s",linesize,buf);	)
					
			// on traite la request-line
			if(firstline){
				n=sscanf(buf,"%9s %299s HTTP/%d.%d",
					smethod,url,&clientvmaj,&clientvmin);
				if(n<4){
DBG(				printf("HTTP/0.9 client\n");	)
					clientvmaj=0;
					clientvmin=9;
				}
				if(!strcmp_ic(smethod,"GET")) method=kmeth_get;
				else if(!strcmp_ic(smethod,"HEAD")) method=kmeth_head;
				else{
					method=kmeth_none;
DBG(				printf("Unhandled HTTP method\n");	)
				}
				firstline=0;
				if(clientvmaj<1) goto break1;	// pour HTTP/0.9: fin de la requête
			}else{

				if(*buf=='\r') goto break1;	// ligne vide : fin du header
	
				// on traite les header fields
				if(2==sscanf(buf,"%39[^:]:%*[ \t]%79[^\r\n]",headertype,headerdata)){
					if(!strcmp_ic(headertype,"If-Modified-Since"))
						ifmodifiedsince=http_stringtotime(headerdata);
				}
			}

			memmove(buf,nextline,size-linesize);
			size-=linesize;	// on supprime cette ligne de buf
		}
	}
break1:

DBG(printf("version=%d.%d, method=%d\n",clientvmaj,clientvmin,method);)
DBG(printf("if-modified-since=%d\n",ifmodifiedsince);)
DBG(printf("URL=%s\n",url);)

	// on vérifie que la méthode est GET ou HEAD
	if(method!=kmeth_get && method!=kmeth_head){
		if(clientvmaj>=1) 
			sendresponseheader(so,"400 Bad request");
		senderrorbody(so,"Bad request","<P>Unhandled HTTP method.</P>\n");
		r=0;
		goto err3;
	}
	
	// on traite les "@@@"
	l=strlen(url);
	if(url[l-3]=='@' && url[l-2]=='@' && url[l-1]=='@'){
		url[l-3]=0;
		if(g_extcontrol){
			g_mode=k_online;
			go_online();	// on modifie l'interface en conséquence
		}
		if(clientvmaj>=1){
			sprintf(buf,"301 Moved Permanently\r\n"
				"Location: %s",url);
			sendresponseheader(so,buf);
		}
		if(method!=kmeth_head){
			sprintf(buf,"<P>Moved to <A HREF=\"%s\">here</A>.</P>\n",url);
			senderrorbody(so,"Moved Permanently",buf);
		}
		r=0;
		goto err3;
	}

	// on vérifie que l'URL demandée est valide
	httpurl.set(url);
	if(!httpurl.m_valid){
		if(clientvmaj>=1) 
			sendresponseheader(so,"400 Bad request");
		if(method!=kmeth_head)
			senderrorbody(so,"Invalid URL",
				"<P>The URL you requested is not a valid HTTP URL.</P>\n");
		r=0;
		goto err3;
	}
	
	// on traite les pages Charisma
	if(!strcmp(httpurl.m_host,"charisma")){
		if(!strcmp(httpurl.m_path,"/")){
			if(clientvmaj>=1) 
				sendresponseheader(so,"200 OK");
			if(method!=kmeth_head)
				senderrorbody(so,"Charisma test page",
					"<P>Your browser was succesfully configured to interface with Charisma.<P>\n");
			r=0;
			goto err3;
		}else if(!strcmp(httpurl.m_path,"/background.gif")){
			if(clientvmaj>=1) 
				sendresponseheader(so,"200 OK","image/gif",sizeof bkgif);
			if(method!=kmeth_head)
				send(so,bkgif,sizeof bkgif,0);
			r=0;
			goto err3;
		}else if(!strcmp(httpurl.m_path,"/go.gif")){
			if(clientvmaj>=1) 
				sendresponseheader(so,"200 OK","image/gif",sizeof gogif);
			if(method!=kmeth_head)
				send(so,gogif,sizeof gogif,0);
			r=0;
			goto err3;
		}else{
			if(clientvmaj>=1) 
				sendresponseheader(so,"404 Not found");
			if(method!=kmeth_head)
				senderrorbody(so,"Document not found",
					"<P>The document you requested does not exist.</P>\n");
			r=0;
			goto err3;
		}
	}
	
	// on acquière fd_sem pour s'assurer qu'il reste au moins 1 fd
	r=acquire_sem(fd_sem);
	if(r){
		if(clientvmaj>=1) 
			sendresponseheader(so,"500 Internal server error");
		if(method!=kmeth_head)
			senderrorbody(so,"Internal server error",
				"<P>Semaphore cannot be acquired.</P>\n");
		goto err3;
	}

	// on détermine le chemin du fichier local
	httpurl.getuniquefile(g_webdir,localfile);	

	// on crée le parent du fichier local
	if(mode==k_online){
		for(i=0;localfile[i];i++)
			if(localfile[i]=='/') j=i;
		memcpy(localdir,localfile,j);
		localdir[j]=0;
		r=create_directory_(localdir,0777);
		if(r){
			if(clientvmaj>=1) 
				sendresponseheader(so,"500 Internal server error");
			if(method!=kmeth_head)
				senderrorbody(so,"Internal server error",
					"<P>The directory cannot be created</P>\n");
			goto err2;
		}
	}

	// on ouvre le fichier local
	if(!stat(localfile,&st) && S_ISDIR(st.st_mode))
		strcat(localfile,":FILE");	
	fd=open(localfile,mode==k_online?
		B_READ_WRITE|B_CREATE_FILE:B_READ_ONLY);
	if(fd<0){
		if(clientvmaj>=1) 
			sendresponseheader(so,"404 Not found");
		if(method!=kmeth_head)
			sendnotonline(so,url);
		r=0;
		goto err2;
	}

	// on vérouille le fichier local
	r=afl_lock(localfile);
	if(r){
		if(clientvmaj>=1) 
			sendresponseheader(so,"500 Internal server error");
		if(method!=kmeth_head)
			senderrorbody(so,"Internal server error",
				"<P>The file cannot be locked</P>\n");
		goto err1;
	}

	// par défaut, pas de If-Modified-Since
	*sdate=0;

	// on vérifie l'état du fichier local
	if(read_attr_str(fd,"HTTP:STATUS",sstatus,sizeof sstatus)<0) goto retrieve;
	if(strcmp(sstatus,"OK")) goto retrieve;
	
	// on vérifie la réponse HTTP du fichier local
	if(read_attr_str(fd,"HTTP:REPLY",sreply,sizeof sreply)<0) goto retrieve;
	if(1!=sscanf(sreply,"HTTP/%*d.%*d %d",&reply)) goto retrieve;
	if(reply!=200 && reply !=304 && reply !=301 && reply !=302)
		goto retrieve;

	// on vérifie si le fichier local ne doit pas être raffraîchi
	if(mode==k_online){
		if(!refreshdate) goto retrieve;	// "dumb"
		if(read_attr_str(fd,"HTTP:date",sdate,sizeof sdate)<0) *sdate=0;
		if(read_attr_str(fd,"HTTP:LOCALDATE",slocaldate,sizeof slocaldate)<0
		|| (localdate=http_stringtotime(slocaldate))<0) goto retrieve;
DBG(	printf("localdate=%d\trefreshdate=%d\trefreshdate-localdate=%d\n",
			localdate,refreshdate,refreshdate-localdate);)
		if(localdate<refreshdate) goto retrieve;
	}
	
	// tout est bien, on a peut utiliser la copie locale
	if(reply==304) reply=200;
	sendfile(so,fd,reply,method==kmeth_head);
	goto noerr;	// fini !

retrieve:	// il faut transférer ou retransférer le fichier
	if(mode!=k_online){
		if(clientvmaj>=1) 
			sendresponseheader(so,"404 Not found");
		if(method!=kmeth_head)
			sendnotonline(so,url);
		r=0;
		goto err1;
	}

	// le transfert commence
DBG(printf("Retrieving: %s\n",localfile);)
	chmod(localfile,0666);

	// on détermine l'adresse IP du serveur
DBG(printf("Looking up host: %s\n",httpurl.m_host);)
	ipaddress=ghostcache.gethostbyname(httpurl.m_host);
DBG(printf("Host %s has address %08X\n",httpurl.m_host,ipaddress);)
	if(!ipaddress){
		if(clientvmaj>=1) 
			sendresponseheader(so,"404 Not found");
		if(method!=kmeth_head)
			senderrorbody(so,"Not found",
				"<P>This host is unknown (no DNS entry).</P>\n");
		r=-1;
		goto err1;
	}
	
	// on transfère le fichier
	r=getFileViaHttp_p(
		ipaddress,httpurl.m_port,
		httpurl.m_path,
		httpurl.m_host,httpurl.m_port,
		*sdate?sdate:NULL,
		method==kmeth_head,fd,so,&reply,es);
	if(r){
DBG(	printf("problem with url <%s>: %s\n",url,es);)
		goto err1;
	}
	
DBG(printf("Retrieved: %s\n",localfile);)

	if(reply==304)		// fichier non-modifié
		sendfile(so,fd,200,method==kmeth_head);	// on transmet le cache

noerr:
	r=0;
err1:
	close(fd);
err2:
	release_sem(fd_sem);
err3:
	closesocket(so);
	
	return r;
}

int sendfile(int so, int fd, int replycode, int headonly)
{
	#define ATTRMAXSIZE 100
	DIR *dir;
	struct dirent *ent;
	char buf[4096];
	int size;
	char sdate[40];
	int cb;
	status_t r;
	
	http_timetostring_gmt(time(NULL),sdate,sizeof sdate);

	sprintf(buf,
		"HTTP/1.0 %d\r\n"
		"date: %s\r\n",
		replycode,sdate);
	size=strlen(buf);

	dir=fs_fopen_attr_dir(fd);
	if(!dir){
DBG(	printf("fs_fopen_attr_dir failed: %s\n",strerror(errno));)
		return errno;
	}

	while(ent=fs_read_attr_dir(dir)){
		if(!memcmp(ent->d_name,"HTTP:",5)
		&& islower(ent->d_name[5])
		&& strcmp(ent->d_name+5,"date")
		&& strcmp(ent->d_name+5,"connection")){	// au cas où il en resterait
			strcpy(buf+size,ent->d_name+5);
			size+=strlen(ent->d_name+5);
			buf[size++]=':';
			buf[size++]=' ';	
		}else if(!strcmp(ent->d_name,"BEOS:TYPE")){
			strcpy(buf+size,"content-type: ");
			size+=14;
		}else continue;
		if(size+ATTRMAXSIZE>sizeof buf) break;
		cb=fs_read_attr(fd,ent->d_name,B_STRING_TYPE,0,buf+size,ATTRMAXSIZE);
		if(cb<0){
DBG(		printf("fs_read_attr failed: %s\n",strerror(errno));)
			r=errno;
			fs_close_attr_dir(dir);
			return r;
		}
		size+=cb-1;	// on supprime le '\0'
		buf[size++]='\r';
		buf[size++]='\n';
	}

	fs_close_attr_dir(dir);
	
	buf[size++]='\r';
	buf[size++]='\n';
	cb=send(so,buf,size,0);
	if(cb<=0){
DBG(	printf("Client broke connection.\n");)
		return errno;
	}
DBG(printf("***\n%.*s***\n",size,buf);)
	
	if(headonly) return 0;
	
	lseek(fd,0,SEEK_SET);
	for(;;){
		cb=read(fd,buf,sizeof buf);
		if(cb<=0) break;
		
		cb=send(so,buf,cb,0);
		if(cb<=0){
DBG(		printf("Client broke connection.\n");)
			return errno;
		}
	}
	
	return 0;
}

int sendnotonline(int so, const char *url)
{
	char buf[4096];
	
	if(g_extcontrol)
		sprintf(buf,
			"<P>This document was not fetched because Charisma is presently in offline browsing mode.</P>\n"
			"<P>Would you like to go online and fetch this document?</P>\n"
			"<TABLE BORDER=\"0\" CELLSPACING=\"10\" CELLPADDING=\"0\"><TR>\n"
			"<TD><A HREF=\"%s@@@\"><IMG BORDER=0 SRC=\"http://charisma/go.gif\"></A></TD>\n"
			"<TD>Click on the arrow to go online and fetch:<BR>&lt;%s&gt;</TD>\n"
			"</TR></TABLE>\n",
			url,url);
	else
		sprintf(buf,
			"<P>This document was not fetched because Charisma is presently in offline browsing mode.</P>\n"
			"<P>To go online, set the <strong>mode</strong> menu to <strong>online</strong> in Charisma's window.</P>\n"
			"<TABLE BORDER=\"0\" CELLSPACING=\"10\" CELLPADDING=\"0\"><TR>\n"
			"<TD><A HREF=\"%s\"><IMG BORDER=0 SRC=\"http://charisma/go.gif\"></A></TD>\n"
			"<TD>When you're online, click on the arrow to fetch:<BR>&lt;%s&gt;</TD>\n"
			"</TR></TABLE>\n",
			url,url);
	
	return senderrorbody(so,"Not available in Charisma's cache",buf);
}

int sendresponseheader(int so, const char *response, 
	const char *contenttype, int contentlength)
{
	char buf[1024];
	char slength[40];
	char sdate[40];
	int cb;

	if(contentlength)
		printf(slength,"Content-Length: %d\r\n",contentlength);
	else
		strcpy(slength,"Connection: close\r\n");

	http_timetostring_gmt(time(NULL),sdate,sizeof sdate);

	sprintf(buf,
		"HTTP/1.0 %s\r\n"
		"Server: Charisma/" SPROGVERSION "\r\n"
		"Content-Type: %s\r\n"
		"%s"
		"Date: %s\r\n"
		"\r\n",
		response,contenttype,slength,sdate);
DBG(printf("%s",buf);)

	cb=send(so,buf,strlen(buf),0);
	if(cb<=0) return errno;
	
	return 0;
}

int senderrorbody(int so, const char *title, const char *text)
{
	char buf[4096];
	int cb;
	
	sprintf(buf,
		"<HTML>\n"

		"<HEAD>\n"
		"<TITLE>%s</TITLE>\n"	// title
		"</HEAD>\n"
		
		"<BODY BACKGROUND=\"http://charisma/background.gif\" BGCOLOR=\"#ffcb00\">\n"

		"<TABLE BORDER=\"0\" CELLSPACING=\"0\" CELLPADDING=\"0\">\n"
		"<TR>\n"
		"<TD WIDTH=\"120\"></TD>\n"
		"<TD>\n"

		"<P><H1 ALIGN=CENTER>%s</H1></P>\n"		// title
		"<BR>"
		"%s"	// text

		"<BR><BR><BR><HR>\n"
		"<FONT SIZE=-2>\n"
		"Charisma " SPROGVERSION ", &copy; 1998, Sylvain Demongeot.<BR>\n"
		"Visit us at <A HREF=\"http://www.wildbits.com/stamina/\">http://www.wildbits.com/stamina/</A><BR>\n"
		"Support for registered users at <A HREF=\"mailto:support@wildbits.com\">support@wildbits.com</A><BR>\n"
		"Bug reports and suggestions at <A HREF=\"mailto:report@wildbits.com\">report@wildbits.com</A><BR>\n"
		"</FONT>\n"

		"</TD></TR>\n"
		"</TABLE>\n"

		"</BODY>\n"

		"</HTML>\n",

		title,title,text);
		
DBG(printf("%s",buf);)

	cb=send(so,buf,strlen(buf),0);
	if(cb<=0) return errno;
	
	return 0;
}
