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
    float                           minDist;

    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);
    bool                            RGridIsect(vtkRectilinearGrid *, 
                                               float &dist, 
                                               float isect[3], 
                                               int ijk[3]);
    int                             LocatorFindCell(vtkDataSet *ds, 
                                                    float &dist, 
                                                    float *isect);
};


#endif


