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
#ifndef Static_Int_Lists_included
#define Static_Int_Lists_included

#include <TRUSTTabs_forward.h>
#include <TRUSTArray.h>

/*! @brief This class allows storing lists of integers accessible in constant time.
 *
 * The size of the lists cannot change without
 *   losing the content (these are static lists).
 *   Example:
 *    Static_Int_List l;
 *    ArrOfInt tailles(3);
 *    tailles[0] = 2; tailles[1] = 3; tailles[2] = 0;
 *    // Reserve memory for three lists of size 2, 3 and 0:
 *    l.set_list_sizes(tailles);
 *    // Assign a value to the second element of the first list:
 *    l.set_value(0,1,765);
 *    // Print the value
 *    Cout << l(0,1);
 *
 */
template <typename _SIZE_>
class Static_Int_Lists_32_64
{
public:
  using int_t = _SIZE_;
  using ArrOfInt_t = ArrOfInt_T<_SIZE_>;
  using ArrsOfInt_t = ArrsOfInt_T<_SIZE_>;

  void set_list_sizes(const ArrOfInt_t& sizes);
  void reset();

  void copy_list_to_array(int_t i_liste, ArrOfInt_t& array) const;

  inline void   set_value(int_t i_liste, int_t i_element, int_t valeur);
  inline int_t operator() (int_t i_liste, int_t i_element) const;
  inline int_t get_list_size(int_t i_liste) const;
  inline int_t get_nb_lists() const;
  const ArrOfInt_t& get_index() const  { return index_;    }
  const ArrOfInt_t& get_data() const   { return valeurs_;  }
  void set_data(const ArrOfInt_t& data);
  void set_index_data(const ArrOfInt_t& index, const ArrOfInt_t& data);
  void trier_liste(int_t i);
  void set(const ArrsOfInt_t& src);

  Sortie& printOn(Sortie& os) const;
  Entree& readOn(Entree& is);
  Sortie& ecrire(Sortie& os) const;

private:
  // Integer lists are stored contiguously
  // in the valeurs_ array.
  // The first element of list i is valeurs_[index_[i]]
  // and the last element is valeurs_[index_[i+1]-1]
  // (similar to Morse storage of matrices).
  ArrOfInt_t index_;
  ArrOfInt_t valeurs_;
};

/*! @brief Assigns "valeur" to the j-th element of the i-th list with 0 <= i < get_nb_lists()  and  0 <= j < get_list_size(i).
 *
 */
template <typename _SIZE_>
inline void Static_Int_Lists_32_64<_SIZE_>::set_value(int_t i, int_t j, int_t valeur)
{
  const int_t index = index_[i] + j;
  assert(index < index_[i+1]);
  valeurs_[index] = valeur;
}

/*! @brief Returns the j-th element of the i-th list with 0 <= i < get_nb_lists()  and  0 <= j < get_list_size(i).
 *
 */
template <typename _SIZE_>
inline typename Static_Int_Lists_32_64<_SIZE_>::int_t Static_Int_Lists_32_64<_SIZE_>::operator() (int_t i, int_t j) const
{
  const int_t index = index_[i] + j;
  assert(index < index_[i+1]);
  const int_t val = valeurs_[index];
  return val;
}

/*! @brief Returns the number of elements in list i.
 *
 */
template <typename _SIZE_>
inline typename Static_Int_Lists_32_64<_SIZE_>::int_t Static_Int_Lists_32_64<_SIZE_>::get_list_size(int_t i) const
{
  return index_[i+1] - index_[i];
}

/*! @brief Returns the number of stored lists.
 *
 */
template <typename _SIZE_>
inline typename Static_Int_Lists_32_64<_SIZE_>::int_t Static_Int_Lists_32_64<_SIZE_>::get_nb_lists() const
{
  return index_.size_array() - 1;
}

using Static_Int_Lists = Static_Int_Lists_32_64<int>;
using Static_Int_Lists_64 = Static_Int_Lists_32_64<trustIdType>;

#endif
