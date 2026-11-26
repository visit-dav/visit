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

#ifndef Process_included
#define Process_included

#include <TRUST_Version.h>  // so that it is accessible from everywhere in TRUST
#include <TRUSTTabs_forward.h>
#include <arch.h>

#ifdef LATATOOLS
#include <string>
#include <stdlib.h>
#include <stdexcept>
#else
#include <kokkos++.h>
#endif

class Objet_U;
class Nom;
class Sortie;

int get_disable_stop();
void change_disable_stop(int new_stop);

/*! @brief Base class of TRUST (in particular Objet_U).
 *
 * It provides a few basic services accessible from anywhere in the code.
 *    These services were historically non-static methods; since all of them are now static,
 *    this class has no other purpose than to group those methods.
 *
 * @sa Objet_U
 */

class Process
{
public:
  virtual ~Process() { }

  // Simplified dummy API for lata_tools
#ifdef LATATOOLS
  static int me() { return 0; }
  static int nproc()  { return 1; }
  static bool is_parallel()  { return false; }
  static void exit(int exit_code = -1) { throw std::invalid_argument("an error is occured"); }
  static void exit(const std::string& s) {  throw std::invalid_argument(std::string("an error is occured ")+s); }
  static double mp_sum(double x) { return x; }
  static float mp_sum(float x) { return x; }
  static double mp_max(double x) { return x; }
  static int mp_max(int x) { return x; }
  static double mp_min(double x) { return x; }
  static int mp_min(int x) { return x; }
  static trustIdType mp_sum(int x) { return x; }
  static trustIdType mp_sum(trustIdType x) { return x; }
  template<typename _TYPE_>
  static void mp_sum_for_each_item(TRUSTArray<_TYPE_>& x) { }
  template<typename _TYPE_>
  static void mp_max_for_each_item(TRUSTArray<_TYPE_>& x) { }
  template<typename _TYPE_>
  static void mp_min_for_each_item(TRUSTArray<_TYPE_>& x) { }
#else
  static int me(); /* my rank in the current group */
  static int nproc();
  static bool is_parallel();
  static void exit(int exit_code = -1);

  //
  // Reduction
  //
  static double mp_sum(double);
  static float mp_sum(float);
  static trustIdType mp_sum(trustIdType);
  static double mp_max(double);
  static double mp_min(double);
  static int mp_max(int);
  static int mp_min(int);
#if INT_is_64_ == 2
  // Careful, the sum of many 'int' on several procs, might return a 'long'!!
  static trustIdType mp_sum(int v) { return mp_sum(static_cast<trustIdType>(v)); }
  static trustIdType mp_max(trustIdType);
  static trustIdType mp_min(trustIdType);
#endif

  // When computing percentages or ratios, useful:
  static double mp_sum_as_double(int v) { return static_cast<double>(mp_sum(v)); }
#if INT_is_64_ == 2
  static double mp_sum_as_double(trustIdType v) { return static_cast<double>(mp_sum(v)); }
#endif

  // Summing for all procs before me:
  static trustIdType mppartial_sum(trustIdType i);
#if INT_is_64_ == 2
  static trustIdType mppartial_sum(int i) { return mppartial_sum(static_cast<trustIdType>(i)); }
#endif

  // Reduction on several values (C++14 compatible - explicit overloads)
  // mp_sum_for_each
  template<typename T>
  static void mp_sum_for_each(T& arg1, T& arg2);
  template<typename T>
  static void mp_sum_for_each(T& arg1, T& arg2, T& arg3);
  template<typename T>
  static void mp_sum_for_each(T& arg1, T& arg2, T& arg3, T& arg4);
  template<typename T>
  static void mp_sum_for_each(T& arg1, T& arg2, T& arg3, T& arg4, T& arg5);
  // mp_max_for_each
  template<typename T>
  static void mp_max_for_each(T& arg1, T& arg2);
  template<typename T>
  static void mp_max_for_each(T& arg1, T& arg2, T& arg3);
  template<typename T>
  static void mp_max_for_each(T& arg1, T& arg2, T& arg3, T& arg4);
  template<typename T>
  static void mp_max_for_each(T& arg1, T& arg2, T& arg3, T& arg4, T& arg5);
  // mp_min_for_each
  template<typename T>
  static void mp_min_for_each(T& arg1, T& arg2);
  template<typename T>
  static void mp_min_for_each(T& arg1, T& arg2, T& arg3);
  template<typename T>
  static void mp_min_for_each(T& arg1, T& arg2, T& arg3, T& arg4);
  template<typename T>
  static void mp_min_for_each(T& arg1, T& arg2, T& arg3, T& arg4, T& arg5);
  // Reduction on values of a TRUSTArray
  template<typename _TYPE_>
  static void mp_sum_for_each_item(TRUSTArray<_TYPE_>& x, int n=-1);
  template<typename _TYPE_>
  static void mp_max_for_each_item(TRUSTArray<_TYPE_>& x, int n=-1);
  template<typename _TYPE_>
  static void mp_min_for_each_item(TRUSTArray<_TYPE_>& x, int n=-1);

  static bool mp_and(bool);
  static bool mp_or(bool);

  static int check_int_overflow(trustIdType);

  static int je_suis_maitre();
  KOKKOS_INLINE_FUNCTION static void Kokkos_exit(const char*);
  static int node_master();
  static void exit(const Nom& message, int exit_code = -1);
  static bool is_sequential(); // serial ?
  static void barrier();
  static void abort();

  static Sortie& Journal(int message_level = 0);
  static double ram_processeur();
  static void imprimer_ram_totale(int all_process = 0);
  //static void print_allocated_memory(std::string s); // To look for memory increase
  static int exception_sur_exit;
  static int multiple_files;
  static bool force_single_file(const int ranks, const Nom& filename);
#endif
};

/*! @brief Exit routine for TRUST within a Kokkos region.
 *
 * @param str Error message to display.
 */
#ifndef LATATOOLS
KOKKOS_INLINE_FUNCTION void Process::Kokkos_exit(const char* str)
{
#ifdef KOKKOS
  // ToDo Kokkos: try to exit more properly on device...
  Kokkos::abort(str);
  //Kokkos::finalize();
#else
  Process::exit(str);
#endif
}
#endif

#endif /* Process_included */

