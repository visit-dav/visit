/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//               avtTimeIteratorDataTreeIteratorExpression.h                 //
// ************************************************************************* //

#ifndef AVT_TIME_ITERATOR_DATA_TREE_ITERATOR_EXPRESSION_H
#define AVT_TIME_ITERATOR_DATA_TREE_ITERATOR_EXPRESSION_H

#include <avtTimeIteratorExpression.h>


// ****************************************************************************
//  Class: avtTimeIteratorDataTreeIteratorExpression
//
//  Purpose:
//      A derived type of time iterator that's role is to iterate over data
//      data trees, much like avtDataTreeIterator.  This is an abstract class,
//      and its derived types do the actual work.
//
//  Programmer: Hank Childs
//  Creation:   February 14, 2009
//
//  Modifications:
//    Jeremy Meredith, Wed Mar 18 14:06:58 EDT 2009
//    Added support for variable types other than scalars.
//
// ****************************************************************************

class EXPRESSION_API avtTimeIteratorDataTreeIteratorExpression 
    : public avtTimeIteratorExpression
{
  public:
                              avtTimeIteratorDataTreeIteratorExpression();
    virtual                  ~avtTimeIteratorDataTreeIteratorExpression();

    virtual const char       *GetType(void)   
                                  { return "avtTimeIteratorDataTreeIteratorExpression"; };

  protected:
    int                          arrayIndex;
    std::vector<vtkDataArray *>  vars;

    virtual void              ProcessDataTree(avtDataTree_p, int ts);
    void                      InternalProcessDataTree(avtDataTree_p, int ts);
    virtual void              InitializeOutput(void);
    virtual void              FinalizeOutput(void);

    virtual void              PrepareAndExecuteDataset(vtkDataSet *, int);
    virtual void              ExecuteDataset(std::vector<vtkDataArray *> &, 
                                             vtkDataArray *, int) = 0;

    virtual int               GetIntermediateSize(void)
                                    { return GetVariableDimension(); };
    virtual vtkDataArray     *ConvertIntermediateArrayToFinalArray(vtkDataArray *);

    avtDataTree_p             ConstructOutput(avtDataTree_p);
    virtual avtVarType        GetVariableType();
    virtual int               GetVariableDimension();
};


#endif


