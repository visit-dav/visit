// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//               avtPerformColorTableLookupExpression.h                      //
// ************************************************************************* //

#ifndef AVT_PERFORMCOLORTABLELOOKUP_FILTER_H
#define AVT_PERFORMCOLORTABLELOOKUP_FILTER_H

#include <avtUnaryMathExpression.h>

#include <string>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtPerformColorTableLookupExpression
//
//  Purpose:
//      A filter that converts a scalar variable to a color by performing a
//      color table lookup.
//
//  Programmer: Gunther H. Weber
//  Creation:   Tue Jan  8 16:27:49 PST 2008
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtPerformColorTableLookupExpression : public avtUnaryMathExpression
{
  public:
                              avtPerformColorTableLookupExpression();
    virtual                  ~avtPerformColorTableLookupExpression();

    virtual const char       *GetType(void)
                                  { return "avtPerformColorTableLookupExpression"; };
    virtual const char       *GetDescription(void) 
                                  { return "Perform color table lookup"; };
    virtual void              PreExecute(void);
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);

  protected:
    virtual int               GetVariableDimension()
                                  { return 3; }
    virtual int               GetNumberOfComponentsInOutput(int numInInput)
                                  { return 3; }
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
  private:
    enum LookupMapping         { Identity = 0, Log = 1, Skew = 2 } ;
    std::string                mLUTName;
    double                     mExtents[2];
    LookupMapping              mLUTMapping;
    double                     mSkew;
};

#endif


