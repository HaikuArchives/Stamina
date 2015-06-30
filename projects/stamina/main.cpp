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

/*	Nom :				stamina.cpp
	Type :				source C++
	Auteur :			Sylvain Demongeot
	Date de Création:	Février 1998
	Projet :			Stamina
	Environnement :		BeOS
	Fonction :			interface graphique pour "Stamina"
	Remarques : 		-
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include <Application.h>
#include <Window.h>
#include <View.h>
#include <Invoker.h>
#include <ListItem.h>
#include <FilePanel.h>
#include <Path.h>
#include <Alert.h>
#include <ListView.h>
#include <FindDirectory.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <TextControl.h>
#include <RadioButton.h>
#include <CheckBox.h>
#include <MenuField.h>
#include <StringView.h>
#include <TabView.h>
#include <ScrollView.h>
#include <fs_attr.h>
#include <Beep.h>
#include <tools.beos.ui.h>
#include <messages.h>
#include <httpurl.h>
#include <parsehtml.h>
#include <grab.h>
#include <afl_client.h>
#include <runnerview.h>
#include <fatal.h>
#include <registerdoctype.h>
#include <checkreg.h>
#include <progversion.h>
#include <printf_patch.h>

/////////////
#if 0
#include <sdtab.h>
#define BTab SDTab
#define BTabView SDTabView
#endif
/////////////

#if 0
	#define DBG(A) A
#else
	#define DBG(A)
#endif

#define N_VISIBLE_ERRORS 4

const char kapptype[]="application/x-vnd.SD-Stamina";
const char kdoctype[]="application/x-vnd.SD-Stamina-file";
//const int _0=0;
#define _0 0

class StaminaApp: public BApplication{	
public:
	StaminaApp();
	void MessageReceived(BMessage *message);
	void RefsReceived(BMessage *message);
	void ReadyToRun();
	int getdirectory();
	int registertypes();

	int windowcount;
	int nrefsreceived;
	BFilePanel *openpanel;
	BPoint windoworigin;
	int nuntitled;
};

class StaminaWindow: public BWindow{
public:
	StaminaWindow(BPoint origin);
	~StaminaWindow();
	
	virtual void MessageReceived(BMessage *message);
	virtual bool QuitRequested();
	
	void startnow();
	void stopnow();
	
	void gorunning();
	void goidle();
	status_t pulsestats();
	void refreshstats();
	void enablecontrols(bool enabled);
	void setdefaults();
	int savetofile(const char *filename);
	int loadfromfile(const char *filename);
	void setuntitled();
	void printdaysleft();
	int getnetposURL();
	void about();

	static status_t pulsestats_(void *this_)
		{return ((StaminaWindow*)this_)->pulsestats();}

	enum{i_thruput,i_fileswaiting,
		i_meanthruput,i_filesloading,
		i_totalvolume,i_filesloaded,
		NSTATS};
	
	BTextControl *rooturl;
	BTextControl *depth;
	BRadioButton *anywhere;
	BRadioButton *sameserver;
	BRadioButton *downwards;
	BCheckBox *sizecheck;
	BTextControl *sizetext;
	BCheckBox *imagescheck;
	BCheckBox *objectscheck;
	BCheckBox *framescheck;
	BCheckBox *nonHTMLcheck;
	BMenuField *smartrefresh;
	BTextControl *maxsockets;
	BButton *startstop;
	BStringView *stats[NSTATS];
	BListView *errors;
	BTabView *tabview;
	RunnerView *runner;
	
	BFilePanel *savepanel;
	char filepath[B_PATH_NAME_LENGTH];
	
	Grabber *grabber;
	thread_id statpulser;
	BMessenger reporter;
	BInvoker gameover;

	float thruput,thruput1;
	float meanthruput,meanthruput1;
	int32 totalvolume,totalvolume1;
	int32 fileswaiting,fileswaiting1;
	int32 filesloading,filesloading1;
	int32 filesloaded,filesloaded1;

	int netposwindow;
};

class ErrorItem: public BListItem{
public:
	char *text1;
	char *text2;
	
	ErrorItem(const char *text1, const char *text2);
	~ErrorItem();

	virtual void DrawItem(BView *owner, BRect itemRect, bool complete = false);
	virtual void Update(BView *owner, const BFont *font);
};

extern char g_webdir[B_PATH_NAME_LENGTH];

extern int gregistered;
extern char gusername[80];
extern char gregcode[80];

int daysleftprinted=0;

int main()
{	
	StaminaApp *app;

	app=new StaminaApp;
	app->Run();
	delete app;
	
	return 0;
}

StaminaApp::StaminaApp():
	BApplication(kapptype)
{
	int r;
	int32 code;

	checkreg();
	
	windowcount=0;
	nrefsreceived=0;
	openpanel=new BFilePanel(B_OPEN_PANEL,&BMessenger(this));
	windoworigin.Set(100,100);
	nuntitled=1;
	
	// on lance afl_server
	r=afl_init();
	if(r)
		fatal(__FILE__,__LINE__,
			"The file locking mechanism could not be started: %s",
			strerror(r));
	
	// on va chercher le Web directory dans les prefs de Charisma
	getdirectory();
DBG(printf("g_webdir=%s\n",g_webdir);)

	registerdoctype(kdoctype);
}

void StaminaApp::MessageReceived(BMessage *message)
{
	switch (message->what) {
	case kMsg_AddWindow:
		windowcount++;
		break;
		
	case kMsg_RemoveWindow:
		windowcount--;
		if(windowcount<=0)
			PostMessage(B_QUIT_REQUESTED);
		break;
		
	case kMsg_GameOver:
		(new BAlert("Stamina message",
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

void StaminaApp::RefsReceived(BMessage *message)
{
	StaminaWindow *window;
	uint32 type;
	int32 count; 
	entry_ref ref; 
	int i;
	BEntry entry;
	BPath path;

	message->GetInfo("refs", &type, &count); 
	if(type!=B_REF_TYPE) return;
		
	for(i=0;i<count;i++)
		if(message->FindRef("refs", i, &ref)==B_OK){
			window=new StaminaWindow(windoworigin);
			windoworigin+=BPoint(20,20);
			entry.SetTo(&ref);
			entry.GetPath(&path);
			if(window->loadfromfile(path.Path())){
				delete window;
			}else{
				strcpy(window->filepath,path.Path());
				window->SetTitle(path.Leaf());
				window->printdaysleft();
				window->Show();
				nrefsreceived++;
			}
		}
}

void StaminaApp::ReadyToRun()
{
	const char s1[]="Stamina " SPROGVERSION ", © 1998-1999, Sylvain Demongeot";
	const char s2[]="Have a look at 'readme.html' to learn more about this program.";
	StaminaWindow *window;

	if(!nrefsreceived){
		window=new StaminaWindow(windoworigin);
		window->setuntitled();
		windoworigin+=BPoint(20,20);
		window->setdefaults();
		window->errors->AddItem(new ErrorItem(s1,s2));
		window->printdaysleft();
		window->Show();
	}
}

int StaminaApp::getdirectory()
{
	FILE *f;
	char s[200];
	int d;
	char fname[B_PATH_NAME_LENGTH];
	int r;
	
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

	fclose(f);
	return 0;
	
error:
	fclose(f);
	return -1;
}

StaminaWindow::StaminaWindow(BPoint origin):
	BWindow(BRect(origin.x,origin.y,origin.x+100,origin.y+100),
		"Stamina", B_TITLED_WINDOW, 0),
	reporter(this),
	gameover(new BMessage(kMsg_Stop),this)
{
	const float margin=6;
	const float svmargin=9;
	static const char *statmaxstr[NSTATS]={ 	// ceci ne sert qu'à calibrer l'espace occupé	
		"Throughput: 999.9 KB/s",
		"Files waiting: 99999",
		"Mean throughput: 999.9 KB/s",
		"Files loading: 99999",
		"Total volume: 999.9 KB",
		"Files loaded: 99999"
	};
	static const char *statname[NSTATS]={
		"throughput",
		"files waiting",
		"mean throughput",
		"files loading",
		"total volume",
		"files loaded"
	};

	BMenuBar *menubar;
	BMenu *m;
	BMenuItem *mitem;
	BView *setupview;
	BView *box1,*box2,*box3,*box4;
	BTab *tab;
	BScrollView *sv;
	BRect emptyrect(0,0,0,0),r,frame;
	float x,y;
	int i;
	BPoint p;
	
	// initialisation des membres	
	grabber=NULL;
	statpulser=0;
	thruput=0;
	meanthruput=0;
	totalvolume=0;
	filesloaded=0;
	filesloading=0;
	fileswaiting=0,
	thruput1=-1;
	meanthruput1=-1;
	totalvolume1=-1;
	fileswaiting1=-1;
	filesloading1=-1;
	filesloaded1=-1;
	savepanel=new BFilePanel(B_SAVE_PANEL,&BMessenger(this));
	*filepath=0;
	netposwindow=-1;

	frame=Frame();

	// notre barre de menu
	menubar=new BMenuBar(BRect(0,0,0,0),"menu bar");
	m=new BMenu("File");
	m->AddItem(new BMenuItem("New Window",new BMessage(kMsg_NewWindow),'N'));
	m->AddItem(new BMenuItem("Open…",new BMessage(kMsg_OpenWindow),'O'));
	m->AddItem(new BMenuItem("Close",new BMessage(B_QUIT_REQUESTED),'W'));
	m->AddSeparatorItem();
	m->AddItem(new BMenuItem("Save",new BMessage(kMsg_SaveWindow),'S'));
	m->AddItem(new BMenuItem("Save as…",new BMessage(kMsg_SaveWindowAs),'S',B_SHIFT_KEY));
	m->AddSeparatorItem();
	m->AddItem(new BMenuItem("About…",new BMessage(kMsg_About)));
	m->AddSeparatorItem();
	m->AddItem(new BMenuItem("Quit",new BMessage(B_QUIT_REQUESTED),'Q'));
	menubar->AddItem(m);
	m=new BMenu("Edit");
	mitem=new BMenuItem("Cut",new BMessage(B_CUT),'X');
	mitem->SetTarget(NULL,this);
	m->AddItem(mitem);
	mitem=new BMenuItem("Copy",new BMessage(B_COPY),'C');
	mitem->SetTarget(NULL,this);
	m->AddItem(mitem);
	mitem=new BMenuItem("Paste",new BMessage(B_PASTE),'V');
	mitem->SetTarget(NULL,this);
	m->AddItem(mitem);
	mitem=new BMenuItem("Select All",new BMessage(B_SELECT_ALL),'A');
	mitem->SetTarget(NULL,this);
	m->AddItem(mitem);
	m->AddSeparatorItem();
	m->AddItem(new BMenuItem("Get URL from Net+",new BMessage(kMsg_GetNPosURL),'G'));
	m->AddSeparatorItem();
	m->AddItem(new BMenuItem("Clear Error Log",new BMessage(kMsg_ClearErrorLog)));
	menubar->AddItem(m);
	AddChild(menubar);

	// le fond gris
	y=menubar->Frame().bottom;
	setupview=new BView(
		BRect(0,y,frame.Width(),frame.Height()),
		"background",B_FOLLOW_ALL_SIDES,B_WILL_DRAW);
	setupview->SetViewColor(0xD8,0xD8,0xD8);
	AddChild(setupview);
	
	// "From"
	rooturl=new BTextControl(
		BRect(margin,margin,frame.Width()-margin,0),"url",
		"From:",0,
		NULL,
		B_FOLLOW_TOP|B_FOLLOW_LEFT_RIGHT);
	BTextControl_setdivider(rooturl);
	setupview->AddChild(rooturl);
	
	// "Where"
	box1=new BView(emptyrect,"where",B_FOLLOW_NONE,B_WILL_DRAW);
	box1->SetViewColor(0xD8,0xD8,0xD8);
	
	depth=new BTextControl(
		emptyrect,"depth",
		"Depth:","",
		NULL);
	anywhere=new BRadioButton(
		emptyrect,"anywhere",
		"Anywhere",
		NULL);
	sameserver=new BRadioButton(
		emptyrect,"same server",
		"Same server",
		NULL);
	downwards=new BRadioButton(
		emptyrect,"downwards",
		"Downwards",
		NULL);
	{
		BView *views[]={
					depth,anywhere,
					NULL,sameserver,
					NULL,downwards};
		BView_addchilds(box1,views,6);
		BTextControl_resize(depth,"999");
		BView_tile(views,3,2,0.0f,0.0f,20.0f,3.0f);
		BView_cleanup(box1);
	}
	
	// "What"
	box2=new BView(emptyrect,"what",B_FOLLOW_NONE,B_WILL_DRAW);
	box2->SetViewColor(0xD8,0xD8,0xD8);
	
	sizecheck=new BCheckBox(
		emptyrect,"size check",
		"Size limit:",
		new BMessage(kMsg_SizeLimit));
	sizetext=new BTextControl(
		emptyrect,"size text",
		"","",
		NULL);
	imagescheck=new BCheckBox(
		emptyrect,"images",
		"Images",
		NULL);
	objectscheck=new BCheckBox(
		emptyrect,"objects/java",
		"Objects/Java",
		NULL);
	framescheck=new BCheckBox(
		emptyrect,"frames",
		"Frames",
		NULL);
	nonHTMLcheck=new BCheckBox(
		emptyrect,"non-HTML",
		"Non-HTML",
		NULL);
	{
		BView *views[]={sizecheck,sizetext,
					imagescheck,objectscheck,
					framescheck,nonHTMLcheck};
		BView_addchilds(box2,views,6);
		sizecheck->ResizeBy(-4,0);
		BTextControl_resize(sizetext,"100 KB");
		BRect r=BView_tile(views,1,2);
		BView_tile(views+2,2,2,0.0f,r.bottom,25.0f);
		BView_cleanup(box2);
	}
	
	// "How"
	box3=new BView(emptyrect,"how",B_FOLLOW_NONE,B_WILL_DRAW);
	box3->SetViewColor(0xD8,0xD8,0xD8);
	
	m=new BPopUpMenu("");
	m->AddItem(new BMenuItem("Dumb",NULL));
	m->AddSeparatorItem();
	m->AddItem(new BMenuItem("Always",NULL));
	m->AddItem(new BMenuItem("Once per session",NULL));
	m->AddSeparatorItem();
	m->AddItem(new BMenuItem("After 1 hour",NULL));
	m->AddItem(new BMenuItem("After 6 hours",NULL));
	m->AddItem(new BMenuItem("After 12 hours",NULL));
	m->AddSeparatorItem();
	m->AddItem(new BMenuItem("After 1 day",NULL));
	m->AddItem(new BMenuItem("After 2 days",NULL));
	m->AddItem(new BMenuItem("After 3 days",NULL));
	m->AddSeparatorItem();
	m->AddItem(new BMenuItem("After 1 week",NULL));
	m->AddItem(new BMenuItem("After 2 weeks",NULL));
	m->AddSeparatorItem();
	m->AddItem(new BMenuItem("After 1 month",NULL));
	m->AddItem(new BMenuItem("After 2 month",NULL));
	m->AddItem(new BMenuItem("After 6 month",NULL));
	m->AddSeparatorItem();
	m->AddItem(new BMenuItem("After 1 year",NULL));
	m->AddItem(new BMenuItem("After 2 years",NULL));
	m->AddSeparatorItem();
	m->AddItem(new BMenuItem("never",NULL));
	smartrefresh=new BMenuField(
		emptyrect,"smart refresh",
		"Smart refresh:",
		m);
	maxsockets=new BTextControl(
		emptyrect,"max. connections",
		"Max. connections:","",
		NULL);
	{
		BView *views[]={smartrefresh,
					maxsockets};
		BView_addchilds(box3,views,2);
		BMenuField_resize(smartrefresh);
		BTextControl_resize(maxsockets,"999");
		BView_tile(views,2,1);
		BView_cleanup(box3);
	}
		
	// status
	box4=new BView(emptyrect,"status",B_FOLLOW_NONE,B_WILL_DRAW);
	box4->SetViewColor(0xD8,0xD8,0xD8);

	for(i=0;i<NSTATS;i++)
		stats[i]=new BStringView(
		BRect(0,0,margin+setupview->StringWidth(statmaxstr[i]),
			BView_textheight(setupview)),statname[i],
		statmaxstr[i]); 
	BView_setup(box4,(BView**)stats,3,2);
	
	{
		BView *views[]={box1,box2,box3,box4};		
		r=BView_unionframe(views,4);
	}
	
	r.OffsetTo(margin,rooturl->Frame().bottom+margin);
	r.right+=6.0;
	r.bottom+=9.0+BView_textheight(setupview)+6.0;	
		// tabview->TabHeight()==9.0+BView_textheight(setupview)
		
	tabview=new BTabView(r,"tab view",B_WIDTH_FROM_LABEL, 
   		B_FOLLOW_NONE,
   		B_WILL_DRAW|B_NAVIGABLE);

	tab=new BTab(); 
	box1->MoveBy(2.0f,0.0f);
	tabview->AddTab(box1,tab);
	tab->SetLabel("Where"); 
	
	tab=new BTab(); 
	box2->MoveBy(2.0f,0.0f);
	tabview->AddTab(box2,tab);
	tab->SetLabel("What"); 

	tab=new BTab(); 
	box3->MoveBy(2.0f,0.0f);
	tabview->AddTab(box3,tab);
	tab->SetLabel("How"); 
	
	tab=new BTab(); 
	box4->MoveBy(2.0f,0.0f);
	tabview->AddTab(box4,tab);
	tab->SetLabel("Stats"); 
	
	setupview->AddChild(tabview);
	
	// start/stop
	startstop=new BButton(
		rectright(tabview),"start/stop",
		"Start",new BMessage(kMsg_StartStop));
	setupview->AddChild(startstop);
	startstop->ResizeToPreferred();
	SetDefaultButton(startstop);
	
	// runner
	r=startstop->Frame();
	p.x=r.left+(r.Width()-RUNNERWIDTH)/2+RUNNERBORDER;
	p.y=r.bottom+10.0f;
	runner=new RunnerView(p,"runner",B_FOLLOW_NONE);
	setupview->AddChild(runner);

	// errors
	y=BView_childrenframe(setupview).bottom+svmargin;
	r=BRect(svmargin,y,
		frame.Width()-svmargin-B_V_SCROLL_BAR_WIDTH,
		y+N_VISIBLE_ERRORS*BView_textheight(setupview)+6);
	errors=new BListView(
		r,"error log",
		B_MULTIPLE_SELECTION_LIST,B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	sv=new BScrollView("error log scrollview",errors,
		B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP,0,false,true);
	setupview->AddChild(sv);
	
	r=BView_childrenframe(setupview);
	ResizeTo(r.right+margin,menubar->Frame().bottom+r.bottom+svmargin);
	errors->SetResizingMode(B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP_BOTTOM);
	sv->SetResizingMode(B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP_BOTTOM);
	
	x=Frame().Width();
	y=Frame().Height();
	SetSizeLimits(x,10000,y,10000);
	
	goidle();
	
	be_app->PostMessage(kMsg_AddWindow);

	SetPulseRate(0);
}

StaminaWindow::~StaminaWindow()
{
	delete savepanel;
	be_app->PostMessage(kMsg_RemoveWindow);
}

void StaminaWindow::MessageReceived(BMessage *message)
{
	StaminaWindow *window;
	BPoint origin;
	entry_ref ref;
	BEntry entry;
	BPath path;
	char *s,*s1,*s2;	
	int i,n;
	ErrorItem *item;

	switch(message->what){
	
	case kMsg_About:
		about();
		break;

	case kMsg_NewWindow:
		origin=Frame().LeftTop();
		origin+=BPoint(20,20);
		window=new StaminaWindow(origin);
		window->setuntitled();
		window->setdefaults();
		window->Show();
		break;
		
	case kMsg_OpenWindow:
		((StaminaApp*)be_app)->openpanel->Show();
		break;	
		
	case kMsg_SaveWindow:
		if(*filepath){
			savetofile(filepath);
			break;
		}
		
	case kMsg_SaveWindowAs:
		savepanel->Show();
		break;
		
	case B_SAVE_REQUESTED:
		message->FindRef("directory",&ref);
		message->FindString("name",&s);
		entry.SetTo(&ref);
		entry.GetPath(&path);
		sprintf(filepath,"%s/%s",path.Path(),s);
		savetofile(filepath);
		SetTitle(s);
		break;
		
	case kMsg_GetNPosURL:
		getnetposURL();
		break;
	
	case kMsg_ClearErrorLog:
		n=errors->CountItems();
		for(i=0;i<n;i++){
			item=(ErrorItem*)errors->ItemAt(0);
			errors->RemoveItem(item);
			delete item;
	   }
		break;
		
	case kMsg_SizeLimit:
		sizetext->SetEnabled(sizecheck->Value());
		break;
		
	case kMsg_StartStop:
		if(!grabber)
			startnow();
		else
			stopnow();
		break;
		
	case kMsg_Stop:
		stopnow();
		break;

	case kMsg_Report:
		message->FindString("line1",&s1);
		message->FindString("line2",&s2);
		errors->AddItem(new ErrorItem(s1,s2));
		break;
		
	default:
//		message->PrintToStream();//--
		BWindow::MessageReceived(message);
	}
}

bool StaminaWindow::QuitRequested()
{
	stopnow();
	return(TRUE);
}

void StaminaWindow::startnow()
{
	CHttpURL httpurl;
	char url[B_PATH_NAME_LENGTH];
	int idepth;
	int filter;
	int flags;
	int sizelimit;
	float fsizelimit;
	int imaxthreads;
	char multiplier[2];
	int n,i;
	BMenu *m;
	long smartrefresh_date;
	struct system_info sinfo;
	
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
	
	if(grabber) return;

	httpurl.set(rooturl->Text());
	if(!httpurl.m_valid){
		(new BAlert("Stamina error",
			"The URL you entered in the 'From' box is not a valid HTTP URL. "
			"Try URLs like 'http://www.wildbits.com/'",
			"OK",NULL,NULL,B_WIDTH_AS_USUAL,B_STOP_ALERT))->Go();
		return;
	}

	if(1!=sscanf(depth->Text(),"%d",&idepth)
	|| idepth<0){
		tabview->Select(0);
		(new BAlert("Stamina error",
			"'Depth' must be a positive number!",
			"OK",NULL,NULL,B_WIDTH_AS_USUAL,B_STOP_ALERT))->Go();
		return;
	}

	if(sizecheck->Value()){
		n=sscanf(sizetext->Text(),"%f%1s",&fsizelimit,multiplier);
		if(n<1 || fsizelimit<0)
			goto sizeerror;
		if(n==2)
			switch(toupper(*multiplier)){
			case 'K':
				fsizelimit*=0x400;
				break;
			case 'M':
				fsizelimit*=0x100000;
				break;
			case 'G':
				fsizelimit*=0x40000000;
				break;
			default:
			sizeerror:
				tabview->Select(1);
				(new BAlert("Stamina error",
					"'Size limit' must be a size. Valid sizes look like 2048, 100K or 0.5MB.",
					"OK",NULL,NULL,B_WIDTH_AS_USUAL,B_STOP_ALERT))->Go();
				return;
			}
		sizelimit=fsizelimit;
	}else
		sizelimit=0;
DBG(printf("sizelimit=%d\n",sizelimit);)

	if(1!=sscanf(maxsockets->Text(),"%d",&imaxthreads)
	|| imaxthreads<=0){
		tabview->Select(2);
		(new BAlert("Stamina error",
			"'Max. connections' must be a positive number!",
			"OK",NULL,NULL,B_WIDTH_AS_USUAL,B_STOP_ALERT))->Go();
		return;
	}
	
	filter=PHTML_A_HREF|PHTML_AREA_HREF;
	if(imagescheck->Value()) filter|=PHTML_BODY_BACKGROUND|PHTML_IMG_SRC;
	if(objectscheck->Value()) filter|=PHTML_OBJECT_DATA|PHTML_APPLET_CODE;
	if(framescheck->Value()) filter|=PHTML_FRAME_SRC;

	flags=0;
	if(nonHTMLcheck->Value()) flags|=GRAB_NON_HTML_HREFS;
	if(sameserver->Value()||downwards->Value()) flags|=GRAB_SAMESERVER;
	if(downwards->Value()) flags|=GRAB_DOWNWARDS;
	
	m=smartrefresh->Menu();
	i=m->IndexOf(m->FindMarked());
	switch(i){
	case 0:		// dumb
		smartrefresh_date=0;
		break;
	case 2:		// always
		smartrefresh_date=LONG_MAX;
		break;
	case 3:		// once per session
		get_system_info(&sinfo);
		smartrefresh_date=sinfo.boot_time/1000000;
		break;
	case 23:		// never
		smartrefresh_date=-2;
		break;
	default:
		smartrefresh_date=time(NULL)-sroffsets[i];
		break;
	}
	
	gorunning();
	httpurl.get(url);
	rooturl->SetText(url);
	
	grabber=new Grabber(filter,flags,httpurl,
		sizelimit,smartrefresh_date,imaxthreads,
		&totalvolume,&filesloaded,&filesloading,&fileswaiting,
		&reporter,&gameover);

	httpurl.getsimple(url);
	grabber->addurl(&GrabParams(url,idepth));	
}

void StaminaWindow::stopnow()
{
	if(!grabber) return;

	delete grabber;
	grabber=NULL;

	goidle();
}

void StaminaWindow::gorunning()
{
	enablecontrols(false);
	runner->setanimate(true);
	startstop->SetLabel("Stop");
	tabview->Select(3);
	
	totalvolume=0;
	filesloaded=0;
	filesloading=0;
	fileswaiting=0;
	thruput=0;
	meanthruput=0;
	refreshstats();

	statpulser=spawn_thread(pulsestats_,"StaminaWindow::pulsestats",
		B_NORMAL_PRIORITY,this);
	if(statpulser<B_NO_ERROR)
		fatal(__FILE__,__LINE__,"spawn_thread failed");
	resume_thread(statpulser);
}

void StaminaWindow::goidle()
{
	status_t r;

	if(statpulser>0){
DBG(	printf("Killing statpulser (tid=%d)...\n",statpulser);)
		kill_thread(statpulser);
		wait_for_thread(statpulser,&r);
DBG(	printf("Killed statpulser (tid=%d).\n",statpulser);)
		statpulser=0;
	}
	
	enablecontrols(true);
	runner->setanimate(false);
	startstop->SetLabel("Start");
	
	thruput=0;
	filesloading=0;
	fileswaiting=0;
	refreshstats();
}

status_t StaminaWindow::pulsestats()
{
	bigtime_t time0,time,time1;
	int32 totalvolume1;
	
	time0=0;
	time1=0;
	totalvolume1=0;
	
	for(;;){
		snooze(1000000);

		time=real_time_clock_usecs();
		thruput=(float)(totalvolume-totalvolume1)/(time-time1)*1000000.0;
		meanthruput=time0?(float)totalvolume/(time-time0)*1000000.0:0;

		if(!time0 && totalvolume) time0=time;		
		totalvolume1=totalvolume;
		time1=time;
		
		Lock();
		refreshstats();
		Unlock();		
	}
	
	return 0;
}

void StaminaWindow::refreshstats()
{
	char s[80];
	
	if(thruput!=thruput1){
		if(thruput<1024)
			sprintf(s,"Throughput: %d B/s",(int)thruput);
		else
			sprintf(s,"Throughput: %0.1f KB/s",thruput/1024);
		stats[i_thruput]->SetText(s);
		thruput1=thruput;
	}
	
	if(meanthruput!=meanthruput1){
		if(meanthruput<1024)
			sprintf(s,"Mean throughput: %d B/s",(int)meanthruput);
		else
			sprintf(s,"Mean throughput: %0.1f KB/s",meanthruput/1024);
		stats[i_meanthruput]->SetText(s);
		meanthruput1=meanthruput;
	}
	
	if(totalvolume!=totalvolume1){
		sprintf(s,"Total volume: %ld KB",totalvolume/1024);
		stats[i_totalvolume]->SetText(s);
		totalvolume1=totalvolume;
	}
	
	if(fileswaiting!=fileswaiting1){
		sprintf(s,"Files waiting: %ld",fileswaiting);
		stats[i_fileswaiting]->SetText(s);
		fileswaiting1=fileswaiting;
	}
	
	if(filesloading!=filesloading1){
		sprintf(s,"Files loading: %ld",filesloading);
		stats[i_filesloading]->SetText(s);
		filesloading1=filesloading;
	}
	
	if(filesloaded!=filesloaded1){
		sprintf(s,"Files loaded: %ld",filesloaded);
		stats[i_filesloaded]->SetText(s);
		filesloaded1=filesloaded;
	}		
}

void StaminaWindow::enablecontrols(bool enabled)
{
	rooturl->SetEnabled(enabled);
	depth->SetEnabled(enabled);
	anywhere->SetEnabled(enabled);
	sameserver->SetEnabled(enabled);
	downwards->SetEnabled(enabled);
	sizecheck->SetEnabled(enabled);
	sizetext->SetEnabled(enabled && sizecheck->Value());
	imagescheck->SetEnabled(enabled);
	objectscheck->SetEnabled(enabled);
	framescheck->SetEnabled(enabled);
	nonHTMLcheck->SetEnabled(enabled);
	smartrefresh->SetEnabled(enabled);
	maxsockets->SetEnabled(enabled);
}

void StaminaWindow::setdefaults()
{
	rooturl->SetText("http://");
	depth->SetText("0");
	anywhere->SetValue(0);
	sameserver->SetValue(1);
	downwards->SetValue(0);
	sizecheck->SetValue(0);
	sizetext->SetEnabled(sizecheck->Value());
	sizetext->SetText("100K");
	imagescheck->SetValue(1);
	objectscheck->SetValue(1);
	framescheck->SetValue(1);
	nonHTMLcheck->SetValue(1);
	smartrefresh->Menu()->ItemAt(3)->SetMarked(true);
	maxsockets->SetText("5");
}

int StaminaWindow::savetofile(const char *filename)
{
	FILE *f;
	BMenu *m;
	
	f=fopen(filename,"w");
	if(!f)return -1;
	
	fprintf(f,"Stamina_file\n");
	fprintf(f,"version=1\n");
	fprintf(f,"URL=%s\n",rooturl->Text());
	fprintf(f,"depth=%s\n",depth->Text());
	fprintf(f,"where=%s\n",
		anywhere->Value()?"anywhere"
		:sameserver->Value()?"same_server"
		:downwards->Value()?"downwards"
		:"?");
	fprintf(f,"size_limit=%s\n",sizecheck->Value()?"yes":"no");
	fprintf(f,"max_size=%s\n",sizetext->Text());
	fprintf(f,"images=%s\n",imagescheck->Value()?"yes":"no");
	fprintf(f,"objects=%s\n",objectscheck->Value()?"yes":"no");
	fprintf(f,"frames=%s\n",framescheck->Value()?"yes":"no");
	fprintf(f,"non-HTML=%s\n",nonHTMLcheck->Value()?"yes":"no");
	m=smartrefresh->Menu();
	fprintf(f,"smart_refresh=%ld\n",m->IndexOf(m->FindMarked()));
	fprintf(f,"max_sockets=%s\n",maxsockets->Text());
	
	fs_write_attr(fileno(f),"BEOS:TYPE",'MIMS',
		0,kdoctype,strlen(kdoctype)+1);
		
	fclose(f);
	
	return 0;
}

int StaminaWindow::loadfromfile(const char *filename)
{
	FILE *f;
	char s[200],param[200];
	int d;
	
	f=fopen(filename,"r");
	if(!f)return -1;
	
	if(!fgets(s,sizeof s,f)) goto error;
	if(strcmp(s,"Stamina_file\n")) goto error;
	
	if(!fgets(s,sizeof s,f)) goto error;
	if(!sscanf(s,"version=%d",&d)) goto error;

	if(!fgets(s,sizeof s,f)) goto error;
	if(!sscanf(s,"URL=%s",param)) goto error;
	rooturl->SetText(param);

	if(!fgets(s,sizeof s,f)) goto error;
	if(!sscanf(s,"depth=%s",param)) goto error;
	depth->SetText(param);

	if(!fgets(s,sizeof s,f)) goto error;
	if(!sscanf(s,"where=%s",param)) goto error;
	anywhere->SetValue(!strcmp(param,"anywhere"));
	sameserver->SetValue(!strcmp(param,"same_server"));
	downwards->SetValue(!strcmp(param,"downwards"));

	if(!fgets(s,sizeof s,f)) goto error;
	if(!sscanf(s,"size_limit=%s",param)) goto error;
	sizecheck->SetValue(!strcmp(param,"yes"));

	if(!fgets(s,sizeof s,f)) goto error;
	if(!sscanf(s,"max_size=%s",param)) goto error;
	sizetext->SetText(param);

	if(!fgets(s,sizeof s,f)) goto error;
	if(!sscanf(s,"images=%s",param)) goto error;
	imagescheck->SetValue(!strcmp(param,"yes"));

	if(!fgets(s,sizeof s,f)) goto error;
	if(!sscanf(s,"objects=%s",param)) goto error;
	objectscheck->SetValue(!strcmp(param,"yes"));

	if(!fgets(s,sizeof s,f)) goto error;
	if(!sscanf(s,"frames=%s",param)) goto error;
	framescheck->SetValue(!strcmp(param,"yes"));

	if(!fgets(s,sizeof s,f)) goto error;
	if(!sscanf(s,"non-HTML=%s",param)) goto error;
	nonHTMLcheck->SetValue(!strcmp(param,"yes"));

	if(!fgets(s,sizeof s,f)) goto error;
	if(!sscanf(s,"smart_refresh=%d",&d)) goto error;
	smartrefresh->Menu()->ItemAt(d)->SetMarked(true);

	if(!fgets(s,sizeof s,f)) goto error;
	if(!sscanf(s,"max_sockets=%s",param)) goto error;
	maxsockets->SetText(param);
		
	fclose(f);
	return 0;
	
error:
	fclose(f);
	return -1;
}

void StaminaWindow::setuntitled()
{
	char s[100];
	
	sprintf(s,"untitled #%d",((StaminaApp*)be_app)->nuntitled++);
	SetTitle(s);
}

void StaminaWindow::printdaysleft()
{
	const char s1[]="This copy of Stamina is unregistered.";
	const char s2[]="Please use the 'Register' application";

	if(gregistered || daysleftprinted) return;
	errors->AddItem(new ErrorItem(s1,s2));
	daysleftprinted=1;
}

int StaminaWindow::getnetposURL()
{
	const char appsig[]="application/x-vnd.Be-NPOS";
	status_t r;
	app_info appinfo;
	BMessage reply; 
	const char *s;
	int32 n;
	int i;

	r=be_roster->GetAppInfo(appsig,&appinfo);
	if(r){
		beep();
		return r;
	}
	
	{	// count windows
		BMessage message;
		
		// set the command constant 
		message.what = B_COUNT_PROPERTIES; 
		
		// construct the specifier stack 
		message.AddSpecifier("Window");
		
		// send the message and fetch the result 
		r=BMessenger(appsig).SendMessage(&message, &reply);
		if(r){
DBG(		printf("SendMessage: %s\n",strerror(r));)
			beep();
			return r;
		}
DBG(	reply.PrintToStream();)
		if(reply.what!='RPLY'){
			beep();
			return reply.what;
		}
		if(reply.FindInt32("result",&n)){
			beep();
			return -1;
		}
	}
	
	for(i=0;i<n;i++){
		{	// get control label
			BMessage message;
			
			netposwindow++;
			if(netposwindow>=n) netposwindow=0;
			
			// set the command constant 
			message.what = B_GET_PROPERTY; 
			
			// construct the specifier stack 
			message.AddSpecifier("Label");
			message.AddSpecifier("View",5);
			message.AddSpecifier("View",_0);	// compilateur=stupide !
			message.AddSpecifier("View",1);
			message.AddSpecifier("Window",netposwindow);
			
			// send the message and fetch the result 
			r=BMessenger(appsig).SendMessage(&message, &reply); 
			if(r){
DBG(			printf("SendMessage: %s\n",strerror(r));)
				beep();
				return r;
			}
DBG(		reply.PrintToStream();)
			if(reply.what=='RPLY'
			&& !reply.FindString("result",&s)
			&& !strcmp(s,"Location:"))
			{	// get control text
				BMessage message;
				
				// set the command constant 
				message.what = B_GET_PROPERTY; 
				
				// construct the specifier stack 
				message.AddSpecifier("Value");
				message.AddSpecifier("View",5);
				message.AddSpecifier("View",_0);	// compilateur=stupide !
				message.AddSpecifier("View",1);
				message.AddSpecifier("Window",netposwindow);
				
				// send the message and fetch the result 
				r=BMessenger(appsig).SendMessage(&message, &reply); 
				if(r){
DBG(				printf("SendMessage: %s\n",strerror(r));)
					beep();
					return r;
				}
DBG(			reply.PrintToStream();)
				if(reply.what!='RPLY'){
					beep();
					return reply.what;
				}
				if(reply.FindString("result",&s)){
					beep();
					return -1;
				}
				rooturl->SetText(s);
				return 0;
			}			
		}
	}
	
	beep();
	return -1;
}

void StaminaWindow::about()
{
	char buf[1024]="Stamina " SPROGVERSION "\n"
			"© 1998-1999, Sylvain Demongeot.\n"
			"All rights reserved.\n\n";
			
	if(gregistered)
		sprintf(buf+strlen(buf),"This copy is registered to: %s.\n"
			"Registration code: %s",
			gusername,gregcode);
	else
		strcat(buf,"This copy is unregistered.");
	
	(new BAlert("About Stamina",buf,"OK"))->Go();
}

ErrorItem::ErrorItem(const char *text1, const char *text2)
{
	this->text1=new char[strlen(text1)+1];
	this->text2=new char[strlen(text2)+1];
	strcpy(this->text1,text1);
	strcpy(this->text2,text2);
}

ErrorItem::~ErrorItem()
{
	delete[] text1;
	delete[] text2;
}

void ErrorItem::DrawItem(BView *owner, BRect itemRect, bool complete)
{
	if(complete) owner->FillRect(itemRect,B_SOLID_LOW);
	owner->DrawString(text1,BPoint(itemRect.left+2,itemRect.bottom-2-itemRect.Height()/2));
	owner->DrawString(text2,BPoint(itemRect.left+2+8,itemRect.bottom-2));
	if(IsSelected()) owner->InvertRect(itemRect);
}

void ErrorItem::Update(BView *owner, const BFont *font)
{
	BListItem::Update(owner,font);
	SetHeight(2*Height());
}
