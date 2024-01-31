// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     avtMultipleInputMathExpression.h                      //
// ************************************************************************* //

#ifndef AVT_MULTIPLE_INPUT_MATH_EXPRESSION_H
#define AVT_MULTIPLE_INPUT_MATH_EXPRESSION_H

#include <avtMultipleInputExpressionFilter.h>

#include <vtkDataArray.h>

// ****************************************************************************
//  Class: avtMultipleInputMathExpression
//
//  Purpose:
//      A filter that performs a math calculation with multiple arguments.
//
//  Programmer: Eddie Rusu
//  Creation:   Tue Sep 24 09:07:44 PDT 2019
//
//  Modifications:
//
//      Eddie Rusu, Mon Sep 30 15:00:24 PDT 2019
//      Added CreateOutputVariable and modified RecenterData.
//
// ****************************************************************************

class EXPRESSION_API avtMultipleInputMathExpression
    : public avtMultipleInputExpressionFilter
{
    public:
                 avtMultipleInputMathExpression();
        virtual ~avtMultipleInputMathExpression();

        virtual const char *GetType() {
                return "avtMultipleInputMathExpression";
            };
        virtual const char *GetDescription() = 0;
        virtual int         NumVariableArguments() {return nProcessedArgs;};

    protected:
        virtual vtkDataArray *DeriveVariable(vtkDataSet*, int);
        virtual vtkDataArray *ExtractCenteredData(avtCentering*, vtkDataSet*,
                                                  const char*);
        virtual vtkDataArray *CreateOutputVariable();
        virtual vtkDataArray *CreateOutputVariable(int);
        virtual vtkDataArray *DoOperation() = 0;
        virtual void          RecenterData(vtkDataSet*);

        avtCentering centering;
        std::vector<vtkDataArray*> dataArrays;
        std::vector<avtCentering>  centerings;
};

#endif
