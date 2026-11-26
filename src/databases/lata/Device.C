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

#include <TRUSTArray.h>
#include <Device.h>
#ifdef TRUST_USE_GPU
#include <DeviceMemory.h>
#endif
#include <ctime>
#include <string>
#include <sstream>
#include <map>
#include <tuple>
#include <chrono>

#ifndef LATATOOLS
#include <Perf_counters.h>
#include <kokkos++.h>
#include <comm_incl.h>
#include <Comm_Group_MPI.h>
#include <PE_Groups.h>
#endif

std::string ptrToString(const void* adr)
{
  std::stringstream ss;
  ss << adr;
  return ss.str();
}

// See AmgXWrapper (src/init.cpp)
int AmgXWrapperScheduling(int rank, int nRanks, int nDevs)
{
  int devID;
  if (nRanks <= nDevs) // Less process than devices
    devID = rank;
  else // More processes than devices
    {
      int nBasic = nRanks / nDevs,
          nRemain = nRanks % nDevs;
      if (rank < (nBasic+1)*nRemain)
        devID = rank / (nBasic + 1);
      else
        devID = (rank - (nBasic+1)*nRemain) / nBasic + nRemain;
    }
  return devID;
}


#ifdef TRUST_USE_GPU
void init_device()
{
  if (statistics().get_init_device()) return;
  statistics().set_init_device(true);
  if (getenv("TRUST_CLOCK_ON")!= nullptr) statistics().set_gpu_verbose(true);
  if (getenv("TRUST_DISABLE_FENCE")!=nullptr) statistics().set_gpu_fence(false);
  Process::imprimer_ram_totale(); // Print before copying data to GPU
}
#endif

#ifndef LATATOOLS
#ifdef TRUST_USE_CUDA
#include <cuda_runtime.h>
void init_cuda()
{
  // Necessary on JeanZay to use GPU Direct (http://www.idris.fr/jean-zay/gpu/jean-zay-gpu-mpi-cuda-aware-gpudirect.html)
  // but performance is worse (trust PAR_gpu_3D 2) so disabled pending further tests:
  // Absolutely necessary on JeanZay (if OpenMPI-Cuda because otherwise crashes during IO)
  // See: https://www.open-mpi.org/faq/?category=runcuda#mpi-cuda-aware-support to enable or disable at compile time!
#if defined(MPIX_CUDA_AWARE_SUPPORT) && MPIX_CUDA_AWARE_SUPPORT
  char* local_rank_env;
  cudaError_t cudaRet;
  /* Retrieve the local rank of the process via the environment variable
     set by Slurm; using MPI_Comm_rank is not yet possible
     since this routine is called BEFORE MPI initialization */
  // ToDo could be called later in AmgX or PETSc GPU...
  local_rank_env = getenv("SLURM_LOCALID");
  if (local_rank_env)
    {
      int rank = atoi(local_rank_env);
      int nRanks = atoi(getenv("SLURM_NTASKS"));
      if (rank==0) printf("The MPI library has CUDA-aware support and TRUST will try using this feature...\n");
      /* Define the GPU to use for each MPI process */
      int nDevs = 0;
      cudaGetDeviceCount(&nDevs);
      int devID = AmgXWrapperScheduling(rank, nRanks, nDevs);
      cudaRet = cudaSetDevice(devID);
      if(cudaRet != cudaSuccess)
        {
          printf("Error: cudaSetDevice failed\n");
          abort();
        }
      else
        {
          if (rank==0) printf("init_cuda() done!");
          cerr << "[MPI] Assigning rank " << rank << " to device " << devID << endl;
        }
    }
  else
    {
      printf("Error : can't guess the local rank of the task\n");
      abort();
    }
#endif     /* MPIX_CUDA_AWARE_SUPPORT */
}
#endif /* TRUST_USE_CUDA */
#endif /* LATATOOLS */

// Address on device (return host adress if no device):
template <typename _TYPE_>
_TYPE_* addrOnDevice(_TYPE_* ptr)
{
#ifdef TRUST_USE_GPU
  _TYPE_ *device_ptr = static_cast<_TYPE_*>(DeviceMemory::addrOnDevice(ptr));
  return device_ptr;
#else
  return ptr;
#endif
}
template <typename _TYPE_, typename _SIZE_>
_TYPE_* addrOnDevice(TRUSTArray<_TYPE_,_SIZE_>& tab)
{
#ifdef TRUST_USE_GPU
  if (tab.get_data_location()==DataLocation::HostOnly) return tab.data();
  else return addrOnDevice(tab.data());
#else
  return tab.data();
#endif
}

// Allocated ?
template <typename _TYPE_>
bool isAllocatedOnDevice(_TYPE_* tab_addr)
{
#ifdef TRUST_USE_GPU
  return DeviceMemory::isAllocatedOnDevice(tab_addr);
#else
  return false;
#endif
}

template <typename _TYPE_, typename _SIZE_>
bool isAllocatedOnDevice(TRUSTArray<_TYPE_,_SIZE_>& tab)
{
#ifdef TRUST_USE_GPU
  bool isAllocatedOnDevice1 = (tab.get_data_location() != DataLocation::HostOnly);
  bool isAllocatedOnDevice2 = isAllocatedOnDevice(tab.data());
  if (isAllocatedOnDevice1!=isAllocatedOnDevice2)
    Process::exit("isAllocatedOnDevice(TRUSTArray<_TYPE_>& tab) error! Seems tab.get_data_location() is not up-to-date !");
  return isAllocatedOnDevice2;
#else
  return false;
#endif
}

// Allocate on device:
template <typename _TYPE_, typename _SIZE_>
_TYPE_* allocateOnDevice(TRUSTArray<_TYPE_,_SIZE_>& tab)
{
  _TYPE_ *tab_addr = tab.data();
#ifdef TRUST_USE_GPU
  if (isAllocatedOnDevice(tab)) deleteOnDevice(tab);
  allocateOnDevice(tab_addr, tab.size_mem());
  tab.set_data_location(DataLocation::Device);
#endif
  return tab_addr;
}

template <typename _TYPE_, typename _SIZE_>
_TYPE_* allocateOnDevice(_TYPE_* ptr, _SIZE_ size)
{
#ifdef TRUST_USE_GPU
  if (isAllocatedOnDevice(ptr))
    {
      Process::Journal() << "Error! allocateOnDevice: host address " << ptrToString(ptr) << " is already registered on device:" << finl;
      DeviceMemory::printMemoryMap();
      Process::exit("Error! allocateOnDevice: host address already allocated on device. See log.");
    }
  statistics().begin_count(STD_COUNTERS::gpu_malloc_free,statistics().get_last_opened_counter_level()+1);
  _TYPE_ *device_ptr = nullptr;
  if (size>0)
    {
      size_t bytes = sizeof(_TYPE_) * size;
      size_t free_bytes = DeviceMemory::deviceMemGetInfo(0);
      size_t total_bytes = DeviceMemory::deviceMemGetInfo(1);
      if (bytes > free_bytes)
        {
          Cerr << "Error ! Trying to allocate " << bytes << " bytes on GPU memory whereas only " << free_bytes
               << " bytes are available." << finl;
          Process::exit();
        }
      device_ptr = static_cast<_TYPE_ *>(Kokkos::kokkos_malloc(bytes));
      if (statistics().is_gpu_verbose_on() && Process::je_suis_maitre())
        {
          std::string clock(Process::is_parallel() ? "[clock]#" + std::to_string(Process::me()) : "[clock]  ");
          double ms = 1000 * statistics().get_time_since_last_open(STD_COUNTERS::gpu_malloc_free);
          printf("%s %7.3f ms [Data]   Allocate on device [%9s] %6ld Bytes (%ld/%ldGB free) Currently allocated: %6ld\n",
                 clock.c_str(), ms, ptrToString(ptr).c_str(), long(bytes), free_bytes / (1024 * 1024 * 1024),
                 total_bytes / (1024 * 1024 * 1024), long(DeviceMemory::allocatedBytesOnDevice()));
        }
    }
  // Map host_ptr with device_ptr:
  DeviceMemory::add(ptr, device_ptr, size * sizeof(_TYPE_));
  statistics().end_count(STD_COUNTERS::gpu_malloc_free);
#ifndef NDEBUG
  const _TYPE_ INVALIDE_ = (std::is_same<_TYPE_,double>::value) ? DMAXFLOAT*0.999 : ( (std::is_same<_TYPE_,int>::value) ? INT_MIN : 0); // Identique a TRUSTArray<_TYPE_>::fill_default_value()
  Kokkos::View<_TYPE_*> ptr_v(device_ptr, size);
  Kokkos::parallel_for(start_gpu_timer(__KERNEL_NAME__), size, KOKKOS_LAMBDA(const int i)
  {
    ptr_v(i) = INVALIDE_;
  });
  end_gpu_timer(__KERNEL_NAME__);
#endif
#endif
  return ptr;
}

// Delete on device:
template <typename _TYPE_, typename _SIZE_>
void deleteOnDevice(TRUSTArray<_TYPE_,_SIZE_>& tab)
{
#ifdef TRUST_USE_GPU
  _TYPE_ *tab_addr = tab.data();
  if (statistics().get_init_device() && tab_addr && isAllocatedOnDevice(tab))
    {
      deleteOnDevice(tab_addr, tab.size_mem());
      tab.set_data_location(DataLocation::HostOnly);
    }
#endif
}

template <typename _TYPE_, typename _SIZE_>
void deleteOnDevice(_TYPE_* ptr, _SIZE_ size)
{
#ifdef TRUST_USE_GPU
  statistics().begin_count(STD_COUNTERS::gpu_malloc_free,statistics().get_last_opened_counter_level()+1);
  std::string clock;
  if (PE_Groups::get_nb_groups()>0 && Process::is_parallel()) clock = "[clock]#"+std::to_string(Process::me());
  else
    clock = "[clock]  ";
  trustIdType bytes = DeviceMemory::deviceBytes(ptr);
  if (statistics().is_gpu_verbose_on() && Process::je_suis_maitre())
    cout << clock << "            [Data]   Delete on device array [" << ptrToString(ptr).c_str() << "] of " << bytes << " Bytes. It remains " << DeviceMemory::getMemoryMap().size()-1 << " arrays." << endl << flush;
  Kokkos::kokkos_free(addrOnDevice(ptr));
  DeviceMemory::del(ptr);
  statistics().end_count(STD_COUNTERS::gpu_malloc_free);
#endif
}

// Update const array on device if necessary
// Before		After		Copy ?
// HostOnly	    HostDevice	Yes
// Host		    HostDevice	Yes
// HostDevice	HostDevice	No
// Device		Device		No
// Note: when the host block is released by free_host_memory() (see TRUSTArray), the resulting
// status is Device and not HostDevice, since the host copy no longer exists.
template <typename _TYPE_, typename _SIZE_>
const _TYPE_* mapToDevice(const TRUSTArray<_TYPE_,_SIZE_>& tab)
{
  // Update data on device if necessary
  DataLocation loc = tab.isDataOnDevice() ? tab.get_data_location() : DataLocation::HostDevice;
  const _TYPE_ *tab_addr = mapToDevice_(const_cast<TRUSTArray<_TYPE_,_SIZE_> &>(tab), loc);
  return tab_addr;
}

template <typename _TYPE_, typename _SIZE_>
_TYPE_* mapToDevice_(TRUSTArray<_TYPE_,_SIZE_>& tab, DataLocation nextLocation)
{
  _TYPE_ *tab_addr = tab.data();
#ifdef TRUST_USE_GPU
  DataLocation currentLocation = tab.get_data_location();
  tab.set_data_location(nextLocation); // Important to specify the new status before retrieving the pointer:
  // Important for ref_array/ref_tab support, we take the size of the memory allocated, not the size of the array (tab.size_array()):
  //int memory_size = tab.size_array();
  int memory_size = tab.size_mem();
  if (currentLocation==DataLocation::HostOnly)
    {
      // Not a Trav which is already allocated on device:
      if (!(tab.get_mem_storage() == STORAGE::TEMP_STORAGE && isAllocatedOnDevice(tab_addr)))
        allocateOnDevice(tab_addr, memory_size);
      copyToDevice(tab_addr, memory_size);
    }
  else if (currentLocation==DataLocation::Host)
    {
      copyToDevice(tab_addr, memory_size);
      if (DeviceMemory::warning(memory_size)) // Warning for large array only:
        ToDo_Kokkos("H2D update of large array! Add a breakpoint to find the reason.");
    }
  tab.free_host_memory();
#endif
  return tab_addr;
}

template <typename _TYPE_, typename _SIZE_>
void copyToDevice(_TYPE_* ptr, _SIZE_ size)
{
#ifdef TRUST_USE_GPU
  if (size>0)
    {
      assert(isAllocatedOnDevice(ptr));
      _SIZE_ bytes = sizeof(_TYPE_) * size;
      start_gpu_timer("copyToDevice",bytes);
      statistics().begin_count(STD_COUNTERS::gpu_copytodevice,statistics().get_last_opened_counter_level()+1);
      Kokkos::View<_TYPE_*> host_view(ptr, size);
      Kokkos::View<_TYPE_*> device_view(addrOnDevice(ptr), size);
      Kokkos::deep_copy(device_view, host_view);
      statistics().end_count(STD_COUNTERS::gpu_copytodevice,1,static_cast<int>(bytes));
      std::stringstream message;
      message << "Copy to device host block [" << ptrToString(ptr) << "] " << size << " items ";
      end_gpu_timer(message.str(), 0, bytes);
    }
#endif
}

// Copy non-const array on device if necessary for computation on device
template <typename _TYPE_, typename _SIZE_>
_TYPE_* computeOnTheDevice(TRUSTArray<_TYPE_,_SIZE_>& tab)
{
  // non-const array will be modified on device:
  _TYPE_ *tab_addr = mapToDevice_(tab, DataLocation::Device);
  return tab_addr;
}

// ToDo OpenMP: rename copy -> update or map ?
// Copy non-const array to host from device
template <typename _TYPE_, typename _SIZE_>
void copyFromDevice(TRUSTArray<_TYPE_,_SIZE_>& tab)
{
#ifdef TRUST_USE_GPU
  if (tab.get_data_location() == DataLocation::Device)
    {
      tab.reallocate_host_memory();
      copyFromDevice(tab.data(), tab.size_mem());
      tab.set_data_location(DataLocation::HostDevice);
    }
#endif
}
template <typename _TYPE_, typename _SIZE_>
void copyFromDevice(_TYPE_* ptr, _SIZE_ size)
{
#ifdef TRUST_USE_GPU
  if (size>0)
    {
      assert(isAllocatedOnDevice(ptr));
      _SIZE_ bytes = sizeof(_TYPE_) * size;
      start_gpu_timer("copyFromDevice",bytes);
      statistics().begin_count(STD_COUNTERS::gpu_copyfromdevice,statistics().get_last_opened_counter_level()+1);
      Kokkos::View<_TYPE_*> host_view(ptr, size);
      Kokkos::View<_TYPE_*> device_view(addrOnDevice(ptr), size);
      Kokkos::deep_copy(host_view, device_view);
      statistics().end_count(STD_COUNTERS::gpu_copyfromdevice,1,static_cast<int>(bytes));
      std::stringstream message;
      message << "Copy from device [" << ptrToString(ptr) << "] " << size << " items ";
      end_gpu_timer(message.str(), 0, bytes);
      //if (statistics().is_gpu_verbose_on()) printf("\n");
      if (DeviceMemory::warning(size)) // Warning for large array only:
        ToDo_Kokkos("D2H update of large array! Add a breakpoint to find the reason if not IO.");
    }
#endif
}

// Copy const array to host from device
template <typename _TYPE_, typename _SIZE_>
void copyFromDevice(const TRUSTArray<_TYPE_,_SIZE_>& tab)
{
  copyFromDevice(const_cast<TRUSTArray<_TYPE_,_SIZE_>&>(tab));
}


//
//  Explicit template instanciations
//
template char* addrOnDevice<char>(char* ptr);
template double* addrOnDevice<double>(TRUSTArray<double>& tab);
template int* addrOnDevice<int>(TRUSTArray<int>& tab);
template float* addrOnDevice<float>(TRUSTArray<float>& tab);

template bool isAllocatedOnDevice<double>(double* tab_addr);
template bool isAllocatedOnDevice<int>(int* tab_addr);
template bool isAllocatedOnDevice<float>(float* tab_addr);
template bool isAllocatedOnDevice<char>(char* tab_addr);
template bool isAllocatedOnDevice<double>(TRUSTArray<double>& tab);
template bool isAllocatedOnDevice<int>(TRUSTArray<int>& tab);
template bool isAllocatedOnDevice<float>(TRUSTArray<float>& tab);

template double* allocateOnDevice<double>(TRUSTArray<double>& tab);
template int* allocateOnDevice<int>(TRUSTArray<int>& tab);
template float* allocateOnDevice<float>(TRUSTArray<float>& tab);
template char* allocateOnDevice<char>(char* ptr, int size);

template const double* allocateOnDevice<double>(const TRUSTArray<double>& tab);
template const int* allocateOnDevice<int>(const TRUSTArray<int>& tab);
template const float* allocateOnDevice<float>(const TRUSTArray<float>& tab);

template void deleteOnDevice<double>(TRUSTArray<double>& tab);
template void deleteOnDevice<int>(TRUSTArray<int>& tab);
template void deleteOnDevice<float>(TRUSTArray<float>& tab);
template void deleteOnDevice<char>(char* ptr, int size);
template void deleteOnDevice<int>(int* ptr, int size);
template void deleteOnDevice<float>(float* ptr, int size);
template void deleteOnDevice<double>(double* ptr, int size);

template const double* mapToDevice<double>(const TRUSTArray<double>& tab);
template const int* mapToDevice<int>(const TRUSTArray<int>& tab);
template const float* mapToDevice<float>(const TRUSTArray<float>& tab);
template void copyToDevice<char>(char* ptr, int size);

template double* mapToDevice_<double>(TRUSTArray<double>& tab, DataLocation nextLocation);
template int* mapToDevice_<int>(TRUSTArray<int>& tab, DataLocation nextLocation);
template float* mapToDevice_<float>(TRUSTArray<float>& tab, DataLocation nextLocation);

template double* computeOnTheDevice<double>(TRUSTArray<double>& tab);
template int* computeOnTheDevice<int>(TRUSTArray<int>& tab);
template float* computeOnTheDevice<float>(TRUSTArray<float>& tab);

template void copyFromDevice<double>(TRUSTArray<double>& tab);
template void copyFromDevice<int>(TRUSTArray<int>& tab);
template void copyFromDevice<float>(TRUSTArray<float>& tab);
template void copyFromDevice<char>(char* ptr, int size);

template void copyFromDevice<double>(const TRUSTArray<double>& tab);
template void copyFromDevice<int>(const TRUSTArray<int>& tab);
template void copyFromDevice<float>(const TRUSTArray<float>& tab);

#if INT_is_64_ == 2

// The ones needed for 64 bits:
template double* addrOnDevice<double>(TRUSTArray<double,trustIdType>& tab);
template int* addrOnDevice<int>(TRUSTArray<int,trustIdType>& tab);
template trustIdType* addrOnDevice<trustIdType>(TRUSTArray<trustIdType,trustIdType>& tab);
template trustIdType* addrOnDevice<trustIdType>(TRUSTArray<trustIdType,int>& tab);
template float* addrOnDevice<float>(TRUSTArray<float,trustIdType>& tab);

template bool isAllocatedOnDevice<trustIdType>(trustIdType* tab_addr);
template bool isAllocatedOnDevice<double>(TRUSTArray<double,trustIdType>& tab);
template bool isAllocatedOnDevice<float>(TRUSTArray<float,trustIdType>& tab);
template bool isAllocatedOnDevice<int>(TRUSTArray<int,trustIdType>& tab);
template bool isAllocatedOnDevice<trustIdType>(TRUSTArray<trustIdType,trustIdType>& tab);
template bool isAllocatedOnDevice<trustIdType>(TRUSTArray<trustIdType,int>& tab);

template double* allocateOnDevice<double>(TRUSTArray<double,trustIdType>& tab);
template int* allocateOnDevice<int>(TRUSTArray<int,trustIdType>& tab);
template trustIdType* allocateOnDevice<trustIdType>(TRUSTArray<trustIdType,trustIdType>& tab);
template trustIdType* allocateOnDevice<trustIdType>(TRUSTArray<trustIdType,int>& tab);
template float* allocateOnDevice<float>(TRUSTArray<float,trustIdType>& tab);

template const double* allocateOnDevice<double>(const TRUSTArray<double,trustIdType>& tab);
template const int* allocateOnDevice<int>(const TRUSTArray<int,trustIdType>& tab);
template const trustIdType* allocateOnDevice<trustIdType>(const TRUSTArray<trustIdType,trustIdType>& tab);
template const trustIdType* allocateOnDevice<trustIdType>(const TRUSTArray<trustIdType,int>& tab);
template const float* allocateOnDevice<float>(const TRUSTArray<float,trustIdType>& tab);

template void deleteOnDevice<double>(TRUSTArray<double,trustIdType>& tab);
template void deleteOnDevice<int>(TRUSTArray<int,trustIdType>& tab);
template void deleteOnDevice<trustIdType>(TRUSTArray<trustIdType,trustIdType>& tab);
template void deleteOnDevice<trustIdType>(TRUSTArray<trustIdType,int>& tab);
template void deleteOnDevice<float>(TRUSTArray<float,trustIdType>& tab);

template void deleteOnDevice<int>(int* ptr, long size);
template void deleteOnDevice<int>(int* ptr, long long size);
template void deleteOnDevice<trustIdType>(trustIdType* ptr, long size);
template void deleteOnDevice<trustIdType>(trustIdType* ptr, long long size);
template void deleteOnDevice<trustIdType>(trustIdType* ptr, int size);
template void deleteOnDevice<float>(float* ptr, long size);
template void deleteOnDevice<double>(double* ptr, long size);
template void deleteOnDevice<float>(float* ptr, long long size);
template void deleteOnDevice<double>(double* ptr, long long size);

template const double* mapToDevice<double>(const TRUSTArray<double,trustIdType>& tab);
template const int* mapToDevice<int>(const TRUSTArray<int,trustIdType>& tab);
template const trustIdType* mapToDevice<trustIdType>(const TRUSTArray<trustIdType,trustIdType>& tab);
template const trustIdType* mapToDevice<trustIdType>(const TRUSTArray<trustIdType,int>& tab);
template const float* mapToDevice<float>(const TRUSTArray<float,trustIdType>& tab);

template int* computeOnTheDevice(TRUSTArray<int,trustIdType>& tab);
template trustIdType* computeOnTheDevice(TRUSTArray<trustIdType,trustIdType>& tab);
template trustIdType* computeOnTheDevice(TRUSTArray<trustIdType,int>& tab);
template float* computeOnTheDevice(TRUSTArray<float,trustIdType>& tab);
template double* computeOnTheDevice(TRUSTArray<double,trustIdType>& tab);

template void copyFromDevice<int, trustIdType>(TRUSTArray<int,trustIdType>& tab);
template void copyFromDevice<trustIdType, int>(TRUSTArray<trustIdType,int>& tab);
template void copyFromDevice<trustIdType, trustIdType>(TRUSTArray<trustIdType,trustIdType>& tab);
template void copyFromDevice<float, trustIdType>(TRUSTArray<float,trustIdType>& tab);
template void copyFromDevice<double, trustIdType>(TRUSTArray<double,trustIdType>& tab);

// With const:
template void copyFromDevice<int, trustIdType>(const TRUSTArray<int,trustIdType>& tab);
template void copyFromDevice<trustIdType, int>(const TRUSTArray<trustIdType,int>& tab);
template void copyFromDevice<trustIdType, trustIdType>(const TRUSTArray<trustIdType,trustIdType>& tab);
template void copyFromDevice<float, trustIdType>(const TRUSTArray<float,trustIdType>& tab);
template void copyFromDevice<double, trustIdType>(const TRUSTArray<double,trustIdType>& tab);


// GPU timers (with ability to disable with if (timer) in some CPU-critical kernels):
std::string start_gpu_timer(std::string str, int bytes)
{
#ifdef TRUST_USE_GPU
  if (!statistics().get_init_device())
    return str;
  //std::cerr << "Provisoire start_gpu_timer " << str << std::endl;
  if (statistics().get_gpu_timer())
    {
      Cerr << "A GPU KERNEL is still running, you can't open a new one (" << str << ") yet." << finl;
      Cerr <<"Probably you forgot to define a end_gpu_timer(...) call." << finl;
      Cerr <<"Or more subtil bug: you copy a C++ object on the device which has at least one TRUST array as attribute." << finl;
      Process::exit();
    }
  statistics().start_gpu_timer();
  statistics().add_to_gpu_timer_counter(1);
  if (bytes == -1)
    statistics().begin_count(STD_COUNTERS::gpu_kernel,statistics().get_last_opened_counter_level()+1);
#ifdef TRUST_USE_CUDA
  if (!str.empty()) nvtxRangePush(str.c_str());
#endif
#ifdef TRUST_USE_ROCM
  if (!str.empty()) roctxRangePush(str.c_str());
#endif
#endif
  return str;
}

void end_gpu_timer(const std::string& str, int onDevice, int bytes) // Return in [ms]
{
#ifdef TRUST_USE_GPU
  if (!statistics().get_init_device())
    return;
  //std::cerr << "Provisoire end_gpu_timer " << str << std::endl;
  statistics().add_to_gpu_timer_counter(-1);
  if (onDevice)
    {
#ifdef TRUST_USE_UVM
      cudaDeviceSynchronize();
#endif
#ifdef KOKKOS
      if (statistics().get_gpu_fence()) Kokkos::fence();  // Barrier for real time
#endif
    }
  if (bytes == -1)
    statistics().end_count(STD_COUNTERS::gpu_kernel,onDevice);
  if (statistics().is_gpu_verbose_on() && Process::je_suis_maitre()) // Display
    {
      std::string clock(Process::is_parallel() ? "[clock]#" + std::to_string(Process::me()) : "[clock]  ");
      double ms = 1000 * statistics().stop_gpu_timer_and_compute_gpu_time();
      if (bytes == -1)
        {
          if (!str.empty())
            printf("%s %7.3f ms [%s %15s\n", clock.c_str(), ms, onDevice ? "Device]" : "Host]  ", str.c_str());
        }
      else
        {
          double mo = (double) bytes / 1024 / 1024;
          if (ms == 0 || bytes == 0)
            printf("%s            [Data]   %15s\n", clock.c_str(), str.c_str());
          else
            printf("%s %7.3f ms [Data]   %15s %6ld Bytes %5.1f Go/s\n", clock.c_str(), ms, str.c_str(),
                   long(bytes), mo / ms);
          //printf("%s %7.3f ms [Data]   %15s %6ld Mo %5.1f Go/s\n", clock.c_str(), ms, str.c_str(), long(mo), mo/ms);
        }
      fflush(stdout);
    }
  else
    statistics().stop_gpu_timer();

#ifdef TRUST_USE_CUDA
  if (!str.empty()) nvtxRangePop();
#endif
#ifdef TRUST_USE_ROCM
  if (!str.empty()) roctxRangePop();
#endif
#endif
}

#endif

