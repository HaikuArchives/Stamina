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

#include <registerdoctype.h>

int registerdoctype(const char *doctype)
{
	app_info appinfo;
	BFile file;
	BAppFileInfo appfileinfo;
	BBitmap icon1(BRect(0,0,31,31),B_CMAP8);
	BBitmap icon2(BRect(0,0,15,15),B_CMAP8);
	
	BMimeType mime(doctype);

	be_app->GetAppInfo(&appinfo);
	file.SetTo(&appinfo.ref,B_READ_ONLY);
	appfileinfo.SetTo(&file);

	appfileinfo.GetIconForType(doctype,&icon1,B_LARGE_ICON);
	mime.SetIcon(&icon1,B_LARGE_ICON);

	appfileinfo.GetIconForType(doctype,&icon2,B_MINI_ICON);
	mime.SetIcon(&icon2,B_MINI_ICON);
	
	return 0;
}
