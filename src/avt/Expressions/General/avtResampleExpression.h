// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtResampleExpression.h                        //
// ************************************************************************* //

#ifndef AVT_RESAMPLE_EXPRESSION_H
#define AVT_RESAMPLE_EXPRESSION_H

#include <avtSingleInputExpressionFilter.h>
#include <string>


// ****************************************************************************
//  Class: avtResampleExpression
//
//  Purpose:
//      Resamples a variable.
//          
//  Programmer: Dave Pugmire
//  Creation:   Fri Jul  2 14:10:45 EDT 2010
//
//  Modifications:
//
//
// ****************************************************************************

class EXPRESSION_API avtResampleExpression : public avtExpressionFilter
{
  public:
                              avtResampleExpression();
    virtual                  ~avtResampleExpression();

    virtual const char       *GetType(void) { return "avtResampleExpression"; };
    virtual const char       *GetDescription(void) {return "Resampling";};
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);

  protected:
    virtual void              Execute(void);
    virtual int               NumVariableArguments(){ return 0; }
    virtual int               GetVariableDimension(void);
    virtual bool              CanHandleSingletonConstants(void) {return true;}
    virtual bool              IsPointVariable(void);
    
    int                       samplesX, samplesY, samplesZ;
};


#endif


