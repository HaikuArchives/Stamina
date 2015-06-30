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

#include <string.h>
#include <sdtab.h>

#define SDTABHEIGHT 20
#define EXTAB 3
#define MARGIN 14

const rgb_color kWhite = {255,255,255,255}; 
const rgb_color kGray = {219,219,219,255}; 
const rgb_color kMedGray = {180,180,180,255}; 
const rgb_color kDarkGray = {100,100,100,255}; 
const rgb_color kBlackColor = {0,0,0,255}; 

SDTab::SDTab(BView* v)
{
	SetView(v);
	SetLabel("");
}

void SDTab::SetLabel(const char* label)
{
	strcpy(_label,label);
}

void SDTab::Select(BView* owner)
{
	owner->AddChild(View());
}

void SDTab::Deselect()
{
	View()->RemoveSelf();
}

void SDTab::SetView(BView* v)
{
	fView=v;
}

BView* SDTab::View() const
{
	return fView;
}

void SDTab::DrawLabel(BView* owner, BRect frame)
{
	BPoint p;
	float w;

	w=owner->StringWidth(_label);
	p.x=frame.left+(frame.Width()-w)/2;
	p.y=frame.top+frame.Height()*0.7;
	owner->DrawString(_label,p);
}

void SDTab::DrawTab(BView *owner, BRect frame, tab_position position, bool full)
{ 
   rgb_color      hi; 
   rgb_color      lo; 

   // Save the original colors 

   hi = owner->HighColor(); 
   lo = owner->LowColor(); 

   // Draw the label by calling DrawLabel() 

   owner->SetHighColor(kBlackColor); 
   owner->SetLowColor(kGray); 
   DrawLabel(owner, frame); 

   // Start a line array to draw the tab -- 
   // this is faster than drawing the lines 
   // one by one. 

   owner->BeginLineArray(7); 

   // Do the bottom left corner, visible 
   // only on the frontmost tab. 

   if (position != B_TAB_ANY) { 
      owner->AddLine(BPoint(frame.left, frame.bottom), 
                BPoint(frame.left+3, frame.bottom-3), kWhite); 
   } 

   // Left wall -- always drawn 

   owner->AddLine(BPoint(frame.left+4, frame.bottom-4), 
             BPoint(frame.left+4, frame.top), kWhite); 

   // Top -- always drawn 

   owner->AddLine(BPoint(frame.left+5, frame.top), 
             BPoint(frame.right-5, frame.top), kWhite); 

   // Right wall -- always drawn.  Has a nice bevel. 

   owner->AddLine(BPoint(frame.right-4, frame.top), 
             BPoint(frame.right-4, frame.bottom-4), kDarkGray); 
   owner->AddLine(BPoint(frame.right-5, frame.top), 
             BPoint(frame.right-5, frame.bottom-4), kMedGray); 

   // Bottom-right corner, only visible if the tab 
   // is either frontmost or the rightmost tab. 

   if (full) { 
      owner->AddLine(BPoint(frame.right-3, frame.bottom-3), 
                BPoint(frame.right, frame.bottom), kDarkGray); 
      owner->AddLine(BPoint(frame.right-4, frame.bottom-3), 
                BPoint(frame.right-1, frame.bottom), kMedGray); 
   } 

   owner->EndLineArray(); 

   owner->SetHighColor(hi); 
   owner->SetLowColor(lo); 
}

SDTabView::SDTabView(BRect frame, const char *name,
	button_width width, uint32 resizingMode, uint32 flags):
		BView(frame,name,resizingMode,flags)
{
	BRect rect;
	
	fTabList=new BList;
	fSelection=0;
	fTabWidth=0;
	fTabHeight=SDTABHEIGHT;
	SetViewColor(kGray);
	
	rect=frame;
	rect.OffsetTo(B_ORIGIN);
	rect.top=fTabHeight;
	rect.InsetBy(8,6);
	fViewContainer=new BView(rect,"",B_FOLLOW_ALL,B_WILL_DRAW);
	fViewContainer->SetViewColor(kGray);
	AddChild(fViewContainer);
}

void SDTabView::MouseDown(BPoint p)
{
	int i;

	if(p.y<fTabHeight){
		i=(p.x-MARGIN)/fTabWidth;
		if(i>=0 && i<fTabList->CountItems())
			Select(i);
	}
}

void SDTabView::Select(int32 i)
{
	if(i==fSelection) return;

	TabAt(fSelection)->Deselect();
	TabAt(i)->Select(fViewContainer);
	fSelection=i;

	Invalidate();
}

void SDTabView::Draw(BRect rect)
{
	DrawBox(DrawTabs());
}

BRect SDTabView::DrawTabs()
{
	int i,n;
	BRect r,rr;
	tab_position pos;
	bool full;
	
	n=fTabList->CountItems();
	for(i=0;i<n;i++){
		r=TabFrame(i);
		pos=i==0?B_TAB_FIRST:i==fSelection?B_TAB_FRONT:B_TAB_ANY;
		full=i!=fSelection-1;
		TabAt(i)->DrawTab(this,r,pos,full);
		if(i==fSelection) rr=r;
	}
	return rr;
}

void SDTabView::DrawBox(BRect selTabRect)
{
	BRect frame;
	float w,h;

	frame=Frame();
	w=frame.Width();
	h=frame.Height();
	
	BeginLineArray(7); 
	AddLine(BPoint(selTabRect.right,selTabRect.bottom), 
		BPoint(w-2, selTabRect.bottom), kWhite);
	AddLine(BPoint(w-1,selTabRect.bottom), 
		BPoint(w-1, h-1), kMedGray);
	AddLine(BPoint(w,selTabRect.bottom), 
		BPoint(w, h), kDarkGray);
	AddLine(BPoint(w-1, h-1), 
		BPoint(frame.left+1, h-1), kMedGray);
	AddLine(BPoint(w, h), 
		BPoint(frame.left+1, h), kDarkGray);
	AddLine(BPoint(frame.left, h), 
		BPoint(frame.left, selTabRect.bottom), kWhite);
	AddLine(BPoint(frame.left, selTabRect.bottom), 
		BPoint(selTabRect.left, selTabRect.bottom), kWhite);
	EndLineArray(); 
}

BRect SDTabView::TabFrame(int32 i) const
{
	return BRect(MARGIN+i*fTabWidth-EXTAB,0,
		MARGIN+(i+1)*fTabWidth+EXTAB,fTabHeight);
}

void SDTabView::AddTab(BView* v, SDTab* tab)
{
	tab->SetView(v);
	fTabList->AddItem(tab);
	if(fTabList->CountItems()==1) tab->Select(fViewContainer);
	
	fTabWidth=(Frame().Width()-2*MARGIN)
		/fTabList->CountItems();
	Invalidate();
}

SDTab* SDTabView::TabAt(int32 i) const
{
	return (SDTab*)(fTabList->ItemAt(i));
}
