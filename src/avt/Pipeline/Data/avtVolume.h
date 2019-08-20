// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                 avtVolume.h                               //
// ************************************************************************* //

#ifndef AVT_VOLUME_H
#define AVT_VOLUME_H

#include <pipeline_exports.h>

#include <avtRay.h>

#include <BadIndexException.h>
#include <vector>

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
//    Hank Childs, Sat Dec 11 11:23:42 PST 2004
//    Add ResetSamples.
//
//    Hank Childs, Sun Oct  2 12:06:39 PDT 2005
//    Add image partition argument to GetVariables.
//
//    Hank Childs, Thu May 31 23:07:02 PDT 2007
//    Add argument for number of arrays being passed in.
//
//    Hank Childs, Sun Aug 31 08:04:42 PDT 2008
//    Remove support for calculating gradients on the samples.  That never
//    really worked right.
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
                                        double *);

    char                     *ConstructMessages(avtImagePartition *, char **,
                                                int *);
    void                      EstimateNumberOfSamplesPerScanline(int *);
    void                      ExtractSamples(const char * const *, const int *,
                                             int);

    void                      Restrict(int, int, int, int);
    void                      ResetSamples(void);
    void                      DestroySamples(void);

    void                      SetUseKernel(bool uk) { useKernel = uk; };

    void                      GetVariables(double, vtkDataArray **, int,
                                           avtImagePartition * = NULL);
    void                      SetGradientVariable(int);

    void                      SetProgressCallback(PixelProgressCallback,void*);

  protected:
    struct RayMemoryBlock
    {
        RayMemoryBlock(int ns, int nv);
        ~RayMemoryBlock();
        bool Full() const;
        void GetRayMemory(int ns, int nv, double *&samples, bool *&validSamples);
        int    nBlocks;
        int    nBlocksUsed;
        void  *block; 
    };
    void ReserveRayMemory(double *&samples, bool *&validSamples);
    avtRay *GetUsedRay();

    avtRay                 ***rays;
    std::vector<RayMemoryBlock *> rayMemory;

    avtRay                  **usedRays;
    int                       usedRayIndex;
    int                       usedRaySize;

    int                       volumeWidth;
    int                       volumeHeight;
    int                       volumeDepth;

    int                       numVariables;

    bool                      useKernel;

    int                       restrictedMinWidth;
    int                       restrictedMaxWidth;
    int                       restrictedMinHeight;
    int                       restrictedMaxHeight;

    PixelProgressCallback     progressCallback;
    void                     *progressCallbackArgs;
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
        avtRay *r = GetUsedRay();
        if(r == NULL)
        {
            double *s;
            bool *svalid;
            ReserveRayMemory(s, svalid);

            // Pass the memory to the rays.
            rays[h][w] = new avtRay(s, svalid, volumeDepth, numVariables);
        }
        else
        {
            // Used the reclaimed ray.
            rays[h][w] = r;
        }
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


