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

/*	Nom :				grab.h
	Type :				Header C++
	Auteur :			Sylvain Demongeot
	Date de Création:	Février 1998
	Projet :			Webgrabber
	Environnement :		BeOS, HTTP
	Fonction :			
	Remarques : 		-
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#ifndef _GRAB_H_
#define _GRAB_H_

#include <hostcache.h>

enum{
	GRAB_SAMESERVER		=0x00000001,
	GRAB_DOWNWARDS		=0x00000002,
	GRAB_NON_HTML_HREFS	=0x00000004
};

class GrabParams{
public:
	int size;
	int depth;
	char url[300];
	GrabParams(){}
	GrabParams(const char *url_, int depth_);
};

class Grabber{
public:
	Grabber(int filter, int flags, CHttpURL &baseurl,
		int sizelimit, long smartrefresh_date, int maxthreads,
		int32 *totalvolume, int32 *filesloaded,
		int32 *filesloading, int32 *fileswaiting,
		BMessenger *reporter,
		BInvoker *gameover);
	~Grabber();	
	
	int addurl(const GrabParams *gp);

private:
	friend class GrabParser;

	void report(const char *url, const char* format, ...);
	status_t serve();
	status_t grab(const GrabParams *gp);
	thread_id grab_async(const GrabParams *gp);

	static status_t serve_(void *this_);
	static status_t grab_(void *this_);
		
	int m_filter;
	int m_flags;
	CHttpURL m_baseurl;
	int m_lastslash;	// position du dernier slash dans m_baseurl.m_path
	int m_sizelimit;
	long m_smartrefresh_date;
	int32 *totalvolume;
	int32 *filesloaded;
	int32 *filesloading;
	int32 *fileswaiting;
	BMessenger *reporter;
	BInvoker *gameover;
	Hostcache hostcache;
	
	GrabParams *m_grabbedurl;
	int m_gubufsize;
	int m_ngrabbedurl;
	BLocker m_gulock;

	thread_id serverthread;
	sem_id url_sem;
	sem_id thread_sem;
	int32 balance;	
};

#endif	// _GRAB_H_
