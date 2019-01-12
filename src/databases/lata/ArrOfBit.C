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

#include <ArrOfBit.h>
#include <string.h>
implemente_instanciable_sans_constructeur_ni_destructeur(ArrOfBit,"ArrOfBit",Objet_U)

const unsigned int ArrOfBit::SIZE_OF_INT_BITS = 5;
const unsigned int ArrOfBit::DRAPEAUX_INT = 31;

// Description: Constructeur d'un tableau de taille n, non initialise
ArrOfBit::ArrOfBit(entier n)
{
  taille = 0;
  data = 0;
  resize_array(n);
}

// Description: Destructeur.
ArrOfBit::~ArrOfBit()
{
  if (data)
    delete[] data;
  data = 0;
}

// Description: Constructeur par copie (deep copy)
ArrOfBit::ArrOfBit(const ArrOfBit & array)
{
  taille = 0; 
  data = 0;
  operator=(array);
}

// Description:
// Taille en "int" du tableau requis pour stocker un tableau de bits
// de taille donnees.
entier ArrOfBit::calculer_int_size(entier taille) const
{
  assert(taille >= 0);
  entier siz = taille >> SIZE_OF_INT_BITS;
  if (taille & DRAPEAUX_INT)
    siz++;
  return siz;
}

// Description: Change la taille du tableau et copie les donnees
// existantes. Si la taille est plus petite, les donnees sont
// tronquees, et si la taille est plus grande, les nouveaux elements
// ne sont pas initialises.
ArrOfBit & ArrOfBit::resize_array(entier n)
{
  if (taille == n)
    return *this;
  assert(n >= 0);
  if (n > 0) {
    entier oldsize = calculer_int_size(taille);
    entier newsize = calculer_int_size(n);
    unsigned int * newdata = new unsigned int[newsize];
    entier size_copy = (newsize > oldsize) ? oldsize : newsize;
    if (size_copy) {
      memcpy(newdata, data, size_copy);
      delete[] data;
    }
    data = newdata;
    taille = n;
  } else {
    delete[] data; // data!=0 sinon taille==n et on ne serait pas ici
    data = 0;
    taille = 0;
  }
  return *this;
}

// Description: Operateur copie (deep copy).
ArrOfBit & ArrOfBit::operator=(const ArrOfBit & array)
{
  entier newsize = calculer_int_size(array.taille);
  if (taille != array.taille) {
    if (data) {
      delete[] data;
      data = 0;
    }
    if (newsize > 0)
      data = new unsigned int[newsize];
  }
  taille = array.taille;
  if (taille)
    memcpy(data, array.data, newsize * sizeof(unsigned int));
  return *this;
}

// Description: Si la valeur est non nulle, met la valeur 1 dans
// tous les elements du tableau, sinon met la valeur 0.

ArrOfBit & ArrOfBit::operator=(entier val)
{
  unsigned int valeur = val ? (~((unsigned int) 0)) : 0;
  entier size = calculer_int_size(taille);
  entier i;
  for (i = 0; i < size; i++)
    data[i] = valeur;
  return *this;
}

// Description: Ecriture du tableau. Format:
// n
// 0 1 0 0 1 0 ... (n valeurs)
Sortie& ArrOfBit::printOn(Sortie& os) const 
{
  os << taille << finl;
  entier i;
  // Un retour a la ligne tous les 32 bits,
  // Une espace tous les 8 bits
  for (i = 0; i < taille; i++) {
    os << operator[](i);
    if ((i & 7) == 7)
      os << " ";
    if ((i & 31) == 31)
      os << finl;
  }
  // Un retour a la ligne si la derniere ligne n'etait pas terminee
  if (i & 31) 
    os << finl;
  return os;
}

// Description: Lecture du tableau. Format:
// n
// 0 1 0 0 1 0 ... (n valeurs)
Entree& ArrOfBit::readOn(Entree& is)
{
  entier newsize;
  is >> newsize;
  resize_array(newsize);
  operator=(0);

  entier i;
  for (i = 0; i < taille; i++) {
    entier bit;
    is >> bit;
    if (bit) setbit(i);
  }
  return is;
}
