// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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


