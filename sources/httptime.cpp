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

/*	Nom :				httptime.cpp
	Type :				Source C++
	Auteur :			Sylvain Demongeot
	Date de Création:	Mars 1998
	Projet :			-
	Environnement :		Posix, HTTP
	Fonction :			convertit une chaîne en date et vice-versa (format de date HTTP)
	Remarques : 		pas complètement testé
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <strcmp_ic.h>
#include <httptime.h>

time_t http_stringtotime(const char *sdate)
{
	struct tm tm;
	int i;
	char smonth[4];
	const char *monthes[]={"jan","feb","mar","apr",
							"may","jun","jul","aug",
							"sep","oct","nov","dec"};

	memset(&tm,0,sizeof tm);
	
	if(6!=sscanf(sdate,"%*[^,], %d %3s %d %d:%d:%d",
		&tm.tm_mday,smonth,&tm.tm_year,
		&tm.tm_hour,&tm.tm_min,&tm.tm_sec)) return -1;
		
	tm.tm_year-=1900;
		
	for(i=0;i<12;i++)
		if(!strcmp_ic(smonth,monthes[i])) break;
	if(i>=12) return -1;
	tm.tm_mon=i;
		
	return mktime(&tm);
}

time_t http_stringtotime_gmt(const char *sdate)
{
	struct tm tm;

	memset(&tm,0,sizeof tm);
	tm.tm_sec=0;
	tm.tm_min=0;
	tm.tm_hour=0;
	tm.tm_mday=1;
	tm.tm_mon=0;
	tm.tm_year=70;
		
	return http_stringtotime(sdate)-mktime(&tm);
}

void http_timetostring(time_t date, char *sdate, int sdatesize)
{
	struct tm *tm;
	
	tm=localtime(&date);
	strftime(sdate,sdatesize,"%a, %d %b %Y %H:%M:%S",tm);
}

void http_timetostring_gmt(time_t date, char *sdate, int sdatesize)
{
	struct tm *tm;
	
	tm=gmtime(&date);
	strftime(sdate,sdatesize,"%a, %d %b %Y %H:%M:%S GMT",tm);
}
