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

/*	Nom :				setupnetpos.cpp
	Type :				Source c++
	Auteur :			Sylvain Demongeot
	Date de Création:	Mai 1998
	Projet :			-
	Environnement :		BeOS, Netpositive
	Fonction :			modification des préférences de Netpositive
	Remarques : 		-
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#include <setupnetpos.h>

const char appsig[]="application/x-vnd.Be-NPOS";

int setNPprefs(int enableproxies)
{
	status_t r;
	app_info appinfo;

	r=be_roster->GetAppInfo(appsig,&appinfo);
	if(r)
		return setNPprefs_byfile(enableproxies);
	else
		return setNPprefs_byscript(enableproxies);
}

int setNPprefs_byscript(int enableproxies)
{
	BMessage reply; 
	const char *s;

	// open the prefs window
	{
		BMessage message;
		
		// set the command constant 
		message.what = B_EXECUTE_PROPERTY; 
		
		// construct the specifier stack 
		message.AddSpecifier("MenuItem","Preferences…");
		message.AddSpecifier("Menu","Options");
		message.AddSpecifier("MenuBar");
		message.AddSpecifier("Window", 0);
		
		// send the message and fetch the result 
		BMessenger(appsig).SendMessage(&message, &reply); 
		if(reply.what!='RPLY') return reply.what;
	}
		
	// check that "Enable Proxies" is where we expect it
	{
		BMessage message;
		
		// set the command constant 
		message.what = B_GET_PROPERTY; 
		
		// construct the specifier stack 
		message.AddSpecifier("Label");
		message.AddSpecifier("View",0);
		message.AddSpecifier("View",2);
		message.AddSpecifier("View",0);
		message.AddSpecifier("Window", "Preferences");
		
		// send the message and fetch the result 
		BMessenger(appsig).SendMessage(&message, &reply); 
		if(reply.what!='RPLY') return reply.what;

		// check the result
		if(reply.FindString("result",&s)
		|| strcmp(s,"Enable Proxies"))
			return -1;
	}
	
	// set "Enable Proxies"
	{
		BMessage message;
		
		// set the command constant 
		message.what = B_SET_PROPERTY; 
		
		// construct the specifier stack 
		message.AddSpecifier("Value");
		message.AddSpecifier("View",0);
		message.AddSpecifier("View",2);
		message.AddSpecifier("View",0);
		message.AddSpecifier("Window", "Preferences");
		
		// specify the data
		message.AddInt32("data", enableproxies);
	
		// send the message and fetch the result 
		BMessenger(appsig).SendMessage(&message, &reply); 
		if(reply.what!='RPLY') return reply.what;
	}
	
	// check that "HTTP" is where we expect it
	{
		BMessage message;
		
		// set the command constant 
		message.what = B_GET_PROPERTY; 
		
		// construct the specifier stack 
		message.AddSpecifier("Label");
		message.AddSpecifier("View",1);
		message.AddSpecifier("View",2);
		message.AddSpecifier("View",0);
		message.AddSpecifier("Window", "Preferences");
		
		// send the message and fetch the result 
		BMessenger(appsig).SendMessage(&message, &reply); 
		if(reply.what!='RPLY') return reply.what;

		// check the result
		if(reply.FindString("result",&s)
		|| strcmp(s,"HTTP:"))
			return -1;
	}
	
	// set "HTTP"
	{
		BMessage message;
		
		// set the command constant 
		message.what = B_SET_PROPERTY; 
		
		// construct the specifier stack 
		message.AddSpecifier("Value");
		message.AddSpecifier("View",1);
		message.AddSpecifier("View",2);
		message.AddSpecifier("View",0);
		message.AddSpecifier("Window", "Preferences");
		
		// specify the data
		message.AddString("data", "127.0.0.1");
	
		// send the message and fetch the result 
		BMessenger(appsig).SendMessage(&message, &reply); 
		if(reply.what!='RPLY') return reply.what;
	}
		
	// check that "Port" is where we expect it
	{
		BMessage message;
		
		// set the command constant 
		message.what = B_GET_PROPERTY; 
		
		// construct the specifier stack 
		message.AddSpecifier("Label");
		message.AddSpecifier("View",2);
		message.AddSpecifier("View",2);
		message.AddSpecifier("View",0);
		message.AddSpecifier("Window", "Preferences");
		
		// send the message and fetch the result 
		BMessenger(appsig).SendMessage(&message, &reply); 
		if(reply.what!='RPLY') return reply.what;

		// check the result
		if(reply.FindString("result",&s)
		|| strcmp(s,"Port:"))
			return -1;
	}
	
	// set "Port"
	{
		BMessage message;
		
		// set the command constant 
		message.what = B_SET_PROPERTY; 
		
		// construct the specifier stack 
		message.AddSpecifier("Value");
		message.AddSpecifier("View",2);
		message.AddSpecifier("View",2);
		message.AddSpecifier("View",0);
		message.AddSpecifier("Window", "Preferences");
		
		// specify the data
		message.AddString("data", "8080");
	
		// send the message and fetch the result 
		BMessenger(appsig).SendMessage(&message, &reply); 
		if(reply.what!='RPLY') return reply.what;
	}
		
	// check that "OK" is where we expect it
	{
		BMessage message;
		
		// set the command constant 
		message.what = B_GET_PROPERTY; 
		
		// construct the specifier stack 
		message.AddSpecifier("Label");
		message.AddSpecifier("View",4);
		message.AddSpecifier("View",0);
		message.AddSpecifier("Window", "Preferences");
		
		// send the message and fetch the result 
		BMessenger(appsig).SendMessage(&message, &reply); 
		if(reply.what!='RPLY') return reply.what;

		// check the result
		if(reply.FindString("result",&s)
		|| strcmp(s,"OK"))
			return -1;
	}
	
	// click "OK"
	{
		BMessage message;
		
		// set the command constant 
		message.what = B_SET_PROPERTY;
		
		// construct the specifier stack 
		message.AddSpecifier("Value");
		message.AddSpecifier("View",4);
		message.AddSpecifier("View",0);
		message.AddSpecifier("Window", "Preferences");
		
		// specify the data
		message.AddInt32("data", 1);
	
		// send the message and fetch the result 
		BMessenger(appsig).SendMessage(&message, &reply); 
		if(reply.what!='RPLY') return reply.what;
	}

	return 0;
}

int setNPprefs_byfile(int enableproxies)
{
	const char file[]="/boot/home/config/settings/NetPositive/settings";
	const char tmpfile[]="/tmp/npos-settings";
	FILE *fin,*fout;
	char buf[1024];
	int i;
	
	fin=fopen(file,"r");
	if(!fin) return -1;
	
	fout=fopen(tmpfile,"w");
	if(!fout){
		fclose(fin);
		return -1;
	}
	
	for(i=0;;i++){
		if(!fgets(buf,sizeof buf,fin))break;
		
		switch(i){
		case 2:
			fprintf(fout,"%d\n",enableproxies);
			break;
		case 3:
			fprintf(fout,"127.0.0.1 8080\n");
			break;
		default:
			fputs(buf,fout);
			break;		
		}
	}
	
	fclose(fout);
	fclose(fin);
	
	return rename(tmpfile,file);
}
