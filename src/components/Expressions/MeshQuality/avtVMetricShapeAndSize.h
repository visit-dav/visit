// ************************************************************************* //
//                          avtVMetricShapeAndSize.h                         //
// ************************************************************************* //

#ifndef AVT_VMETRIC_SHAPE_AND_SIZE_H
#define AVT_VMETRIC_SHAPE_AND_SIZE_H
#include <expression_exports.h>
#include <avtVerdictFilter.h>

// ****************************************************************************
//  Class: avtVMetricShapeAndSize
//
//  Purpose:
//    This metric measures shape_and_size.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricShapeAndSize : public avtVerdictFilter
{
    public:
        double Metric(double coords[][3], int type);
    protected:
        virtual bool              RequiresSizeCalculation() { return true; }
};

#endif


