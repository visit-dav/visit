// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtCondenseDatasetFilter.h                         //
// ************************************************************************* //

#ifndef AVT_CONDENSE_DATASET_FILTER_H
#define AVT_CONDENSE_DATASET_FILTER_H

#include <filters_exports.h>

#include <avtDataTreeIterator.h>


// ****************************************************************************
//  Class: avtCondenseDatasetFilter
//
//  Purpose:
//      Condenses the size of the dataset by removing irrelevant points and
//      data arrays.
//  
//  Programmer: Kathleen Bonnell 
//  Creation:   November 07, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Apr 10 10:49:10 PDT 2001 
//    Changed ExecuteDomain to ExecuteData.
//
//    Hank Childs, Wed Jun  6 09:09:33 PDT 2001
//    Removed CalcDomainList and Equivalent.
//
//    Hank Childs, Fri Jul 25 21:21:08 PDT 2003
//    Renamed from avtRelevantPointsFilter.
//
//    Kathleen Bonnell, Wed Nov 12 18:26:21 PST 2003 
//    Added a flag that tells this filter to keep avt and vtk data arrays
//    around, and a method for setting the flag. 
//
//    Kathleen Bonnell, Wed Apr 14 17:51:36 PDT 2004 
//    Added a flag that tells this filter to force usage of relevant points 
//    filter (bypassHeuristic).  
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
//    Eric Brugger, Fri Jul 18 16:04:48 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class AVTFILTERS_API avtCondenseDatasetFilter : public avtDataTreeIterator
{
  public:
                         avtCondenseDatasetFilter();
    virtual             ~avtCondenseDatasetFilter();

    virtual const char  *GetType(void) { return "avtCondenseDatasetFilter"; };
    virtual const char  *GetDescription(void) 
                             { return "Removing unneeded points"; };

    virtual void         ReleaseData(void);
    void                 KeepAVTandVTK(bool val) {keepAVTandVTK = val; };
    void                 BypassHeuristic(bool val) {bypassHeuristic = val; };

  protected:
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual bool         FilterUnderstandsTransformedRectMesh();

  private:
    bool                 keepAVTandVTK;
    bool                 bypassHeuristic;
};


#endif


