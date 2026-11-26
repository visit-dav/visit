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
#ifndef MD_Vector_included
#define MD_Vector_included

#include <MD_Vector_base.h>
#include <memory>

// Options for arithmetic operations on vectors (mp_min_vect_local, apply_operator, etc...)
//  VECT_SEQUENTIAL_ITEMS: compute requested operation only on sequential items (real items that are not received from another processor)
//   (this is generally slower than VECT_REAL_ITEMS)
//  VECT_REAL_ITEMS: compute requested operation on real items if size_reelle_ok(), otherwise on all items
//  VECT_ALL_ITEMS: compute requested operation on all items (this is equivalent to a call to the Array class operator)
enum Mp_vect_options { VECT_SEQUENTIAL_ITEMS, VECT_REAL_ITEMS, VECT_ALL_ITEMS };

/*! @brief : This class is an OWN_PTR but the pointed object is shared among multiple
 *
 *   instances of this class. The pointed object can only be accessed as "const"
 *    and is only accessible through MD_Vector instances. Therefore
 *    there is no way to access it as "non-const" other than with a cast.
 *   The attach() method and the copy constructor attach the pointer to an
 *    existing instance already attached to a pointer.
 *   The attach_detach() method takes ownership of the object pointed to by the OWN_PTR
 *    and detaches the object from the OWN_PTR. This is the only way to "construct" MD_Vector objects
 *    (avoids a copy and ensures that the MD_Vect can no longer be modified
 *     once it has been attached to an MD_Vector).
 *   WARNING: the safety of this method relies
 *    on the fact that the instance pointed to by MD_Vector is accessible nowhere
 *    else but through MD_Vector objects. DO NOT ADD a method
 *     attach(const MD_Vector_base &), as this breaks the class safety!!! (B.Mathieu)
 *   As many methods as possible are inlined to avoid penalising non-distributed arrays,
 *    while avoiding including MD_Vector_base.h.
 *
 */
class MD_Vector
{
public:
  MD_Vector() {}
  inline MD_Vector(const MD_Vector&);
  inline MD_Vector& operator=(const MD_Vector&);
  inline void attach(const MD_Vector&);
  inline void detach();

  int non_nul() const
  {
#ifndef LATATOOLS
    return (ptr_ != 0);
#else
    return 0;
#endif
  }

  explicit operator bool() const noexcept
  {
#ifndef LATATOOLS
    return (ptr_ != nullptr);
#else
    return false;
#endif
  }

#ifndef LATATOOLS
  void copy(const MD_Vector_base&);

  const MD_Vector_base& valeur() const
  {
    assert(ptr_);
    return *ptr_;
  }

  inline const MD_Vector_base* operator ->() const { assert(ptr_); return ptr_.get(); }

  int operator==(const MD_Vector&) const;
  int operator!=(const MD_Vector&) const;

private:
  std::shared_ptr<MD_Vector_base> ptr_;
#endif
};

/*! @brief Copy constructor, attaches the pointer to the same object as the source.
 */
inline MD_Vector::MD_Vector(const MD_Vector& src)
{
  attach(src);
}

/*! @brief Detaches the pointer from the pointed object.
 */
inline void MD_Vector::detach()
{
#ifndef LATATOOLS
  ptr_ = nullptr;
#endif
}

/*! @brief Detaches the pointer and attaches to the same object as src.
 */
inline void MD_Vector::attach(const MD_Vector& src)
{
#ifndef LATATOOLS
  if (this == &src)
    return; // otherwise the next line would destroy the pointer?
  ptr_ = src.ptr_;
#endif
}

/*! @brief Same as attach(src).
 */
inline MD_Vector& MD_Vector::operator=(const MD_Vector& src)
{
  attach(src);
  return *this;
}

#endif
