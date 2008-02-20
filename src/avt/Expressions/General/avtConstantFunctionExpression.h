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
//                           avtConstantFunctionExpression.h                          //
// ************************************************************************* //

#ifndef AVT_CONSTANT_FUNCTION_EXPRESSION_H
#define AVT_CONSTANT_FUNCTION_EXPRESSION_H

#include <avtMultipleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtConstantFunctionExpression
//
//  Purpose:
//      Create a constant-valued nodal variable on a mesh.  Useful if you
//      want to create a constant on a mesh using only a single function.
//          
//  Programmer: Jeremy Meredith
//  Creation:   February 19, 2008
//
//  Modifications:
//    Jeremy Meredith, Tue Feb 19 16:04:11 EST 2008
//    By having this function specify it only had one argument, it was
//    possible to clobber other constant creations.  I upped it to 2 and
//    had the second arg push a textual representation on the state.
//
//    Jeremy Meredith, Wed Feb 20 10:00:31 EST 2008
//    Support either nodal or zonal values.
//
// ****************************************************************************

class EXPRESSION_API avtConstantFunctionExpression
    : public avtMultipleInputExpressionFilter
{
  public:
                          avtConstantFunctionExpression(bool nodal);
    virtual              ~avtConstantFunctionExpression();

    virtual const char   *GetType() { return "avtConstantFunctionExpression"; }
    virtual const char   *GetDescription() { return "Assigning constant."; }
    virtual void          ProcessArguments(ArgsExpr*, ExprPipelineState *);
  protected:
    virtual vtkDataArray *DeriveVariable(vtkDataSet *);
    virtual bool          IsPointVariable()  { return nodal; }
    virtual int           GetVariableDimension() { return 1; }
    virtual int           NumVariableArguments(void) { return 2; }
  private:
    bool    nodal;
    float   value;
};


#endif


