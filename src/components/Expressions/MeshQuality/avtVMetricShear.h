// ************************************************************************* //
//                          avtVMetricShear.h                                //
// ************************************************************************* //

#ifndef AVT_VMETRIC_SHEAR_H
#define AVT_VMETRIC_SHEAR_H
#include <expression_exports.h>
#include <avtVerdictFilter.h>

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

class EXPRESSION_API avtVMetricShear : public avtVerdictFilter
{
    public:
        double Metric(double coords[][3], int type);
    protected:
        virtual bool              RequiresSizeCalculation() { return true; } 
};

#endif


