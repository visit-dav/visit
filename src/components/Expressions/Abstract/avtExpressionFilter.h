// ************************************************************************* //
//                            avtExpressionFilter.h                          //
// ************************************************************************* //

#ifndef AVT_EXPRESSION_FILTER_H
#define AVT_EXPRESSION_FILTER_H

#include <expression_exports.h>
#include <avtStreamer.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;

// ****************************************************************************
//  Class: avtExpressionFilter
//
//  Purpose:
//      This is a base class that lets derived types not worry about how to set
//      up a derived variable.
//
//  Programmer: Hank Childs
//  Creation:   June 7, 2002
//
//  Modifications:
//      Sean Ahern, Fri Jun 13 11:18:07 PDT 2003
//      Added the virtual function NumVariableArguments that lets an
//      expression declare how many of its arguments are variables.
//
// ****************************************************************************

class EXPRESSION_API avtExpressionFilter : public avtStreamer
{
  public:
                             avtExpressionFilter();
    virtual                 ~avtExpressionFilter();

    void                     SetOutputVariableName(const char *);
    virtual void             AddInputVariableName(const char *var)
                                {SetActiveVariable(var);}

    virtual void             ProcessArguments(ArgsExpr *, ExprPipelineState *);
    virtual int              NumVariableArguments() = 0;

  protected:
    char                    *outputVariableName;

    virtual bool             IsPointVariable();

    virtual void             PreExecute(void);
    virtual void             PostExecute(void);
    virtual vtkDataSet      *ExecuteData(vtkDataSet *, int, std::string);
    virtual void             RefashionDataObjectInfo(void);
    virtual avtPipelineSpecification_p
                             PerformRestriction(avtPipelineSpecification_p);

    virtual int              GetVariableDimension() { return 1; };
    virtual vtkDataArray    *DeriveVariable(vtkDataSet *) = 0;
};


#endif


