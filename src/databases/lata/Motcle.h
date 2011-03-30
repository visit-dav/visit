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

#ifndef LataMotcle_H
#define LataMotcle_H
#include <string>
#include <iostream>
#include <LataVector.h>
#include <arch.h>
// pour gcc 2.96:
#include <stdio.h>
using std::istream;
using std::ostream;
using std::cerr;
using std::endl;

class Motcle;

class Nom
{
public:
  Nom() { s_ = "??"; };



  Nom(std::string str) { s_ = str; }



  virtual ~Nom() { };
  Nom(const char * nom)  : s_(nom) { };
  Nom(char c) { s_ = c; };
  Nom(int i) { char s[30]; sprintf(s, "%d", i); s_ = s; }
  operator const char *() const { return s_.c_str(); }
  virtual Nom & operator=(const char * nom) { s_ = nom; return *this; }
  virtual entier longueur() const { return s_.length()+1; /*ATTENTION: +1 pour compatibilite avec Trio_U*/ }
  virtual void read(istream & is) { is >> s_; }
  virtual void write(ostream & os) const { os << s_; }
  virtual int operator==(const char * s) const { return (s_ == s); }
  virtual int operator!=(const char * s) const { return !operator==(s); }
  virtual Nom & operator+=(const char * n) { s_ += n; return *this; }
  virtual entier find(const char * n) const;
  virtual int debute_par(const char * s) const;
  virtual int finit_par(const char * s) const;
  virtual Nom & prefix(const char * s);
  Nom &         majuscule();
protected:
  friend class Motcle;
  std::string s_;
};

class Motcle : public Nom
{
public:
  Motcle() {};
  Motcle(const char * s) : Nom(s) {};
  Motcle(const Nom & n) : Nom(n) {};
  ~Motcle() {};
  int operator==(const char * s) const { return (strcmp_uppercase(s_.c_str(), s) == 0); }
  int operator!=(const char * s) const { return !operator==(s); }
  Motcle & operator+=(const char * n) { s_ += n; return *this; }
  int debute_par(const char * s) const;
  int finit_par(const char * s) const;
  
  static int strcmp_uppercase(const char * s1, const char * s2, int opt = 0);
  virtual entier find(const char * n) const { return Nom(*this).majuscule().find(Nom(n).majuscule()); }
};

typedef LataVector<Motcle> Motcles;
typedef LataVector<Nom> Noms;

istream & operator>>(istream & is, Nom & nom);
ostream & operator<<(ostream & os, const Nom & nom);

#endif
