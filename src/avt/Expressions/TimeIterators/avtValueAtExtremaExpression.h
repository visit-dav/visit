/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
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
//                    avtValueAtExtremaExpression.h                          //
// ************************************************************************* //

#ifndef AVT_VALUE_AT_EXTREMA_EXPRESSION_H
#define AVT_VALUE_AT_EXTREMA_EXPRESSION_H

#include <avtTimeIteratorDataTreeIteratorExpression.h>


// ****************************************************************************
//  Class: avtValueAtExtremaExpression
//
//  Purpose:
//      A derived type of time iterator/data tree iterator that calculates the
//      time/cycle/time index/separate variable at the minimum/maximum.
//
//  Programmer: Hank Childs
//  Creation:   February 16, 2009
//
// ****************************************************************************

typedef enum
{
    VE_OUTPUT_TIME,
    VE_OUTPUT_CYCLE,
    VE_OUTPUT_TIME_INDEX,
    VE_OUTPUT_VARIABLE
} VE_OutputType;


class EXPRESSION_API avtValueAtExtremaExpression 
    : public avtTimeIteratorDataTreeIteratorExpression
{
  public:
                              avtValueAtExtremaExpression();
    virtual                  ~avtValueAtExtremaExpression();

    virtual const char       *GetType(void)   
                               { return "avtValueAtExtremaExpression"; };
    virtual const char       *GetDescription(void)   
                               { return "Calculating value at extrema";};

    void                      SetAtMaximum(bool b) {atMaximum=b;};
    void                      SetOutputType(VE_OutputType t) {outputType=t;};

  protected:
    VE_OutputType             outputType;
    bool                      atMaximum;

    virtual void              ExecuteDataset(std::vector<vtkDataArray *> &, 
                                             vtkDataArray *, int ts);
    virtual int               NumberOfVariables(void) 
                                   { return (outputType == VE_OUTPUT_VARIABLE 
                                             ? 2 : 1); };
    virtual bool              CanHandleSingtonConstants(void) {return true;};

    virtual int               GetIntermediateSize(void)
                                    { return 1+GetVariableDimension(); };
    virtual vtkDataArray     *ConvertIntermediateArrayToFinalArray(vtkDataArray *);
};


#endif


