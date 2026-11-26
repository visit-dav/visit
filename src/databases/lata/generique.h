/****************************************************************************
* Copyright (c) 2022, CEA
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/


#ifndef generique_inclus

#define generique_inclus

/* macros to paste tokens together to form new names */
/* 4.2BSD and System V cpp's have different mechanisms for
   pasting tokens together, but 4.3BSD agrees with System V.
   ANSI C has defined a new way.
*/

#if defined(__STDC__) || defined (__STDCPP__) || defined (MICROSOFT)
/* ANSI C preprocessors will not expand the arguments to a macro;
 * so we need to add a level of indirection to allow macro expansion of
 * arguments.  (Reiser preprocessors allowed the first arg to be expanded;
 * this method will allow both to be expanded, which is better than none.)
 */
#define name2(a,b)      _name2_aux(a,b)
#define _name2_aux(a,b)      a##b
#define name3(a,b,c)    _name3_aux(a,b,c)
#define _name3_aux(a,b,c)    a##b##c
#define name4(a,b,c,d)  _name4_aux(a,b,c,d)
#define _name4_aux(a,b,c,d)  a##b##c##d

#else

#ifdef pyr
/*
 * Pyramid run SVR3, but its cpp uses the BSD4.2 version of argument pasting
 */
#define name2(a,b) a                                \
  b
#define name3(a,b,c) a                                \
  b                                                \
  c
#define name4(a,b,c,d) a                        \
  b                                                \
  c                                                \
  d
#else
/*
 * Most non-ANSI cpps use the "null comment" method
 */
#define name2(a,b)        a/**/b
#define name3(a,b,c)        a/**/b/**/c
#define name4(a,b,c,d)        a/**/b/**/c/**/d
#endif

#endif

#define declare(a,t) name2(declare_,a)(t)
#define implemente(a,t) name2(implemente_,a)(t)

// GF new macro to build real names for lists, Deriv, Ref,
#ifdef _COMPILE_AVEC_GCC_
#define string_macro_trio(a,b) _inter_name(b)
#define _inter_name(a) #a
#else
#define string_macro_trio(a,b) a
#endif
#endif
