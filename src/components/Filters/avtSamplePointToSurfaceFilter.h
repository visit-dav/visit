// ************************************************************************* //
//                        avtSamplePointToSurfaceFilter.h                    //
// ************************************************************************* //

#ifndef AVT_SAMPLE_POINT_TO_SURFACE_FILTER_H
#define AVT_SAMPLE_POINT_TO_SURFACE_FILTER_H

#include <filters_exports.h>

#include <avtSamplePointsToDatasetFilter.h>

class     avtImagePartition;


typedef enum
{
    FRONT_SURFACE =      0,
    MIDDLE_SURFACE,   /* 1 */
    BACK_SURFACE,     /* 2 */
    NOT_SPECIFIED     /* 3 */
} SurfaceType;


// ****************************************************************************
//  Class: avtSamplePointToSurfaceFilter
//
//  Purpose:
//      A filter that will take in sample points and create a surface.  The
//      surface can be the surface at the front of the samples, the back of
//      the samples, or in the middle of the samples.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
// ****************************************************************************

class AVTFILTERS_API avtSamplePointToSurfaceFilter 
    : public avtSamplePointsToDatasetFilter
{
  public:
                              avtSamplePointToSurfaceFilter();
    virtual                  ~avtSamplePointToSurfaceFilter();

    void                      SetSurfaceType(SurfaceType st)
                                    { surfType = st; };
    void                      SetImagePartition(avtImagePartition *ip)
                                    { imagePartition = ip; };
    
    virtual const char       *GetType(void)
                                    { return "avtSamplePointToSurfaceFilter"; };
    virtual const char       *GetDescription(void)
                                    { return "Creating surface from samples";};

  protected:
    SurfaceType               surfType;
    avtImagePartition        *imagePartition;

    virtual void              Execute(void);
};


#endif


