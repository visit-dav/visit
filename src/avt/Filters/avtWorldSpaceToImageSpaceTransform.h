// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                    avtWorldSpaceToImageSpaceTransform.h                   //
// ************************************************************************* //

#ifndef AVT_WORLD_SPACE_TO_IMAGE_SPACE_TRANSFORM_H
#define AVT_WORLD_SPACE_TO_IMAGE_SPACE_TRANSFORM_H

#include <filters_exports.h>

#include <avtTransform.h>

#include <avtViewInfo.h>

#include <vector>

class   avtIntervalTree;


// ****************************************************************************
//  Class: avtWorldSpaceToImageSpaceTransform
//
//  Purpose:
//      Transforms an avtDataset by a view matrix.
//
//  Note:     This class should probably redefine the CalcDomainList method and
//            have it cull away unused domains using a spatial extents interval
//            tree.
//
//  Programmer: Hank Childs
//  Creation:   November 27, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Nov 26 18:33:16 PST 2001
//    Add support for aspect ratios.
//
//    Hank Childs, Fri Nov 19 13:38:21 PST 2004
//    Define ExecuteData so we can pass rectilinear grids through if specified.
//
//    Jeremy Meredith, Wed Jan 17 11:39:01 EST 2007
//    Added support for transformed rectilinear grids.
//
//    Hank Childs, Thu May 29 10:23:39 PDT 2008
//    Added argument to method for culling domains for the aspect ratio.
//
//    Hank Childs, Wed Dec 24 14:17:30 PST 2008
//    Make CalculateTransform be public and remove PreExecute, the majority
//    of whose logic went into avtRayTracer.
//
//    Hank Childs, Tue Sep 22 20:40:39 PDT 2009
//    Redefine virtual method in order to disable transformation of vectors.
//
//    Dave Pugmire, Fri May 14 08:04:43 EDT 2010
//    Move vector transform flag into base class.
//
//    Eric Brugger, Tue Jul 22 12:32:20 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class AVTFILTERS_API avtWorldSpaceToImageSpaceTransform : public avtTransform
{
  public:
                       avtWorldSpaceToImageSpaceTransform(const avtViewInfo &,
                                                          double);
                       avtWorldSpaceToImageSpaceTransform(const avtViewInfo &,
                                                          const double *);
    virtual           ~avtWorldSpaceToImageSpaceTransform();

    virtual const char  *GetType(void)
                              { return "avtWorldSpaceToImageSpaceTransform"; };
    virtual const char  *GetDescription(void)
                              { return "Transforming data to image cube"; };

    static void        GetDomainsList(const avtViewInfo &, std::vector<int> &,
                                      const avtIntervalTree *, double aspect);

    void               TightenClippingPlanes(bool t)
                              { tightenClippingPlanes = t; };
    void               SetPassThruRectilinearGrids(bool t)
                              { passThruRectilinear = t; };
    static void        CalculateTransform(const avtViewInfo &,
                                       vtkMatrix4x4 *, const double *, double);

  protected:
    vtkMatrix4x4           *transform;
    avtViewInfo             view;
    double                  scale[3];
    double                  aspect;
    bool                    tightenClippingPlanes;
    bool                    passThruRectilinear;

    virtual vtkMatrix4x4   *GetTransform(void);
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);

    static void             CalculatePerspectiveTransform(const avtViewInfo &,
                                                          vtkMatrix4x4 *);
    static void             CalculateOrthographicTransform(const avtViewInfo &,
                                                           vtkMatrix4x4 *);
    virtual avtContract_p
                            ModifyContract(avtContract_p);

    virtual void            UpdateDataObjectInfo(void);
    virtual bool            FilterUnderstandsTransformedRectMesh();
};


#endif


