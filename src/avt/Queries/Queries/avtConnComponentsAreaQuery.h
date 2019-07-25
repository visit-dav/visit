// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtConnComponentsAreaQuery.h                         //
// ************************************************************************* //

#ifndef AVT_CONN_COMPONENTS_AREA_QUERY_H
#define AVT_CONN_COMPONENTS_AREA_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>
#include <avtConnComponentsQuery.h>

#include <vector>

class avtVMetricArea;
class vtkDataSet;


// ****************************************************************************
//  Class: avtConnComponentsAreaQuery
//
//  Purpose:
//      Obtains the area of each connected component. 
//      Requires a 2D dataset. 
//
//  Programmer: Cyrus Harrison
//  Creation:   February 8, 2007 
//
// ****************************************************************************

class QUERY_API avtConnComponentsAreaQuery : public avtConnComponentsQuery
{
  public:
                                    avtConnComponentsAreaQuery();
    virtual                        ~avtConnComponentsAreaQuery();

    virtual const char             *GetType(void)
                                  { return "avtConnComponentsAreaQuery"; };
    virtual const char             *GetDescription(void)
                                  { return "Finding per component area."; };

  protected:

    avtVMetricArea                 *areaFilter;
    std::vector<double>             areaPerComp;

    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);
    virtual avtDataObject_p         ApplyFilters(avtDataObject_p);
    virtual void                    VerifyInput(void);

};


#endif



