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


#ifndef ARROFBIT_H
#define ARROFBIT_H

#include <assert.h>
#include <Objet_U.h>

class ArrOfBit : public Objet_U
{
  declare_instanciable_sans_constructeur_ni_destructeur(ArrOfBit)
public:
  ArrOfBit(entier n=0);
  ArrOfBit(const ArrOfBit & array);             // Constructeur par copie
  ~ArrOfBit();                                  // Destructeur
  ArrOfBit & operator=(const ArrOfBit & array); // Operateur copie
  ArrOfBit & operator=(entier i);
  inline entier operator[](entier i) const;
  inline void setbit(entier i) const;
  inline entier testsetbit(entier i) const;
  inline void clearbit(entier i) const;
  inline entier size_array() const;
  ArrOfBit & resize_array(entier n);
  entier calculer_int_size(entier taille) const;
protected:
  entier taille;
  unsigned int *data;
  static const unsigned int SIZE_OF_INT_BITS;
  static const unsigned int DRAPEAUX_INT;
};

// Description: Renvoie 1 si le bit e est mis, 0 sinon.
inline entier ArrOfBit::operator[](entier e) const
{
  assert(e >= 0 && e < taille);
  unsigned int i = (unsigned int) e;
  unsigned int x = data[i >> SIZE_OF_INT_BITS];
  unsigned int flag = 1 << (i & DRAPEAUX_INT);
  entier resultat = ((x & flag) != 0) ? 1 : 0;
  return resultat;
}

// Description: Met le bit e a 1.
inline void ArrOfBit::setbit(entier e) const
{
  assert(e >= 0 && e < taille);
  unsigned int i = (unsigned int) e;
  unsigned int flag = 1 << (i & DRAPEAUX_INT);
  data[i >> SIZE_OF_INT_BITS] |= flag;
}

// Description: Renvoie la valeur du bit e, puis met le bit e a 1.
inline entier ArrOfBit::testsetbit(entier e) const
{
  assert(e >= 0 && e < taille);
  unsigned int i = (unsigned int) e;
  unsigned int flag = 1 << (i & DRAPEAUX_INT);
  entier index = i >> SIZE_OF_INT_BITS;
  unsigned int old = data[index];
  data[index] = old | flag;
  return ((old & flag) != 0) ? 1 : 0;
}

// Description: Met le bit e a 0.
inline void ArrOfBit::clearbit(entier e) const
{
  assert(e >= 0 && e < taille);
  unsigned int i = (unsigned int) e;
  unsigned int flag = 1 << (i & DRAPEAUX_INT);
  data[i >> SIZE_OF_INT_BITS] &= ~flag;
}

// Description: Renvoie la taille du tableau en bits
inline entier ArrOfBit::size_array() const
{
  return taille;
}

#endif
