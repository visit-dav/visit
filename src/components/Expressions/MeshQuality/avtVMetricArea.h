// ************************************************************************* //
//                          avtVMetricArea.h                                 //
// ************************************************************************* //

#ifndef AVT_VMETRIC_AREA_H
#define AVT_VMETRIC_AREA_H
#include <expression_exports.h>
#include <avtVerdictFilter.h>

// ****************************************************************************
//  Class: avtVMetricArea
//
//  Purpose:
//      This metric measures area.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricArea : public avtVerdictFilter
{
    public:
        double Metric(double coords[][3], int type);
};

#endif
