// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtLocateQuery.h                              //
// ************************************************************************* //

#ifndef AVT_LOCATE_QUERY_H
#define AVT_LOCATE_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>
#include <PickAttributes.h>

class vtkDataSet;
class vtkRectilinearGrid;

// ****************************************************************************
//  Class: avtLocateQuery
//
//  Purpose:
//      This query locates a cell or node and domain given a world-coordinate
//      point or ray.
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 18, 2004
//
//  Modifications:
//    Kathleen Bonnell, Fri Sep  3 10:10:28 PDT 2004
//    Added VerifyInput.
//
//    Kathleen Bonnell, Wed Oct  6 10:48:23 PDT 2004
//    Added RayIntersectsDataSet.
//
//    Kathleen Bonnell, Mon Jun 27 15:46:29 PDT 2005
//    Removed ijk args from RGridIsect, made it return int instead of bool.
//
//    Kathleen Bonnell, Wed Jun 14 16:41:03 PDT 2006
//    Add ApplyFilters.
//
//    Kathleen Biagas, Fri Jul  7 08:54:17 PDT 2017
//    Added ClosestLineToLine.
//
// ****************************************************************************

class QUERY_API avtLocateQuery : public avtDatasetQuery
{
  public:
                                    avtLocateQuery();
    virtual                        ~avtLocateQuery();

    void                            SetPickAtts(const PickAttributes *);
    const PickAttributes           *GetPickAtts(void);


  protected:
    PickAttributes                  pickAtts;
    int                             foundDomain;
    int                             foundElement;
    double                          minDist;

    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);
    virtual void                    VerifyInput(void);
    virtual avtDataObject_p         ApplyFilters(avtDataObject_p);
    bool                            RayIntersectsDataSet(vtkDataSet *);
    int                             RGridIsect(vtkRectilinearGrid *,
                                               double &dist,
                                               double isect[3]);

    int                             LocatorFindCell(vtkDataSet *ds,
                                                    double &dist,
                                                    double *isect);

    int                             ClosestLineToLine(vtkDataSet *ds,
                                                    bool returnNode,
                                                    double &dist,
                                                    double *isect);
};


#endif


