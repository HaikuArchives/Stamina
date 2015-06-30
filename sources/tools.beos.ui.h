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

/*	Nom :				tools.beos.ui.h
	Type :				Header C++
	Auteur :			Sylvain Demongeot
	Date de Création:	1997
	Projet :			-
	Environnement :		BeOS
	Fonction :			Divers compléments pour "Interface Kit"
	Remarques : 		-
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#ifndef _TOOLS_BEOS_UI_H_
#define _TOOLS_BEOS_UI_H_

void BMenuField_resize(BMenuField *mf);		// ajuste la taille
float BMenu_width(const BMenu *m);			// calcule la largeur

void BTextControl_resize(BTextControl *tc, const char *maxtext);	// ajuste la taille
void BTextControl_setdivider(BTextControl *tc);						// ajuste la séparation

void BView_setup(BView *parent, BView **views, int nrows, int ncols,
	float leftmargin=10.0f, float topmargin=10.0f,				// topmargin=20.0f pour une BBox
	float rightmargin=10.0f, float bottommargin=10.0f);			// ajoute et dispose des enfants
void BView_addchilds(BView *parent, BView **views, int nviews);	// ajoute des enfants et ajuste leurs tailles
BRect BView_tile(BView **views, int nrows, int ncols,
	float leftmargin=0.0f, float topmargin=0.0f,
	float xspace=4.0f, float yspace=4.0f);				// dispose les vues
void BView_cleanup(BView *parent,
	float leftmargin=10.0f, float topmargin=10.0f,				// topmargin=20.0f pour une BBox
	float rightmargin=10.0f, float bottommargin=10.0f);			// ajuste les marges
void BView_resize(BView *parent,
	float rightmargin=10.0f, float bottommargin=10.0f);			// ajuste la taille
BRect BView_childrenframe(const BView *parent);					// calcule le rectangle englobant
BRect BView_unionframe(const BView **views, int nviews);		// calcule le rectangle englobant
float BView_textheight(BView *view);							// calcule la hauteur du texte

BRect rectright(const BRect *rect, float w=32.0, float h=32.0);		// retourne un rectangle placé à droite
BRect rectunder(const BRect *rect, float w=32.0, float h=32.0);		// retourne un rectangle placé en dessous
inline BRect rectright(const BView *view, float w=32.0, float h=32.0) {return rectright(&view->Frame(),w,h);}
inline BRect rectunder(const BView *view, float w=32.0, float h=32.0) {return rectunder(&view->Frame(),w,h);}

#endif	// _TOOLS_BEOS_UI_H_
