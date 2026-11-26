/****************************************************************************
* Copyright (c) 2022, CEA
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

#ifndef LataVector_H
#define LataVector_H
#include <assert.h>
#include <arch.h>

// This vector class uses an array of pointers so that objects stored are never
// moved in memory when the array is resized.
template <class C>
class LataVector
{
public:
  LataVector() : n_(0), data_(0) { }
  LataVector(const LataVector<C>& x) : n_(0), data_(0) { operator=(x); }
  LataVector(int n) : n_(0), data_(0) { for (int i=0; i<n; i++) add(); }
  ~LataVector() { reset(); }
  void reset() { for (int i=0; i<n_; i++) { delete data_[i]; }; delete[] data_; n_ = 0; data_ = 0; }
  const C& operator[](int i) const { assert(i>=0 && i<n_); return *(data_[i]); }
  C& operator[](int i) { assert(i>=0 && i<n_); return *(data_[i]); }
  C& add(const C& item) { return add_item(new C(item)); }
  C& add() { return add_item(new C); }
  int size() const { return n_; }
  int rang(const C& c) const
  {
    for (int i = 0; i < n_; i++)
      if (*(data_[i]) == c) return i;
    return -1;
  }
  LataVector<C>& operator=(const LataVector<C>& x) { reset(); for (int i=0; i<x.n_; i++) add(x[i]); return *this; }
private:
  C& add_item(C* added_item)
  {
    C** old = data_;
    data_ = new C*[n_+1];
    for (int i=0; i<n_; i++) data_[i] = old[i];
    delete[] old;
    data_[n_++] = added_item;
    return *added_item;
  }
  int n_;
  C** data_;
};
#endif
