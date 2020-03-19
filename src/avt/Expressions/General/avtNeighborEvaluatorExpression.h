// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                    avtNeighborEvaluatorExpression.h                       //
// ************************************************************************* //

#ifndef AVT_NEIGHBOR_EVALUATOR_FILTER_H
#define AVT_NEIGHBOR_EVALUATOR_FILTER_H

#include <avtSingleInputExpressionFilter.h>
#include <vtkType.h>

class vtkDataArray;

// ****************************************************************************
//  Class: avtNeighborEvaluatorExpression
//
//  Purpose:
//      A filter that can evaluate neighbor's values.  Options include biggest,
//      smallest, and average.
//
//  Programmer: Hank Childs
//  Creation:   September 17, 2004
//
//  Modifications:
// 
//    Hank Childs, Thu Jan 20 11:10:08 PST 2005
//    Add better warnings when error conditions are encountered.
//
// ****************************************************************************

class EXPRESSION_API avtNeighborEvaluatorExpression 
    : public avtSingleInputExpressionFilter
{
  public:

    typedef enum
    {
        BIGGEST_NEIGHBOR,
        SMALLEST_NEIGHBOR,
        AVERAGE_NEIGHBOR
    } EvaluationType;

                              avtNeighborEvaluatorExpression();
    virtual                  ~avtNeighborEvaluatorExpression();
    void                      SetEvaluationType(EvaluationType t)
                                         { evaluationType = t; };

    virtual const char       *GetType(void)   
                                  { return "avtNeighborEvaluatorExpression"; };
    virtual const char       *GetDescription(void)
                                  { return "Calculating neighbors values"; };
    virtual void              PreExecute(void);

  protected:
    EvaluationType            evaluationType;
    bool                      haveIssuedWarning;

    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual avtContract_p
                              ModifyContract(avtContract_p);

    template <class Accessor>
    void                     DeriveVariableT(vtkDataSet *, bool, 
                                 vtkDataArray *, vtkDataArray *&);
    template <class Accessor>
    void                     EvaluateNeighbor(vtkIdType, int *, double, 
                                 vtkDataArray *&);
};


#endif


