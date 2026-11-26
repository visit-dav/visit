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

#ifndef TRUSTArray_TPP_included
#define TRUSTArray_TPP_included

#include <string.h>
#include <algorithm>
#include <TRUSTTravPool.h>
#include <DeviceMemory.h>


/*! Destroy array, potentially making block available again if this was a Trav
 */
template <typename _TYPE_, typename _SIZE_>
inline TRUSTArray<_TYPE_, _SIZE_>::~TRUSTArray()
{
  detach_array(); // release all resources properly (esp. Trav!!)
}

/**
 * Change the size of the array.
 *
 * This updates the underlying std::vector size.
 *
 * Conditions:
 *  - if the size changes, type must not be a derived class of TRUSTArray.
 *  - the array can not be resized if data is shared (ref_array)
 *  - the array can not be resized if it is a 'ref_data'.
 */
template <typename _TYPE_, typename _SIZE_>
inline void TRUSTArray<_TYPE_, _SIZE_>::resize_array(_SIZE_ new_size, RESIZE_OPTIONS opt)
{
  // Si le tableau change de taille, il doit etre du type TRUSTArray
  assert(  ( mem_ == nullptr || size_array() == new_size ) ||
           std::string(typeid(*this).name()).find("TRUSTArray") != std::string::npos );
  // ref_arrays can not be resized, except if they keep the same size
  assert( size_array() == new_size || ref_count() <= 1 );
  // ref_data can not be resized:
  assert( span_.empty() || mem_ != nullptr );
  resize_array_(new_size, opt);
}

/**  Methode virtuelle (dans Array_base) identique a resize_array(), permet de traiter
 *   de facon generique les ArrOf, Vect et Tab. Si l'objet est de type TRUSTArray, appel a resize_array(n)
 *
 *   Prerequis: le tableau doit etre "resizable" (voir resize_array()). S'il est d'un type derive (Vect ou Tab),
 *    il ne doit pas avoir de descripteur parallele si la taille est effectivement modifiee.
 */
template <typename _TYPE_, typename _SIZE_>
inline void TRUSTArray<_TYPE_, _SIZE_>::resize_tab(_SIZE_ n, RESIZE_OPTIONS opt)
{
  resize_array(n, opt);
}

//  Change le mode d'allocation memoire lors des resize (voir VTRUSTdata et TRUST_ptr_trav)
//   Exemple pour creer un tableau avec allocation temporaire:
//    DoubleTab tab; // Creation d'un tableau vide
//    tab.set_mem_storage(TEMP_STORAGE); // Changement de mode d'allocation
//    tab.resize(n); // Allocation memoire
template <typename _TYPE_, typename _SIZE_>
inline void TRUSTArray<_TYPE_, _SIZE_>::set_mem_storage(const STORAGE storage)
{
  storage_type_ = storage;
}


/** Make the array point to the memory zone indicated by data_. The array is detached from its current underlying
 * data ('mem_' is released).
 *
 * Warning: virtual method. In derived classes this method initializes the structures to create a sequential array.
 * To create a ref on a parallel array, see TRUSTTab::ref()
 */
template <typename _TYPE_, typename _SIZE_>
inline void TRUSTArray<_TYPE_, _SIZE_>::ref_data(_TYPE_* ptr, _SIZE_ size)
{
  assert(ptr != 0 || size == 0);
  assert(size >= 0);
  assert(storage_type_ != STORAGE::TEMP_STORAGE);  // Not a Trav!
  detach_array(); // ToDo OpenMP revenir en arriere sur TRUSTArray.h
  span_ = Span_(ptr, size);
  // By default, the memory is allocated only on the host:
  data_location_ = std::make_shared<DataLocation>(DataLocation::HostOnly);
}

/** Make the current array point to the data of another existing array. Ownership of the data is hence shared.
 *
 * The current array is first detached (see detach_array()), and then attached to the provided data (see attach_array())
 * Wanring: virtual -> in derived classes this method initializes structures to create a sequential array.
 */
template <typename _TYPE_, typename _SIZE_>
inline void TRUSTArray<_TYPE_, _SIZE_>::ref_array(TRUSTArray& m, _SIZE_ start, _SIZE_ size)
{
  assert(&m != this);
  assert(storage_type_ != STORAGE::TEMP_STORAGE);  // Not a Trav!
  detach_array();
  attach_array(m, start, size);
}

/** Copy the data from another array, potentially resizing.
 * The storage type is not copied.
 */
template <typename _TYPE_, typename _SIZE_>
inline TRUSTArray<_TYPE_,_SIZE_>& TRUSTArray<_TYPE_, _SIZE_>::operator=(const TRUSTArray& m)
{
  if (&m != this)
    {
      const _SIZE_ new_size = m.size_array();
      // On utilise la methode resize_array() qui teste le type derive de l'objet (resize interdit sur un type derive)
      resize_array(new_size, RESIZE_OPTIONS::NOCOPY_NOINIT);
      inject_array(m);
    }
  return *this;
}

/**  operateur [] retourne le ieme element du tableau
* Parametre: _SIZE_ i
*    Signification: indice dans l'intervalle 0 <= i < size_array()
* Exception: assert si i n'est pas dans l'intervalle
*/
template<typename _TYPE_, typename _SIZE_>
inline _TYPE_& TRUSTArray<_TYPE_, _SIZE_>::operator[](_SIZE_ i)
{
#ifdef TRUST_USE_GPU
  this->ensureDataOnHost();
#endif
  assert(i >= 0 && i < size_array());
  return span_[i];
}

template<typename _TYPE_, typename _SIZE_>
inline const _TYPE_& TRUSTArray<_TYPE_, _SIZE_>::operator[](_SIZE_ i) const
{
#ifdef TRUST_USE_GPU
  this->ensureDataOnHost();
#endif
  assert(i >= 0 && i < size_array());
  // [ABN] We can not perform this check here, since we might be *setting* the value from an un-initialized state.
  //       And an uninitialized state might well be completly off the bounds!
  //  assert(span_[i] > -DMAXFLOAT && span_[i] < DMAXFLOAT);
  return span_[i];
}

/** Returns a pointer on the first element of the array. nullptr will be returned if array is detached.
 * Careful, address might change after a resize_array(), ref_data(), etc.
 */
template <typename _TYPE_, typename _SIZE_>
inline _TYPE_* TRUSTArray<_TYPE_, _SIZE_>::addr()
{
  ensureDataOnHost();
  return span_.data();
}

template <typename _TYPE_, typename _SIZE_>
inline const _TYPE_* TRUSTArray<_TYPE_, _SIZE_>::addr() const
{
  ensureDataOnHost();
  return span_.data();
}

template <typename _TYPE_, typename _SIZE_>
inline _TYPE_ *TRUSTArray<_TYPE_, _SIZE_>::data()
{
  return span_.data();
}

template <typename _TYPE_, typename _SIZE_>
inline const _TYPE_ *TRUSTArray<_TYPE_, _SIZE_>::data() const
{
  return span_.data();
}

/** Returns the size of the array.
 */
template <typename _TYPE_, typename _SIZE_>
inline _SIZE_ TRUSTArray<_TYPE_, _SIZE_>::size_array() const
{
  return (_SIZE_)span_.size();
}

/** Returns the number of shared owners of the data underlying the array. If -1 detached array.
  * This is simply the ref count of the shared pointer 'mem_'
  */
template <typename _TYPE_, typename _SIZE_>
inline int TRUSTArray<_TYPE_, _SIZE_>::ref_count() const
{
  return mem_ ? (int)mem_.use_count() : -1;
}

#ifdef TRUST_GTEST
template <typename _TYPE_, typename _SIZE_>
inline int TRUSTArray<_TYPE_, _SIZE_>::nb_dim() const
{
  return this->nb_dim_;
}
#endif

/**  Ajoute une case en fin de tableau et y stocke la "valeur"
 * Precondition:
 *  Le tableau ne doit pas etre "ref_data",
 *  et il ne doit pas y avoir plus d'une reference a la zone de memoire pointee (meme precondition que resize_array())
 *  Le tableau doit etre de type TRUSTArray (pas un type derive)
 */
template <typename _TYPE_, typename _SIZE_>
inline void TRUSTArray<_TYPE_, _SIZE_>::append_array(_TYPE_ valeur)
{
  this->ensureDataOnHost();
  // Call the official resize, with all its checks and management of Trav:
  const _SIZE_ sz = size_array();
  resize_array_(sz+1, RESIZE_OPTIONS::NOCOPY_NOINIT);
  operator[](sz) = valeur;
}

/**  Tri des valeurs du tableau dans l'ordre croissant.
 */
template <typename _TYPE_, typename _SIZE_>
inline void TRUSTArray<_TYPE_,_SIZE_>::ordonne_array()
{
  ensureDataOnHost();
  std::sort(span_.begin(), span_.end());
}

/** Attach the array to (part of) an existing block of data stored in another array.
 * The data is then shared between that other array and this.
 *
 * If size < 0, we take the data from the specified start till the end of the block.
 * Array must be detached first before invoking this method.
 * It is forbidden to attach to a ref_data.
 */
template <typename _TYPE_, typename _SIZE_>
inline void TRUSTArray<_TYPE_, _SIZE_>::attach_array(const TRUSTArray& m, _SIZE_ start, _SIZE_ size)
{
  // Array must be detached
  assert(span_.empty() && mem_ == nullptr);
  // we might attach to an already detached array ... make sure that start and size are coherent
  assert(m.mem_ != nullptr || (start == 0 && size <= 0));
  // we don't attach to ourself:
  assert(&m != this);
  // we don't attach to a ref_data:
  assert(! (m.mem_ == nullptr && !m.span_.empty()) );

  if (m.host_memory_freed()) m.ensureDataOnHost();

  if (size < 0)
    size = m.size_array() - start;

  assert(start >= 0 && size >=0 && start + size <= m.size_array());

  // shared_ptr copy! One more owner for the underlying data - note we migth copy nullptr here ...
  mem_ = m.mem_;

  // Copy (shared) data location from m - locations will be synchronized automatically between ref arrays
  data_location_ = m.data_location_;

  // Copy storage type! A Tab might become a Trav ... (but not the other way around, see ref_*() methods and asserts)
  storage_type_ = m.storage_type_;

  // stupid enough, but we might have ref'ed a detached array ...
  if (mem_ != nullptr)
    span_ = Span_((_TYPE_ *)(m.span_.begin()+start), size);
  else
    span_ = Span_();

}

/** Bring the current array in a detached state, i.e. both 'mem_' and 'span_' are cleared, whatever the current state.
*/
template <typename _TYPE_, typename _SIZE_>
inline bool TRUSTArray<_TYPE_, _SIZE_>::detach_array()
{
  if (mem_ != nullptr && ref_count() == 1)
    {
      if (storage_type_ == STORAGE::TEMP_STORAGE)
        // Give it back to the pool of free blocks - this means a shared_ptr copy, memory will be preserved
        TRUSTTravPool<_TYPE_>::ReleaseBlock(mem_);
      else if (isAllocatedOnDevice(*this))
        deleteOnDevice(*this); // Delete block memory on GPU
    }

  mem_ = nullptr;
  span_ = Span_();
  data_location_ = nullptr;

  return true;
}

//From TRUSTArray.h, now also here, overidden in TRUSTTab
template<typename _TYPE_, typename _SIZE_>
inline _SIZE_ TRUSTArray<_TYPE_,_SIZE_>::dimension_tot(int i) const
{
  assert(i == 0); //If Array, we never want size of index i>0
  return size_array();
}

#endif /* TRUSTArray_TPP_included */
