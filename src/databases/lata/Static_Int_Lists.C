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

#include <Static_Int_Lists.h>
// Description: detruit toutes les listes
void Static_Int_Lists::reset()
{
  index_.resize_array(0);
  valeurs_.resize_array(0);
}

// Description: detruit les listes existantes et en cree de nouvelles.
//  On cree autant de listes que d'elements dans le tableau sizes.
//  La i-ieme liste a une taille sizes[i]
//  Les valeurs sizes doivent etre positives ou nulles.
void Static_Int_Lists::set_list_sizes(const ArrOfInt & sizes)
{
  reset();

  const entier nb_listes = sizes.size_array();
  index_.resize_array(nb_listes + 1);
  // Construction du tableau d'index
  index_[0];
  entier i;
  for (i = 0; i < nb_listes; i++) {
    assert(sizes[i] >= 0);
    index_[i+1] = index_[i] + sizes[i];
  }
  const entier somme_sizes = index_[nb_listes];
  valeurs_.resize_array(somme_sizes);
}

// Description: tri par ordre croissant des valeurs de la i-ieme liste.
//  Si num_liste < 0, on trie toutes les listes.
void Static_Int_Lists::trier_liste(entier num_liste)
{
  const entier i_debut = (num_liste < 0) ? 0 : num_liste;
  const entier i_fin   = (num_liste < 0) ? index_.size_array() - 1 : num_liste + 1;

  entier i;
  ArrOfInt valeurs_liste;
  for (i = i_debut; i < i_fin; i++) {
    const entier index = index_[i];
    const entier size  = index_[i+1] - index;
    entier * data      = valeurs_.addr() + index;
    valeurs_liste.ref_data(data, size);
    valeurs_liste.ordonne_array();
  }
}

// Description: copie la i-ieme liste dans le tableau fourni
//  Le tableau array doit etre resizable.
void Static_Int_Lists::copy_list_to_array(entier i, ArrOfInt & array) const
{
  const entier n = get_list_size(i);
  array.resize_array(0); // Ne pas copier les donnees d'origine
  array.resize_array(n);
  entier index = index_[i];
  entier j = 0;
  for (j = 0; j < n; index++, j++)
    array[j] = valeurs_[index];
}

Sortie& Static_Int_Lists::printOn(Sortie& os) const {
  os << index_   << space;
  os << valeurs_ << space;
  return os;
}

Entree& Static_Int_Lists::readOn(Entree& is) {
  reset();
  is >> index_;
  is >> valeurs_;
  return is;
}

Sortie& Static_Int_Lists::ecrire(Sortie& os) const {
  os << "nb lists       : " << get_nb_lists() << finl;
  os << "sizes of lists : ";
  for (entier i=0; i<get_nb_lists(); ++i) {
    os << get_list_size(i) << " ";
  }
  os << finl;

  for (entier i=0; i<get_nb_lists(); ++i) {
    os << "{ " ;
    const entier sz = get_list_size(i);
    for (entier j=0; j<sz; ++j) {
      os <<  valeurs_[(index_[i]+j)] << " ";
    }
    os << "}" << finl;
  }
  return os;
}
