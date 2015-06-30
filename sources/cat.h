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

/*	Nom :				cat.h
	Type :				Header C
	Auteur :			Sylvain Demongeot
	Date de Création:	1997
	Projet :			-
	Environnement :		-
	Fonction :			concaténation de symboles
	Remarques : 		-
	Bugs :				-
	Copyright :			© Sylvain Demongeot. Reproduction interdite
*/

#define CAT_(x,y) x ## y
#define CAT(x,y) CAT_(x,y)

#define CAT3(a,b,c) CAT(a,CAT_(b,c))
#define CAT4(a,b,c,d) CAT(a,CAT3(b,c,d))
#define CAT5(a,b,c,d,e) CAT(a,CAT4(b,c,d,e))
#define CAT6(a,b,c,d,e,f) CAT(a,CAT5(b,c,d,e,f))
#define CAT7(a,b,c,d,e,f,g) CAT(a,CAT6(b,c,d,e,f,g))
#define CAT8(a,b,c,d,e,f,g,h) CAT(a,CAT7(b,c,d,e,f,g,h))
#define CAT9(a,b,c,d,e,f,g,h,i) CAT(a,CAT8(b,c,d,e,f,g,h,i))
#define CAT10(a,b,c,d,e,f,g,h,i,j) CAT(a,CAT9(b,c,d,e,f,g,h,i,j))
