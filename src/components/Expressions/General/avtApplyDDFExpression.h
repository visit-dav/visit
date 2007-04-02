// ************************************************************************* //
//                           avtApplyDDFExpression.h                         //
// ************************************************************************* //

#ifndef AVT_APPLY_DDF_EXPRESSION_H
#define AVT_APPLY_DDF_EXPRESSION_H


#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;

class     avtDDF;


typedef avtDDF *   (*GetDDFCallback)(void *, const char *);


// ****************************************************************************
//  Class: avtApplyDDFExpression
//
//  Purpose:
//      Will retrieve a derived data function and make a new expression out of
//      it.
//          
//  Programmer: Hank Childs
//  Creation:   February 18, 2006
//
// ****************************************************************************

class EXPRESSION_API avtApplyDDFExpression 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtApplyDDFExpression();
    virtual                  ~avtApplyDDFExpression();

    virtual const char       *GetType(void) 
                                 { return "avtApplyDDFExpression"; };
    virtual const char       *GetDescription(void)
                                 { return "Applying derived data function."; };

    static void               RegisterGetDDFCallback(GetDDFCallback, void *);
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);

    avtPipelineSpecification_p 
                              PerformRestriction(avtPipelineSpecification_p);

  protected:
    avtDDF                   *theDDF;
    std::string               ddfName;

    static  GetDDFCallback    getDDFCallback;
    static  void             *getDDFCallbackArgs;

    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual int               GetVariableDimension(void) { return 1; };
};


#endif


