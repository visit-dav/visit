// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtExpressionFilter.h                          //
// ************************************************************************* //

#ifndef AVT_EXPRESSION_FILTER_H
#define AVT_EXPRESSION_FILTER_H

#include <expression_exports.h>

#include <string>

#include <avtDataTreeIterator.h>
#include <avtTypes.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;
class     ExprNode;


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
//    Add currentTimeState, ExamineContract.
//
//    Hank Childs, Mon Dec 27 10:13:44 PST 2004
//    Separate out parts that are related to streaming.
//
//    Hank Childs, Fri Aug  5 16:40:12 PDT 2005
//    Added GetVariableType.
//
//    Hank Childs, Mon Aug 29 14:44:20 PDT 2005
//    Added SetExpressionAtts.
//
//    Hank Childs, Sun Jan 22 12:38:57 PST 2006
//    Made Recenter be a static, public function.  Also improved implementation
//    of GetVariableDimension.
//
//    Hank Childs, Mon Jan  8 10:17:15 PST 2007
//    Added method to determine variable type of variable.
//
//    Sean Ahern, Mon Dec 10 09:59:51 EST 2007
//    Added an expression name to Recenter for error messages.
//
//    Hank Childs, Sun Jan 13 13:46:15 PST 2008
//    Add method for handling singleton constants.
//
//    Sean Ahern, Wed Sep 10 12:21:41 EDT 2008
//    Added a target centering.
//
//    Kathleen Bonnell, Thu Apr 16 09:50:56 PDT 2009
//    Added GetOutputVariableName.
//
//    Kathleen Bonnell, Fri May  8 13:44:16 PDT 2009
//    Added method GetNumericVal (moved from avtCylindricalRadiusExpression.h.
//
//    Hank Childs, Thu Aug 26 16:36:30 PDT 2010
//    Add data member for whether or not to calculate the extents.
//
//    Kathleen Biagas, Thu Apr 5 10:07:43 PDT 2012
//    Added CreateArrayFromMesh.
//
//    Eddie Rusu, Wed Sep 11 08:59:52 PDT 2019
//    Added nProcessedArgs to keep track of the number of arguments that this
//    filter processes in ProcessArguments().
//
// ****************************************************************************

class EXPRESSION_API avtExpressionFilter : virtual public 
                                                     avtDatasetToDatasetFilter
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
    static vtkDataArray     *Recenter(vtkDataSet*, vtkDataArray*,
                                     avtCentering, std::string name,
                                     avtCentering targCent = AVT_UNKNOWN_CENT);

    static vtkDataArray     *CreateArrayFromMesh(vtkDataSet*);

    const char              *GetOutputVariableName() 
                                 { return outputVariableName; }

    bool                     GetNumericVal(ExprNode *, double &res);

  protected:
    char                    *outputVariableName;
    int                      currentTimeState;
    int                      nProcessedArgs;
    bool                     calculateExtents;

    virtual bool             IsPointVariable();

    virtual void             PreExecute(void);
    virtual void             PostExecute(void);
    virtual void             UpdateDataObjectInfo(void);
    void                     SetExpressionAttributes(const avtDataAttributes &,
                                                     avtDataAttributes &);
    virtual avtContract_p
                             ModifyContract(avtContract_p);
    virtual void             ExamineContract(avtContract_p);

    virtual int              GetVariableDimension();
    virtual avtVarType       GetVariableType() { return AVT_UNKNOWN_TYPE; };

    //
    // The method "GetVariableType" declares what the variable type of the
    // output is.  This method will determine the variable type of an
    // existing variable.
    //
    avtVarType               DetermineVariableType(std::string &);

    void                     UpdateExtents(avtDataTree_p);
    virtual bool             CanHandleSingletonConstants(void) 
                                                             { return false; };
};


#endif


