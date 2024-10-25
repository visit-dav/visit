// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtGlobalRMSExpression.h                    //
// ************************************************************************* //

#ifndef AVT_GLOBAL_RMS_FILTER_H
#define AVT_GLOBAL_RMS_FILTER_H

#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtGlobalRMSExpression
//
//  Purpose:
//      TODO
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtGlobalRMSExpression : public avtUnaryMathExpression
{
  public:
                              avtGlobalRMSExpression();
    virtual                  ~avtGlobalRMSExpression();

    virtual const char       *GetType(void)   { return "avtGlobalRMSExpression"; };
    virtual const char       *GetDescription(void) 
                                              { return "Calculating root mean square across mesh"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
};


#endif


