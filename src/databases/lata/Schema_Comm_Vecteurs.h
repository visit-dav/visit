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

#ifndef Schema_Comm_Vecteurs_included
#define Schema_Comm_Vecteurs_included

#include <TRUSTArray.h>

enum IsExchangeBlocking
{
  DefaultBlocking,
  NonBlockingStart,
  NonBlockingFinish
};

class Schema_Comm_Vecteurs_Static_Data;

/*! @brief Utility class used notably by MD_Vector::echange_espace_virtuel() methods.
 *
 * Allows exchanging blocks of ints or doubles with other processors,
 * accessible via arrays that are read and written directly
 *   (unlike Schema_Comm which uses readOn and printOn, which is slower).
 *   For performance reasons, communication is split into two phases:
 *   - buffer size definition (allows pre-allocating buffers):
 *      begin_init()
 *      add_send/recv_area_int/double(processor, size)
 *       (declaration of types, sizes and destination processors for blocks to be sent;
 *        multiple blocks of identical or different types can be sent to each processor)
 *      end_init()
 *   - data exchange (as many times as desired):
 *      begin_comm()
 *      for(each block to send) {
 *       ArrOfInt/Double & buf = get_next_area_int/double(pe, size);
 *       for (i=0; i<size; i++)
 *         buf[i] = ...
 *      }
 *      exchange();
 *      for (each block to recv) {
 *        ... get_next_area_int/double(...)
 *      end_comm();
 *
 */
extern bool check_comm_vector;
class Schema_Comm_Vecteurs
{
public:
  Schema_Comm_Vecteurs();
  ~Schema_Comm_Vecteurs();
  void begin_init();

  template <typename _TYPE_>
  inline void add_send_area_template(int pe, int size);

  template <typename _TYPE_>
  inline void add_recv_area_template(int pe, int size);

  template <typename _TYPE_>
  inline TRUSTArray<_TYPE_>& get_next_area_template(int pe, int array_size);

  void end_init();
  void begin_comm(bool bufferOnDevice=false);
  void exchange(IsExchangeBlocking exchange_type = IsExchangeBlocking::DefaultBlocking, const std::string kernel_name="noname");
  void end_comm();

  static void CleanMyStaticViews();

protected:
  inline void add(int pe, int size, ArrOfInt& procs, ArrOfInt& buf_sizes, int align_size);
  int check_buffers_full() const;
  int check_next_area(int pe, int byte_size) const;

  // For each processor in send_proc or recv_proc_, total buffer size in bytes.
  // During the begin_init() phase, these two arrays are of size nproc(), with zero value
  //  for processors we don't communicate with.
  // Afterwards, they are the same size as send_procs_ and recv_procs_.
  ArrOfInt send_buf_sizes_;
  ArrOfInt recv_buf_sizes_;
  // List of processors to which we send data
  ArrOfInt send_procs_;
  ArrOfInt recv_procs_;
  // After the initialization phase, are the processors in ascending order?
  int sorted_ = 1;
  // Buffer size required for this schema
  int min_buf_size_ = -1;
  // Buffer packing/uncpacking on device:
  bool bufferOnDevice_ = false;
  // Support GPU par MPI:
  bool use_gpu_aware_mpi_ = false;

  enum Status { RESET, BEGIN_INIT, END_INIT, BEGIN_COMM, EXCHANGED };
  Status status_;

  // Is the global buffer currently in use?
  static bool buffer_locked_;
  // Temporary read/write areas, returned by get_next... and pointing into buffer_
  static ArrOfDouble tmp_area_double_;
  static ArrOfFloat tmp_area_float_;
  static ArrOfInt tmp_area_int_;
#if INT_is_64_ == 2
  static ArrOfTID tmp_area_tid_;
#endif

  // Class containing malloc arrays (for automatic destruction at end of execution)
  static Schema_Comm_Vecteurs_Static_Data sdata_;
};

/*! @brief Static data shared by all Schema_Comm_Vecteur classes, with destructor to free memory at end of execution.
 *
 */
class Schema_Comm_Vecteurs_Static_Data
{
public:
  Schema_Comm_Vecteurs_Static_Data();
  ~Schema_Comm_Vecteurs_Static_Data();
  void init(int size, bool bufferOnDevice);

  char  *buffer_base_;
  int buffer_base_size_;
  int buffer_base_device_size_;
  int buf_pointers_size_;
  // For each processor between 0 and nproc(), address of the next data to read/write
  // from this proc in the buffer array
  char **buf_pointers_;
};

// Size in bytes of a block of sz ints, rounded up to the next 8 bytes
#ifdef INT_is_64_
#if INT_is_64_ == 1
#define BLOCSIZE_INT(sz) (sz<<3)   // == sz*8
#else
#define BLOCSIZE_INT(sz) (sz<<2)   // == sz*4
#define BLOCSIZE_TID(sz) (sz<<3)   // == sz*8
#endif
#else
#define BLOCSIZE_INT(sz) (sz<<2)   // == sz*4
#endif

#define BLOCSIZE_DOUBLE(sz) (sz<<3)
#define BLOCSIZE_FLOAT(sz) (sz<<2)
#define ALIGN_SIZE(ptr,sz) ptr=sdata_.buffer_base_+((ptr-sdata_.buffer_base_+(sz-1))&(~(sz-1)))

inline void Schema_Comm_Vecteurs::add(int pe, int size, ArrOfInt& procs, ArrOfInt& buf_sizes, int align_size)
{
  assert(status_ == BEGIN_INIT);
  assert(size >= 0);
  int& x = buf_sizes[pe];
  if (x == 0 && size > 0)
    {
      const int n = procs.size_array();
      if (n > 0 && procs[n - 1] > pe)
        sorted_ = 0;
      procs.append_array(pe);
    }
  x = ((x + align_size - 1) & (~(align_size - 1))) + size; // Padding before block
}

inline void Schema_Comm_Vecteurs::CleanMyStaticViews()
{
#ifdef KOKKOS //If Kokkos is defined, we can clear the views
  tmp_area_double_.CleanMyView();
  tmp_area_float_.CleanMyView();
  tmp_area_int_.CleanMyView();
#endif
  return;
}
template<>
inline void Schema_Comm_Vecteurs::add_send_area_template<int>(int pe, int size)
{
  add(pe, BLOCSIZE_INT(size), send_procs_, send_buf_sizes_, sizeof(int));
}

template<>
inline void Schema_Comm_Vecteurs::add_send_area_template<double>(int pe, int size)
{
  add(pe, BLOCSIZE_DOUBLE(size), send_procs_, send_buf_sizes_, sizeof(double));
}

template<>
inline void Schema_Comm_Vecteurs::add_send_area_template<float>(int pe, int size)
{
  add(pe, BLOCSIZE_FLOAT(size), send_procs_, send_buf_sizes_, sizeof(float));
}

template<>
inline void Schema_Comm_Vecteurs::add_recv_area_template<int>(int pe, int size)
{
  add(pe, BLOCSIZE_INT(size), recv_procs_, recv_buf_sizes_, sizeof(int));
}

template<>
inline void Schema_Comm_Vecteurs::add_recv_area_template<double>(int pe, int size)
{
  add(pe, BLOCSIZE_DOUBLE(size), recv_procs_, recv_buf_sizes_, sizeof(double));
}

template<>
inline void Schema_Comm_Vecteurs::add_recv_area_template<float>(int pe, int size)
{
  add(pe, BLOCSIZE_FLOAT(size), recv_procs_, recv_buf_sizes_, sizeof(float));
}

#if INT_is_64_ == 2
template<>
inline void Schema_Comm_Vecteurs::add_send_area_template<trustIdType>(int pe, int size)
{
  add(pe, BLOCSIZE_TID(size), send_procs_, send_buf_sizes_, sizeof(trustIdType));
}

template<>
inline void Schema_Comm_Vecteurs::add_recv_area_template<trustIdType>(int pe, int size)
{
  add(pe, BLOCSIZE_TID(size), recv_procs_, recv_buf_sizes_, sizeof(trustIdType));
}
#endif

/*! @brief Returns an array containing the next "size" values received from processor pe during the current communication.
 *
 *   Warning:
 *   The returned array is a reference to an internal array that is only valid
 *   until the next call to a get_next_xxx method.
 *
 */
template<>
inline ArrOfInt& Schema_Comm_Vecteurs::get_next_area_template<int>(int pe, int size)
{
  ALIGN_SIZE(sdata_.buf_pointers_[pe], sizeof(int));
  assert(check_next_area(pe, BLOCSIZE_INT(size)));
  int *bufptr = (int *) (sdata_.buf_pointers_[pe]);
  // caution with pointer arithmetic, adding a size in bytes
  sdata_.buf_pointers_[pe] += BLOCSIZE_INT(size);
  tmp_area_int_.ref_data(bufptr, size);
  tmp_area_int_.set_data_location(bufferOnDevice_ ? DataLocation::Device : DataLocation::HostOnly);
  return tmp_area_int_;
}

#if INT_is_64_ == 2
template<>
inline ArrOfTID& Schema_Comm_Vecteurs::get_next_area_template<trustIdType>(int pe, int size)
{
  ALIGN_SIZE(sdata_.buf_pointers_[pe], sizeof(trustIdType));
  assert(check_next_area(pe, BLOCSIZE_TID(size)));
  trustIdType *bufptr = (trustIdType *) (sdata_.buf_pointers_[pe]);
  // caution with pointer arithmetic, adding a size in bytes
  sdata_.buf_pointers_[pe] += BLOCSIZE_TID(size);
  tmp_area_tid_.ref_data(bufptr, size);
  return tmp_area_tid_;
}
#endif

template<>
inline ArrOfDouble& Schema_Comm_Vecteurs::get_next_area_template<double>(int pe, int size)
{
  ALIGN_SIZE(sdata_.buf_pointers_[pe], sizeof(double));
  assert(check_next_area(pe, BLOCSIZE_DOUBLE(size)));
  double *bufptr = (double *) (sdata_.buf_pointers_[pe]);
  // caution with pointer arithmetic, adding a size in bytes
  sdata_.buf_pointers_[pe] += BLOCSIZE_DOUBLE(size);
  tmp_area_double_.ref_data(bufptr, size);
  tmp_area_double_.set_data_location(bufferOnDevice_ ? DataLocation::Device : DataLocation::HostOnly);
  if (check_comm_vector)
    {
#ifndef NDEBUG
      // in debug, put dummy values in the array
      if (status_ != EXCHANGED)
        tmp_area_double_ = DMAXFLOAT * 0.999;
#endif
    }
  return tmp_area_double_;
}

template<>
inline ArrOfFloat& Schema_Comm_Vecteurs::get_next_area_template<float>(int pe, int size)
{
  ALIGN_SIZE(sdata_.buf_pointers_[pe], sizeof(float));
  assert(check_next_area(pe, BLOCSIZE_FLOAT(size)));
  float *bufptr = (float *) (sdata_.buf_pointers_[pe]);
  // caution with pointer arithmetic, adding a size in bytes
  sdata_.buf_pointers_[pe] += BLOCSIZE_FLOAT(size);
  tmp_area_float_.ref_data(bufptr, size);
  tmp_area_float_.set_data_location(bufferOnDevice_ ? DataLocation::Device : DataLocation::HostOnly);
  return tmp_area_float_;
}

#undef BLOCSIZE_INT
#undef BLOCSIZE_DOUBLE
#undef BLOCSIZE_FLOAT

#endif /* Schema_Comm_Vecteurs_included */
