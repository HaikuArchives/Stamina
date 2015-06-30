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

/*	Nom :				strcmp_ic.cpp
	Type :				source C
	Auteur :			Sylvain Demongeot
	Date de Création:	1997
	Projet :			-
	Environnement :		-
	Fonction :			comparaison de deux chaînes min/maj ignorées
	Remarques : 		strcmp_ic jamais testé
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#include <ctype.h>
#include <strcmp_ic.h>

int strcmp_ic(const char *s1, const char *s2)
{
	char c1,c2;
	
	for(;;){
		c1=toupper(*s1);
		c2=toupper(*s2);
		if(c1<c2)return -1;
		if(c1>c2)return 1;
		if(!c1)return 0;
		s1++,s2++;
	}
}

int strncmp_ic(const char *s1, const char *s2, int n)
{
	char c1,c2;
	int i;
	
	for(i=0;i<n;i++){
		c1=toupper(*s1);
		c2=toupper(*s2);
		if(c1<c2)return -1;
		if(c1>c2)return 1;
		if(!c1)return 0;
		s1++,s2++;
	}
	return 0;
}


