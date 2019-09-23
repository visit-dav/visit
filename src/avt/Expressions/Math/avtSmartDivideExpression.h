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
//                          avtSmartDivideExpression.h                       //
// ************************************************************************* //

#ifndef AVT_SMART_DIVIDE_EXPRESSION_H
#define AVT_SMART_DIVIDE_EXPRESSION_H

#include <avtMultipleInputMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtBinaryDivideExpression
//
//  Purpose:
//      A filter that calculates the quotient of its two inputs
//
//  Programmer: Sean Ahern
//  Creation:   Tue Jun 11 16:23:45 PDT 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Mon Jan 14 17:58:58 PST 2008
//    Allow constants to be created as singletons.
//
//    Eddie Rusu, Wed Sep 11 08:59:52 PDT 2019
//    Added the notion and logic of "smart division" or "guarded division".
//
// ****************************************************************************

class EXPRESSION_API avtSmartDivideExpression
    : public avtMultipleInputMathExpression
{
  public:
                              avtSmartDivideExpression();
    virtual                  ~avtSmartDivideExpression();

    virtual const char       *GetType(void) 
                                     { return "avtSmartDivideExpression"; };
    virtual const char       *GetDescription(void) 
                                     { return "Calculating smart division"; };
    virtual int           NumVariableArguments() {
                              return nProcessedArgs > 4 ? 4 : nProcessedArgs;
                          };

  protected:
    virtual vtkDataArray *DoOperation();
    virtual void          RecenterData(vtkDataSet*);
    virtual bool          CanHandleSingletonConstants(void) {return true;};
  
  private:
    double CheckZero(double, double);

    double tolerance;
    double value_if_zero;
    
};


#endif


