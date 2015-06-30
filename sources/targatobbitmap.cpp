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

#include <targatobbitmap.h>

BBitmap *targatobbitmap(
	const unsigned char* targa, int targasize)
{
	BBitmap *bitmap;
	int idlength;
	int colormaptype;
	int imagetype;
	int height;
	int width;
	int depth;
	int imagedescriptor;
	int rowbytes;
	int bitssize;
	unsigned char *bits;
	const unsigned char *ptarga;
	unsigned char *pbits;
	int i,j,k;
	int count;
	unsigned char px0,px1,px2;
	
	idlength=targa[0];
	
	colormaptype=targa[1];
	if(colormaptype!=0) return NULL;
	
	imagetype=targa[2];
	if(imagetype!=2 && imagetype!=10) return NULL;
	
	width=targa[12]+(targa[13]<<8);
	height=targa[14]+(targa[15]<<8);
	
	depth=targa[16];
	if(depth!=24) return NULL;
	
	imagedescriptor=targa[17];
	if(imagedescriptor!=0) return NULL;
	
	rowbytes=3*width;
	bitssize=height*rowbytes;
	if(imagetype==2 
	&& 18+idlength+bitssize>targasize) return NULL;	
	
	bits=new unsigned char[bitssize];
	if(!bits) return NULL;

	ptarga=targa+18+idlength;
	
	switch(imagetype){
	case 2:
		pbits=bits+(height-1)*rowbytes;
		for(j=0;j<height;j++){
			for(i=0;i<width;i++){
				pbits[2]=*ptarga++;
				pbits[1]=*ptarga++;
				pbits[0]=*ptarga++;
				pbits+=3;
			}
			pbits-=2*rowbytes;
		}
		break;

		
	case 10:	//	RLE
		pbits=bits+(height-1)*rowbytes;
		for(j=0;j<height;j++){
			for(i=0;i<width;i+=count){
				count=*ptarga++;
				if(count&0x80){
					count&=0x7F;
					count++;
					if(i+count>width) count=width-i;
					px2=*ptarga++;
					px1=*ptarga++;
					px0=*ptarga++;
					for(k=0;k<count;k++){
						pbits[2]=px2;
						pbits[1]=px1;
						pbits[0]=px0;
						pbits+=3;
					}
				}else{
					count++;
					if(i+count>width) count=width-i;
					for(k=0;k<count;k++){
						pbits[2]=*ptarga++;
						pbits[1]=*ptarga++;
						pbits[0]=*ptarga++;
						pbits+=3;
					}
				}
			}
			pbits-=2*rowbytes;
		}
		break;
	}
	
	bitmap=new BBitmap(BRect(0,0,width-1,height-1),B_RGB_32_BIT);
	bitmap->SetBits(bits,bitssize,0,B_RGB_32_BIT);
	
	delete[] bits;
		
	return bitmap;
}

int targatobpic(
	const unsigned char* targa, int targasize,
	BPicture* picture)
{
	BRect frame(0,0,100,100);	// arbitraire
	BBitmap *bitmap;
	BView *view;
	BWindow *window;
	
	bitmap=targatobbitmap(targa,targasize);
	if(!bitmap) return -1;
	
	window=new BWindow(frame,NULL,B_MODAL_WINDOW,0);
	view=new BView(frame,NULL,B_FOLLOW_NONE,0);
	window->AddChild(view);
	
	view->BeginPicture(picture);
	view->DrawBitmap(bitmap);
	view->EndPicture();
	
	delete bitmap;
	delete window;
	
	return 0;
}