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

#ifndef _SDTAB_H
#define _SDTAB_H

#include <BeBuild.h>
#include <Message.h>
#include <View.h>

/*----------------------------------------------------------------*/
/*----- tab definitions ------------------------------------------*/

/*
enum tab_position {
	B_TAB_FIRST = 999,
	B_TAB_FRONT,
	B_TAB_ANY
};
*/

/*----------------------------------------------------------------*/
/*----- SDTab class -----------------------------------------------*/


class SDTab : public BArchivable {
public:
						SDTab(BView* v=NULL);
//virtual					~SDTab();

						SDTab(BMessage* data);
//static 	BArchivable*	Instantiate(BMessage* data);
//virtual	status_t		Archive(BMessage* data, bool deep = true) const;
//virtual status_t		Perform(uint32 d, void *arg);

		const char* 	Label() const;
virtual	void			SetLabel(const char* label);

		bool			IsSelected() const;
virtual	void			Select(BView*);
virtual	void			Deselect();

//virtual	void			SetEnabled(bool on);
		bool			IsEnabled() const;
		
		void			MakeFocus(bool infocus=true);
		bool			IsFocus() const;
		
virtual void			SetView(BView* v);
		BView*			View() const;

//virtual void			DrawFocusMark(BView*, BRect);
virtual void 			DrawLabel(BView*, BRect);
virtual void 			DrawTab(BView*, BRect, tab_position, bool full=true);

/*----- Private or reserved -----------------------------------------*/
private:
/*virtual	void			_ReservedTab1();
virtual	void			_ReservedTab2();
virtual	void			_ReservedTab3();
virtual	void			_ReservedTab4();
virtual	void			_ReservedTab5();
virtual	void			_ReservedTab6();
virtual	void			_ReservedTab7();
virtual	void			_ReservedTab8();
virtual	void			_ReservedTab9();
virtual	void			_ReservedTab10();
virtual	void			_ReservedTab11();
virtual	void			_ReservedTab12();
*/
	SDTab				&operator=(const SDTab &);
		
	bool 				fEnabled;
	bool				fSelected;
	bool				fFocus;
	BView*				fView;
	uint32				_reserved[12];
	
	char _label[64];
};

/*----------------------------------------------------------------*/
/*----- SDTabView class -------------------------------------------*/

class SDTabView : public BView {
public:
						SDTabView(BRect frame, const char *name,
							button_width width=B_WIDTH_AS_USUAL,
							uint32 resizingMode = B_FOLLOW_ALL,
							uint32 flags = B_FULL_UPDATE_ON_RESIZE |
								B_WILL_DRAW | B_NAVIGABLE_JUMP |
								B_FRAME_EVENTS | B_NAVIGABLE);
//						~SDTabView();

//						SDTabView(BMessage*);							
//static	BArchivable*	Instantiate(BMessage*);
//virtual	status_t		Archive(BMessage*, bool deep=true) const;
//virtual status_t		Perform(perform_code d, void *arg);

//virtual void 			WindowActivated(bool state);
//virtual void 			AttachedToWindow();		
//virtual	void			AllAttached();
//virtual	void			AllDetached();
//virtual	void			DetachedFromWindow();

//virtual void 			MessageReceived(BMessage *msg);
//virtual void 			FrameMoved(BPoint new_position);
//virtual void			FrameResized(float w,float h);
//virtual void 			KeyDown(const char * bytes, int32 n);
virtual void			MouseDown(BPoint);
//virtual void			MouseUp(BPoint);
//virtual void 			MouseMoved(BPoint pt, uint32 code, const BMessage *msg);
//virtual	void			Pulse();

virtual	void 			Select(int32);
//		int32			Selection() const;

//virtual	void			MakeFocus(bool focusState = true);
//virtual void			SetFocusTab(int32,bool);
		int32			FocusTab() const;
		
virtual void 			Draw(BRect);
virtual BRect			DrawTabs();
virtual void			DrawBox(BRect);
virtual BRect			TabFrame(int32) const;
				
//virtual	void			SetFlags(uint32 flags);
//virtual	void			SetResizingMode(uint32 mode);

//virtual void 			GetPreferredSize( float *width, float *height);
//virtual void 			ResizeToPreferred();

//virtual BHandler		*ResolveSpecifier(BMessage *msg, int32 index,
//						BMessage *specifier, int32 form, const char *property);
//virtual	status_t		GetSupportedSuites(BMessage *data);
			
virtual	void 			AddTab(BView* v, SDTab* tab=NULL);
//virtual	SDTab*			RemoveTab(int32) const;
virtual	SDTab*			TabAt(int32) const;
		
//virtual	void			SetTabWidth(button_width s);
		button_width	TabWidth() const;
		
//virtual	void			SetTabHeight(float);
		float			TabHeight() const;		
		
/*----- Private or reserved -----------------------------------------*/
private:
		BView*			_ViewContainer() const;

/*virtual	void			_ReservedTabView1();
virtual	void			_ReservedTabView2();
virtual	void			_ReservedTabView3();
virtual	void			_ReservedTabView4();
virtual	void			_ReservedTabView5();
virtual	void			_ReservedTabView6();
virtual	void			_ReservedTabView7();
virtual	void			_ReservedTabView8();
virtual	void			_ReservedTabView9();
virtual	void			_ReservedTabView10();
virtual	void			_ReservedTabView11();
virtual	void			_ReservedTabView12();
*/
						SDTabView(const SDTabView &);
		SDTabView		&operator=(const SDTabView &);
	
		BList*			fTabList;
		BView*			fViewContainer;
		button_width	fTabWidthSetting;
		float 			fTabWidth;
		float			fTabHeight;
		int32			fSelection;
		int32			fInitialSelection;
		int32			fFocus;	
		uint32			_reserved[12];
};

/*-------------------------------------------------------------*/
/*-------------------------------------------------------------*/

#endif /* _SDTAB_H */

