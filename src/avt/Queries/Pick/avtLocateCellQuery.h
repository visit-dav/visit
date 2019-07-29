// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtLocateCellQuery.h                            //
// ************************************************************************* //

#ifndef AVT_LOCATE_CELL_QUERY_H
#define AVT_LOCATE_CELL_QUERY_H
#include <query_exports.h>

#include <avtLocateQuery.h>

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
//    Kathleen Bonnell, Tue May 18 13:12:09 PDT 2004
//    Inherit from avtLocateQuery.  Moved Node specific code to
//    avtLocateNodeQuery.
//
//    Kathleen Bonnell, Wed Jul  7 14:48:44 PDT 2004
//    Added FindClosestCell, for use with line plots.
//
// ****************************************************************************

class QUERY_API avtLocateCellQuery : public avtLocateQuery
{
  public:
                                    avtLocateCellQuery();
    virtual                        ~avtLocateCellQuery();

    virtual const char             *GetType(void)
                                             { return "avtLocateCellQuery"; };
    virtual const char             *GetDescription(void)
                                             { return "Locating cell."; };

  protected:
    virtual void                    Execute(vtkDataSet *, const int);
    int                             RGridFindCell(vtkDataSet *,
                                                    double &, double*);
    int                             FindClosestCell(vtkDataSet *ds,
                                                    double &dist,
                                                    double isect[3]);
};


#endif


