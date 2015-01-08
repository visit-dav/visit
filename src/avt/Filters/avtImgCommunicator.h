/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

struct code{
  float count;  // should be int but float makes it easier to send with MPI!
  float color[4];
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
  int totalPatches;
  int numPatchesToCompose;
  int *processorPatchesCount;

  float *imgBuffer;
  iotaMeta *allRecvIotaMeta;


  std::set<float> all_avgZ_proc0;
  std::vector<std::vector<float> > boundsPerBlockVec;

  int *patchesToSendArray, *patchesToRecvArray;
  int *numPatchesToSendArray, *numPatchesToRecvArray;
  int *recvDisplacementForProcs, *sendDisplacementForProcs;

  int *numPatchesToSendRecvArray;
  float *boundsPerBlockArray;
  int *blockDisplacementForProcs;
  int *numBlocksPerProc;

  int* patchesToCompositeLocallyArray;
  int* numPatchesToCompositeLocally;
  int* compositeDisplacementForProcs;

  int *compressedSizePerDiv;  //size of each division
  
  unsigned char background[3];

    int     num_procs;
    int     my_id;

    imgMetaData setImg(int _inUse, int _procId, int _patchNumber, float dim_x, float dim_y, float screen_ll_x, float screen_ll_y, float screen_ur_x, float screen_ur_y, float _avg_z);
    iotaMeta setIota(int _procId, int _patchNumber, int dim_x, int dim_y, int screen_ll_x, int screen_ll_y, float _avg_z);
    int getDataPatchID(int procID, int patchID);
   

public:
  avtImgCommunicator();
  ~avtImgCommunicator();

  virtual const char       *GetType(void)
                                         { return "avtImgCommunicator"; };
    virtual const char       *GetDescription(void)
                                         { return "Doing compositing for ray casting SLIVR";};

  void init();

  void gatherNumPatches(int numPatches);
  void gatherIotaMetaData(int arraySize, float *allIotaMetadata);

  void patchAllocationLogic();    // decides which processor should get which patches and tell each processor how many patches it will receive

  void scatterNumDataToCompose(int& totalSendData, int& totalRecvData, int& numDivisions, int& totalPatchesToCompositeLocally);
  void scatterDataToCompose(  int& totalSendData, int* informationToSendArray, 
                int& totalRecvData, int* informationToRecvArray, 
                int& numDivisions, float* blocksPerProc,
                int& totalPatchesToCompositeLocally, int* patchesToCompositeLocally);

  void sendPointToPoint(imgMetaData toSendMetaData, imgData toSendImgData, int tag);  // Send out the patches and receive them
  void recvPointToPoint(imgMetaData &recvMetaData, imgData &recvImgData);

  void recvPointToPointMetaData(imgMetaData &recvMetaData, int tag);
  void recvPointToPointImgData(imgMetaData recvMetaData, imgData &recvImgData, int tag);

  void gatherEncodingSizes(int *sizeEncoding, int numDivisions);
  void gatherAndAssembleEncodedImages(int sizex, int sizey, int sizeSending, float *image, int numDivisions);   // do the compositing of the subpatches

  void getcompositedImage(int imgBufferWidth, int imgBufferHeight, unsigned char *wholeImage);  // get the final composited image


  int rleEncodeAll(int dimsX, int dimsY, int numDivs, float *imgArray,  float *& encoding, int *& sizeOfEncoding);
  void rleDecode(int encSize, float *encoding, int offset, float *& img);

  void syncAllProcs();
  int GetNumProcs(){ return num_procs;};
  int GetMyId(){ return my_id;};

  float clamp(float x);
  void setBackground(unsigned char _background[3]){ for (int i=0; i<3; i++) background[i] = _background[i]; }
  
#ifdef PARALLEL
  MPI_Status status;
  MPI_Datatype _img_mpi;

  MPI_Datatype createMetaDataType();
#endif
};


#endif
