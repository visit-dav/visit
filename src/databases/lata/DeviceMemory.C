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

#include <DeviceMemory.h>
#include <iostream>
#include <Process.h>
#include <Nom.h>
#include <Device.h>
#ifndef LATATOOLS
#ifdef TRUST_USE_ROCM
#include <hip/hip_runtime.h>
#endif
#endif

map_t DeviceMemory::memory_map_;
size_t DeviceMemory::initial_free_ = 0;
// Typical size of internal_items (elem,face,som) used as threshold to detect excessive H2D/D2H copies or host array allocation
trustIdType DeviceMemory::internal_items_size_ = 1e9;
int DeviceMemory::nb_pas_dt_ = -1;

// Memory:
size_t DeviceMemory::deviceMemGetInfo(bool print_total) // free or total bytes of the device
{
  size_t free=0, total=0;
#ifndef LATATOOLS
#ifdef TRUST_USE_CUDA
  cudaMemGetInfo(&free, &total);
#endif
#ifdef TRUST_USE_ROCM
  auto err = hipMemGetInfo(&free, &total);
  if (err) Process::exit("Error during hipMemGetInfo");
#endif
#endif
  return print_total ? total : free;
}
size_t DeviceMemory::allocatedBytesOnDevice()
{
  size_t free = deviceMemGetInfo(0);
  initial_free_ = (initial_free_==0) ? free : initial_free_;
  return initial_free_ - free;
}

/** Check if a chunk (even empty) is allocated on the device for the host ptr
 * Return true/false
*/
bool DeviceMemory::isAllocatedOnDevice(void * ptr)
{
  if (memory_map_.empty()) return false;
  auto it = memory_map_.find(ptr);
  return (it != memory_map_.end());
}

/** Return the device address of the host ptr
 * Return nullptr if ptr null or map empty
 * Fatal error if address not found on the device
*/
#ifndef LATATOOLS
void* DeviceMemory::addrOnDevice(void * ptr)
{
  if (ptr==nullptr || memory_map_.empty())
    return nullptr;
  // Exact match (the common case) is a O(1) lookup:
  auto found = memory_map_.find(ptr);
  if (found != memory_map_.end())
    return std::get<2>(found->second);
  // Case of buffer_base (ptr is not necessarily the start address of the memory block...) or DoubleTab_parts
  trustIdType host_bytes, device_bytes;
  void* device_ptr;
  void* host_ptr;
  for (auto it = memory_map_.begin(); it != memory_map_.end(); ++it)
    {
      host_ptr = it->first;
      std::tie(host_bytes, device_bytes, device_ptr) = it->second;
      if (ptr > host_ptr && ptr < static_cast<char *>(host_ptr) + host_bytes)
        {
          if (host_bytes != device_bytes)
            {
              Cerr << "Error! Interior host address " << ptrToString(ptr) << " resolved into a block whose host memory was released ("
                   << host_bytes << " host bytes vs " << device_bytes << " device bytes):" << finl;
              DeviceMemory::printMemoryMap();
              Process::exit("Error! Interior host address in a released block. See log.");
            }
          return static_cast<char *>(device_ptr) + (static_cast<char *>(ptr) - static_cast<char *>(host_ptr));
        }
    }
  // Reproduce OpenMP-target spec: FATAL ERROR: data in use_device clause was not found on device 1: host:0x3dc75600
  Cerr << "Error! Device address for host address " << ptrToString(ptr) << " not found:" << finl;
  DeviceMemory::printMemoryMap();
  Process::exit("Error! Device address for host address not found. See log.");
  return nullptr;
}

/** Add a new line to the memory_map_
 * ptr should be non-null, device_ptr and size may be null
 */
void DeviceMemory::add(void * ptr, void * device_ptr, trustIdType bytes)
{
  if (ptr==nullptr) return;
  DeviceMemory::getMemoryMap()[ptr] = {bytes, bytes, device_ptr};
}

/** Delete a line into the memory_map_
 *
 */
void DeviceMemory::del(void * ptr)
{
  DeviceMemory::getMemoryMap().erase(ptr);
}

/** Move an existing line of the memory_map_ from old_ptr to new_ptr. The device block is
 * untouched, so its pointer and size are preserved; only the host block size is updated to
 * describe the block now sitting at new_ptr (addrOnDevice() resolves interior host pointers with it).
 */
void DeviceMemory::rekey(void * old_ptr, void * new_ptr, trustIdType host_bytes)
{
  if (old_ptr == new_ptr) return;
  auto it = memory_map_.find(old_ptr);
  if (it == memory_map_.end())
    {
      Cerr << "Error! DeviceMemory::rekey: host address " << ptrToString(old_ptr) << " not found:" << finl;
      DeviceMemory::printMemoryMap();
      Process::exit("Error! DeviceMemory::rekey: host address not found. See log.");
    }
  trustIdType device_bytes = std::get<1>(it->second);
  void* device_ptr = std::get<2>(it->second);
  memory_map_.erase(it);
  memory_map_[new_ptr] = {host_bytes, device_bytes, device_ptr};
}

/** Print the memory map for debug
 *
 */
void DeviceMemory::printMemoryMap()
{
  trustIdType host_bytes, device_bytes;
  void* device_ptr;
  trustIdType nb_released = 0, nb_on_host = 0;
  trustIdType bytes_device = 0, bytes_released = 0, bytes_on_host = 0;
  Cerr << "=== Memory blocks on the device ===" << finl;
  for (const auto& block : memory_map_)
    {
      void* ptr = block.first;
      std::tie(host_bytes, device_bytes, device_ptr) = block.second;
      Cerr << "Host ptr: " << ptrToString(ptr) << " (" << host_bytes << " bytes) Device ptr: "
           << ptrToString(device_ptr) << " (" << device_bytes << " bytes)"
           << (host_bytes == device_bytes ? "" : " [host block released]") << finl;
      bytes_device += device_bytes;
      if (host_bytes == 0 && device_bytes > 0)
        {
          nb_released++;
          bytes_released += device_bytes;
        }
      else
        {
          nb_on_host++;
          bytes_on_host += host_bytes;
        }
    }
  const double MB = 1024. * 1024.;
  Cerr << "Blocks: " << nb_released + nb_on_host << " (" << nb_released << " released on host, " << nb_on_host << " still allocated on host)" << finl;
  Cerr << "Device memory:                  " << static_cast<double>(bytes_device) / MB << " MB" << finl;
  Cerr << "Host memory saved (released):   " << static_cast<double>(bytes_released) / MB << " MB" << finl;
  Cerr << "Host memory still allocated:    " << static_cast<double>(bytes_on_host) / MB << " MB" << finl;
  Cerr << "===================================" << finl;
}

/** A released host block is recorded with 0 host bytes, the device block keeping its size.
 */
bool DeviceMemory::hostBlockReleased(void * ptr)
{
  auto it = memory_map_.find(ptr);
  if (it == memory_map_.end()) return false;
  return std::get<0>(it->second) == 0 && std::get<1>(it->second) > 0;
}

trustIdType DeviceMemory::hostBytes(void * ptr)
{
  auto it = memory_map_.find(ptr);
  return it == memory_map_.end() ? 0 : std::get<0>(it->second);
}

trustIdType DeviceMemory::deviceBytes(void * ptr)
{
  auto it = memory_map_.find(ptr);
  return it == memory_map_.end() ? 0 : std::get<1>(it->second);
}

bool DeviceMemory::warning(trustIdType nb_items)
{
  return statistics().is_gpu_verbose_on() && nb_pas_dt_>=0 && nb_items>=internal_items_size_;
}
#endif
