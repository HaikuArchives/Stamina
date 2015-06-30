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

#include <stdio.h>
#include <string.h>
#include <Application.h>
#include <Window.h>
#include <View.h>
#include <Alert.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <MenuField.h>
#include <StringView.h>
#include <FilePanel.h>
#include <Path.h>
#include <FindDirectory.h>
#include <tools.beos.ui.h>
#include <targatobbitmap.h>
#include <afl_client.h>
#include <fatal.h>
#include <messages.h>
#include <proxy.h>
#include <checkreg.h>
#include <setupnetpos.h>
#include <progversion.h>

const char kapptype[]="application/x-vnd.SD-Charisma";

class CharismaWindow;

class CharismaApp: public BApplication{	
public:
	CharismaApp();
	~CharismaApp();
	virtual void MessageReceived(BMessage *message);

	CharismaWindow *cw;
};

class CharismaWindow: public BWindow{
public:
	CharismaWindow(BPoint origin);

	virtual void MessageReceived(BMessage *message);
	virtual bool QuitRequested();
	virtual void Minimize(bool minimize);

	void setdefaults();
	int getprefs();
	int saveprefs();
	void update_proxy_settings();
	void setnetpos();
	void about();
	status_t pulsehits();

	static status_t pulsehits_(void *this_)
		{return ((CharismaWindow*)this_)->pulsehits();}

	BMenuBar *menubar;
	BView *setupview;
	BMenuItem *extcontrol_item;
	BMenuItem *netposautoset_item;
	BMenuField *modemenu;
	BMenuField *smartrefresh;
	BStringView *hits;
	thread_id hitspulser;
	int isminim;
	BFilePanel *selectdirpanel;
	BStringView *currentdir;	// appartient à selectdirpanel !
};

// dans proxy.cpp
extern int g_mode;
extern int g_extcontrol;
extern time_t g_refreshdate;
extern char g_webdir[B_PATH_NAME_LENGTH];

extern int gregistered;
extern char gusername[80];
extern char gregcode[80];

int32 hitcount;
int curnetpossetting=-1;

void go_online();
void increment_hits();

main()
{	
	CharismaApp *app;

	app=new CharismaApp;
	app->Run();
	delete app;	

	return 0;
}

CharismaApp::CharismaApp():
	BApplication(kapptype)
{
	int r;
	int32 code;

	checkreg();
	
	// on lance afl_server
	r=afl_init();
	if(r)
		fatal(__FILE__,__LINE__,
			"The file locking mechanism could not be started: %s",
			strerror(r));

	startproxy();

	cw=new CharismaWindow(BPoint(100,100));
	cw->setdefaults();
	cw->getprefs();
	cw->Show();
	cw->setnetpos();
}

CharismaApp::~CharismaApp()
{
	stopproxy();
}

void CharismaApp::MessageReceived(BMessage *message)
{
	switch (message->what) {
	case kMsg_GameOver:
		(new BAlert("Charisma message",
			"The trial period for this program has expired.\n"
			"The Stamina & Charisma pack costs only $19.\n"
			"Register your copy today!",
			"OK"))->Go();
		exit(-1);
		break;		
		
	default:
		BApplication::MessageReceived(message);
		break;
	}
}

#define	MSG new BMessage(kMsg_ProxySettings)

CharismaWindow::CharismaWindow(BPoint origin):
	BWindow(BRect(origin.x,origin.y,origin.x+200,origin.y+80),
		"Charisma", B_TITLED_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, 
		B_NOT_RESIZABLE|B_NOT_ZOOMABLE|B_WILL_ACCEPT_FIRST_CLICK)
{
	BRect nullrect(0,0,0,0),r;
	BMenu *m;
	BWindow *w;
	char buf[100];
	
	isminim=0;
	
	// le menu
	menubar=new BMenuBar(BRect(0,0,0,0),"menu bar");
	m=new BMenu("File");
	m->AddItem(new BMenuItem("About…",new BMessage(kMsg_About)));
	m->AddSeparatorItem();
	m->AddItem(new BMenuItem("Quit",new BMessage(B_QUIT_REQUESTED),'Q'));
	menubar->AddItem(m);
	m=new BMenu("Settings");
	m->AddItem(new BMenuItem("Select Web Directory…",new BMessage(kMsg_SelectDirectory)));
	m->AddSeparatorItem();
	m->AddItem(extcontrol_item=new BMenuItem("External Control",new BMessage(kMsg_ExternalControl)));
	m->AddItem(netposautoset_item=new BMenuItem("Net+ Autosettings",new BMessage(kMsg_NetposAutosettings)));
	m->AddSeparatorItem();
	m->AddItem(new BMenuItem("Clear Hits",new BMessage(kMsg_ClearHits)));
	menubar->AddItem(m);
	AddChild(menubar);

	// le fond gris
	r=Frame();
	setupview=new BView(
		BRect(0,menubar->Frame().bottom,r.Width(),r.Height()),
		"background",B_FOLLOW_NONE,B_WILL_DRAW);
	setupview->SetViewColor(0xDD,0xDD,0xDD);
	AddChild(setupview);
	
	// "Mode"
	m=new BPopUpMenu("");
	m->AddItem(new BMenuItem("Disabled",MSG));
	m->AddItem(new BMenuItem("Offline",MSG));
	m->AddItem(new BMenuItem("Online",MSG));
	modemenu=new BMenuField(
		BRect(10.0f,10.0f,20.0f,20.0f),"mode",
		"Mode:",
		m);
	BMenuField_resize(modemenu);
	setupview->AddChild(modemenu);

	// "Refresh"
	m=new BPopUpMenu("");
	m->AddItem(new BMenuItem("Dumb",MSG));
	m->AddSeparatorItem();
	m->AddItem(new BMenuItem("Always",MSG));
	m->AddItem(new BMenuItem("Once per session",MSG));
	m->AddSeparatorItem();
	m->AddItem(new BMenuItem("After 1 hour",MSG));
	m->AddItem(new BMenuItem("After 6 hours",MSG));
	m->AddItem(new BMenuItem("After 12 hours",MSG));
	m->AddSeparatorItem();
	m->AddItem(new BMenuItem("After 1 day",MSG));
	m->AddItem(new BMenuItem("After 2 days",MSG));
	m->AddItem(new BMenuItem("After 3 days",MSG));
	m->AddSeparatorItem();
	m->AddItem(new BMenuItem("After 1 week",MSG));
	m->AddItem(new BMenuItem("After 2 weeks",MSG));
	m->AddSeparatorItem();
	m->AddItem(new BMenuItem("After 1 month",MSG));
	m->AddItem(new BMenuItem("After 2 month",MSG));
	m->AddItem(new BMenuItem("After 6 month",MSG));
	m->AddSeparatorItem();
	m->AddItem(new BMenuItem("After 1 year",MSG));
	m->AddItem(new BMenuItem("After 2 years",MSG));
	m->AddSeparatorItem();
	m->AddItem(new BMenuItem("never",MSG));
	smartrefresh=new BMenuField(
		rectunder(modemenu),"refresh",
		"Refresh:",
		m);
	BMenuField_resize(smartrefresh);
	setupview->AddChild(smartrefresh);
	
	// "Hits"
	r.left=10.0f;
	r.top=smartrefresh->Frame().bottom+10.0f;
	r.right=r.left+setupview->StringWidth("hits: 99999");
	r.bottom=r.top+BView_textheight(setupview);
	hits=new BStringView(r,"hits","");
	setupview->AddChild(hits);

	if(!gregistered){
		sprintf(buf,"This copy is not registered");
		r.left=10.0f;
		r.top=hits->Frame().bottom+10.0f;
		r.right=r.left+setupview->StringWidth(buf);
		r.bottom=r.top+BView_textheight(setupview);
		setupview->AddChild(new BStringView(r,NULL,buf));
	}

	r=BView_childrenframe(setupview);
	setupview->ResizeTo(r.right+10,r.bottom+10);
	r=setupview->Frame();
	ResizeTo(r.right,r.bottom);
	
	hitcount=0;
	hitspulser=spawn_thread(pulsehits_,"StaminaWindow::pulsehits",
		B_NORMAL_PRIORITY,this);
	if(hitspulser<B_NO_ERROR)
		fatal(__FILE__,__LINE__,"spawn_thread failed");
	resume_thread(hitspulser);
	
	selectdirpanel=new BFilePanel(
		B_OPEN_PANEL,
		&BMessenger(this),
		NULL,
		B_DIRECTORY_NODE,
		false,
		new BMessage(kMsg_DirSelected));
	w=selectdirpanel->Window();
	w->Lock();
	w->SetTitle("Select Web Directory");
	selectdirpanel->SetButtonLabel(B_DEFAULT_BUTTON,"Select");
	r=w->FindView("cancel button")->Frame();
	r.right=r.left-15.0f;
	r.left=10.0f;
	r.bottom=r.top+BView_textheight(w->ChildAt(0));
	currentdir=new BStringView(r,"current","",B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM);
	w->ChildAt(0)->AddChild(currentdir);
	w->Unlock();
	
}

void CharismaWindow::MessageReceived(BMessage *message)
{
	char buf[B_PATH_NAME_LENGTH+1024];
	BWindow *w;
	entry_ref ref;
	BEntry entry;
	BPath path;

	switch (message->what){
	
	case kMsg_About:
		about();
		break;

	case kMsg_SelectDirectory:
		w=selectdirpanel->Window();
		w->Lock();
		sprintf(buf,"Current: %s",g_webdir);
		currentdir->SetText(buf);
		w->Unlock();
		selectdirpanel->Show();
		break;
		
	case kMsg_DirSelected:
		if(message->FindRef("refs", 0, &ref)) break;
		entry.SetTo(&ref);
		entry.GetPath(&path);
		sprintf(g_webdir,"%s/",path.Path());
		sprintf(buf,
			"The new Web directory is: %s\n"
			"If you wish to use previously acquired data, you should move or copy them here.\n"
			"This modification will take effect for Stamina when Stamina is re-launched.",
			g_webdir);
		message->FindRef("refs",&ref);
		(new BAlert("Charisma message",buf,"OK"))->Go();
		saveprefs();
		break;

	case kMsg_ExternalControl:
		extcontrol_item->SetMarked(!extcontrol_item->IsMarked());
		update_proxy_settings();
		break;

	case kMsg_NetposAutosettings:
		netposautoset_item->SetMarked(!netposautoset_item->IsMarked());
		setnetpos();
		break;
		
	case kMsg_ClearHits:
		hitcount=0;
		break;
		
	case kMsg_ProxySettings:
		update_proxy_settings();
		setnetpos();
		break;
		
	default:
		BWindow::MessageReceived(message);
		break;
	}
}

bool CharismaWindow::QuitRequested()
{
	saveprefs();
	g_mode=k_disabled;
	setnetpos();
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}

void CharismaWindow::Minimize(bool minimize)
{
	const float margin=4.0f;
	BRect r;
	
	// en réalité, minimize est ignoré
	
	if(!isminim){
		menubar->Hide();
		setupview->MoveTo(B_ORIGIN);
		modemenu->MoveTo(margin,margin);
	
		r=modemenu->Frame();
		ResizeTo(r.right+margin,r.bottom+margin);
		
		SetLook(B_MODAL_WINDOW_LOOK);

		isminim=1;
	}else{
		menubar->Show();
		setupview->MoveTo(0,menubar->Frame().bottom);
		modemenu->MoveTo(10.0f,10.0f);

		r=setupview->Frame();
		ResizeTo(r.right,r.bottom);

		SetLook(B_TITLED_WINDOW_LOOK);

		isminim=0;
	}
}

void CharismaWindow::setdefaults()
{
	extcontrol_item->SetMarked(1);
	netposautoset_item->SetMarked(0);
		
	modemenu->Menu()->ItemAt(k_offline)->SetMarked(true);
	smartrefresh->Menu()->ItemAt(3)->SetMarked(true);

	update_proxy_settings();
}

int CharismaWindow::getprefs()
{
	FILE *f;
	char s[200],param[200];
	int d;
	char fname[B_PATH_NAME_LENGTH];
	int r;
	float x,y;

	r=find_directory(B_COMMON_SETTINGS_DIRECTORY,0,true,fname,sizeof fname);
	if(r) return r;
	strcat(fname,"/");
	strcat(fname,"Charisma_settings");
	
	f=fopen(fname,"r");
	if(!f)return -1;

	if(!fgets(s,sizeof s,f)) goto error;
	if(strcmp(s,"Charisma_settings\n")) goto error;
	
	if(!fgets(s,sizeof s,f)) goto error;
	if(!sscanf(s,"version=%d",&d)) goto error;

	if(!fgets(s,sizeof s,f)) goto error;
	if(!sscanf(s,"directory=%[^\n]",g_webdir)) goto error;

	if(!fgets(s,sizeof s,f)) goto error;
	if(!sscanf(s,"mode=%d",&d)) goto error;
	modemenu->Menu()->ItemAt(d)->SetMarked(true);

	if(!fgets(s,sizeof s,f)) goto error;
	if(!sscanf(s,"smart_refresh=%d",&d)) goto error;
	smartrefresh->Menu()->ItemAt(d)->SetMarked(true);

	if(!fgets(s,sizeof s,f)) goto error;
	if(!sscanf(s,"external_control=%s",param)) goto error;
	extcontrol_item->SetMarked(!strcmp(param,"yes"));

	if(!fgets(s,sizeof s,f)) goto error;
	if(!sscanf(s,"netpositive_autoset=%s",param)) goto error;
	netposautoset_item->SetMarked(!strcmp(param,"yes"));

	if(!fgets(s,sizeof s,f)) goto error;
	if(2!=sscanf(s,"position=%f,%f\n",&x,&y)) goto error;
	MoveTo(x,y);

	if(!fgets(s,sizeof s,f)) goto error;
	if(!sscanf(s,"minimized=%s",&param)) goto error;
	if(!strcmp(param,"yes")) Minimize(true);

	fclose(f);
	update_proxy_settings();
	return 0;
	
error:
	fclose(f);
	update_proxy_settings();
	return -1;
}

int CharismaWindow::saveprefs()
{
	FILE *f;
	BMenu *m;
	char fname[B_PATH_NAME_LENGTH];
	int r;
	BRect rect;
	
	r=find_directory(B_COMMON_SETTINGS_DIRECTORY,0,true,fname,sizeof fname);
	if(r) return r;
	strcat(fname,"/");
	strcat(fname,"Charisma_settings");
	
	f=fopen(fname,"w");
	if(!f)return -1;

	fprintf(f,"Charisma_settings\n");
	fprintf(f,"version=1\n");
	fprintf(f,"directory=%s\n",g_webdir);
	m=modemenu->Menu();
	fprintf(f,"mode=%d\n",m->IndexOf(m->FindMarked()));
	m=smartrefresh->Menu();
	fprintf(f,"smart_refresh=%d\n",m->IndexOf(m->FindMarked()));
	fprintf(f,"external_control=%s\n",extcontrol_item->IsMarked()?"yes":"no");
	fprintf(f,"netpositive_autoset=%s\n",netposautoset_item->IsMarked()?"yes":"no");
	rect=Frame();
	fprintf(f,"position=%f,%f\n",rect.left,rect.top);
	fprintf(f,"minimized=%s\n",isminim?"yes":"no");

	fclose(f);
	
	return 0;
}

void CharismaWindow::update_proxy_settings()
{
	const long sroffsets[]={
		0,
		0,
		0,
		0,
		0,
		3600,
		3600*6,
		3600*12,
		0,
		3600*24,
		3600*24*2,
		3600*24*3,
		0,
		3600*24*7,
		3600*24*14,
		0,
		3600*24*30,
		3600*24*30*2,
		3600*24*30*6,
		0,
		3600*24*365,
		3600*24*365*2,
		0,
		3600*24*365*10
	};
	
	BMenu *m;
	int i;
	struct system_info sinfo;

	// g_mode
	m=modemenu->Menu();
	g_mode=m->IndexOf(m->FindMarked());
	smartrefresh->SetEnabled(g_mode==k_online);

	// g_extcontrol
	g_extcontrol=extcontrol_item->IsMarked();

	// g_refreshdate
	m=smartrefresh->Menu();
	i=m->IndexOf(m->FindMarked());
	switch(i){
	case 0:		// dumb
		g_refreshdate=0;
		break;
	case 2:		// always
		g_refreshdate=LONG_MAX;
		break;
	case 3:		// once per session
		get_system_info(&sinfo);
		g_refreshdate=sinfo.boot_time/1000000;
		break;
	case 23:	// never
		g_refreshdate=-2;
		break;
	default:
		g_refreshdate=-sroffsets[i];
		// il faut ajouter time(NULL) à cette valeur !
		// (voir proxy.cpp)
		break;
	}
}

void CharismaWindow::setnetpos()
{
	int newnetpossetting;

	if(!netposautoset_item->IsMarked())
		return;
		
	newnetpossetting=g_mode!=k_disabled;
	if(newnetpossetting!=curnetpossetting){
		setNPprefs(newnetpossetting);
		curnetpossetting=newnetpossetting;
	}
}

void CharismaWindow::about()
{
	char buf[1024]="Charisma " SPROGVERSION "\n"
			"© 1998-1999, Sylvain Demongeot.\n"
			"All rights reserved.\n\n";
			
	if(gregistered)
		sprintf(buf+strlen(buf),"This copy is registered to: %s.\n"
			"Registration code: %s",
			gusername,gregcode);
	else
		strcat(buf,"This copy is unregistered.");
	
	(new BAlert("About Charisma",buf,"OK"))->Go();
}

status_t CharismaWindow::pulsehits()
{
	int hits0,hits1;
	char s[40];

	hits1=-1;
	for(;;){
		snooze(100000);

		hits0=hitcount;
		if(hits0!=hits1){
			sprintf(s,"Hits: %d",hitcount);
			Lock();
			hits->SetText(s);
			Unlock();
		}
		hits1=hits0;
	}
	
	return 0;
}

void go_online()
{
	CharismaWindow *cw;
	
	cw=((CharismaApp*)be_app)->cw;
	cw->Lock();
	cw->modemenu->Menu()->ItemAt(k_online)->SetMarked(true);
	cw->smartrefresh->SetEnabled(1);
	cw->setnetpos();
	cw->Unlock();
}


