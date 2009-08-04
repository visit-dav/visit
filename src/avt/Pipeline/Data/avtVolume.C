/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                                 avtVolume.C                               //
// ************************************************************************* //

#include <avtVolume.h>

#include <vtkDataArray.h>

#include <avtImagePartition.h>
#include <avtRay.h>
#include <avtRayFunction.h>

#include <ImproperUseException.h>
#include <Utility.h>


// ****************************************************************************
//  Method: avtVolume constructor
//
//  Arguments:
//      sw       The volume width.
//      sh       The volume height.
//      sd       The volume depth.
//      nv       The number of variables.
//
//  Note:   This would be the spot to check the input and modify the volume
//          depth if the number of samples is too large and memory will be
//          overrun.
//
//  Programmer: Hank Childs
//  Creation:   November 29, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Nov 14 11:41:50 PST 2001
//    Add support for multiple variables.
//
//    Hank Childs, Tue Feb  5 09:17:12 PST 2002
//    Be more forgiving of volumes with zero variables.
//
//    Hank Childs, Tue Feb 28 08:14:32 PST 2006
//    Initialize useKernel.
//
//    Hank Childs, Sun Aug 31 08:04:42 PDT 2008
//    Remove support for gradients.
//
// ****************************************************************************

avtVolume::avtVolume(int sw, int sh, int sd, int nv)
{
    volumeWidth  = sw;
    volumeHeight = sh;
    volumeDepth  = sd;

    numVariables = nv;
    if (numVariables < 0)
    {
        EXCEPTION0(ImproperUseException);
    }

    restrictedMinWidth  = 0;
    restrictedMaxWidth  = sw-1;
    restrictedMinHeight = 0;
    restrictedMaxHeight = sh-1;

    rays = new avtRay**[volumeHeight];
    for (int i = 0 ; i < volumeHeight ; i++)
    {
        rays[i] = NULL;
    }

    progressCallback     = NULL;
    progressCallbackArgs = NULL;

    useKernel = false;
}


// ****************************************************************************
//  Method: avtVolume destructor
//
//  Programmer: Hank Childs
//  Creation:   November 29, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Dec 11 11:22:33 PST 2004
//    Offload some of the destruction work to ResetSamples.
//
//    Hank Childs, Sun Aug 31 08:04:42 PDT 2008
//    Remove support for gradients.
//
// ****************************************************************************

avtVolume::~avtVolume()
{
    ResetSamples();
    delete [] rays;
}


// ****************************************************************************
//  Method: avtVolume::ResetSamples
//
//  Purpose:
//      Clears out all of the samples in this object.
//
//  Programmer: Hank Childs
//  Creation:   December 10, 2004
//
// ****************************************************************************

void
avtVolume::ResetSamples(void)
{
    if (rays != NULL)
    {
        for (int i = 0 ; i < volumeHeight ; i++)
        {
            if (rays[i] != NULL)
            {
                for (int j = 0 ; j < volumeWidth ; j++)
                {
                    if (rays[i][j] != NULL)
                    {
                        delete rays[i][j];
                        rays[i][j] = NULL;
                    }
                }
                delete [] rays[i];
                rays[i] = NULL;
            }
        }
    }
}


// ****************************************************************************
//  Method: avtVolume::Restrict
//
//  Purpose:
//      Restricts the area that the volume will operate on to a portion of
//      the original volume.  This is only used for parallel settings.  This
//      is done because some cells may have some sample points on this
//      processor and _many_ that are not and restricting the area of
//      interest allows us to play some games with the frustum to make sure
//      they don't get calculated.
//
//  Arguments:
//      minw    The new minimum width.
//      maxw    The new maximum width.
//      minh    The new minimum height.
//      maxh    The new maximum height.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2001
//
// ****************************************************************************

void
avtVolume::Restrict(int minw, int maxw, int minh, int maxh)
{
    restrictedMinWidth  = minw;
    restrictedMaxWidth  = maxw;
    restrictedMinHeight = minh;
    restrictedMaxHeight = maxh;
}


// ****************************************************************************
//  Method: avtVolume::GetPixels
//
//  Purpose:
//      Gets the values for each pixel.
//
//  Arguments:
//      rayfoo   The ray function.
//      data     An input image.  This might be just the background or an
//               opaque image.
//      zbuffer  The zbuffer of the input.
//
//  Programmer:  Hank Childs
//  Creation:    November 29, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Feb  3 20:30:59 PST 2001
//    Put pixels straight into array instead of some intermediate structure.
//
//    Hank Childs, Tue Feb 13 16:25:32 PST 2001
//    Modified to work for opaque images as well.
//
//    Hank Childs, Wed Nov 21 17:36:19 PST 2001
//    Pack pixels right-to-left so they look the same as VTK.  This may reflect
//    a bug with how the sampling is done somewhere else.
//
//    Hank Childs, Tue Nov 27 10:58:09 PST 2001
//    Added progress callbacks.
//
//    Hank Childs, Sun Jan  1 17:46:21 PST 2006
//    Added call to avtRay::Finalize for kernel based sampling.
//
//    Hank Childs, Tue Sep 12 08:57:39 PDT 2006
//    Added support for ray functions that need pixel indices.
//
//    Hank Childs, Sun Aug 31 08:04:42 PDT 2008
//    Remove support for gradients.
//
// ****************************************************************************

void
avtVolume::GetPixels(avtRayFunction *rayfoo,unsigned char *data,float *zbuffer)
{
    int fullwidth  = (restrictedMaxWidth-restrictedMinWidth+1);
    int fullheight = (restrictedMaxHeight-restrictedMinHeight+1);
    int numRays = fullwidth*fullheight;
    int currentTenth = 0;
    bool needPixelIndices = rayfoo->NeedPixelIndices();

    for (int i = restrictedMinHeight ; i <= restrictedMaxHeight ; i++)
    {
        if (rays[i] != NULL)
        {
            for (int j = restrictedMinWidth ; j <= restrictedMaxWidth ; j++)
            {
                if (rays[i][j] != NULL)
                {
                    //
                    // Put the background color into an rgb block so that the
                    // ray function can use it as the background.  The z-buffer
                    // value will also be sent along in case there is an opaque
                    // image in the input.
                    //
                    int w = j-restrictedMinWidth;
                    int h = i-restrictedMinHeight;
                    int index = h*fullwidth + w;
                    unsigned char rgb[3];
                    rgb[0] = data[3*index];
                    rgb[1] = data[3*index+1];
                    rgb[2] = data[3*index+2];

                    //
                    // Get the value of the ray.
                    //
                    rays[i][j]->Finalize();
                    if (needPixelIndices)
                        rayfoo->SetPixelIndex(j, i);
                    rayfoo->GetRayValue(rays[i][j], rgb, zbuffer[index]);

                    //
                    // Copy the pixel onto the image.
                    //
                    data[3*index    ] = rgb[0];
                    data[3*index + 1] = rgb[1];
                    data[3*index + 2] = rgb[2];
                }
                if (progressCallback != NULL)
                {
                    int w = j-restrictedMinWidth;
                    int h = i-restrictedMinHeight;
                    int index = h*fullwidth + w;
                    if ((10*index)/numRays > currentTenth)
                    {
                        currentTenth = (10*index)/numRays;
                        progressCallback(progressCallbackArgs,currentTenth,10);
                    }
                }
            }
        }
    }
}


// ****************************************************************************
//  Method: avtVolume::ConstructMessages
//
//  Purpose:
//      Goes through the rays and constructs the messages for each one.
//
//  Arguments:
//      part    The image partition that determines to which each partition
//                  belongs.
//      msgs    A place to store the messages.
//      cnt     A place to store the size of each message.
//
//  Returns:    The character string that all of the messages strings sit on
//              top of.
//      
//  Programmer: Hank Childs
//  Creation:   February 4, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Nov 14 11:41:50 PST 2001
//    Add support for multiple variables.
//
//    Hank Childs, Mon Dec 31 13:37:50 PST 2001
//    Add support for more efficient byte-packing.
//
//    Hank Childs, Sun Oct  2 12:06:39 PDT 2005
//    Add support for distributed resampling/duplicated points in an image
//    partition.
//
// ****************************************************************************

char *
avtVolume::ConstructMessages(avtImagePartition *part, char **msgs, int *cnt)
{
    int i, j, k, l;
    int numparts = part->GetNumPartitions();

    //
    // Count how big each message will be.
    //
    for (i = 0 ; i < numparts ; i++)
    {
        cnt[i] = 0;
    }
    for (i = 0 ; i < volumeHeight ; i++)
    {
        if (rays[i] != NULL)
        {
            for (j = 0 ; j < volumeWidth ; j++)
            {
                if (rays[i][j] != NULL)
                {
                    bool duplicatedOnOthers;
                    int index = part->Partition(j, i, duplicatedOnOthers);

                    //
                    // Since there are often many consecutive sample points,
                    // it is efficient to store the ray as a series of "runs",
                    // where a run has the first sample point in the run and
                    // how many consecutive sample points are contained within
                    // the run.
                    //
                    // Then we need:
                    //    Two ints for the ray index.
                    //    One int for the number of runs.
                    //    Two ints for each run (starting index and number of
                    //    samples in the run).
                    //    A float for each sample point and for each variable
                    //    on that sample point.
                    //
                    cnt[index] += 3*sizeof(int);
                    cnt[index] += 2*rays[i][j]->numRuns*sizeof(int);
                    cnt[index] += rays[i][j]->numValidSamples * numVariables *
                                  sizeof(float);

                    if (duplicatedOnOthers)
                    {
                        std::vector<int> otherParts = 
                                         part->GetPartitionsAsADuplicate(j, i);
                        for (size_t p = 0 ; p < otherParts.size() ; p++)
                        {
                            int index = otherParts[p];
                            cnt[index] += 3*sizeof(int);
                            cnt[index] += 2*rays[i][j]->numRuns*sizeof(int);
                            cnt[index] += rays[i][j]->numValidSamples 
                                          * numVariables * sizeof(float);
                        }
                    }
                }
            }
        }
    }

    //
    // Use a convenience routine to construct the underlying character string
    // that all of the messages will sit on top of.
    //
    char *concat = CreateMessageStrings(msgs, cnt, numparts);

    //
    // Now serialize the sample points.
    //
    char **current = new char*[numparts];
    for (i = 0 ; i < numparts ; i++)
    {
        current[i] = msgs[i];
    }

    for (i = 0 ; i < volumeHeight ; i++)
    {
        if (rays[i] != NULL)
        {
            for (j = 0 ; j < volumeWidth ; j++)
            {
                if (rays[i][j] != NULL)
                {
                    bool duplicatedOnOthers;
                    int index = part->Partition(j, i, duplicatedOnOthers);
                    std::vector<int> list;
                    if (duplicatedOnOthers)
                    {
                        list = part->GetPartitionsAsADuplicate(j, i);
                    }
                    list.push_back(index);
                    
                    int nSamps = list.size();
                    for (int p = 0 ; p < nSamps ; p++)
                    {
                        index = list[p];
                        int width  = j;
                        int height = i;
                        InlineCopy(current[index], (char *)&width, sizeof(int));
                        InlineCopy(current[index], (char *)&height, sizeof(int));
                        int numRuns = rays[i][j]->numRuns;
                        InlineCopy(current[index], (char *)&numRuns, sizeof(int));
    
                        //
                        // -Optimization- Make a local copy of the rays' arrays,
                        // so we don't have to look up their values repeatedly.
                        //
                        int      numSamples  = rays[i][j]->numSamples;
                        bool    *validSample = rays[i][j]->validSample;
                        float  **sample      = rays[i][j]->sample;
    
                        bool inRun = false;
                        for (k = 0 ; k < numSamples ; k++)
                        {
                            if (validSample[k])
                            {
                                if (!inRun)
                                {
                                    //
                                    // Determine how many samples are in this run.
                                    //
                                    int ind = k;
                                    while (ind < numSamples && validSample[ind])
                                    {
                                        ind++;
                                    }
                                    int numSamplesInRun = ind-k;
    
                                    //
                                    // Now copy over the start of the run and how
                                    // many samples it has.
                                    //
                                    InlineCopy(current[index], (char *)&k,
                                               sizeof(int));
                                    InlineCopy(current[index],
                                            (char *)&numSamplesInRun, sizeof(int));
                                }
                                for (l = 0 ; l < numVariables ; l++)
                                {
                                    InlineCopy(current[index],
                                           (char *)&(sample[l][k]), sizeof(float));
                                }
                            }
                            inRun = validSample[k];
                        }
                    }
                }
            }
        }
    }

    delete [] current;

    return concat;
}


// ****************************************************************************
//  Method: avtVolume::EstimateNumberOfSamplesPerScanline
//
//  Purpose:
//      Estimates the number of samples in each scanline.  This isn't really
//      an estimate since we know that number exactly, but it is an estimate
//      for cell lists (so it is named estimate for consistency).
//
//  Arguments:
//      samples    A running list of how many samples points there are per
//                 scanline.
//
//  Programmer:    Hank Childs
//  Creation:      January 1, 2002
//
// ****************************************************************************

void
avtVolume::EstimateNumberOfSamplesPerScanline(int *samples)
{
    for (int i = 0 ; i < volumeHeight ; i++)
    {
        if (rays[i] != NULL)
        {
            for (int j = 0 ; j < volumeWidth ; j++)
            {
                if (rays[i][j] != NULL)
                {
                    samples[i] += rays[i][j]->numValidSamples;
                }
            }
        }
    }
}


// ****************************************************************************
//  Method: avtVolume::ExtractSamples
//
//  Purpose:
//      Extracts the sample points from the input strings.
//
//  Arguments:
//      msgs    The messages from other processors.
//      cnt     The length of each message.
//      nmsgs   The number of messages in msgs.
//
//  Programmer: Hank Childs
//  Creation:   February 4, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Nov 14 11:41:50 PST 2001
//    Add support for multiple variables.
//
//    Hank Childs, Mon Dec 31 13:37:50 PST 2001
//    Add support for more efficient byte-packing.
//
// ****************************************************************************

void
avtVolume::ExtractSamples(const char * const *msgs, const int *cnt, int nmsgs)
{
    for (int i = 0 ; i < nmsgs ; i++)
    {
        const char *current = msgs[i];
        while (current < (msgs[i] + cnt[i]) )
        {
            //
            // It will first have the ray index (w, h).
            //
            int  rayW, rayH;
            InlineExtract((char *)&rayW, current, sizeof(int));
            InlineExtract((char *)&rayH, current, sizeof(int));
            avtRay *ray = GetRay(rayW, rayH);

            //
            // Get the number of runs.
            //
            int  numRuns;
            InlineExtract((char *)&numRuns, current, sizeof(int));

            //
            // Process each run one at a time.
            //
            for (int j = 0 ; j < numRuns ; j++)
            {
                 int    index;
                 InlineExtract((char *)&index, current, sizeof(int));

                 int    numSamples;
                 InlineExtract((char *)&numSamples, current, sizeof(int));

                 for (int k = 0 ; k < numSamples ; k++)
                 {
                     float value[AVT_VARIABLE_LIMIT];
                     for (int l = 0 ; l < numVariables ; l++)
                     {
                         InlineExtract((char *)&(value[l]), current,
                                       sizeof(float));
                     }
                     ray->SetSample(index+k, value);
                 }
            }
        }
    }
}


// ****************************************************************************
//  Method: avtVolume::GetVariables
//
//  Purpose:
//      Gets all of the values in a volume and puts them in a vtkScalars.
//
//  Arguments:
//      defaultVal   The default value.
//      arrays       The arrays to be populated.  They should already have
//                   been instantiated, but do not need to have been set up
//                   (-> you call New, but this will call Set#OfTuples)
//      nArrays      The number of arrays passed in.
//
//  Notes:      This is only a reasonable thing to do for small volumes.
//
//  Programmer: Hank Childs
//  Creation:   March 26, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Nov 14 11:41:50 PST 2001
//    Add support for multiple variables.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Fri Sep 30 15:03:28 PDT 2005
//    Add support for get the variables only from our portion of the partitions
//    subvolume.
//
//    Hank Childs, Tue Feb 28 08:14:32 PST 2006
//    Initialize differently if we are using the kernel method.
//
//    Hank Childs, Thu May 31 23:07:02 PDT 2007
//    Add support for non-scalars.
//
// ****************************************************************************

void
avtVolume::GetVariables(float defaultVal, vtkDataArray **arrays, int nArrays,
                        avtImagePartition *ip)
{
    int   i, j, k, l, m;

    //
    // Create the scalars and tell it how many elements we want.
    //
    for (i = 0 ; i < nArrays ; i++)
    {
        arrays[i]->SetNumberOfTuples(volumeWidth*volumeHeight*volumeDepth);
    }

    //
    // Set up the indices we are going to iterate over.  This is normally
    // going to be the whole volume.  But if we have partitioned the volume
    // across the processors, it may not be.  This happens when the resample
    // filter does a distributed resampling.
    //
    int wStart = 0;
    int wEnd   = volumeWidth;
    int hStart = 0;
    int hEnd   = volumeHeight;
    if (ip != NULL)
    {
        ip->GetThisPartition(wStart, wEnd, hStart, hEnd);
        wEnd++;
        hEnd++;
    }
    if (hEnd > volumeHeight)
        return;

    int *arraySize = new int[nArrays];
    for (i = 0 ; i < nArrays ; i++)
        arraySize[i] = arrays[i]->GetNumberOfComponents();

    //
    // Put the default value into the variable, so if we don't have samples
    // we don't have to worry about it.
    //
    int count = 0;
    for (i = wStart ; i < wEnd ; i++)
    {
        for (j = hStart ; j < hEnd ; j++)
        {
            for (k = 0 ; k < volumeDepth ; k++)
            {
                for (l = 0 ; l < nArrays ; l++)
                {
                    for (m = 0 ; m < arraySize[l] ; m++)
                    {
                        float val = (useKernel && l == (numVariables-1)
                                      ? 0. : defaultVal);
                        arrays[l]->SetComponent(count, m, val);
                    }
                }
                count++;
            }
        }
    }

    //
    // Get all of the samples out of the rays and put them in out output.
    //
    int vHeight = hEnd-hStart;
    int vWidth  = wEnd-wStart;
    for (i = hStart ; i < hEnd ; i++)
    {
        if (rays[i] != NULL)
        {
            for (j = wStart ; j < wEnd ; j++)
            {
                if (rays[i][j] != NULL)
                {
                    for (k = 0 ; k < volumeDepth ; k++)
                    {
                        float sample[AVT_VARIABLE_LIMIT];
                        if (rays[i][j]->GetSample(k, sample))
                        {
                            int index = k*vHeight*vWidth + (i-hStart)*vWidth 
                                      + (j-wStart);
                            int nv = 0;
                            for (l = 0 ; l < nArrays ; l++)
                            {
                                for (m = 0 ; m < arraySize[l] ; m++)
                                    arrays[l]->SetComponent(index, m, 
                                                            sample[nv++]);
                            }
                        }
                    }
                }
            }
        }
    }

    delete [] arraySize;
}


// ****************************************************************************
//  Method: avtVolume::SetProgressCallback
//
//  Purpose:
//      Sets a callback that this data object can use to update the progress.
//
//  Arguments:
//      pc       The progress callback.
//      pca      The arguments to the progress callback.
//
//  Programmer:  Hank Childs
//  Creation:    November 27, 2001
//
// ****************************************************************************

void
avtVolume::SetProgressCallback(PixelProgressCallback pc, void *pca)
{
    progressCallback     = pc;
    progressCallbackArgs = pca;
}


