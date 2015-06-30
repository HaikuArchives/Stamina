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

/*	Nom :				fatal.cpp
	Type :				Source C
	Auteur :			Sylvain Demongeot
	Date de Création:	Mai 1998
	Projet :			Charisma, Stamina
	Environnement :		BeOS
	Fonction :			sortie du programme après erreur fatale
	Remarques : 		-
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#include <stdio.h>
#include <stdarg.h>
#include <fatal.h>

void fatal(const char *file, int line, const char *format, ...)
{
	char buf1[1024],buf2[1024];
	va_list args;

	va_start(args, format);
	vsprintf(buf1, format, args);
	va_end(args);
	
	sprintf(buf2,
		"Fatal error: %s\n"
		"This error occured in file %s, line %d.\n"
		"This program will quit.",
		buf1,file,line);
	
	puts(buf2);
	(new BAlert("Fatal error",buf2,
		"OK",NULL,NULL,B_WIDTH_AS_USUAL,B_STOP_ALERT))->Go();
	
	exit(-1);
}
