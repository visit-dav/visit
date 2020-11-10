// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtSLIVRImageCompositor.C                         //
// ************************************************************************* //

#include <avtParallel.h>
#include <avtSLIVRImageCompositor.h>
#include <ImproperUseException.h>
#include <DebugStream.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <set>


enum blendDirection {FRONT_TO_BACK = 0, BACK_TO_FRONT = 1};

// ****************************************************************************
//  Method: avtSLIVRImageCompositor::avtSLIVRImageCompositor
//
//  Purpose: Constructor
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
// ****************************************************************************
avtSLIVRImageCompositor::avtSLIVRImageCompositor()
{
  #ifdef PARALLEL
    MPI_Comm_size(VISIT_MPI_COMM, &num_procs);
    MPI_Comm_rank(VISIT_MPI_COMM, &my_id);

  #else
    num_procs = 1;
    my_id = 0;
  #endif

    totalPatches = 0;

    intermediateImage = NULL;
    imgBuffer = NULL;

    for (int i=0; i<4; i++)
    {
        intermediateImageExtents[i] = 0;
        intermediateImageBB[i] = 0;
    }
}



// ****************************************************************************
//  Method: avtSLIVRImageCompositor::~avtSLIVRImageCompositor
//
//  Purpose:
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
// ****************************************************************************
avtSLIVRImageCompositor::~avtSLIVRImageCompositor()
{
    if (my_id == 0)
    {
        if (imgBuffer != NULL)
            delete []imgBuffer;
    }
}



// ****************************************************************************
//  Method: avtSLIVRImageCompositor::
//
//  Purpose:
//    Barrier, useful for debugging
//
//  Programmer: Pascal Grosset
//  Creation: July 2013
//
//  Modifications:
//
// ****************************************************************************
void avtSLIVRImageCompositor::barrier(){
  #ifdef PARALLEL
    MPI_Barrier( MPI_COMM_WORLD );
  #endif
}



// ****************************************************************************
//  Method: avtSLIVRImageCompositor::getcompositedImage
//
//  Purpose:
//      Returns the whole image if needed
//
//  Programmer: Pascal Grosset
//  Creation: July 2013
//
//  Modifications:
//
// ****************************************************************************
void avtSLIVRImageCompositor::getcompositedImage(int imgBufferWidth, int imgBufferHeight, unsigned char *wholeImage)
{
    for (int i=0; i< imgBufferHeight; i++)
        for (int j=0; j<imgBufferWidth; j++){
            int bufferIndex = (imgBufferWidth*4*i) + (j*4);
            int wholeImgIndex = (imgBufferWidth*3*i) + (j*3);

            wholeImage[wholeImgIndex+0] = (imgBuffer[bufferIndex+0] ) * 255;
            wholeImage[wholeImgIndex+1] = (imgBuffer[bufferIndex+1] ) * 255;
            wholeImage[wholeImgIndex+2] = (imgBuffer[bufferIndex+2] ) * 255;
        }

    if (imgBuffer != NULL)
        delete []imgBuffer;
    imgBuffer = NULL;
}



// ****************************************************************************
//  Method: avtSLIVRImageCompositor::
//
//  Purpose:
//       Fills an image with a specific color
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
// ****************************************************************************
void
avtSLIVRImageCompositor::colorImage(float *& srcImage, int widthSrc, int heightSrc, float _color[4])
{
    for (int _y=0; _y<heightSrc; _y++)
        for (int _x=0; _x<widthSrc; _x++)
        {
            int srcIndex = widthSrc*_y*4 + _x*4;

            srcImage[srcIndex+0] = _color[0];
            srcImage[srcIndex+1] = _color[1];
            srcImage[srcIndex+2] = _color[2];
            srcImage[srcIndex+3] = _color[3];
        }
}



// ****************************************************************************
//  Method: avtSLIVRImageCompositor::placeInImage
//
//  Purpose:
//      Puts srcImage into dstImage
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
// ****************************************************************************
void
avtSLIVRImageCompositor::placeInImage(float * srcImage, int srcExtents[4], float *& dstImage, int dstExtents[4])
{
    int widthSrc, heightSrc, widthDst;
    widthSrc  = srcExtents[1] - srcExtents[0];
    heightSrc = srcExtents[3] - srcExtents[2];

    widthDst  = dstExtents[1] - dstExtents[0];

    for (int _y=0; _y<heightSrc; _y++)
        for (int _x=0; _x<widthSrc; _x++)
        {
            int startingX = srcExtents[0];
            int startingY = srcExtents[2];

            int srcIndex = widthSrc*_y*4 + _x*4;                                                                  // index in the subimage
            int dstIndex = ( (startingY+_y - dstExtents[2])*widthDst*4  + (startingX+_x - dstExtents[0])*4 );     // index in the big buffer

            dstImage[dstIndex+0] = srcImage[srcIndex+0];
            dstImage[dstIndex+1] = srcImage[srcIndex+1];
            dstImage[dstIndex+2] = srcImage[srcIndex+2];
            dstImage[dstIndex+3] = srcImage[srcIndex+3];
        }
}



// ****************************************************************************
//  Method: avtSLIVRImageCompositor::blendWithBackground
//
//  Purpose:
//      Blends _image with the backgroundColor
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
// **************************************************************************
void
avtSLIVRImageCompositor::blendWithBackground(float *_image, int extents[4], float backgroundColor[4])
{
    int numPixels = (extents[3]-extents[2]) * (extents[1]-extents[0]);

    for (int index=0; index<numPixels; index++)      // estimated potential speedup: 2.240
    {
        int indexSrc = index*4;
        float alpha = (1.0 - _image[indexSrc+3]);

        _image[indexSrc+0] = backgroundColor[0] * alpha +  _image[indexSrc+0];
        _image[indexSrc+1] = backgroundColor[1] * alpha +  _image[indexSrc+1];
        _image[indexSrc+2] = backgroundColor[2] * alpha +  _image[indexSrc+2];
        _image[indexSrc+3] = backgroundColor[3] * alpha +  _image[indexSrc+3];
    }
}



// ****************************************************************************
//  Method: avtSLIVRImageCompositor::
//
//  Purpose:
//      Blends tow patches in a front to back manner
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
// **************************************************************************
void
avtSLIVRImageCompositor::blendFrontToBack(float * srcImage, int srcExtents[4], int blendExtents[4], float *& dstImage, int dstExtents[4])
{
    int widthSrc, heightSrc, widthDst;
    widthSrc  = srcExtents[1] - srcExtents[0];
    heightSrc = srcExtents[3] - srcExtents[2];

    widthDst  = dstExtents[1] - dstExtents[0];

    #if defined(VISIT_THREADS)
    #endif

    for (int _y=blendExtents[2]; _y<blendExtents[3]; _y++)
        for (int _x=blendExtents[0]; _x<blendExtents[1]; _x++)
        {

            int srcIndex = (_y-srcExtents[2]) * widthSrc * 4 + (_x-srcExtents[0]) * 4;
            int dstIndex = (_y-dstExtents[2]) * widthDst * 4 + (_x-dstExtents[0]) * 4;

            // back to Front compositing: composited_i = composited_i-1 * (1.0 - alpha_i) + incoming; alpha = alpha_i-1 * (1- alpha_i)
            float alpha = 1.0 - dstImage[dstIndex+3];
            dstImage[dstIndex+0] = clamp( (srcImage[srcIndex+0] * alpha) + dstImage[dstIndex+0] );
            dstImage[dstIndex+1] = clamp( (srcImage[srcIndex+1] * alpha) + dstImage[dstIndex+1] );
            dstImage[dstIndex+2] = clamp( (srcImage[srcIndex+2] * alpha) + dstImage[dstIndex+2] );
            dstImage[dstIndex+3] = clamp( (srcImage[srcIndex+3] * alpha) + dstImage[dstIndex+3] );
        }
}



// ****************************************************************************
//  Method: avtSLIVRImageCompositor::
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
avtSLIVRImageCompositor::blendBackToFront(float * srcImage, int srcExtents[4], int blendExtents[4], float *& dstImage, int dstExtents[4])
{
    int widthSrc, heightSrc, widthDst;
    widthSrc  = srcExtents[1] - srcExtents[0];
    heightSrc = srcExtents[3] - srcExtents[2];

    widthDst  = dstExtents[1] - dstExtents[0];

    for (int _y=blendExtents[2]; _y<blendExtents[3]; _y++)
        for (int _x=blendExtents[0]; _x<blendExtents[1]; _x++)
        {

            int srcIndex = (_y-srcExtents[2]) * widthSrc * 4 + (_x-srcExtents[0]) * 4;
            int dstIndex = (_y-dstExtents[2]) * widthDst * 4 + (_x-dstExtents[0]) * 4;

            // back to Front compositing: composited_i = composited_i-1 * (1.0 - alpha_i) + incoming; alpha = alpha_i-1 * (1- alpha_i)
            float alpha = 1.0 - srcImage[srcIndex+3];
            dstImage[dstIndex+0] = clamp( (dstImage[dstIndex+0] * alpha) + srcImage[srcIndex+0] );
            dstImage[dstIndex+1] = clamp( (dstImage[dstIndex+1] * alpha) + srcImage[srcIndex+1] );
            dstImage[dstIndex+2] = clamp( (dstImage[dstIndex+2] * alpha) + srcImage[srcIndex+2] );
            dstImage[dstIndex+3] = clamp( (dstImage[dstIndex+3] * alpha) + srcImage[srcIndex+3] );
        }
}

// ****************************************************************************
//  Method: avtSLIVRImageCompositor::
//
//  Purpose:
//      Blends tow patches in a front to back manner
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
// **************************************************************************
void
avtSLIVRImageCompositor::blendFrontToBack(float * srcImage, int srcExtents[4], float *& dstImage, int dstExtents[4])
{
    int widthSrc, heightSrc, widthDst;
    widthSrc  = srcExtents[1] - srcExtents[0];
    heightSrc = srcExtents[3] - srcExtents[2];

    widthDst  = dstExtents[1] - dstExtents[0];

    for (int _y=0; _y<heightSrc; _y++)
        for (int _x=0; _x<widthSrc; _x++)
        {
            int startingX = srcExtents[0];
            int startingY = srcExtents[2];

            if ((startingX + _x) > dstExtents[1])
                continue;

            if ((startingY + _y) > dstExtents[3])
                continue;

            int srcIndex = widthSrc*_y*4 + _x*4;                                                                  // index in the subimage
            int dstIndex = ( (startingY+_y - dstExtents[2])*widthDst*4  + (startingX+_x - dstExtents[0])*4 );     // index in the big buffer

            // back to Front compositing: composited_i = composited_i-1 * (1.0 - alpha_i) + incoming; alpha = alpha_i-1 * (1- alpha_i)
            float alpha = 1.0 - dstImage[dstIndex+3];
            dstImage[dstIndex+0] = clamp( (srcImage[srcIndex+0] * alpha) + dstImage[dstIndex+0] );
            dstImage[dstIndex+1] = clamp( (srcImage[srcIndex+1] * alpha) + dstImage[dstIndex+1] );
            dstImage[dstIndex+2] = clamp( (srcImage[srcIndex+2] * alpha) + dstImage[dstIndex+2] );
            dstImage[dstIndex+3] = clamp( (srcImage[srcIndex+3] * alpha) + dstImage[dstIndex+3] );
        }
}


// ****************************************************************************
//  Method: avtSLIVRImageCompositor::
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
avtSLIVRImageCompositor::blendBackToFront(float * srcImage, int srcExtents[4], float *& dstImage, int dstExtents[4])
{
    int widthSrc, heightSrc, widthDst;
    widthSrc  = srcExtents[1] - srcExtents[0];
    heightSrc = srcExtents[3] - srcExtents[2];

    widthDst  = dstExtents[1] - dstExtents[0];

    for (int _y=0; _y<heightSrc; _y++)
        for (int _x=0; _x<widthSrc; _x++)
        {
            int startingX = srcExtents[0];
            int startingY = srcExtents[2];

            if ((startingX + _x) > dstExtents[1])
                continue;

            if ((startingY + _y) > dstExtents[3])
                continue;

            int srcIndex = widthSrc*_y*4 + _x*4;                                                                  // index in the subimage
            int dstIndex = ( (startingY+_y - dstExtents[2])*widthDst*4  + (startingX+_x - dstExtents[0])*4 );     // index in the big buffer

            // back to Front compositing: composited_i = composited_i-1 * (1.0 - alpha_i) + incoming; alpha = alpha_i-1 * (1- alpha_i)
            float alpha = 1.0 - srcImage[srcIndex+3];
            dstImage[dstIndex+0] = clamp( (dstImage[dstIndex+0] * alpha) + srcImage[srcIndex+0] );
            dstImage[dstIndex+1] = clamp( (dstImage[dstIndex+1] * alpha) + srcImage[srcIndex+1] );
            dstImage[dstIndex+2] = clamp( (dstImage[dstIndex+2] * alpha) + srcImage[srcIndex+2] );
            dstImage[dstIndex+3] = clamp( (dstImage[dstIndex+3] * alpha) + srcImage[srcIndex+3] );
        }
}

// ****************************************************************************
//  Method: avtSLIVRImageCompositor::regionAllocation
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
avtSLIVRImageCompositor::regionAllocation(int numMPIRanks, int *& regions)
{
    regions = new int[numMPIRanks];

    // Initial allocation: partition for section rank
    for (int i=0; i<numMPIRanks; i++)
        regions[i] = i;
}

// ****************************************************************************
//  Method: avtSLIVRImageCompositor::
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
avtSLIVRImageCompositor::updateBoundingBox(int currentBoundingBox[4], int imageExtents[4])
{
    if ( (currentBoundingBox[0] == 0 && currentBoundingBox[1] == 0) && (currentBoundingBox[2] == 0 && currentBoundingBox[3] == 0))
    {
        currentBoundingBox[0]=imageExtents[0];
        currentBoundingBox[1]=imageExtents[1];
        currentBoundingBox[2]=imageExtents[2];
        currentBoundingBox[3]=imageExtents[3];

        return;
    }

    if (imageExtents[0] < currentBoundingBox[0])
        currentBoundingBox[0] = imageExtents[0];

    if (imageExtents[2] < currentBoundingBox[2])
        currentBoundingBox[2] = imageExtents[2];

    if (imageExtents[1] > currentBoundingBox[1])
        currentBoundingBox[1] = imageExtents[1];

    if (imageExtents[3] > currentBoundingBox[3])
        currentBoundingBox[3] = imageExtents[3];
}


// ****************************************************************************
//  Method: avtSLIVRImageCompositor::gatherDepthAtRoot
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
avtSLIVRImageCompositor::gatherDepthAtRoot(int numlocalPatches, float *localPatchesDepth, int &totalPatches, int *& patchCountPerRank, float *& allPatchesDepth)
{
  #ifdef PARALLEL
    //
    // Get how many patches are coming from each MPI rank
    totalPatches = 0;
    int *patchesOffset = NULL;


    if (my_id == 0) // root!
        patchCountPerRank = new int[num_procs]();

    MPI_Gather(&numlocalPatches, 1, MPI_INT,   patchCountPerRank, 1, MPI_INT,    0, MPI_COMM_WORLD);


    //
    // Gather number of patch group
    if (my_id == 0)
    {
        patchesOffset = new int[num_procs]();
        patchesOffset[0] = 0;

        for (int i=0; i<num_procs; i++)
        {
            totalPatches += patchCountPerRank[i];

            if (i == 0)
                patchesOffset[i] = 0;
            else
                patchesOffset[i] = patchesOffset[i-1] + patchCountPerRank[i-1];
        }

        allPatchesDepth = new float[totalPatches];
    }

    MPI_Gatherv(localPatchesDepth, numlocalPatches, MPI_FLOAT,   allPatchesDepth, patchCountPerRank, patchesOffset,    MPI_FLOAT, 0, MPI_COMM_WORLD);

    //
    // Cleanup
    if (my_id == 0)
        if (patchesOffset != NULL)
            delete []patchesOffset;

    patchesOffset = NULL;
  #endif
}



// ****************************************************************************
//  Method: avtSLIVRImageCompositor::serialDirectSend
//
//  Purpose:
//      A very simple compositing that we can fall back to if Parallel direct send is buggy.
//      Works with convex patches though
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
// **************************************************************************
void
avtSLIVRImageCompositor::serialDirectSend(int numPatches, float *localPatchesDepth, int *extents, float *imgData, float backgroundColor[4], int width, int height)
{
  #ifdef PARALLEL
    //debug5 << "serialDirectSend" << std::endl;

    float *recvImage = NULL;

    int tags[2] = {5781, 5782};

    int totalPatches;
    int *patchCountPerRank = NULL;
    float *patchesDepth = NULL;
    gatherDepthAtRoot(numPatches, localPatchesDepth, totalPatches, patchCountPerRank, patchesDepth);

    if (my_id == 0)
    {
        //
        // Root
        int srcSize[2], srcPos[2], dstSize[2], dstPos[2];
        srcSize[0] = width;  srcSize[1] = height;
        srcPos[0] = 0;       srcPos[1] = 0;

        //
        // Sort patches we will receive
        std::multimap<float,int> depthRankPatches;

        int index = 0;
        for (int i=0; i<num_procs; i++)
            for (int j=0; j<patchCountPerRank[i]; j++)
            {
                depthRankPatches.insert( std::pair<float,int>(patchesDepth[index],i) );
                index++;
            }


        //
        // Create space for buffers
        int recvParams[4];                          // minX, maxX, minY, maxY
        int imgExtents[4];
        imgExtents[0] = 0;  imgExtents[1] = width;
        imgExtents[2] = 0;  imgExtents[3] = height;

        recvImage = new float[width*height*4]();
        imgBuffer = new float[width*height*4]();

        int localIndex = 0;

        //
        // Compositing
        for (std::multimap<float,int>::iterator it=depthRankPatches.begin(); it!=depthRankPatches.end(); ++it)
        {
            int rank = (*it).second;

            //debug5 << "\nRecv and blend from " << rank << " depth: " << (*it).first << std::endl;

            if (rank != my_id)
            {
                MPI_Recv(recvParams,             4, MPI_INT,   rank, tags[0],  MPI_COMM_WORLD, MPI_STATUS_IGNORE);  // recv image info
                MPI_Recv(recvImage, width*height*4, MPI_FLOAT, rank, tags[1],  MPI_COMM_WORLD, MPI_STATUS_IGNORE);  // recv image

                dstPos[0]  = dstPos[0];                      dstPos[1]  = dstPos[1];
                dstSize[0] = recvParams[2]-recvParams[0];    dstSize[1] = recvParams[3]-recvParams[1];
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

            blendFrontToBack(recvImage, recvParams, imgBuffer, imgExtents);
        }
        blendWithBackground(imgBuffer, imgExtents, backgroundColor);

        //writeArrayToPPM("/home/pascal/Desktop/debugImages/full_with back_" + toStr(__index), imgBuffer, imgExtents[1]-imgExtents[0], imgExtents[3]-imgExtents[2]);
    }
    else
    {
        //
        // Sender
        for (int i=0; i<numPatches; i++)
        {
            int imgSize = (extents[i*4 + 1] - extents[i*4 + 0]) * (extents[i*4 + 3] - extents[i*4 + 2]) * 4;

            if (imgSize > 0)
            {
                //debug5 << "Sending: Extents " <<  extents[i*4 + 0] << ", " << extents[i*4 + 1] << ", " << extents[i*4 + 2] << ", " << extents[i*4 + 3] << std::endl;
                //writeArrayToPPM("/home/pascal/Desktop/debugImages/sending_to_root_from_" + toStr(my_id), &imgData[i*(width*height*4)], (extents[i*4 + 1] - extents[i*4 + 0]), (extents[i*4 + 3] - extents[i*4 + 2]) );

                MPI_Send( &extents[i*4],                       4, MPI_INT,   0, tags[0], MPI_COMM_WORLD);
                MPI_Send( &imgData[i*(width*height*4)],  imgSize, MPI_FLOAT, 0, tags[1], MPI_COMM_WORLD);
            }
        }
    }

    //debug5 << "Free memory" << std::endl;

    //
    // Cleanup
    if (patchesDepth != NULL)
      delete []patchesDepth;

    if (patchCountPerRank != NULL)
        delete []patchCountPerRank;

    if (recvImage != NULL)
        delete []recvImage;

    recvImage = NULL;
    patchCountPerRank = NULL;
    patchesDepth = NULL;

  #endif
}



// ****************************************************************************
//  Method: avtSLIVRImageCompositor::parallelDirectSend
//
//  Purpose:
//      Parallel Direct Send rendering that can blend convex patches from each MPI rank.
//      However, since we are not guaranteed to have convex patches. It's not used.
//
//  Programmer: Pascal Grosset
//  Creation:   August 19, 2016
//
//  Modifications:
//
// **************************************************************************
void
avtSLIVRImageCompositor::parallelDirectSend(float *imgData, int imgExtents[4], int region[], int numRegions, int tags[2], int fullImageExtents[4])
{
  #ifdef PARALLEL
    //
    // Determine position in region (myPositionInRegion)
    int width =  fullImageExtents[1]-fullImageExtents[0];
    int height = fullImageExtents[3]-fullImageExtents[2];

    //debug5 << "fullImageExtents: " << fullImageExtents[0] << ", " << fullImageExtents[1] << "   " << fullImageExtents[2] << ", " << fullImageExtents[3] << endl;

    compositingDone = false;
    int myPositionInRegion = -1;
    bool inRegion = true;
    std::vector<int> regionVector(region, region+numRegions);
    std::vector<int>::iterator it = std::find(regionVector.begin(), regionVector.end(), my_id);

    if (it == regionVector.end())
    {
        inRegion = false;
        //debug5 << my_id << " ~ SHOULD NOT HAPPEN: Not found " << my_id <<  " !!!" << std::endl;
    }
    else
        myPositionInRegion = it - regionVector.begin();

    //
    // Region boundaries
    int regionHeight = height/numRegions;
    int lastRegionHeight = height - regionHeight*(numRegions-1);

    // Extents of my region
    int myStartingHeight = fullImageExtents[2] + myPositionInRegion * regionHeight;
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
            if ( regionVector[i] == my_id )
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

        if ( dest == my_id )
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

        //std::cout << my_id << " ~ i: " << i << "   regionVector[index]: " << regionVector[index] << "  extents: " <<  sendExtents[index*5 + 0] << ", " << sendExtents[index*5 + 1]  << ", " << sendExtents[index*5 + 2] << ", " << sendExtents[index*5 + 3] << "  sending ... " << std::endl;
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

    intermediateImageBB[0] = intermediateImageBB[2] = 0;
    intermediateImageBB[1] = intermediateImageBB[3] = 0;

    if (inRegion)
    {
        for (int i=0; i<numRegions; i++)
        {
            int index = i;

            //debug5 << "regionVector[" << i << "] " << regionVector[index] << std::endl;

            if (regionVector[index] == my_id)
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

                    blendFrontToBack(imgData, imgExtents, extentsSectionRecv, intermediateImage, intermediateImageExtents);
                    //debug5 << "Blend with: " << regionVector[index]  << "  extentsSectionRecv: " << extentsSectionRecv[0] << ", " << extentsSectionRecv[1] << "    " << extentsSectionRecv[2] << ", " << extentsSectionRecv[3] << ", "  << std::endl;
                    //writeArrayToPPM("/home/pascal/Desktop/debugImages/composited_AFTER_recv_from_" + toStr(regionVector[index]) + "_at_" + toStr(my_id), intermediateImage, intermediateImageExtents[1]-intermediateImageExtents[0], intermediateImageExtents[3]-intermediateImageExtents[2]);


                    updateBoundingBox(intermediateImageBB, extentsSectionRecv);
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

                    blendFrontToBack(recvImageData, recvImageExtents, intermediateImage, intermediateImageExtents);

                    updateBoundingBox(intermediateImageBB, recvImageExtents);
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
        intermediateImageBB[0]=intermediateImageBB[1]=intermediateImageBB[2]=intermediateImageBB[3] = 0;

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


// ****************************************************************************
//  Method: avtSLIVRImageCompositor::findRegionsForPatch
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
avtSLIVRImageCompositor::findRegionsForPatch(int patchExtents[4], int screenProjectedExtents[4], int numRegions, int &from, int &to)
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


// ****************************************************************************
//  Method: avtSLIVRImageCompositor::computeRegionExtents
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
avtSLIVRImageCompositor::computeRegionExtents(int numRanks, int height)
{
    int regionHeight = round((float)height/numRanks);
    regularRegionSize = regionHeight;
    maxRegionHeight = 0;
    regionRankExtents.resize(numRanks*3);
    for (int i=0; i<numRanks; i++)
    {
        int startRegionExtents, endRegionExtents, _currentRegionHeight;

        startRegionExtents = clamp(regionHeight * i, 0, height);
        endRegionExtents = clamp(regionHeight * i + regionHeight, 0, height);

        if ( i == numRanks -1 )
            if ( endRegionExtents < height )
                endRegionExtents = height;

        _currentRegionHeight = clamp(endRegionExtents-startRegionExtents, 0, height);
        maxRegionHeight = std::max(maxRegionHeight, _currentRegionHeight);

        regionRankExtents[i*3+0] = startRegionExtents;
        regionRankExtents[i*3+1] = endRegionExtents;
        regionRankExtents[i*3+2] = _currentRegionHeight;

        debug5 << i << " : (start, end, region): " << startRegionExtents << ", " << endRegionExtents << ", " << _currentRegionHeight << std::endl;
    }
}


// ****************************************************************************
//  Method: avtSLIVRImageCompositor::parallelDirectSendManyPatches
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
avtSLIVRImageCompositor::parallelDirectSendManyPatches(std::multimap<int, imgData> imgDataHashMap, std::vector<imgMetaData> imageMetaPatchVector, int numPatches, int region[], int numRegions, int tags[2], int fullImageExtents[4])
{
    int myRegionHeight = 0;
  #ifdef PARALLEL
    debug5 << "Parallel Direct Send" << endl;

    //
    // Some initializations
    for (int i=0; i<4; i++)
    {
        intermediateImageExtents[i] = 0;
        intermediateImageBB[i] = 0;
    }

    //
    // Find my position in region
    compositingDone = false;
    int myPositionInRegion = -1;
    bool inRegion = true;
    std::vector<int> regionVector(region, region+numRegions);
    std::vector<int>::iterator it = std::find(regionVector.begin(), regionVector.end(), my_id);

    if (it == regionVector.end())
    {
        inRegion = false;
        debug5 << my_id << " ~ SHOULD NOT HAPPEN!!!!: Not found " << my_id <<  " !!!" << std::endl;
    }
    else
        myPositionInRegion = it - regionVector.begin();

    int width =  fullImageExtents[1]-fullImageExtents[0];
    int height = fullImageExtents[3]-fullImageExtents[2];

    debug5 << my_id << " ~ myPositionInRegion: " << myPositionInRegion << ", num_procs: " << num_procs << std::endl;
    debug5 << "width: " << width << ", height : " << height 
           << " | fullImageExtents: " << fullImageExtents[0] << ", " << fullImageExtents[1] << ", " << fullImageExtents[2] << ", " << fullImageExtents[3] << std::endl;
    
    //
    // Region boundaries
    computeRegionExtents(num_procs,height);
    int myStartingHeight = getScreenRegionStart(myPositionInRegion, fullImageExtents[2], fullImageExtents[3]);
    int myEndingHeight   = getScreenRegionEnd(myPositionInRegion, fullImageExtents[2], fullImageExtents[3]);
    myRegionHeight = clamp( (myEndingHeight-myStartingHeight), 0, height);

    debug5 << "myStartingHeight: " << myStartingHeight << ", myEndingHeight: " << myEndingHeight << ", myRegionHeight: " << myRegionHeight << std::endl;


    // Size of one buffer
    int sizeOneBuffer = getMaxRegionHeight() * width * 4;


    //
    // Determine how many patches and pixel to send to each region
    std::vector<int> numPatchesPerRegion;
    std::vector<int> areaPerRegion;
    std::set<int> numOfRegions;

    numPatchesPerRegion.resize(numRegions);
    areaPerRegion.resize(numRegions);


    // 2D array: extents for each partition
    std::vector < std::vector<float> > extentsPerPartiton;
    for (int i=0; i<numRegions; i++)
        extentsPerPartiton.push_back( std::vector<float>() );

    debug5 << "Parallel Direct Send ~ numPatches " << numPatches << endl;

    int totalSendBufferSize = 0;
    for (int i=0; i<numPatches; i++)
    {
        int _patchExtents[4];
        imgMetaData temp;
        temp = imageMetaPatchVector.at(i);

        _patchExtents[0]=temp.screen_ll[0];   // minX
        _patchExtents[1]=temp.screen_ur[0];   // maxX
        _patchExtents[2]=temp.screen_ll[1];   // minY
        _patchExtents[3]=temp.screen_ur[1];   // maxY

        std::multimap<int, imgData>::iterator it = imgDataHashMap.find( i );

        int from, to;
        int numRegionIntescection = findRegionsForPatch(_patchExtents, fullImageExtents, numRegions, from, to);
        if (numRegionIntescection <= 0)
            continue;

        debug5 << "\nParallel Direct Send ~ patch " << i << "  from:" << from << "  to:" << to << "  numPatches: " << numPatches 
        << "   _patchExtents: " << _patchExtents[0] << ", " << _patchExtents[1] << ", " << _patchExtents[2] << ", " << _patchExtents[3] 
        << ", fullImageExtents[2]: " << fullImageExtents[2] << ", numRegions: " <<  numRegions << ", totalSendBufferSize: " << totalSendBufferSize << endl;

        for (int j=from; j<=to; j++)
            numPatchesPerRegion[j]++;

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

    //
    // Copy the data for each region for each patch

    // Create buffer
    float *sendDataBuffer = new float[totalSendBufferSize];     // contains all the data arranged by region
    int *sendDataBufferSize = new int[numRegionsWithData]();
    int *sendDataBufferOffsets = new int[numRegionsWithData]();

    int *sendBuffer = new int[numRegions*2]();
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
            std::multimap<int, imgData>::iterator it = imgDataHashMap.find( _patchID );

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

            if (regionVector[i] != my_id)
                numRegionsToSend++;
        }

        sendBuffer[i*2+0] = numPatchesPerRegion[i];
        sendBuffer[i*2+1] = areaPerRegion[i];

        debug5 << "Region: " << i << "  numPatchesPerRegion: " << sendBuffer[i*2+0] << ", sendBuffer[i*2+1]: " << sendBuffer[i*2+1] << std::endl;
    }

    

    //
    // Exchange information about size to recv
    int *recvInfoATABuffer = new int[numRegions*2]();
    MPI_Alltoall(sendBuffer, 2, MPI_INT,  recvInfoATABuffer, 2, MPI_INT, MPI_COMM_WORLD);
    delete []sendBuffer;
    sendBuffer = NULL;

    debug5 << "\nParallel Direct Send ~ Exchange information about size to recv" << endl;

    //
    // Calculate buffer size needed
    int infoBufferSize = 0;
    int dataBufferSize = 0;
    int numRegionsToRecvFrom = 0;
    for (int i=0; i<numRegions; i++)
    {
        infoBufferSize += recvInfoATABuffer[i*2 + 0];   // number of patches per region
        dataBufferSize += recvInfoATABuffer[i*2 + 1];   // area per region

        debug5 << "From: " << i << ", #patches: " << recvInfoATABuffer[i*2 + 0] << ", " << recvInfoATABuffer[i*2 + 1] << std::endl;

        if (i == my_id)
            continue;

        if (recvInfoATABuffer[i*2 + 0] != 0)
            numRegionsToRecvFrom++;
    }


    //
    // Create structure for MPI Async send/recv

    // Send
    MPI_Request *sendMetaRq = new MPI_Request[ numRegionsToSend ];
    MPI_Status *sendMetaSt = new MPI_Status[ numRegionsToSend ];

    MPI_Request *sendImageRq = new MPI_Request[ numRegionsToSend  ];
    MPI_Status *sendImageSt = new MPI_Status[ numRegionsToSend  ];

    // Recv
    MPI_Request *recvMetaRq = NULL;
    MPI_Status *recvMetaSt = NULL;

    MPI_Request *recvImageRq = NULL;
    MPI_Status *recvImageSt = NULL;

    int recvInfoCount = 0;
    int offsetMeta = 0;
    int offsetData = 0;
    
    //
    // Create recv buffers
    float *recvInfoBuffer = new float[infoBufferSize*6];  // 6 - passing 6 parameters for each patch
    float *recvDataBuffer =  new float[dataBufferSize*4]; // 4 - to account for RGBA

    debug5 << "infoBufferSize: " << infoBufferSize << ", dataBufferSize: " << dataBufferSize << std::endl;
    if (myRegionHeight != 0)
    {

        // Recv
        recvMetaRq = new MPI_Request[ numRegionsToRecvFrom ];
        recvMetaSt = new MPI_Status[ numRegionsToRecvFrom ];

        recvImageRq = new MPI_Request[ numRegionsToRecvFrom  ];
        recvImageSt = new MPI_Status[ numRegionsToRecvFrom  ];


        //
        // Async Recv for info
        for (int i=0; i<numRegions; i++)
        {
            if (recvInfoATABuffer[i*2 + 0] == 0)
                continue;

            if ( regionVector[i] == my_id )
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


    //
    // Async Send
    int offset = 0;
    int sendCount = 0;
    int mpiSendCount = 0;

    for (int i=0; i<numRegions; i++)
    {
        if ( extentsPerPartiton[i].size() != 0 ){
            if ( regionVector[i] == my_id )
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
        intermediateImageBB[0] = intermediateImageExtents[0] = fullImageExtents[0];
        intermediateImageBB[1] = intermediateImageExtents[1] = fullImageExtents[1];
        intermediateImageBB[2] = intermediateImageExtents[2] = myStartingHeight;
        intermediateImageBB[3] = intermediateImageExtents[3] = myEndingHeight;

        intermediateImage = new float[width * (myEndingHeight-myStartingHeight) * 4]();


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

            blendFrontToBack(&recvDataBuffer[ patchOffset[_id] ], _extents, _extents, intermediateImage, intermediateImageExtents);

            //writeArrayToPPM("/home/pascal/Desktop/debugImages/blending_" + toStr(my_id) + "_"+ toStr(numBlends), intermediateImage, intermediateImageExtents[1]-intermediateImageExtents[0], intermediateImageExtents[3]-intermediateImageExtents[2]);
            numBlends++;
        }

        if (numBlends == 0)
            intermediateImageBB[0]=intermediateImageBB[1]=intermediateImageBB[2]=intermediateImageBB[3] = 0;
    }


    MPI_Waitall(numRegionsToSend, sendImageRq, sendImageSt);   // Means that we have sent everything!


    if (myRegionHeight == 0)
        compositingDone = true;

    //
    // Cleanup
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

    debug5 << "All PDS done" << std::endl;
  #endif
    return myRegionHeight;
}




// ****************************************************************************
//  Method: avtSLIVRImageCompositor::gatherImages
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
avtSLIVRImageCompositor::gatherImages(int regionGather[], int totalNumRanks, float * inputImg, int imgExtents[4], int boundingBox[4], int tag, int fullImageExtents[4], int myRegionHeight)
{
  #ifdef PARALLEL
    debug5 << "gatherImages starting... totalNumRanks: " << totalNumRanks << ", compositingDone: " << compositingDone
           << ", imgExtents: " << imgExtents[0] << ", " << imgExtents[1] << ", " << imgExtents[2] << ", " << imgExtents[3] << std::endl;

    for (int i=0; i<4; i++)
        finalImageExtents[i] = finalBB[i] = 0;

    if (my_id == 0)
    {
        int width =  fullImageExtents[1]-fullImageExtents[0];
        int height = fullImageExtents[3]-fullImageExtents[2];

        debug5 << "Gather Images at 0, final size: " << fullImageExtents[1]-fullImageExtents[0] << " x " << fullImageExtents[3]-fullImageExtents[2] << std::endl;

        //
        // Receive at root/display node!
        imgBuffer = new float[width*height*4];
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
        if (getRegionSize(my_id) != 0) 
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

            if (src == my_id)
                continue;

            if (i == totalNumRanks-1)
            {
                if (lastBufferSize != 0)
                {
                    MPI_Irecv(&imgBuffer[i*regularBufferSize], lastBufferSize,     MPI_FLOAT, src, tag, MPI_COMM_WORLD,  &recvImageRq[recvCount] );
                }
            }
            else
                MPI_Irecv(&imgBuffer[i*regularBufferSize], regularBufferSize,  MPI_FLOAT, src, tag, MPI_COMM_WORLD,  &recvImageRq[recvCount] );
            

            debug5 << i << " ~ recvCount: " << recvCount << std::endl;
            recvCount++;
        }

        if (compositingDone == false)   // If root has data for the final image
            placeInImage(inputImg, imgExtents, imgBuffer, finalImageExtents);

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
