// ************************************************************************* //
//                          avtVMetricRelativeSize.h                         //
// ************************************************************************* //

#ifndef AVT_VMETRIC_RELATIVE_SIZE_H
#define AVT_VMETRIC_RELATIVE_SIZE_H
#include <expression_exports.h>
#include <avtVerdictFilter.h>

// ****************************************************************************
//  Class: avtVMetricRelativeSize
//
//  Purpose:
//    This metric measures relative_size.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricRelativeSize : public avtVerdictFilter
{
    public:
        double Metric(double coords[][3], int type);
    protected:
        virtual bool              RequiresSizeCalculation() { return true; }
};

#endif


