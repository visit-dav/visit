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

#ifndef TRUSTTab_TPP_included
#define TRUSTTab_TPP_included

#include <TRUSTTab.h>
#include <MD_Vector_seq.h>
#include <tuple>
#include <string>
#ifndef LATATOOLS
#include <Lec_Diffuse_base.h>
#include <Entree_complete.h>
#endif

/*! @brief Returns the size of the "real" space of the array (number of values held by real items, virtual items excluded).
 *
 * If md_vector_ is null, this is the same as size_array(). Otherwise it is the number of real items of the MD_Vector
 * multiplied by line_size(), or the call is invalid (in particular if the real items are not grouped at the start of
 * the array). Use size_reelle_ok() to know whether the call is valid.
 *
 * @return the number of values of the real space
 */
template<typename _TYPE_, typename _SIZE_>
inline _SIZE_ TRUSTTab<_TYPE_,_SIZE_>::size_reelle() const
{
  // Si cet assert plante, c'est que l'appel a ete declare invalide par le MD_Vect associe a ce vecteur.
  assert(size_reelle_ >= 0);
  // Si cet assert plante, c'est que le tableau a ete redimensionne avec resize_array() au lieu de resize(). (invalide pour un Vect ou un Tab).
  assert( (TRUSTArray<_TYPE_,_SIZE_>::size_array() == size_reelle_ || md_vector_.non_nul()) );
  return size_reelle_;
}

/*! @brief Tells whether size() and size_reelle() may be called on this array.
 *
 * @return 1 if the calls are valid, 0 otherwise
 */
template<typename _TYPE_, typename _SIZE_>
inline _SIZE_ TRUSTTab<_TYPE_,_SIZE_>::size_reelle_ok() const
{
  return size_reelle_ >= 0;
}

/*! @brief Same as size_reelle().
 *
 */
template<typename _TYPE_, typename _SIZE_>
inline _SIZE_ TRUSTTab<_TYPE_,_SIZE_>::size() const
{
  return size_reelle();
}

/*! @brief Computes x += alpha * y, where x is this array.
 *
 * If opt is VECT_ALL_ITEMS, the virtual space is then updated with echange_espace_virtuel(). With any other option,
 * no exchange is done (same as ajoute_sans_ech_esp_virt()).
 *
 * @param[in] alpha the scalar factor
 * @param[in] y the array to add; it must have the same structure as this array
 * @param[in] opt the items on which the operation is done (see Mp_vect_options)
 */
template<typename _TYPE_, typename _SIZE_>
template<typename _SCALAR_TYPE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::ajoute(_SCALAR_TYPE_ alpha, const TRUSTTab& y, Mp_vect_options opt)
{
  assert ( (std::is_scalar<_SCALAR_TYPE_>::value) );
  ajoute_alpha_v(*this, alpha, y, opt);
  if (opt == VECT_ALL_ITEMS) echange_espace_virtuel();
}

/*! @brief Same as size_array(): the number of values, real and virtual items included.
 *
 */
template<typename _TYPE_, typename _SIZE_>
inline _SIZE_ TRUSTTab<_TYPE_,_SIZE_>::size_totale() const
{
  return TRUSTArray<_TYPE_,_SIZE_>::size_array();
}

/*! @brief Returns the number of values per item of the parallel structure (product of dimension(i) for i >= 1).
 *
 * It can be zero only if size_array() is zero.
 */
template<typename _TYPE_, typename _SIZE_>
inline int TRUSTTab<_TYPE_,_SIZE_>::line_size() const
{
  // Si line_size_ est nulle, size_array doit etre nul aussi
  assert( (line_size_ > 0 || TRUSTArray<_TYPE_,_SIZE_>::size_array() == 0) );
  return line_size_;
}

/*! @brief Sets line_size_ to n.
 *
 * n must be >= 1; n == 0 is allowed only if size_array() == 0.
 * Precondition: md_vector_ must be null, or line_size_ must not change (case of a resize that changes nothing).
 *   md_vector_ must be set after line_size_, because setting md_vector_ checks the array sizes against line_size_.
 *
 * @param[in] n the new line size
 */
template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::set_line_size_(int n)
{
  assert(!md_vector_.non_nul() || line_size_ == n);
  assert(n >= 0);
  line_size_ = n;
}

/*! @brief Internal resize of the flat data to n values, called by the resize() methods. line_size_ is not changed.
 *
 * Precondition: the array must not have a parallel structure (exits otherwise), it must be resizable (see the
 *   preconditions of TRUSTArray::resize_array()), and n must be a multiple of line_size_.
 *
 * @param[in] n the new total number of values
 * @param[in] opt the resize option (copy and/or initialize the values)
 */
template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::resize_vect_(_SIZE_ n, RESIZE_OPTIONS opt)
{
  if (md_vector_.non_nul())
    {
      Cerr << "Resize of a distributed array is forbidden!" << finl;
      Process::exit();
    }
  assert(n == 0 || (n > 0 && line_size_ > 0 && n % line_size_ == 0));
  TRUSTArray<_TYPE_,_SIZE_>::resize_array_(n, opt);
  size_reelle_ = n;
  // ne pas mettre line_size_ a 1 ici, voir DoubleTab::resize_dim0()
}

/*! @brief Same as ref(const TRUSTTab&), but the result is a one-dimensional array with a line size of 1.
 *
 * A new MD_Vector matching this unit line size is built from the one of v.
 *
 * @param[in] v the array to point to
 */
template<typename _TYPE_, typename _SIZE_>
void TRUSTTab<_TYPE_,_SIZE_>::ref_rewrite_unit_line_size(const TRUSTTab& v)
{
  if (&v != this)
    {
      TRUSTArray<_TYPE_,_SIZE_>::detach_array();
      TRUSTArray<_TYPE_,_SIZE_>::attach_array(v);
      MD_Vector_tools::rewrite_for_unit_line_size(v.line_size_, v.md_vector_, md_vector_);
      size_reelle_ = v.size_reelle_;
      line_size_ = 1;
      this->nb_dim_ = 1;
      dimensions_[0] = size_reelle_;
      dimension_tot_0_ = this->size_array();
    }
}

/*! @brief Copies the size and the values (unless opt is NOCOPY_NOINIT) of the plain array v. The result is one-dimensional.
 *
 * v must really be a TRUSTArray, not a derived type (otherwise it is ambiguous whether the MD_Vector should be copied).
 * Precondition: this array must not have a parallel structure.
 *
 * @param[in] v the array to copy
 * @param[in] opt the resize option
 */
template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::copy(const TRUSTArray<_TYPE_,_SIZE_>& v, RESIZE_OPTIONS opt)
{
  assert(std::string(typeid(v).name()).find("TRUSTArray") != std::string::npos);
  assert(!md_vector_.non_nul());
  resize(v.size_array(), opt);
  if (opt != RESIZE_OPTIONS::NOCOPY_NOINIT) this->inject_array(v);
}

/*! @brief Copies the values (unless opt is NOCOPY_NOINIT), the parallel structure, size_reelle_ and line_size_ of v.
 *
 * Protected helper callable from derived classes, with no precondition on the derived type of *this.
 * The dimensions of the array are not updated: this is up to the caller.
 * Precondition: v is not this array. If this array already has a parallel structure, v must have the same one
 *   (use inject_array() to copy only the values, or reset() first to replace the structure).
 *
 * @param[in] v the array to copy
 * @param[in] opt the resize option
 */
template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::copy_(const TRUSTTab& v, RESIZE_OPTIONS opt)
{
  assert(&v != this); // Il faut avoir fait le test avant !
  // Si le vecteur a deja une structure parallele, la copie n'est autorisee que si
  // le vecteur source a la meme structure. Si ce n'est pas le cas, utiliser inject_array() pour copier uniquement les valeurs, ou faire d'abord reset() si on veut ecraser la structure.
#ifndef LATATOOLS
  assert((!md_vector_.non_nul()) || (md_vector_ == v.md_vector_));
#endif
  TRUSTArray<_TYPE_,_SIZE_>::resize_array_(v.size_array(), RESIZE_OPTIONS::NOCOPY_NOINIT);
  if (v.isDataOnDevice() && !isAllocatedOnDevice(*this))
    allocateOnDevice(*this); // Alloue de la memoire sur le device si v est deja alloue sur le device
  if (opt != RESIZE_OPTIONS::NOCOPY_NOINIT)
    TRUSTArray<_TYPE_,_SIZE_>::inject_array(v);
  md_vector_ = v.md_vector_; // Pour le cas ou md_vector_ est nul et pas v.md_vector_
  size_reelle_ = v.size_reelle_;
  line_size_ = v.line_size_;
}

/*! @brief Updates the values of the virtual items with the values of the matching real items on the other processes.
 *
 * Does nothing in a sequential run. Not allowed on big arrays (_SIZE_ == trustIdType).
 *
 * @param[in] is_exchange_blocking blocking exchange, or start/finish of a non-blocking one
 * @param[in] kernel_name label of the GPU timer around a non-blocking exchange
 */
template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::echange_espace_virtuel(IsExchangeBlocking is_exchange_blocking, const std::string kernel_name)
{
#ifndef LATATOOLS
  if(Process::is_sequential()) return;
#if INT_is_64_ == 2
  // echange_espace_virtuel() should not be called on big array (but can be called on small arrays of TIDs, in SolvPetsc for example):
  assert( (!std::is_same<_SIZE_, trustIdType>::value) );
#endif
  MD_Vector_tools::echange_espace_virtuel(*this, MD_Vector_tools::Operations_echange::ECHANGE_EV, is_exchange_blocking, kernel_name);
#endif
}

/*! @brief Starts a non-blocking echange_espace_virtuel(). It must be completed with finish_echange_espace_virtuel_async().
 *
 * @param[in] kernel_name label of the GPU timer around a non-blocking exchange
 */
template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::start_echange_espace_virtuel_async(const std::string kernel_name)
{
  TRUSTTab<_TYPE_,_SIZE_>::echange_espace_virtuel(IsExchangeBlocking::NonBlockingStart, kernel_name);
}

/*! @brief Completes a non-blocking exchange started with start_echange_espace_virtuel_async().
 *
 * @param[in] kernel_name label of the GPU timer around a non-blocking exchange
 */
template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::finish_echange_espace_virtuel_async(const std::string kernel_name)
{
  TRUSTTab<_TYPE_,_SIZE_>::echange_espace_virtuel(IsExchangeBlocking::NonBlockingFinish, kernel_name);
}

#if INT_is_64_ == 2
// We should never have to do MPI on big arrays:
// [ABN] do not know why, constexpr test in generic version not working ...
template<> inline void TRUSTTab<int, trustIdType>::echange_espace_virtuel(IsExchangeBlocking is_exchange_blocking, const std::string kernel_name)
{
  assert(false);
  Process::exit("echange_espace_virtuel() called on big array or array of int!");
}
template<> inline void TRUSTTab<trustIdType, trustIdType>::echange_espace_virtuel(IsExchangeBlocking is_exchange_blocking, const std::string kernel_name)
{
  assert(false);
  Process::exit("echange_espace_virtuel() called on big array or array of TID!");
}
template<> inline void TRUSTTab<double, trustIdType>::echange_espace_virtuel(IsExchangeBlocking is_exchange_blocking, const std::string kernel_name)
{
  assert(false);
  Process::exit("echange_espace_virtuel() called on big array or array of double!");
}
template<> inline void TRUSTTab<float, trustIdType>::echange_espace_virtuel(IsExchangeBlocking is_exchange_blocking, const std::string kernel_name)
{
  assert(false);
  Process::exit("echange_espace_virtuel() called on big array or array of float!");
}

template<> inline void TRUSTTab<int, trustIdType>::start_echange_espace_virtuel_async(const std::string kernel_name)
{
  assert(false);
  Process::exit("start_echange_espace_virtuel_async(const std::string kernel_name) called on big array or array of int!");
}
template<> inline void TRUSTTab<trustIdType, trustIdType>::start_echange_espace_virtuel_async(const std::string kernel_name)
{
  assert(false);
  Process::exit("start_echange_espace_virtuel_async(const std::string kernel_name) called on big array or array of TID!");
}
template<> inline void TRUSTTab<double, trustIdType>::start_echange_espace_virtuel_async(const std::string kernel_name)
{
  assert(false);
  Process::exit("start_echange_espace_virtuel_async(const std::string kernel_name) called on big array or array of double!");
}
template<> inline void TRUSTTab<float, trustIdType>::start_echange_espace_virtuel_async(const std::string kernel_name)
{
  assert(false);
  Process::exit("start_echange_espace_virtuel_async(const std::string kernel_name) called on big array or array of float!");
}

template<> inline void TRUSTTab<int, trustIdType>::finish_echange_espace_virtuel_async(const std::string kernel_name)
{
  assert(false);
  Process::exit("finish_echange_espace_virtuel_async(const std::string kernel_name) called on big array or array of int!");
}
template<> inline void TRUSTTab<trustIdType, trustIdType>::finish_echange_espace_virtuel_async(const std::string kernel_name)
{
  assert(false);
  Process::exit("finish_echange_espace_virtuel_async(const std::string kernel_name) called on big array or array of TID!");
}
template<> inline void TRUSTTab<double, trustIdType>::finish_echange_espace_virtuel_async(const std::string kernel_name)
{
  assert(false);
  Process::exit("finish_echange_espace_virtuel_async(const std::string kernel_name) called on big array or array of double!");
}
template<> inline void TRUSTTab<float, trustIdType>::finish_echange_espace_virtuel_async(const std::string kernel_name)
{
  assert(false);
  Process::exit("finish_echange_espace_virtuel_async(const std::string kernel_name) called on big array or array of float!");
}


#endif

/*! @brief Computes x += alpha * y, where x is this array, without updating the virtual space.
 *
 * @param[in] alpha the scalar factor
 * @param[in] y the array to add; it must have the same structure as this array
 * @param[in] opt the items on which the operation is done (see Mp_vect_options)
 */
template<typename _TYPE_, typename _SIZE_>
template<typename _SCALAR_TYPE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::ajoute_sans_ech_esp_virt(_SCALAR_TYPE_ alpha, const TRUSTTab& y, Mp_vect_options opt)
{
  ajoute_alpha_v(*this, alpha, y, opt); // x+=alpha*y sans echange_espace_virtuel
}

/*! @brief Computes z += alpha * x * y element by element, where z is this array. The virtual space is not updated.
 *
 * @param[in] alpha the scalar factor
 * @param[in] x first factor; it must have the same structure as this array
 * @param[in] y second factor; it must have the same structure as this array
 * @param[in] opt the items on which the operation is done (see Mp_vect_options)
 */
template<typename _TYPE_, typename _SIZE_>
template<typename _SCALAR_TYPE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::ajoute_produit_scalaire(_SCALAR_TYPE_ alpha, const TRUSTTab& x, const TRUSTTab& y, Mp_vect_options opt)
{
  ajoute_produit_scalaire(*this, alpha, x, y, opt);  // z+=alpha*x*y;
}

/*! @brief Computes x += alpha * y * y element by element, where x is this array. The virtual space is not updated.
 *
 * @param[in] alpha the scalar factor
 * @param[in] y the array to square; it must have the same structure as this array
 * @param[in] opt the items on which the operation is done (see Mp_vect_options)
 */
template<typename _TYPE_, typename _SIZE_>
template<typename _SCALAR_TYPE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::ajoute_carre(_SCALAR_TYPE_ alpha, const TRUSTTab& y, Mp_vect_options opt)
{
  ajoute_operation_speciale_generic<TYPE_OPERATION_VECT_SPEC::SQUARE_,_TYPE_>(*this,alpha,y,opt);
}



#ifndef LATATOOLS
/*! @brief Returns the underlying stream of is if the reader may rewind it, nullptr otherwise.
 *
 * Diffuse readers (the master reads, then broadcasts) and Entree_complete do not give access to a stream that
 * can be rewound on every process.
 */
inline std::istream * rewindable_stream_(Entree& is)
{
  if (dynamic_cast<Lec_Diffuse_base*>(&is) || dynamic_cast<Entree_complete*>(&is))
    return nullptr;
  return &is.get_istream();
}
#endif

template<typename _TYPE_, typename _SIZE_>
Entree& TRUSTTab<_TYPE_,_SIZE_>::readOn(Entree& is)
{
#ifndef LATATOOLS
  // What should we do if we read into a vector that already has a parallel structure?
  if (md_vector_)
    Process::exit("Error in TRUSTTab::readOn: vector has a parallel structure");

  std::istream * stream = rewindable_stream_(is);
  const std::istream::pos_type start_position = stream ? stream->tellg() : std::istream::pos_type(-1);
  const bool can_rewind = stream && start_position != std::istream::pos_type(-1);

  auto fallback_or_exit = [&]() -> Entree&
  {
    if (!can_rewind)
      Process::exit("ERROR in TRUSTTab::readOn: Cannot rewind this stream to read it as a TRUSTVect.");
    Cerr << "Warning in TRUSTTab::readOn: reading the array again in the one-dimensional format" << finl;
    stream->clear();
    stream->seekg(start_position);
    return readOnAsVect(is);
  };

  is >> this->nb_dim_;
  if (this->nb_dim_ < 1 || this->nb_dim_ > MAXDIM_TAB)
    {
      Cerr << "Warning in TRUSTTab::readOn: wrong this->nb_dim_ = " << this->nb_dim_ << finl;
      return fallback_or_exit();
    }
  is.get(dimensions_, this->nb_dim_);
  if (dimensions_[0] < 0)
    {
      Cerr << "Warning in TRUSTTab::readOn: wrong dimension(0) = " << dimensions_[0] << finl;
      return fallback_or_exit();
    }
  int l_size = 1;
  for (int i = 1; i < this->nb_dim_; i++)
    {
      if (dimensions_[i] < 0)
        {
          Cerr << "Warning in TRUSTTab::readOn: wrong dimension(" << i << ") = " << dimensions_[i] << finl;
          return fallback_or_exit();
        }
      l_size *= (int)dimensions_[i];
    }
  dimension_tot_0_ = dimensions_[0];
  TRUSTArray<_TYPE_,_SIZE_>::readOn(is);
  size_reelle_ = this->size_array();
  line_size_ = 1;
  set_line_size_(l_size);
  if (dimension_tot_0_ * l_size != this->size_array())
    {
      Cerr << "Warning in TRUSTTab::readOn: wrong size_array " << this->size_array() << ", expected " << dimension_tot_0_ * l_size << finl;
      return fallback_or_exit();
    }
  assert(verifie_LINE_SIZE());
#endif
  return is;
}

template<typename _TYPE_, typename _SIZE_>
Entree& TRUSTTab<_TYPE_,_SIZE_>::readOnAsVect(Entree& is)
{
#ifndef LATATOOLS
  TRUSTArray<_TYPE_,_SIZE_>::readOn(is);
  size_reelle_ = this->size_array();
  line_size_ = 1;
  this->nb_dim_ = 1;
  init_dimensions(dimensions_);
  dimensions_[0] = dimension_tot_0_ = size_reelle_;
  assert(verifie_LINE_SIZE());
#endif
  return is;
}


template<typename _TYPE_, typename _SIZE_>
inline _TYPE_& TRUSTTab<_TYPE_,_SIZE_>::operator()(_SIZE_ i)
{
  return TRUSTArray<_TYPE_,_SIZE_>::operator[](i);
}

template<typename _TYPE_, typename _SIZE_>
inline const _TYPE_& TRUSTTab<_TYPE_,_SIZE_>::operator()(_SIZE_ i) const
{
  return TRUSTArray<_TYPE_,_SIZE_>::operator[](i);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-overflow"
template<typename _TYPE_, typename _SIZE_>
inline _TYPE_& TRUSTTab<_TYPE_,_SIZE_>::operator()(_SIZE_ i1, int i2)
{
  assert(this->nb_dim_ == 2);
  assert(i1 >= 0 && i1 < dimension_tot_0_);
  assert(i2 >= 0 && i2 < dimensions_[1]);
  return TRUSTArray<_TYPE_,_SIZE_>::operator[](i1*dimensions_[1]+i2);
}

template<typename _TYPE_, typename _SIZE_>
inline const _TYPE_& TRUSTTab<_TYPE_,_SIZE_>::operator()(_SIZE_ i1, int i2) const
{
  assert(this->nb_dim_ == 2);
  assert(i1 >= 0 && i1 < dimension_tot_0_);
  assert(i2 >= 0 && i2 < dimensions_[1]);
  return TRUSTArray<_TYPE_,_SIZE_>::operator[](i1*dimensions_[1]+i2);
}

template<typename _TYPE_, typename _SIZE_>
inline _TYPE_& TRUSTTab<_TYPE_,_SIZE_>::operator()(_SIZE_ i1, int i2, int i3)
{
  assert(this->nb_dim_ == 3);
  assert(i1 >= 0 && i1 < dimension_tot_0_);
  assert(i2 >= 0 && i2 < dimensions_[1]);
  assert(i3 >= 0 && i3 < dimensions_[2]);
  return TRUSTArray<_TYPE_,_SIZE_>::operator[]((i1*dimensions_[1]+i2)*dimensions_[2]+i3);
}

template<typename _TYPE_, typename _SIZE_>
inline const _TYPE_& TRUSTTab<_TYPE_,_SIZE_>::operator()(_SIZE_ i1, int i2, int i3) const
{
  assert(this->nb_dim_ == 3);
  assert(i1 >= 0 && i1 < dimension_tot_0_);
  assert(i2 >= 0 && i2 < dimensions_[1]);
  assert(i3 >= 0 && i3 < dimensions_[2]);
  return TRUSTArray<_TYPE_,_SIZE_>::operator[]((i1*dimensions_[1]+i2)*dimensions_[2]+i3);
}

template<typename _TYPE_, typename _SIZE_>
inline _TYPE_& TRUSTTab<_TYPE_,_SIZE_>::operator()(_SIZE_ i1, int i2, int i3, int i4)
{
  assert(this->nb_dim_ == 4);
  assert(i1 >= 0 && i1 < dimension_tot_0_);
  assert(i2 >= 0 && i2 < dimensions_[1]);
  assert(i3 >= 0 && i3 < dimensions_[2]);
  assert(i4 >= 0 && i4 < dimensions_[3]);
  return TRUSTArray<_TYPE_,_SIZE_>::operator[](((i1*dimensions_[1]+i2)*dimensions_[2]+i3)*dimensions_[3]+i4);
}

template<typename _TYPE_, typename _SIZE_>
inline const _TYPE_& TRUSTTab<_TYPE_,_SIZE_>::operator()(_SIZE_ i1, int i2, int i3, int i4) const
{
  assert(this->nb_dim_ == 4);
  assert(i1 >= 0 && i1 < dimension_tot_0_);
  assert(i2 >= 0 && i2 < dimensions_[1]);
  assert(i3 >= 0 && i3 < dimensions_[2]);
  assert(i4 >= 0 && i4 < dimensions_[3]);
  return TRUSTArray<_TYPE_,_SIZE_>::operator[](((i1*dimensions_[1]+i2)*dimensions_[2]+i3)*dimensions_[3]+i4);
}
#pragma GCC diagnostic pop

/*!  Returns one of the "real" dimensions of the multi-dimensionnal array, as defined by:
 *   dimension(0) = size_reelle() / line_size(), or 0 if line_size()==0
 *  and, for i >= 1 : dimension(i) is equal to dimension_tot(i)
 *  If size_reelle_ok() returns 0, it is invalid to ask for dimension(0). You can only ask for dimension_tot(0) (see size_reelle_ok())
 *
 *  In 64 bits, dimensions higher than 1 can always safely be casted down to an int, only the first dimension might be big.
 *  To help with this, the _RET_TYPE_ parameter can be used. One can write:
 *        int d1 = toto.dimension<int>(1);
 *  which is cleaner than doing a wild cast like
 *        int d1 = (int)toto.dimension(1);
 *  and it will also check for potential overflow (with an assert).
 *  This type of pattern is used in the 64b part of the code (before Scatter) when retreiving higher dimensions of arrays.
 *  See arch.h.in for more explanations on 64b.
 */
template<typename _TYPE_, typename _SIZE_>
inline _SIZE_ TRUSTTab<_TYPE_,_SIZE_>::dimension(int i) const
{
  assert(i >= 0 && i < this->nb_dim_);
  // Si dimension_[0] == -1, c'est que c'est un vecteur distribue et que l'attribut size() est invalide. Il faut alors utiliser dimension_tot pour ce tableau.
  assert(dimensions_[i] >= 0);
  assert(i == 0 || dimensions_[i] < std::numeric_limits<int>::max());
  return dimensions_[i];
}

/*! In 64 bits, dimensions higher than 1 can always safely be casted down to an int, only the first dimension might be big.
*  To help with this, this method can be used. One can write:
*        int d1 = toto.dimension_int(1);
*  which is cleaner than doing a wild cast like
*        int d1 = (int)toto.dimension(1);
*  and it will also check for potential overflow (with an assert).
*  This type of pattern is used in the 64b part of the code (before Scatter) when retrieving higher dimensions of arrays.
*  See arch.h.in for more explanations on 64b.
*/
template<typename _TYPE_, typename _SIZE_>
inline int TRUSTTab<_TYPE_,_SIZE_>::dimension_int(int i) const
{
  assert(i > 0);
  return static_cast<int>(dimension(i));  // overflow check down above
}

//  Returns the total dimensions of the multi-dimensionnal array, including virtual items (used in parallel distributed arrays)
template<typename _TYPE_, typename _SIZE_>
inline _SIZE_ TRUSTTab<_TYPE_,_SIZE_>::dimension_tot(int i) const
{
  assert(i >= 0 && i < this->nb_dim_);
  return (i == 0) ? dimension_tot_0_ : dimensions_[i];
}

/*!
 * See doc in TRUSTArray. This one also deals with multi-dim.
 */
template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::from_tid_to_int(TRUSTTab<int, int>& out) const
{
  // Manage multi-dim structure - I could not copy members directly because TRUSTTab<tid,tid>
  // can't see internals of TRUSTTab<int,int> and I could not get the 'friend' clause to work there ...
  ArrOfInt sizes;
  sizes.resize_array(this->nb_dim_);
  for (int i = 0; i < this->nb_dim_; i++)
    {
      assert(dimensions_[i] < std::numeric_limits<int>::max());
      sizes[i] = (int)dimensions_[i];
    }
  out.resize(sizes);

  if (md_vector_.non_nul())
    Process::exit("ERROR: from_tid_to_int() not implemented for parallel vect/tab!!");
  out.line_size_ = line_size_;
  TRUSTArray<_TYPE_,_SIZE_>::from_tid_to_int(out);
}

template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::ref_as_big(TRUSTTab<_TYPE_, trustIdType>& out) const
{
  out.nb_dim_ = this->nb_dim_;
  out.dimension_tot_0_ = dimension_tot_0_;
  for(int d=0; d<out.nb_dim_; d++)
    out.dimensions_[d] = dimensions_[d];
  if (md_vector_.non_nul())
    Process::exit("ERROR: ref_as_big() not implemented for parallel vect/tab!!");
  out.line_size_ = line_size_;
  TRUSTArray<_TYPE_,_SIZE_>::ref_as_big(out);
}

template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::ref_as_small(TRUSTTab<_TYPE_, int>& out) const
{
  out.nb_dim_ = this->nb_dim_;
  if(dimension_tot_0_ > std::numeric_limits<int>::max() || dimensions_[0] > std::numeric_limits<int>::max())
    Process::exit("TRUSTTab<_TYPE_,_SIZE_>::ref_as_small() - initial array is too big to be referenced as 32b-long array!");
  out.dimension_tot_0_ = static_cast<int>(dimension_tot_0_);
  for(int d=0; d<out.nb_dim_; d++)
    out.dimensions_[d] = static_cast<int>(dimensions_[d]);
  if (md_vector_.non_nul())
    Process::exit("ERROR: ref_as_small() not implemented for parallel vect/tab!!");
  out.line_size_ = line_size_;
  TRUSTArray<_TYPE_,_SIZE_>::ref_as_small(out);
}


/*! @brief Adds 1 to dimension_tot(0) and puts the given values in the added line.
 *
 * @param vals the 1 to 4 values of the new line, each converted to _TYPE_.
 *
 * Precondition: line_size() must be equal to the number of values and the array must be resizable.
 */
template<typename _TYPE_, typename _SIZE_>
template<typename ...Args>
inline void TRUSTTab<_TYPE_,_SIZE_>::append_line(Args... vals)
{
  constexpr int N = sizeof...(Args);
  static_assert(N >= 1 && N <= 4, "append_line takes 1 to 4 values");
  this->ensureDataOnHost();
  assert( (this->line_size() == N) );
  assert( (dimension_tot_0_ * this->line_size() == this->size_array()) );
  const _SIZE_ n = dimension_tot_0_ * N;
  resize_dim0(dimension_tot_0_ + 1, RESIZE_OPTIONS::COPY_NOINIT);
  _TYPE_ * ptr = this->addr() + n;
  int i = 0;
  ((ptr[i++] = static_cast<_TYPE_>(vals)), ...);
}

//  fait pointer le tableau sur le tableau t en recuperant la structure parallele. Attention, on fige le tableau qui ne pourra plus etre resize
template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::ref(const TRUSTTab& src)
{
  if (&src == this)
    return;
  TRUSTArray<_TYPE_,_SIZE_>::detach_array();
  TRUSTArray<_TYPE_,_SIZE_>::attach_array(src);
  md_vector_ = src.md_vector_;
  size_reelle_ = src.size_reelle_;
  line_size_ = src.line_size_;
  this->nb_dim_ = src.nb_dim_;
  for (int i = 0; i < MAXDIM_TAB; i++) dimensions_[i] = src.dimensions_[i];
  dimension_tot_0_ = src.dimension_tot_0_;
  assert(verifie_LINE_SIZE());
}

/*! See TRUSTArray::ref_data().
 * Create a one-dimensional array without any parallel structure.
 */
template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::ref_data(_TYPE_* ptr, _SIZE_ new_size)
{
  md_vector_.detach();
  TRUSTArray<_TYPE_,_SIZE_>::ref_data(ptr, new_size);
  size_reelle_ = new_size;
  line_size_ = 1;
  if (new_size<0) new_size=-new_size;
  assert( (!this->get_md_vector().non_nul() && this->size_reelle() == this->size_array()) );
  this->nb_dim_ = 1;
  dimensions_[0] = dimension_tot_0_ = new_size;
  assert(verifie_LINE_SIZE());
}

//  voir ArrOfDouble/Int::ref_array() (cree un tableau monodimensionnel sans structure parallele)
//  Attention, le tableau source et destination sont figes (resize interdit)
template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::ref_array(TRUSTArray<_TYPE_,_SIZE_>& src, _SIZE_ start, _SIZE_ sz)
{
  md_vector_.detach();
  TRUSTArray<_TYPE_,_SIZE_>::ref_array(src, start, sz);
  size_reelle_ = this->size_array(); // pas size qui peut valoir -1
  line_size_ = 1;
  assert( (!this->get_md_vector().non_nul() && this->size_reelle() == this->size_array()) );
  this->nb_dim_ = 1;
  dimensions_[0] = dimension_tot_0_ = this->size_array(); // pas sz qui peut valoir -1
  assert(verifie_LINE_SIZE());
}

//  fait pointer le tableau sur une sous-partie du tableau t definie par la valeur du premier indice et ne nombre de "lignes" du tableau
//   a recuperer (une ligne = toutes les valeurs tab(i,j,k,...) pour un i donne). Le nombre de dimensions du tableau est le meme que pour t,
//   les dimension(i) pour i>=1 sont les memes et dimension(0) = nb_lines.
template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::ref_tab(TRUSTTab& t, _SIZE_ start_line, _SIZE_ nb_lines)
{
  if (nb_lines < 0) nb_lines = t.dimension_tot_0_ - start_line;
  assert(start_line >= 0 && nb_lines >= 0 && start_line + nb_lines <= t.dimension_tot_0_);
  const int l_size = t.line_size();
  md_vector_.detach();
  TRUSTArray<_TYPE_,_SIZE_>::ref_array(t, start_line * l_size, nb_lines * l_size);
  size_reelle_ = this->size_array();
  line_size_ = 1;
  assert( (!this->get_md_vector().non_nul() && this->size_reelle() == this->size_array()) );
  this->set_line_size_(l_size);
  this->nb_dim_ = t.nb_dim_;
  dimension_tot_0_ = nb_lines;
  dimensions_[0] = nb_lines;
  for (int i = 1; i < MAXDIM_TAB; i++) dimensions_[i] = t.dimensions_[i];
  assert(verifie_LINE_SIZE());
}

/*! @brief Resets the object to the state obtained by the default constructor: an empty one-dimensional array without parallel structure.
 *
 */
template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::reset()
{
  this->nb_dim_ = 1;
  dimensions_[0] = 0;
  dimension_tot_0_ = 0;
  md_vector_.detach();
  line_size_ = 1;
  size_reelle_ = 0;
  TRUSTArray<_TYPE_,_SIZE_>::reset();
  assert(verifie_LINE_SIZE());
}

//  methode virtuelle qui force le tableau a changer de taille. Change aussi this->nb_dim_ a 1. Equivalent a TRUSTTab::resize(n, opt)
template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::resize_tab(_SIZE_ n, RESIZE_OPTIONS opt)
{
  resize(n, opt);
  assert(verifie_LINE_SIZE());
}

// redimensionne les dimensions d'un tableau tout en gardant le meme espace memoire.
template<typename _TYPE_, typename _SIZE_>
template<typename ...Args>
inline void TRUSTTab<_TYPE_,_SIZE_>::reshape(_SIZE_ n1, Args ...args)
{
  static_assert( (true & ... & (std::is_same<Args,int>::value) ) ); //All arguments beyond n1 are int.

  int constexpr nb_dims = sizeof...(Args)+1;
  static_assert(nb_dims<=MAXDIM_TAB);

  assert( (!this->get_md_vector().non_nul() && this->size_reelle() == this->size_array()) );
  assert( (n1>=0) && (true && ... && (args>=0) ));
  assert( (this->size_array() == (n1*...*args)) );
  this->set_line_size_((1*...*args));//All but n1

  init_dimensions(dimensions_);

  dimensions_[0] = dimension_tot_0_ = n1;
  int k=1;

  //We define collect, that look at each argument of arg and updates dimensions_ accordingly
  [[maybe_unused]] auto collect = [&](auto nk)
  {
    dimensions_[k++] = nk;
  };

  //Apply collect to each arg
  (collect(args), ...);

  this->nb_dim_ = nb_dims;

  assert(verifie_LINE_SIZE());
}

template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::promote_scalar_to_dim2()
{
  // Non-destructive reinterpretation of a scalar tab so downstream code always sees (n,1)
  if (this->nb_dim_ == 2 && dimensions_[1] == 1) return;

  assert(this->nb_dim_ == 1);
  assert((this->line_size() == 1));

  this->nb_dim_ = 2;
  dimensions_[1] = 1;

  assert(verifie_LINE_SIZE());
}


/*! @brief Changes dimension(0) of the array, keeping the other dimensions.
 *
 * @param n the new dimension(0) (and dimension_tot(0)).
 * @param opt the resize options, see TRUSTTab::resize_vect_().
 *
 * Precondition: the array must not have a parallel structure.
 */
template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::resize_dim0(_SIZE_ n, RESIZE_OPTIONS opt)
{
  assert(n >= 0);
  assert( (dimension_tot_0_ * this->line_size() == this->size_array()) );
  verifie_MAXDIM_TAB();
  switch(this->nb_dim_)
    {
    case 1:
      resize(n, opt);
      break;
    case 2:
      resize(n, dimensions_[1], opt);
      break;
    case 3:
      resize(n, dimensions_[1], dimensions_[2], opt);
      break;
    default:
      resize(n, dimensions_[1], dimensions_[2], dimensions_[3], opt);
    }
}

/*! @brief Resizes the array to dimensions (n, args...).
 *
 * nb_dim() becomes 1 + the number of extra dimensions and line_size() becomes their product.
 * Usage: resize(n), resize(n, n2), resize(n, n2, n3), resize(n, n2, n3, n4), each optionally followed by a RESIZE_OPTIONS.
 *
 * @param n the new dimension(0) (and dimension_tot(0)).
 * @param args up to 3 extra dimensions (converted to int), optionally followed by a RESIZE_OPTIONS (default COPY_INIT).
 *
 * Precondition: same as TRUSTTab::resize_vect_().
 */
template<typename _TYPE_, typename _SIZE_>
template<typename ...Args>
inline void TRUSTTab<_TYPE_,_SIZE_>::resize(_SIZE_ n, Args ...args)
{
  //We count the number of opt, and ensure it's 0 or 1
  //0 + ... f(Args) = sum of f for each Arg. Here the number of arguments that are RESIZE_OPTIONS. There should be one at most
  constexpr int nb_opt = (0 + ... + (int)std::is_same<Args, RESIZE_OPTIONS>::value);
  static_assert((nb_opt==1)||(nb_opt==0), "Only one opt argument at most !");

  //We count the number of dimensions, and ensure it's <= MAXDIM_TAB.
  //sizeof...(Args): number of arguments in Args. - nb_opt +1 = number of dimensions. +1 is for n.
  constexpr int nb_dims = (int)sizeof...(Args) - nb_opt+1;
  static_assert(nb_dims <= MAXDIM_TAB, "too many dimensions");

  //If there is one, it must be the last.
  if constexpr(nb_opt==1)
    {
      //tuple_element_t<i, tuple>: type of the ith element of a tuple
      using last_t = std::tuple_element_t<nb_dims, std::tuple<int, Args...>>;
      static_assert(std::is_same<last_t, RESIZE_OPTIONS>::value, "The opt argument MUST be the last");
    }

  //default choice
  RESIZE_OPTIONS opt = RESIZE_OPTIONS::COPY_INIT;
  int dims[MAXDIM_TAB - 1] = {};
  int nd = 0;

  //We define collect, that look at each argument of argm and updates opt and dims accordingly
  [[maybe_unused]] auto collect = [&](auto a)
  {
    //if it's a opt, we overwrite the default opt
    //Else, we it's a dimension. We add it to our dims array.
    if constexpr (std::is_same<decltype(a), RESIZE_OPTIONS>::value) opt = a;
    else dims[nd++] = static_cast<int>(a);
  };
  //Apply collect to each arg
  (collect(args), ...);

  assert(n >= 0);
  int l_size = 1;
  for (int i = 0; i < nd; i++)
    {
      assert(dims[i] >= 0);
      l_size *= dims[i];
    }
  this->set_line_size_(l_size);
  _SIZE_ new_size = n * l_size;

  //32 bits check
  if (std::is_same<_SIZE_,int>::value && new_size < 0)
    {
      Cerr << "n1";
      for (int i = 0; i < nd; i++) Cerr << "*n" << i + 2;
      Cerr << " > 2^31. Error! Contact TRUST support, integer 32 bits limit exceeded with n1=" << n;
      for (int i = 0; i < nd; i++) Cerr << " and n" << i + 2 << "=" << dims[i];
      Cerr << finl;
      Process::exit();
    }

  //Lower level resize.
  this->resize_vect_(new_size, opt);
  //nd does not take n into account.
  this->nb_dim_ = nd + 1;
  dimensions_[0] = dimension_tot_0_ = n;
  for (int i = 0; i < nd; i++) dimensions_[i + 1] = dims[i];
  assert(verifie_LINE_SIZE());
}

/*! @brief Resizes the array: nb_dim() becomes tailles.size_array() and dimension(i) becomes tailles[i].
 *
 * @param tailles the new dimensions (1 to 4 non-negative values).
 * @param opt the resize options, see TRUSTTab::resize_vect_().
 *
 * Precondition: same as TRUSTTab::resize_vect_().
 */
template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::resize(const TRUSTArray<_SIZE_, int>& tailles, RESIZE_OPTIONS opt)
{
  const int nb_dim = tailles.size_array();
  if (nb_dim <= 0 || nb_dim > MAXDIM_TAB)
    {
      Cerr << "Internal error in TRUSTTab::resize(const ArrOfInt & tailles, ...) \n" << " wrong dimensions number " << nb_dim << finl;
      Process::exit();
    }
  for (int i = 0; i < nb_dim; i++)
    if (tailles[i] < 0)
      {
        Cerr << "Internal error in TRUSTTab::resize(const ArrOfInt & tailles, ...) \n";
#ifndef LATATOOLS
        Cerr << " wrong dimensions: " << tailles << finl;
#endif
        Process::exit();
      }
  switch(nb_dim)
    {
    case 1:
      resize(tailles[0], opt);
      break;
    case 2:
      resize(tailles[0], tailles[1], opt);
      break;
    case 3:
      resize(tailles[0], tailles[1], tailles[2], opt);
      break;
    default:
      resize(tailles[0], tailles[1], tailles[2], tailles[3], opt);
    }
}

//  copie la structure et les valeurs du tableau src
//  (attention, si le tableau est de type ref_data ou ref, restrictions et cas particuliers !!!)
//  Attention: si on ne veut pas copier les structures paralleles, utiliser inject_array()
//  Si le tableau *this a deja une structure parallele, l'appel n'est autorise que les md_vector
//  sont deja identiques, sinon il faut d'abord faire un reset() du tableau (pour copier la structure), ou utiliser inject_array() (pour ne pas copier la structure).
//  (ceci pour eviter d'ecraser accidentellement une structure parallele alors qu'on ne veut que copier les valeurs).
template<typename _TYPE_, typename _SIZE_>
inline TRUSTTab<_TYPE_,_SIZE_>& TRUSTTab<_TYPE_,_SIZE_>::operator=(const TRUSTTab& src)
{
  copy(src);
  return *this;
}

template<typename _TYPE_, typename _SIZE_>
inline TRUSTTab<_TYPE_,_SIZE_>& TRUSTTab<_TYPE_,_SIZE_>::operator=(_TYPE_ d)
{
  TRUSTArray<_TYPE_,_SIZE_>::operator=(d);
  return *this;
}

template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::copy(const TRUSTTab& src, RESIZE_OPTIONS opt)
{
  if (&src != this)
    {
      this->copy_(src, opt);
      this->nb_dim_ = src.nb_dim_;
      for (int i = 0; i < MAXDIM_TAB; i++) dimensions_[i] = src.dimensions_[i];
      dimension_tot_0_ = src.dimension_tot_0_;
      assert(verifie_LINE_SIZE());
    }
}

template<typename _TYPE_, typename _SIZE_>
inline _TYPE_& TRUSTTab<_TYPE_,_SIZE_>::operator()(const TRUSTArray<_SIZE_,int>& indice)
{
  assert(indice.size_array() == this->nb_dim_);
  verifie_MAXDIM_TAB();
  switch(this->nb_dim_)
    {
    case 1:
      return operator()(indice[0]);
    case 2:
      return operator()(indice[0], indice[1]);
    case 3:
      return operator()(indice[0], indice[1], indice[2]);
    default:
      return operator()(indice[0], indice[1], indice[2], indice[3]);
    }
}

template<typename _TYPE_, typename _SIZE_>
inline _TYPE_ TRUSTTab<_TYPE_,_SIZE_>::operator()(const TRUSTArray<_SIZE_,int>& indice) const
{
  assert(indice.size_array() == this->nb_dim_);
  verifie_MAXDIM_TAB();
  switch(this->nb_dim_)
    {
    case 1:
      return operator()(indice[0]);
    case 2:
      return operator()(indice[0], indice[1]);
    case 3:
      return operator()(indice[0], indice[1], indice[2]);
    default:
      return operator()(indice[0], indice[1], indice[2], indice[3]);
    }
}

//  associe le md_vector au vecteur et initialise l'attribut size_reelle_ (voir methode size_reelle()). dimension(0) sera initialise a md_vector...get_nb_items_reels().
//  Si md_vector est nul, detache simplement le md_vector existant.
// Precondition: le vecteur doit deja avoir la taille appropriee au nouveau md_vector, c'est a dire md_vector...get_nb_items_tot() * line_size_,
//  et dimension_tot(0) doit etre egal a get_nb_items_tot() du md_vector.
template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::set_md_vector(const MD_Vector& md_vector)
{
#ifndef LATATOOLS
  _SIZE_ dim0 = dimension_tot_0_;
  if (md_vector.non_nul())
    {
      if (sub_type(MD_Vector_seq, md_vector.valeur()))
        dim0 = (_SIZE_)md_vector->nb_items_seq_tot();
      else
        // renvoie -1 si l'appel est invalide ou si le MD_Vector est mix (cf doc MD_Vector_base):
        dim0 = md_vector->get_nb_items_reels();
    }
  dimensions_[0] = dim0;
  assert(verifie_LINE_SIZE());

  _SIZE_ size_r = this->size_array();
  if (md_vector.non_nul())
    {
      size_r = md_vector->get_nb_items_reels();
      if (size_r >= 0) size_r *= line_size_;
      else size_r = -1; // Cas particulier ou la size_reelle ne veut rien dire

      // Will throw if there is a problem:
      md_vector->validate(this->size_array(), line_size_);
    }
  size_reelle_ = size_r;
  md_vector_ = md_vector;
#endif
}

template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::ecrit(Sortie& os) const
{
#ifndef LATATOOLS
  os << this->nb_dim_ << finl;
  if (this->nb_dim_ > 0) os.put(dimensions_, this->nb_dim_, this->nb_dim_);
  TRUSTArray<_SIZE_,int> tmp(this->nb_dim_);
  for (int i = 0; i < this->nb_dim_; i++) tmp[i] = dimension_tot(i);
  os << tmp;
  ecrit_values_(os);
#endif
}

//  ecriture des valeurs du tableau "raw" sans structure parallele
template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::ecrit_values_(Sortie& os) const
{
#ifndef LATATOOLS
  TRUSTArray<_TYPE_,_SIZE_>::printOn(os);
  os << (int)-1 << finl; // le marqueur -1 indique que c'est le nouveau format "ecrit", sans structure parallele
#endif
}

template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::jump(Entree& is)
{
  TRUSTTab<_TYPE_,_SIZE_>::lit(is, 0 /* Do not resize&read the array */);
}

//  lecture d'un tableau pour reprise de calcul. On lit les valeurs "raw".
//  Attention, si le tableau n'est pas vide, il doit deja avoir la bonne taille et la bonne structure, sinon erreur !
// Parameter resize_and_read if the array is sized AND read (by default, yes)
template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::lit(Entree& is, bool resize_and_read)
{
#ifndef LATATOOLS
  TRUSTArray<_SIZE_,int> tmp; // the dim array wich is read
  is >> tmp;
  bool ok = (tmp.size_array() == this->nb_dim_);

  if (ok)
    {
      if (this->size_reelle_ok() && dimension(0) != tmp[0]) ok = 0;
      for (int i = 1; i < this->nb_dim_; i++)
        if (dimension(i) != tmp[i]) ok = 0;
    }
  is >> tmp;
  if (ok && tmp.size_array() != this->nb_dim_) ok = 0;
  if (ok)
    for (int i = 0; i < this->nb_dim_; i++)
      if (dimension_tot(i) != tmp[i]) ok = 0;

  // Autorisation ancien format des champs scalaire 183:
  if (tmp.size_array()==1 && this->nb_dim_==2 && dimension(1)==1 && dimension_tot(0) == tmp[0]) ok = 1;

  if (resize_and_read)
    {
      if (this->size_array() == 0 && (!this->get_md_vector().non_nul()))
        resize(tmp, RESIZE_OPTIONS::NOCOPY_NOINIT);
      else if (!ok)
        {
          // Si on cherche a relire un tableau de taille inconnue, le tableau doit etre reset() a l'entree. On n'aura pas la structure parallele du tableau !
          Cerr << finl;
          Cerr << "Error in DoubleTab::lit: array has wrong dimensions" << finl;
          if (Process::is_parallel() && dimension(0) != tmp[0]) // Add message to detect different partitionning
            {
              Cerr << "We try to read an array with " << dimension(0) << " items whereas we are waiting for a size of " << tmp[0] << "!" << finl;
              Cerr << "Probably a different partitionning from your previous calculation..." << finl;
              Cerr << "Change your current partitionning to match the previous calculation or try .xyz restart protocol." << finl;
            }
          Process::exit();
        }
    }
  lit_values_(is, resize_and_read);
#endif
}

//  lecture des valeurs "raw" du tableau, ecrites par ecrit_values_()
template<typename _TYPE_, typename _SIZE_>
inline void TRUSTTab<_TYPE_,_SIZE_>::lit_values_(Entree& is, bool resize_and_read)
{
#ifndef LATATOOLS
  _SIZE_ sz = -1;
  is >> sz;
  if (resize_and_read)
    {
      if (this->size_array() == 0 && (!md_vector_.non_nul()))
        resize_vect_(sz, RESIZE_OPTIONS::NOCOPY_NOINIT);
      else if (sz != this->size_array())
        {
          // Si on cherche a relire un tableau de taille inconnue, le tableau doit etre reset() a l'entree. On n'aura pas la structure parallele du tableau !
          Cerr << "Error in TRUSTTab::lit(Entree & is): array has already a structure with incorrect size" << finl;
          Process::exit();
        }
      is.get(this->addr(), sz);
    }
  else
    {
      // May be slow if large chunks are read:
      // Double tmp;
      //for (_SIZE_ i=0;i<sz;i++) is >> tmp;
      // So we bufferize:
      const _SIZE_ sz_max = 128000;
      _SIZE_ buffer_size = std::min(sz,sz_max);
      TRUSTArray<_TYPE_,_SIZE_> tmp(buffer_size);
      while(sz>buffer_size)
        {
          is.get(tmp.addr(), buffer_size);
          sz-=buffer_size;
        }
      is.get(tmp.addr(), sz);
    }
  _SIZE_ sz_reel = -2;
  is >> sz_reel;
  if (sz_reel >= 0)
    {
      // Lecture de l'ancien format. Ignore les valeurs lues.
      _SIZE_ sz_virt;
      is >> sz_virt;
      DescStructure toto;
      is >> toto;
      if (!std::is_same<_TYPE_,int>::value) /* double ou float */
        {
          TRUSTArray<int,_SIZE_> it_communs;
          is >> it_communs;
          TRUSTArray<int,_SIZE_> it_communs_tot;
          is >> it_communs_tot;
        }
    }
#endif
}

// ------------------------------------------------------
// Juste pour double/float

//  Quelqu'un veut-il expliquer ce que fait cette methode ?
template<typename _TYPE_, typename _SIZE_>
template <typename _T_>
inline void TRUSTTab<_TYPE_,_SIZE_>::ajoute_produit_tensoriel(_T_ alpha, const TRUSTTab<_T_,_SIZE_>& x, const TRUSTTab<_T_,_SIZE_>& y)
{
  // Tableaux vus comme des tableaux unidimensionnels (pour ne pas avoir a gerer nb_dim)
  const TRUSTTab<_T_,_SIZE_>& vx = x, &vy = y;
  TRUSTTab<_T_,_SIZE_>& v = *this;

  const int line_size_x = vx.line_size(), line_size_y = vy.line_size(), line_size_xy = v.line_size();
  assert(line_size_xy == line_size_x * line_size_y);
  // Pour ne pas diviser par line_size()
  assert(vx.size_totale() * line_size_xy == v.size_totale() * line_size_x);
  assert(vy.size_totale() * line_size_xy == v.size_totale() * line_size_y);

  // Logic similar to what is done in TRUSTTab_tools.cpp, with ::determine_blocks()
  Block_Iter<_SIZE_> bloc_itr; // By default, nothing in the iterator
  int nblocs_left = 0;
  if (v.get_md_vector().non_nul() && v.get_md_vector()->use_blocks())
    {
      const ArrOfInt& items_blocs = v.get_md_vector()->get_blocs_items_to_compute();
      assert(items_blocs.size_array() % 2 == 0);
      nblocs_left = items_blocs.size_array() >> 1;
      bloc_itr = Block_Iter<_SIZE_>(items_blocs);
    }
  else
    {
      if (v.size_totale() > 0)
        {
          nblocs_left = 1;
          bloc_itr = Block_Iter<_SIZE_>(0, v.size_totale() / v.line_size());   // iterator on a single (big) block
        }
    }
  if (nblocs_left == 0) return;

  ::ajoute_produit_tensoriel(alpha, v, vx, vy, nblocs_left, bloc_itr, line_size_x, line_size_y, line_size_xy);
}

//  Resolution du systeme Ax=b
template<typename _TYPE_, typename _SIZE_>
template <typename _T_>
inline bool TRUSTTab<_TYPE_,_SIZE_>::inverse_LU(const TRUSTArray<_T_,_SIZE_>& b, TRUSTArray<_T_,_SIZE_>& solution)
{
  b.ensureDataOnHost();
  solution.ensureDataOnHost();
  _SIZE_ n = b.size_array();
  TRUSTArray<int,_SIZE_> index(n);
  TRUSTTab<_T_,_SIZE_> lu_dec(n,n);
  bool cvg = (*this).decomp_LU(n,index,lu_dec);

  if(cvg) lu_dec.resoud_LU(n,index,b,solution);

  return cvg;
}

//  Decomposition d'une matrice en L.U: methode de Crout (diagonale de L =1)
// Retour: matrice A_ = assemblage (L-diagonale)+U
template<typename _TYPE_, typename _SIZE_>
template <typename _T_>
inline bool TRUSTTab<_TYPE_,_SIZE_>::decomp_LU(_SIZE_ n, TRUSTArray<int,_SIZE_>& index, TRUSTTab<_T_,_SIZE_>& matLU)
{
  TRUSTArray<_T_,_SIZE_> vv(n);
  _SIZE_ i, j, k,  imax = -1, cvg = 1;
  _T_ big, dum, sum, temp;
  matLU = (*this);

  //Recupere le coeff max d'une ligne, stocke dans vv
  for (i=0 ; i<n ; i++)
    {
      big = 0;
      for (j=0 ; j<n ; j++)
        if ((temp = std::fabs(matLU(i,j))) > big) big = temp;

      if (big == 0)
        {
          Cerr <<"Singular matrix in LU decomposition"<<finl;
          cvg = 0;
          Process::exit();
        }
      vv[i] = 1./big;
    }

  //calcul de la matrice matLU
  for (j=0 ; j<n ; j++)
    {
      for (i=0 ; i<j ; i++)
        {
          sum = matLU(i,j);
          for (k=0 ; k<i ; k++) sum -= matLU(i,k) * matLU(k,j);
          matLU(i,j) = sum;
        }

      big = 0;
      for (i=j ; i<n ; i++)
        {
          sum = matLU(i,j);
          for (k=0 ; k<j ; k++) sum -= matLU(i,k) * matLU(k,j);
          matLU(i,j) = sum;
          if ((dum = vv[i]*std::fabs(sum)) >= big)
            {
              big = dum;
              imax = i;
            }
        }

      if (j != imax)
        {
          for (k=0 ; k<n ; k++)
            {
              dum = matLU(imax,k);
              matLU(imax,k) = matLU(j,k);
              matLU(j,k) = dum;
            }
          vv[imax] = vv[j];
        }

      index[j] = imax;
      dum = 1./matLU(j,j);
      for (i=j+1 ; i<n ; i++) matLU(i,j) *= dum;
    }
  return cvg;
}

//  Resolution du systeme A_x=b : A_ contenant le decompostion LU de A (stockee dans une seule matrice)
template<typename _TYPE_, typename _SIZE_>
template <typename _T_>
inline void TRUSTTab<_TYPE_,_SIZE_>::resoud_LU(_SIZE_ n, TRUSTArray<int,_SIZE_>& index, const TRUSTArray<_T_,_SIZE_>& b, TRUSTArray<_T_,_SIZE_>& solution)
{
  _SIZE_ i,ii=-1,ip,j;
  _T_ sum;
  solution = b;
  for (i=0 ; i<n ; i++)
    {
      ip = index[i];
      sum = solution[ip];
      solution[ip] = solution[i];
      if (ii!=-1)
        for (j=ii ; j<i ; j++) sum -= (*this)(i,j)*solution[j];
      else if (sum) ii =i;

      solution[i] = sum;
    }

  for (i=n-1 ; i>=0 ; i--)
    {
      sum = solution[i];
      for (j=i+1 ; j<n ; j++) sum -= (*this)(i,j)*solution[j];
      solution[i] = sum/(*this)(i,i);
    }
}

//  Fonction utilisee pour le calcul du du/u (pour convergence implicite)
//    renvoie le max de abs(du(i)/u(i)). utilisation    max_ = (u(n+1)-u(n)).max_du_u(u(n))
template<typename _TYPE_, typename _SIZE_>
template <typename _T_>
inline _T_ TRUSTTab<_TYPE_,_SIZE_>::max_du_u(const TRUSTTab<_T_,_SIZE_>& u)
{
  u.ensureDataOnHost();
  assert( (this->size_array() == u.size_array()) );
  const _T_ *du_ptr = this->addr();
  const _T_ *u_ptr = u.addr();
  const _T_ epsilon = 1.e-8;
  _T_ res = 0.;
  for (_SIZE_ n = this->size_array(); n; n--)
    {
      _T_ a = std::fabs(*du_ptr), b = std::fabs(*u_ptr), c = a / (b + epsilon);
      if (b > 1.e-2 && c > res) res = c;
      du_ptr++;
      u_ptr++;
    }
  return res;
}

#endif /* TRUSTTab_TPP_included */
