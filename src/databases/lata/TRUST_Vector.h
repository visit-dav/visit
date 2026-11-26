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

#ifndef TRUST_Vector_included
#define TRUST_Vector_included

#include <type_traits>
#include <algorithm> // for std::transform
#include <vector>
#include <memory> // for std::shared_ptr
#include <Nom.h>
#include <Separateur.h>

class MD_Vector;

// MACRO to replace VECT(THECLASS) by TRUST_Vector<THECLASS> & keep previous syntax for some developers
#define VECT(_TYPE_) TRUST_Vector<_TYPE_>

/*! @brief TRUST_Vector class
 *
 *  - The template class TRUST_Vector can be used for any class.
 *
 *      Usage examples:
 *
 *        - TRUST_Vector<MD_Vector>
 *        - TRUST_Vector<Milieu_base>
 *        - TRUST_Vector<TRUSTArray<double>>
 */
template<typename _CLASSE_>
class TRUST_Vector: public Objet_U
{
  using value_type = _CLASSE_;
  using pointer_type = std::shared_ptr<_CLASSE_>;
  using STLVect = std::vector<pointer_type>; // vector of shared ptr to _CLASSE_
  using Iterator = typename STLVect::iterator;
  using Const_Iterator = typename STLVect::const_iterator;

  // XXX : Elie Saikali
  // We have an std vector of shared ptrs => iterators return pointers and not the passed value type
  // These classes allows a return of the value type
  class IteratorWrapper
  {
  public:
    explicit IteratorWrapper(Iterator it) : it_(it) { }
    value_type& operator*() { return *it_->get(); }
    value_type* operator->() { return it_->get(); }
    IteratorWrapper& operator++() { ++it_; return *this; }
    IteratorWrapper operator++(int) { return IteratorWrapper(it_++); }
    bool operator==(const IteratorWrapper& other) const { return it_ == other.it_; }
    bool operator!=(const IteratorWrapper& other) const { return it_ != other.it_; }

  private:
    Iterator it_;
  };

  class ConstIteratorWrapper
  {
  public:
    explicit ConstIteratorWrapper(Const_Iterator it) : it_(it) {}
    const value_type& operator*() const { return *it_->get(); }
    const value_type* operator->() const { return it_->get(); }
    ConstIteratorWrapper& operator++() { ++it_; return *this; }
    ConstIteratorWrapper operator++(int) { return ConstIteratorWrapper(it_++); }
    bool operator==(const ConstIteratorWrapper& other) const { return it_ == other.it_; }
    bool operator!=(const ConstIteratorWrapper& other) const { return it_ != other.it_; }

  private:
    Const_Iterator it_;
  };

protected:
  unsigned taille_memoire() const override { throw; }
  int duplique() const override { throw; }
#ifndef LATATOOLS
  Sortie& printOn(Sortie& s) const override { return printOn_<_CLASSE_>(s); }
  Entree& readOn(Entree& s) override { return readOn_<_CLASSE_>(s); }
#else
  Sortie& printOn(Sortie& s) const override { return s; }
  Entree& readOn(Entree& s) override { return s; }
#endif
private:
  STLVect z_vect_;

#ifndef LATATOOLS
  template<typename _TYPE_>
  std::enable_if_t< !(std::is_same<_TYPE_,MD_Vector>::value), Entree&>
  readOn_(Entree& s)
  {
    int i;
    s >> i;
    clear();
    _TYPE_ obj;
    for (int ind = 0; ind < i; ind++)
      {
        s >> obj;
        add(std::move(obj));
      }
    return s;
  }

  template<typename _TYPE_>
  std::enable_if_t< !(std::is_same<_TYPE_,MD_Vector>::value), Sortie&>
  printOn_(Sortie& s) const
  {
    s << (int) z_vect_.size() << tspace;
    for (auto &itr : z_vect_) s << *itr << tspace;
    s << finl;
    return s;
  }

  // MD_Vector class does not derive from Objet_U => no readOn & printOn
  template<typename _TYPE_>
  std::enable_if_t<(std::is_same<_TYPE_,MD_Vector>::value), Entree&>
  readOn_(Entree& s) { return s; }

  template<typename _TYPE_>
  std::enable_if_t<(std::is_same<_TYPE_,MD_Vector>::value), Sortie&>
  printOn_(Sortie& s) const { return s ; }
#else
  Sortie& printOn_(Sortie& s) const { return s; }
  Entree& readOn_(Entree& s) { return s; }
#endif
public:
  ~TRUST_Vector() { z_vect_.clear(); }

  TRUST_Vector() = default;

  explicit TRUST_Vector(int i) { dimensionner_force(i); } /* clear, resize to i & fill with i empty shared ptrs */

  // XXX : Elie Saikali
  // Nota Bene : Deep copy : should not use the same memory !! So dangerous attention !!
  TRUST_Vector(const TRUST_Vector& avect) : Objet_U(avect)
  {
    clear();
    for (int i = 0; i < avect.size(); i++)
      z_vect_.push_back(std::make_shared<_CLASSE_>(*avect.z_vect_[i]));
  }

  // get stl vector : attention its a vector of shared ptrs !!
  const STLVect& get_stl_vect() const { return z_vect_; }
  STLVect& get_stl_vect() { return z_vect_; }

  // iterators on TRUST_Vector
  IteratorWrapper begin() { return IteratorWrapper(z_vect_.begin()); }
  IteratorWrapper end() { return IteratorWrapper(z_vect_.end()); }
  const ConstIteratorWrapper begin() const { return ConstIteratorWrapper(z_vect_.begin()); }
  const ConstIteratorWrapper end() const { return ConstIteratorWrapper(z_vect_.end()); }

  /* Iterator begin() { return (z_vect_.begin()); }
  Iterator end() { return (z_vect_.end()); }
  const Const_Iterator begin() const { return (z_vect_.begin()); }
  const Const_Iterator end() const { return (z_vect_.end()); } */

  const value_type& operator[](int i) const { return *z_vect_[i]; }
  value_type& operator[](int i) { return *z_vect_[i]; }

  // XXX : Elie Saikali : No negotiation for this, sorry ...
  const value_type& operator()(int i) const = delete;
  value_type& operator()(int i) = delete;
  value_type& add() = delete; //{ return add(_CLASSE_()); }

  int size() const { return (int)z_vect_.size(); }
  void reset() { z_vect_.clear(); }
  void clear() { z_vect_.clear(); }
  void resize(int i) { dimensionner(i); }
  value_type& back() { return *(z_vect_.back()); }
  value_type& front() { return *(z_vect_.front()); }

  // XXX : Elie Saikali
  // Attention : it is so important to implement dimensionner and dimensionner_force as this otherwise issue with the standard = operator.
  // A pre-initialised shared ptr is required in each zone memory of the sized vector.
  void dimensionner(int i)
  {
    const int old_size = (int)z_vect_.size();
    if (old_size == i) return;

    if (old_size != 0)
      {
        Cerr << "WARNING : dimensionner method of a TRUST_Vector" << finl;
        Cerr << "Old vector size : " << old_size << finl;
        Cerr << "New vector size : " << i << finl;
      }

    z_vect_.resize(i);
    for (int j = old_size; j < i; j++)
      z_vect_[j] = std::make_shared<_CLASSE_>();
  }

  void dimensionner_force(int i)
  {
    z_vect_.clear();
    z_vect_.resize(i);
    for (auto& itr : z_vect_) itr = std::make_shared<_CLASSE_>();
  }
#ifndef LATATOOLS
  Entree& lit(Entree& s) { return readOn_<_CLASSE_>(s); }
#endif
  TRUST_Vector& operator=(const TRUST_Vector& avect)
  {
    if (this == &avect) return *this;

    clear();
    for (int i = 0; i < avect.size(); i++)
      z_vect_.push_back(std::make_shared<_CLASSE_>(*avect.z_vect_[i]));

    return *this;
  }

  /* Add a new element to the vect */
  value_type& add(value_type&& data_to_add)
  {
    z_vect_.emplace_back(std::make_shared<_CLASSE_>(std::forward<_CLASSE_>(data_to_add)));
    return *(z_vect_.back());
  }

  value_type& add(const value_type& data_to_add)
  {
    z_vect_.push_back(std::make_shared<_CLASSE_>(data_to_add));
    return *(z_vect_.back());
  }

  /* Append a vect to a vect */
  void add(const TRUST_Vector& v2)
  {
    STLVect tmp_;
    tmp_.reserve(v2.size());
    auto lambda_ = [](pointer_type const &iter) { return std::make_shared<_CLASSE_>(*iter); };
    std::transform(v2.get_stl_vect().begin(), v2.get_stl_vect().end(), std::back_inserter(tmp_), lambda_);
    z_vect_.insert(z_vect_.end(), std::make_move_iterator(tmp_.begin()), std::make_move_iterator(tmp_.end()));
  }
};

#endif /* TRUST_Vector_included */
