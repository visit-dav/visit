// ************************************************************************* //
//                          avtVMetricDiagonal.h                             //
// ************************************************************************* //

#ifndef AVT_VMETRIC_DIAGONAL_H
#define AVT_VMETRIC_DIAGONAL_H
#include <expression_exports.h>
#include <avtVerdictFilter.h>

// ****************************************************************************
//  Class: avtVMetricDiagonal
//
//  Purpose:
//    This metric measures aspect ratio.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricDiagonal : public avtVerdictFilter
{
    public:
        double Metric(double coords[][3], int type);
};

#endif


