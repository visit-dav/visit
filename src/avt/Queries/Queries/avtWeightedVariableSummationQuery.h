// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtWeightedVariableSummationQuery.h                  //
// ************************************************************************* //

#ifndef AVT_WEIGHTED_VARIABLE_SUMMATION_QUERY_H
#define AVT_WEIGHTED_VARIABLE_SUMMATION_QUERY_H

#include <query_exports.h>

#include <avtSummationQuery.h>

#include <string>

class     avtBinaryMultiplyExpression;
class     avtEdgeLength;
class     avtRevolvedVolume;
class     avtVMetricArea;
class     avtVMetricVolume;
class     avtConstantCreatorExpression;


// ****************************************************************************
//  Class: avtWeightedVariableSummationQuery
//
//  Purpose:
//      A query that will sum all of one variables values.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Jul 28 08:50:51 PDT 2004
//    Added VerifyInput.
//
//    Kathleen Bonnell, Fri Feb  3 10:32:12 PST 2006
//    Added revolvedVolume.
//
//    Hank Childs, Thu May 11 13:28:50 PDT 2006
//    Added new virtual methods so that new queries can inherit from this.
//
//    Hank Childs, Wed Apr 28 05:25:52 PDT 2010
//    Add support for 1D cross sections.
//
//    Kathleen Biagas, Wed Sep 11, 2024
//    Added GetTimeCurveSpecs.
//
// ****************************************************************************

class QUERY_API avtWeightedVariableSummationQuery : public avtSummationQuery
{
  public:
                         avtWeightedVariableSummationQuery();
    virtual             ~avtWeightedVariableSummationQuery();

    virtual const char  *GetType(void)
                             { return "avtWeightedVariableSummationQuery"; }

    const MapNode       &GetTimeCurveSpecs(const QueryAttributes *) override;

  protected:
    avtEdgeLength               *length;
    avtVMetricArea              *area;
    avtVMetricVolume            *volume;
    avtRevolvedVolume           *revolvedVolume;
    avtBinaryMultiplyExpression *multiply;
    avtConstantCreatorExpression *constExpr;

    virtual avtDataObject_p    ApplyFilters(avtDataObject_p);
    virtual int                GetNFilters(void) { return 2; };
    virtual void               VerifyInput(void);

    virtual avtDataObject_p    CreateVariable(avtDataObject_p d) { return d; };
    virtual std::string        GetVarname(std::string &s) { return s; };
};


#endif


