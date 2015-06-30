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

/*	Nom :				parsehtml.cpp
	Type :				Source c++
	Auteur :			Sylvain Demongeot
	Date de Création:	1997
	Projet :			Stamina
	Environnement :		HTML
	Fonction :			Recherche de références dans un texte HTML
	Remarques : 		-
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <strcmp_ic.h>
#include <parsehtml.h>
#include <printf_patch.h>

#if 0
	#define DBG(A) A
#else
	#define DBG(A)
#endif

void CParseHTML::parsehtml(char *text, int size)
{
	int i;
	int intag;
	int tagstart;
	
	i=0;
	intag=0;
	while(i<size)
		if(text[i++]=='<'){
			tagstart=i;
			while(i<size)
				if(text[i++]=='>'){
					parsetag(text+tagstart,i-tagstart-1);
					break;
				}
		}
}

void CParseHTML::parsetag(char *text, int size)
{
	char tagtype[20];
	int i,j;
	char c;

DBG(printf("TAG: %.*s\n",size,text);	)

	i=j=0;	
	while(i<size){
		c=text[i++];
		if(isspace(c)) break;
		tagtype[j++]=toupper(c);
		if(j>=sizeof tagtype) return;
	}
	tagtype[j]=0;
	text+=i;
	size-=i;
	
DBG(printf("\tTAG TYPE: %s\n",tagtype);	)

	if(m_filter&PHTML_BODY_BACKGROUND && !strcmp(tagtype,"BODY"))
		checkparam(text,size,"BACKGROUND");
	else if(m_filter&PHTML_IMG_SRC && !strcmp(tagtype,"IMG"))
		checkparam(text,size,"SRC");
	else if(m_filter&PHTML_OBJECT_DATA && !strcmp(tagtype,"OBJECT"))
		checkparam(text,size,"DATA");
	else if(m_filter&PHTML_APPLET_CODE && !strcmp(tagtype,"APPLET"))
		checkparam(text,size,"CODE");
	else if(m_filter&PHTML_FRAME_SRC && !strcmp(tagtype,"FRAME"))
		checkparam(text,size,"SRC");
	else if(m_filter&PHTML_A_HREF && !strcmp(tagtype,"A"))
		checkparam(text,size,"HREF");
	else if(m_filter&PHTML_AREA_HREF && !strcmp(tagtype,"AREA"))
		checkparam(text,size,"HREF");
	else if(m_filter&PHTML_LINK_HREF && !strcmp(tagtype,"LINK"))
		checkparam(text,size,"HREF");
}

void CParseHTML::checkparam(char *text, int size, const char *expectedparamtype)
{
	int i;
	char *paramtype,*paramdata;
	int ptsize,pdsize;
	int retrieved;
	int r;
	
	i=0;
	while(i<size){
		r=retrieveparam(text+i,size-i,
			&paramtype,&ptsize,
			&paramdata,&pdsize,&retrieved);
		if(r<0)return;
		if(retrieved==2 && !strncmp_ic(paramtype,expectedparamtype,ptsize)){
			foundref(paramdata,pdsize,strcmp(expectedparamtype,"HREF")?0:1);
			break;
		}
		i+=r;
	}
	return;
}

// text est de la forme "TYPE=data"
// retourne:
//  >0 le nombre de caractères parcourus
//  <0 erreur
// en retour, *retrieved vaut:
//  0 pas de paramètre
//  1 type seulement
//  2 type et data
int CParseHTML::retrieveparam(char *text, int size,
		char **paramtype, int *ptsize,
		char **paramdata, int *pdsize,
		int *retrieved)
{
	int i,j;

	i=0;
	*retrieved=0;

	// on passe les espaces
	for(;;){
		if(i>=size) return -1;
		if(!isspace(text[i])) break;
		i++;
	}
	
	// le type
	*paramtype=text+i;
	j=0;
	for(;;){
		if(i>=size) return -1;
		if(!isalpha(text[i])) break;
		j++;
		i++;
	}
	if(!j)return -1;
	*ptsize=j;
	*retrieved=1;
	
	// on passe les espaces
	for(;;){
		if(i>=size) return -1;
		if(!isspace(text[i])) break;
		i++;
	}
	
	// on vérifie la présence de '='
	// sinon, on retourne le nombre de caractères parcourus
	if(text[i]!='=') return i;
	i++;

	// on passe les espaces
	for(;;){
		if(i>=size) return -1;
		if(!isspace(text[i])) break;
		i++;
	}
	
	// les données
	j=0;
	if(text[i]=='"'){
		i++;
		*paramdata=text+i;
		for(;;){
			if(i>=size) break;
			if(text[i]=='"') break;
			j++;
			i++;
		}
		i++;
	}else
		*paramdata=text+i;
		for(;;){
			if(i>=size) break;
			if(isspace(text[i])) break;
			j++;
			i++;
		}
	*pdsize=j;
	*retrieved=2;
	
	return i;
}

void CParseHTML::foundref(char *text, int size, int distance)
{
	printf("FOUND REF: %.*s (distance=%d)\n",size,text,distance);
}
