// ************************************************************************* //
//                                 avtVolume.h                               //
// ************************************************************************* //

#ifndef AVT_VOLUME_H
#define AVT_VOLUME_H
#include <pipeline_exports.h>


#include <avtRay.h>

#include <BadIndexException.h>


class   avtGradients;
class   avtImagePartition;
class   avtRayFunction;
class   vtkDataArray;

typedef void (*PixelProgressCallback)(void *, int, int);


// ****************************************************************************
//  Class: avtVolume
//
//  Purpose:
//      This is a virtual volume.  Its size is preset.  To save on space (as
//      much of the volume will never be used), a volume contains a bunch of
//      scanlines which are allocated only when needed.  Each of those
//      scanlines contains a bunch of rays which are also allocated only when
//      needed.
//
//  Programmer: Hank Childs
//  Creation:   November 29, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Mar 26 11:02:49 PST 2001
//    Allow for variable to be directly fetched from volume.
//
//    Hank Childs, Wed Nov 14 11:41:50 PST 2001
//    Add support for multiple variables.
//
//    Hank Childs, Tue Jan  1 13:15:09 PST 2002
//    Add estimates of number of samples per scanline to help the image
//    partition to create non-uniform partitions with approximately equal
//    sized partitions with respect to number of samples.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
// ****************************************************************************

class PIPELINE_API avtVolume
{
  public:
                              avtVolume(int, int, int, int);
    virtual                  ~avtVolume();

    inline avtRay            *GetRay(const int &, const int &);
    inline avtRay            *QueryGetRay(const int &, const int &);
    int                       GetVolumeHeight(void) const
                                        { return volumeHeight; };
    int                       GetVolumeWidth(void) const
                                        { return volumeWidth; };
    int                       GetVolumeDepth(void) const
                                        { return volumeDepth; };
    int                       GetRestrictedVolumeHeight(void) const
                                        { return restrictedMaxHeight
                                                 - restrictedMinHeight + 1; };
    int                       GetRestrictedVolumeWidth(void) const
                                        { return restrictedMaxWidth
                                                 - restrictedMinWidth + 1; };
    int                       GetRestrictedMinWidth(void) const
                                        { return restrictedMinWidth; };
    int                       GetRestrictedMinHeight(void) const
                                        { return restrictedMinHeight; };

    void                      GetPixels(avtRayFunction *, unsigned char *,
                                        float *);

    char                     *ConstructMessages(avtImagePartition *, char **,
                                                int *);
    void                      EstimateNumberOfSamplesPerScanline(int *);
    void                      ExtractSamples(const char * const *, const int *,
                                             int);

    void                      Restrict(int, int, int, int);

    void                      GetVariables(float, vtkDataArray **);
    void                      SetGradientVariable(int);

    void                      SetProgressCallback(PixelProgressCallback,void*);

  protected:
    avtRay                 ***rays;
    avtGradients             *gradients;
    
    int                       volumeWidth;
    int                       volumeHeight;
    int                       volumeDepth;
    int                       gradientSpread;

    int                       numVariables;
    int                       gradientVariable;

    int                       restrictedMinWidth;
    int                       restrictedMaxWidth;
    int                       restrictedMinHeight;
    int                       restrictedMaxHeight;

    PixelProgressCallback     progressCallback;
    void                     *progressCallbackArgs;

    void                      GetGradients(avtGradients *, int, int,
                                           avtRayFunction *);
    void                      GetSobelGradients(avtGradients *, int, int,
                                                avtRayFunction *);
    void                      GetCentralDifferencingGradients(avtGradients *,
                                                   int, int, avtRayFunction *);
    void                      ContributeRay(avtRay *, avtGradients *,
                                            int [9], avtRayFunction *);
};


// ****************************************************************************
//  Method: avtVolume::GetRay
//
//  Purpose:
//      Gets the ray specified by the arguments.
//
//  Arguments:
//      w       The x-index of the ray (width).
//      h       The y-index of the ray (height).
//
//  Returns:    The ray for (w, h). (Never NULL)
//
//  Programmer: Hank Childs
//  Creation:   November 29, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Feb  3 18:45:55 PST 2001
//    Changed routine to return a ray instead of setting sample points.
//
//    Hank Childs, Wed Nov 14 11:41:50 PST 2001
//    Add support for multiple variables.
//
// ****************************************************************************

inline avtRay *
avtVolume::GetRay(const int &w, const int &h)
{
    if (h < 0 || h >= volumeHeight)
    {
        EXCEPTION2(BadIndexException, h, volumeHeight);
    }
    if (w < 0 || w >= volumeWidth)
    {
        EXCEPTION2(BadIndexException, w, volumeWidth);
    }

    if (rays[h] == NULL)
    {
        rays[h] = new avtRay*[volumeWidth];
        for (int i = 0 ; i < volumeWidth ; i++)
        {
            rays[h][i] = NULL;
        }
    }

    if (rays[h][w] == NULL)
    {
        rays[h][w] = new avtRay(volumeDepth, numVariables);
    }

    return rays[h][w];
}


// ****************************************************************************
//  Method: avtVolume::QueryGetRay
//
//  Purpose:
//      Gets the ray specified by the arguments -- provided it exists.
//
//  Arguments:
//      w       The x-index of the ray (width).
//      h       The y-index of the ray (height).
//
//  Returns:    The ray for (w, h).  May be NULL.
//
//  Programmer: Hank Childs
//  Creation:   February 11, 2002
//
// ****************************************************************************

inline avtRay *
avtVolume::QueryGetRay(const int &w, const int &h)
{
    if (h < 0 || h >= volumeHeight)
    {
        return NULL;
    }
    if (w < 0 || w >= volumeWidth)
    {
        return NULL;
    }

    if (rays[h] == NULL)
    {
        return NULL;
    }

    return rays[h][w];
}


#endif


