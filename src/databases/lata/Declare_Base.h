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


// .SECTION Description
// The macros Declare_base_sans_destructeur and
// Implemente_base_sans_destructeur
// allow simple creation of base classes
// conforming to TRUST.

// .SECTION Header description
// class A_base : public B {
// Declare_base_sans_destructeur(A_base);
// };
// .SECTION Source description
// Implemente_base_sans_destructeur(A_base, "A_base", B);

#include <arch.h>

#ifdef LATATOOLS
#define Declare_base_sans_constructeur_ni_destructeur_ni_readon(_TYPE_)  \
  public :                                                                \
  unsigned taille_memoire() const override { return 0; };                                      \
  int duplique() const override { return 0; };                                                \
  protected :                                                                \
  Sortie& printOn(Sortie& x) const override

#define Implemente_base_sans_constructeur_ni_destructeur(_TYPE_,_NOM_,_BASE_)

#else
#include <Cast.h>
#include <Type_info.h>
#define Declare_base_sans_constructeur_ni_destructeur_ni_readon(_TYPE_)        \
                                                                \
  public :                                                        \
  static const Type_info info_obj;                                \
  static const Type_info* info();                                \
  const Type_info* get_info() const override;                                \
  /* method added to cast in python */                        \
  static _TYPE_& self_cast( Objet_U&) ;                                \
  static const _TYPE_& self_cast(const Objet_U&) ;                \
  protected :                                                        \
  Sortie& printOn(Sortie& x) const override


#define Implemente_base_sans_constructeur_ni_destructeur(_TYPE_,_NOM_,_BASE_) \
                                                                           \
  static_assert(std::is_base_of<_BASE_, _TYPE_>::value, "\n\n!!!!! Error in Implemente_base* macro usage: type '" #_TYPE_ "' does not inherit from '" #_BASE_ "' !!!!\n\n"); \
                                                                           \
  const Type_info* name2(_TYPE_,bases)[1]={                                \
    &(_BASE_::info_obj)};                                                \
  const Type_info _TYPE_::info_obj(_NOM_, 1, name2(_TYPE_,bases));        \
                                                                        \
  _TYPE_& _TYPE_::self_cast( Objet_U& r)   {                                \
    return ref_cast_non_const(_TYPE_,r); /* _non_const important otherwise recursion in ref_cast */ \
  }                                                                        \
  const _TYPE_& _TYPE_::self_cast(const Objet_U& r)   {                        \
    return ref_cast_non_const(_TYPE_,r); /* _non_const important otherwise recursion in ref_cast */ \
  }                                                                        \
  const Type_info*  _TYPE_::get_info() const {                                \
    return &info_obj;                                                        \
  }                                                                        \
  const Type_info*  _TYPE_::info() {                                        \
    return &info_obj;                                                        \
  }                                                                          \
  class __dummy__
#endif

/////////////////////////////////////////
/////// Those are shared with LATATOOLS
/////////////////////////////////////////

#define Declare_base_sans_constructeur_ni_destructeur(_TYPE_)        \
  Declare_base_sans_constructeur_ni_destructeur_ni_readon(_TYPE_);   \
  Entree& readOn(Entree&) override

#define Declare_base_sans_constructeur(_TYPE_)                \
  public:                                                \
  ~_TYPE_() override;                                                \
  Declare_base_sans_constructeur_ni_destructeur(_TYPE_)

#define Declare_base_sans_destructeur(_TYPE_)                \
  public:                                                \
  _TYPE_();                                                \
  Declare_base_sans_constructeur_ni_destructeur(_TYPE_)

#define Declare_base(_TYPE_)                                \
  public:                                                \
  _TYPE_();                                                \
  ~_TYPE_() override;                                                \
  Declare_base_sans_constructeur_ni_destructeur(_TYPE_)

#define Declare_base_sans_readon(_TYPE_)                                \
  public:                                                \
  _TYPE_();                                                \
  ~_TYPE_() override;                                                \
  Declare_base_sans_constructeur_ni_destructeur_ni_readon(_TYPE_)

#define Declare_base_with_param(_TYPE_)                                \
  Declare_base_sans_readon(_TYPE_); \
  protected: \
  void set_param(Param&) const override

#define Implemente_base_sans_constructeur(_TYPE_,_NOM_,_BASE_)                \
  _TYPE_::~_TYPE_() { }                                                        \
  Implemente_base_sans_constructeur_ni_destructeur(_TYPE_,_NOM_,_BASE_)

#define Implemente_base_sans_destructeur(_TYPE_,_NOM_,_BASE_)                \
  _TYPE_::_TYPE_() { }                                                        \
  Implemente_base_sans_constructeur_ni_destructeur(_TYPE_,_NOM_,_BASE_)

#define Implemente_base(_TYPE_,_NOM_,_BASE_)                                \
  _TYPE_::_TYPE_() { }                                                        \
  _TYPE_::~_TYPE_() { }                                                        \
  Implemente_base_sans_constructeur_ni_destructeur(_TYPE_,_NOM_,_BASE_)


/////////////////////////////////////////
/////// TEMPLATE VERSION for 32/64 bits !!! Oh yes baby!
/////////////////////////////////////////

// The declare remain identical but we create a new macro so that bin/KSH/mk_Instanciable spots it correctly:
#define Declare_base_sans_constructeur_ni_destructeur_32_64(_TYPE_) Declare_base_sans_constructeur_ni_destructeur(_TYPE_>)
#define Declare_base_sans_constructeur_32_64(_TYPE_) Declare_base_sans_constructeur(_TYPE_)
#define Declare_base_sans_destructeur_32_64(_TYPE_) Declare_base_sans_destructeur(_TYPE_)
#define Declare_base_32_64(_TYPE_) Declare_base(_TYPE_)
#define Declare_base_sans_readon_32_64(_TYPE_) Declare_base_sans_readon(_TYPE_)
#define Declare_base_with_param_32_64(_TYPE_) Declare_base_with_param(_TYPE_)

// Helper macro for info_obj static variable definition:
#if INT_is_64_ == 2
# define info_obj_def_macro_base(_NOM_, _TYPE_) \
  template <> const Type_info _TYPE_<int>::info_obj(_NOM_, 1, name2(_TYPE_,bases)<int> );  \
  template <> const Type_info _TYPE_<trustIdType>::info_obj(_NOM_ "_64", 1, name2(_TYPE_, bases)<trustIdType> );
#else
# define info_obj_def_macro_base(_NOM_, _TYPE_) \
  template <> const Type_info _TYPE_<int>::info_obj(_NOM_, 1, name2(_TYPE_,bases)<int> );
#endif

#ifdef LATATOOLS
#define Implemente_base_sans_constructeur_ni_destructeur_32_64(_TYPE_,_NOM_,_BASE_)

#else
// The implemente are of course different:
#define Implemente_base_sans_constructeur_ni_destructeur_32_64(_TYPE_,_NOM_,_BASE_) \
                                                                        \
  template <typename _T_> const Type_info* name2(_TYPE_,bases)[1] = { &(_BASE_::info_obj) };     \
                                                                                                  \
  info_obj_def_macro_base(_NOM_, _TYPE_)                               \
                                                                        \
  template <typename _T_> _TYPE_<_T_>& _TYPE_<_T_>::self_cast( Objet_U& r)   {                                \
    return ref_cast_non_const(_TYPE_<_T_>,r); /* _non_const important otherwise recursion in ref_cast */ \
  }                                                                        \
  template <typename _T_> const _TYPE_<_T_>& _TYPE_<_T_>::self_cast(const Objet_U& r)   {                        \
    return ref_cast_non_const(_TYPE_<_T_>,r); /* _non_const important otherwise recursion in ref_cast */ \
  }                                                                        \
  template <typename _T_> const Type_info*  _TYPE_<_T_>::get_info() const {                                \
    return &info_obj;                                                        \
  }                                                                        \
  template <typename _T_> const Type_info*  _TYPE_<_T_>::info() {                                        \
    return &info_obj;                                                        \
  }                                                                          \
  class __dummy__

#endif // LATATOOLS

#define Implemente_base_sans_constructeur_32_64(_TYPE_,_NOM_,_BASE_)                \
  template <typename _T_> _TYPE_<_T_>::~_TYPE_() { }                                                        \
  Implemente_base_sans_constructeur_ni_destructeur_32_64(_TYPE_,_NOM_,_BASE_)

#define Implemente_base_sans_destructeur_32_64(_TYPE_,_NOM_,_BASE_)                \
  template <typename _T_> _TYPE_<_T_>::_TYPE_() { }                                                        \
  Implemente_base_sans_constructeur_ni_destructeur_32_64(_TYPE_,_NOM_,_BASE_)

#define Implemente_base_32_64(_TYPE_,_NOM_,_BASE_)                                \
  template <typename _T_> _TYPE_<_T_>::_TYPE_() { }                                                        \
  template <typename _T_> _TYPE_<_T_>::~_TYPE_() { }                                                        \
  Implemente_base_sans_constructeur_ni_destructeur_32_64(_TYPE_,_NOM_,_BASE_)

