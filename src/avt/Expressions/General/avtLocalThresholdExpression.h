// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtLocalThresholdExpression.h                        //
// ************************************************************************* //

#ifndef AVT_LOCALTHRESHOLD_EXPRESSION_H
#define AVT_LOCALTHRESHOLD_EXPRESSION_H


#include <avtMultipleInputExpressionFilter.h>
#include <avtMergeTreeExpression.h>
#include <vtkSmartPointer.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;
class     ConstExpr;

// ****************************************************************************
//  Class: avtLocalThresholdExpression
//
//  Purpose:
//      Uses a merge/split tree to apply a local threshold to a scalar 
//      function.
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************


class EXPRESSION_API avtLocalThresholdExpression : public avtMultipleInputExpressionFilter
{
public:

    enum TransformationType
    {
        RELEVANCE = 0,
        UNDEFINED = 1,
    };

    static const char* TransformationName[];

    avtLocalThresholdExpression();
    virtual ~avtLocalThresholdExpression();

    virtual int            NumVariableArguments()
                                {return 2;}
    virtual const char    *GetType(void)
                                { return "avtLocalThresholdExpression"; };
    virtual const char    *GetDescription(void)
                                {return "Apply a local threshold using a merge tree";};

    virtual void           ProcessArguments(ArgsExpr*, ExprPipelineState *);

protected:

    virtual int            GetVariableDimension(void)
                                { return 1; };

    virtual vtkDataArray  *DeriveVariable(vtkDataSet *,
                                          int currentDomainsIndex);

    // helpers used to implement the local threshold
    class Transformation
    {
        public:
            virtual double eval(vtkMergeTree* tree,
                                vtkDataArray* function,
                                vtkDataArray* labels,
                                vtkIdType index) = 0;

            virtual double fillValue() {return 0;}
    };

    class Relevance : public Transformation
    {
        public:
            virtual double eval(vtkMergeTree* tree,
                                vtkDataArray* function,
                                vtkDataArray* labels,
                                vtkIdType index);
    };

    TransformationType    DetermineTransformationType(const std::string& name);

private:
    TransformationType transformation;
};





#endif /* AVT_LOCALTHRESHOLD_EXPRESSION_H */
