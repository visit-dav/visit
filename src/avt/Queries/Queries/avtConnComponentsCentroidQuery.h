// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                    avtConnComponentsCentroidQuery.h                       //
// ************************************************************************* //

#ifndef AVT_CONN_COMPONENTS_CENTROID_QUERY_H
#define AVT_CONN_COMPONENTS_CENTROID_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>
#include <avtConnComponentsQuery.h>

#include <vector>

class vtkDataSet;
class avtConnComponentExpression;

// ****************************************************************************
//  Class: avtConnComponentsCentroidQuery
//
//  Purpose:
//      Finds the centroid of each connected component. 
//
//  Programmer: Cyrus Harrison
//  Creation:   February 5, 2007 
//
// ****************************************************************************

class QUERY_API avtConnComponentsCentroidQuery : public avtConnComponentsQuery
{
  public:
                                    avtConnComponentsCentroidQuery();
    virtual                        ~avtConnComponentsCentroidQuery();

    virtual const char             *GetType(void)
                                 { return "avtConnComponentsCentroidQuery"; };
    virtual const char             *GetDescription(void)
                                   { return "Finding component centroids."; };

  protected:
    std::vector<int>                nCellsPerComp;

    std::vector<double>             xCentroidPerComp;
    std::vector<double>             yCentroidPerComp;
    std::vector<double>             zCentroidPerComp;

    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);

};


#endif


