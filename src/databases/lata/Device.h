/****************************************************************************
* Copyright (c) 2025, CEA
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

#ifndef Device_included
#define Device_included

#include <TRUSTTabs_forward.h>
#include <Nom.h>

#ifndef LATATOOLS
#include <Perf_counters.h>
#include <kokkos++.h>
#endif

#ifdef TRUST_USE_CUDA
// See https://nvidia.github.io/NVTX/
// See https://stackoverflow.com/questions/23230003/something-between-func-and-pretty-function/29856690#29856690
#include <nvtx3/nvToolsExt.h>
#endif
#ifdef TRUST_USE_ROCM
// See https://nvidia.github.io/NVTX/
// See https://stackoverflow.com/questions/23230003/something-between-func-and-pretty-function/29856690#29856690
#include <rocprofiler-sdk-roctx/roctx.h>
#endif

/*
extern bool init_device_, clock_on, timer;
extern double clock_start;
extern int timer_counter;
*/

void self_test();
void init_device();
void init_cuda();
std::string ptrToString(const void* adr);

#ifdef TRUST_USE_GPU
#define ToDo_Kokkos(str)                              \
        Cerr << "[Kokkos " << str << "] Warning, code running slow cause not ported yet: line " << __LINE__ << " in " << __FILE__ << finl;
#else
#define ToDo_Kokkos(str)
#endif

// Macro Kernel_Name
#ifdef TRUST_USE_GPU
inline const std::string methodName(const std::string& prettyFunction, const int line)
{
  size_t colons = prettyFunction.find("::");
  if (colons != std::string::npos)
    {
      // method::class:line
      size_t begin = prettyFunction.substr(0, colons).rfind(" ") + 1;
      size_t end = prettyFunction.rfind("(") - begin;
      return prettyFunction.substr(begin, end) + ":" + std::to_string(line);
    }
  else
    {
      // return_type function:line
      colons = prettyFunction.find('(');
      return prettyFunction.substr(0, colons) + ":" + std::to_string(line);
    }
}
#define __KERNEL_NAME__ methodName(__PRETTY_FUNCTION__,__LINE__)
#else
#define __KERNEL_NAME__ ""
#endif

extern std::string start_gpu_timer(std::string str="kernel", int bytes=-1);

extern void end_gpu_timer(const std::string& str, int onDevice=1, int bytes=-1); // Return in [ms]

template <typename _TYPE_>
extern _TYPE_* addrOnDevice(_TYPE_* ptr);

template <typename _TYPE_, typename _SIZE_=int>
extern _TYPE_* addrOnDevice(TRUSTArray<_TYPE_,_SIZE_>& tab);

template <typename _TYPE_, typename _SIZE_=int>
inline const _TYPE_* addrOnDevice(const TRUSTArray<_TYPE_,_SIZE_>& tab)
{
  return addrOnDevice(const_cast<TRUSTArray<_TYPE_,_SIZE_>&>(tab));
}

template <typename _TYPE_, typename _SIZE_=int>
extern _TYPE_* allocateOnDevice(TRUSTArray<_TYPE_,_SIZE_>& tab);

template <typename _TYPE_, typename _SIZE_=int>
inline const _TYPE_* allocateOnDevice(const TRUSTArray<_TYPE_,_SIZE_>& tab)
{
  return allocateOnDevice(const_cast<TRUSTArray<_TYPE_,_SIZE_>&>(tab));
}

template <typename _TYPE_>
extern bool isAllocatedOnDevice(_TYPE_* tab_addr);

template <typename _TYPE_, typename _SIZE_=int>
extern bool isAllocatedOnDevice(TRUSTArray<_TYPE_,_SIZE_>& tab);

template <typename _TYPE_, typename _SIZE_=int>
extern void deleteOnDevice(TRUSTArray<_TYPE_,_SIZE_>& tab);

template <typename _TYPE_, typename _SIZE_=int>
extern const _TYPE_* mapToDevice(const TRUSTArray<_TYPE_,_SIZE_>& tab);

template <typename _TYPE_, typename _SIZE_=int>
extern _TYPE_* mapToDevice_(TRUSTArray<_TYPE_,_SIZE_>& tab, DataLocation nextLocation);

template <typename _TYPE_, typename _SIZE_=int>
extern _TYPE_* computeOnTheDevice(TRUSTArray<_TYPE_,_SIZE_>& tab);

template <typename _TYPE_, typename _SIZE_=int>
extern void copyFromDevice(TRUSTArray<_TYPE_,_SIZE_>& tab);

template <typename _TYPE_, typename _SIZE_=int>
extern void copyFromDevice(const TRUSTArray<_TYPE_,_SIZE_>& tab);

template <typename _TYPE_, typename _SIZE_=int>
extern _TYPE_* allocateOnDevice(_TYPE_* ptr, _SIZE_ size);

template <typename _TYPE_, typename _SIZE_=int>
extern void deleteOnDevice(_TYPE_* ptr, _SIZE_ size);

template <typename _TYPE_, typename _SIZE_=int>
extern void copyToDevice(_TYPE_* ptr, _SIZE_ size);

template <typename _TYPE_, typename _SIZE_=int>
extern void copyFromDevice(_TYPE_* ptr, _SIZE_ size);

#endif
