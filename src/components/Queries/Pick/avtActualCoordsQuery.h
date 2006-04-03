// ************************************************************************* //
//                           avtActualCoordsQuery.h                          //
// ************************************************************************* //

#ifndef AVT_ACTUAL_COORDS_QUERY_H
#define AVT_ACTUAL_COORDS_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>
#include <PickAttributes.h>



// ****************************************************************************
//  Class: avtActualCoordsQuery
//
//  Purpose:
//    This is an abstract class for queries that determine the actual 
//    coordinates of a zone or node.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 18, 2004 
//
//  Modifications:
//
// ****************************************************************************

class QUERY_API avtActualCoordsQuery : public avtDatasetQuery
{
  public:
                             avtActualCoordsQuery();
    virtual                 ~avtActualCoordsQuery();

    void                     SetPickAtts(const PickAttributes *);
    const PickAttributes    *GetPickAtts(void);



  protected:
    PickAttributes           pickAtts; 
    int                      actualId;
    double                   actualCoords[3];
    virtual void             PreExecute(void);
    virtual void             PostExecute(void);
};


#endif


