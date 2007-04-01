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

    void                    SetInclusionMode(InclusionMode incMode)
                                { inclusionMode = incMode; };
    void                    SetMins(const float *_mins);
    void                    SetMaxs(const float *_maxs);

    void                    GetMins(float *_mins) const;
    void                    GetMaxs(float *_maxs) const;
    InclusionMode           GetInclusionMode() const { return inclusionMode; };

    bool                    operator==(const avtSpatialBoxSelection &) const;

    void                    GetLogicalBounds(const int nodeCounts[3],
                                             float *nodalCoords[3],
                                             int *firstZone, int *lastZone) const;

    void                    Compose(const avtSpatialBoxSelection& sel,
                                          avtSpatialBoxSelection& composeSel);

  private:

    InclusionMode inclusionMode;
    float mins[3];
    float maxs[3];

};

typedef ref_ptr<avtSpatialBoxSelection> avtSpatialBoxSelectiont_p;


#endif


