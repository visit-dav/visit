// ************************************************************************* //
//                            avtExpressionFilter.h                          //
// ************************************************************************* //

#ifndef AVT_EXPRESSION_FILTER_H
#define AVT_EXPRESSION_FILTER_H

#include <expression_exports.h>

#include <string>

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
//
//    Sean Ahern, Fri Jun 13 11:18:07 PDT 2003
//    Added the virtual function NumVariableArguments that lets an
//    expression declare how many of its arguments are variables.
//
//    Hank Childs, Tue Aug 12 10:34:02 PDT 2003
//    Store the domain label and index when deriving a variable.  Certain
//    filters need this information.
//  
//    Hank Childs, Fri Oct 24 14:46:20 PDT 2003
//    Made Evaluator be a friend class so that it could call perform
//    restriction on the expressions it contains.
//
//    Hank Childs, Wed Dec 10 09:40:18 PST 2003
//    Add support for recentering a variable.  It is at this level so all
//    derived types can use the same routine.
// 
//    Kathleen Bonnell, Mon Jun 28 07:48:55 PDT 2004 
//    Add currentTimeState, ExamineSpecification.
//
// ****************************************************************************

class EXPRESSION_API avtExpressionFilter : public avtStreamer
{
    friend class             avtExpressionEvaluatorFilter;

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
    std::string              currentDomainsLabel;
    int                      currentDomainsIndex;
    int                      currentTimeState;

    virtual bool             IsPointVariable();

    virtual void             PreExecute(void);
    virtual void             PostExecute(void);
    virtual vtkDataSet      *ExecuteData(vtkDataSet *, int, std::string);
    virtual void             RefashionDataObjectInfo(void);
    virtual avtPipelineSpecification_p
                             PerformRestriction(avtPipelineSpecification_p);
    virtual void             ExamineSpecification(avtPipelineSpecification_p);

    virtual int              GetVariableDimension() { return 1; };
    virtual vtkDataArray    *DeriveVariable(vtkDataSet *) = 0;

    vtkDataArray            *Recenter(vtkDataSet*, vtkDataArray*,avtCentering);
};


#endif
