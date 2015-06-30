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

/*	Nom :				grab.cpp
	Type :				Source C++
	Auteur :			Sylvain Demongeot
	Date de Création:	Février 1998
	Projet :			Stamina
	Environnement :		BeOS, HTTP
	Fonction :			Coeur du système de capture de site
	Remarques : 		-
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <strcmp_ic.h>
#include <getfileviahttp.h>
#include <parsehtml.h>
#include <httpurl.h>
#include <afl_client.h>
#include <hostcache.h>
#include <messages.h>
#include <grab.h>
#include <create_directory_.h>
#include <fatal.h>
#include <httptime.h>
#include <tools.beos.h>
#include <printf_patch.h>

#if 0
	#define DBG(A) A
#else
	#define DBG(A)
#endif

#define GUQUANTUM 8192

char g_webdir[B_PATH_NAME_LENGTH]="/boot/home/theWWW/";

static sem_id fd_sem=0;

status_t create_directory_(const char *path, mode_t mode);
unsigned long gethostbyname_(const char *hostname);
int ishtml(const char *s);

class GrabParser: public CParseHTML{
public:	
	GrabParser(Grabber& grabber, CHttpURL& url, int depth):
		m_grabber(grabber),m_url(url){m_depth=depth;}
		
	void foundref(char *text, int size, int distance);
	
	Grabber& m_grabber;
	CHttpURL& m_url;
	int m_depth;
};

GrabParams::GrabParams(const char *url_, int depth_)
{
	strcpy(url,url_);
	depth=depth_;
	size=sizeof size+sizeof depth+strlen(url_)+1;
	if(size&3) size=(size&~3)+4;
}

Grabber::Grabber(int filter, int flags, CHttpURL &baseurl,
	int sizelimit, long smartrefresh_date, int maxthreads, 
	int32 *totalvolume_, int32 *filesloaded_,
	int32 *filesloading_, int32 *fileswaiting_,
	BMessenger *reporter_,
	BInvoker *gameover_):
	
	m_gulock("Grabber::m_gulock"),
	hostcache(100)	// on peut faire meilleur choix...
{
	int i;
	
	// static
	if(!fd_sem) fd_sem=create_sem(128,"Grabber::fd_sem");
	if(fd_sem<B_NO_ERROR)
		fatal(__FILE__,__LINE__,"create_sem failed");
	// end static

	m_filter=filter;
	m_flags=flags;
	m_baseurl=baseurl;
	m_sizelimit=sizelimit;
	m_smartrefresh_date=smartrefresh_date;
	totalvolume=totalvolume_;
	filesloaded=filesloaded_;
	filesloading=filesloading_;
	fileswaiting=fileswaiting_;
	gameover=gameover_;
	reporter=reporter_;
	
	m_lastslash=0;
	for(i=0;m_baseurl.m_path[i];i++)
		if(m_baseurl.m_path[i]=='/')
			m_lastslash=i;
	
	balance=0;

	m_gubufsize=GUQUANTUM;
	m_grabbedurl=(GrabParams*)malloc(m_gubufsize);
	if(!m_grabbedurl)
		fatal(__FILE__,__LINE__,"malloc failed");
	m_ngrabbedurl=0;
	
	url_sem=create_sem(0,"Grabber::url_sem");
	if(url_sem<B_NO_ERROR)
		fatal(__FILE__,__LINE__,"create_sem failed");
	
	thread_sem=create_sem(maxthreads,"Grabber::thread_sem");
	if(thread_sem<B_NO_ERROR)
		fatal(__FILE__,__LINE__,"create_sem failed");

	serverthread=spawn_thread(serve_,"Grabber::serve",
		B_NORMAL_PRIORITY,this);
	if(serverthread<B_NO_ERROR)
		fatal(__FILE__,__LINE__,"spawn_thread failed");
	resume_thread(serverthread);
}

Grabber::~Grabber()
{
	thread_id tid;
	status_t r;
	int n;
	char name[32];
	
//	printf("Killing serverthread (tid=%d)...\n",serverthread);
	kill_thread(serverthread);
	wait_for_thread(serverthread,&r);
//	printf("Killed serverthread (tid=%d).\n",serverthread);
	
	sprintf(name,"Grabber@%08X::grab",this);
	n=0;
	for(;;){
		tid=find_thread(name);
		if(tid<0) break;
//		printf("Killing %s (tid=%d)...\n",name,tid);
		kill_thread(tid);
		wait_for_thread(tid,&r);
//		printf("Killed %s (tid=%d).\n",name,tid);
		n++;
	}
//	printf("Killed %d threads\n",n);
	
	delete_sem(url_sem);
	delete_sem(thread_sem);
	free(m_grabbedurl);
}

int Grabber::addurl(const GrabParams *gp)
{
	int i;
	GrabParams *gpt;
	int offset;
	
	if(!serverthread)return -1;
	
	m_gulock.Lock();

	gpt=m_grabbedurl;
	for(i=0;i<m_ngrabbedurl;i++){
		if(!strcmp(gpt->url,gp->url)
		&& gpt->depth>=gp->depth){
			m_gulock.Unlock();
			return 1;
		}
		gpt=(GrabParams*)(((char*)gpt)+gpt->size);
	}

	offset=(char*)gpt-(char*)m_grabbedurl;
	if(offset+gp->size>m_gubufsize){
		m_gubufsize+=GUQUANTUM;
		m_grabbedurl=(GrabParams*)realloc(m_grabbedurl,m_gubufsize);
		if(!m_grabbedurl)
			fatal(__FILE__,__LINE__,"realloc failed");
		gpt=(GrabParams*)((char*)m_grabbedurl+offset);
	}
	
	memcpy(gpt,gp,gp->size);
	m_ngrabbedurl++;
	m_gulock.Unlock();
//	printf("registered: %s (%d)\n",gp->url,gp->depth);
	if(fileswaiting) atomic_add(fileswaiting,1);
	atomic_add(&balance,1);

	release_sem(url_sem);
	
	return 0;
}

void Grabber::report(const char *url, const char* format, ...)
{
	char buf[300];
	va_list args;
	BMessage msg(kMsg_Report);
	
	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);
	msg.AddString("line1",url);
	msg.AddString("line2",buf);
	reporter->SendMessage(&msg);
}

status_t Grabber::serve()
{
	int offset;
	GrabParams *gp;
	status_t r;
	
	offset=0;
	for(;;){
		r=acquire_sem(url_sem);
		if(r){
			report(gp->url,"Semaphore error: %s",strerror(r));
			return r;
		}
		r=acquire_sem(thread_sem);
		if(r){
			report(gp->url,"Semaphore error: %s",strerror(r));
			return r;
		}
		m_gulock.Lock();
		gp=(GrabParams*)((char*)m_grabbedurl+offset);
		grab_async(gp);
		offset+=gp->size;
		m_gulock.Unlock();
		if(fileswaiting) atomic_add(fileswaiting,-1);
	}
	return 0;
}

status_t Grabber::serve_(void *this_)
{
	return ((Grabber*)this_)->serve();
}

status_t Grabber::grab(const GrabParams *gp)
{
	CHttpURL httpurl;
	int size;
	char *text;
	ulong ipaddress;
	int reply;
	char sreply[100];
	char es[80];
	status_t r;
	char localfile[B_PATH_NAME_LENGTH],localdir[B_PATH_NAME_LENGTH];
	int i,j;
	int fd;
	char mimetype[80];
	char sdate[80],slocaldate[80];
	time_t localdate;
	char sstatus[20];
	char snewurl[B_PATH_NAME_LENGTH];
	CHttpURL newurl;
	
	// on acquière fd_sem pour s'assurer qu'il reste au moins 1 fd
	r=acquire_sem(fd_sem);
	if(r){
		report(gp->url,"Semaphore error: %s",strerror(r));
		goto err3;
	}

	// on détermine le chemin du fichier local
	httpurl.set(gp->url);
	httpurl.getuniquefile(g_webdir,localfile);

	// on crée le parent du fichier local
	for(i=0;localfile[i];i++)
		if(localfile[i]=='/') j=i;
	memcpy(localdir,localfile,j);
	localdir[j]=0;
	r=create_directory_(localdir,0777);
	if(r){
		report(gp->url,"Cannot create directory %s: %s",localdir,strerror(r));
		goto err2;
	}

	// on ouvre le fichier local
label_open:
	fd=open(localfile,B_READ_WRITE|B_CREATE_FILE);
	if(fd<0){
		if(errno==B_IS_A_DIRECTORY){
			strcat(localfile,":FILE");
			goto label_open;
		}else{
			report(gp->url,"Cannot open file %s: %s",localfile,strerror(errno));
			r=errno;
			goto err2;
		}
	}

	// on vérouille le fichier local
	r=afl_lock(localfile);
	if(r){
		report(gp->url,"Cannot lock file %s: %s",localfile,strerror(r));
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
	if(!m_smartrefresh_date) goto retrieve;	// "dumb"
	if(read_attr_str(fd,"HTTP:date",sdate,sizeof sdate)<0) *sdate=0;
	if(read_attr_str(fd,"HTTP:LOCALDATE",slocaldate,sizeof slocaldate)<0
	|| (localdate=http_stringtotime(slocaldate))<0) goto retrieve;
DBG(printf("localdate=%d\tm_smartrefresh_date=%d\tm_smartrefresh_date-localdate=%d\n",
		localdate,m_smartrefresh_date,m_smartrefresh_date-localdate);)
	if(localdate<m_smartrefresh_date) goto retrieve;
	
	// on détermine le type du fichier local
	if(read_attr_str(fd,"BEOS:TYPE",mimetype,sizeof mimetype)<0) goto retrieve;

	// tout est bien, on a peut utiliser la copie locale
	lseek(fd,0,SEEK_END);
	goto parse;

retrieve:	// il faut transférer ou retransférer le fichier

#if 0
	// tests only...
	report(gp->url,"NOT Saving: %s",localfile);
	r=-1;
	goto err1;
#endif

	// le transfert commence
DBG(printf("Retrieving: %s\n",localfile);)
	if(filesloading) atomic_add(filesloading,1);
	chmod(localfile,0666);

	// on détermine l'adresse IP du serveur
DBG(printf("Looking up host: %s\n",httpurl.m_host);)
	ipaddress=hostcache.gethostbyname(httpurl.m_host);
DBG(printf("Host %s has address %08X\n",httpurl.m_host,ipaddress);)
	if(!ipaddress){
		report(gp->url,"Unknown host: %s (no DNS entry)",httpurl.m_host);
		if(filesloading) atomic_add(filesloading,-1);
		r=-1;
		goto err1;
	}
	
	// on transfère le fichier
	r=getFileViaHttp(
		ipaddress,httpurl.m_port,
		httpurl.m_path,
		httpurl.m_host,httpurl.m_port,
		*sdate?sdate:NULL,
		m_sizelimit,fd,&reply,es,mimetype,totalvolume);
	if(r){
		report(gp->url,"HTTP: %s",es);
		if(filesloading) atomic_add(filesloading,-1);
		goto err1;
	}
	
	// on vérifie la réponse HTTP
	if(reply!=200 && reply !=304 && reply !=301 && reply !=302){
		report(gp->url,"Server returned code: %d",reply);
		if(filesloading) atomic_add(filesloading,-1);
		r=-1;
		goto err1;
	}
	
DBG(printf("Retrieved: %s\n",localfile);)
	if(filesloading) atomic_add(filesloading,-1);
	if(filesloaded) atomic_add(filesloaded,1);
	
parse:
DBG(printf("type=%s\n",mimetype);)
	if(reply==301 || reply==302){
		// redirection
		if(read_attr_str(fd,"HTTP:location",snewurl,sizeof snewurl)>=0){
			newurl.set(snewurl);
			newurl.getsimple(snewurl);
			addurl(&GrabParams(snewurl,gp->depth));
			report(gp->url,"Warning: data moved to %s",snewurl);
		}else
			report(gp->url,"Data moved to unknown location.");
		
	}else if(!strcmp_ic(mimetype,"text/html")||ishtml(gp->url)){
		size=lseek(fd,0,SEEK_CUR);
		if(size>0){
			// on charge le fichier en mémoire
			text=(char*)malloc(size);
			if(!text){
				report(gp->url,"Cannot allocate buffer");
				r=-1;
				goto err1;
			}
			lseek(fd,0,SEEK_SET);
			if(read(fd,text,size)<0){
				report(gp->url,"Could not read file %s: %s",localfile,strerror(errno));
				r=errno;
				goto err1;
			}
			
			// on le parcourt
			GrabParser parser(*this,httpurl,gp->depth);
			parser.parsehtml(text,size);
	
			// on libère le tampon
			free(text);
		}
	}
	
	r=0;
err1:	
	close(fd);
DBG(printf("thru with fd=%d\n",fd);)
err2:
	release_sem(fd_sem);
err3:
	release_sem(thread_sem);
	if(atomic_add(&balance,-1)<=1 && gameover)
		gameover->Invoke();
	
	return r;
}

status_t Grabber::grab_(void *this_)
{
	GrabParams gp;
	thread_id tid;

	receive_data(&tid,&gp,sizeof gp);
	return ((Grabber*)this_)->grab(&gp);
}

thread_id Grabber::grab_async(const GrabParams *gp)
{
	thread_id sonthread;
	char name[32];

	sprintf(name,"Grabber@%08X::grab",this);
	sonthread=spawn_thread(grab_, name, B_NORMAL_PRIORITY,this);
	if(sonthread<B_NO_ERROR) return NULL;
	
	send_data(sonthread,0,gp,gp->size);
	
	resume_thread(sonthread);
	
	return sonthread;
}

void GrabParser::foundref(char *text, int size, int distance)
{
	char urls[B_PATH_NAME_LENGTH];
	CHttpURL newurl;
	int flags;
	
	if(m_depth-distance<0) return;
	
	memcpy(urls,text,size);
	urls[size]=0;	
	newurl.setrel(m_url,urls);

	flags=m_grabber.m_flags;

	if(flags&GRAB_SAMESERVER
	&& (strcmp(newurl.m_host,m_grabber.m_baseurl.m_host)
		||newurl.m_port!=m_grabber.m_baseurl.m_port))
		return;

	if(flags&GRAB_DOWNWARDS
	&& strncmp(newurl.m_path,m_grabber.m_baseurl.m_path,
				m_grabber.m_lastslash+1))
		return;

	if(distance
	&& !(flags&GRAB_NON_HTML_HREFS)
	&& !ishtml(newurl.m_path))
		return;
	
	newurl.getsimple(urls);
//	printf("FOUND URL: %s\n",urls);

	m_grabber.addurl(&GrabParams(urls,m_depth-distance));
}

unsigned long gethostbyname_(const char *hostname)
{
	unsigned int ipnumbers[4];
	char ctrash;
	struct hostent* host;
	unsigned long ipaddress;
		
	if(4==sscanf(hostname,"%d.%d.%d.%d%c",
		ipnumbers,ipnumbers+1,ipnumbers+2,ipnumbers+3,&ctrash)
	&& ipnumbers[0]<0x100
	&& ipnumbers[1]<0x100
	&& ipnumbers[2]<0x100
	&& ipnumbers[3]<0x100)
		return ipnumbers[0]<<24
			| ipnumbers[1]<<16
			| ipnumbers[2]<<8
			| ipnumbers[3];
	
	host=gethostbyname(hostname);
	if(host){
		memcpy(&ipaddress,host->h_addr_list[0],4);
		return ipaddress;
	}else return 0;
}

int ishtml(const char *s)
{
	int l;
	
	l=strlen(s);
	return 
		(l>=1 && s[l-1]=='/')
		||(l>=5 && !strcmp_ic(s+l-5,".html"))
		||(l>=4 && !strcmp_ic(s+l-4,".htm"));
}
