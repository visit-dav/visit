// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtGyRadiusQuery.h                              //
// ************************************************************************* //

#ifndef AVT_GYRADIUS_QUERY_H
#define AVT_GYRADIUS_QUERY_H
#include <query_exports.h>
#include <avtDatasetQuery.h>
#include <avtExpressionEvaluatorFilter.h>

class vtkDataArray;
class vtkDataSet;
class avtCentroidQuery;

// ****************************************************************************
//  Class: avtGyRadiusQuery
//
//  Purpose:
//    A query that performs the following calculation: 
//    R = sqrt(sum( (Mi*(Ri - Rcm)^2) ) / sum(Mi)), where Mi is the mass of cell
//    i and (Ri - Rcm) is the distance of cell i from the center of mass.
//
//  Programmer: Kevin Griffin
//  Creation:   August 9, 2016
//
//  Modifications:
//
// ****************************************************************************

class QUERY_API avtGyRadiusQuery : virtual public avtDatasetQuery
{
public:
                            avtGyRadiusQuery();
    virtual                ~avtGyRadiusQuery();
    
    virtual const char     *GetType(void) { return "avtGyRadiusQuery"; };
    virtual const char     *GetDescription(void) { return "GyRadius."; };
    
    virtual void            SetInputParams(const MapNode &);
    static  void            GetDefaultInputParams(MapNode &);
    
protected:
    virtual void            PreExecute(void);
    virtual void            Execute(vtkDataSet *, const int);
    virtual void            PostExecute(void);
    virtual avtDataObject_p ApplyFilters(avtDataObject_p);
    
private:
    doubleVector    centroid;
    bool            overrideCentroid;
    double          totalSum;
    double          totalMass;
    bool            isZonal;
    std::string     varName;
    
    avtExpressionEvaluatorFilter *eef;
};

#endif
