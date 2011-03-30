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

#ifndef Static_Int_Lists_def
#define Static_Int_Lists_def

#include <ArrOfInt.h>

// .DESCRIPTION
//  Cette classe permet de stocker des listes d'entiers accessibles
//  en temps constant. La taille des listes ne peut pas changer sans
//  perdre le contenu (ce sont des listes statiques).
//  Exemple:
//   Static_Int_List l;
//   ArrOfInt tailles(3);
//   tailles[0] = 2; tailles[1] = 3; tailles[2] = 0;
//   // On reserve la memoire pour trois listes de taille 2, 3 et 0:
//   l.set_list_sizes(tailles);
//   // On affecte une valeur au deuxieme element de la premiere liste:
//   l.set_value(0,1,765);
//   // Affiche la valeur 
//   cout << l(0,1); 
class Static_Int_Lists
{
public:
  void set_list_sizes(const ArrOfInt & sizes);
  void reset();
  void copy_list_to_array(entier i_liste, ArrOfInt & array) const;

  inline void   set_value(entier i_liste, entier i_element, entier valeur);
  inline entier operator() (entier i_liste, entier i_element) const;
  inline entier get_list_size(entier i_liste) const;
  inline entier get_nb_lists() const;

  void trier_liste(entier i);

  // MODIF ELI LAUCOIN (22/08/2007) :
  // j'ajoute un printOn et un readOn 
  Sortie& printOn(Sortie& os) const;
  Entree& readOn(Entree& is);

  // MODIF ELI LAUCOIN (02/09/2007) :
  // j'ajoute un ecrire pour faciliter le debug
  Sortie& ecrire(Sortie& os) const;

private:
  // Les listes d'entiers sont stockees de facon contigue
  // dans le tableau valeurs_. 
  // Le premier element de la liste i est valeurs_[index_[i]] 
  // et le dernier element est valeurs_[index_[i+1]-1]
  // (c'est comme le stockage morse des matrices).
  ArrOfInt index_;
  ArrOfInt valeurs_;
};

// Description: affecte la "valeur" au j-ieme element de la i-ieme liste avec
//  0 <= i < get_nb_lists()  et  0 <= j < get_list_size(i)
inline void Static_Int_Lists::set_value(entier i, entier j, entier valeur)
{
  const entier index = index_[i] + j;
  assert(index < index_[i+1]);
  valeurs_[index] = valeur;
}

// Description: renvoie le j-ieme element de la i-ieme liste avec
//  0 <= i < get_nb_lists()  et  0 <= j < get_list_size(i)
inline entier Static_Int_Lists::operator() (entier i, entier j) const
{
  const entier index = index_[i] + j;
  assert(index < index_[i+1]);
  const entier val = valeurs_[index];
  return val;
}

// Description: renvoie le nombre d'elements de la liste i
inline entier Static_Int_Lists::get_list_size(entier i) const
{
  const entier size = index_[i+1] - index_[i];
  return size;
}

// Description: renvoie le nombre de listes stockees
inline entier Static_Int_Lists::get_nb_lists() const
{
  return index_.size_array() - 1;
}

#endif
