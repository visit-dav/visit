// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtApplyDataBinningExpression.h                      //
// ************************************************************************* //

#ifndef AVT_APPLY_DATA_BINNING_EXPRESSION_H
#define AVT_APPLY_DATA_BINNING_EXPRESSION_H


#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;

class     avtDataBinning;


typedef avtDataBinning *   (*GetDataBinningCallback)(void *, const char *);


// ****************************************************************************
//  Class: avtApplyDataBinningExpression
//
//  Purpose:
//      Will retrieve a data binning and make a new expression out of
//      it.
//          
//  Programmer: Hank Childs
//  Creation:   February 18, 2006
//
//  Modifications:
//
//    Hank Childs, Sat Aug 21 14:05:14 PDT 2010
//    Rename expressions (DDFs to DataBinnings).
//
// ****************************************************************************

class EXPRESSION_API avtApplyDataBinningExpression 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtApplyDataBinningExpression();
    virtual                  ~avtApplyDataBinningExpression();

    virtual const char       *GetType(void) 
                                 { return "avtApplyDataBinningExpression"; };
    virtual const char       *GetDescription(void)
                                 { return "Applying derived data function."; };

    static void               RegisterGetDataBinningCallback(GetDataBinningCallback, void *);
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);

    avtContract_p 
                              ModifyContract(avtContract_p);

  protected:
    avtDataBinning           *theDataBinning;
    std::string               dbName;

    static  GetDataBinningCallback    getDataBinningCallback;
    static  void                     *getDataBinningCallbackArgs;

    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual int               GetVariableDimension(void) { return 1; };
};


#endif


