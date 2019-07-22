// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtVMetricShear.h                                //
// ************************************************************************* //

#ifndef AVT_VMETRIC_SHEAR_H
#define AVT_VMETRIC_SHEAR_H
#include <expression_exports.h>
#include <avtVerdictExpression.h>

// ****************************************************************************
//  Class: avtVMetricShear
//
//  Purpose:
//    This metric measures shear.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricShear : public avtVerdictExpression
{
    public:
        double Metric(double coords[][3], int type);
    protected:
        virtual bool              RequiresSizeCalculation() { return true; } 
};

#endif


