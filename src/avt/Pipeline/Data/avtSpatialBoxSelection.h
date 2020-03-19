// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtSpatialBoxSelection.h                           //
// ************************************************************************* //

#ifndef AVT_SPATIAL_BOX_SELECTION_H
#define AVT_SPATIAL_BOX_SELECTION_H 
#include <pipeline_exports.h>

#include <ref_ptr.h>

#include <avtDataSelection.h>

// ****************************************************************************
//  Class: avtSpatialBoxSelection
//
//  Purpose: Specify a data selection by an axis-aligned bounding box which
//  may be infinitely thin in one or more dimensions. The box is specified in
//  spatial, cartesian coordinates. Note that because it can be infinitely
//  thin in a dimension, we can also use this to specify orthogonal slices.
//
//  The default is a 3D box from FLT_MIN to FLT_MAX on each axis
//
//  Programmer: Mark C. Miller 
//  Creation:   September 26, 2004 
//
//  Modifications:
//
//    Hank Childs, Tue Dec 20 14:43:08 PST 2011
//    Add method DescriptionString.
//
// ****************************************************************************

class PIPELINE_API avtSpatialBoxSelection : public avtDataSelection 
{
  public:

    enum InclusionMode
    {
        Whole,         // whole zone is included only if it is wholly in box 
        Partial,       // whole zone is included if some of it is in box 
        Clip           // part of zone in box after clipping is included 
    };
                            avtSpatialBoxSelection();
    virtual                ~avtSpatialBoxSelection() {} ;

    virtual const char *    GetType() const
                                { return "Spatial Box Data Selection"; }; 
    virtual std::string     DescriptionString(void);

    void                    SetInclusionMode(InclusionMode incMode)
                                { inclusionMode = incMode; };
    void                    SetMins(const double *_mins);
    void                    SetMaxs(const double *_maxs);

    void                    GetMins(double *_mins) const;
    void                    GetMaxs(double *_maxs) const;
    InclusionMode           GetInclusionMode() const { return inclusionMode; };

    bool                    operator==(const avtSpatialBoxSelection &) const;

    void                    GetLogicalBounds(const int nodeCounts[3],
                                             double *nodalCoords[3],
                                             int *firstZone, int *lastZone) const;

    void                    Compose(const avtSpatialBoxSelection& sel,
                                          avtSpatialBoxSelection& composeSel);

  private:

    InclusionMode inclusionMode;
    double mins[3];
    double maxs[3];

};

typedef ref_ptr<avtSpatialBoxSelection> avtSpatialBoxSelectiont_p;


#endif


