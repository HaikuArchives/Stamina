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

#ifndef _RUNNERVIEW_H_
#define _RUNNERVIEW_H_

#define RUNNERWIDTH 46
#define RUNNERHEIGHT 50
#define RUNNERBORDER 2

class RunnerView: public BView{
public:
	RunnerView(BPoint p, const char *name, uint32 resizingMode);
	~RunnerView();
	
	virtual void Draw(BRect updateRect);
	void blit();
	void setanimate(int animate);
	status_t run();
	
	static status_t run_(void *this_)
		{return ((RunnerView*)this_)->run();}
	
	int m_animate;
	int imagenum;
	thread_id tid;
};

#endif // _RUNNERVIEW_H_
