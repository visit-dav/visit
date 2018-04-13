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

// ************************************************************************* //
//                          avtImgCommunicator.h                             //
// ************************************************************************* //

#ifndef AVT_IMG_COMMUNICATOR_H
#define AVT_IMG_COMMUNICATOR_H

#include <filters_exports.h>
#include <pipeline_exports.h>
#include <imgMetaData.h>
#include <avtSamplePointExtractor.h>
#include <algorithm>
#include <string>

#ifdef PARALLEL
#   include <mpi.h>
#endif

#define MSG_DATA 100
#define MSG_RESULT 101

const int SEND = 1;
const int RECEIVE = 2;

struct imageBuffer{
    float *image;
    float depth;
};

// ****************************************************************************
//  Class: avtRayTracer
//
//  Purpose:
//      Does the composition for Ray casting: SLIVR
//
//  Programmer: Pascal Grosset
//  Creation:   Spetember 20, 2013
//
// ****************************************************************************

class avtImgCommunicator
{
    int num_procs;
    int my_id;
    int totalPatches;
    bool compositingDone;

    // image sizing for compositing
    int maxRegionHeight;
    int regularRegionSize;
    std::vector<int>regionRankExtents;

    void placeInImage(float * srcImage, int srcExtents[4], float *& dstImage, int dstExtents[4]);
    void colorImage(float *& srcImage, int widthSrc, int heightSrc, float _color[4]);
    void updateBoundingBox(int currentBoundingBox[4], int imageExtents[4]);

    void gatherDepthAtRoot(int numlocalPatches, float *localPatchesDepth, int &totalPatches, int *& patchCountPerRank, float *& allPatchesDepth);
    void blendWithBackground(float *_image, int extents[4], float backgroundColor[4]);

    void blendFrontToBack(float * srcImage, int srcExtents[4], float *& dstImage, int dstExtents[4]);
    void blendBackToFront(float * srcImage, int srcExtents[4], float *& dstImage, int dstExtents[4]);

    void blendFrontToBack(float * srcImage, int srcExtents[4], int blendExtents[4], float *& dstImage, int dstExtents[4]);
    void blendBackToFront(float * srcImage, int srcExtents[4], int blendExtents[4], float *& dstImage, int dstExtents[4]);

    

    void computeRegionExtents(int numRanks, int height);
    
    int getRegularRegionSize(){ return regularRegionSize; } 
    int getRegionStart(int region){ return regionRankExtents[region*3+0]; }
    int getRegionEnd(int region){ return regionRankExtents[region*3+1]; }
    int getRegionSize(int region){ return regionRankExtents[region*3+2]; }
    int getMaxRegionHeight(){ return maxRegionHeight; }
    
    int getScreenRegionStart(int region, int screenImgMinY, int screenImgMaxY){
        return clamp( getRegionStart(region)+screenImgMinY, screenImgMinY, screenImgMaxY); 
    }
    int getScreenRegionEnd(int region, int screenImgMinY, int screenImgMaxY){
        return clamp( getRegionEnd(region)+screenImgMinY, screenImgMinY, screenImgMaxY); 
    }

    

public:
    float *imgBuffer;                   // Final image is here
    int finalImageExtents[4];
    int finalBB[4];

    float *intermediateImage;           // Intermediate image, e.g. in parallel direct send
    int intermediateImageExtents[4];
    int intermediateImageBB[4];


    avtImgCommunicator();
    ~avtImgCommunicator();

    virtual const char *GetType(void) { return "avtImgCommunicator"; };
    virtual const char *GetDescription(void) { return "Doing compositing for ray casting SLIVR";};

    int clamp(int value, int _min, int _max){ return std::max( std::min(value,_max), _min); }
    float clamp(float x){ return std::min( std::max(x, 0.0f), 1.0f); }
    
    void barrier();

    int GetNumProcs(){ return num_procs;};
    int GetMyId(){ return my_id;};

    void getcompositedImage(int imgBufferWidth, int imgBufferHeight, unsigned char *wholeImage);  // get the final composited image
    void regionAllocation(int numMPIRanks, int *& regions);

    int findRegionsForPatch(int patchExtents[4], int screenProjectedExtents[4], int numRegions, int &from, int &to);

    // Both currently unused but good for simple testing
    void serialDirectSend(int numPatches, float *localPatchesDepth, int *extents, float *imgData, float backgroundColor[4], int width, int height);
    void parallelDirectSend(float *imgData, int imgExtents[4], int region[], int numRegions, int tags[2], int fullImageExtents[4]);
    
    int parallelDirectSendManyPatches(std::multimap<int, imgData> imgDataHashMap, std::vector<imgMetaData> imageMetaPatchVector, int numPatches, int region[], int numRegions, int tags[2], int fullImageExtents[4]);
    void gatherImages(int regionGather[], int numToRecv, float * inputImg, int imgExtents[4], int boundingBox[4], int tag, int fullImageExtents[4], int myRegionHeight);
};


#endif
