/****************************************************************************
* Copyright (c) 2024, CEA
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
#include <Static_Int_Lists.h>
#include <TRUSTArrays.h>

/*! @brief Destroys all lists.
 */
template <typename _SIZE_>
void Static_Int_Lists_32_64<_SIZE_>::reset()
{
  index_.resize_array(0);
  valeurs_.resize_array(0);
}

/*! @brief Destroys existing lists and creates new ones.
 *
 * Creates as many lists as there are elements in the sizes array.
 *   The i-th list has a size of sizes[i].
 *   The sizes values must be non-negative.
 *
 */
template <typename _SIZE_>
void Static_Int_Lists_32_64<_SIZE_>::set_list_sizes(const ArrOfInt_t& sizes)
{
  reset();

  const int_t nb_listes = sizes.size_array();
  index_.resize_array(nb_listes + 1);
  // Build the index array
  index_[0];
  for (int_t i = 0; i < nb_listes; i++)
    {
      assert(sizes[i] >= 0);
      index_[i+1] = index_[i] + sizes[i];
    }
  const int_t somme_sizes = index_[nb_listes];
  valeurs_.resize_array(somme_sizes);
}

/*! @brief Replaces the values stored in all lists with those from the data array.
 *
 * data must have a size equal to the sum of the sizes of all lists.
 *
 */
template <typename _SIZE_>
void Static_Int_Lists_32_64<_SIZE_>::set_data(const ArrOfInt_t& data)
{
  assert(data.size_array() == valeurs_.size_array());
  valeurs_.inject_array(data);
}

#ifndef NDEBUG
// Check consistency of the index and data arrays
template <typename _SIZE_>
static bool check_index_data(const ArrOfInt_T<_SIZE_>& index, const ArrOfInt_T<_SIZE_>& data)
{
  if (index.size_array() < 1)
    return false;
  if (index[0] != 0)
    return false;
  const _SIZE_ n = index.size_array() - 1; // number of lists
  for (_SIZE_ i = 0; i < n; i++)
    if (index[i+1] < index[i])
      return false;
  if (index[n] != data.size_array())
    return false;
  return true;
}
#endif

/*! @brief Replaces index and data arrays.
 *
 * @param index the new index array
 * @param data the new data array
 */
template <typename _SIZE_>
void Static_Int_Lists_32_64<_SIZE_>::set_index_data(const ArrOfInt_t& index, const ArrOfInt_t& data)
{
  assert(check_index_data(index, data));
  index_ = index;
  valeurs_ = data;
}

/*! @brief Sorts the values of the i-th list in ascending order.
 *
 * If num_liste < 0, all lists are sorted.
 *
 * @param num_liste index of the list to sort, or -1 to sort all lists
 */
template <typename _SIZE_>
void Static_Int_Lists_32_64<_SIZE_>::trier_liste(int_t num_liste)
{
  const int_t i_debut = (num_liste < 0) ? 0 : num_liste;
  const int_t i_fin   = (num_liste < 0) ? index_.size_array() - 1 : num_liste + 1;

  ArrOfInt_t valeurs_liste;
  for (int_t i = i_debut; i < i_fin; i++)
    {
      const int_t index = index_[i];
      const int_t size  = index_[i+1] - index;
      valeurs_liste.ref_array(valeurs_, index, size);
      valeurs_liste.ordonne_array();
    }
}

/*! @brief Copies the i-th list into the provided array. The array must be resizable.
 *
 * @param i index of the list to copy
 * @param array destination array to fill
 */
template <typename _SIZE_>
void Static_Int_Lists_32_64<_SIZE_>::copy_list_to_array(int_t i, ArrOfInt_t& array) const
{
  const int_t n = get_list_size(i);
  array.resize_array(n, RESIZE_OPTIONS::NOCOPY_NOINIT);
  const int_t index = index_[i];
  array.inject_array(valeurs_, n, 0 /* destination index */, index /* source index */);
}

template <typename _SIZE_>
Sortie& Static_Int_Lists_32_64<_SIZE_>::printOn(Sortie& os) const
{
#ifndef LATATOOLS
  os << index_   << tspace;
  os << valeurs_ << tspace;
#endif
  return os;
}

template <typename _SIZE_>
Entree& Static_Int_Lists_32_64<_SIZE_>::readOn(Entree& is)
{
  reset();
#ifndef LATATOOLS
  is >> index_;
  is >> valeurs_;
#endif
  return is;
}

template <typename _SIZE_>
Sortie& Static_Int_Lists_32_64<_SIZE_>::ecrire(Sortie& os) const
{
#ifndef LATATOOLS
  os << "nb lists       : " << get_nb_lists() << finl;
  os << "sizes of lists : ";
  for (int_t i=0; i<get_nb_lists(); ++i)
    {
      os << get_list_size(i) << " ";
    }
  os << finl;

  for (int_t i=0; i<get_nb_lists(); ++i)
    {
      os << "{ " ;
      const int_t sz = get_list_size(i);
      for (int_t j=0; j<sz; ++j)
        os <<  valeurs_[(index_[i]+j)] << " ";
      os << "}" << finl;
    }
#endif
  return os;
}

template <typename _SIZE_>
void Static_Int_Lists_32_64<_SIZE_>::set(const ArrsOfInt_t& src)
{
  const int nb_lists = src.size();
  index_.resize_array(nb_lists + 1, RESIZE_OPTIONS::NOCOPY_NOINIT);
  int_t idx = 0;
  index_[0] = 0;
  for (int i = 0; i < nb_lists; i++)
    {
      idx += src[i].size_array();
      index_[i+1] = idx;
    }

  valeurs_.resize_array(idx, RESIZE_OPTIONS::NOCOPY_NOINIT);
  idx = 0;
  for (int i = 0; i < nb_lists; i++)
    {
      const ArrOfInt_t& a = src[i];
      int_t sz = a.size_array();
      valeurs_.inject_array(a, sz, idx /* dest index */, 0 /* source index */);
      idx += sz;
    }
}

template class Static_Int_Lists_32_64<int>;
#if INT_is_64_ == 2
template class Static_Int_Lists_32_64<trustIdType>;
#endif
