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

/*	Nom :				runnerview.cpp
	Type :				source C++
	Auteur :			Sylvain Demongeot
	Date de Création:	Mai 1998
	Projet :			Stamina
	Environnement :		BeOS
	Fonction :			courreur de "Stamina"
	Remarques : 		-
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#include <Window.h>
#include <View.h>
#include <targatobbitmap.h>
#include <runnerview.h>

#define STOPIMAGE 5

extern char runnertga[77477];
BBitmap *runnerbits=NULL;

RunnerView::RunnerView(BPoint p, const char *name, uint32 resizingMode):
	BView(BRect(p.x,p.y,p.x+RUNNERWIDTH+2*RUNNERBORDER-1,p.y+RUNNERHEIGHT+2*RUNNERBORDER-1),
	name,resizingMode,B_WILL_DRAW)
{
	if(!runnerbits)
		runnerbits=targatobbitmap((unsigned char*)runnertga,sizeof runnertga);

	SetViewColor(0xDD,0xDD,0xDD);
	
	m_animate=0;
	imagenum=STOPIMAGE;

	tid=spawn_thread(run_,"RunnerView::run",B_NORMAL_PRIORITY,this);
	if(tid<B_NO_ERROR){
		printf("spawn_thread failed file=%s line %d\n",__FILE__,__LINE__);
		exit(-1);
	}
	resume_thread(tid);
}

RunnerView::~RunnerView()
{
	status_t r;

	kill_thread(tid);
	wait_for_thread(tid,&r);
}

void RunnerView::Draw(BRect updateRect)
{
	float x1,y1,x2,y2;
	int i;

	x1=0;
	y1=0;
	x2=RUNNERWIDTH+2*RUNNERBORDER-1;
	y2=RUNNERHEIGHT+2*RUNNERBORDER-1;
	for(i=0;i<RUNNERBORDER;i++){
		SetHighColor(0,0,0);
		StrokeLine(BPoint(x2,y1),BPoint(x1,y1));
		StrokeLine(BPoint(x1,y2));
		SetHighColor(0xFF,0xFF,0xFF);
		StrokeLine(BPoint(x2,y2));
		StrokeLine(BPoint(x2,y1));
		x1++;
		y1++;
		x2--;
		y2--;
	}
	blit();
}

void RunnerView::blit()
{
	BRect srcrect(0,RUNNERHEIGHT*imagenum,RUNNERWIDTH-1,RUNNERHEIGHT*(imagenum+1)-1);
	DrawBitmap(runnerbits,srcrect,BRect(RUNNERBORDER,RUNNERBORDER,
		RUNNERBORDER+RUNNERWIDTH-1,RUNNERBORDER+RUNNERHEIGHT-1));
}

void RunnerView::setanimate(int animate)
{
	if(m_animate && !animate){
		imagenum=STOPIMAGE;
		Draw(Frame());
	}
	m_animate=animate;
}

status_t RunnerView::run()
{
	for(;;){
		snooze(50000);
		if(m_animate){
			Window()->Lock();
			imagenum++;
			if(imagenum>=12) imagenum=0;
			blit();
			Window()->Unlock();
//			if(!imagenum){printf("*");fflush(stdout);}
		}
	}
	
	return 0;
}

