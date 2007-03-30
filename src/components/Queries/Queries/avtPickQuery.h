// ************************************************************************* //
//                            avtPickQuery.h                                 //
// ************************************************************************* //

#ifndef AVT_PICK_QUERY_H
#define AVT_PICK_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>

#include <string>
#include <PickAttributes.h>

class avtMatrix;
class vtkDataSet;
class vtkRectilinearGrid;


// ****************************************************************************
//  Class: avtPickQuery
//
//  Purpose:
//    This query gathers information about a cell picked by the user. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 15, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Thu Apr 17 09:39:19 PDT 2003  
//    Added member invTransform.
//
//    Kathleen Bonnell, Wed May  7 13:24:37 PDT 2003 
//    Renamed 'LocateCell' as 'LocatorFindCell'. Added 'RGridFindCell'. 
//
// ****************************************************************************

class QUERY_API avtPickQuery : public avtDatasetQuery
{
  public:
                                    avtPickQuery();
    virtual                        ~avtPickQuery() {;};


    virtual const char             *GetType(void)
                                             { return "avtPickQuery"; };
    virtual const char             *GetDescription(void)
                                             { return "Picking"; };

    void                            SetPickAtts(const PickAttributes *pa);
    const PickAttributes *          GetPickAtts(void);

  protected:
    PickAttributes                  pickAtts;
    int                             cellOrigin;
    int                             blockOrigin;
    const avtMatrix                *invTransform;

    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);
    virtual avtDataObject_p         ApplyFilters(avtDataObject_p);   
    int                             LocatorFindCell(vtkDataSet *);
    int                             RGridFindCell(vtkRectilinearGrid *);
};


#endif


