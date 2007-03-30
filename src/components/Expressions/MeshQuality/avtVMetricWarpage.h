// ************************************************************************* //
//                          avtVMetricWarpage.h                              //
// ************************************************************************* //

#ifndef AVT_VMETRIC_WARPAGE_H
#define AVT_VMETRIC_WARPAGE_H
#include <expression_exports.h>
#include <avtVerdictFilter.h>

// ****************************************************************************
//  Class: avtVMetricWarpage
//
//  Purpose:
//    This metric measures Warpage.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricWarpage : public avtVerdictFilter
{
    public:
        double Metric(double coords[][3], int type);
};

#endif


