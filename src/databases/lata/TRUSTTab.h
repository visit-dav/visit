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

#ifndef TRUSTTab_included
#define TRUSTTab_included

#include <type_traits>
#include <MD_Vector_tools.h>
#include <MD_Vector_base.h>
#include <TRUSTArray.h>
#include <limits.h>
#include <math.h>

#include <MD_Vector.h>
#ifndef LATATOOLS   // Lata tools does not use parallelism
#include <DescStructure.h>
#include <communications.h>
#endif

/*! @brief N-dimensional array for N <= 4, optionally carrying a distributed array structure (MD_Vector).
 *
 * Built on top of TRUSTArray with index computation for each dimension. A vector is a TRUSTTab with nb_dim() == 1,
 * and the names TRUSTVect, DoubleVect, IntVect, ... are aliases of TRUSTTab.
 *
 * Element access:
 *  - operator[](i) and operator()(i) are flattened accessors: they read the underlying data as a 1D array whatever
 *    nb_dim() is,
 *  - operator()(i, j, ...) with several indices checks (in debug) that the number of indices matches nb_dim().
 *
 */
template<typename _TYPE_, typename _SIZE_>
class TRUSTTab : public TRUSTArray<_TYPE_,_SIZE_>
{
protected:
  inline unsigned taille_memoire() const override { throw; }

  inline int duplique() const override
  {
    TRUSTTab* xxx = new  TRUSTTab(*this);
    if(!xxx) Process::exit("Not enough memory ");
    return xxx->numero();
  }

  /*! @brief Writes a sequential array (no meaningful action is defined for a distributed array).
   *
   */
  Sortie& printOn(Sortie& os) const override
  {
#ifndef LATATOOLS
    assert(verifie_LINE_SIZE());
    if (TRUSTArray<_TYPE_,_SIZE_>::nproc() > 1 && md_vector_)
      Process::exit("Error in TRUSTTab::printOn: try to print a parallel vector");
    os << this->nb_dim_ << finl;
    assert(dimensions_[0] == dimension_tot_0_);
    if (this->nb_dim_ > 0) os.put(dimensions_, this->nb_dim_, this->nb_dim_);

    const _SIZE_ sz = TRUSTArray<_TYPE_,_SIZE_>::size_array();
    os << sz << finl;
    const int l_size = line_size();
    const _TYPE_ *data = TRUSTArray<_TYPE_,_SIZE_>::addr();

    if (sz > 0)  os.put(data, sz, l_size);
#endif
    return os;
  }

  /*! @brief Reads a sequential array written in the TRUSTTab format: nb_dim, dimensions, size, values.
   *
   * If the data does not look like this format and the input can be rewound, the input is read again in the
   * one-dimensional format "size, values" (see readOnAsVect()). Diffuse inputs (Lec_Diffuse_base) and
   * Entree_complete cannot be rewound: for them a wrong format is an error.
   *
   * @param[in,out] is the input stream
   * @return the input stream
   */
  Entree& readOn(Entree& is) override;

  /*! @brief Reads a sequential one-dimensional array written in the format "size, values" (the format of TRUSTArray::printOn()).
   *
   * @param[in,out] is the input stream
   * @return the input stream
   */
  Entree& readOnAsVect(Entree& is);

public:

  using Span_ = tcb::span<_TYPE_>;

  // One instanciation with given template parameter may see all other template versions (useful in ref_as_xxx())
  template<typename _TYPE2_, typename _SIZE2_> friend class TRUSTTab;

  TRUSTTab() : dimension_tot_0_(0)
  {
    this->nb_dim_=1;
    init_dimensions(dimensions_);
    dimensions_[0] = 0;
  }

  /*! @brief Copy constructor.
   *
   * This is a deep copy; see ArrOfDouble::ArrOfDouble(const ArrOfDouble &). Note: there is no copy constructor from ArrOfDouble on purpose,
   *    to avoid large performance losses from implicit object creation that are hard to track down.
   *    (Example: calling toto(const IntVect &) with an ArrOfInt would silently copy the array!)
   *   Use copy() to copy an ArrOfDouble into a DoubleTab.
   *
   */
  TRUSTTab(const TRUSTTab& dbt): TRUSTArray<_TYPE_,_SIZE_>(dbt), md_vector_(dbt.md_vector_), size_reelle_(dbt.size_reelle_), line_size_(dbt.line_size_), dimension_tot_0_(dbt.dimension_tot_0_)
  {
    this->nb_dim_=dbt.nb_dim_;
    for (int i = 0; i < MAXDIM_TAB; i++) dimensions_[i] = dbt.dimensions_[i];
  }

  /*! @brief Constructs a one-dimensional array of size n.
   *
   * Elements are initialized to zero by default. To skip initialization, use: DoubleTab toto;
   *    toto.resize(n, RESIZE_OPTIONS::NOCOPY_NOINIT);
   *
   */
  TRUSTTab(_SIZE_ n) : TRUSTArray<_TYPE_,_SIZE_>(n), size_reelle_(n), dimension_tot_0_(n)
  {
    this->nb_dim_=1;
    init_dimensions(dimensions_);
    dimensions_[0] = n;
  }

  TRUSTTab(_SIZE_ n1, int n2): TRUSTArray<_TYPE_,_SIZE_>(n1*n2), size_reelle_(n1*n2), dimension_tot_0_(n1)
  {
    this->nb_dim_=2;
    assert(n1 >= 0 && n2 >= 0);
    if (std::is_same<_SIZE_,int>::value && (long)n1*(long)n2 > (long)std::numeric_limits<int>::max())
      {
        Cerr << "n1*n2 > 2^31. Error! Contact TRUST support, integer 32 bits limit exceeded with n1=" << n1 << " and n2=" << n2 << finl;
        Process::exit();
      }
    init_dimensions(dimensions_);
    dimensions_[0]=n1;
    dimensions_[1]=n2;
    set_line_size_(n2);
  }

  TRUSTTab(_SIZE_ n1, int n2, int n3) : TRUSTArray<_TYPE_,_SIZE_>(n1*n2*n3), size_reelle_(n1*n2*n3), dimension_tot_0_(n1)
  {

    this->nb_dim_=3;
    assert(n1 >= 0 && n2 >= 0 && n3 >= 0);
    if (std::is_same<_SIZE_,int>::value && (long)n1*(long)n2*(long)n3 > (long)std::numeric_limits<int>::max())
      {
        Cerr << "n1*n2*n3 > 2^31. Error! Contact TRUST support, integer 32 bits limit exceeded with n1=" << n1 << " and n2=" << n2 << " and n3=" << n3 << finl;
        Process::exit();
      }
    init_dimensions(dimensions_);
    dimensions_[0]=n1;
    dimensions_[1]=n2;
    dimensions_[2]=n3;
    set_line_size_(n2*n3);
  }

  TRUSTTab(_SIZE_ n1, int n2, int n3, int n4) : TRUSTArray<_TYPE_,_SIZE_>(n1*n2*n3*n4), size_reelle_(n1*n2*n3*n4), dimension_tot_0_(n1)
  {
    this->nb_dim_=4;
    assert(n1 >= 0 && n2 >= 0 && n3 >= 0 && n4 >= 0);
    if (std::is_same<_SIZE_,int>::value && (long)n1*(long)n2*(long)n3*(long)n4 > (long)std::numeric_limits<int>::max())
      {
        Cerr << "n1*n2*n3*n4 > 2^31. Error! Contact TRUST support, integer 32 bits limit exceeded with n1=" << n1 << " and n2=" << n2 << " and n3=" << n3 << " and n3=" << n3 << finl;
        Process::exit();
      }
    init_dimensions(dimensions_);
    dimensions_[0]=n1;
    dimensions_[1]=n2;
    dimensions_[2]=n3;
    dimensions_[3]=n4;
    set_line_size_(n2*n3*n4);
  }

  inline _SIZE_ size() const;
  inline _SIZE_ size_totale() const;
  inline _SIZE_ size_reelle() const;
  inline _SIZE_ size_reelle_ok() const;
  inline int line_size() const;   // Even in 64b we suppose line_size_ is always sufficiently small to fit in an int.

  inline void resize_dim0(_SIZE_ n, RESIZE_OPTIONS opt=RESIZE_OPTIONS::COPY_INIT);
  template<typename ...Args> inline void resize(_SIZE_ n, Args ...args); // up to 3 extra dims, optionally followed by a RESIZE_OPTIONS
  inline void resize(const TRUSTArray<_SIZE_,int>& tailles, RESIZE_OPTIONS opt=RESIZE_OPTIONS::COPY_INIT);
  inline void copy(const TRUSTArray<_TYPE_,_SIZE_>&, RESIZE_OPTIONS opt=RESIZE_OPTIONS::COPY_INIT);
  inline void copy(const TRUSTTab&, RESIZE_OPTIONS opt=RESIZE_OPTIONS::COPY_INIT);
  template<typename ...Args> inline void append_line(Args... vals);

  // See doc in .tpp file!
  _SIZE_ dimension(int d) const;
  int dimension_int(int d) const;

  inline int nb_dim() const { return this->nb_dim_; }

  // See same methods in TRUSTArray - CAREFUL, this is not an override because arg types are different (tab vs arr)
  inline void from_tid_to_int(TRUSTTab<int, int>& out) const;
  inline void ref_as_big(TRUSTTab<_TYPE_, trustIdType>& out) const;
  inline void ref_as_small(TRUSTTab<_TYPE_, int>& out) const;

  inline TRUSTTab& operator=(const TRUSTTab&);
  inline TRUSTTab& operator=(_TYPE_ d);
  inline _TYPE_& operator()(const TRUSTArray<_SIZE_,int>& indice);
  inline _TYPE_ operator()(const TRUSTArray<_SIZE_,int>& indice) const;
  inline _TYPE_& operator()(_SIZE_ i);
  inline const _TYPE_& operator()(_SIZE_ i) const ;
  inline _TYPE_& operator()(_SIZE_ i1, int i2);
  inline const _TYPE_& operator()(_SIZE_ i1, int i2) const ;
  inline _TYPE_& operator()(_SIZE_ i1, int i2, int i3);
  inline const _TYPE_& operator()(_SIZE_ i1, int i2, int i3) const ;
  inline _TYPE_& operator()(_SIZE_ i1, int i2, int i3, int i4);
  inline const _TYPE_& operator()(_SIZE_ i1, int i2, int i3, int i4) const ;

  inline void operator+=(const TRUSTTab& v) { operator_add(*this, v); }
  inline void operator+=(const _TYPE_ x) { operator_add(*this, x); }
  inline void operator-=(const TRUSTTab& v) { operator_sub(*this, v); }
  inline void operator-=(const _TYPE_ x) { operator_sub(*this, x); }
  inline void operator*=(const TRUSTTab& v) { operator_multiply(*this, v); }
  inline void operator*= (const _TYPE_ x) { operator_multiply(*this, x); }

  template<typename _T_ /* double ou float */>
  inline void operator/=(const TRUSTTab<_T_, _SIZE_>& v) { operator_divide(*this, v); }
  inline void operator/= (const _TYPE_ x) { operator_divide(*this, x); }

  inline virtual const MD_Vector& get_md_vector() const { return md_vector_; }

  // Default options chosen for compatibility with the previous version. Note: there was an echange_espace_virtuel call before, which is not strictly equivalent.
  inline void abs(Mp_vect_options opt=VECT_ALL_ITEMS) { operator_abs(*this, opt); }
  inline void carre(Mp_vect_options opt=VECT_ALL_ITEMS) { carre_(*this, opt); }
  inline void racine_carree(Mp_vect_options opt=VECT_ALL_ITEMS) { racine_carree_(*this, opt); }

  template<typename _SCALAR_TYPE_>
  inline void ajoute(_SCALAR_TYPE_ alpha, const TRUSTTab& y, Mp_vect_options opt=VECT_ALL_ITEMS);

  template<typename _SCALAR_TYPE_>
  inline void ajoute_sans_ech_esp_virt(_SCALAR_TYPE_ alpha, const TRUSTTab& y, Mp_vect_options opt=VECT_REAL_ITEMS);

  template<typename _SCALAR_TYPE_>
  inline void ajoute_produit_scalaire(_SCALAR_TYPE_ alpha, const TRUSTTab& x, const TRUSTTab& y, Mp_vect_options opt=VECT_ALL_ITEMS);

  template<typename _SCALAR_TYPE_>
  inline void ajoute_carre(_SCALAR_TYPE_ alpha, const TRUSTTab& y, Mp_vect_options opt=VECT_ALL_ITEMS);

#ifndef LATATOOLS
  //
  //    All the methods below involve parallelism or are not used in lata_tools
  //

  // Default: min and max over real items (compat. 1.5.6):
  inline _TYPE_ local_max_vect(Mp_vect_options opt=VECT_REAL_ITEMS) const { return local_max_vect_(*this, opt); }
  inline _TYPE_ local_min_vect(Mp_vect_options opt=VECT_REAL_ITEMS) const { return local_min_vect_(*this, opt); }
  inline _TYPE_ local_max_abs_vect(Mp_vect_options opt=VECT_REAL_ITEMS) const { return local_max_abs_vect_(*this, opt); }
  inline _TYPE_ local_min_abs_vect(Mp_vect_options opt=VECT_REAL_ITEMS) const { return local_min_abs_vect_(*this, opt); }
  inline _TYPE_ mp_max_vect(Mp_vect_options opt=VECT_REAL_ITEMS) const { return mp_max_vect_(*this, opt); }
  inline _TYPE_ mp_min_vect(Mp_vect_options opt=VECT_REAL_ITEMS) const { return mp_min_vect_(*this, opt); }
  inline _TYPE_ mp_max_abs_vect(Mp_vect_options opt=VECT_REAL_ITEMS) const { return mp_max_abs_vect_(*this, opt); }
  inline _TYPE_ mp_min_abs_vect(Mp_vect_options opt=VECT_REAL_ITEMS) const { return mp_min_abs_vect_(*this, opt); }
  inline _TYPE_ mp_norme_vect() const { return mp_norme_vect_(*this); }
#endif  // LATATOOLS

  // Only for TRUSTTab<double/float>
  template <typename _T_> inline void ajoute_produit_tensoriel(_T_ alpha, const TRUSTTab<_T_,_SIZE_>&, const TRUSTTab<_T_,_SIZE_>&); // z+=alpha*x*y;
  template <typename _T_> inline void resoud_LU(_SIZE_, TRUSTArray<int,_SIZE_>&, const TRUSTArray<_T_,_SIZE_>&, TRUSTArray<_T_,_SIZE_>&);
  template <typename _T_> inline bool inverse_LU(const TRUSTArray<_T_,_SIZE_>&, TRUSTArray<_T_,_SIZE_>&);
  template <typename _T_> inline bool decomp_LU(_SIZE_, TRUSTArray<int,_SIZE_>&, TRUSTTab<_T_,_SIZE_>&);
  template <typename _T_> inline _T_ max_du_u(const TRUSTTab<_T_,_SIZE_>&);

  void ajoute_produit_tensoriel(int alpha, const TRUSTTab<int,_SIZE_>&, const TRUSTTab<int,_SIZE_>&) = delete; // z+=alpha*x*y;
  void resoud_LU(int, TRUSTArray<int,_SIZE_>&, const TRUSTArray<int,_SIZE_>&, TRUSTArray<int,_SIZE_>&) = delete;
  int inverse_LU(const TRUSTArray<int,_SIZE_>&, TRUSTArray<int,_SIZE_>&) = delete;
  int decomp_LU(int, TRUSTArray<int,_SIZE_>&, TRUSTTab<int,_SIZE_>&) = delete;
  int max_du_u(const TRUSTTab<int,_SIZE_>&) = delete;

  // Virtual methods
  inline _SIZE_ dimension_tot(int) const override;
  inline virtual void ref(const TRUSTTab&);
  void ref_rewrite_unit_line_size(const TRUSTTab&);
  inline virtual void ref_tab(TRUSTTab&, _SIZE_ start_line=0, _SIZE_ nb_lines=-1);

  inline virtual void echange_espace_virtuel(IsExchangeBlocking exchange_type = IsExchangeBlocking::DefaultBlocking, const std::string kernel_name="noname");
  inline virtual void start_echange_espace_virtuel_async(const std::string kernel_name);
  inline virtual void finish_echange_espace_virtuel_async(const std::string kernel_name);

  inline virtual void set_md_vector(const MD_Vector&);
  inline virtual void jump(Entree&);
  inline virtual void lit(Entree&, bool resize_and_read=true);
  inline virtual void ecrit(Sortie&) const;
  inline virtual void detach_vect() { md_vector_.detach(); }

  inline void ref_data(_TYPE_* ptr, _SIZE_ size) override;
  inline void ref_array(TRUSTArray<_TYPE_,_SIZE_>&, _SIZE_ start=0, _SIZE_ sz=-1) override;
  inline void reset() override;
  inline void resize_tab(_SIZE_ n, RESIZE_OPTIONS opt=RESIZE_OPTIONS::COPY_INIT) override;

  inline Span_ get_span() override { return Span_(TRUSTArray<_TYPE_,_SIZE_>::addr(),size_reelle()); }
  inline Span_ get_span_tot() override { return Span_(TRUSTArray<_TYPE_,_SIZE_>::addr(),TRUSTArray<_TYPE_,_SIZE_>::size_array()); }
  inline const Span_ get_span() const override { return Span_((_TYPE_*)TRUSTArray<_TYPE_,_SIZE_>::addr(), size_reelle()); }
  inline const Span_ get_span_tot() const override { return Span_((_TYPE_*)TRUSTArray<_TYPE_,_SIZE_>::addr(), TRUSTArray<_TYPE_,_SIZE_>::size_array()); }

  template<typename ...Args> void reshape(_SIZE_ n1, Args ...args);

  inline void promote_scalar_to_dim2(); // if nb_dim()==1, expose it as (n,1)

protected:
  inline void set_line_size_(int n);   // Even in 64b we suppose line_size_ is always sufficiently small to fit in an int.
  inline void resize_vect_(_SIZE_ n, RESIZE_OPTIONS opt=RESIZE_OPTIONS::COPY_INIT);
  inline void copy_(const TRUSTTab& v, RESIZE_OPTIONS opt=RESIZE_OPTIONS::COPY_INIT);
  inline void lit_values_(Entree&, bool resize_and_read);
  inline void ecrit_values_(Sortie&) const;

  /*! Distributed array structure: a DoubleTab is an ArrOfDouble that optionally carries one. May be null.
   */
  MD_Vector md_vector_;

  /*! Number of values of the real space (see size_reelle()). -1 means that calls to size_reelle() and size() are invalid for this array.
   */
  _SIZE_ size_reelle_ = 0;

  /*! Multiplicative factor between md_vector_.nb_items_tot() and size_array(), and between md_vector_.nb_items_reels() and size_reelle_.
   *  This is the product of dimension(i) for i >= 1 (one array row per geometric item in the descriptor).
   *  Warning: line_size_ may be zero for an array with zero columns, but not if a descriptor is attached.
   *  Even in 64b we suppose line_size_ is always sufficiently small to fit in an int.
   */
  int line_size_ = 1;

private:
  static constexpr int MAXDIM_TAB = 4;
  /*! Real dimensions (dimensions_[0] * line_size() = size_reelle()): line_size() equals the product of dimensions_[i] for 1 <= i < this->nb_dim_.
   *  Everything is stored as _SIZE_ but higher dims (>=1) should fit in an int. See line_size().
   */
  _SIZE_ dimensions_[MAXDIM_TAB];

  // Total dimension (number of rows) = number of real rows + number of virtual rows
  // Dimensions dimension_tot(i>=1) are implicitly equal to dimension(i)
  _SIZE_ dimension_tot_0_;

  inline void verifie_MAXDIM_TAB() const
  {
    if (MAXDIM_TAB != 4)
      {
        Cerr << "Update code for MAXDIM_TAB for CHECK_LINE_SIZE" << finl;
        throw;
      }
  }

  inline bool verifie_LINE_SIZE() const
  {
    return ((line_size() == ((this->nb_dim_ > 1) ? dimensions_[1] : 1) * ((this->nb_dim_ > 2) ? dimensions_[2] : 1) * ((this->nb_dim_ > 3) ? dimensions_[3] : 1))
            && (line_size() * dimension_tot_0_ == TRUSTArray<_TYPE_,_SIZE_>::size_array()));
  }

  inline void init_dimensions(_SIZE_ * tab)
  {
#ifndef NDEBUG
    for (int i = 0; i < MAXDIM_TAB; i++) tab[i] = -1;
#endif
  }
};

using DoubleTab = TRUSTTab<double, int>;
using FloatTab = TRUSTTab<float, int>;
using IntTab = TRUSTTab<int, int>;
using TIDTab = TRUSTTab<trustIdType, int>;

template <typename _TYPE_>
using BigTRUSTTab = TRUSTTab<_TYPE_, trustIdType>;

using BigDoubleTab = BigTRUSTTab<double>;
using BigIntTab = BigTRUSTTab<int>;
using BigTIDTab = BigTRUSTTab<trustIdType>;

template <typename _TYPE_>
using BigTRUSTVect = TRUSTTab<_TYPE_, trustIdType>;

/* *************************************** *
 * NON-MEMBER FUNCTIONS OF TRUSTTab        *
 * *************************************** */

#include <TRUSTTab_tools.tpp> // external templates function specializations ici ;)

/* *********************************** *
 * MEMBER FUNCTIONS OF TRUSTTab        *
 * *********************************** */

#include <TRUSTTab.tpp> // The rest here!

#endif /* TRUSTTab_included */
