/*****************************************************************************
*
* Copyright (c) 2011, CEA
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of CEA, nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/

// Class declared for compatibility with Trio_U
#ifndef Objet_U_inclu
#define Objet_U_inclu
#include <LataJournal.h>
#include <Sortie.h>
#include <Entree.h>
#include <math.h>
class Process
{
public:
  enum Errors { EXIT };
//  static void exit(int i=0) { throw(EXIT); }
};
class Objet_U: public Process
{
public:
  virtual ~Objet_U() {};
protected:
  virtual Entree & readOn(Entree & is) { return is; };
  virtual Sortie & printOn(Sortie & os) const { return os; };
};
#define declare_instanciable(x)\
 protected:\
  Entree & readOn(Entree & is);\
  Sortie & printOn(Sortie & os) const;
#define declare_instanciable_sans_constructeur_ni_destructeur(x)\
 protected:\
  Entree & readOn(Entree & is);\
  Sortie & printOn(Sortie & os) const;
#define implemente_instanciable(x,y,z)
#define implemente_instanciable_sans_constructeur_ni_destructeur(x,y,z)
#define Cerr Journal()
#define finl endl
#endif
