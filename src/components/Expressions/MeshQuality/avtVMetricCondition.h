// ************************************************************************* //
//                          avtVMetricCondition.h                            //
// ************************************************************************* //

#ifndef AVT_VMETRIC_CONDITION_H
#define AVT_VMETRIC_CONDITION_H
#include <expression_exports.h>
#include <avtVerdictFilter.h>

// ****************************************************************************
//  Class: avtVMetricCondition
//
//  Purpose:
//    This metric measures condition.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricCondition : public avtVerdictFilter
{
    public:
        double Metric(double coords[][3], int type);
};

#endif


