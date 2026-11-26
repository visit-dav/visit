/****************************************************************************
* Copyright (c) 2026, CEA
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

#ifndef TRUST_Ref_included
#define TRUST_Ref_included

#include <type_traits>
#include <assert.h>

class Objet_U;
class Nom;

/*! @brief Template reference class for any class _CLASSE_ deriving from Objet_U.
 *
 *   TRUST_Ref<_CLASSE_> is a small template class containing a pointer to an instance of _CLASSE_.
 *
 *   Definition of the REF:
 *
 *   #include <TRUST_Ref.h>
 *   #include <_CLASSE_.h>
 *
 *   Creating an object of type OBS_PTR(_CLASSE_):
 *
 *    OBS_PTR(_CLASSE_) la_ref_;
 *
 *    or equivalently
 *
 *    TRUST_Ref<_CLASSE_>
 *
 */

// MACRO to replace OBS_PTR(_TYPE_) by TRUST_Ref<_TYPE_*> & keep previous syntax for some developers
#define REF(_TYPE_) \
  static_assert(false, "The old REF MACRO is now deprecated. Please use OBS_PTR instead.")

#define OBS_PTR(_TYPE_) TRUST_Ref<_TYPE_*>

template<typename _CLASSE_>
class TRUST_Ref final
{
  /*
   * Elie & Adrien :
   * Define the underlying (non-pointer) type
   * For example if _CLASSE_ = 'Probleme_base*', value_type is 'Probleme_base' : https://en.cppreference.com/w/cpp/types/remove_pointer
   */
  using value_type = typename std::remove_pointer<_CLASSE_>::type;

private:
  value_type * p_ = nullptr;

public:
  static constexpr bool HAS_POINTER = true;

  ~TRUST_Ref() = default;
  TRUST_Ref() = default;
  TRUST_Ref(const value_type& t) :  p_((value_type*)&t) { }
  TRUST_Ref(const TRUST_Ref& t) : p_(t.p_) { }

  const TRUST_Ref& operator=(const value_type& t)
  {
    p_ = const_cast<value_type *>(&t);
    return *this;
  }

  const TRUST_Ref& operator=(const TRUST_Ref& t)
  {
    p_ = t.p_;
    return *this;
  }

  // not deleted because of issues in TRUST_List iterators, e.g. LIST(REF ...)
  operator const value_type& () const { return valeur(); }
  operator value_type& () { return valeur(); }

  inline const value_type& valeur() const { assert(p_ != nullptr); return *p_; }
  inline value_type& valeur() { assert(p_ != nullptr); return *p_; }
  inline const value_type* operator ->() const { assert(p_ != nullptr); return p_; }
  inline value_type* operator ->() { assert(p_ != nullptr); return p_; }

  explicit operator bool() const noexcept { return p_ != nullptr; }

  const Nom& le_nom() const = delete;
  void reset() {  p_ = nullptr; }
};

// The result of == is true if r1 and r2 point to the same object (same key), or if both references are null.
template<typename _CLASSE_>
inline int operator ==(const TRUST_Ref<_CLASSE_>& r1, const TRUST_Ref<_CLASSE_>& r2)
{
  if (!r1 && !r2) return 1;
  if (r1->numero() == r2->numero()) return 1;
  return 0;
}

/* ======================================================= *
 * ======================================================= *
 * ======================================================= */

/*! @brief Class TRUST_Ref_Objet_U.
 *
 * This class is almost identical to TRUST_Ref<>, except that it does not contain implicit conversion operators.
 *
 */
class TRUST_Ref_Objet_U
{
private:
  Objet_U * p_ = nullptr;

public:
  static constexpr bool HAS_POINTER = true;

  ~TRUST_Ref_Objet_U();
  TRUST_Ref_Objet_U();
  TRUST_Ref_Objet_U(const Objet_U& t);
  TRUST_Ref_Objet_U(const TRUST_Ref_Objet_U& t);

  const TRUST_Ref_Objet_U& operator=(const Objet_U& t);
  const TRUST_Ref_Objet_U& operator=(const TRUST_Ref_Objet_U& t);

  explicit operator bool() const noexcept { return p_ != nullptr; }

  inline const Objet_U& valeur() const { assert(p_ != nullptr); return *p_; }
  inline Objet_U& valeur() { assert(p_ != nullptr); return *p_; }
  inline const Objet_U* operator ->() const { assert(p_ != nullptr); return p_; }
  inline Objet_U* operator ->() { assert(p_ != nullptr); return p_; }
};

int operator ==(const TRUST_Ref_Objet_U& r1, const TRUST_Ref_Objet_U& r2);

using RefObjU = TRUST_Ref_Objet_U;

#endif /* TRUST_Ref_included */
