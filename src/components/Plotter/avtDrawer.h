// ************************************************************************* //
//                                avtDrawer.h                                //
// ************************************************************************* //

#ifndef AVT_DRAWER_H
#define AVT_DRAWER_H
#include <plotter_exports.h>


#include <avtDatasetToImageFilter.h>
#include <avtMapper.h>
#include <avtViewInfo.h>

class vtkCamera;


// ****************************************************************************
//  Class: avtDrawer
//
//  Purpose:
//    Creates an avtImage from an avtGeometryDrawable.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 04, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Mon Jan  8 11:48:07 PST 2001
//    Inherit from avtDatasetToImageFilter. 
//
// ****************************************************************************

class PLOTTER_API avtDrawer : public avtDatasetToImageFilter
{
  public:
                          avtDrawer(avtMapper *);
    virtual              ~avtDrawer();

    virtual const char   *GetType(void) { return "avtDrawer"; };

    void                  SetWindowSize(const int size[2]);
    void                  SetWindowSize(const int, const int);

    void                  SetViewInfo(const avtViewInfo &);
    void                  SetViewInfo(vtkCamera *);

  protected:
    virtual avtPipelineSpecification_p
                          PerformRestriction(avtPipelineSpecification_p);
    virtual void          Execute(void);

    avtMapper            *mapper;
    avtViewInfo           viewInfo;
    int                   windowSize[2];
};

#endif


