// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                avtConnComponentsWeightedVariableQuery.h                   //
// ************************************************************************* //

#ifndef AVT_CONN_COMPONENTS_WEIGHTED_VARIABLE_QUERY_H
#define AVT_CONN_COMPONENTS_WEIGHTED_VARIABLE_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>
#include <avtConnComponentsQuery.h>

#include <string>
#include <vector>

class avtEdgeLength;
class avtRevolvedVolume;
class avtVMetricArea;
class avtVMetricVolume;

class vtkDataSet;


// ****************************************************************************
//  Class: avtConnComponentsWeightedVariableQuery
//
//  Purpose:
//      Finds the weighted sum of a specified variable for each connected 
//      component. The sum values are weighted by area, revolved volume, or 
//      volume as appropriate.
//
//  Programmer: Cyrus Harrison
//  Creation:   February 8, 2007
//
//  Modifications:
//    Cyrus Harrison, Mon Jun  6 17:02:12 PDT 2011
//    Added lengthFilter.
//
// ****************************************************************************

class QUERY_API avtConnComponentsWeightedVariableQuery 
: public avtConnComponentsQuery
{
  public:
                                     avtConnComponentsWeightedVariableQuery();
    virtual                         ~avtConnComponentsWeightedVariableQuery();

    virtual const char             *GetType(void)
                          { return "avtConnComponentsWeightedVariableQuery"; };
    virtual const char             *GetDescription(void)
                          { return "Finding per component weighted sum."; };

  protected:

    avtEdgeLength                  *lengthFilter;
    avtRevolvedVolume              *revolvedVolumeFilter;
    avtVMetricArea                 *areaFilter;
    avtVMetricVolume               *volumeFilter;

    std::string                     variableName;
    std::vector<double>             sumPerComp;

    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);
    virtual avtDataObject_p         ApplyFilters(avtDataObject_p);
    virtual void                    VerifyInput(void);


};


#endif



