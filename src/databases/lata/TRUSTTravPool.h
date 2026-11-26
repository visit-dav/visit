/****************************************************************************
* Copyright (c) 2024, CEA
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

#ifndef TRUSTTravPool_included
#define TRUSTTravPool_included

#include <memory>
#include <vector>

#include <TVAlloc.h>

/*! @brief Pool of memory blocks used when requesting temporary storage (Trav arrays)
 *
 * Purely static methods. One pool per base type (int, double, etc...).
 *
 * The implementation details are in the .cpp file.
 */
template<typename _TYPE_>
class TRUSTTravPool
{
public:
  using block_ptr_t = std::shared_ptr<std::vector<_TYPE_, TVAlloc<_TYPE_> > >;

  TRUSTTravPool() = delete;

#ifndef LATATOOLS
  static block_ptr_t GetFreeBlock(int sz);
  static block_ptr_t ResizeBlock(block_ptr_t p, int new_sz);
  static void ReleaseBlock(block_ptr_t);
  static void ClearPool();
#else // Cheating when compiling lata_tools: Trav are never needed:
  static block_ptr_t GetFreeBlock(int sz) { return nullptr; }
  static block_ptr_t ResizeBlock(block_ptr_t p, int new_sz) { return nullptr; }
  static void ReleaseBlock(block_ptr_t) { }

  static void ClearPool() { }
#endif
  static void DeleteOnDevice();
  static void PrintStats();
};


#endif  // TRUSTTravPool_included
