// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtGhostZoneAndFacelistFilter.h                    //
// ************************************************************************* //

#ifndef AVT_GHOSTZONE_AND_FACELIST_FILTER_H
#define AVT_GHOSTZONE_AND_FACELIST_FILTER_H

#include <filters_exports.h>

#include <avtDatasetToDatasetFilter.h>

class   avtGhostZoneFilter;
class   avtFacelistFilter;


// ****************************************************************************
//  Class: avtGhostZoneAndFacelistFilter
//
//  Purpose:
//    Applies the ghostZone and facelist filters, after
//    determining which should come first. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 20, 2001 
//
//  Modifications:
//
//    Hank Childs, Fri Jul 27 14:54:13 PDT 2001
//    Added UpdateDataObjectInfo.
//
//    Hank Childs, Thu Sep  6 11:14:38 PDT 2001
//    Allowed for dynamic load balancing.
//
//    Hank Childs, Wed Aug 11 09:46:53 PDT 2004
//    Added ModifyContract.
//
//    Hank Childs, Fri Sep 23 10:38:31 PDT 2005
//    Add support for edge lists.
//
//    Hank Childs, Wed Dec 20 09:34:20 PST 2006
//    Add support for new methods with facelist filter and ghost zone filter
//    to ensure that ghosts are removed and polydata is produced.
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
//    Hank Childs, Fri Aug  3 16:53:56 PDT 2007
//    Added method SetGhostNodeTypesToRemove.
//
//    Hank Childs, Sun Oct 28 10:35:05 PST 2007
//    Added second ghost zone filter for exterior boundary ghosts.  In this 
//    case, we should remove the ghosts along the boundary, then find the
//    external faces, then remove the faces that are interior and duplicated.
//
//    Jeremy Meredith, Tue Oct 14 15:42:56 EDT 2008
//    Changed interface to SetMustCreatePolyData to allow either t/f setting.
//
//    Hank Childs, Wed Dec 22 01:27:33 PST 2010
//    Implement virtual method to tell base class that we don't want to 
//    change the active variable.
//
// ****************************************************************************

class AVTFILTERS_API avtGhostZoneAndFacelistFilter :
                                               public avtDatasetToDatasetFilter
{
  public:
                          avtGhostZoneAndFacelistFilter();
    virtual              ~avtGhostZoneAndFacelistFilter(); 

    virtual const char   *GetType(void)
                                   { return "avtGhostZoneAndFacelistFilter"; };
    virtual const char   *GetDescription(void)
                                   { return "Finding visible triangles"; };

    void                  SetUseFaceFilter(bool val) { useFaceFilter = val; };
    void                  SetCreate3DCellNumbers(bool val);
    void                  SetForceFaceConsolidation(bool);
    void                  SetCreateEdgeListFor2DDatasets(bool val);
    void                  SetGhostNodeTypesToRemove(unsigned char val);
    void                  GhostDataMustBeRemoved();
    void                  SetMustCreatePolyData(bool val=true);

    virtual void          ReleaseData(void);
    virtual bool          AutomaticallyMakePipelineVariableActive(void)
                                  { return false; };

  protected:
    avtGhostZoneFilter   *ghostFilter;
    avtGhostZoneFilter   *exteriorBoundaryGhostFilter;
    avtFacelistFilter    *faceFilter;
    bool                  useFaceFilter;
    bool                  useGhostFilter;

    virtual void          Execute(void);
    virtual void          UpdateDataObjectInfo(void);
    virtual int           AdditionalPipelineFilters(void);
    virtual void          ChangedInput(void);
    virtual avtContract_p
                          ModifyContract(avtContract_p);
    virtual bool          FilterUnderstandsTransformedRectMesh();
};


#endif


