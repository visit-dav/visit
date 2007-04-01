// ************************************************************************* //
//                           avtLocateCellQuery.h                            //
// ************************************************************************* //

#ifndef AVT_LOCATE_CELL_QUERY_H
#define AVT_LOCATE_CELL_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>
#include <PickAttributes.h>

#include <string>

class vtkDataSet;

// ****************************************************************************
//  Class: avtLocateCellQuery
//
//  Purpose:
//      This query locates a cell and domain given a world-coordinate point. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 15, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jan 31 11:34:03 PST 2003 
//    Added data member 'minDist'.
//
//    Kathleen Bonnell, Mon Apr 14 09:43:11 PDT 2003
//    Added member invTransform.
//
//    Kathleen Bonnell, Thu Apr 17 09:39:19 PDT 2003   
//    Removed member invTransform.
//
//    Kathleen Bonnell, Wed May  7 13:24:37 PDT 2003 
//    Add methods 'RGridFindCell' and 'LocatorFindCell'.
//
//    Kathleen Bonnell, Tue Jun  3 15:20:35 PDT 2003 
//    Removed 'tolerance' parameter from FindCell methods.
//
//    Kathleen Bonnell, Fri Oct 10 11:45:24 PDT 2003
//    Added DeterminePickedNode. 
//
//    Kathleen Bonnell, Tue Nov  4 08:18:54 PST 2003 
//    Added  SetPickAtts, GetPickAtts, and PickAttributes data member.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
// ****************************************************************************

class QUERY_API avtLocateCellQuery : public avtDatasetQuery
{
  public:
                                    avtLocateCellQuery();
    virtual                        ~avtLocateCellQuery();

    virtual const char             *GetType(void)
                                             { return "avtLocateCellQuery"; };
    virtual const char             *GetDescription(void)
                                             { return "Locating cell."; };

    void                            SetPickAtts(const PickAttributes *);
    const PickAttributes           *GetPickAtts(void);

  protected:
    PickAttributes                  pickAtts;
    int                             foundDomain;
    int                             foundZone;
    float                           minDist;

    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);
    int                             LocatorFindCell(vtkDataSet *, 
                                                    float &, float*);
    int                             RGridFindCell(vtkDataSet *, 
                                                    float &, float*);

    void                            DeterminePickedNode(vtkDataSet *, 
                                                    int, float*);
};


#endif


