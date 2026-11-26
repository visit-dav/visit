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

#ifndef ArrOfBit_included
#define ArrOfBit_included

#include <assert.h>
#include <Objet_U.h>

#ifdef setbit
#undef setbit   // setbit is also a macro in /usr/include/sys/param.h .... too bad ...
#endif

template <typename _SIZE_>
class ArrOfBit_32_64 : public Objet_U
{

  Declare_instanciable_sans_constructeur_ni_destructeur_32_64(ArrOfBit_32_64);

public:
  using int_t = _SIZE_;

  ArrOfBit_32_64(int_t n=0);
  ArrOfBit_32_64(const ArrOfBit_32_64& array);              // Copy constructor
  ~ArrOfBit_32_64() override;                                  // Destructor
  ArrOfBit_32_64& operator=(const ArrOfBit_32_64& array);   // Copy operator
  ArrOfBit_32_64& operator=(int_t i);
  inline int operator[](int_t i) const;
  inline void setbit(int_t i) const;
  inline int testsetbit(int_t i) const;
  inline void clearbit(int_t i) const;
  /// Returns the size of the array in bits
  inline int_t size_array() const { return taille; }
  ArrOfBit_32_64& resize_array(int_t n);
protected:
  int_t calculer_int_size(int_t taille) const;
  int_t taille;
  unsigned int *data;
  static constexpr unsigned int SIZE_OF_INT_BITS = 5;
  static constexpr unsigned int DRAPEAUX_INT = 31;
};

/*! @brief Returns 1 if bit e is set, 0 otherwise.
 *
 */
template <typename _SIZE_>
inline int ArrOfBit_32_64<_SIZE_>::operator[](int_t e) const
{
  assert(e >= 0 && e < taille);
  unsigned int i = (unsigned int) e;
  unsigned int x = data[i >> SIZE_OF_INT_BITS];
  unsigned int flag = 1 << (i & DRAPEAUX_INT);
  int resultat = ((x & flag) != 0) ? 1 : 0;
  return resultat;
}

/*! @brief Set bit e to 1.
 *
 */
template <typename _SIZE_>
inline void ArrOfBit_32_64<_SIZE_>::setbit(int_t e) const
{
  assert(e >= 0 && e < taille);
  unsigned int i = (unsigned int) e;
  unsigned int flag = 1 << (i & DRAPEAUX_INT);
  data[i >> SIZE_OF_INT_BITS] |= flag;
}

/*! @brief Returns the value of bit e, then sets bit e to 1.
 *
 */
template <typename _SIZE_>
inline int ArrOfBit_32_64<_SIZE_>::testsetbit(int_t e) const
{
  assert(e >= 0 && e < taille);
  unsigned int i = (unsigned int) e;
  unsigned int flag = 1 << (i & DRAPEAUX_INT);
  int index = i >> SIZE_OF_INT_BITS;
  unsigned int old = data[index];
  data[index] = old | flag;
  return ((old & flag) != 0) ? 1 : 0;
}

/*! @brief Set bit e to 0.
 *
 */
template <typename _SIZE_>
inline void ArrOfBit_32_64<_SIZE_>::clearbit(int_t e) const
{
  assert(e >= 0 && e < taille);
  unsigned int i = (unsigned int) e;
  unsigned int flag = 1 << (i & DRAPEAUX_INT);
  data[i >> SIZE_OF_INT_BITS] &= ~flag;
}

using ArrOfBit = ArrOfBit_32_64<int>;
using BigArrOfBit = ArrOfBit_32_64<trustIdType>;

#endif
