// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtGhostZoneFilter.h                           //
// ************************************************************************* //

#ifndef AVT_GHOST_ZONE_FILTER_H
#define AVT_GHOST_ZONE_FILTER_H

#include <filters_exports.h>

#include <avtDataTreeIterator.h>


// ****************************************************************************
//  Class: avtGhostZoneFilter
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 1, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Sep 19 12:55:57 PDT 2001
//    Added string argument to Execute method in order to match new interface.
//
//    Hank Childs, Tue Sep 10 12:51:33 PDT 2002
//    Inherited from avtDataTreeIterator instead of avtSIMODataTreeIterator.  Re-worked
//    the paradigm for memory management.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Wed Dec 20 09:25:42 PST 2006
//    Add "ghost data must be removed".
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
//    Hank Childs, Fri Aug  3 13:27:27 PDT 2007
//    Add support for removing only ghost data of a certain type.
//
//    Hank Childs, Sun Oct 28 10:48:50 PST 2007
//    Add a mode for removing only ghost zones of a certain type.
//
//    Eric Brugger, Mon Jul 21 13:22:09 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
//    Kevin Griffin, Thu Jul 30 08:43:33 PDT 2020
//    Added ability to force confirmation of region for Structured grids.
//
// ****************************************************************************

class AVTFILTERS_API avtGhostZoneFilter : public avtDataTreeIterator
{
  public:
                         avtGhostZoneFilter();
    virtual             ~avtGhostZoneFilter();

    virtual const char  *GetType(void) { return "avtGhostZoneFilter"; };
    virtual const char  *GetDescription(void) 
                             { return "Removing ghost cells"; };

    // There are some cases where the ghost zone filter will allow
    // ghost data to pass through and allow the renderer to remove the
    // ghost data on the fly.  This disallows that behavior.
    void                 GhostDataMustBeRemoved()
                             { ghostDataMustBeRemoved = true; };

    // If you call this method, polygons with the specified ghost node types 
    // will be removed.  The ghost node information will then be removed,
    // meaning that other ghost nodes types will no longer be treated as 
    // ghost.
    void                 SetGhostNodeTypesToRemove(unsigned char);

    // If you call this method, only zones with the specified ghost zones types 
    // will be removed.  Ghost zones information will not be removed,
    // meaning that future ghost zone removals can occur.
    void                 SetGhostZoneTypesToRemove(unsigned char);
    
    // If you call this method, this will force checking (if true) to make sure
    // that zones planned to be removed are uniformly of the type to remove for
    // Structured grids.
    void                 SetConfirmRegion(bool);

  protected:
    bool                        ghostDataMustBeRemoved;
    unsigned char               ghostNodeTypesToRemove; // remove all types if must be removed
    unsigned char               ghostZoneTypesToRemove; // remove all types if must be removed
    bool                        confirmRegion;

    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void                UpdateDataObjectInfo(void);
    virtual bool                FilterUnderstandsTransformedRectMesh();
};


#endif


