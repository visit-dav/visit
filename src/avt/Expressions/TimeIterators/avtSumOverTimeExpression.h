// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     avtSumOverTimeExpression.h                            //
// ************************************************************************* //

#ifndef AVT_SUM_OVER_TIME_EXPRESSION_H
#define AVT_SUM_OVER_TIME_EXPRESSION_H

#include <avtTimeIteratorDataTreeIteratorExpression.h>


// ****************************************************************************
//  Class: avtSumOverTimeExpression
//
//  Purpose:
//      A derived type of time iterator/data tree iterator that calculates the
//      summation over time.
//
//  Programmer: Hank Childs
//  Creation:   February 16, 2009
//
// ****************************************************************************

class EXPRESSION_API avtSumOverTimeExpression 
    : public avtTimeIteratorDataTreeIteratorExpression
{
  public:
                              avtSumOverTimeExpression();
    virtual                  ~avtSumOverTimeExpression();

    virtual const char       *GetType(void)   
                                  { return "avtSumOverTimeExpression"; };
    virtual const char       *GetDescription(void)   
                                  { return "Calculating summation over time"; };

  protected:
    virtual void              ExecuteDataset(std::vector<vtkDataArray *> &, 
                                             vtkDataArray *, int ts);
    virtual int               NumberOfVariables(void) { return 1; };
};


#endif


