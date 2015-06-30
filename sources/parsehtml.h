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

/*	Nom :				parsehtml.h
	Type :				header C++
	Auteur :			Sylvain Demongeot
	Date de Création:	1997
	Projet :			Webgrabber
	Environnement :		HTML
	Fonction :			Recherche de références dans un texte HTML
	Remarques : 		-
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#ifndef _PARSEHTML_H_
#define _PARSEHTML_H_

enum{
	PHTML_BODY_BACKGROUND		=0x00000001,
	PHTML_IMG_SRC				=0x00000002,
	PHTML_OBJECT_DATA			=0x00000004,
	PHTML_APPLET_CODE			=0x00000008,
	PHTML_FRAME_SRC				=0x00000010,
	PHTML_A_HREF				=0x00000100,
	PHTML_AREA_HREF				=0x00000200,
	PHTML_LINK_HREF				=0x00000400
};

class CParseHTML{
public:
	CParseHTML(uint filter=-1) {m_filter=filter;}

	void parsehtml(char *text, int size);
	virtual void foundref(char *text, int size, int distance);
	
	uint32 m_filter;
	
private:
	void parsetag(char *text, int size);
	void checkparam(char *text, int size, const char *expectedparamtype);
	int retrieveparam(char *text, int size,
		char **paramtype, int *ptsize,
		char **paramdata, int *pdsize,
		int *retrieved);
};

#endif	// _PARSEHTML_H_
