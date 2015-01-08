/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/
#ifndef simV2_TypeTraits_hxx
#define simV2_TypeTraits_hxx

#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkIntArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkLongArray.h>

// template type traits for conversions between VisIt's datatype 
// enumerations (eg VISIT_DATATYPE_FLOAT), VTK arrays and enumerations
// (eg vtkFloatArray, VTK_FLOAT), and C++ types (eg. float).
// Given a VisIt C++ type enum, the class provides the following
// typdefs:
//      ::cppType -> C++ type (eg float)
//      ::vtkType -> a vtkDataArray subclass (eg vtkFloatArray)
//      ::vtkEnum -> the VTK C++ type enum (eg VTK_FLOAT)
template<int E> class simV2_TypeTraits {};

#define simV2_TypeTraits_Specialize(visitE, cppT, vtkT, vtkE)   \
template<> class simV2_TypeTraits<visitE>                       \
{                                                               \
public:                                                         \
    typedef cppT cppType;                                       \
    typedef vtkT vtkType;                                       \
    enum { vtkEnum =  vtkE };                                   \
};
simV2_TypeTraits_Specialize(VISIT_DATATYPE_FLOAT, float, vtkFloatArray, VTK_FLOAT);
simV2_TypeTraits_Specialize(VISIT_DATATYPE_DOUBLE, double, vtkDoubleArray, VTK_DOUBLE);
simV2_TypeTraits_Specialize(VISIT_DATATYPE_INT, int, vtkIntArray, VTK_INT);
simV2_TypeTraits_Specialize(VISIT_DATATYPE_CHAR, unsigned char, vtkUnsignedCharArray, VTK_UNSIGNED_CHAR);
simV2_TypeTraits_Specialize(VISIT_DATATYPE_LONG, long, vtkLongArray, VTK_LONG);


// The simV2TemplateMacro avoids duplication of long switch statement
// case lists. It enumerates the "case" body for each datatype in
// simV2's API providing "simV2_TT" typedef for each case.
//
// This version of the macro allows the template to take any number of
// arguments.  Example usage:
// switch(dataType)
//   {
//   simV2TemplateMacro(
//       myFunc(static_cast<simV2_TT::cppType>(data), arg2));
//   }
#define simV2TemplateMacroCase(visitE, call)                                \
  case visitE: { typedef simV2_TypeTraits<visitE> simV2_TT; call; }; break
#define simV2TemplateMacro(call)                                            \
  simV2TemplateMacroCase(VISIT_DATATYPE_FLOAT, call);                       \
  simV2TemplateMacroCase(VISIT_DATATYPE_DOUBLE, call);                      \
  simV2TemplateMacroCase(VISIT_DATATYPE_INT, call);                         \
  simV2TemplateMacroCase(VISIT_DATATYPE_CHAR, call);                        \
  simV2TemplateMacroCase(VISIT_DATATYPE_LONG, call);

// The simV2FloatTemplateMacro does the same things as simV2TemplateMacro
// however it only expands the cases for float and double. This addresses
// common situations in numerical computation where integers don't make sense
// and for use with vtkPoints which only suppport float and double.
#define simV2FloatTemplateMacro(call)                                            \
  simV2TemplateMacroCase(VISIT_DATATYPE_FLOAT, call);                       \
  simV2TemplateMacroCase(VISIT_DATATYPE_DOUBLE, call);

// verify that the type enum names one of the supported types
inline
bool simV2_ValidDataType(int dataType)
{
    if ( (dataType == VISIT_DATATYPE_CHAR)
      || (dataType == VISIT_DATATYPE_INT)
      || (dataType == VISIT_DATATYPE_LONG)
      || (dataType == VISIT_DATATYPE_FLOAT)
      || (dataType == VISIT_DATATYPE_DOUBLE) )
    {
        return true;
    }
    return false;
}

// verify that the type enum names one of the supported types
inline
bool simV2_ValidFloatDataType(int dataType)
{
    if ( (dataType == VISIT_DATATYPE_FLOAT)
      || (dataType == VISIT_DATATYPE_DOUBLE) )
    {
        return true;
    }
    return false;
}
#endif
