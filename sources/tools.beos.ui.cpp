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

/*	Nom :				tools.beos.ui.cpp
	Type :				Source C++
	Auteur :			Sylvain Demongeot
	Date de Création:	1997
	Projet :			-
	Environnement :		BeOS
	Fonction :			Divers compléments pour "Interface Kit"
	Remarques : 		-
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#include <string.h>
#include <float.h>
#include <View.h>
#include <tools.beos.ui.h>

void BView_setup(BView *parent, BView **views, int nrows, int ncols,
	float leftmargin, float topmargin,
	float rightmargin, float bottommargin)
{
	BView_addchilds(parent,views,nrows*ncols);
	BView_tile(views,nrows,ncols);
	BView_cleanup(parent,leftmargin,topmargin,rightmargin,bottommargin);
}

void BView_cleanup(BView *parent,
	float leftmargin, float topmargin,
	float rightmargin, float bottommargin)
{
//	const float leftmargin=10;
//	const float rightmargin=10;
//	const float topmargin=20;
//	const float bottommargin=10;
	float dx,dy;
	BView *child;
	BRect frame;

	// on calcule le rectangle englobant tous les enfants
	frame=BView_childrenframe(parent);

	// on positionne les enfants par rapport au coin haut-gauche
	dx=leftmargin-frame.left;
	dy=topmargin-frame.top;
	for(child=parent->ChildAt(0);child;child=child->NextSibling())
		child->MoveBy(dx,dy);

	// on dimensionne la boite convenablement
	parent->ResizeTo(leftmargin+frame.Width()+rightmargin,
				topmargin+frame.Height()+bottommargin);
}

void BView_resize(BView *parent,
	float rightmargin, float bottommargin)
{
//	const float rightmargin=10;
//	const float bottommargin=10;
	BRect frame;

	// on calcule le rectangle englobant tous les enfants
	frame=BView_childrenframe(parent);

	// on dimensionne la boite convenablement
	parent->ResizeTo(frame.right+rightmargin,
				frame.bottom+bottommargin);
}

BRect BView_unionframe(const BView **views, int nviews)
{
	int i;
	BRect frame(FLT_MAX,FLT_MAX,FLT_MIN,FLT_MIN);

	// on calcule le rectangle englobant toutes les vues
	for(i=0;i<nviews;i++)
		frame=frame|views[i]->Frame();
	return frame;
}

BRect BView_childrenframe(const BView *parent)
{
	BView *child;
	BRect frame(FLT_MAX,FLT_MAX,FLT_MIN,FLT_MIN);

	// on calcule le rectangle englobant tous les enfants
	for(child=parent->ChildAt(0);child;child=child->NextSibling())
		frame=frame|child->Frame();
	return frame;
}

void BView_addchilds(BView *parent, BView **views, int nviews)
{
	int i;
	
	for(i=0;i<nviews;i++)
		if(views[i]){
			parent->AddChild(views[i]);
			views[i]->ResizeToPreferred();
		}
}

BRect BView_tile(BView **views, int nrows, int ncols,
	float leftmargin, float topmargin,
	float xspace, float yspace)
{
	float *maxwidth=new float[ncols];
	float *maxheight=new float[nrows];
	BRect frame;
	int i,j;
	float x,y;
	
	memset(maxwidth, 0, ncols*sizeof(float));
	memset(maxheight, 0, nrows*sizeof(float));
		
	for(i=0;i<nrows;i++)
		for(j=0;j<ncols;j++)
			if(views[ncols*i+j]){
				frame=views[ncols*i+j]->Frame();
				if(maxheight[i]<frame.Height()) maxheight[i]=frame.Height();
				if(maxwidth[j]<frame.Width()) maxwidth[j]=frame.Width();
			}
	
	y=topmargin;
	for(i=0;i<nrows;i++){
		x=leftmargin;
		for(j=0;j<ncols;j++){
			if(views[ncols*i+j]){
				views[ncols*i+j]->MoveTo(x,y);
				views[ncols*i+j]->ResizeTo(maxwidth[j],maxheight[i]);
			}
			x+=maxwidth[j]+xspace;
		}
		y+=maxheight[i]+yspace;
	}
	
	delete maxwidth;
	delete maxheight;
	
	return BRect(leftmargin,topmargin,x,y);
}

void BTextControl_setdivider(BTextControl *tc)
{
	const float labelmargin=6.0f;

	tc->SetDivider(labelmargin+tc->StringWidth(tc->Label()));
}

void BTextControl_resize(BTextControl *tc, const char *maxtext)
{
	const float labelmargin=6.0f;
	BTextView *tv;
	float w,dw;
	
	tv=tc->TextView();
	w=tc->StringWidth(maxtext);
	dw=w+labelmargin-tv->Frame().Width();
	tc->ResizeBy(dw,0);
	tv->ResizeBy(dw,0);
}

float BView_textheight(BView *view)
{
	BFont font;
	font_height fh;
	
	view->GetFont(&font);
	font.GetHeight(&fh);
	return fh.ascent+fh.descent;
}

void BMenuField_resize(BMenuField *mf)
{
	const float xmargin1=6;
	const float xmargin2=20;
	const float ymargin=12;
	float w1,w2,h;

	w1=mf->StringWidth(mf->Label());
	w2=BMenu_width(mf->Menu());
	h=BView_textheight(mf);

	mf->ResizeTo(xmargin1+w1+xmargin2+w2,ymargin+h);
	mf->SetDivider(xmargin1+w1);
}

float BMenu_width(const BMenu *m)
{
	int i,n;
	float w,wi;
	
	w=0;
	n=m->CountItems();
	for(i=0;i<n;i++){
		wi=m->StringWidth(m->ItemAt(i)->Label());
		if(w<wi)w=wi;
	}
	return w;
}

BRect rectright(const BRect *rect, float w, float h)
{
	const float margin=6;

	return BRect(
		rect->right+margin,
		rect->top,
		rect->right+margin+w,
		rect->top+h);
}

BRect rectunder(const BRect *rect, float w, float h)
{
	const float margin=6;

	return BRect(
		rect->left,
		rect->bottom+margin,
		rect->left+w,
		rect->bottom+margin+h);
}
