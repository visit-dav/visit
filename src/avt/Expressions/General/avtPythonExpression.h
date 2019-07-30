/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
//                          avtPythonExpression.h                            //
// ************************************************************************* //

#ifndef AVT_PYTHON_EXPRESSION_H
#define AVT_PYTHON_EXPRESSION_H

#include <avtExpressionFilter.h>

class avtPythonFilterEnvironment;

// ****************************************************************************
//  Class: avtPythonExpression
//
//  Purpose:
//      Interface to python expressions.
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Feb  2 13:45:21 PST 2010
//
//  Modifications:
//   Cyrus Harrison, Tue Jan 11 16:33:27 PST 2011
//   Added exprType & exprDescription members.
//
// ****************************************************************************

class EXPRESSION_API avtPythonExpression : public avtExpressionFilter
{
  public:
                              avtPythonExpression();
    virtual                  ~avtPythonExpression();
    void                      CleanUp();

    virtual const char       *GetType(void);
    virtual const char       *GetDescription(void);

    virtual int               NumVariableArguments(void);
    virtual int               GetVariableDimension(void);
    virtual bool              IsPointVariable(void);

    virtual void              ProcessArguments(ArgsExpr *, ExprPipelineState *);
    virtual void              AddInputVariableName(const char *);


  protected:
    // Keep track of this filter's input variables
    std::vector<char *>       varnames;

    virtual void              Execute(void);
    virtual avtContract_p     ModifyContract(avtContract_p);


  private:
    avtPythonFilterEnvironment *pyEnv;
    std::string                 pyScript;

    std::string                 exprType;
    std::string                 exprDescription;

};


#endif


