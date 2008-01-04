/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

// ************************************************************************* //
//                            avtConstantCreatorFilter.h                     //
// ************************************************************************* //

#ifndef AVT_CONSTANT_CREATOR_FILTER_H
#define AVT_CONSTANT_CREATOR_FILTER_H

#include <avtUnaryMathFilter.h>

// ****************************************************************************
//  Class: avtConstantCreatorFilter
//
//  Purpose:
//      This class creates constant variables.
//
//  Programmer: Sean Ahern
//  Creation:   Fri Feb 21 23:28:13 America/Los_Angeles 2003
//
//  Modifications:
//
//    Hank Childs, Mon Nov  3 15:15:47 PST 2003
//    Override base class' definition of CreateArray.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Mon Jan 23 11:32:14 PST 2006
//    Make sure constants are always scalars.
//
// ****************************************************************************

class EXPRESSION_API avtConstantCreatorFilter : public avtUnaryMathFilter
{
  public:
                             avtConstantCreatorFilter();
    virtual                 ~avtConstantCreatorFilter();

    void                     SetValue(double v) {value = v;}
    virtual const char *     GetType(void) 
                                         { return "avtConstantCreatorFilter"; }
    virtual const char *     GetDescription(void) 
                                         { return "Generating constant"; }

  protected:
    virtual void             DoOperation(vtkDataArray *in, vtkDataArray *out,
                                         int ncomponents, int ntuples);
    virtual int              GetNumberOfComponentsInOutput(int) { return 1; };
    virtual int              GetVariableDimension(void) { return 1; };
    virtual vtkDataArray    *CreateArray(vtkDataArray *);

    double                   value;
};


#endif


