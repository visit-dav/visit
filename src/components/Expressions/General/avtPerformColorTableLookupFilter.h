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
//                             avtPerformColorTableLookupFilter.h                             //
// ************************************************************************* //

#ifndef AVT_PERFORMCOLORTABLELOOKUP_FILTER_H
#define AVT_PERFORMCOLORTABLELOOKUP_FILTER_H

#include <avtUnaryMathFilter.h>

#include <string>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtPerformColorTableLookupFilter
//
//  Purpose:
//      A filter that converts a scalar variable to a color by performing a
//      color table lookup.
//
//  Programmer: Gunther H. Weber
//  Creation:   Tue Jan  8 16:27:49 PST 2008
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtPerformColorTableLookupFilter : public avtUnaryMathFilter
{
  public:
                              avtPerformColorTableLookupFilter();
    virtual                  ~avtPerformColorTableLookupFilter();

    virtual const char       *GetType(void)
                                  { return "avtPerformColorTableLookupFilter"; };
    virtual const char       *GetDescription(void) 
                                  { return "Perform color table lookup"; };
    virtual void              PreExecute(void);
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);

  protected:
    virtual int               GetVariableDimension()
                                  { return 3; }
    virtual int               GetNumberOfComponentsInOutput(int numInInput)
                                  { return 3; }
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
  private:
    enum LookupMapping         { Identity = 0, Log = 1, Skew = 2 } ;
    std::string                mLUTName;
    double                     mExtents[2];
    LookupMapping              mLUTMapping;
    double                     mSkew;
};

#endif


