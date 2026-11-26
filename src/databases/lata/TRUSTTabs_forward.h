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

#ifndef TRUSTTabs_forward_included
#define TRUSTTabs_forward_included

#include <arch.h>

template <typename _SIZE_> class ArrOfBit_32_64;
using ArrOfBit = ArrOfBit_32_64<int>;

template <typename _TYPE_, typename _SIZE_=int> class TRUSTArray;
using ArrOfDouble = TRUSTArray<double, int>;
using ArrOfFloat = TRUSTArray<float, int>;
using ArrOfInt = TRUSTArray<int, int>;
using ArrOfTID = TRUSTArray<trustIdType, int>;

using BigArrOfDouble = TRUSTArray<double, trustIdType>;
using BigArrOfFloat = TRUSTArray<float, trustIdType>;
using BigArrOfInt = TRUSTArray<int, trustIdType>;
using BigArrOfTID = TRUSTArray<trustIdType, trustIdType>;

template<typename _TYPE_, typename _SIZE_=int> class TRUSTTab;
using DoubleTab = TRUSTTab<double, int>;
using FloatTab = TRUSTTab<float, int>;
using IntTab = TRUSTTab<int, int>;

using TIDTab = TRUSTTab<trustIdType, int>;

using BigDoubleTab = TRUSTTab<double, trustIdType>;
using BigFloatTab = TRUSTTab<float, trustIdType>;
using BigIntTab = TRUSTTab<int, trustIdType>;
using BigTIDTab = TRUSTTab<trustIdType, trustIdType>;

template<typename _TYPE_, typename _SIZE_=int> using TRUSTVect = TRUSTTab<_TYPE_, _SIZE_>;
using DoubleVect = TRUSTVect<double, int>;
using FloatVect = TRUSTVect<float, int>;
using IntVect = TRUSTVect<int, int>;
using TIDVect = TRUSTVect<trustIdType, int>;

using BigDoubleVect = TRUSTVect<double, trustIdType>;
using BigIntVect = TRUSTVect<int, trustIdType>;
using BigTIDVect = TRUSTVect<trustIdType, trustIdType>;

template<typename _TYPE_> class TRUSTTrav;
using IntTrav = TRUSTTrav<int>;
using DoubleTrav = TRUSTTrav<double>;
using FloatTrav = TRUSTTrav<float>;

// previous VECT macros :-)
// TODO : FIXME : Add float typedefs ...
template<typename _TYPE_> class TRUST_Vector;
using ArrsOfDouble = TRUST_Vector<TRUSTArray<double, int>>; // remplace VECT(ArrOfDouble)
using ArrsOfInt = TRUST_Vector<TRUSTArray<int, int>>; // remplace VECT(ArrOfInt)
using DoubleVects = TRUST_Vector<TRUSTVect<double, int>>; // remplace VECT(DoubleVect)
using IntVects = TRUST_Vector<TRUSTVect<int, int>>; // remplace VECT(IntVect)
using DoubleTabs = TRUST_Vector<TRUSTTab<double, int>>; // remplace VECT(DoubleTab)
using IntTabs = TRUST_Vector<TRUSTTab<int, int>>; // remplace VECT(IntTab)


//
// Handy 'using' clauses used in all the class templatized on the 32/64b aspect (Domaine, Octree, Elem_geom, etc.)
//
template <typename _SIZE_> using ArrOfInt_T = TRUSTArray<_SIZE_, _SIZE_>;      // either an ArrOfInt (32 bits) or a BigArrOfTID (64b)
template <typename _SIZE_> using IntVect_T = TRUSTVect<_SIZE_, _SIZE_>;       // either an IntVect (32 bits) or a BigTIDVect (64b)
template <typename _SIZE_> using IntTab_T = TRUSTTab<_SIZE_, _SIZE_>;         // either an IntTab (32 bits) or a BigTIDTab (64b)
template <typename _TYPE_> using SmallArrOfTID_T = TRUSTArray<_TYPE_, int>;    // a small array of identifiers (small even in 64b)
template <typename _TYPE_> using BigArrOfInt_T = TRUSTArray<int, _TYPE_>;    // a big array, but always storing small values (rarely used ...)
template <typename _TYPE_> using BigIntVect_T = TRUSTVect<int, _TYPE_>;      // a big vect, but always storing small values (rarely used ...)

template <typename _SIZE_> using ArrOfDouble_T= TRUSTArray<double, _SIZE_>;
template <typename _SIZE_> using DoubleVect_T = TRUSTVect<double, _SIZE_>;
template <typename _SIZE_> using DoubleTab_T = TRUSTTab<double, _SIZE_>;         // either an DoubleTab (32 bits) or a BigDoubleTab (64b)

template <typename _SIZE_> using FloatVect_T = TRUSTVect<float, _SIZE_>;

template <typename _SIZE_> using ArrsOfInt_T = TRUST_Vector<TRUSTArray<_SIZE_, _SIZE_>>;
template <typename _SIZE_> using DoubleTabs_T = TRUST_Vector<TRUSTTab<double, _SIZE_>>;

#endif /* TRUSTTabs_forward_included */
