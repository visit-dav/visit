/*****************************************************************************
*
* Copyright (c) 2000 - 2018, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

// ************************************************************************ //
//                        avtSLIVRImgCommunicator.C                         //
// ************************************************************************ //

#include <avtSLIVRImgCommunicator.h>
#include <avtSLIVROSPRayFilter.h>
#include <avtParallel.h>
#include <ImproperUseException.h>

#ifdef PARALLEL
#  ifdef VISIT_ICET
#    include <GL/ice-t.h>
#    include <GL/ice-t_mpi.h>
#  endif
#endif

#include <cmath>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <limits>
#include <fstream>
#include <algorithm>
#include <set>

#if defined (_MSC_VER) && (_MSC_VER < 1800) && !defined(round)
inline double round(double x) {return (x-floor(x))>0.5?ceil(x):floor(x);}
#endif

enum blendDirection {FRONT_TO_BACK = 0, BACK_TO_FRONT = 1};

// ***************************************************************************
// Threaded Blending
// ***************************************************************************

bool CheckThreadedBlend_Communicator()
{
    bool use = true;
    const char* env_use = std::getenv("SLIVR_NOT_USE_THREADED_BLEND");
    if (env_use) { 
        use = atoi(env_use) <= 0; 
    }
    if (!use) {
        ospout << "[avtSLIVRImgCommunicator] "
               << "Not Using Multi-Threading for Blending"
               << std::endl;
    } else {
        ospout << "[avtSLIVRImgCommunicator] "
               << "Using Multi-Threading for Blending"
               << std::endl;
    }
    return use;
}
#ifdef VISIT_OSPRAY
bool UseThreadedBlend_Communicator = CheckThreadedBlend_Communicator();
#else
bool UseThreadedBlend_Communicator = false;
#endif

// ***************************************************************************
//  Class: avtSLIVRImgComm_IceT
// ***************************************************************************

class avtSLIVRImgComm_IceT : public avtSLIVRImgComm
{
public:
    avtSLIVRImgComm_IceT(int mpiSize, int mpiRank);
    ~avtSLIVRImgComm_IceT();
    void Init(int, int);
    void SetTile(const float*, const int*, const float&);
    void Composite(float*&);
    static bool Valid();
};

bool avtSLIVRImgComm_IceT::Valid() { 
    return false;
}

avtSLIVRImgComm_IceT::avtSLIVRImgComm_IceT(int mpiSize, int mpiRank)
    : avtSLIVRImgComm(mpiSize, mpiRank)
{       
}

avtSLIVRImgComm_IceT::~avtSLIVRImgComm_IceT()
{
}

void avtSLIVRImgComm_IceT::Init(int W, int H)
{
}

void avtSLIVRImgComm_IceT::SetTile(const float* d, 
                                   const int*   e, 
                                   const float& z)
{
}

void avtSLIVRImgComm_IceT::Composite(float*& output)
{
}

// ***************************************************************************
//  End Class: avtSLIVRImgComm_IceT
// ***************************************************************************

// ***************************************************************************
//  Method: avtSLIVRImgCommunicator::avtSLIVRImgCommunicator
//
//  Purpose: Constructor
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
//    Qi WU, Wed Jun 20 2018
//    Add support for volume rendering using OSPRay (RayCasting:OSPRay)
//
// ***************************************************************************

avtSLIVRImgCommunicator::avtSLIVRImgCommunicator()
{
#ifdef PARALLEL
    MPI_Comm_size(VISIT_MPI_COMM, &mpiSize);
    MPI_Comm_rank(VISIT_MPI_COMM, &mpiRank);
#else
    mpiSize = 1;
    mpiRank = 0;
#endif
    finalImage = NULL;
    compositor = NULL;

    /////////////////////////////////////////////////////////////
    intermediateImageExtents[0] = intermediateImageExtents[1] = 0.0;
    intermediateImageExtents[2] = intermediateImageExtents[3] = 0.0;
    intermediateImageBBox[0] = intermediateImageBBox[1] = 0.0;
    intermediateImageBBox[2] = intermediateImageBBox[3] = 0.0;

    totalPatches = 0;
    intermediateImage = NULL;
    /////////////////////////////////////////////////////////////
}

// ***************************************************************************
//  Method: avtSLIVRImgCommunicator::~avtSLIVRImgCommunicator
//
//  Purpose:
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
// ***************************************************************************

avtSLIVRImgCommunicator::~avtSLIVRImgCommunicator()
{
    //if (mpiRank == 0) { if (finalImage != NULL) { delete [] finalImage; } }
}

// ***************************************************************************
//  Method: avtSLIVRImgCommunicator::Barrier
//
//  Purpose:
//    Barrier, useful for debugging
//
//  Programmer: Pascal Grosset
//  Creation: July 2013
//
//  Modifications:
//
// ***************************************************************************

void avtSLIVRImgCommunicator::Barrier() {
#ifdef PARALLEL
    MPI_Barrier(MPI_COMM_WORLD);
#endif
}


// ***************************************************************************
//  Method: avtSLIVRImgCommunicator::ColorImage
//
//  Purpose:
//       Fills a 4 channel image with a specific color
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
// ***************************************************************************

void
avtSLIVRImgCommunicator::ColorImage(float *&srcImage,
                                    const int widthSrc,
                                    const int heightSrc,
                                    const float color[4])
{
    for (int i = 0; i < heightSrc * widthSrc; ++i) {
        const int srcIndex = 4 * i;
        srcImage[srcIndex+0] = color[0];
        srcImage[srcIndex+1] = color[1];
        srcImage[srcIndex+2] = color[2];
        srcImage[srcIndex+3] = color[3];
    }
}


// ***************************************************************************
//  Method: avtSLIVRImgCommunicator::PlaceImage
//
//  Purpose:
//      Puts srcImage into dstImage
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
// ***************************************************************************

void
avtSLIVRImgCommunicator::PlaceImage(const float *srcImage,
                                    const int srcExtents[4], 
                                    float *&dstImage,
                                    const int dstExtents[4])
{
    const int srcX = srcExtents[1] - srcExtents[0];
    const int srcY = srcExtents[3] - srcExtents[2];
    const int dstX = dstExtents[1] - dstExtents[0];
    const int dstY = dstExtents[3] - dstExtents[2];
    const int startingX = std::max(srcExtents[0], dstExtents[0]);
    const int startingY = std::max(srcExtents[2], dstExtents[2]);
    const int endingX = std::min(srcExtents[1], dstExtents[1]);
    const int endingY = std::min(srcExtents[3], dstExtents[3]);
    
    for (int y = startingY; y < endingY; ++y) {
        for (int x = startingX; x < endingX; ++x) {
            // index in the sub-image
            const int srcIndex = 
                (srcX * (y-srcExtents[2]) + x-srcExtents[0]) * 4; 
            // index in the larger buffer
            const int dstIndex = 
                (dstX * (y-dstExtents[2]) + x-dstExtents[0]) * 4;
            dstImage[dstIndex+0] = srcImage[srcIndex+0];
            dstImage[dstIndex+1] = srcImage[srcIndex+1];
            dstImage[dstIndex+2] = srcImage[srcIndex+2];
            dstImage[dstIndex+3] = srcImage[srcIndex+3];
        }
    }
}


// ***************************************************************************
//  Method: avtSLIVRImgCommunicator::BlendWithBackground
//
//  Purpose:
//      Blends _image with the backgroundColor
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
//    Qi WU, Wed Jun 20 2018
//    Add support for threaded blending using OSPRay
//
// **************************************************************************
void
avtSLIVRImgCommunicator::BlendWithBackground(float *&image,
                                             const int extents[4],
                                             const float bgColor[4])
{
    const int pixelSize = (extents[3]-extents[2]) * (extents[1]-extents[0]);
    // estimated potential speedup: 2.240
    for (int i = 0; i < pixelSize; ++i)
    {
        const int   idx = i * 4;
        const float alpha = (1.0 - image[idx+3]);
        image[idx+0] = bgColor[0] * alpha + image[idx+0];
        image[idx+1] = bgColor[1] * alpha + image[idx+1];
        image[idx+2] = bgColor[2] * alpha + image[idx+2];
        image[idx+3] = bgColor[3] * alpha + image[idx+3];
    }
}


// ***************************************************************************
//  Method: avtSLIVRImgCommunicator::BlendFrontToBack
//
//  Purpose:
//      Blends tow patches in a front to back manner
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
//    Qi WU, Wed Jun 20 2018
//    Add support for threaded blending using OSPRay
//
// **************************************************************************

void
avtSLIVRImgCommunicator::BlendFrontToBack(const float *srcImage,
                                          const int srcExtents[4],
                                          const int blendExtents[4], 
                                          float *&dstImage,
                                          const int dstExtents[4])
{
#ifdef VISIT_OSPRAY
    if (UseThreadedBlend_Communicator) {
        visit::BlendFrontToBack(blendExtents,
                                srcExtents,
                                srcImage,
                                dstExtents,
                                dstImage);
    } else {
#endif
    // image sizes
    const int srcX = srcExtents[1] - srcExtents[0];
    const int srcY = srcExtents[3] - srcExtents[2];
    const int dstX = dstExtents[1] - dstExtents[0];
    const int dstY = dstExtents[3] - dstExtents[2];
    // determin the region to blend
    const int startX = 
        std::max(std::max(blendExtents[0], srcExtents[0]), dstExtents[0]);
    const int startY = 
        std::max(std::max(blendExtents[2], srcExtents[2]), dstExtents[2]);
    const int endX = 
        std::min(std::min(blendExtents[1], srcExtents[1]), dstExtents[1]);
    const int endY = 
        std::min(std::min(blendExtents[3], srcExtents[3]), dstExtents[3]);
    
    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            // get indices
            int srcIndex = (srcX * (y-srcExtents[2]) + x-srcExtents[0]) * 4;
            int dstIndex = (dstX * (y-dstExtents[2]) + x-dstExtents[0]) * 4;

            // front to back compositing
            if (dstImage[dstIndex + 3] < 1.0f) {
                float trans = 1.0f - dstImage[dstIndex + 3];
                dstImage[dstIndex+0] = 
                    CLAMP(srcImage[srcIndex+0] * trans + dstImage[dstIndex+0],
                          0.0f, 1.0f);
                dstImage[dstIndex+1] = 
                    CLAMP(srcImage[srcIndex+1] * trans + dstImage[dstIndex+1],
                          0.0f, 1.0f);
                dstImage[dstIndex+2] = 
                    CLAMP(srcImage[srcIndex+2] * trans + dstImage[dstIndex+2],
                          0.0f, 1.0f);
                dstImage[dstIndex+3] = 
                    CLAMP(srcImage[srcIndex+3] * trans + dstImage[dstIndex+3],
                          0.0f, 1.0f);
            }
        }
    }
#ifdef VISIT_OSPRAY
    }
#endif
}


// ***************************************************************************
//  Method: avtSLIVRImgCommunicator::BlendBackToFront
//
//  Purpose:
//      Blends tow patches in a back to front manner
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
//    Qi WU, Wed Jun 20 2018
//    Add support for threaded blending using OSPRay
//
// **************************************************************************

void
avtSLIVRImgCommunicator::BlendBackToFront(const float *srcImage,
                                          const int srcExtents[4],
                                          const int blendExtents[4], 
                                          float *&dstImage,
                                          const int dstExtents[4])
{
#ifdef VISIT_OSPRAY
    if (UseThreadedBlend_Communicator) {
        visit::BlendBackToFront(blendExtents,
                                srcExtents,
                                srcImage,
                                dstExtents,
                                dstImage);
    } else {
#endif
    // image sizes
    const int srcX = srcExtents[1] - srcExtents[0];
    const int srcY = srcExtents[3] - srcExtents[2];
    const int dstX = dstExtents[1] - dstExtents[0];
    const int dstY = dstExtents[3] - dstExtents[2];
    // determin the region to blend
    const int startX = 
        std::max(std::max(blendExtents[0], srcExtents[0]), dstExtents[0]);
    const int startY = 
        std::max(std::max(blendExtents[2], srcExtents[2]), dstExtents[2]);
    const int endX = 
        std::min(std::min(blendExtents[1], srcExtents[1]), dstExtents[1]);
    const int endY = 
        std::min(std::min(blendExtents[3], srcExtents[3]), dstExtents[3]);
    
    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            // get indices
            int srcIndex = (srcX * (y-srcExtents[2]) + x-srcExtents[0]) * 4;
            int dstIndex = (dstX * (y-dstExtents[2]) + x-dstExtents[0]) * 4;
            // back to front compositing            
            float trans = 1.0f - srcImage[srcIndex + 3];
            dstImage[dstIndex+0] = 
                CLAMP(dstImage[dstIndex+0] * trans + srcImage[srcIndex+0],
                      0.0f, 1.0f);
            dstImage[dstIndex+1] = 
                CLAMP(dstImage[dstIndex+1] * trans + srcImage[srcIndex+1],
                      0.0f, 1.0f);
            dstImage[dstIndex+2] = 
                CLAMP(dstImage[dstIndex+2] * trans + srcImage[srcIndex+2],
                      0.0f, 1.0f);
            dstImage[dstIndex+3] = 
                CLAMP(dstImage[dstIndex+3] * trans + srcImage[srcIndex+3],
                      0.0f, 1.0f);
        }
    }
#ifdef VISIT_OSPRAY
    }
#endif
}


// ***************************************************************************
//  Method: avtSLIVRImgCommunicator::BlendFrontToBack
//
//  Purpose:
//      Blends tow patches in a front to back manner
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
//    Qi WU, Wed Jun 20 2018
//    Add support for threaded blending using OSPRay
//
// **************************************************************************
void
avtSLIVRImgCommunicator::BlendFrontToBack(const float * srcImage,
                                          const int srcExtents[4], 
                                          float *& dstImage,
                                          const int dstExtents[4])
{
    BlendFrontToBack(srcImage, srcExtents, srcExtents, dstImage, dstExtents);
}


// ***************************************************************************
//  Method: avtSLIVRImgCommunicator::BlendBackToFront
//
//  Purpose:
//      Blends tow patches in a back to front manner
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
// **************************************************************************

void
avtSLIVRImgCommunicator::BlendBackToFront
(const float * srcImage, const int srcExtents[4], 
 float *& dstImage, const int dstExtents[4])
{
    BlendBackToFront(srcImage, srcExtents, srcExtents, dstImage, dstExtents);
}


// ***************************************************************************
//  Method: avtSLIVRImgCommunicator::UpdateBoundingBox
//
//  Purpose:
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
// **************************************************************************

void
avtSLIVRImgCommunicator::UpdateBoundingBox(int currentBoundingBox[4],
                                           const int imageExtents[4])
{
    if ((currentBoundingBox[0] == 0 && currentBoundingBox[1] == 0) && 
        (currentBoundingBox[2] == 0 && currentBoundingBox[3] == 0))
    {
        currentBoundingBox[0]=imageExtents[0];
        currentBoundingBox[1]=imageExtents[1];
        currentBoundingBox[2]=imageExtents[2];
        currentBoundingBox[3]=imageExtents[3];
        return;
    }

    if (imageExtents[0] < currentBoundingBox[0])
    { currentBoundingBox[0] = imageExtents[0]; }

    if (imageExtents[2] < currentBoundingBox[2])
    { currentBoundingBox[2] = imageExtents[2]; }

    if (imageExtents[1] > currentBoundingBox[1])
    { currentBoundingBox[1] = imageExtents[1]; }

    if (imageExtents[3] > currentBoundingBox[3])
    { currentBoundingBox[3] = imageExtents[3]; }
}

// ***************************************************************************
//  Method: avtSLIVRImgCommunicator::IceT
//
//  Purpose:
//
//  Programmer: Qi WU
//  Creation:   
//
//  Modifications:
//
// **************************************************************************

bool 
avtSLIVRImgCommunicator::IceTValid() { 
    return avtSLIVRImgComm_IceT::Valid(); 
}

void 
avtSLIVRImgCommunicator::IceTInit(int W, int H)
{
    if (!avtSLIVRImgComm_IceT::Valid()) {
        debug1 << "ERROR: IceT compositor is not valid. "
               << "Probably IceT is not compiled with VisIt"
               << std::endl;
        EXCEPTION1(VisItException, 
                   "ERROR: IceT compositor is not valid. "
                   "Probably IceT is not compiled with VisIt");
        return;
    }
    compositor = new avtSLIVRImgComm_IceT(mpiSize, mpiRank);
    compositor->Init(W, H);
}

void avtSLIVRImgCommunicator::IceTSetTile(const float* d, 
                                          const int*   e,
                                          const float& z)
{
    int timingDetail;
    //---------------------------------------------------------------------//
    slivr::CheckSectionStart("avtSLIVRImgCommunicator", 
                             "IceTSetTile", timingDetail,
                             "IceT Setup Image Tile");
    //---------------------------------------------------------------------//
    compositor->SetTile(d, e, z);
    //---------------------------------------------------------------------//
    slivr::CheckSectionStop("avtSLIVRImgCommunicator", 
                            "IceTSetTile", timingDetail,
                            "IceT Setup Image Tile");
    //---------------------------------------------------------------------//
}

void avtSLIVRImgCommunicator::IceTComposite(float*& output)
{
    int timingDetail;
    //---------------------------------------------------------------------//
    slivr::CheckSectionStart("avtSLIVRImgCommunicator", 
                             "IceTComposite", timingDetail,
                             "IceT Image Composition");
    //---------------------------------------------------------------------//
    compositor->Composite(output);
    if (compositor != NULL) { delete compositor; }
    compositor = NULL;
    //---------------------------------------------------------------------//
    slivr::CheckSectionStop("avtSLIVRImgCommunicator", 
                            "IceTComposite", timingDetail,
                            "IceT Image Composition");
    //---------------------------------------------------------------------//
}

// ***************************************************************************
//  Method: avtSLIVRImgCommunicator::GatherDepthAtRoot
//
//  Purpose:
//      Used by Serial Direct Send
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
// **************************************************************************

void
avtSLIVRImgCommunicator::GatherDepthAtRoot(const int numlocalPatches,
                                           const float *localPatchesDepth,
                                           int &totalPatches,
                                           int *&patchCountPerRank,
                                           float *&allPatchesDepth)
{
#ifdef PARALLEL
    // Get how many patches are coming from each MPI rank
    totalPatches = 0;
    int *patchesOffset = NULL;

    if (mpiRank == 0) // root!
    { patchCountPerRank = new int[mpiSize](); }

    // reference
    // https://www.mpich.org/static/docs/v3.1/www3/MPI_Gather.html
    MPI_Gather(&numlocalPatches, /* send buffer */
               1, /* send count */
               MPI_INT, 
               patchCountPerRank, /* address of receive buffer (root) */
               1, /* number of elements for any single receive (root) */
               MPI_INT, 
               0, /* rank of receiving process (integer) */
               MPI_COMM_WORLD); /* communicator (handle) */

    // gather number of patch group
    if (mpiRank == 0)
    {
        patchesOffset = new int[mpiSize]();
        patchesOffset[0] = 0; // a bit redundant

        for (int i=0; i<mpiSize; i++)
        {
            totalPatches += patchCountPerRank[i];
            if (i == 0)
            { patchesOffset[i] = 0; }
            else
            {
                patchesOffset[i] = 
                    patchesOffset[i-1] + patchCountPerRank[i-1]; 
            }
        }
        
        // allocate only at root
        allPatchesDepth = new float[totalPatches];
    }

    // Gathers into specified locations from all processes in a group
    MPI_Gatherv(localPatchesDepth, numlocalPatches, MPI_FLOAT, 
                allPatchesDepth, /* receive all depth */
                patchCountPerRank, patchesOffset, MPI_FLOAT, 
                0, MPI_COMM_WORLD);

    // Cleanup
    if (mpiRank == 0 && patchesOffset != NULL)
    { delete [] patchesOffset; }
    patchesOffset = NULL;
#endif
}

// ***************************************************************************
//  Method: avtSLIVRImgCommunicator::SerialDirectSend
//
//  Purpose:
//      A very simple compositing that we can fall back to if parallel direct
//      send is buggy.
//      Works with convex patches though
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
// **************************************************************************

void
avtSLIVRImgCommunicator::SerialDirectSend(int localNumPatches,
                                          float *localPatchesDepth, 
                                          int *extents,
                                          float *imgData,
                                          float bgColor[4],
                                          int width, int height)
{
#ifdef PARALLEL
    //
    // Message
    //
    debug5 << "-- Serial Direct Send --" << std::endl;

    float *recvImage = NULL;
    int tags[2] = {5781, 5782};
    //
    // Retrieve depth info through MPI
    //
    int    totalPatches; // total number of patches
    int   *totalPatchCountsPerRank = NULL;
    float *totalPatchDepths = NULL;
    GatherDepthAtRoot(localNumPatches,
                      localPatchesDepth,
                      totalPatches, 
                      totalPatchCountsPerRank,
                      totalPatchDepths);
    //
    //
    //
    if (mpiRank == 0)
    {
        // Root
        int srcSize[2] = {width, height};
        int srcPos[2]  = {0, 0};
        int dstSize[2], dstPos[2];

        // Sort patches we will receive
        std::multimap<float,int> sortedPatches;

        int patchId = 0;
        for (int i=0; i<mpiSize; i++) {
            for (int j=0; j<totalPatchCountsPerRank[i]; j++) {
                sortedPatches.insert
                    (std::make_pair(totalPatchDepths[patchId++],i));
            }
        }

        // Create space for buffers
        int recvParams[4]; // minX, maxX, minY, maxY
        int imgExtents[4] = {0,width,0,height};

        recvImage = new float[width*height*4]();
        finalImage = new float[width*height*4]();
        
        int localIndex = 0;

        // Compositing
        for (std::multimap<float,int>::iterator it = sortedPatches.begin(); 
             it != sortedPatches.end(); ++it)
        {
            int rank = (*it).second;
            if (rank != mpiRank)
            {
                // recv image info
                MPI_Recv(recvParams, 4, MPI_INT, rank, 
                         tags[0], MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
                // recv image
                MPI_Recv(recvImage, width*height*4, MPI_FLOAT, rank,
                         tags[1],  MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                dstPos[0]  = dstPos[0];                     
                dstPos[1]  = dstPos[1];
                dstSize[0] = recvParams[2]-recvParams[0];   
                dstSize[1] = recvParams[3]-recvParams[1];
            }
            else
            {
                // It's local
                recvParams[0] = extents[ localIndex*4 + 0];
                recvParams[1] = extents[ localIndex*4 + 1];
                recvParams[2] = extents[ localIndex*4 + 2];
                recvParams[3] = extents[ localIndex*4 + 3];
                recvImage = &imgData[ localIndex*(width*height*4) ];
                localIndex++;
            }
            BlendFrontToBack(recvImage, recvParams, finalImage, imgExtents);
        }
        BlendWithBackground(finalImage, imgExtents, bgColor);
    }
    else
    {
        // Sender
        for (int i=0; i<localNumPatches; i++)
        {
            int imgSize = 
                (extents[i*4 + 1] - extents[i*4 + 0]) *
                (extents[i*4 + 3] - extents[i*4 + 2]) * 4;          
            if (imgSize > 0)
            {
                MPI_Send(&extents[i*4],
                         4, MPI_INT, 0, tags[0], MPI_COMM_WORLD);
                MPI_Send(&imgData[i*(width*height*4)], 
                         imgSize, MPI_FLOAT, 0, tags[1], MPI_COMM_WORLD);
            }
        }
    }

    // Cleanup
    if (totalPatchDepths != NULL)
        delete []totalPatchDepths;

    if (totalPatchCountsPerRank != NULL)
        delete []totalPatchCountsPerRank;

    if (recvImage != NULL)
        delete []recvImage;

    recvImage = NULL;
    totalPatchCountsPerRank = NULL;
    totalPatchDepths = NULL;
#endif
}

// ***************************************************************************
//  Method: avtSLIVRImgCommunicator::regionAllocation
//
//  Purpose:
//      Arbitrarily allocates regions to MPI ranks
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
// ***************************************************************************

void
avtSLIVRImgCommunicator::RegionAllocation(int *& regions)
{
    regions = new int[mpiSize];
    // Initial allocation: partition for section rank
    for (int i=0; i<mpiSize; i++) { regions[i] = i; }
}


// ***************************************************************************
//  Method: avtSLIVRImgCommunicator::parallelDirectSend
//
//  Purpose:
//      Parallel Direct Send rendering that can blend convex patches from each
//      MPI rank. However, since we are not guaranteed to have convex patches.
//      It's not used.
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
// **************************************************************************
void
avtSLIVRImgCommunicator::parallelDirectSend(float *imgData,
                                            int imgExtents[4],
                                            int region[],
                                            int numRegions,
                                            int tags[2],
                                            int fullImageExtents[4])
{
#ifdef PARALLEL
    //
    // Determine position in region (myPositionInRegion)
    int width  = fullImageExtents[1]-fullImageExtents[0];
    int height = fullImageExtents[3]-fullImageExtents[2];

    //debug5 << "fullImageExtents: " << fullImageExtents[0] << ", " << fullImageExtents[1] << "   " << fullImageExtents[2] << ", " << fullImageExtents[3] << endl;

    compositingDone = false;
    int myPositionInRegion = -1;
    bool inRegion = true;
    std::vector<int> regionVector(region, region+numRegions);
    std::vector<int>::iterator it = std::find(regionVector.begin(),
                                              regionVector.end(),
                                              mpiRank);

    if (it == regionVector.end())
    {
        inRegion = false;
        //debug5 << mpiRank << " ~ SHOULD NOT HAPPEN: Not found " << mpiRank <<  " !!!" << std::endl;
    }
    else
        myPositionInRegion = it - regionVector.begin();

    //
    // Region boundaries
    int regionHeight = height/numRegions;
    int lastRegionHeight = height - regionHeight*(numRegions-1);

    // Extents of my region
    int myStartingHeight =
        fullImageExtents[2] +
        myPositionInRegion *
        regionHeight;
    int myEndingHeight = myStartingHeight + regionHeight;
    if (myPositionInRegion == numRegions-1)
        myEndingHeight = fullImageExtents[3];

    int myRegionHeight = myEndingHeight-myStartingHeight;

    // Size of one buffer
    int sizeOneBuffer = std::max(regionHeight,lastRegionHeight) * width * 4;

    //debug5 << "myPositionInRegion: " << myPositionInRegion << std::endl;
    //debug5 << "My extents: " << imgExtents[0] << ", " << imgExtents[1] << ", " << imgExtents[2] << ", " << imgExtents[3] << std::endl;
    //debug5 << "myRegionHeight: " << myRegionHeight << "  lastRegionHeight: " << lastRegionHeight << " regionHeight: " << regionHeight << "  myStartingHeight: " << myStartingHeight << "  myEndingHeight: " << myEndingHeight << std::endl;


    //
    // MPI Async

    // Recv
    MPI_Request *recvMetaRq = new MPI_Request[ numRegions-1 ];
    MPI_Request *recvImageRq = new MPI_Request[ numRegions-1 ];

    MPI_Status *recvMetaSt = new MPI_Status[ numRegions-1 ];
    MPI_Status *recvImageSt = new MPI_Status[ numRegions-1 ];

    // Send
    MPI_Request *sendMetaRq = new MPI_Request[ numRegions-1 ];
    MPI_Request *sendImageRq = new MPI_Request[ numRegions-1 ];

    MPI_Status *sendMetaSt = new MPI_Status[ numRegions-1 ];
    MPI_Status *sendImageSt = new MPI_Status[ numRegions-1 ];


    //
    // Create Buffers

    // Create buffer for receiving images
    float *recvDataBuffer;
    recvDataBuffer = new float[ sizeOneBuffer * numRegions];

    // Create buffer for receiving messages
    std::vector<int> msgBuffer;
    msgBuffer.clear();
    msgBuffer.resize(5 * numRegions);

    // Create buffer for sending messages
    int *sendExtents = new int[numRegions*5];

    //
    // Async Recv
    if (inRegion)
    {
        int recvCount=0;
        for (int i=0; i<numRegions; i++)
        {
            if ( regionVector[i] == mpiRank )
                continue;

            int src = regionVector[i];
            MPI_Irecv(&msgBuffer[i*5],                              5, MPI_INT,   src, tags[0], MPI_COMM_WORLD,  &recvMetaRq[recvCount] );
            MPI_Irecv(&recvDataBuffer[i*sizeOneBuffer], sizeOneBuffer, MPI_FLOAT, src, tags[1], MPI_COMM_WORLD,  &recvImageRq[recvCount] );
            recvCount++;
        }
    }

    //debug5 << "Async Recv setup done " << std::endl;

    //
    // Async Send
    int sendCount = 0;
    int sendingOffset;
    for (int i=0; i<numRegions; i++)
    {
        int regionStart, regionEnd, imgSize, dest;
        dest = regionVector[i];

        if ( dest == mpiRank )
            continue;

        regionStart = i*regionHeight;
        regionEnd = regionStart + regionHeight;
        if (i == numRegions-1) // the last one in region
            regionEnd = height;

        int startingYExtents = fullImageExtents[2] + regionStart;
        int endingYExtents = fullImageExtents[2] + regionEnd;

        //debug5 << "startingYExtents: " << startingYExtents <<"   endingYExtents: " << endingYExtents <<  std::endl;

        if (startingYExtents < imgExtents[2])
            startingYExtents = imgExtents[2];

        if (endingYExtents > imgExtents[3])
            endingYExtents = imgExtents[3];

        bool hasData = true;
        if (endingYExtents - startingYExtents <= 0 || imgExtents[1]-imgExtents[0] <= 0)
        {
            hasData = false;

            sendingOffset = 0;
            imgSize = sendExtents[i*5 + 0] = sendExtents[i*5 + 1] = sendExtents[i*5 + 2] = sendExtents[i*5 + 3] =  sendExtents[i*5 + 4] = 0;
        }
        else
        {
            imgSize = (endingYExtents-startingYExtents) * (imgExtents[1]-imgExtents[0]) * 4;
            sendingOffset = (startingYExtents-imgExtents[2]) * (imgExtents[1]-imgExtents[0]) * 4;

            sendExtents[i*5 + 0] = imgExtents[0];
            sendExtents[i*5 + 1] = imgExtents[1];
            sendExtents[i*5 + 2] = startingYExtents;
            sendExtents[i*5 + 3] = endingYExtents;
            sendExtents[i*5 + 4] = 0;
        }

        //std::cout << mpiRank << " ~ i: " << i << "   regionVector[index]: " << regionVector[index] << "  extents: " <<  sendExtents[index*5 + 0] << ", " << sendExtents[index*5 + 1]  << ", " << sendExtents[index*5 + 2] << ", " << sendExtents[index*5 + 3] << "  sending ... " << std::endl;
        MPI_Isend(&sendExtents[i*5],             5,   MPI_INT, dest, tags[0], MPI_COMM_WORLD, &sendMetaRq[sendCount]);
        MPI_Isend(&imgData[sendingOffset], imgSize, MPI_FLOAT, dest, tags[1], MPI_COMM_WORLD, &sendImageRq[sendCount]);

        //debug5 << "dest: " << dest <<"   sendExtents: " << sendExtents[i*5 +0] << ", " << sendExtents[i*5 +1] << "    " << sendExtents[i*5 +2] << ", " << sendExtents[i*5 +3] << std::endl << std::endl;

        sendCount++;
    }

    //debug5 << "Async Recv" << std::endl;

    //
    // Create buffer for region
    intermediateImageExtents[0] = fullImageExtents[0];  intermediateImageExtents[1] = fullImageExtents[1];
    intermediateImageExtents[2] = myStartingHeight;     intermediateImageExtents[3] = myEndingHeight;

    intermediateImage = new float[width * (myEndingHeight-myStartingHeight) * 4]();

    int recvImageExtents[4];
    float *recvImageData;

    //
    // Blend
    int numBlends = 0;
    int countBlend = 0;

    intermediateImageBBox[0] = intermediateImageBBox[2] = 0;
    intermediateImageBBox[1] = intermediateImageBBox[3] = 0;

    if (inRegion)
    {
        for (int i=0; i<numRegions; i++)
        {
            int index = i;

            //debug5 << "regionVector[" << i << "] " << regionVector[index] << std::endl;

            if (regionVector[index] == mpiRank)
            {
                int startingYExtents = myStartingHeight;
                int endingYExtents = myEndingHeight;

                if (startingYExtents < imgExtents[2])
                    startingYExtents = imgExtents[2];

                if (endingYExtents > imgExtents[3])
                    endingYExtents = imgExtents[3];


                bool hasData = true;
                if (endingYExtents - startingYExtents <= 0)
                {
                    hasData = false;
                    endingYExtents = startingYExtents = 0;
                }

                if (hasData == true)
                {
                    int extentsSectionRecv[4];
                    extentsSectionRecv[0] = imgExtents[0];
                    extentsSectionRecv[1] = imgExtents[1];
                    extentsSectionRecv[2] = startingYExtents;
                    extentsSectionRecv[3] = endingYExtents;

                    BlendFrontToBack(imgData, imgExtents, extentsSectionRecv, intermediateImage, intermediateImageExtents);

                    UpdateBoundingBox(intermediateImageBBox, extentsSectionRecv);
                    numBlends++;
                }
            }
            else
            {
                MPI_Wait(&recvMetaRq[countBlend], &recvMetaSt[countBlend]);

                for (int j=0; j<4; j++)
                    recvImageExtents[j] = msgBuffer[index*5 + j];

                bool hasData =  false;
                if (recvImageExtents[1]-recvImageExtents[0] > 0 && recvImageExtents[3]-recvImageExtents[2] > 0)
                {
                    hasData = true;
                    MPI_Wait(&recvImageRq[countBlend], &recvImageSt[countBlend]);
                    recvImageData = &recvDataBuffer[index*sizeOneBuffer];
                }

                if (hasData)
                {

                    BlendFrontToBack(recvImageData, recvImageExtents, intermediateImage, intermediateImageExtents);

                    UpdateBoundingBox(intermediateImageBBox, recvImageExtents);
                    numBlends++;
                }

                countBlend++;
            }
        }
    }
    else
        compositingDone = true;

    //debug5 << "PDS blending done" << std::endl;

    msgBuffer.clear();


    if (recvDataBuffer != NULL)
        delete []recvDataBuffer;
    recvDataBuffer = NULL;


    if (numBlends == 0)
        intermediateImageBBox[0]=intermediateImageBBox[1]=intermediateImageBBox[2]=intermediateImageBBox[3] = 0;

    delete []recvMetaRq;
    delete []recvImageRq;
    delete []recvMetaSt;
    delete []recvImageSt;

    delete []sendMetaRq;
    delete []sendImageRq;
    delete []sendMetaSt;
    delete []sendImageSt;

    recvMetaRq = NULL;
    recvImageRq = NULL;
    recvMetaSt = NULL;
    recvImageSt = NULL;

    sendMetaRq = NULL;
    sendImageRq = NULL;
    sendMetaSt = NULL;
    sendImageSt = NULL;
#endif
}


// ***************************************************************************
//  Method: avtSLIVRImgCommunicator::findRegionsForPatch
//
//  Purpose:
//      Needed by Parallel Direct Send to determine the regions a patch will overlap
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
// **************************************************************************
int
avtSLIVRImgCommunicator::findRegionsForPatch(int patchExtents[4], int screenProjectedExtents[4], int numRegions, int &from, int &to)
{
    from = to = 0;
    if (patchExtents[1]-patchExtents[0] <=0 || patchExtents[3]-patchExtents[2] <=0)
        return 0;

    if ( patchExtents[0] > screenProjectedExtents[1])
        return 0;

    if ( patchExtents[1] < screenProjectedExtents[0])
        return 0;

    if ( patchExtents[2] > screenProjectedExtents[3])
        return 0;

    if ( patchExtents[3] < screenProjectedExtents[2])
        return 0;


    // find from
    for (int i=numRegions-1; i>=0; i--)
        if ( patchExtents[2] >= getScreenRegionStart(i, screenProjectedExtents[2], screenProjectedExtents[3]) )
        {
            from = i;
            break;
        }
    

    // find to
    for (int i=numRegions-1; i>=0; i--)
        if ( patchExtents[3] > getScreenRegionStart(i, screenProjectedExtents[2], screenProjectedExtents[3]) )
        {
            to = i;
            break;
        }

    return ( std::max( (to - from) + 1, 0) );
}


// ***************************************************************************
//  Method: avtSLIVRImgCommunicator::computeRegionExtents
//
//  Purpose:
//      Compute extents for each region
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
// **************************************************************************
void
avtSLIVRImgCommunicator::computeRegionExtents(int numRanks, int height)
{
    //debug5 << "computeRegionExtents height " << height << std::endl;

    int regionHeight = round((float)height/numRanks);
    regularRegionSize = regionHeight;
    maxRegionHeight = 0;
    regionRankExtents.resize(numRanks*3);
    for (int i=0; i<numRanks; i++)
    {
        int startRegionExtents, endRegionExtents, _currentRegionHeight;

        startRegionExtents = CLAMP(regionHeight * i, 0, height);
        endRegionExtents = CLAMP(regionHeight * i + regionHeight, 0, height);

        if ( i == numRanks -1 )
            if ( endRegionExtents < height )
                endRegionExtents = height;

        _currentRegionHeight = CLAMP(endRegionExtents-startRegionExtents, 0, height);
        maxRegionHeight = std::max(maxRegionHeight, _currentRegionHeight);

        regionRankExtents[i*3+0] = startRegionExtents;
        regionRankExtents[i*3+1] = endRegionExtents;
        regionRankExtents[i*3+2] = _currentRegionHeight;

        debug5 << i << " : (start, end, region): " << startRegionExtents << ", " << endRegionExtents << ", " << _currentRegionHeight << std::endl;
    }
}


// ***************************************************************************
//  Method: avtSLIVRImgCommunicator::ParallelDirectSendManyPatches
//
//  Purpose:
//      Parallel Direct Send rendering that can blend individual patches
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
// **************************************************************************

int
avtSLIVRImgCommunicator::ParallelDirectSendManyPatches
    (const std::multimap<int, slivr::ImgData> &imgDataHashMap, 
     const std::vector<slivr::ImgMetaData>    &imageMetaPatchVector, 
     int numPatches,
     int *region,
     int numRegions, 
     int tags[2],
     int fullImageExtents[4])
{
    int myRegionHeight = 0;
#ifdef PARALLEL

    //
    // Some Initializations
    //
    debug5 << "Parallel Direct Send" << endl;
    int timingDetail;
    for (int i=0; i<4; i++)
    {
        intermediateImageExtents[i] = 0;
        intermediateImageBBox[i] = 0;
    }


    //
    // Find My Position in Regions
    //
    //---------------------------------------------------------------------//
    slivr::CheckSectionStart("avtSLIVRImgCommunicator", 
                             "ParallelDirectSendManyPatches", timingDetail,
                             "Find My position in Regions");
    //---------------------------------------------------------------------//
    compositingDone = false;
    int myPositionInRegion = -1;
    bool inRegion = true;
    std::vector<int> regionVector(region, region+numRegions);
    const std::vector<int>::const_iterator it = 
        std::find(regionVector.begin(),
                  regionVector.end(), 
                  mpiRank);
    if (it == regionVector.end())
    {
        inRegion = false;
        debug5 << mpiRank << " ~ SHOULD NOT HAPPEN!!!!: Not found " 
               << mpiRank <<  " !!!" << std::endl;
    }
    else 
    {
        myPositionInRegion = it - regionVector.begin();
    }
    int width =  fullImageExtents[1]-fullImageExtents[0];
    int height = fullImageExtents[3]-fullImageExtents[2];
    //---------------------------------------------------------------------//
    slivr::CheckSectionStop("avtSLIVRImgCommunicator", 
                            "ParallelDirectSendManyPatches", timingDetail,
                            "Find My position in Regions");
    debug5 << mpiRank << " ~ myPositionInRegion: " 
           << myPositionInRegion << ", numRanks: " << mpiSize << std::endl;
    debug5 << "width: " << width << ", height : " << height 
           << " | fullImageExtents: "
           << fullImageExtents[0] << ", " 
           << fullImageExtents[1] << ", " 
           << fullImageExtents[2] << ", "
           << fullImageExtents[3] << std::endl;
    //---------------------------------------------------------------------//


    //
    // Compute Region Boundaries
    //
    //---------------------------------------------------------------------//
    slivr::CheckSectionStart("avtSLIVRImgCommunicator", 
                             "ParallelDirectSendManyPatches", timingDetail,
                             "Compute Region Boundaries");
    //---------------------------------------------------------------------//
    computeRegionExtents(mpiSize, height); // ?
    int myStartingHeight = getScreenRegionStart
        (myPositionInRegion, fullImageExtents[2], fullImageExtents[3]);
    int myEndingHeight   = getScreenRegionEnd
        (myPositionInRegion, fullImageExtents[2], fullImageExtents[3]);
    myRegionHeight = CLAMP((myEndingHeight-myStartingHeight), 0, height);
    //---------------------------------------------------------------------//
    slivr::CheckSectionStop("avtSLIVRImgCommunicator", 
                            "ParallelDirectSendManyPatches", timingDetail,
                            "Compute Region Boundaries");
    debug5 << "myStartingHeight: " << myStartingHeight << ", "
           << "myEndingHeight: "   << myEndingHeight   << ", "
           << "myRegionHeight: "   << myRegionHeight   << std::endl;
    //---------------------------------------------------------------------//


    //
    // Size of one buffer
    //
    int sizeOneBuffer = getMaxRegionHeight() * width * 4;


    //
    // Determine How Many Patches and Pixel to Send to Each Region
    //
    //---------------------------------------------------------------------//
    slivr::CheckSectionStart("avtSLIVRImgCommunicator", 
                             "ParallelDirectSendManyPatches", timingDetail,
                             "Determine How Many Patches and Pixel to Send "
                             "to Each Region");
    //---------------------------------------------------------------------//
    std::vector<int> numPatchesPerRegion;
    std::vector<int> areaPerRegion;
    std::set<int> numOfRegions;
    numPatchesPerRegion.resize(numRegions);
    areaPerRegion.resize(numRegions);

    // 2D array: extents for each partition
    std::vector < std::vector<float> > extentsPerPartiton;
    for (int i=0; i<numRegions; i++) { 
        extentsPerPartiton.push_back(std::vector<float>()); 
    }
    debug5 << "Parallel Direct Send ~ numPatches " << numPatches << endl;
    int totalSendBufferSize = 0;
    for (int i=0; i<numPatches; i++)
    {
        int _patchExtents[4];
        slivr::ImgMetaData temp;
        temp = imageMetaPatchVector.at(i);
        _patchExtents[0]=temp.screen_ll[0];   // minX
        _patchExtents[1]=temp.screen_ur[0];   // maxX
        _patchExtents[2]=temp.screen_ll[1];   // minY
        _patchExtents[3]=temp.screen_ur[1];   // maxY
        const std::multimap<int, slivr::ImgData>::const_iterator it = 
            imgDataHashMap.find( i );
        int from, to;
        int numRegionIntescection = findRegionsForPatch(_patchExtents, 
                                                        fullImageExtents,
                                                        numRegions, 
                                                        from, to);
        if (numRegionIntescection <= 0) continue;
        debug5 << "\nParallel Direct Send ~ patch " << i 
               << "  from:" << from << "  to:" << to 
               << "  numPatches: " << numPatches 
               << "   _patchExtents: " 
               << _patchExtents[0] << ", " 
               << _patchExtents[1] << ", " 
               << _patchExtents[2] << ", " 
               << _patchExtents[3] 
               << ", fullImageExtents[2]: " << fullImageExtents[2] 
               << ", numRegions: " <<  numRegions 
               << ", totalSendBufferSize: " << totalSendBufferSize << endl;
        for (int j=from; j<=to; j++) numPatchesPerRegion[j]++;
        for (int partition=from; partition<=to; partition++)
        {
            int _extentsYStart = std::max( _patchExtents[2], getScreenRegionStart(partition, fullImageExtents[2], fullImageExtents[3]) );
            int _extentsYEnd   = std::min( _patchExtents[3], getScreenRegionEnd(  partition, fullImageExtents[2], fullImageExtents[3]) );
            int _area = (_extentsYEnd-_extentsYStart)*(_patchExtents[1]-_patchExtents[0]);
            areaPerRegion[partition] += _area;
            totalSendBufferSize += _area;
            debug5 << "_patchExtents[2]: " << _patchExtents[2] << ", region start: " << getScreenRegionStart(partition, fullImageExtents[2], fullImageExtents[3]) <<  ", _extentsYStart: " << _extentsYStart<< endl;
            debug5 << "_patchExtents[3]: " << _patchExtents[3] << ", region end: " << getScreenRegionEnd(partition, fullImageExtents[2], fullImageExtents[3]) << ", _extentsYEnd: " << _extentsYEnd << endl;
            debug5 << "_area " << _area << endl;
            extentsPerPartiton[partition].push_back(i);
            extentsPerPartiton[partition].push_back(_patchExtents[0]);
            extentsPerPartiton[partition].push_back(_patchExtents[1]);
            extentsPerPartiton[partition].push_back(_extentsYStart);
            extentsPerPartiton[partition].push_back(_extentsYEnd);
            extentsPerPartiton[partition].push_back(temp.eye_z);
            numOfRegions.insert(partition);
        }
    }
    totalSendBufferSize *= 4;                           // to account for RGBA
    int numRegionsWithData = numOfRegions.size();
    debug5 << "\nParallel Direct Send ~ creating buffers" << endl;
    //---------------------------------------------------------------------//
    slivr::CheckSectionStop("avtSLIVRImgCommunicator", 
                            "ParallelDirectSendManyPatches", timingDetail,
                            "Determine How Many Patches and Pixel to Send "
                            "to Each Region");
    //---------------------------------------------------------------------//


    //
    // Copy the Data for Each Region for Each Patch
    //
    //---------------------------------------------------------------------//
    slivr::CheckSectionStart("avtSLIVRImgCommunicator", 
                             "ParallelDirectSendManyPatches", timingDetail,
                             "Copy the Data for Each Region for Each Patch");
    //---------------------------------------------------------------------//
    // Create buffer
    float *sendDataBuffer = new float[totalSendBufferSize];     // contains all the data arranged by region
    int   *sendDataBufferSize = new int[numRegionsWithData]();
    int   *sendDataBufferOffsets = new int[numRegionsWithData]();
    int   *sendBuffer = new int[numRegions*2]();
    int regionWithDataCount = 0;
    int numRegionsToSend = 0;
    // Populate the buffer with data
    int dataSendBufferOffset = 0;
    for (int i=0; i<numRegions; i++)
    {
        int _dataSize = 0;
        debug5 << "Region: " << i << "  size: " << extentsPerPartiton[i].size() << std::endl;
        for (int j=0; j<extentsPerPartiton[i].size(); j+=6)
        {
            int _patchID = extentsPerPartiton[i][j + 0];
            const std::multimap<int, slivr::ImgData>::const_iterator it = imgDataHashMap.find( _patchID );

            int _width = (extentsPerPartiton[i][j+2] - extentsPerPartiton[i][j+1]);
            int _bufferSize = _width * (extentsPerPartiton[i][j+4] - extentsPerPartiton[i][j+3]) * 4;
            int _dataOffset = extentsPerPartiton[i][j+3] - imageMetaPatchVector[_patchID].screen_ll[1];

            memcpy(&sendDataBuffer[dataSendBufferOffset], &(((*it).second).imagePatch[_width * _dataOffset * 4]), _bufferSize*sizeof(float) );

            dataSendBufferOffset += _bufferSize;
            _dataSize += _bufferSize;
        }

        if (_dataSize != 0){
            sendDataBufferSize[regionWithDataCount] = _dataSize;

            regionWithDataCount ++;
            if (regionWithDataCount != numRegionsWithData)
                sendDataBufferOffsets[regionWithDataCount] = sendDataBufferOffsets[regionWithDataCount-1] + sendDataBufferSize[regionWithDataCount-1];

            if (regionVector[i] != mpiRank)
                numRegionsToSend++;
        }

        sendBuffer[i*2+0] = numPatchesPerRegion[i];
        sendBuffer[i*2+1] = areaPerRegion[i];

        debug5 << "Region: " << i << "  numPatchesPerRegion: " << sendBuffer[i*2+0] << ", sendBuffer[i*2+1]: " << sendBuffer[i*2+1] << std::endl;
    }
    //---------------------------------------------------------------------//
    slivr::CheckSectionStop("avtSLIVRImgCommunicator", 
                            "ParallelDirectSendManyPatches", timingDetail,
                            "Copy the Data for Each Region for Each Patch");
    //---------------------------------------------------------------------//


    //
    // Exchange Information about Size to Recv
    //
    //---------------------------------------------------------------------//
    slivr::CheckSectionStart("avtSLIVRImgCommunicator", 
                             "ParallelDirectSendManyPatches", timingDetail,
                             "Exchange Information about Size to Recv");
    //---------------------------------------------------------------------//
    int *recvInfoATABuffer = new int[numRegions*2]();
    MPI_Alltoall(sendBuffer, 2, MPI_INT,  recvInfoATABuffer, 2, MPI_INT, MPI_COMM_WORLD);
    delete []sendBuffer;
    sendBuffer = NULL;
    //---------------------------------------------------------------------//
    slivr::CheckSectionStop("avtSLIVRImgCommunicator", 
                            "ParallelDirectSendManyPatches", timingDetail,
                            "Exchange Information about Size to Recv");
    debug5 << "Parallel Direct Send ~ Exchange information about size to recv" << endl;
    //---------------------------------------------------------------------//


    //
    // Calculate Buffer Size Needed
    //
    //---------------------------------------------------------------------//
    slivr::CheckSectionStart("avtSLIVRImgCommunicator", 
                             "ParallelDirectSendManyPatches", timingDetail,
                             "Calculate Buffer Size Needed");
    //---------------------------------------------------------------------//
    int infoBufferSize = 0;
    int dataBufferSize = 0;
    int numRegionsToRecvFrom = 0;
    for (int i=0; i<numRegions; i++)
    {
        infoBufferSize += recvInfoATABuffer[i*2 + 0];   // number of patches per region
        dataBufferSize += recvInfoATABuffer[i*2 + 1];   // area per region
        debug5 << "From: " << i << ", #patches: " << recvInfoATABuffer[i*2 + 0] << ", " << recvInfoATABuffer[i*2 + 1] << std::endl;
        if (i == mpiRank) continue;
        if (recvInfoATABuffer[i*2 + 0] != 0)
            numRegionsToRecvFrom++;
    }
    //---------------------------------------------------------------------//
    slivr::CheckSectionStop("avtSLIVRImgCommunicator", 
                            "ParallelDirectSendManyPatches", timingDetail,
                            "Calculate Buffer Size Needed");
    //---------------------------------------------------------------------//


    //
    // Create Structure for MPI Async send/recv
    //
    // Send
    MPI_Request *sendMetaRq = new MPI_Request[ numRegionsToSend ];
    MPI_Status *sendMetaSt = new MPI_Status[ numRegionsToSend ];
    MPI_Request *sendImageRq = new MPI_Request[ numRegionsToSend ];
    MPI_Status *sendImageSt = new MPI_Status[ numRegionsToSend ];
    // Recv
    MPI_Request *recvMetaRq = NULL;
    MPI_Status *recvMetaSt = NULL;
    MPI_Request *recvImageRq = NULL;
    MPI_Status *recvImageSt = NULL;
    // counters
    int recvInfoCount = 0;
    int offsetMeta = 0;
    int offsetData = 0;


    //
    // Create Recv Buffers
    //
    //---------------------------------------------------------------------//
    slivr::CheckSectionStart("avtSLIVRImgCommunicator", 
                             "ParallelDirectSendManyPatches", timingDetail,
                             "Create Recv Buffers");
    //---------------------------------------------------------------------//
    float *recvInfoBuffer = new float[infoBufferSize*6];  // 6 - passing 6 parameters for each patch
    float *recvDataBuffer =  new float[dataBufferSize*4]; // 4 - to account for RGBA

    debug5 << "infoBufferSize: " << infoBufferSize << ", dataBufferSize: " << dataBufferSize << std::endl;
    if (myRegionHeight != 0)
    {
        // Recv
        recvMetaRq = new MPI_Request[ numRegionsToRecvFrom ];
        recvMetaSt = new MPI_Status[ numRegionsToRecvFrom ];

        recvImageRq = new MPI_Request[ numRegionsToRecvFrom ];
        recvImageSt = new MPI_Status[ numRegionsToRecvFrom ];

        // Async Recv for info
        for (int i=0; i<numRegions; i++)
        {
            if (recvInfoATABuffer[i*2 + 0] == 0)
                continue;

            if ( regionVector[i] == mpiRank )
                continue;

            int src = regionVector[i];
            MPI_Irecv(&recvInfoBuffer[offsetMeta], recvInfoATABuffer[i*2 + 0]*6, MPI_FLOAT, src, tags[0], MPI_COMM_WORLD,  &recvMetaRq[recvInfoCount] );
            MPI_Irecv(&recvDataBuffer[offsetData], recvInfoATABuffer[i*2 + 1]*4, MPI_FLOAT, src, tags[1], MPI_COMM_WORLD,  &recvImageRq[recvInfoCount] );

            offsetMeta += recvInfoATABuffer[i*2 + 0]*6;
            offsetData += recvInfoATABuffer[i*2 + 1]*4;
            recvInfoCount++;
        }
        debug5 << "Async recv setup - numRegionsToRecvFrom: " << numRegionsToRecvFrom << "   recvInfoCount: " << recvInfoCount << endl;
    }
    //---------------------------------------------------------------------//
    slivr::CheckSectionStop("avtSLIVRImgCommunicator", 
                            "ParallelDirectSendManyPatches", timingDetail,
                            "Create Recv Buffers");
    //---------------------------------------------------------------------//


    //
    // Async Send
    //
    //---------------------------------------------------------------------//
    slivr::CheckSectionStart("avtSLIVRImgCommunicator", 
                             "ParallelDirectSendManyPatches", timingDetail,
                             "Async Send");
    //---------------------------------------------------------------------//
    int offset = 0;
    int sendCount = 0;
    int mpiSendCount = 0;
    for (int i=0; i<numRegions; i++)
    {
        if ( extentsPerPartiton[i].size() != 0 )
        {
            if ( regionVector[i] == mpiRank )
            {
                memcpy( &recvInfoBuffer[offsetMeta], &extentsPerPartiton[i][0], extentsPerPartiton[i].size()*sizeof(float) );
                memcpy( &recvDataBuffer[offsetData], &sendDataBuffer[offset],   sendDataBufferSize[ sendCount ]*sizeof(float) );

                offset += sendDataBufferSize[sendCount];
                sendCount++;
            }
            else
            {
                MPI_Isend(&extentsPerPartiton[i][0],  extentsPerPartiton[i].size(),  MPI_FLOAT, region[i], tags[0], MPI_COMM_WORLD, &sendMetaRq[mpiSendCount]);
                MPI_Isend(&sendDataBuffer[offset], sendDataBufferSize[ sendCount ], MPI_FLOAT, region[i], tags[1], MPI_COMM_WORLD, &sendImageRq[mpiSendCount]);

                offset += sendDataBufferSize[sendCount];
                sendCount++;
                mpiSendCount++;
            }
        }
    }

    debug5 << "Asyn send setup done ~ numRegionsToSend: " << numRegionsToSend << "  mpiSendCount: " << mpiSendCount << endl;

    if (myRegionHeight != 0)
    {
        debug5 << "MPI_Waitall ..." << std::endl;
        MPI_Waitall(recvInfoCount, recvImageRq, recvImageSt);   // Means that we have reveived everything!

        debug5 << "MAPI_WAITALL done!" << std::endl;

        if (recvInfoATABuffer != NULL)
            delete []recvInfoATABuffer;
        recvInfoATABuffer = NULL;

        debug5 << "Sorting..." << std::endl;

        //
        // Sort the data
        std::multimap<float,int> patchData;
        std::vector<int> patchOffset;
        patchOffset.push_back(0);
        for (int i=0; i<infoBufferSize; i++)
        {
            patchData.insert( std::pair<float,int> (recvInfoBuffer[i*6 + 5],i));
            int _patchSize = (recvInfoBuffer[i*6 + 4]-recvInfoBuffer[i*6 + 3]) * (recvInfoBuffer[i*6 + 2]-recvInfoBuffer[i*6 + 1]) * 4;
            int _offset = patchOffset[i] + _patchSize;

            if (i != infoBufferSize-1)
                patchOffset.push_back(_offset);
        }

        //
        // Create buffer for current region
        intermediateImageBBox[0] = intermediateImageExtents[0] = fullImageExtents[0];
        intermediateImageBBox[1] = intermediateImageExtents[1] = fullImageExtents[1];
        intermediateImageBBox[2] = intermediateImageExtents[2] = myStartingHeight;
        intermediateImageBBox[3] = intermediateImageExtents[3] = myEndingHeight;
        intermediateImage = new float[width * (myEndingHeight - myStartingHeight) * 4]();
        debug5 << "intermediate image size " << width << ", " << (myEndingHeight - myStartingHeight) << std::endl;

        //
        // Blend
        int numBlends = 0;
        for (std::multimap<float,int>::iterator it=patchData.begin(); it!=patchData.end(); ++it)
        {
            int _id = (*it).second;
            int _extents[4];
            _extents[0] = recvInfoBuffer[_id*6 + 1];
            _extents[1] = recvInfoBuffer[_id*6 + 2];
            _extents[2] = recvInfoBuffer[_id*6 + 3];
            _extents[3] = recvInfoBuffer[_id*6 + 4];
            BlendFrontToBack(&recvDataBuffer[patchOffset[_id]], _extents, _extents, intermediateImage, intermediateImageExtents);
            numBlends++;
        }

        if (numBlends == 0) {
            intermediateImageBBox[0]=intermediateImageBBox[1]=intermediateImageBBox[2]=intermediateImageBBox[3] = 0;
        }
    }

    MPI_Waitall(numRegionsToSend, sendImageRq, sendImageSt);   // Means that we have sent everything!

    if (myRegionHeight == 0) compositingDone = true;

    //---------------------------------------------------------------------//
    slivr::CheckSectionStop("avtSLIVRImgCommunicator", 
                            "ParallelDirectSendManyPatches", timingDetail,
                            "Async Send");
    //---------------------------------------------------------------------//


    //
    // Cleanup
    //
    //---------------------------------------------------------------------//
    slivr::CheckSectionStart("avtSLIVRImgCommunicator", 
                             "ParallelDirectSendManyPatches", timingDetail,
                             "Cleanup");
    //---------------------------------------------------------------------//
    if (sendDataBuffer != NULL)
        delete []sendDataBuffer;
    sendDataBuffer = NULL;

    if (sendDataBufferSize != NULL)
        delete []sendDataBufferSize;
    sendDataBufferSize = NULL;

    if (sendDataBufferOffsets != NULL)
        delete []sendDataBufferOffsets;
    sendDataBufferOffsets = NULL;


    if (sendMetaRq != NULL)
        delete []sendMetaRq;

    if (sendImageRq != NULL)
        delete []sendImageRq;

    if (sendMetaSt != NULL)
        delete []sendMetaSt;

    if (sendImageSt != NULL)
        delete []sendImageSt;

    sendMetaRq = NULL;
    sendImageRq = NULL;
    sendMetaSt = NULL;
    sendImageSt = NULL;

    if (myRegionHeight != 0)
    {
        if (recvInfoBuffer != NULL)
            delete []recvInfoBuffer;
        recvInfoBuffer = NULL;

        if (recvDataBuffer != NULL)
            delete []recvDataBuffer;
        recvDataBuffer = NULL;

        if (recvMetaRq != NULL)
            delete []recvMetaRq;

        if (recvMetaSt != NULL)
            delete []recvMetaSt;

        if (recvImageRq != NULL)
            delete []recvImageRq;

        if (recvImageSt != NULL)
            delete []recvImageSt;

        recvMetaRq = NULL;
        recvImageRq = NULL;
        recvMetaSt = NULL;
        recvImageSt = NULL;
    }
    //---------------------------------------------------------------------//
    slivr::CheckSectionStop("avtSLIVRImgCommunicator", 
                            "ParallelDirectSendManyPatches", timingDetail,
                            "Cleanup");
    debug5 << "All Parallel Direct Send is Done" << std::endl;
    //---------------------------------------------------------------------//
#endif
    return myRegionHeight;
}

// ***************************************************************************
//  Method: avtSLIVRImgCommunicator::gatherImages
//
//  Purpose:
//      Gather images from Parallel Direct Send
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
// **************************************************************************
void
avtSLIVRImgCommunicator::gatherImages(int regionGather[], int totalNumRanks, float * inputImg, int imgExtents[4], int boundingBox[4], int tag, int fullImageExtents[4], int myRegionHeight)
{
#ifdef PARALLEL
    debug5 << "gatherImages starting... totalNumRanks: " << totalNumRanks << ", compositingDone: " << compositingDone
           << ", imgExtents: " << imgExtents[0] << ", " << imgExtents[1] << ", " << imgExtents[2] << ", " << imgExtents[3] << std::endl;

    for (int i=0; i<4; i++)
        finalImageExtents[i] = finalBB[i] = 0;

    if (mpiRank == 0)
    {
        int width =  fullImageExtents[1]-fullImageExtents[0];
        int height = fullImageExtents[3]-fullImageExtents[2];

        debug5 << "Gather Images at 0, final size: " << fullImageExtents[1]-fullImageExtents[0] << " x " << fullImageExtents[3]-fullImageExtents[2] << std::endl;

        //
        // Receive at root/display node!
        finalImage = new float[width*height*4];
        finalImageExtents[0] = fullImageExtents[0];
        finalImageExtents[1] = fullImageExtents[1];
        finalImageExtents[2] = fullImageExtents[2];
        finalImageExtents[3] = fullImageExtents[3];

        int numRegionsWithData = 0;
        int numToRecv = 0;
        for (int i=0; i<totalNumRanks; i++)
        {
            if (getRegionSize(i) != 0)
                numRegionsWithData++;
        }
        numToRecv = numRegionsWithData;

        // remove itself from the recv
        if (getRegionSize(mpiRank) != 0) 
            numToRecv--;


        //
        // Create buffers for async reciving
        MPI_Request *recvImageRq = new MPI_Request[ numToRecv ];
        MPI_Status  *recvImageSt = new MPI_Status[ numToRecv ];

        int lastBufferSize    = getRegionSize(totalNumRanks-1) * width * 4;
        int regularBufferSize = regularRegionSize * width * 4;

        debug5 << "numToRecv: " << numToRecv << ", numRegionsWithData: " << numRegionsWithData << std::endl;
        debug5 << "regularBufferSize: " << regularBufferSize << ", lastBufferSize: " << lastBufferSize << std::endl;

        // Async Recv
        int recvCount=0;
        for (int i=0; i<numRegionsWithData; i++)
        {
            int src = regionGather[i];

            if (src == mpiRank)
                continue;

            if (i == totalNumRanks-1)
            {
                if (lastBufferSize != 0)
                {
                    MPI_Irecv(&finalImage[i*regularBufferSize], lastBufferSize,     MPI_FLOAT, src, tag, MPI_COMM_WORLD,  &recvImageRq[recvCount] );
                }
            }
            else
                MPI_Irecv(&finalImage[i*regularBufferSize], regularBufferSize,  MPI_FLOAT, src, tag, MPI_COMM_WORLD,  &recvImageRq[recvCount] );
                        

            debug5 << i << " ~ recvCount: " << recvCount << std::endl;
            recvCount++;
        }

        if (compositingDone == false)   // If root has data for the final image
            PlaceImage(inputImg, imgExtents, finalImage, finalImageExtents);

        MPI_Waitall(numToRecv, recvImageRq, recvImageSt);
        compositingDone = true;

        delete []recvImageRq;
        recvImageRq = NULL;
        delete []recvImageSt;
        recvImageSt = NULL;
    }
    else
    {
        if (compositingDone == false)   
        {
            int imgSize = (imgExtents[1]-imgExtents[0]) * (imgExtents[3]-imgExtents[2]) * 4;
            debug5 << "imgSize: " << imgSize << std::endl;

            MPI_Send(inputImg, imgSize, MPI_FLOAT, 0, tag, MPI_COMM_WORLD);
            compositingDone = true;
        }
    }

#endif
}

// ***************************************************************************
//  Method: avtSLIVRImgCommunicator::getcompositedImage
//
//  Purpose:
//      Returns the whole image if needed
//
//  Programmer: Pascal Grosset
//  Creation: July 2013
//
//  Modifications:
//
// ***************************************************************************

void avtSLIVRImgCommunicator::getcompositedImage
(int imgBufferWidth, int imgBufferHeight, unsigned char *wholeImage)
{
    for (int i=0; i< imgBufferHeight; i++) {
        for (int j=0; j<imgBufferWidth; j++) {
            int bufferIndex = (imgBufferWidth*4*i) + (j*4);
            int wholeImgIndex = (imgBufferWidth*3*i) + (j*3);
            wholeImage[wholeImgIndex+0] = (finalImage[bufferIndex+0] ) * 255;
            wholeImage[wholeImgIndex+1] = (finalImage[bufferIndex+1] ) * 255;
            wholeImage[wholeImgIndex+2] = (finalImage[bufferIndex+2] ) * 255;
        }
    }
    if (finalImage != NULL)
    { delete []finalImage; }
    finalImage = NULL;
}
