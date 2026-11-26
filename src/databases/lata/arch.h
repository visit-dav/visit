//TRUST_NO_INDENT
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

/*****************************************************************************/
/* WARNING: this file is generated automatically from arch.h.in !!           */
/* DO NOT EDIT MANUALLY.                                                     */
/*****************************************************************************/


/*! @brief Management of machine dependencies for calling FORTRAN routines. The macro F77DECLARE(TOTO) allows declaring a FORTRAN routine.
 *
 *   The macro F77NAME(TOTO) allows calling a FORTRAN routine.
 *   Example:
 *   toto.f:
 *          SUBROUTINE TOTO(DOUBLE X)
 *          X=2.0*X
 *          RETURN
 *          END
 *
 *   appel_toto.cpp
 *   void F77DECLARE(TOTO) (const double* );
 *   void main()
 *   {
 *      double x;
 *      F77NAME(TOTO)(x);
 *   }
 *
 */

#ifndef _ARCH_H_
#define _ARCH_H_

#include <generique.h>

// The below used to differ according to compilers, but now should be standard:
#define IOS_OPEN_MODE std::ios_base::openmode
#define IOS_FORMAT std::ios_base::fmtflags

#ifdef __CYGWIN__
void srand48(int) ;
double drand48();
char* strdup(const char* s);
#endif

#ifdef MICROSOFT
void srand48(int) ;
double drand48();
double atanh(const double&);
#include <iso646.h>
#define NO_RANDOM
#define F77DECLARE(x) F77NAME(x)
#else
#define F77DECLARE(x) F77NAME(x)
#endif

#ifndef F77CALLWITHOUT_
#define F77NAME(x) name2(x,_)
#else
#define F77NAME(x) x
#endif

// Used for FORTRAN interfaces:
typedef int integer;

extern char* pwd();

/* **************************************
            32b / 64b management
  **************************************
  Below we define 'trustIdType', type used to store all geometrical entity indices (element index, node index, ...) that might
  exceed the 32b limit.

  All the TRUST libraries (LIBLATAFILTER notably) and all the main C++ source code can be compiled in either 32b or 64b depending
  on the flag set at configure time.

  However the lata_tools and VisIt plugin are **only** 64b for simplicity, and because it will be provided to the VisIt team for integration.

  In the core TRUST C++ code, the strategy is as follows: all the C++ classes that are potentially used before the Scatter keyword
  should support both 32 or 64b mode. This is typically done by having them templatized on the SIZE parameter. In practice however
  some classes are not yet ported to 64b and will remain only functional in 32b.
  The idea is that once the computation is distributed on several processors, each proc will never need to use 64b indices. And everything
  happening after Scatter (like the operator computations, etc.) is done on one proc only.
  (note that there are some exceptions for the postprocessing where we still need a total number of elements for example).

  The Zone files (containing splitted domains after Partition) are always written in 32b (see DomaineCutter and look for set_64b(false))
  Similarly, the save/restart files are always 32b (see SaveRestart.cpp).
  
  The input/output management is sometimes tricky, since even with an executable compiled in 64b, we might need to write (/read) 32b data.
  See what is done class AbstractIO about this.

  Various types/sizes of arrays are declared in TRUSTTab_forward.h. For example you might want a small of array of (big) identifiers (=SmallArrOfTID)
  or a big array of regular 32b int (=BigArrOfInt). Notice that for big arrays, only the first dimension (dimension 0) might be big. Other axes
  always remain within 32b.
  
  For the IJK parts of the code, the rule is that : the three directions nx, ny and nz are always within 32b, but their *product* might exceed 32b.

  At the time of writing, there are two ways to produce a 64b version of the code:
    - either with configure flag '-with-64-bit-indices' which brutally replace all 'int' in the source code with 'long' (historical).
    When done so, C++ define tag 'INT_is_64_' is set to 1.
    - or with configure flag '-64-bit-new' (the final target), defining trustIdType to int64. In this case C++ define tag
    'INT_is_64_' is set to 2.
*/

#include <cstdint>
// We will often rely on std::numeric_limits<int>::max(), so need this:
#include <limits>

#if 1          // are we in 64b mode?
using trustIdType = std::int64_t;
#  ifndef INT_is_64_           // it might have been already defined by the CMake (e.g. in lata_tools compilation)
#    if  1     // are we in 64b mode, new way of doing it?
#      define INT_is_64_ 2
#    else
#      define INT_is_64_ 1
#    endif
#  endif
#else
#  ifdef LATATOOLS
using trustIdType = std::int64_t;   // In lata_tools and VisIt lata plugin, trustIdType is always 64b, so that the tools
//                                  // can deal with any type of lata files (32b or 64b)
#  else
using trustIdType = int;
#  endif
#endif

#endif // _ARCH_H_
