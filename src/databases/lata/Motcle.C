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

#include <Motcle.h>
#include <string.h>
#include <istream>
#include <ostream>
#include <string>
Nom & Nom::majuscule()
{
  const int n = longueur()-1; 
  for (int i = 0; i < n; i++) {
    char c = s_[i];
    if (c >= 'a' && c <= 'z')
      s_[i] = c + 'A' - 'a';
  }
  return *this;
}

static inline char char_uppercase(char c)
{
  if (c >= 'a' && c <= 'z')
    c += 'A' - 'a';
  return c;
}


// opt=0 => comparaison des chaines completes
// opt=1 => le debut de n1 doit etre egal a n2
int Motcle::strcmp_uppercase(const char *n1, const char *n2, int opt)
{
  entier i = 0;
  unsigned char c1, c2;
  entier delta;
  do {
    c1 = (unsigned char) char_uppercase(n1[i]);
    c2 = (unsigned char) char_uppercase(n2[i]);
    delta = c1 - c2;
    i++;
    if (c2 == 0 && opt == 1) {
      // Fin de la deuxieme chaine et opt=1 (fonction "debute_par"):
      // Test ok
      return 0;
    }
  } while ((delta == 0) && (c1 != 0) && (c2 != 0));
  return delta;
}

int Nom::debute_par(const char * s) const
{
  const int l1 = longueur()-1;
  const int l2 = strlen(s);
  return (l1>=l2) ? (strncmp(s_.c_str(), s, l2) == 0) : 0;
}

int Nom::finit_par(const char * s) const
{
  const int l1 = longueur()-1;
  const int l2 = strlen(s);
  return (l1>=l2) ? (strncmp(s_.c_str()+(l1-l2), s, l2) == 0) : 0;
}

entier Nom::find(const char * n) const
{ 
  std::size_t x = s_.find(n); 
  return (x != std::string::npos) ? x : -1; 
}

Nom & Nom::prefix(const char *s)
{
  if (finit_par(s)) {
    entier n = strlen(s_.c_str());
    entier n2 = strlen(s);
    s_.erase(n-n2,n2);
  }
  return *this;
}

int Motcle::debute_par(const char * s) const
{
  return (strcmp_uppercase(s_.c_str(), s, 1) == 0);
}

int Motcle::finit_par(const char * s) const
{
  const int l1 = longueur()-1;
  const int l2 = strlen(s);
  return (l1>=l2) ? (strcmp_uppercase(s_.c_str()+(l1-l2), s) == 0) : 0;
}

Motcles noms_to_motcles(const Noms & a) 
{
  Motcles b;
  entier n = a.size();
  for (entier i = 0; i < n; i++)
    b.add() = a[i]; // ouais, ecriture bizarre mais la plus efficace...
  return b;
}

std::istream & operator>>(std::istream & is, Nom & nom)
{ 
  nom.read(is);
  return is; 
}

std::ostream & operator<<(std::ostream & os, const Nom & nom)
{ 
  nom.write(os); 
  return os; 
}

