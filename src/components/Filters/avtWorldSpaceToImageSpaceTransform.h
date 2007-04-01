// ************************************************************************* //
//                    avtWorldSpaceToImageSpaceTransform.h                   //
// ************************************************************************* //

#ifndef AVT_WORLD_SPACE_TO_IMAGE_SPACE_TRANSFORM_H
#define AVT_WORLD_SPACE_TO_IMAGE_SPACE_TRANSFORM_H
#include <filters_exports.h>


#include <vector>

#include <avtTransform.h>
#include <avtViewInfo.h>


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
                                      const avtIntervalTree *);

    void               TightenClippingPlanes(bool t)
                              { tightenClippingPlanes = t; };

  protected:
    vtkMatrix4x4           *transform;
    avtViewInfo             view;
    double                  scale[3];
    double                  aspect;
    bool                    tightenClippingPlanes;

    virtual vtkMatrix4x4   *GetTransform(void);

    static void             CalculatePerspectiveTransform(const avtViewInfo &,
                                                          vtkMatrix4x4 *);
    static void             CalculateOrthographicTransform(const avtViewInfo &,
                                                           vtkMatrix4x4 *);
    static void             CalculateTransform(const avtViewInfo &,
                                       vtkMatrix4x4 *, const double *, double);
    virtual avtPipelineSpecification_p
                            PerformRestriction(avtPipelineSpecification_p);

    virtual void            RefashionDataObjectInfo(void);
    virtual void            PreExecute(void);
};


#endif


