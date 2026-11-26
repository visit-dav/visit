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

#ifndef TRUSTArray_included
#define TRUSTArray_included

#include <Array_base.h>
#include <TVAlloc.h>     // Custom allocator
#include <Double.h>
#include <span.hpp>
#include <memory>
#include <climits>
#include <vector>
#include <TRUSTTabs_forward.h>

#include <Device.h>

//Booleans for checking if an execution space is host or device
#ifndef LATATOOLS

#include <View_Types.h>  // Kokkos stuff
//When compiled on CPU, DefaultExecutionSpace=DefaultHostExecutionSpace = serial or OpenMP
//When compiled on GPU, DefaultExecutionSpace=Cuda, DefaultHostExecutionSpace=serial or OpenMP

//Checking if ExecSpace is Kokkos::DefaultExecutionSpace
//GPU compiled: True if Exec=Device
//CPU compiled: Always true
template <typename EXEC_SPACE >
constexpr bool is_default_exec_space = std::is_same<EXEC_SPACE, Kokkos::DefaultExecutionSpace>::value;

//Checking if GPU enabled, and if so, checking if ExecSpace is Kokkos::DefaultExecutionSpace
//GPU compiled: true is Exec=Host
//CPU compiled: Always false
template <typename EXEC_SPACE >
constexpr bool gpu_enabled_is_host_exec_space = std::is_same<EXEC_SPACE, Kokkos::DefaultHostExecutionSpace>::value &&
                                                !std::is_same<Kokkos::DefaultHostExecutionSpace, Kokkos::DefaultExecutionSpace>::value;
#endif

/*! @brief Represents a an array of int/int64/double/... values.
 *
 * The two main members are mem_ and span_:
 * - 'mem_' is the (shared) pointer to the actual underlying data. The block of data itself is a std::vector. This
 *    block of data can be shared among several arrays (see below) or can be null if we reference external data (ref_data)
 * - 'span_' is a view on the actual data, and can point to a sub-part of *mem_.
 *
 * We can have 3 states for the array:
 *   - "detached": meaning mem_==nullptr, span_.empty() == true (state obtained with the default ctor and detach_array())
 *   - "normal" : in this case mem_ is a non-null shared pointer to a std::vector holding the data. 'span_' then typically represents the entire span of the vector.
 *    The array is always initialised with 0.
 *    When the array is destroyed, the shared_ptr 'mem_' is destroyed too, and if this was the last reference to the underlying data, the std::vector itself
 *    is freed.
 *    The memory space of the array can be shared among several TRUSTArray ('mem_' has the same underlying value in several instances).
 *    This is typically produced by ref_array().
 *    Note that when this happens, we have two instances pointing to the same underlying block of data, but *none* of them has precedence over the other one (none
 *    of them is 'the' owner of the data). Ownership is shared, and when the last owner is destroyed, memory is released.
 *   - "ref_data" : this is used to point to an exterior existing memory zone (not managed by TRUSTArray - for example data provided by an external Fortran func)
 *    In this case, mem_ remains nullptr, just the span_ is correctly filled, and no memory is released when the array is destroyed.
 *
 * Finally, in case of a "XXTrav" array, the memory allocation is special:
 *  - a Trav array is allocated once, but when released, memory is kept in a pool so that another request for a new Trav might re-use it without having
 *  to perform the allocation again.
 *  - a Trav may not be ref_data or ref_array.
 *  - see implementation details of this mechanism in the TRUSTTravPool class.
 *
 *  In the template parameters below, _TYPE_ is the value type (what it contains), _SIZE_ is the extent type (how many items).
 */
template <typename _TYPE_, typename _SIZE_>
class TRUSTArray : public Array_base
{
protected:
  unsigned taille_memoire() const override { return sizeof(TRUSTArray<_TYPE_, _SIZE_>); }

  int duplique() const override
  {
    TRUSTArray* xxx = new  TRUSTArray(*this);
    if(!xxx) Process::exit("Not enough memory ");
    return xxx->numero();
  }
  Sortie& printOn(Sortie& os) const override;
  Entree& readOn(Entree& is) override;

  //From TRUSTTab.h; now here
  int nb_dim_ = 1;

public:
  using Value_type_ = _TYPE_;
  using int_t = _SIZE_;
  using Iterator_ = typename tcb::span<_TYPE_>::iterator;
  using Vector_ = std::vector<_TYPE_, TVAlloc<_TYPE_> >;
  using Span_ = tcb::span<_TYPE_>;

  // Tests can inspect whatever they want:
  friend class TRUSTArrayKokkos;

  // One instanciation with given template parameter may see all other template versions (useful in ref_as_big())
  template<typename _TYPE2_, typename _SIZE2_> friend class TRUSTArray;

  // Iterators
  inline Iterator_ begin() { return span_.begin(); }
  inline Iterator_ end() { return span_.end(); }
  inline const Iterator_ begin() const { return span_.begin(); }
  inline const Iterator_ end() const { return span_.end(); }

  inline virtual ~TRUSTArray();

  TRUSTArray() : TRUSTArray(0) { }

  TRUSTArray(_SIZE_ n) : storage_type_(STORAGE::STANDARD)
  {
    if (n)
      {
        // Initialize underlying std::vector<> with 0:
        mem_ = std::make_shared<Vector_>(Vector_(n, (_TYPE_)0));
        span_ = Span_(*mem_);
        data_location_ = std::make_shared<DataLocation>(DataLocation::HostOnly);
      }
  }

  /**
   * Copy ctor. Performs a deep copy.
   *
   * It is forbidden to deep copy a ref_data.
   */
  TRUSTArray(const TRUSTArray& A) :
    Array_base(), storage_type_(A.storage_type_)
  {
    assert(A.mem_ != nullptr || A.span_.empty());
    const _SIZE_ size = A.size_array();
    if (size > 0)
      {
        // storage_type_ must be set properly before invoking this! So that Trav mechanism works:
        resize_array_(size, RESIZE_OPTIONS::NOCOPY_NOINIT);
        if (A.get_data_location() != DataLocation::HostOnly)   // Only allocate *this on device if A is on the device
          // Not a Trav already allocated on device (avoid double alloc on device for Trav)
          if(!(storage_type_ == STORAGE::TEMP_STORAGE && isAllocatedOnDevice(mem_->data())))
            allocateOnDevice(*this);
        // Set new location AFTER possible allocation
        data_location_ = std::make_shared<DataLocation>(A.get_data_location());
        inject_array(A);
      }
  }

  // Resizing methods
  inline void resize(_SIZE_ new_size, RESIZE_OPTIONS opt=RESIZE_OPTIONS::COPY_INIT) { resize_array(new_size, opt); }
  inline void resize_array(_SIZE_ new_size, RESIZE_OPTIONS opt=RESIZE_OPTIONS::COPY_INIT);

  // Conversion method - from a small array (_SIZE_=int) of TID (_TYPE_=trustIdType), return a big one (_SIZE_=long).
  // No data copied! This behaves somewhat like a ref_array:
  void ref_as_big(TRUSTArray<_TYPE_, trustIdType>& out) const;

  // The other way around compared to ref_as_big! From big to small. In debug, size limit is checked.
  void ref_as_small(TRUSTArray<_TYPE_, int>& out) const;

  // Conversion from a BigArrOfTID to an ArrOfInt - see also similar method in TRUSTTab
  void from_tid_to_int(TRUSTArray<int, int>& out) const;

  /*! Memory allocation type - TEMP arrays (i.e. Trav) have a different allocation mechanism - see TRUSTTravPool.h) */
  inline void set_mem_storage(const STORAGE storage);
  inline STORAGE get_mem_storage() const { return storage_type_; }

  inline std::shared_ptr<Vector_> get_mem() {return mem_;}

  inline TRUSTArray& operator=(const TRUSTArray&);

  inline _TYPE_& operator[](_SIZE_ i);
  inline const _TYPE_& operator[](_SIZE_ i) const;

  inline _TYPE_& operator()(_SIZE_ i) { return operator[](i); }
  inline const _TYPE_& operator()(_SIZE_ i) const { return operator[](i); }

  // These methods return a pointer to the first element of the array for use on the host
  inline _TYPE_ * addr();
  inline const _TYPE_ * addr() const;
  // Les memes methodes pour une utilisation sur le device
  inline _TYPE_ *data();

  inline const _TYPE_ *data() const;

  /*! Return the size of the span on the data (not the full underlying allocated size)   */
  inline _SIZE_ size_array() const;

  /*! Returns the number of owners of the data, i.e. the number of Arrays pointing to the same underlying data */
  inline int ref_count() const;

#ifdef TRUST_GTEST
  inline int nb_dim() const;
#endif

  /*! Add a slot at the end of the array and store it valeur -> similar to vector<>::push_back */
  inline void append_array(_TYPE_ valeur);

  /*! Assign 'x' to all slots in the array */
  TRUSTArray& operator=(_TYPE_ x);

  /*! Element-wise addition over all entries: the size of y must be at least equal to the size of *this */
  TRUSTArray& operator+=(const TRUSTArray& y);

  /*! Adds the same value to all entries of the array */
  TRUSTArray& operator+=(const _TYPE_ dy);

  /*! Element-wise subtraction over all entries: array must have the same size as *this */
  TRUSTArray& operator-=(const TRUSTArray& y);

  /*! Subtracts the same value from all entries */
  TRUSTArray& operator-=(const _TYPE_ dy);

  /*! Multiplies all entries by dy */
  TRUSTArray& operator*= (const _TYPE_ dy);

  /*! Divides all entries by dy (not for TRUSTArray<int>) */
  TRUSTArray& operator/= (const _TYPE_ dy);

  TRUSTArray& inject_array(const TRUSTArray& source, _SIZE_ nb_elements=-1,  _SIZE_ first_element_dest=0, _SIZE_ first_element_source=0);

  inline TRUSTArray& copy_array(const TRUSTArray& a)
  {
    operator=(a);
    return *this;
  }

  inline void ordonne_array();

  // Virtual methods

  /*! Constructs an array pointing to existing data !!! Use ref_data with caution */
  inline virtual void ref_data(_TYPE_* ptr, _SIZE_ size);
  /*! Resets the array to the state obtained by the default constructor (frees memory but keeps the current memory allocation mode) */
  inline virtual void reset() { detach_array(); }
  inline virtual void ref_array(TRUSTArray&, _SIZE_ start=0, _SIZE_ sz=-1);
  inline virtual void resize_tab(_SIZE_ n, RESIZE_OPTIONS opt=RESIZE_OPTIONS::COPY_INIT);

  // Host/Device methods:
  inline DataLocation get_data_location() {  return data_location_ == nullptr ? DataLocation::HostOnly : *data_location_;   }
  inline DataLocation get_data_location() const { return data_location_ == nullptr ? DataLocation::HostOnly : *data_location_; }
  inline void set_data_location(DataLocation flag) { if (data_location_ != nullptr) *data_location_ = flag; }
  inline void set_data_location(DataLocation flag) const { if (data_location_ != nullptr) *data_location_ = flag; }
  inline int size_mem() { return mem_ == nullptr ? 0 : (int)mem_->size(); };

  // Host memory freed/alloc on GPU:
  void free_host_memory();
  void reallocate_host_memory();
  bool host_memory_freed() const;

  inline void ensureDataOnHost();
  inline void ensureDataOnHost() const;
  inline bool isDataOnDevice() const;
  inline bool checkDataOnDevice();
  inline bool checkDataOnDevice() const;
  inline bool checkDataOnDevice(const TRUSTArray& arr);

  inline virtual Span_ get_span() { return span_; }
  inline virtual Span_ get_span_tot() { return span_; }
  inline virtual const Span_ get_span() const { return span_; }
  inline virtual const Span_ get_span_tot() const { return span_; }

  //From TRUSTArray.h, now also here and overridden in TRUSTTab
  inline virtual _SIZE_ dimension_tot(int) const;

#ifdef KOKKOS

  template<int _SHAPE_>
  inline bool check_flattened() const;

  template<int _SHAPE_>
  std::array<_SIZE_, 4> getDims() const;

  //Clean the internal view of the Trust Array in case it is needed, if the Array is static to avoid it's destruction after Kokkos::finalize
  inline void CleanMyView()
  {
    device_view_1_=DeviceView<_TYPE_,1>();
    device_view_2_=DeviceView<_TYPE_,2>();
    device_view_3_=DeviceView<_TYPE_,3>();
    device_view_4_=DeviceView<_TYPE_,4>();
  }

  /*! @name Kokkos view accessors
   *
   * Return an unmanaged Kokkos view on the data of the array. The view does not own the memory:
   * it stays valid only as long as the array is neither resized nor destroyed.
   * See implementation in TRUSTArray_kokkos.tpp and examples in test/UnitTests/unit_array_kokkos.cpp
   *
   * The shape is always explicit, there is no default:
   * @code
   * ConstView<double,1> a = arr.view_ro<1>();   // ArrOfDouble / DoubleVect
   * ConstView<double,2> t = tab.view_ro<2>();   // 2D DoubleTab
   * View<double,3>      u = tab3.view_rw<3>();  // 3D DoubleTab
   * ConstView<double,1> f = tab.view_ro<1>();   // any DoubleTab, flattened
   * @endcode
   *
   * Allowed shapes (checked by assert in debug):
   *  - on an array (TRUSTArray): only _SHAPE_ = 1
   *  - on a tab: _SHAPE_ = 1 gives a flattened view of extent size_array(), otherwise _SHAPE_ must equal nb_dim().
   *    A multi-D view on a 1D tab (nb_dim()==1) is also accepted: this happens in Op_Conv_VEF_Face.cpp where a
   *    default constructed tab (nb_dim=1) points to other tabs and is used in Kokkos (weird case).
   *
   * Inside a template, when the array type depends on a template parameter, write x.template view_ro<N>().
   *
   * Each accessor has two overloads selected by EXEC_SPACE:
   *  - EXEC_SPACE = Kokkos::DefaultExecutionSpace (default): device view, data is moved to the device if needed.
   *  - EXEC_SPACE = Kokkos::DefaultHostExecutionSpace, GPU build only: host view, data is copied back to the host
   *    if needed. On a CPU build this overload does not exist (both spaces are the same) and the default one
   *    already gives host memory.
   *
   * @tparam _SHAPE_ rank of the returned view, from 1 to 4
   * @tparam EXEC_SPACE execution space where the view will be used
   */
  ///@{

  /*! @brief Read-only device view.
   *
   * Allocates the array on the device and copies it there if it is not up to date. The host copy stays valid
   * (location becomes HostDevice unless the data was already on the device).
   * @return ConstView<_TYPE_,_SHAPE_> on device memory
   */
  template <int _SHAPE_, typename EXEC_SPACE = Kokkos::DefaultExecutionSpace>
  inline std::enable_if_t<is_default_exec_space<EXEC_SPACE>, ConstView<_TYPE_,_SHAPE_> >
  view_ro() const;

  /*! @brief Read-only host view (GPU build only).
   *
   * Copies the data back from the device if the host copy is out of date.
   * @return ConstHostView<_TYPE_,_SHAPE_> on host memory
   */
  template <int _SHAPE_, typename EXEC_SPACE = Kokkos::DefaultExecutionSpace>
  inline std::enable_if_t<gpu_enabled_is_host_exec_space<EXEC_SPACE>, ConstHostView<_TYPE_,_SHAPE_> >
  view_ro() const;

  /*! @brief Write-only device view.
   *
   * The array is marked as up to date on the device only (location Device): the host copy is stale until
   * the next host access. Every value accessed through the view is expected to be written.
   * @note The data movement is currently the same as view_rw(): an out of date device copy is still refreshed first.
   * @return View<_TYPE_,_SHAPE_> on device memory
   */
  template <int _SHAPE_, typename EXEC_SPACE=Kokkos::DefaultExecutionSpace>
  inline std::enable_if_t<is_default_exec_space<EXEC_SPACE>, View<_TYPE_,_SHAPE_> >
  view_wo();

  /*! @brief Write-only host view (GPU build only).
   *
   * Brings the data back to the host if needed and marks it as up to date on the host only.
   * @return HostView<_TYPE_,_SHAPE_> on host memory
   */
  template <int _SHAPE_, typename EXEC_SPACE=Kokkos::DefaultExecutionSpace>
  inline std::enable_if_t<gpu_enabled_is_host_exec_space<EXEC_SPACE>, HostView<_TYPE_,_SHAPE_> >
  view_wo();

  /*! @brief Read-write device view.
   *
   * Copies the data to the device if it is not up to date there, then marks it as up to date on the device
   * only (location Device): the host copy is stale until the next host access.
   * @return View<_TYPE_,_SHAPE_> on device memory
   */
  template <int _SHAPE_, typename EXEC_SPACE=Kokkos::DefaultExecutionSpace>
  inline std::enable_if_t<is_default_exec_space<EXEC_SPACE>, View<_TYPE_,_SHAPE_> >
  view_rw();

  /*! @brief Read-write host view (GPU build only).
   *
   * Copies the data back from the device if needed and marks it as up to date on the host only.
   * @return HostView<_TYPE_,_SHAPE_> on host memory
   */
  template <int _SHAPE_, typename EXEC_SPACE=Kokkos::DefaultExecutionSpace>
  inline std::enable_if_t<gpu_enabled_is_host_exec_space<EXEC_SPACE>, HostView<_TYPE_,_SHAPE_> >
  view_rw();

  ///@}

#endif

protected:
  inline void attach_array(const TRUSTArray& a, _SIZE_ start=0, _SIZE_ size=-1);
  inline bool detach_array();

  void resize_array_(_SIZE_ n, RESIZE_OPTIONS opt=RESIZE_OPTIONS::COPY_INIT);

#ifdef KOKKOS
  // Kokkos members
  template<int _SHAPE_> inline void init_device_view() const;

  mutable DeviceView<_TYPE_, 1> device_view_1_;
  mutable DeviceView<_TYPE_, 2> device_view_2_;
  mutable DeviceView<_TYPE_, 3> device_view_3_;
  mutable DeviceView<_TYPE_, 4> device_view_4_;

#endif

private:

  /*! Shared pointer to the actual underlying memory block:
   *   - shared_ptr because data can be shared between several owners -> see ref_array()
   *   - std::vector<> because we want contiguous data, with a smart allocation mechanism
   * WARNING: allocation mechanism for a Trav array is special.
   */
  std::shared_ptr<Vector_> mem_;

  /*! Actual view on the data. See comments at the top of the class */
  Span_ span_;

  /*! Flag indicating whether memory allocation uses a standard new or the TRUST temporary memory pool */
  STORAGE storage_type_;

  // Flag for the data status on the Device:
  // HostOnly  : Not yet allocated on the device
  // Host      : Up to date on the host, not on the device
  // Device    : Up to date on the device, not on the host
  // HostDevice: Up to date on both host and device
  // In a shared_ptr because this state has the same status as mem_ (same sharing properties)
  mutable std::shared_ptr<DataLocation> data_location_;

private:
  /*! Debug */
  template<typename _TAB_> void ref_conv_helper_(_TAB_& out) const;

#ifdef KOKKOS

  // get_device_view for _SHAPE_ == 1
  template<int _SHAPE_>
  typename std::enable_if<_SHAPE_ == 1, DeviceView<_TYPE_, 1>>::type& get_device_view() const { return device_view_1_; }

  // get_device_view for _SHAPE_ == 2
  template<int _SHAPE_>
  typename std::enable_if<_SHAPE_ == 2, DeviceView<_TYPE_, 2>>::type& get_device_view() const { return device_view_2_; }

  // get_device_view for _SHAPE_ == 3
  template<int _SHAPE_>
  typename std::enable_if<_SHAPE_ == 3, DeviceView<_TYPE_, 3>>::type& get_device_view() const { return device_view_3_; }

  // get_device_view for _SHAPE_ == 4
  template<int _SHAPE_>
  typename std::enable_if<_SHAPE_ == 4, DeviceView<_TYPE_, 4>>::type& get_device_view() const { return device_view_4_; }

#endif
};

using ArrOfDouble = TRUSTArray<double, int>;
using ArrOfFloat = TRUSTArray<float, int>;
using ArrOfInt = TRUSTArray<int, int>;
using ArrOfTID = TRUSTArray<trustIdType, int>;

template <typename _TYPE_>
using BigTRUSTArray = TRUSTArray<_TYPE_, trustIdType>;

using BigArrOfDouble = BigTRUSTArray<double>;
using BigArrOfInt = BigTRUSTArray<int>;
using BigArrOfTID = BigTRUSTArray<trustIdType>;


/* *********************************** *
 * NON-MEMBER FUNCTIONS OF TRUSTArray  *
 * *********************************** */

#include <TRUSTArray_tools.tpp> // external templates function specializations ici ;)

/* ******************************* *
 * MEMBER FUNCTIONS OF TRUSTArray  *
 * ******************************* */

#include <TRUSTArray_device.tpp> // OMP stuff
#ifndef LATATOOLS
#include <TRUSTArray_kokkos.tpp> // Kokkos stuff
#endif

#include <TRUSTArray.tpp> // The rest here!

#endif /* TRUSTArray_included */
