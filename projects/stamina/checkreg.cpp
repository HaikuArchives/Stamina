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
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <Window.h>
#include <Alert.h>
#include <TextControl.h>
#include <Button.h>
#include <Screen.h>
#include <FindDirectory.h>
#include <checkreg.h>

enum{
	kMsg_Register='*RG*',
	kMsg_Later='*LR*',
	kMsg_BadCode='*BC*'
};

class RegisterWindow: public BWindow{
public:
	RegisterWindow();
	virtual void MessageReceived(BMessage *message);
	
	void doregister();

	int ntries;
	BTextControl *namebox;
	BTextControl *codebox;
};

int gregistered=0;
char gusername[80];
char gregcode[80];

void checkreg(void)
{
	if(checkregistered())
		doregister();
}

int doregister(void)
{
	RegisterWindow *regw;
	status_t status;
	
	regw=new RegisterWindow();
	wait_for_thread(regw->Thread(),&status);
	
	return !gregistered;
}

RegisterWindow::RegisterWindow():
	BWindow(BRect(0,0,262,110),"Stamina & Charisma registration",
		B_TITLED_WINDOW, B_NOT_RESIZABLE|B_NOT_ZOOMABLE)
{
	BRect rect;
	BView *setupview;
	BButton *button;
	
	ntries=0;

	rect=BScreen(this).Frame();
	MoveTo((rect.Width()-Bounds().Width())/2,
		(rect.Height()-Bounds().Height())/3);
	
	setupview=new BView(Bounds(),NULL,B_FOLLOW_ALL_SIDES,B_WILL_DRAW);
	setupview->SetViewColor(0xDD,0xDD,0xDD);
	AddChild(setupview);
	
	namebox=new BTextControl(BRect(10,10,250,35),NULL,"Name:","",NULL);
	namebox->SetDivider(45);
	setupview->AddChild(namebox);
	
	codebox=new BTextControl(BRect(10,40,250,65),NULL,"Code:","",NULL);
	codebox->SetDivider(45);
	setupview->AddChild(codebox);
	
	button=new BButton(BRect(10,75,100,100),NULL,"Register",new BMessage(kMsg_Register));
	setupview->AddChild(button);
	
	button=new BButton(BRect(160,75,250,100),NULL,"Later",new BMessage(kMsg_Later));
	setupview->AddChild(button);
	
	Show();
}

void RegisterWindow::MessageReceived(BMessage *message)
{
	switch (message->what) {
	case kMsg_Register:
		doregister();
		break;
		
	case kMsg_Later:
		PostMessage(B_QUIT_REQUESTED);
		break;
		
	case kMsg_BadCode:
		snooze(500000);
		(new BAlert("",
			"The code you entered doesn't appear to be a valid registration code. "
			"Please check that you typed the right name and code.\n"
			"If you need help, write to: support@wildbits.com",
			"OK"))->Go();
		if(ntries++>=10) exit(-1);
		break;

	default:
		BWindow::MessageReceived(message);
	}
}

void RegisterWindow::doregister()
{
	FILE *f;
	char fname[B_PATH_NAME_LENGTH];
	int i;
	
	strcpy(gusername,namebox->Text());
	strcpy(gregcode,codebox->Text());
	for(i=0;gusername[i];i++) gusername[i]=toupper(gusername[i]);

	if(checkcode(gusername,gregcode))
		PostMessage(kMsg_BadCode);
	else{
		gregistered=1;
		if(!find_directory(B_COMMON_SETTINGS_DIRECTORY,0,true,fname,sizeof fname)){
			strcat(fname,"/");
			strcat(fname,"SC_reg_code");
			
			f=fopen(fname,"w");
			if(f){
				fprintf(f,"%s\n%s\n",gusername,gregcode);
				fclose(f);
			}
		}
		PostMessage(B_QUIT_REQUESTED);
	}
}

int checkregistered(void)
{
	FILE *f;
	char fname[B_PATH_NAME_LENGTH];
	int r;
	
	r=find_directory(B_COMMON_SETTINGS_DIRECTORY,0,true,fname,sizeof fname);
	if(r) return -1;
	strcat(fname,"/");
	strcat(fname,"SC_reg_code");
	
	f=fopen(fname,"r");
	if(!f)return -1;

	if(!fgets(gusername,sizeof gusername,f)){
		fclose(f);
		return -1;
	}
	gusername[strlen(gusername)-1]=0;
	
	if(!fgets(gregcode,sizeof gregcode,f)){
		fclose(f);
		return -1;
	}
	gregcode[strlen(gregcode)-1]=0;
	
	fclose(f);

	gregistered=!checkcode(gusername,gregcode);
	return !gregistered;
}

int checkcode(const char *name, const char* regcode)
{
	int n;
	unsigned int crc,cs1;

	if(regcode[0]!='A') return -1;
	if(!sscanf(regcode+1,"%*d%n-%X",&n,&cs1)) return -1;
	crc=50671;
	crc=calccrc(crc,(unsigned char*)"A",1);
	crc=calccrc(crc,(unsigned char*)name,strlen(name));
	crc=calccrc(crc,(unsigned char*)regcode+1,n);
	if(crc!=cs1) return -1;
	return 0;
}

unsigned int calccrc(unsigned int crc, const unsigned char* buf, unsigned int sz)
{
	unsigned int i,c,q;

	for(i=0;i<sz;i++){
		c=*buf++;
		
		q=(crc^c)&0xF;
		crc=(crc>>4)^(q*0x1081);
		
		q=(crc^(c>>4))&0xF;
		crc=(crc>>4)^(q*0x1081);
	}

	return crc;
}

