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

#ifndef DeviceMemory_included
#define DeviceMemory_included

#include <Array_base.h>
#include <ctime>
#include <string>
#include <sstream>
#include <map>
#include <unordered_map>
#include <tuple>

// Map: host ptr -> (host block bytes, device block bytes, device ptr)
// Host and device bytes are equal at allocation but differ once free_host_memory() has
// shrunk the host block to a stub: the device block keeps its full size.
using map_t = std::unordered_map<void*, std::tuple<trustIdType, trustIdType, void*>>;

struct DeviceMemory
{
public:

  DeviceMemory() = delete;

  static size_t allocatedBytesOnDevice();
  static size_t deviceMemGetInfo(bool);
  static map_t& getMemoryMap() { return memory_map_; };
  static bool isAllocatedOnDevice(void *);
  static void* addrOnDevice(void*);
  static void add(void * ptr, void * device_ptr, trustIdType bytes);
  static void del(void * ptr);
  static void rekey(void * old_ptr, void * new_ptr, trustIdType host_bytes);
  static bool hostBlockReleased(void * ptr);
  static trustIdType hostBytes(void * ptr);
  static trustIdType deviceBytes(void * ptr);
  static void printMemoryMap();
  static trustIdType internal_items_size_;
  // can't reference Schema_temps_base cause Kernel/Utilitaires...
  static void set_nb_pas_dt(int nb_pas_dt) { nb_pas_dt_ = nb_pas_dt; }
  static int nb_pas_dt() { return nb_pas_dt_; }
  static bool warning(trustIdType nb_items);

private:
  static map_t memory_map_; // Define a map to track memory allocations on device
  static size_t initial_free_;
  static int nb_pas_dt_;
};

#endif






