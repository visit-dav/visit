/*****************************************************************************
*
* Copyright (c) 2011, CEA
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of CEA, nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/

#ifndef Lata_tools_include_
#define Lata_tools_include_
#include <assert.h>
#include <arch.h>





#ifdef WIN32
#define __BIG_ENDIAN    111
#define __LITTLE_ENDIAN 121
#define __BYTE_ORDER __LITTLE_ENDIAN

#define strtoll _strtoi64
// This must be able to contain a total memory size
// or a very big operation counter.
typedef __int64 BigEntier;

#else
// This must be able to contain a total memory size
// or a very big operation counter.
typedef long long BigEntier;
#endif

#ifndef __BYTE_ORDER
#include <endian.h>
#endif

#include <LataVector.h>


#define PATH_SEPARATOR '/'

#ifndef __BYTE_ORDER
#error "Byte order not defined."
#endif
#if (__BYTE_ORDER == __BIG_ENDIAN)
const bool mymachine_msb =  true;
#elif (__BYTE_ORDER == __LITTLE_ENDIAN)
const bool mymachine_msb =  false;
#else
#error "Byte order is neither __BIG_ENDIAN nor __LITTLE_ENDIAN : "
#endif

class ArrOfInt;
class ArrOfDouble;
class ArrOfFloat;
class ArrOfBit;
BigEntier memory_size(const ArrOfInt &);
BigEntier memory_size(const ArrOfDouble &);
BigEntier memory_size(const ArrOfFloat &);
BigEntier memory_size(const ArrOfBit &);

class LataObject
{
public:
  virtual ~LataObject() {};
  virtual BigEntier compute_memory_size() const;
};

// A 'LataDeriv<X> ptr' object can hold an object of class Y which is any derived type of X.
// The contained object can be accessed via "valeur()" (you get an object of type X)
//   or "refcast()" (get an object of any derived type Z between X and Y)
//   (refcast() throws an exception if you try to cast with a wrong type)
// It can also be null (hold no object). valeur() will then throw an exception.
// Example:
//  LataDeriv<X> deriv_x;
//  Y & y = deriv_x.instancie(Y); // Creates an instance of type Y within deriv_x
//  X & x = deriv_x.valeur(); // Get a reference to the contained object
//  Y & y2 = deriv_x.refcast(Y); // Same, but you get a reference of type Y
//  Z & z = deriv_x.refcast(Z); // Throw an exception if Z is not a derived class of X and a base class of Y
//  x.reset(); // Destroys the contained object (also destroyed when deriv_x is destroyed)
#define instancie(x) instancie_(new x)
#define refcast(x) refcast_((x*) 0)

template <class C>
class LataDeriv : public LataObject
{
public:
  enum DERIV_ERROR { ERROR_TYPE, ERROR_NULL };
  LataDeriv() : ptr_(0) { };
  ~LataDeriv() { delete ptr_; ptr_ = 0; }
  void reset() { delete ptr_; ptr_ = 0; }
  entier non_nul() const { return ptr_ != 0; }
  // operator C &() { return valeur(); }
  // operator const C &() const { return valeur(); }
  C & valeur() { if (!ptr_) throw ERROR_NULL; return *ptr_; }
  const C & valeur() const { if (!ptr_) throw ERROR_NULL; return *ptr_; }
  template<class DER_C> DER_C & instancie_(DER_C *ptr) {
    reset();
    ptr_ = ptr;
    if (!dynamic_cast<C*>(ptr_)) {
      delete ptr_;
      throw ERROR_TYPE; // DER_C is not a derived type of C
    }
    return (DER_C &) (*ptr_); 
  }
  template<class DER_C> DER_C & refcast_(DER_C *cast_type) {
    if (!ptr_)
      throw ERROR_NULL;
    DER_C * x = dynamic_cast<DER_C *>(ptr_);
    if (!x)
      throw ERROR_TYPE;
    return *x;
  }
  BigEntier compute_memory_size() const { if (ptr_) return ptr_->compute_memory_size(); else return 0; }
protected:
  LataDeriv(const LataDeriv<C> & c) { ptr_ = 0; operator=(c); }
  LataDeriv(const C & c) { ptr_ = 0; operator=(c); }
  LataDeriv<C> & operator=(const LataDeriv<C> &);
  LataDeriv<C> & operator=(const C &);
  C *ptr_;
};

// This is a reference to an object of type C, but thr reference can be null
template<class C>
class LataRef
{
public:
  enum REF_ERROR { ERROR_NULL };
  LataRef() : ptr_(0) { }
  ~LataRef() { ptr_ = 0; }
  LataRef(const LataRef<C> & x) : ptr_(x.ptr_) { }
  LataRef(C & x) : ptr_(&x) { }
  LataRef<C> & operator=(LataRef<C> & x) { ptr_ = x.ptr_; return *this; }
  LataRef<C> & operator=(C & x) { ptr_ = &x; return *this; }
  void reset() { ptr_ = 0; }
  operator C&() { if (!ptr_) throw ERROR_NULL; return *ptr_; }  
  C& valeur() { if (!ptr_) throw ERROR_NULL; return *ptr_; }  
  entier non_nul() const { return ptr_ != 0; }
protected:
  C *ptr_;
};

void array_sort_indirect(const ArrOfInt & array_to_sort, ArrOfInt & index);

class Nom;
void split_path_filename(const char *full_name, Nom & path, Nom & filename);

// To optimize small loops: replace for(i=0;i<n;i++) with n<=3 by
//  for (i=0; i<loop_max(n,3); i++) {
//    loop_instructions();...
//    break_loop(i,n);
//  }
#define loop_max(nloops,max) max
#define break_loop(index,nloops) if (index >= nloops-1) break

#include <LataJournal.h>
#include <Motcle.h>
#include <Noms.h>
#include <DoubleTab.h>
#include <IntTab.h>
#include <FloatTab.h>

Motcles noms_to_motcles(const Noms & noms);

#endif
