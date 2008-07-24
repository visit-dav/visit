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
//                           avtVerdictExpression.h                          //
// ************************************************************************* //

// Caveat: Verdict filters currently support triangles, but not triangle strips

#ifndef AVT_VERDICT_FILTER_H
#define AVT_VERDICT_FILTER_H

#include <expression_exports.h>

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtVerdictExpression
//
//  Purpose:
//    This is a abstract base class for the verdict metric filters.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Hank Childs, Thu May 19 10:44:29 PDT 2005
//    Add support for sub-types operating directly on the mesh.
//
//    Hank Childs, Mon Aug 28 10:29:49 PDT 2006
//    Declare the variable dimension.
//
//    Hank Childs, Thu Jul 24 12:49:19 PDT 2008
//    Added a virtual method to help with supporting polygonal and polyhedral
//    data.
//
// ****************************************************************************

class EXPRESSION_API avtVerdictExpression : public avtSingleInputExpressionFilter
{
  public:
                              avtVerdictExpression();
    virtual                  ~avtVerdictExpression();

    virtual void              PreExecute();

    virtual const char       *GetType(void)   { return "avtVerdictExpression"; };
    virtual const char       *GetDescription(void)
                                 { return "Calculating Verdict expression."; };

    virtual bool              OperateDirectlyOnMesh(vtkDataSet *)
                                       { return false; };
    virtual void              MetricForWholeMesh(vtkDataSet *, vtkDataArray *);

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);

    virtual double            Metric(double coordinates[][3], int type) = 0;

    virtual bool              RequiresSizeCalculation() { return false; };
    virtual bool              SummationValidForOddShapes(void) { return false; };

    virtual bool              IsPointVariable() {  return false; };
    virtual int               GetVariableDimension(void) { return 1; };
};

#endif
