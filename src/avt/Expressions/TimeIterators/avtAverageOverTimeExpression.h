// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     avtAverageOverTimeExpression.h                        //
// ************************************************************************* //

#ifndef AVT_AVERAGE_OVER_TIME_EXPRESSION_H
#define AVT_AVERAGE_OVER_TIME_EXPRESSION_H

#include <avtTimeIteratorDataTreeIteratorExpression.h>


// ****************************************************************************
//  Class: avtAverageOverTimeExpression
//
//  Purpose:
//      A derived type of time iterator/data tree iterator that calculates the
//      average over time.
//
//  Programmer: Hank Childs
//  Creation:   February 16, 2009
//
// ****************************************************************************

class EXPRESSION_API avtAverageOverTimeExpression 
    : public avtTimeIteratorDataTreeIteratorExpression
{
  public:
                              avtAverageOverTimeExpression();
    virtual                  ~avtAverageOverTimeExpression();

    virtual const char       *GetType(void)   
                                  { return "avtAverageOverTimeExpression"; };
    virtual const char       *GetDescription(void)   
                                  { return "Calculating average over time"; };

  protected:
    virtual void              ExecuteDataset(std::vector<vtkDataArray *> &, 
                                             vtkDataArray *, int ts);
    virtual int               NumberOfVariables(void) { return 1; };
};


#endif


