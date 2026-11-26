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

#include <TRUSTTab.h>
#include <TRUSTTab_tools.tpp>
#include <TRUSTTabs.h>
#include <limits>
#ifndef LATATOOLS
#include <View_Types.h>
#include <TRUSTTrav.h>
#endif
#include <MD_Vector_seq.h>

#ifndef LATATOOLS
#include <Perf_counters.h>
#endif

/**************************************************************************************/
/* Warning ! This kernels are critical for performance into several TRUST applications !
 * Do not change implementation without using performance regression testing !
 * You are warned.
 */

/*! @brief Determine which blocks of indices should be used to perform an operation.
 * @param opt option specifying which vector items to process (all, sequential, real)
 * @param md the parallel metadata vector describing the distributed structure
 * @param vect_size_tot total size of the vector
 * @param line_size number of components per item (line size)
 * @param nblocs_left output: number of blocks to iterate over
 * @return a block iterator over the selected index ranges
 */
template <typename _SIZE_>
Block_Iter<_SIZE_> determine_blocks(Mp_vect_options opt, const MD_Vector& md, const _SIZE_ vect_size_tot, const int line_size, int& nblocs_left)
{
  nblocs_left = 1;
#ifndef LATATOOLS
  // Should we use the members bloc_items_to_* of the MD_Vector_* classes ?
  // (this must be avoided when
  //    - md.valeur() is not defined (md is nul)
  //    - we want all items (VECT_SEQUENTIAL_ITEMS)
  //    - md.valeur() is a MD_Vector_seq
  //    - or md.valeur() is a MD_Vector_composite when it is an aggregation of several MD_Vector_seq)
  const bool use_blocks = (opt != VECT_ALL_ITEMS && md && md->use_blocks());

  if (use_blocks)
    {
      assert(opt == VECT_SEQUENTIAL_ITEMS || opt == VECT_REAL_ITEMS);
#if INT_is_64_ == 2
      // Should never use parallel patterns in 64b:
      assert( (!std::is_same<_SIZE_,std::int64_t>::value) );
#endif
      const ArrOfInt& items_blocs = (opt == VECT_SEQUENTIAL_ITEMS) ? md->get_blocs_items_to_sum() : md->get_blocs_items_to_compute();
      const ArrOfInt& items = (opt == VECT_SEQUENTIAL_ITEMS) ? md->get_items_to_sum() : md->get_items_to_compute();
      assert(items_blocs.size_array() % 2 == 0);
      nblocs_left = items_blocs.size_array() >> 1;
      return Block_Iter<_SIZE_>(items_blocs, items);  // iterator on int, but operator*() will cast and return a _SIZE_
    }
  else
#endif
    if (vect_size_tot > 0)
      {
        // Warning, if vect_size_tot is 0, line_size might be 0 too
        // Compute all data, in the vector (including virtual data), build a big bloc:
        nblocs_left = 1;
        return Block_Iter<_SIZE_>(0, vect_size_tot / line_size);   // iterator on a single (big) block
      }
  return Block_Iter<_SIZE_>();
}

// Explicit instanciations
template Block_Iter<int> determine_blocks(Mp_vect_options opt, const MD_Vector& md, const int vect_size_tot, const int line_size, int& nblocs_left);
#if INT_is_64_ == 2
template Block_Iter<trustIdType> determine_blocks(Mp_vect_options opt, const MD_Vector& md, const trustIdType vect_size_tot, const int line_size, int& nblocs_left);
#endif


template<typename _TYPE_, typename _SIZE_>
void ajoute_produit_scalaire(TRUSTTab<_TYPE_,_SIZE_>& resu, _TYPE_ alpha, const TRUSTTab<_TYPE_,_SIZE_>& vx, const TRUSTTab<_TYPE_,_SIZE_>& vy, Mp_vect_options opt)
{
#ifndef LATATOOLS
  ToDo_Kokkos("critical"); // Ne semble pas utilise...
  resu.ensureDataOnHost();
  vx.ensureDataOnHost();
  vy.ensureDataOnHost();
  // Master vect donne la structure de reference, les autres vecteurs doivent avoir la meme structure.
  const TRUSTTab<_TYPE_,_SIZE_>& master_vect = resu;
  const int line_size = master_vect.line_size(), vect_size_tot = master_vect.size_totale();
  const MD_Vector& md = master_vect.get_md_vector();
  assert(vx.line_size() == line_size && vy.line_size() == line_size);
  assert(vx.size_totale() == vect_size_tot && vy.size_totale() == vect_size_tot); // this test is necessary if md is null
#ifndef LATATOOLS
  assert(vx.get_md_vector() == md && vy.get_md_vector() == md);
#endif
  // Determine blocs of data to process, depending on " opt"
  int nblocs_left;
  Block_Iter<_SIZE_> bloc_itr = ::determine_blocks(opt, md, vect_size_tot, line_size, nblocs_left);
  // Shortcut for empty arrays (avoid case line_size == 0)
  if (bloc_itr.empty()) return;

  _TYPE_ *resu_base = resu.addr();
  const _TYPE_ *x_base = vx.addr();
  const _TYPE_ *y_base = vy.addr();
  for (; nblocs_left; nblocs_left--)
    {
      // Get index of next bloc start:
      const int begin_bloc = (*(bloc_itr++)) * line_size, end_bloc = (*(bloc_itr++)) * line_size;
      assert(begin_bloc >= 0 && end_bloc <= vect_size_tot && end_bloc >= begin_bloc);
      _TYPE_ *resu_ptr = resu_base + begin_bloc;
      const _TYPE_ *x_ptr = x_base + begin_bloc;
      const _TYPE_ *y_ptr = y_base + begin_bloc;
      int count = end_bloc - begin_bloc;
      for (; count; count--)
        {
          const _TYPE_ x = *x_ptr;
          const _TYPE_ y = *(y_ptr++);
          _TYPE_& p_resu = *(resu_ptr++);
          p_resu += alpha * x * y;
          x_ptr++;
        }
    }
  // In debug mode, put invalid values where data has not been computed
#ifndef NDEBUG
  invalidate_data(resu, opt);
#endif
  return;
#endif // LATATOOLS
}

// Explicit instanciation for templates:
template void ajoute_produit_scalaire<double, int>(TRUSTTab<double, int>& resu, double alpha, const TRUSTTab<double, int>& vx, const TRUSTTab<double, int>& vy, Mp_vect_options opt);
template void ajoute_produit_scalaire<float, int>(TRUSTTab<float, int>& resu, float alpha, const TRUSTTab<float, int>& vx, const TRUSTTab<float, int>& vy, Mp_vect_options opt);


//Process bloc function used below in operation_speciale_tres_generic
//It is templated as a function of the in/out view location and execution spaces (Device/Host)
#ifndef LATATOOLS
namespace
{
// True if ExecSpace is the host execution space: always the case for CPU builds, and for the host instance of GPU builds
template <typename EXEC_SPACE>
constexpr bool is_host_exec_space = std::is_same<EXEC_SPACE, Kokkos::DefaultHostExecutionSpace>::value;

/*! @brief Loop on the items selected by bloc_itr and call apply(item) for each one.
 * CPU: one contiguous kernel per block (no indirection, no gather).
 * GPU: one single kernel when the blocks are fragmented (through the list of items), one flat kernel otherwise.
 * @param label name of the kernel (use __KERNEL_NAME__ at the call site)
 * @param apply KOKKOS_LAMBDA(const _SIZE_ item)
 */
template<typename ExecSpace, typename _SIZE_, typename F>
void parallel_for_items(const std::string& label, Block_Iter<_SIZE_>& bloc_itr, int nblocs_left,
                        const int line_size, const _SIZE_ vect_size_tot, F apply)
{
  if constexpr (is_host_exec_space<ExecSpace>)
    {
      // Optimized for CPU
      for (; nblocs_left; nblocs_left--)
        {
          // Get index of next bloc start:
          const _SIZE_ begin_bloc = (*(bloc_itr++)) * line_size;
          const _SIZE_ end_bloc = (*(bloc_itr++)) * line_size;
          assert(begin_bloc >= 0 && end_bloc <= vect_size_tot && end_bloc >= begin_bloc);
          if (statistics().get_use_gpu()) start_gpu_timer(label);
          Kokkos::parallel_for(label, Kokkos::RangePolicy<ExecSpace>(begin_bloc, end_bloc), KOKKOS_LAMBDA(const _SIZE_ item)
          {
            apply(item);
          });
          if (statistics().get_use_gpu()) end_gpu_timer(label, is_default_exec_space<ExecSpace>);
        }
    }
  else
    {
      // Optimized for GPU (1 kernel launch instead of several : latency caused MPI imbalance)
      if (bloc_itr.items_)
        {
          // Fragmented blocks
          auto items = bloc_itr.items_->template view_ro<1, ExecSpace>().data();
          const _SIZE_ n_items = bloc_itr.items_->size_array();
          if (statistics().get_use_gpu()) start_gpu_timer(label);
          Kokkos::parallel_for(label,
                               Kokkos::MDRangePolicy<ExecSpace, Kokkos::Rank<2>>({0, 0}, {n_items, line_size}),
                               KOKKOS_LAMBDA(const _SIZE_ i, const int j)
          {
            const _SIZE_ item = items[i] * line_size + j;
            apply(item);
          });
          if (statistics().get_use_gpu()) end_gpu_timer(label, is_default_exec_space<ExecSpace>);
        }
      else
        {
          // No fragmented blocks (we avoid items indirection)
          if (statistics().get_use_gpu()) start_gpu_timer(label);
          Kokkos::parallel_for(label, Kokkos::RangePolicy<ExecSpace>(0, vect_size_tot), KOKKOS_LAMBDA(const _SIZE_ item)
          {
            apply(item);
          });
          if (statistics().get_use_gpu()) end_gpu_timer(label, is_default_exec_space<ExecSpace>);
        }
    }
}

/*! @brief Reduce on the items selected by bloc_itr, calling apply(item, value) for each one. Same strategy than parallel_for_items.
 * @param label name of the kernel (use __KERNEL_NAME__ at the call site)
 * @param apply KOKKOS_LAMBDA(const _SIZE_ item, Reducer::value_type& value) which accumulates into value
 * @param result overwritten with the reduction on all the selected items
 * @tparam Reducer Kokkos::Sum<T>, Kokkos::MaxLoc<T, _SIZE_>, Kokkos::MinLoc<T, _SIZE_>...
 */
template<typename ExecSpace, typename Reducer, typename _SIZE_, typename F>
void parallel_reduce_items(const std::string& label, Block_Iter<_SIZE_>& bloc_itr, int nblocs_left,
                           const int line_size, const _SIZE_ vect_size_tot, F apply, typename Reducer::value_type& result)
{
  using value_type = typename Reducer::value_type;
  if constexpr (is_host_exec_space<ExecSpace>)
    {
      // Optimized for CPU: one reduction per block, joined into result
      Reducer reducer(result);
      reducer.init(result);
      for (; nblocs_left; nblocs_left--)
        {
          // Get index of next bloc start:
          const _SIZE_ begin_bloc = (*(bloc_itr++)) * line_size;
          const _SIZE_ end_bloc = (*(bloc_itr++)) * line_size;
          assert(begin_bloc >= 0 && end_bloc <= vect_size_tot && end_bloc >= begin_bloc);
          value_type bloc_result;
          if (statistics().get_use_gpu()) start_gpu_timer(label);
          Kokkos::parallel_reduce(label, Kokkos::RangePolicy<ExecSpace>(begin_bloc, end_bloc),
                                  KOKKOS_LAMBDA(const _SIZE_ item, value_type& value)
          {
            apply(item, value);
          }, Reducer(bloc_result));
          reducer.join(result, bloc_result);
          if (statistics().get_use_gpu()) end_gpu_timer(label, is_default_exec_space<ExecSpace>);
        }
    }
  else
    {
      // Optimized for GPU (1 kernel launch instead of several : latency caused MPI imbalance)
      if (bloc_itr.items_)
        {
          // Fragmented blocks
          auto items = bloc_itr.items_->template view_ro<1, ExecSpace>().data();
          const _SIZE_ n_items = bloc_itr.items_->size_array();
          if (statistics().get_use_gpu()) start_gpu_timer(label);
          Kokkos::parallel_reduce(label,
                                  Kokkos::MDRangePolicy<ExecSpace, Kokkos::Rank<2>>({0, 0}, {n_items, line_size}),
                                  KOKKOS_LAMBDA(const _SIZE_ i, const int j, value_type& value)
          {
            const _SIZE_ item = items[i] * line_size + j;
            apply(item, value);
          }, Reducer(result));
          if (statistics().get_use_gpu()) end_gpu_timer(label, is_default_exec_space<ExecSpace>);
        }
      else
        {
          // No fragmented blocks (we avoid items indirection)
          if (statistics().get_use_gpu()) start_gpu_timer(label);
          Kokkos::parallel_reduce(label, Kokkos::RangePolicy<ExecSpace>(0, vect_size_tot),
                                  KOKKOS_LAMBDA(const _SIZE_ item, value_type& value)
          {
            apply(item, value);
          }, Reducer(result));
          if (statistics().get_use_gpu()) end_gpu_timer(label, is_default_exec_space<ExecSpace>);
        }
    }
}
template<typename ExecSpace, typename _TYPE_, typename _SIZE_, bool IS_MUL>
void operation_speciale_tres_generic_kernel(TRUSTVect<_TYPE_, _SIZE_>& tab_resu, const TRUSTVect<_TYPE_, _SIZE_>& tab_vx, int nblocs_left,
                                            Block_Iter<_SIZE_>& bloc_itr, const int line_size_vx, const _SIZE_ vect_size_tot, const int delta_line_size)
{
  auto vx = tab_vx.template view_ro<1, ExecSpace>().data();
  auto resu = tab_resu.template view_rw<1, ExecSpace>().data();
  auto apply = KOKKOS_LAMBDA(const _SIZE_ item)
  {
    const _TYPE_ x = vx[item];
    for (int j = 0; j < delta_line_size; ++j)
      {
        const _SIZE_ resu_idx = item * delta_line_size + j;
        if (IS_MUL) resu[resu_idx] *= x;
        else        resu[resu_idx] *= ((_TYPE_)1 / x);
      }
  };
  parallel_for_items<ExecSpace>(__KERNEL_NAME__, bloc_itr, nblocs_left, line_size_vx, vect_size_tot, apply);
}
}
#endif

template<TYPE_OPERATION_VECT_SPEC_GENERIC _TYPE_OP_, typename _TYPE_, typename _SIZE_>
void operation_speciale_tres_generic(TRUSTVect<_TYPE_, _SIZE_>& resu, const TRUSTVect<_TYPE_,_SIZE_>& vx, Mp_vect_options opt)
{
#ifndef LATATOOLS

  // Check the nature of the operation
  static constexpr bool IS_MUL = (_TYPE_OP_ == TYPE_OPERATION_VECT_SPEC_GENERIC::MUL_); //it's either MUL or DIV

  // get info for computation
  const int line_size = resu.line_size(), line_size_vx = vx.line_size();
  const MD_Vector& md = resu.get_md_vector();
  // The line_size of vector resu must be a multiple of the line_size of vector vx
  assert(line_size > 0 && line_size_vx > 0 && line_size % line_size_vx == 0);
  const int delta_line_size = line_size / line_size_vx;
  assert(vx.size_totale() * delta_line_size == resu.size_totale()); // this test is necessary if md is null
  assert(vx.get_md_vector() == md);

  // Determine blocs of data to process using vx dimensions so items_ holds flat vx indices
  int nblocs_left;
  Block_Iter<_SIZE_> bloc_itr = ::determine_blocks(opt, md, vx.size_totale(), line_size_vx, nblocs_left);
  // Shortcut for empty arrays (avoid case line_size == 0)
  if (bloc_itr.empty())
    return;

  bool kernelOnDevice = resu.checkDataOnDevice(vx);

  //Lauch computation with the execution space and view types as (template) parameters
  if (kernelOnDevice)
    operation_speciale_tres_generic_kernel<Kokkos::DefaultExecutionSpace, _TYPE_, _SIZE_, IS_MUL>(resu, vx, nblocs_left, bloc_itr, line_size_vx, vx.size_totale(), delta_line_size);
  else
    operation_speciale_tres_generic_kernel<Kokkos::DefaultHostExecutionSpace, _TYPE_, _SIZE_, IS_MUL>(resu, vx, nblocs_left, bloc_itr, line_size_vx, vx.size_totale(), delta_line_size);

#ifndef NDEBUG
  // In debug mode, put invalid values where data has not been computed
  invalidate_data(resu, opt);
#endif
  return;
#else
  Cerr << "Error! operation_speciale_tres_generic can't be called in your project!" << finl;
  Process::exit();
#endif
}


// Explicit instanciation for templates:
template void operation_speciale_tres_generic<TYPE_OPERATION_VECT_SPEC_GENERIC::MUL_, double, int>(TRUSTVect<double, int>& resu, const TRUSTVect<double, int>& vx, Mp_vect_options opt);
template void operation_speciale_tres_generic<TYPE_OPERATION_VECT_SPEC_GENERIC::MUL_, float, int>(TRUSTVect<float, int>& resu, const TRUSTVect<float, int>& vx, Mp_vect_options opt);
template void operation_speciale_tres_generic<TYPE_OPERATION_VECT_SPEC_GENERIC::DIV_, double, int>(TRUSTVect<double, int>& resu, const TRUSTVect<double, int>& vx, Mp_vect_options opt);
template void operation_speciale_tres_generic<TYPE_OPERATION_VECT_SPEC_GENERIC::DIV_, float, int>(TRUSTVect<float, int>& resu, const TRUSTVect<float, int>& vx, Mp_vect_options opt);

#ifndef LATATOOLS
namespace
{
template<typename ExecSpace, typename _TYPE_, typename _SIZE_, bool IS_ADD>
void operation_speciale_generic_kernel(TRUSTVect<_TYPE_, _SIZE_>& tab_resu, const TRUSTVect<_TYPE_, _SIZE_>& tab_vx, _TYPE_ alpha, int nblocs_left,
                                       Block_Iter<_SIZE_>& bloc_itr, const _SIZE_ vect_size_tot, const int line_size)
{
  auto vx = tab_vx.template view_ro<1, ExecSpace>().data();
  auto resu = tab_resu.template view_rw<1, ExecSpace>().data();
  auto apply = KOKKOS_LAMBDA(const _SIZE_ item)
  {
    const _TYPE_ x = vx[item];
    if (IS_ADD) resu[item] += alpha * x;
    else        resu[item] += alpha * x * x;
  };
  parallel_for_items<ExecSpace>(__KERNEL_NAME__, bloc_itr, nblocs_left, line_size, vect_size_tot, apply);
}
}
#endif

template <TYPE_OPERATION_VECT_SPEC _TYPE_OP_ ,typename _TYPE_, typename _SIZE_>
void ajoute_operation_speciale_generic(TRUSTVect<_TYPE_,_SIZE_>& resu, _TYPE_ alpha,
                                       const TRUSTVect<_TYPE_,_SIZE_>& vx, Mp_vect_options opt)
{
#ifndef LATATOOLS
  static constexpr bool IS_ADD = (_TYPE_OP_ == TYPE_OPERATION_VECT_SPEC::ADD_);

  const int line_size = resu.line_size();
  const _SIZE_ vect_size_tot = resu.size_totale();
  const MD_Vector& md = resu.get_md_vector();
  assert(vx.line_size() == line_size);
  assert(vx.size_totale() == vect_size_tot); // this test is necessary if md is null
  // Determine blocs of data to process, depending on " opt"
  int nblocs_left;
  Block_Iter<_SIZE_> bloc_itr = ::determine_blocks(opt, md, vect_size_tot, line_size, nblocs_left);
  // Shortcut for empty arrays (avoid case line_size == 0)
  if (bloc_itr.empty()) return;

  bool kernelOnDevice = resu.checkDataOnDevice(vx);

  if (kernelOnDevice)
    operation_speciale_generic_kernel<Kokkos::DefaultExecutionSpace, _TYPE_, _SIZE_, IS_ADD>(resu, vx, alpha, nblocs_left, bloc_itr, vect_size_tot, line_size);
  else
    operation_speciale_generic_kernel<Kokkos::DefaultHostExecutionSpace, _TYPE_, _SIZE_, IS_ADD>(resu, vx, alpha, nblocs_left, bloc_itr, vect_size_tot, line_size);


#ifndef NDEBUG
  invalidate_data(resu, opt);
#endif
  return;
#else
  Cerr << "Error! ajoute_operation_speciale_generic can't be called in your project!" << finl;
  Process::exit();
#endif
}

// Explicit instanciation for templates:
template void ajoute_operation_speciale_generic<TYPE_OPERATION_VECT_SPEC::ADD_, double, int>(TRUSTVect<double, int>& resu, double alpha, const TRUSTVect<double, int>& vx, Mp_vect_options opt);
template void ajoute_operation_speciale_generic<TYPE_OPERATION_VECT_SPEC::ADD_, float, int>(TRUSTVect<float, int>& resu, float alpha, const TRUSTVect<float, int>& vx, Mp_vect_options opt);
template void ajoute_operation_speciale_generic<TYPE_OPERATION_VECT_SPEC::SQUARE_, double, int>(TRUSTVect<double, int>& resu, double alpha, const TRUSTVect<double, int>& vx, Mp_vect_options opt);
template void ajoute_operation_speciale_generic<TYPE_OPERATION_VECT_SPEC::SQUARE_, float, int>(TRUSTVect<float, int>& resu, float alpha, const TRUSTVect<float, int>& vx, Mp_vect_options opt);

#ifndef LATATOOLS
namespace
{
template<typename ExecSpace, typename _TYPE_, typename _SIZE_, TYPE_OPERATOR_VECT _TYPE_OP_>
void operator_vect_vect_generic_kernel(TRUSTVect<_TYPE_,_SIZE_>& tab_resu, const TRUSTVect<_TYPE_, _SIZE_>& tab_vx, int nblocs_left,
                                       Block_Iter<_SIZE_>& bloc_itr,  const _SIZE_ vect_size_tot, const int line_size)
{
  static constexpr bool IS_ADD = (_TYPE_OP_ == TYPE_OPERATOR_VECT::ADD_), IS_SUB = (_TYPE_OP_ == TYPE_OPERATOR_VECT::SUB_),
                        IS_MULT = (_TYPE_OP_ == TYPE_OPERATOR_VECT::MULT_), IS_DIV = (_TYPE_OP_ == TYPE_OPERATOR_VECT::DIV_),
                        IS_EGAL = (_TYPE_OP_ == TYPE_OPERATOR_VECT::EGAL_);

  auto vx = tab_vx.template view_ro<1, ExecSpace>().data();
  auto resu = tab_resu.template view_rw<1, ExecSpace>().data();
  auto apply = KOKKOS_LAMBDA(const _SIZE_ item)
  {
    const _TYPE_ x = vx[item];
    if (IS_ADD)  resu[item] += x;
    if (IS_SUB)  resu[item] -= x;
    if (IS_MULT) resu[item] *= x;
    if (IS_DIV)  resu[item] /= x;
    if (IS_EGAL) resu[item] = x;
  };
  parallel_for_items<ExecSpace>(__KERNEL_NAME__, bloc_itr, nblocs_left, line_size, vect_size_tot, apply);
}
}
#endif


template <typename _TYPE_, typename _SIZE_, TYPE_OPERATOR_VECT _TYPE_OP_>
void operator_vect_vect_generic(TRUSTVect<_TYPE_,_SIZE_>& resu, const TRUSTVect<_TYPE_,_SIZE_>& vx, Mp_vect_options opt)
{
#ifndef LATATOOLS
  const int line_size = resu.line_size();
  const _SIZE_ vect_size_tot = resu.size_totale();
  const MD_Vector& md = resu.get_md_vector();
  assert(vx.line_size() == line_size);
  assert(vx.size_totale() == vect_size_tot); // this test is necessary if md is null
  // Determine blocs of data to process, depending on " opt"
  int nblocs_left;
  Block_Iter<_SIZE_> bloc_itr = ::determine_blocks(opt, md, vect_size_tot, line_size, nblocs_left);
  // Shortcut for empty arrays (avoid case line_size == 0)
  if (bloc_itr.empty()) return;

  bool kernelOnDevice = resu.checkDataOnDevice(vx);

  if (kernelOnDevice)
    operator_vect_vect_generic_kernel<Kokkos::DefaultExecutionSpace, _TYPE_, _SIZE_, _TYPE_OP_>(resu, vx, nblocs_left, bloc_itr, vect_size_tot, line_size);
  else
    operator_vect_vect_generic_kernel<Kokkos::DefaultHostExecutionSpace, _TYPE_, _SIZE_, _TYPE_OP_>(resu, vx, nblocs_left, bloc_itr, vect_size_tot, line_size);
  // In debug mode, put invalid values where data has not been computed
#ifndef NDEBUG
  invalidate_data(resu, opt);
#endif
  return;
#else
  Cerr << "Error! operator_vect_vect_generic can't be called in your project!" << finl;
  Process::exit();
#endif
}
// Explicit instanciation for templates:
template void operator_vect_vect_generic<double, int, TYPE_OPERATOR_VECT::ADD_>(TRUSTVect<double, int>& resu, const TRUSTVect<double, int>& vx, Mp_vect_options opt);
template void operator_vect_vect_generic<int, int, TYPE_OPERATOR_VECT::ADD_>(TRUSTVect<int, int>& resu, const TRUSTVect<int, int>& vx, Mp_vect_options opt);
template void operator_vect_vect_generic<float, int, TYPE_OPERATOR_VECT::ADD_>(TRUSTVect<float, int>& resu, const TRUSTVect<float, int>& vx, Mp_vect_options opt);
template void operator_vect_vect_generic<double, int, TYPE_OPERATOR_VECT::SUB_>(TRUSTVect<double, int>& resu, const TRUSTVect<double, int>& vx, Mp_vect_options opt);
template void operator_vect_vect_generic<int, int, TYPE_OPERATOR_VECT::SUB_>(TRUSTVect<int, int>& resu, const TRUSTVect<int, int>& vx, Mp_vect_options opt);
template void operator_vect_vect_generic<float, int, TYPE_OPERATOR_VECT::SUB_>(TRUSTVect<float, int>& resu, const TRUSTVect<float, int>& vx, Mp_vect_options opt);
template void operator_vect_vect_generic<double, int, TYPE_OPERATOR_VECT::MULT_>(TRUSTVect<double, int>& resu, const TRUSTVect<double, int>& vx, Mp_vect_options opt);
template void operator_vect_vect_generic<int, int, TYPE_OPERATOR_VECT::MULT_>(TRUSTVect<int, int>& resu, const TRUSTVect<int, int>& vx, Mp_vect_options opt);
template void operator_vect_vect_generic<float, int, TYPE_OPERATOR_VECT::MULT_>(TRUSTVect<float, int>& resu, const TRUSTVect<float, int>& vx, Mp_vect_options opt);
template void operator_vect_vect_generic<double, int, TYPE_OPERATOR_VECT::DIV_>(TRUSTVect<double, int>& resu, const TRUSTVect<double, int>& vx, Mp_vect_options opt);
template void operator_vect_vect_generic<int, int, TYPE_OPERATOR_VECT::DIV_>(TRUSTVect<int, int>& resu, const TRUSTVect<int, int>& vx, Mp_vect_options opt);
template void operator_vect_vect_generic<float, int, TYPE_OPERATOR_VECT::DIV_>(TRUSTVect<float, int>& resu, const TRUSTVect<float, int>& vx, Mp_vect_options opt);
template void operator_vect_vect_generic<double, int, TYPE_OPERATOR_VECT::EGAL_>(TRUSTVect<double, int>& resu, const TRUSTVect<double, int>& vx, Mp_vect_options opt);
template void operator_vect_vect_generic<int, int, TYPE_OPERATOR_VECT::EGAL_>(TRUSTVect<int, int>& resu, const TRUSTVect<int, int>& vx, Mp_vect_options opt);
template void operator_vect_vect_generic<float, int, TYPE_OPERATOR_VECT::EGAL_>(TRUSTVect<float, int>& resu, const TRUSTVect<float, int>& vx, Mp_vect_options opt);

#ifndef LATATOOLS
namespace
{
template<typename ExecSpace, typename _TYPE_, typename _SIZE_, TYPE_OPERATOR_SINGLE _TYPE_OP_>
void operator_vect_single_generic_kernel(TRUSTVect<_TYPE_,_SIZE_>& tab_resu, const _TYPE_ x, int nblocs_left,
                                         Block_Iter<_SIZE_>& bloc_itr,  const _SIZE_ vect_size_tot, const int line_size)
{
  static constexpr bool IS_ADD = (_TYPE_OP_ == TYPE_OPERATOR_SINGLE::ADD_), IS_SUB = (_TYPE_OP_ == TYPE_OPERATOR_SINGLE::SUB_),
                        IS_MULT = (_TYPE_OP_ == TYPE_OPERATOR_SINGLE::MULT_), IS_DIV = (_TYPE_OP_ == TYPE_OPERATOR_SINGLE::DIV_), IS_EGAL = (_TYPE_OP_ == TYPE_OPERATOR_SINGLE::EGAL_),
                        IS_NEGATE = (_TYPE_OP_ == TYPE_OPERATOR_SINGLE::NEGATE_), IS_INV = (_TYPE_OP_ == TYPE_OPERATOR_SINGLE::INV_), IS_ABS = (_TYPE_OP_ == TYPE_OPERATOR_SINGLE::ABS_),
                        IS_SQRT = (_TYPE_OP_ == TYPE_OPERATOR_SINGLE::SQRT_), IS_SQUARE = (_TYPE_OP_ == TYPE_OPERATOR_SINGLE::SQUARE_);

  auto resu = tab_resu.template view_rw<1, ExecSpace>().data();
  auto apply = KOKKOS_LAMBDA(const _SIZE_ item)
  {
    if (IS_SUB)    resu[item] -= x;
    if (IS_ADD)    resu[item] += x;
    if (IS_MULT)   resu[item] *= x;
    if (IS_EGAL)   resu[item] = x;
    if (IS_NEGATE) resu[item] = -resu[item];
    if (IS_ABS)    resu[item] = (_TYPE_) Kokkos::abs(resu[item]);
    if (IS_SQRT)   resu[item] = (_TYPE_) Kokkos::sqrt(resu[item]);
    if (IS_SQUARE) resu[item] = resu[item]*resu[item];
    if (IS_DIV)    resu[item] /= x;
    if (IS_INV)    resu[item] = (_TYPE_) ((_TYPE_)1 /resu[item]);
  };
  parallel_for_items<ExecSpace>(__KERNEL_NAME__, bloc_itr, nblocs_left, line_size, vect_size_tot, apply);
}
}
#endif


template <typename _TYPE_, typename _SIZE_, TYPE_OPERATOR_SINGLE _TYPE_OP_ >
void operator_vect_single_generic(TRUSTVect<_TYPE_,_SIZE_>& resu, const _TYPE_ x, Mp_vect_options opt)
{
#ifndef LATATOOLS
  const int line_size = resu.line_size();
  const _SIZE_ vect_size_tot = resu.size_totale();
  const MD_Vector& md = resu.get_md_vector();
  // Determine blocs of data to process, depending on " opt"
  int nblocs_left;
  Block_Iter<_SIZE_> bloc_itr = ::determine_blocks(opt, md, vect_size_tot, line_size, nblocs_left);
  // Shortcut for empty arrays (avoid case line_size == 0)
  if (bloc_itr.empty()) return;

  bool kernelOnDevice = resu.checkDataOnDevice();

  if (kernelOnDevice)
    operator_vect_single_generic_kernel<Kokkos::DefaultExecutionSpace, _TYPE_, _SIZE_, _TYPE_OP_>(resu, x, nblocs_left, bloc_itr, vect_size_tot, line_size);
  else
    operator_vect_single_generic_kernel<Kokkos::DefaultHostExecutionSpace, _TYPE_, _SIZE_, _TYPE_OP_>(resu, x, nblocs_left, bloc_itr, vect_size_tot, line_size);

  // In debug mode, put invalid values where data has not been computed
#ifndef NDEBUG
  invalidate_data(resu, opt);
#endif
  return;
#else
  Cerr << "Error! operator_vect_single_generic can't be called in your project!" << finl;
  Process::exit();
#endif
}
// Explicit instanciation for templates:
template void operator_vect_single_generic<double, int, TYPE_OPERATOR_SINGLE::ADD_>(TRUSTVect<double, int>& resu, const double x, Mp_vect_options opt);
template void operator_vect_single_generic<int, int, TYPE_OPERATOR_SINGLE::ADD_>(TRUSTVect<int, int>& resu, const int x, Mp_vect_options opt);
template void operator_vect_single_generic<float, int, TYPE_OPERATOR_SINGLE::ADD_>(TRUSTVect<float, int>& resu, const float x, Mp_vect_options opt);
template void operator_vect_single_generic<double, int, TYPE_OPERATOR_SINGLE::SUB_>(TRUSTVect<double, int>& resu, const double x, Mp_vect_options opt);
template void operator_vect_single_generic<int, int, TYPE_OPERATOR_SINGLE::SUB_>(TRUSTVect<int, int>& resu, const int x, Mp_vect_options opt);
template void operator_vect_single_generic<float, int, TYPE_OPERATOR_SINGLE::SUB_>(TRUSTVect<float, int>& resu, const float x, Mp_vect_options opt);
template void operator_vect_single_generic<double, int, TYPE_OPERATOR_SINGLE::MULT_>(TRUSTVect<double, int>& resu, const double x, Mp_vect_options opt);
template void operator_vect_single_generic<int, int, TYPE_OPERATOR_SINGLE::MULT_>(TRUSTVect<int, int>& resu, const int x, Mp_vect_options opt);
template void operator_vect_single_generic<float, int, TYPE_OPERATOR_SINGLE::MULT_>(TRUSTVect<float, int>& resu, const float x, Mp_vect_options opt);
template void operator_vect_single_generic<double, int, TYPE_OPERATOR_SINGLE::DIV_>(TRUSTVect<double, int>& resu, const double x, Mp_vect_options opt);
template void operator_vect_single_generic<int, int, TYPE_OPERATOR_SINGLE::DIV_>(TRUSTVect<int, int>& resu, const int x, Mp_vect_options opt);
template void operator_vect_single_generic<float, int, TYPE_OPERATOR_SINGLE::DIV_>(TRUSTVect<float, int>& resu, const float x, Mp_vect_options opt);
template void operator_vect_single_generic<double, int, TYPE_OPERATOR_SINGLE::EGAL_>(TRUSTVect<double, int>& resu, const double x, Mp_vect_options opt);
template void operator_vect_single_generic<int, int, TYPE_OPERATOR_SINGLE::EGAL_>(TRUSTVect<int, int>& resu, const int x, Mp_vect_options opt);
template void operator_vect_single_generic<float, int, TYPE_OPERATOR_SINGLE::EGAL_>(TRUSTVect<float, int>& resu, const float x, Mp_vect_options opt);
template void operator_vect_single_generic<double, int, TYPE_OPERATOR_SINGLE::NEGATE_>(TRUSTVect<double, int>& resu, const double x, Mp_vect_options opt);
template void operator_vect_single_generic<int, int, TYPE_OPERATOR_SINGLE::NEGATE_>(TRUSTVect<int, int>& resu, const int x, Mp_vect_options opt);
template void operator_vect_single_generic<float, int, TYPE_OPERATOR_SINGLE::NEGATE_>(TRUSTVect<float, int>& resu, const float x, Mp_vect_options opt);
template void operator_vect_single_generic<double, int, TYPE_OPERATOR_SINGLE::INV_>(TRUSTVect<double, int>& resu, const double x, Mp_vect_options opt);
template void operator_vect_single_generic<int, int, TYPE_OPERATOR_SINGLE::INV_>(TRUSTVect<int, int>& resu, const int x, Mp_vect_options opt);
template void operator_vect_single_generic<float, int, TYPE_OPERATOR_SINGLE::INV_>(TRUSTVect<float, int>& resu, const float x, Mp_vect_options opt);
template void operator_vect_single_generic<double, int, TYPE_OPERATOR_SINGLE::ABS_>(TRUSTVect<double, int>& resu, const double x, Mp_vect_options opt);
template void operator_vect_single_generic<int, int, TYPE_OPERATOR_SINGLE::ABS_>(TRUSTVect<int, int>& resu, const int x, Mp_vect_options opt);
template void operator_vect_single_generic<float, int, TYPE_OPERATOR_SINGLE::ABS_>(TRUSTVect<float, int>& resu, const float x, Mp_vect_options opt);
template void operator_vect_single_generic<double, int, TYPE_OPERATOR_SINGLE::SQRT_>(TRUSTVect<double, int>& resu, const double x, Mp_vect_options opt);
template void operator_vect_single_generic<int, int, TYPE_OPERATOR_SINGLE::SQRT_>(TRUSTVect<int, int>& resu, const int x, Mp_vect_options opt);
template void operator_vect_single_generic<float, int, TYPE_OPERATOR_SINGLE::SQRT_>(TRUSTVect<float, int>& resu, const float x, Mp_vect_options opt);
template void operator_vect_single_generic<double, int, TYPE_OPERATOR_SINGLE::SQUARE_>(TRUSTVect<double, int>& resu, const double x, Mp_vect_options opt);
template void operator_vect_single_generic<int, int, TYPE_OPERATOR_SINGLE::SQUARE_>(TRUSTVect<int, int>& resu, const int x, Mp_vect_options opt);
template void operator_vect_single_generic<float, int, TYPE_OPERATOR_SINGLE::SQUARE_>(TRUSTVect<float, int>& resu, const float x, Mp_vect_options opt);

#if INT_is_64_ == 2
template void operator_vect_single_generic<trustIdType, trustIdType, TYPE_OPERATOR_SINGLE::ADD_>(TRUSTVect<trustIdType, trustIdType>& resu, const trustIdType x, Mp_vect_options opt);
template void operator_vect_single_generic<int, trustIdType, TYPE_OPERATOR_SINGLE::ADD_>(TRUSTVect<int, trustIdType>& resu, const int x, Mp_vect_options opt);
template void operator_vect_single_generic<float, trustIdType, TYPE_OPERATOR_SINGLE::ADD_>(TRUSTVect<float, trustIdType>& resu, const float x, Mp_vect_options opt);
template void operator_vect_single_generic<double, trustIdType, TYPE_OPERATOR_SINGLE::ADD_>(TRUSTVect<double, trustIdType>& resu, const double x, Mp_vect_options opt);

template void operator_vect_single_generic<trustIdType, trustIdType, TYPE_OPERATOR_SINGLE::SUB_>(TRUSTVect<trustIdType, trustIdType>& resu, const trustIdType x, Mp_vect_options opt);
template void operator_vect_single_generic<int, trustIdType, TYPE_OPERATOR_SINGLE::SUB_>(TRUSTVect<int, trustIdType>& resu, const int x, Mp_vect_options opt);
template void operator_vect_single_generic<float, trustIdType, TYPE_OPERATOR_SINGLE::SUB_>(TRUSTVect<float, trustIdType>& resu, const float x, Mp_vect_options opt);
template void operator_vect_single_generic<double, trustIdType, TYPE_OPERATOR_SINGLE::SUB_>(TRUSTVect<double, trustIdType>& resu, const double x, Mp_vect_options opt);

template void operator_vect_single_generic<double, trustIdType, TYPE_OPERATOR_SINGLE::MULT_>(TRUSTVect<double, trustIdType>& resu, const double x, Mp_vect_options opt);
template void operator_vect_single_generic<float, trustIdType, TYPE_OPERATOR_SINGLE::MULT_>(TRUSTVect<float, trustIdType>& resu, const float x, Mp_vect_options opt);

template void operator_vect_single_generic<double, trustIdType, TYPE_OPERATOR_SINGLE::DIV_>(TRUSTVect<double, trustIdType>& resu, const double x, Mp_vect_options opt);


#endif

#ifndef LATATOOLS
namespace
{
template<typename ExecSpace, typename _TYPE_, typename _SIZE_,typename _TYPE_RETURN_,  TYPE_OPERATION_VECT _TYPE_OP_>
void local_extrema_vect_generic_kernel(const TRUSTVect<_TYPE_,_SIZE_>& vx, int nblocs_left, Block_Iter<_SIZE_>& bloc_itr,
                                       const _SIZE_ vect_size_tot, const int line_size, _TYPE_& min_max_val, _SIZE_& i_min_max)
{
  // Shortcut for empty arrays (avoid case line_size == 0)
  if (bloc_itr.empty()) return ;

  static constexpr bool IS_IMAX = (_TYPE_OP_ == TYPE_OPERATION_VECT::IMAX_), IS_IMIN = (_TYPE_OP_ == TYPE_OPERATION_VECT::IMIN_), IS_MAX = (_TYPE_OP_ == TYPE_OPERATION_VECT::MAX_),
                        IS_MIN = (_TYPE_OP_ == TYPE_OPERATION_VECT::MIN_), IS_MAX_ABS = (_TYPE_OP_ == TYPE_OPERATION_VECT::MAX_ABS_), IS_MIN_ABS = (_TYPE_OP_ == TYPE_OPERATION_VECT::MIN_ABS_);

  //For clearer code, is it max, is it a min, is it done on absolute values ?
  static constexpr bool IS_MAXS = (IS_MAX || IS_MAX_ABS || IS_IMAX);
  static constexpr bool IS_MINS = (IS_MIN || IS_MIN_ABS || IS_IMIN);
  static constexpr bool IS_ABS = (IS_MAX_ABS || IS_MIN_ABS);

  // Define the reducer, based on the reduction type
  using reducer = typename std::conditional<IS_MAXS, Kokkos::MaxLoc<_TYPE_, _SIZE_>, Kokkos::MinLoc<_TYPE_, _SIZE_>>::type;
  // Define the type of what the reducer will return ( a value + a index)
  using reducer_value_type  = typename reducer::value_type;

  if (not(IS_MAXS || IS_MINS)) {Process::exit("Wrong operation type in local_extrema_vect_generic_kernel");}

  auto vx_view= vx.template view_ro<1, ExecSpace>().data();

  auto apply = KOKKOS_LAMBDA(const _SIZE_ item, reducer_value_type& local_min_max)
  {
    const _TYPE_ val = (IS_ABS) ? Kokkos::abs(vx_view[item]) : vx_view[item];
    if ( (IS_MAXS && val>local_min_max.val) || (IS_MINS && val<local_min_max.val) )
      {
        local_min_max.val = val;
        local_min_max.loc = item;
      }
  };
  reducer_value_type global_min_max;
  parallel_reduce_items<ExecSpace, reducer>(__KERNEL_NAME__, bloc_itr, nblocs_left, line_size, vect_size_tot, apply, global_min_max);

  if ( (IS_MAXS && global_min_max.val > min_max_val) || (IS_MINS && global_min_max.val < min_max_val) )
    {
      min_max_val = global_min_max.val;
      i_min_max = global_min_max.loc;
    }
}
}
#endif

template <typename _TYPE_, typename _SIZE_, typename _TYPE_RETURN_, TYPE_OPERATION_VECT _TYPE_OP_ >
_TYPE_RETURN_ local_extrema_vect_generic(const TRUSTVect<_TYPE_,_SIZE_>& vx, Mp_vect_options opt)
{
#ifndef LATATOOLS

  //Array info
  const int line_size = vx.line_size();
  const _SIZE_ vect_size_tot = vx.size_totale();
  const MD_Vector& md = vx.get_md_vector();

  //Asserts
  assert(vx.line_size() == line_size);
  assert(vx.size_totale() == vect_size_tot); // this test is necessary if md is null
  assert(vx.get_md_vector() == md);

  // Determine blocs of data to process, depending on " opt"
  int nblocs_left;
  Block_Iter<_SIZE_> bloc_itr = ::determine_blocks(opt, md, vect_size_tot, line_size, nblocs_left);

  //Initialize results
  _TYPE_ min_max_val = neutral_value<_TYPE_,_TYPE_OP_>(); // _TYPE_ et pas _TYPE_RETURN_ desole ...
  _SIZE_ i_min_max = -1 ; // seulement pour IMAX_ et IMIN_

  //Localize data
  bool kernelOnDevice = vx.checkDataOnDevice();

  //Compute reduction
  if (kernelOnDevice)
    local_extrema_vect_generic_kernel<Kokkos::DefaultExecutionSpace, _TYPE_, _SIZE_, _TYPE_RETURN_, _TYPE_OP_>(vx, nblocs_left, bloc_itr, vect_size_tot, line_size, min_max_val, i_min_max);
  else
    local_extrema_vect_generic_kernel<Kokkos::DefaultHostExecutionSpace, _TYPE_, _SIZE_, _TYPE_RETURN_, _TYPE_OP_>(vx, nblocs_left, bloc_itr, vect_size_tot, line_size, min_max_val, i_min_max);

  //Return index or value
  static constexpr bool IS_IMAX = (_TYPE_OP_ == TYPE_OPERATION_VECT::IMAX_), IS_IMIN = (_TYPE_OP_ == TYPE_OPERATION_VECT::IMIN_);

  return (IS_IMAX || IS_IMIN) ? (_TYPE_RETURN_)i_min_max : (_TYPE_RETURN_)min_max_val;

#else
  Cerr << "Error! local_extrema_vect_generic can't be called in your project!" << finl;
  Process::exit();
  return (_TYPE_RETURN_)0; // For compil in latatools
#endif
}
// Explicit instanciation for templates:
template double local_extrema_vect_generic<double, int, double, TYPE_OPERATION_VECT::IMAX_>(const TRUSTVect<double, int>& vx, Mp_vect_options opt);
template double local_extrema_vect_generic<double, int, double, TYPE_OPERATION_VECT::IMIN_>(const TRUSTVect<double, int>& vx, Mp_vect_options opt);
template double local_extrema_vect_generic<double, int, double, TYPE_OPERATION_VECT::MAX_>(const TRUSTVect<double, int>& vx, Mp_vect_options opt);
template double local_extrema_vect_generic<double, int, double, TYPE_OPERATION_VECT::MIN_>(const TRUSTVect<double, int>& vx, Mp_vect_options opt);
template double local_extrema_vect_generic<double, int, double, TYPE_OPERATION_VECT::MAX_ABS_>(const TRUSTVect<double, int>& vx, Mp_vect_options opt);
template double local_extrema_vect_generic<double, int, double, TYPE_OPERATION_VECT::MIN_ABS_>(const TRUSTVect<double, int>& vx, Mp_vect_options opt);
template int local_extrema_vect_generic<double, int, int, TYPE_OPERATION_VECT::IMAX_>(const TRUSTVect<double, int>& vx, Mp_vect_options opt);
template int local_extrema_vect_generic<double, int, int, TYPE_OPERATION_VECT::IMIN_>(const TRUSTVect<double, int>& vx, Mp_vect_options opt);
template int local_extrema_vect_generic<double, int, int, TYPE_OPERATION_VECT::MAX_>(const TRUSTVect<double, int>& vx, Mp_vect_options opt);
template int local_extrema_vect_generic<double, int, int, TYPE_OPERATION_VECT::MIN_>(const TRUSTVect<double, int>& vx, Mp_vect_options opt);
template int local_extrema_vect_generic<double, int, int, TYPE_OPERATION_VECT::MAX_ABS_>(const TRUSTVect<double, int>& vx, Mp_vect_options opt);
template int local_extrema_vect_generic<double, int, int, TYPE_OPERATION_VECT::MIN_ABS_>(const TRUSTVect<double, int>& vx, Mp_vect_options opt);
template int local_extrema_vect_generic<int, int, int, TYPE_OPERATION_VECT::IMAX_>(const TRUSTVect<int, int>& vx, Mp_vect_options opt);
template int local_extrema_vect_generic<int, int, int, TYPE_OPERATION_VECT::IMIN_>(const TRUSTVect<int, int>& vx, Mp_vect_options opt);
template int local_extrema_vect_generic<int, int, int, TYPE_OPERATION_VECT::MAX_>(const TRUSTVect<int, int>& vx, Mp_vect_options opt);
template int local_extrema_vect_generic<int, int, int, TYPE_OPERATION_VECT::MIN_>(const TRUSTVect<int, int>& vx, Mp_vect_options opt);
template int local_extrema_vect_generic<int, int, int, TYPE_OPERATION_VECT::MAX_ABS_>(const TRUSTVect<int, int>& vx, Mp_vect_options opt);
template int local_extrema_vect_generic<int, int, int, TYPE_OPERATION_VECT::MIN_ABS_>(const TRUSTVect<int, int>& vx, Mp_vect_options opt);

#if INT_is_64_ == 2
template double local_extrema_vect_generic<double, trustIdType, double, TYPE_OPERATION_VECT::MAX_ABS_>(const TRUSTVect<double, trustIdType>& vx, Mp_vect_options opt);
template int local_extrema_vect_generic<int, trustIdType, int, TYPE_OPERATION_VECT::MAX_>(const TRUSTVect<int, trustIdType>& vx, Mp_vect_options opt);
template trustIdType local_extrema_vect_generic<trustIdType, trustIdType, trustIdType, TYPE_OPERATION_VECT::MAX_>(const TRUSTVect<trustIdType, trustIdType>& vx, Mp_vect_options opt);
#endif

#ifndef LATATOOLS
namespace
{
template<typename ExecSpace, typename _TYPE_, typename _SIZE_, TYPE_OPERATION_VECT_BIS _TYPE_OP_>
void local_operations_vect_bis_generic_kernel(const TRUSTVect<_TYPE_,_SIZE_>& tab_vx, int nblocs_left,
                                              Block_Iter<_SIZE_>& bloc_itr, const _SIZE_ vect_size_tot, const int line_size, _TYPE_& sum)
{
  static constexpr bool IS_SQUARE = (_TYPE_OP_ == TYPE_OPERATION_VECT_BIS::SQUARE_), IS_SUM = (_TYPE_OP_ == TYPE_OPERATION_VECT_BIS::SOMME_);
  // Performance important point for TRUSTArray dynamic kernel to have serial mode performance:
  // Use pointer access into Kokkos loop with [] and getting raw pointer to view with .data() !
  auto vx = tab_vx.template view_ro<1, ExecSpace>().data();
  auto apply = KOKKOS_LAMBDA(const _SIZE_ item, _TYPE_& local_sum)
  {
    const _TYPE_ x = vx[item];
    if (IS_SQUARE) local_sum += x * x;
    if (IS_SUM)    local_sum += x;
  };
  parallel_reduce_items<ExecSpace, Kokkos::Sum<_TYPE_>>(__KERNEL_NAME__, bloc_itr, nblocs_left, line_size, vect_size_tot, apply, sum);
}
}
#endif

template <typename _TYPE_, typename _SIZE_, TYPE_OPERATION_VECT_BIS _TYPE_OP_ >
_TYPE_ local_operations_vect_bis_generic(const TRUSTVect<_TYPE_,_SIZE_>& vx,Mp_vect_options opt)
{
#ifndef LATATOOLS
  _TYPE_ sum = 0;
  // Master vect donne la structure de reference, les autres vecteurs doivent avoir la meme structure.
  const TRUSTVect<_TYPE_,_SIZE_>& master_vect = vx;
  const int line_size = master_vect.line_size();
  const _SIZE_ vect_size_tot = master_vect.size_totale();
  const MD_Vector& md = master_vect.get_md_vector();
  assert(vx.line_size() == line_size);
  assert(vx.size_totale() == vect_size_tot); // this test is necessary if md is null
  assert(vx.get_md_vector() == md);
  // Determine blocs of data to process, depending on " opt"
  int nblocs_left;
  Block_Iter<_SIZE_> bloc_itr  = ::determine_blocks(opt, md, vect_size_tot, line_size, nblocs_left);
  // Shortcut for empty arrays (avoid case line_size == 0)
  if (bloc_itr.empty()) return sum;

  bool kernelOnDevice = vx.checkDataOnDevice();

  if (kernelOnDevice)
    local_operations_vect_bis_generic_kernel<Kokkos::DefaultExecutionSpace, _TYPE_, _SIZE_, _TYPE_OP_>(vx, nblocs_left, bloc_itr, vect_size_tot, line_size, sum);
  else
    local_operations_vect_bis_generic_kernel<Kokkos::DefaultHostExecutionSpace, _TYPE_, _SIZE_, _TYPE_OP_>(vx, nblocs_left, bloc_itr, vect_size_tot, line_size, sum);

  return sum;
#else
  Cerr << "Error! local_operations_vect_bis_generic can't be called in your project!" << finl;
  Process::exit();
  return (_TYPE_)0; // For compil in latatools
#endif
}
// Explicit instanciation for templates:
template double local_operations_vect_bis_generic<double, int, TYPE_OPERATION_VECT_BIS::SQUARE_>(const TRUSTVect<double, int>& vx,Mp_vect_options opt);
template int local_operations_vect_bis_generic<int, int, TYPE_OPERATION_VECT_BIS::SQUARE_>(const TRUSTVect<int, int>& vx,Mp_vect_options opt);
template float local_operations_vect_bis_generic<float, int, TYPE_OPERATION_VECT_BIS::SQUARE_>(const TRUSTVect<float, int>& vx,Mp_vect_options opt);
template double local_operations_vect_bis_generic<double, int, TYPE_OPERATION_VECT_BIS::SOMME_>(const TRUSTVect<double, int>& vx,Mp_vect_options opt);
template int local_operations_vect_bis_generic<int, int, TYPE_OPERATION_VECT_BIS::SOMME_>(const TRUSTVect<int, int>& vx,Mp_vect_options opt);
template float local_operations_vect_bis_generic<float, int, TYPE_OPERATION_VECT_BIS::SOMME_>(const TRUSTVect<float, int>& vx,Mp_vect_options opt);

#if INT_is_64_ == 2
template double local_operations_vect_bis_generic<double, trustIdType, TYPE_OPERATION_VECT_BIS::SOMME_>(const TRUSTVect<double, trustIdType>& vx,Mp_vect_options opt);
#endif

// ==================================================================================================================================
// BEGIN code for debug
#ifndef NDEBUG
// INVALID_SCALAR is used to fill arrays when values are not computed (virtual space might not be computed by operators).
// The value below probably triggers errors on parallel test cases but does not prevent from doing "useless" computations with it.
#ifndef LATATOOLS
namespace
{
template<typename ExecSpace, typename _TYPE_, typename _SIZE_>
void invalidate_data_kernel(TRUSTVect<_TYPE_,_SIZE_>& tab_resu,
                            const ArrOfInt& items_blocs, const int line_size, const int blocs_size)
{
  _TYPE_ invalid = (_TYPE_)-987654321;
  auto resu = tab_resu.template view_rw<1, ExecSpace>().data();

  int i = 0;
  for (int blocs_idx = 0; blocs_idx < blocs_size; blocs_idx += 2) // process data until beginning of next bloc, or end of array
    {
      const int bloc_end = line_size * items_blocs[blocs_idx];
      //Define Policy
      Kokkos::RangePolicy<ExecSpace> policy(i, bloc_end);
      //Loop
      if (statistics().get_use_gpu()) start_gpu_timer(__KERNEL_NAME__);
      Kokkos::parallel_for(__KERNEL_NAME__, policy, KOKKOS_LAMBDA(const int count)
      {
        resu[count]=invalid;
      });
      if (statistics().get_use_gpu()) end_gpu_timer(__KERNEL_NAME__, is_default_exec_space<ExecSpace>);
      i = items_blocs[blocs_idx+1] * line_size;
    }
  const _SIZE_ bloc_end = tab_resu.size_array(); // Process until end of vector
  //Define Policy
  Kokkos::RangePolicy<ExecSpace> policy(i, bloc_end);
  //Loop
  if (statistics().get_use_gpu()) start_gpu_timer(__KERNEL_NAME__);
  Kokkos::parallel_for(__KERNEL_NAME__, policy, KOKKOS_LAMBDA(const int count)
  {
    resu[count]=invalid;
  });
  if (statistics().get_use_gpu()) end_gpu_timer(__KERNEL_NAME__, is_default_exec_space<ExecSpace>);
}
}
#endif

template <typename _TYPE_, typename _SIZE_>
void invalidate_data(TRUSTVect<_TYPE_,_SIZE_>& resu, Mp_vect_options opt)
{
#ifndef LATATOOLS
  if (Process::is_sequential()) return; // no invalid values in sequential

  const MD_Vector& md = resu.get_md_vector();
  const int line_size = resu.line_size();
  if (opt == VECT_ALL_ITEMS || (!md)) return; // no invalid values
  assert(opt == VECT_SEQUENTIAL_ITEMS || opt == VECT_REAL_ITEMS);
  const ArrOfInt& items_blocs = (opt == VECT_SEQUENTIAL_ITEMS) ? md->get_blocs_items_to_sum() : md->get_blocs_items_to_compute();
  const int blocs_size = items_blocs.size_array();

  bool kernelOnDevice = resu.checkDataOnDevice();

  if (kernelOnDevice)
    invalidate_data_kernel<Kokkos::DefaultExecutionSpace, _TYPE_, _SIZE_>(resu, items_blocs, line_size, blocs_size);
  else
    invalidate_data_kernel<Kokkos::DefaultHostExecutionSpace, _TYPE_, _SIZE_>(resu, items_blocs, line_size, blocs_size);
#else
  Cerr << "Error! invalidate_data can't be called in your project!" << finl;
  Process::exit();
#endif
}
//END code for debug
// ==================================================================================================================================
// Explicit instanciation for templates:
template void invalidate_data<double>(TRUSTVect<double, int>& resu, Mp_vect_options opt);
template void invalidate_data<float>(TRUSTVect<float, int>& resu, Mp_vect_options opt);
template void invalidate_data<int>(TRUSTVect<int, int>& resu, Mp_vect_options opt);
#endif /* NDEBUG */

#ifndef LATATOOLS
namespace
{
template<typename ExecSpace, typename _TYPE_, typename _SIZE_>
void local_prodscal_kernel(const TRUSTVect<_TYPE_,_SIZE_>& tab_vx, const TRUSTVect<_TYPE_,_SIZE_>& tab_vy, int nblocs_left,
                           Block_Iter<_SIZE_>& bloc_itr, const _SIZE_ vect_size_tot, const int line_size, _TYPE_& sum)
{
  auto vx = tab_vx.template view_ro<1, ExecSpace>().data();
  auto vy = tab_vy.template view_ro<1, ExecSpace>().data();
  auto apply = KOKKOS_LAMBDA(const _SIZE_ item, _TYPE_& local_sum)
  {
    local_sum += vx[item] * vy[item];
  };
  parallel_reduce_items<ExecSpace, Kokkos::Sum<_TYPE_>>(__KERNEL_NAME__, bloc_itr, nblocs_left, line_size, vect_size_tot, apply, sum);
}
}
#endif

template<typename _TYPE_, typename _SIZE_>
_TYPE_ local_prodscal(const TRUSTVect<_TYPE_,_SIZE_>& vx, const TRUSTVect<_TYPE_,_SIZE_>& vy, Mp_vect_options opt)
{
#ifndef LATATOOLS
  _TYPE_ sum = 0;

  const int line_size = vx.line_size();
  const _SIZE_ vect_size_tot = vx.size_totale();
  const MD_Vector& md = vx.get_md_vector();

  assert(vx.line_size() == line_size && vy.line_size() == line_size);
  assert(vx.size_totale() == vect_size_tot && vy.size_totale() == vect_size_tot); // this test is necessary if md is null
  assert(vx.get_md_vector() == md && vy.get_md_vector() == md);
  // Determine blocs of data to process, depending on " opt"
  int nblocs_left;
  Block_Iter<_SIZE_> bloc_itr  = ::determine_blocks(opt, md, vect_size_tot, line_size, nblocs_left);
  // Shortcut for empty arrays (avoid case line_size == 0)
  if (bloc_itr.empty()) return sum;

  bool kernelOnDevice = const_cast<TRUSTVect<_TYPE_,_SIZE_>&>(vx).checkDataOnDevice(vy);

  if (kernelOnDevice)
    local_prodscal_kernel<Kokkos::DefaultExecutionSpace, _TYPE_, _SIZE_>(vx, vy, nblocs_left, bloc_itr, vect_size_tot,line_size, sum);
  else
    local_prodscal_kernel<Kokkos::DefaultHostExecutionSpace, _TYPE_, _SIZE_>(vx, vy, nblocs_left, bloc_itr, vect_size_tot,line_size, sum);

  return sum;

#else
  Cerr << "Error! local_prodscal can't be called in your project!" << finl;
  Process::exit();
  return (_TYPE_)0; // For compil in latatools
#endif
}
// Explicit instanciation for templates:
template double local_prodscal(const TRUSTVect<double, int>& vx, const TRUSTVect<double, int>& vy, Mp_vect_options opt);
template float local_prodscal(const TRUSTVect<float, int>& vx, const TRUSTVect<float, int>& vy, Mp_vect_options opt);

#ifndef LATATOOLS

namespace
{
template <typename ExecSpace, typename _TYPE_>
void local_carre_norme_tab_kernel(const TRUSTTab<_TYPE_>& tableau, TRUSTArray<_TYPE_>& norme_colonne)
{
  const TRUSTVect<_TYPE_,int>& vect = tableau;
  const int lsize = vect.line_size(), vect_size_tot = vect.size_totale();
  assert(lsize == norme_colonne.size_array());

  int nblocs_left;
  Block_Iter<int> bloc_itr = ::determine_blocks(VECT_SEQUENTIAL_ITEMS, tableau.get_md_vector(), vect_size_tot, lsize, nblocs_left);

  for (int j = 0; j < lsize; j++) norme_colonne[j] = 0;

  auto tableau_view = tableau.template view_ro<2, ExecSpace>();
  auto norme_colonne_view = norme_colonne.template view_rw<1, ExecSpace>();
#ifdef TRUST_USE_GPU
  if (nblocs_left>3) ToDo_Kokkos("nblocs_left too high, optimize by rewriting as local_operations_vect_bis_generic_kernel");
#endif
  for (; nblocs_left; nblocs_left--)
    {
      const int begin_bloc = (*(bloc_itr++)), end_bloc = (*(bloc_itr++));
      if (begin_bloc<end_bloc) // very important: empty bloc at the end would erase norme_colonne
        {
          Kokkos::RangePolicy<ExecSpace> policy(begin_bloc, end_bloc);

          for (int j=0; j<lsize; j++) //Outer loop
            {
              if (statistics().get_use_gpu()) start_gpu_timer(__KERNEL_NAME__);
              Kokkos::parallel_reduce(policy,
                                      KOKKOS_LAMBDA(const int i, _TYPE_& local_sum)
              {
                const _TYPE_ x = tableau_view(i,j);
                local_sum += x*x;
              },
              //Reduce in a subview, enabled by specifying execspace in the reducer !
              Kokkos::Sum<_TYPE_, ExecSpace>(Kokkos::subview(norme_colonne_view,j)));

              bool kernelOnDevice = is_default_exec_space<ExecSpace>;
              if (statistics().get_use_gpu()) end_gpu_timer(__KERNEL_NAME__, kernelOnDevice);
            }
        }
    }
}

}

template <typename _TYPE_>
void local_carre_norme_tab(const TRUSTTab<_TYPE_>& tableau, TRUSTArray<_TYPE_>& norme_colonne)
{
  norme_colonne = 0.;
  bool kernelOnDevice = tableau.checkDataOnDevice();

  if (kernelOnDevice)
    local_carre_norme_tab_kernel<Kokkos::DefaultExecutionSpace, _TYPE_>(tableau, norme_colonne);
  else
    local_carre_norme_tab_kernel<Kokkos::DefaultHostExecutionSpace, _TYPE_>(tableau, norme_colonne);
}

namespace
{
template <typename ExecSpace, typename _TYPE_>
void local_max_abs_tab_kernel(const TRUSTTab<_TYPE_>& tableau, TRUSTArray<_TYPE_>& max_colonne)
{
  const TRUSTVect<_TYPE_,int>& vect = tableau;
  const int lsize = vect.line_size(), vect_size_tot = vect.size_totale();
  assert(lsize == max_colonne.size_array());

  int nblocs_left;
  Block_Iter<int> bloc_itr = ::determine_blocks(VECT_REAL_ITEMS, tableau.get_md_vector(), vect_size_tot, lsize, nblocs_left);

  for (int j = 0; j < lsize; j++) max_colonne[j] = 0;
  assert(lsize == max_colonne.size_array());

  auto tableau_view= tableau.template view_ro<2, ExecSpace>();
  auto max_colonne_view= max_colonne.template view_rw<1, ExecSpace>();
#ifdef TRUST_USE_GPU
  if (nblocs_left>3) ToDo_Kokkos("nblocs_left too high, optimize by rewriting as local_operations_vect_bis_generic_kernel");
#endif
  for (; nblocs_left; nblocs_left--)
    {
      const int begin_bloc = (*(bloc_itr++)), end_bloc = (*(bloc_itr++));
      if (begin_bloc<end_bloc) // very important: empty bloc at the end would erase max_colonne
        {
          Kokkos::RangePolicy<ExecSpace> policy(begin_bloc, end_bloc);

          for (int j=0; j<lsize; j++) //Outer loop
            {
              if (statistics().get_use_gpu()) start_gpu_timer(__KERNEL_NAME__);
              Kokkos::parallel_reduce(policy,
                                      KOKKOS_LAMBDA(const int i, _TYPE_& local_max)
              {
                const _TYPE_ x = Kokkos::fabs(tableau_view(i,j));
                local_max = Kokkos::fmax(local_max, x);
              },
              //Reduce in a subview, enabled by specifying execspace in the reducer !
              Kokkos::Max<_TYPE_, ExecSpace>(Kokkos::subview(max_colonne_view,j)));

              bool kernelOnDevice = is_default_exec_space<ExecSpace>;
              if (statistics().get_use_gpu()) end_gpu_timer(__KERNEL_NAME__, kernelOnDevice);
            }
        }
    }
}

}

template <typename _TYPE_>
void local_max_abs_tab(const TRUSTTab<_TYPE_>& tableau, TRUSTArray<_TYPE_>& max_colonne)
{
  max_colonne = std::numeric_limits<_TYPE_>::min();
  bool kernelOnDevice = tableau.checkDataOnDevice();

  if (kernelOnDevice)
    local_max_abs_tab_kernel<Kokkos::DefaultExecutionSpace, _TYPE_>(tableau, max_colonne);
  else
    local_max_abs_tab_kernel<Kokkos::DefaultHostExecutionSpace, _TYPE_>(tableau, max_colonne);
}

template void local_carre_norme_tab<double>(const TRUSTTab<double,int>& tableau, TRUSTArray<double,int>& norme_colonne);
template void local_carre_norme_tab<float>(const TRUSTTab<float,int>& tableau, TRUSTArray<float,int>& norme_colonne);
template void local_max_abs_tab<double>(const TRUSTTab<double,int>& tableau, TRUSTArray<double,int>& max_colonne);
template void local_max_abs_tab<float>(const TRUSTTab<float,int>& tableau, TRUSTArray<float,int>& max_colonne);

template <typename _T_, typename _SIZE_>
void ajoute_produit_tensoriel(_T_ alpha, TRUSTVect<_T_,_SIZE_>& tab_v, const TRUSTVect<_T_,_SIZE_>& tab_x, const TRUSTVect<_T_,_SIZE_>& tab_y,
                              int nblocs_left, Block_Iter<_SIZE_> bloc_itr,
                              int line_size_x, int line_size_y, int line_size_xy)
{
  auto x = tab_x.template view_ro<1>().data();
  auto y = tab_y.template view_ro<1>().data();
  auto v = tab_v.template view_rw<1>().data();
#ifdef TRUST_USE_GPU
  if (nblocs_left > 3) ToDo_Kokkos("nblocs_left too high, optimize by rewriting as ajoute_produit_tensoriel_kernel");
#endif
  for (; nblocs_left; nblocs_left--)
    {
      const _SIZE_ debut = (*(bloc_itr++)), fin = (*(bloc_itr++));
      Kokkos::parallel_for(start_gpu_timer(__KERNEL_NAME__), range_3D({debut, 0, 0}, {fin, line_size_x, line_size_y}),
                           KOKKOS_LAMBDA(const int i, const int j, const int k)
      {
        v[i * line_size_xy + j * line_size_y + k] += alpha * x[i * line_size_x + j] * y[i * line_size_y + k];
      });
      end_gpu_timer(__KERNEL_NAME__);
    }
}

template void ajoute_produit_tensoriel<double, int>(double alpha, TRUSTVect<double,int>& v, const TRUSTVect<double,int>& vx, const TRUSTVect<double,int>& vy, int nblocs_left, Block_Iter<int> bloc_itr, int line_size_x, int line_size_y, int line_size_xy);
template void ajoute_produit_tensoriel<float,  int>(float  alpha, TRUSTVect<float, int>& v, const TRUSTVect<float, int>& vx, const TRUSTVect<float, int>& vy, int nblocs_left, Block_Iter<int> bloc_itr, int line_size_x, int line_size_y, int line_size_xy);

#endif