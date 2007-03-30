// ************************************************************************* //
//                          avtVMetricOddy.h                                 //
// ************************************************************************* //

#ifndef AVT_VMETRIC_ODDY_H
#define AVT_VMETRIC_ODDY_H
#include <expression_exports.h>
#include <avtVerdictFilter.h>

// ****************************************************************************
//  Class: avtVMetricOddy
//
//  Purpose:
//    This metric measures oddy.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricOddy : public avtVerdictFilter
{
    public:
        double Metric(double coords[][3], int type);
};

#endif


