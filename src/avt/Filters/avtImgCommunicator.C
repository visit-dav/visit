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
//                         avtImgCommunicator.C                              //
// ************************************************************************* //
#include <cmath>
#include <avtParallel.h>
#include <ImproperUseException.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

#include <avtImgCommunicator.h>
#include <fstream>
#include <DebugStream.h>


#ifdef PARALLEL

MPI_Datatype createImgDataType(){ 
  MPI_Datatype _img_mpi;
  const int numItems = 8;
  int blockLengths[numItems] = {1, 1,   1, 1,   2, 2, 2, 1};
  MPI_Datatype type[numItems] = { MPI_INT, MPI_INT,    MPI_INT, MPI_INT,   MPI_INT, MPI_INT, MPI_INT,   MPI_FLOAT};
  MPI_Aint offsets[numItems] = {0, sizeof(int), sizeof(int)*2, sizeof(int)*3, sizeof(int)*4, sizeof(int)*6, sizeof(int)*8, sizeof(int)*10};
  MPI_Type_struct(numItems, blockLengths,  offsets, type, &_img_mpi);
  
  return _img_mpi;
}

#endif


bool value_comparer(const std::pair<int,int> &before, const std::pair<int,int> &after){ return before.second < after.second; }
bool sortByVecSize(const std::vector<iotaMeta> &before, const std::vector<iotaMeta> &after){return before.size() > after.size();}
bool sortImgByCoordinatesIota(iotaMeta const& before, iotaMeta const& after){
  if(before.screen_ll[0] != after.screen_ll[0]) 
    return before.screen_ll[0] < after.screen_ll[0];
  else 
    return before.screen_ll[1] < after.screen_ll[1];
}
bool sortImgByDepthIota(iotaMeta const& before, iotaMeta const& after){ 
  if(before.avg_z != after.avg_z) 
    return (before.avg_z < after.avg_z);
  else 
    return (before.procId < after.procId); 
}


// ****************************************************************************
//  Method: avtImgCommunicator::
//
//  Purpose:
//
//  Programmer: 
//  Creation:   
//
//  Modifications:
//
// ****************************************************************************
avtImgCommunicator::avtImgCommunicator(){

#ifdef PARALLEL
  MPI_Comm_size(VISIT_MPI_COMM, &num_procs);
  MPI_Comm_rank(VISIT_MPI_COMM, &my_id);

  _img_mpi = createMetaDataType();
  MPI_Type_commit(&_img_mpi);
#else
    num_procs = 1;
    my_id = 0;
#endif
    
    totalPatches = 0;
    numPatchesToCompose = 0;

    processorPatchesCount = NULL;
  imgBuffer = NULL;

  allRecvIotaMeta = NULL;
  patchesToSendArray = NULL;
  patchesToRecvArray = NULL;
  numPatchesToSendArray = NULL;
  numPatchesToRecvArray = NULL;
  recvDisplacementForProcs = NULL;
  sendDisplacementForProcs = NULL;
  numPatchesToSendRecvArray = NULL;
  boundsPerBlockArray = NULL;
  blockDisplacementForProcs = NULL;
  numBlocksPerProc = NULL;
  patchesToCompositeLocallyArray = NULL;
  numPatchesToCompositeLocally = NULL;
  compositeDisplacementForProcs = NULL;

  compressedSizePerDiv = NULL;

  all_avgZ_proc0.clear();
}



// ****************************************************************************
//  Method: avtImgCommunicator::
//
//  Purpose:
//
//  Programmer: 
//  Creation:   
//
//  Modifications:
//
// ****************************************************************************
avtImgCommunicator::~avtImgCommunicator(){
  if (my_id == 0){
    if (processorPatchesCount != NULL)
      delete []processorPatchesCount;

    if (allRecvIotaMeta != NULL)
      delete []allRecvIotaMeta;

    if (imgBuffer != NULL)
      delete []imgBuffer;

    if (compressedSizePerDiv != NULL)
      delete []compressedSizePerDiv;
    compressedSizePerDiv = NULL;


    if (patchesToSendArray!=NULL) delete[] patchesToSendArray;
        if (patchesToRecvArray!=NULL) delete[] patchesToRecvArray;
        if (numPatchesToSendArray!=NULL) delete[] numPatchesToSendArray;
        if (numPatchesToRecvArray!=NULL) delete[] numPatchesToRecvArray;
        if (recvDisplacementForProcs!=NULL) delete[] recvDisplacementForProcs;
        if (sendDisplacementForProcs!=NULL) delete[] sendDisplacementForProcs;
        if (blockDisplacementForProcs!=NULL) delete[] blockDisplacementForProcs;
        if (numPatchesToSendRecvArray!=NULL) delete[] numPatchesToSendRecvArray;
        if (boundsPerBlockArray!=NULL) delete[] boundsPerBlockArray;
        if (numBlocksPerProc!=NULL) delete[] numBlocksPerProc;
        if (patchesToCompositeLocallyArray!=NULL) delete[] patchesToCompositeLocallyArray;
        if (numPatchesToCompositeLocally!=NULL) delete[] numPatchesToCompositeLocally;
        if (compositeDisplacementForProcs!=NULL) delete[] compositeDisplacementForProcs;

        patchesToSendArray = NULL;
    patchesToRecvArray = NULL;
    numPatchesToSendArray = NULL;
    numPatchesToRecvArray = NULL;
    recvDisplacementForProcs = NULL;
    sendDisplacementForProcs = NULL;
    numPatchesToSendRecvArray = NULL;
    boundsPerBlockArray = NULL;
    blockDisplacementForProcs = NULL;
    numBlocksPerProc = NULL;
    patchesToCompositeLocallyArray = NULL;
    numPatchesToCompositeLocally = NULL;
    compositeDisplacementForProcs = NULL;
  }
  
  all_avgZ_proc0.clear();

#ifdef PARALLEL
  MPI_Type_free(&_img_mpi);
#endif
}



// ****************************************************************************
//  Method: avtImgCommunicator::
//
//  Purpose:
//
//  Programmer: 
//  Creation:   
//
//  Modifications:
//
// ****************************************************************************
int avtImgCommunicator::getDataPatchID(int procID, int patchID){
  int sumPatches = 0;
  for (int i=0; i<procID; i++)
    sumPatches += processorPatchesCount[i];
  
  return (sumPatches+patchID);

}

// ****************************************************************************
//  Method: avtImgCommunicator::
//
//  Purpose:
//    initialize 
//
//  Programmer: Pascal Grosset
//  Creation: July 2013  
//
//  Modifications:
//
// ****************************************************************************
void avtImgCommunicator::init(){
}


// ****************************************************************************
//  Method: avtImgCommunicator::waitToSync
//
//  Purpose:
//      Wait for all processors to hearch here before continuing
//
//  Programmer: Pascal Grosset
//  Creation: July 2013  
//
//  Modifications:
//
// ****************************************************************************
void avtImgCommunicator::syncAllProcs(){
#ifdef PARALLEL
  MPI_Barrier(MPI_COMM_WORLD);
#endif
}



// ****************************************************************************
//  Method: avtImgCommunicator::gatherNumPatches
//
//  Purpose:
//    Get the number of patches each processor has
//
//  Arguments:
//    numPatches: number of patches a processor has
//
//  Programmer: Pascal Grosset
//  Creation: July 2013
//
//  Modifications:
//
// ****************************************************************************
void avtImgCommunicator::gatherNumPatches(int numPatches){

  #ifdef PARALLEL
    int patchesProc[2];
    patchesProc[0] = my_id; patchesProc[1] = numPatches;
    int *tempRecvBuffer = NULL;

    if (my_id == 0){  
      processorPatchesCount = new int[num_procs];   // creates a buffer to hold the number of patches per processor
      tempRecvBuffer = new int[num_procs*2];
    }

    MPI_Gather(patchesProc, 2, MPI_INT,   tempRecvBuffer, 2,MPI_INT,         0, MPI_COMM_WORLD);    // all send to proc 0

    if (my_id == 0){    
      for (int i=0; i<num_procs; i++){
        processorPatchesCount[tempRecvBuffer[i*2]] = tempRecvBuffer[i*2 + 1]; // enter the number of patches for each processor
        totalPatches += processorPatchesCount[tempRecvBuffer[i*2]];       // count the number of patches
      }

      if (tempRecvBuffer != NULL)
        delete []tempRecvBuffer;
      tempRecvBuffer = NULL;
    }

  #endif
}



// ****************************************************************************
//  Method: avtImgCommunicator::
//
//  Purpose: 
//    Send the metadata needed by the root node to make decisions
//
//  Arguments:
//    arraySize   : the number of elements being sent
//    allIotaMetadata : the metadata bieng sent
//
//  Programmer: Pascal Grosset
//  Creation: July 2013
//
//  Modifications:
//
// ****************************************************************************
void avtImgCommunicator::gatherIotaMetaData(int arraySize, float *allIotaMetadata){

  #ifdef PARALLEL
    int *recvSizePerProc = NULL;
    float *tempRecvBuffer = NULL;
    int *offsetBuffer = NULL;

    if (my_id == 0){
      tempRecvBuffer = new float[totalPatches*7]; // x7: procId, patchNumber, dims[0], dims[1], screen_ll[0], screen_ll[1], avg_z
      recvSizePerProc = new int[num_procs]; 
      offsetBuffer = new int[num_procs];  
      for (int i=0; i<num_procs; i++){
        if (i == 0)
          offsetBuffer[i] = 0;
        else
          offsetBuffer[i] = offsetBuffer[i-1] + recvSizePerProc[i-1];

        recvSizePerProc[i] = processorPatchesCount[i]*7;
      }
    }

    MPI_Gatherv(allIotaMetadata, arraySize, MPI_FLOAT,   tempRecvBuffer, recvSizePerProc, offsetBuffer,MPI_FLOAT,    0, MPI_COMM_WORLD);// all send to proc 0

    if (my_id == 0){
      allRecvIotaMeta = new iotaMeta[totalPatches]; // allocate space to receive the many patches

      iotaMeta tempPatch;
      for (int i=0; i<totalPatches; i++){
        tempPatch.procId =    (int) tempRecvBuffer[i*7 + 0];
        tempPatch.patchNumber = (int) tempRecvBuffer[i*7 + 1];
        tempPatch.dims[0] =   (int) tempRecvBuffer[i*7 + 2];
        tempPatch.dims[1] =   (int) tempRecvBuffer[i*7 + 3];
        tempPatch.screen_ll[0] =(int) tempRecvBuffer[i*7 + 4];
        tempPatch.screen_ll[1] =(int) tempRecvBuffer[i*7 + 5];
        tempPatch.avg_z =         tempRecvBuffer[i*7 + 6];

        int patchIndex = getDataPatchID(tempPatch.procId, tempPatch.patchNumber);
        allRecvIotaMeta[patchIndex] = setIota(tempPatch.procId, tempPatch.patchNumber, tempPatch.dims[0], tempPatch.dims[1], tempPatch.screen_ll[0], tempPatch.screen_ll[1], tempPatch.avg_z);
        all_avgZ_proc0.insert(tempPatch.avg_z); //insert avg_zs into the set to keep a count of the total number of avg_zs
      }

      if (recvSizePerProc != NULL)
        delete []recvSizePerProc;
      recvSizePerProc = NULL;

      if (offsetBuffer != NULL)
        delete []offsetBuffer;
      offsetBuffer = NULL;

      if (tempRecvBuffer != NULL)
        delete []tempRecvBuffer;
      tempRecvBuffer = NULL;

    }
  #endif  
}


// ****************************************************************************
//  Method: avtImgCommunicator::
//
//  Purpose:
//
//  Programmer: Manasa Prasad
//  Creation: August 2013
//
//  Modifications:
//
// ****************************************************************************
void determinePatchesToCompositeLocally(const std::vector<iotaMeta>& all_patches_sorted_avgZ_proc0, std::vector<std::vector<iotaMeta> >& patchesToCompositeLocallyVector, const int& procToSend, std::vector<std::vector<int> >& divisions){

  if(all_patches_sorted_avgZ_proc0.size() == 0) return;
  iotaMeta prev_data = *all_patches_sorted_avgZ_proc0.begin();
  bool already_in = false;

  iotaMeta delimiter;
  delimiter.patchNumber = -1;

  for (size_t i=1; i < all_patches_sorted_avgZ_proc0.size(); ++i){
    if(prev_data.procId == all_patches_sorted_avgZ_proc0[i].procId && all_patches_sorted_avgZ_proc0[i].procId != procToSend){
      if(!already_in) {
        patchesToCompositeLocallyVector[prev_data.procId].push_back(delimiter);
        patchesToCompositeLocallyVector[prev_data.procId].push_back(prev_data);
        divisions[prev_data.procId].push_back(patchesToCompositeLocallyVector[prev_data.procId].size() - 1);
        already_in = true;
      }
      patchesToCompositeLocallyVector[prev_data.procId].push_back(all_patches_sorted_avgZ_proc0[i]);
    }else{
      prev_data = all_patches_sorted_avgZ_proc0[i];
      already_in = false;
    }
  }

  for (size_t i=0; i < patchesToCompositeLocallyVector.size(); ++i){
    divisions[i].push_back(patchesToCompositeLocallyVector[i].size() + 1);
  }
}

// ****************************************************************************
//  Method: avtImgCommunicator::
//
//  Purpose:
//
//  Programmer: Manasa Prasad
//  Creation: August 2013
//
//  Modifications:
//
// ****************************************************************************

bool adjacencyTest(const iotaMeta& patch_1, const iotaMeta& patch_2){
  if( (patch_2.screen_ll[0] == patch_1.screen_ll[0] /*+ patch_1.dims[0]*/) && 
    (patch_2.screen_ll[1] <= patch_1.screen_ll[1] + patch_1.dims[1])) 
    return true;
  return false;
}

void determinePatchAdjacency(std::vector<iotaMeta>& allPatchesSorted, std::vector<std::vector<iotaMeta> >& patchesToComposite, std::vector<std::vector<int> >& divisions ){

  if(allPatchesSorted.size() == 0) return;

  iotaMeta delimiter; 
  delimiter.patchNumber = -1;

  std::vector<iotaMeta>::iterator it;

  iotaMeta prevPatch, currentPatch, nextPatch;
  int lower, upper;

  for (size_t i=0; i < patchesToComposite.size(); ++i){
    for(size_t k=0; k<divisions[i].size()-1; k++){

      lower = divisions[i][k];
      upper = divisions[i][k+1]-1;

      std::sort(  patchesToComposite[i].begin()+lower, 
            patchesToComposite[i].begin()+upper,  &sortImgByCoordinatesIota);

      for(int j = lower; j < upper; j++){

        currentPatch = patchesToComposite[i][j];
        if(j < upper-1)   nextPatch = patchesToComposite[i][j+1];
        if(j > lower)   prevPatch = patchesToComposite[i][j-1];
      
        //
        // if patch is adjacent to the preceding patch, remove it from the allPatchesSorted list
        //
        if (j > lower && adjacencyTest(prevPatch, currentPatch)) {
          it = std::find(allPatchesSorted.begin(), allPatchesSorted.end(), currentPatch); 
          allPatchesSorted.erase(it);
        }

        //
        // if patch is adjacent to the succeeding patch, check if:
        //          patch lies somewhere in the middle => insert a delimiter before patch
        //
        else if (j < upper-1 && adjacencyTest(currentPatch, nextPatch)){
          if (j != lower){ //insert a -1      
            it = std::find(patchesToComposite[i].begin(), patchesToComposite[i].end(), currentPatch);
            patchesToComposite[i].insert(it, delimiter);

            for(size_t m = k; m < divisions[i].size() - 1; m++)
              divisions[i][m+1]++;
            j++; upper++;

            int position = it - patchesToComposite[i].begin();
            lower = position + 1;
          }
          // else do nothing. Let the first patch remain
        }

        //
        // if patch does not lie beside any adjoining patch, remove it from patchesToComposite
        //
        else{
          it = std::find(patchesToComposite[i].begin(), patchesToComposite[i].end(), currentPatch);
          patchesToComposite[i].erase(it);

          // if this is the only remaining element, also remove the -1
          if(upper == lower+1){ 
            patchesToComposite[i].erase(--it);
            for(size_t m = k; m < divisions[i].size() - 1; m++)
              divisions[i][m+1] -= 2;
          }else{
            for(size_t m = k; m < divisions[i].size() - 1; m++)
              divisions[i][m+1]--;
            j--; upper--;

          }
        }
        // end of j loop
      }
      // end of k loop
    }
    // end of i loop
  }
}



// ****************************************************************************
//  Method: avtImgCommunicator::
//
//  Purpose:
//
//  Programmer: Manasa Prasad
//  Creation: August 2013
//
//  Modifications:
//
// ****************************************************************************
int calculatePatchDivision (const std::vector<iotaMeta>& imgVector, std::vector<int>& procsAlreadyInList){
std::map<int,int> numPatchesPerProc;
  std::pair<std::map<int,int>::iterator, bool> isPresent;

  if (imgVector.size() == 0) return 0;

  for (size_t i = 0; i < imgVector.size(); ++i){
    const bool is_in = ((std::find(procsAlreadyInList.begin(), procsAlreadyInList.end(), imgVector[i].procId)) != (procsAlreadyInList.end()));
    if(!is_in){
      isPresent = numPatchesPerProc.insert (  std::pair<int,int>(imgVector[i].procId, imgVector[i].dims[0] * imgVector[i].dims[1]) );
      if(isPresent.second == false)     
        numPatchesPerProc[imgVector[i].procId] += imgVector[i].dims[0] * imgVector[i].dims[1];
    }
  }

  if(numPatchesPerProc.size() == 0){
    for (size_t i = 0; i < imgVector.size(); ++i){
      isPresent = numPatchesPerProc.insert (  std::pair<int,int>(imgVector[i].procId, imgVector[i].dims[0] * imgVector[i].dims[1]) );
      if(isPresent.second == false)     
        numPatchesPerProc[imgVector[i].procId] += imgVector[i].dims[0] * imgVector[i].dims[1];
    }
  }

  return std::max_element(numPatchesPerProc.begin(), numPatchesPerProc.end(), value_comparer)->first;
}



// ****************************************************************************
//  Method: avtImgCommunicator::
//
//  Purpose:
//
//  Programmer: Manasa Prasad
//  Creation: August 2013
//
//  Modifications:
//
// ****************************************************************************
void avtImgCommunicator::patchAllocationLogic(){

  // 1. do not send the numpatchespreproc vector!
  // 2. do away with numcompositedPatches test and replace with totalRecvPatches

  std::vector<std::vector<iotaMeta> > all_patches_sorted_avgZ_proc0(num_procs); 
  std::vector<int> procToSend;
  std::vector<int> numAvgZEachBlock (num_procs);

  numBlocksPerProc = new int[num_procs]();
  boundsPerBlockVec.resize(num_procs);

  int num_avgZ_proc0 = all_avgZ_proc0.size();

  // calculate the range of avg_zs
  int num_avgZ_perBlock = (num_avgZ_proc0 / num_procs);
  int rem_avgZ = num_avgZ_proc0 % num_procs;

  //printf("num_avg_z: %d num_procs: %d num_avgZ_perBlock: %d rem_avgZ: %d \n\n", num_avgZ_proc0, num_procs, num_avgZ_perBlock, rem_avgZ);
  
  //procToSend.resize           (num_procs);
  //all_patches_sorted_avgZ_proc0.resize(num_procs);

  for(int i = 0; i < num_procs; i++){
    numAvgZEachBlock[i] = num_avgZ_perBlock + (rem_avgZ-- > 0 ? 1 : 0);
  }

  // Sorting the patches
  std::sort(allRecvIotaMeta, allRecvIotaMeta + totalPatches, &sortImgByDepthIota);

  // Populate the all_patches_sorted_avgZ_proc0 vector with data from allPatchesMeta
  int current_avgZ_Block = 0;
  float prevAvgZ = totalPatches > 0 ? allRecvIotaMeta[0].avg_z : 0;
  int num_avgZ_thisBlock = 1;

  for (int i = 0; i < totalPatches; ++i){
    float currentAvgZ = allRecvIotaMeta[i].avg_z;

    if (currentAvgZ != prevAvgZ && num_avgZ_thisBlock == numAvgZEachBlock[current_avgZ_Block]){
      num_avgZ_thisBlock = 1;
      current_avgZ_Block++;
      prevAvgZ = currentAvgZ;
    } else if (currentAvgZ != prevAvgZ){
      prevAvgZ = currentAvgZ;
      num_avgZ_thisBlock++;
    }

    all_patches_sorted_avgZ_proc0[current_avgZ_Block].push_back(allRecvIotaMeta[i]);
  }

  //sort the avg_z vector by size
  std::sort (all_patches_sorted_avgZ_proc0.begin(), all_patches_sorted_avgZ_proc0.end(), sortByVecSize);
  std::vector<std::vector<iotaMeta> > patchesToCompositeLocallyVector(num_procs);

  // Calculate which block of avg_z to send to which processor
  // ith block is sent to the processor in procToSend[i]

  //printf("num_avg_z: %d num_procs: %d\n\n", num_avgZ_proc0, num_procs);
  for (int i = 0; i < num_procs; ++i){
    
    procToSend.push_back(calculatePatchDivision(all_patches_sorted_avgZ_proc0[i], procToSend));
    std::vector<std::vector<int> > divisions(num_procs);
    determinePatchesToCompositeLocally(all_patches_sorted_avgZ_proc0[i], patchesToCompositeLocallyVector, procToSend[i], divisions); 
    determinePatchAdjacency(all_patches_sorted_avgZ_proc0[i], patchesToCompositeLocallyVector, divisions);

    int size = all_patches_sorted_avgZ_proc0[i].size();
    if(size > 0){ 
      boundsPerBlockVec[procToSend[i]].push_back(all_patches_sorted_avgZ_proc0[i][0].avg_z);
      boundsPerBlockVec[procToSend[i]].push_back(all_patches_sorted_avgZ_proc0[i][size-1].avg_z);
    }
    //printf("division: %d, procToSend: %d \n", i, procToSend[i]);
  }

  // Printing for error check
  for(int i = 0; i < num_procs; ++i){
    debug5 <<  "Block: " << i << " \t size:" << all_patches_sorted_avgZ_proc0[i].size() << endl;
  //  printf("Block %d\n", i );
    for(std::vector<iotaMeta>::iterator it = all_patches_sorted_avgZ_proc0[i].begin(); it != all_patches_sorted_avgZ_proc0[i].end(); ++it){
      debug5 <<  it->avg_z << "\t " << it->procId << endl;
  //    printf("\t %.5f \t %d\n", it->avg_z, it->procId);
    }
  }


  std::vector< std::vector<int> >  patchesToSendVec (num_procs);
  std::vector< std::map<int,int> > patchesToRecvMap (num_procs); 
  std::pair< std::map<int,int>::iterator, bool > isPresent;

  for (int procId = 0; procId < num_procs; procId++)  patchesToRecvMap[procId].clear();
  
  // Populate recvMap and sendVec
  for (size_t i = 0; i < all_patches_sorted_avgZ_proc0.size(); i++){

    int destProcId = procToSend[i];
    for (size_t j = 0; j < all_patches_sorted_avgZ_proc0[i].size(); j++){

      int originProcId = all_patches_sorted_avgZ_proc0[i][j].procId;
      if(originProcId != destProcId){

        // Array of the form [0 1 2 3 ...]
        //           even numbers(0,2..): patchNumber 
        //           odd numbers(1,3...): destProcId
          patchesToSendVec[originProcId].push_back( all_patches_sorted_avgZ_proc0[i][j].patchNumber );
        patchesToSendVec[originProcId].push_back( destProcId );

        // Array of the form [0 1 2 3 ...]
        //           even numbers(0,2..): procId 
        //           odd numbers(1,3...): numPatches
          isPresent = patchesToRecvMap[destProcId].insert( std::pair<int,int>(originProcId, 1) );
          if(isPresent.second == false)   ++patchesToRecvMap[destProcId][originProcId];
          
          //printf("Inserted to dest: %d the origin: %d avg_z: %.2f\n", destProcId, originProcId, allPatchesMeta[patchIndex].avg_z );
      }
    }

    debug5 <<  "------division: " << i << " procToSend:" << procToSend[i] << endl;
  }


  recvDisplacementForProcs  = new int[num_procs]();
  sendDisplacementForProcs  = new int[num_procs]();
  numPatchesToSendArray     = new int[num_procs]();
  numPatchesToRecvArray     = new int[num_procs]();
  blockDisplacementForProcs   = new int[num_procs]();
  numPatchesToSendRecvArray   = new int[4*num_procs]();

  compositeDisplacementForProcs = new int[num_procs]();
  numPatchesToCompositeLocally = new int[num_procs]();

  int totalRecvSize = 0;
  int totalSendSize = 0;
  int totalBlockSize = 0;
  int totalCompositeSize = 0;

  for (int currentProcId = 0; currentProcId < num_procs; currentProcId++){

    numPatchesToSendArray[currentProcId] = patchesToSendVec[currentProcId].size();
    numPatchesToRecvArray[currentProcId] = 2*patchesToRecvMap[currentProcId].size();
    numBlocksPerProc[currentProcId]    = boundsPerBlockVec[currentProcId].size();
    numPatchesToCompositeLocally[currentProcId] = patchesToCompositeLocallyVector[currentProcId].size();

    numPatchesToSendRecvArray[currentProcId*4]    = numPatchesToSendArray[currentProcId];
    numPatchesToSendRecvArray[currentProcId*4 + 1]  = numPatchesToRecvArray[currentProcId];
    numPatchesToSendRecvArray[currentProcId*4 + 2]  = numBlocksPerProc[currentProcId];
    numPatchesToSendRecvArray[currentProcId*4 + 3]  = numPatchesToCompositeLocally[currentProcId];

    totalRecvSize += numPatchesToRecvArray[currentProcId];
    totalSendSize += numPatchesToSendArray[currentProcId];
    totalBlockSize+= numBlocksPerProc[currentProcId];
    totalCompositeSize += numPatchesToCompositeLocally[currentProcId];

    if(currentProcId!=0) {
      recvDisplacementForProcs[currentProcId] = recvDisplacementForProcs[currentProcId-1] + numPatchesToRecvArray[currentProcId-1];
      sendDisplacementForProcs[currentProcId] = sendDisplacementForProcs[currentProcId-1] + numPatchesToSendArray[currentProcId-1];
      blockDisplacementForProcs[currentProcId] = blockDisplacementForProcs[currentProcId-1] + numBlocksPerProc[currentProcId-1];
      compositeDisplacementForProcs[currentProcId] = compositeDisplacementForProcs[currentProcId-1] + numPatchesToCompositeLocally[currentProcId-1];
    }

  }

  patchesToRecvArray  = new int[totalRecvSize];
  patchesToSendArray  = new int[totalSendSize];
  boundsPerBlockArray = new float[totalBlockSize];
  patchesToCompositeLocallyArray = new int[totalCompositeSize];

  for (int currentProcId = 0; currentProcId < num_procs; currentProcId++){

    //printf("\n\n### dest: %d size: %ld\n ", currentProcId, patchesToRecvMap[currentProcId].size());
    int count = 0;
    for (std::map<int,int>::iterator it = patchesToRecvMap[currentProcId].begin(); it!=patchesToRecvMap[currentProcId].end(); ++it){
        patchesToRecvArray[recvDisplacementForProcs[currentProcId] + (count++)] = it->first;
        patchesToRecvArray[recvDisplacementForProcs[currentProcId] + (count++)] = it->second;
        //printf("\t numPatches %d origin: %d\n", it->second, it->first);
    }

    count = 0;
    //printf("\n\n### origin: %d size: %ld\n ", currentProcId, patchesToSendVec[currentProcId].size());
    for (std::vector<int>::iterator it = patchesToSendVec[currentProcId].begin(); it!=patchesToSendVec[currentProcId].end(); ++it){
      patchesToSendArray[sendDisplacementForProcs[currentProcId] + (count++)] = *it;

      //if(count%2!=0) printf("\t patchNumber %d", *it);
      //else       printf(" dest: %d\n", *it);

    }

    count = 0;
    for (std::vector<float>::iterator it = boundsPerBlockVec[currentProcId].begin(); it!=boundsPerBlockVec[currentProcId].end(); ++it)
      boundsPerBlockArray[blockDisplacementForProcs[currentProcId] + (count++)] = *it;

    count = 0;
    for (std::vector<iotaMeta>::iterator it = patchesToCompositeLocallyVector[currentProcId].begin(); it!=patchesToCompositeLocallyVector[currentProcId].end(); ++it)
      patchesToCompositeLocallyArray[compositeDisplacementForProcs[currentProcId] + (count++)] = it->patchNumber;

  }

  //printf("\n ..................................................\n");

}


// ****************************************************************************
//  Method: avtImgCommunicator::
//
//  Purpose:
//
//  Programmer: Manasa Prasad
//  Creation: July 2013
//
//  Modifications:
//
// ****************************************************************************

void avtImgCommunicator::scatterNumDataToCompose(int &totalSendData, int &totalRecvData, int &numDivisions, int &totalPatchesToCompositeLocally){
  int totalSendRecvData[4];
  #ifdef PARALLEL
    MPI_Scatter(numPatchesToSendRecvArray, 4, MPI_INT, totalSendRecvData, 4, MPI_INT, 0, MPI_COMM_WORLD);
  #endif

  totalSendData = totalSendRecvData[0];
  totalRecvData = totalSendRecvData[1];
  numDivisions = totalSendRecvData[2]; // twice the  number of divisions
  totalPatchesToCompositeLocally = totalSendRecvData[3];
}



// ****************************************************************************
//  Method: avtImgCommunicator::
//
//  Purpose:
//
//  Programmer: Manasa Prasad
//  Creation: July 2013
//
//  Modifications:
//
// ****************************************************************************
void avtImgCommunicator::scatterDataToCompose(  int& totalSendData, int* informationToSendArray, 
                        int& totalRecvData, int* informationToRecvArray, 
                        int& numDivisions, float* blocksPerProc,
                        int& totalPatchesToCompositeLocally, int* patchesToCompositeLocally){
  #ifdef PARALLEL
        MPI_Scatterv(patchesToSendArray, numPatchesToSendArray, sendDisplacementForProcs, MPI_INT, informationToSendArray, totalSendData, MPI_INT, 0,MPI_COMM_WORLD);
        MPI_Scatterv(patchesToRecvArray, numPatchesToRecvArray, recvDisplacementForProcs, MPI_INT, informationToRecvArray, totalRecvData, MPI_INT, 0,MPI_COMM_WORLD);
        MPI_Scatterv(boundsPerBlockArray, numBlocksPerProc, blockDisplacementForProcs, MPI_FLOAT, blocksPerProc, numDivisions, MPI_FLOAT, 0,MPI_COMM_WORLD);
        MPI_Scatterv(patchesToCompositeLocallyArray, numPatchesToCompositeLocally, compositeDisplacementForProcs, MPI_INT, patchesToCompositeLocally, totalPatchesToCompositeLocally, MPI_INT, 0, MPI_COMM_WORLD);
  #endif

    if(my_id == 0){
        if (patchesToSendArray!=NULL) delete[] patchesToSendArray;
        if (patchesToRecvArray!=NULL) delete[] patchesToRecvArray;
        if (numPatchesToSendArray!=NULL) delete[] numPatchesToSendArray;
        if (numPatchesToRecvArray!=NULL) delete[] numPatchesToRecvArray;
        if (recvDisplacementForProcs!=NULL) delete[] recvDisplacementForProcs;
        if (sendDisplacementForProcs!=NULL) delete[] sendDisplacementForProcs;
        if (blockDisplacementForProcs!=NULL) delete[] blockDisplacementForProcs;
        if (numPatchesToSendRecvArray!=NULL) delete[] numPatchesToSendRecvArray;
        if (boundsPerBlockArray!=NULL) delete[] boundsPerBlockArray;
        if (numBlocksPerProc!=NULL) delete[] numBlocksPerProc;
        if (patchesToCompositeLocallyArray!=NULL) delete[] patchesToCompositeLocallyArray;
        if (numPatchesToCompositeLocally!=NULL) delete[] numPatchesToCompositeLocally;
        if (compositeDisplacementForProcs!=NULL) delete[] compositeDisplacementForProcs;

        patchesToSendArray = NULL;
    patchesToRecvArray = NULL;
    numPatchesToSendArray = NULL;
    numPatchesToRecvArray = NULL;
    recvDisplacementForProcs = NULL;
    sendDisplacementForProcs = NULL;
    numPatchesToSendRecvArray = NULL;
    boundsPerBlockArray = NULL;
    blockDisplacementForProcs = NULL;
    numBlocksPerProc = NULL;
    patchesToCompositeLocallyArray = NULL;
    numPatchesToCompositeLocally = NULL;
    compositeDisplacementForProcs = NULL;
    }
}



// ****************************************************************************
//  Method: avtImgCommunicator::
//
//  Purpose:
//
//  Programmer: Pascal Grosset
//  Creation: July 2013  
//
//  Modifications:
//
// ****************************************************************************
void avtImgCommunicator::sendPointToPoint(imgMetaData toSendMetaData, imgData toSendImgData, int tag){
  #ifdef PARALLEL
    // Commit the datatype
    MPI_Datatype _TestImg_mpi;
    _TestImg_mpi = createImgDataType();
    MPI_Type_commit(&_TestImg_mpi);

      MPI_Send(&toSendMetaData, 1, _TestImg_mpi, toSendMetaData.destProcId, tag, MPI_COMM_WORLD); // 2
    MPI_Send(toSendImgData.imagePatch, toSendMetaData.dims[0]*toSendMetaData.dims[1]*4, MPI_FLOAT, toSendMetaData.destProcId, tag-1, MPI_COMM_WORLD); //send the image data // 1
   
      MPI_Type_free(&_TestImg_mpi);
    #endif
}


// ****************************************************************************
//  Method: avtImgCommunicator::
//
//  Purpose:
//
//  Programmer: Pascal Grosset
//  Creation: July 2013  
//
//  Modifications:
//
// ****************************************************************************
void avtImgCommunicator::recvPointToPoint(imgMetaData &recvMetaData, imgData &recvImgData){
  #ifdef PARALLEL
    // Commit the datatype
    MPI_Datatype _TestImg_mpi;
    _TestImg_mpi = createImgDataType();
    MPI_Type_commit(&_TestImg_mpi);

        MPI_Recv (&recvMetaData, 1, _TestImg_mpi, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &status);

        recvImgData.procId = recvMetaData.procId;
        recvImgData.patchNumber = recvMetaData.patchNumber;
        recvImgData.imagePatch = new float[recvMetaData.dims[0]*recvMetaData.dims[1] * 4];

        MPI_Recv(recvImgData.imagePatch, recvMetaData.dims[0]*recvMetaData.dims[1]*4, MPI_FLOAT, status.MPI_SOURCE, 1, MPI_COMM_WORLD, &status);

        MPI_Type_free(&_TestImg_mpi);
    #endif
}


// ****************************************************************************
//  Method: avtImgCommunicator::
//
//  Purpose:
//
//  Programmer: Pascal Grosset
//  Creation: July 2013  
//
//  Modifications:
//
// ****************************************************************************
void avtImgCommunicator::recvPointToPointMetaData(imgMetaData &recvMetaData, int tag){
  #ifdef PARALLEL
    // Commit the datatype
    MPI_Datatype _TestImg_mpi;
    _TestImg_mpi = createImgDataType();
    MPI_Type_commit(&_TestImg_mpi);

        MPI_Recv (&recvMetaData, 1, _TestImg_mpi, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status); // 2

        MPI_Type_free(&_TestImg_mpi);
    #endif
}



// ****************************************************************************
//  Method: avtImgCommunicator::
//
//  Purpose:
//
//  Programmer: Pascal Grosset
//  Creation: July 2013  
//
//  Modifications:
//
// ****************************************************************************
void avtImgCommunicator::recvPointToPointImgData(imgMetaData recvMetaData, imgData &recvImgData, int tag){
  #ifdef PARALLEL
        MPI_Recv(recvImgData.imagePatch, recvMetaData.dims[0]*recvMetaData.dims[1]*4, MPI_FLOAT, status.MPI_SOURCE, tag-1, MPI_COMM_WORLD, &status); // 1
    #endif
}



// ****************************************************************************
//  Method: avtImgCommunicator::
//
//  Purpose:
//
//  Programmer: Pascal Grosset
//  Creation: July 2013  
//
//  Modifications:
//
// ****************************************************************************
void avtImgCommunicator::gatherEncodingSizes(int *sizeEncoding, int numDivisions){

  #ifdef PARALLEL
  int *offsetBuffer = NULL;
  int *recvSizePerProc = NULL;
  int totalDivisions = 0;

    // Only proc 0 receives data
    if (my_id == 0){

      recvSizePerProc = new int[num_procs];
      offsetBuffer = new int[num_procs];
      
      for (int i=0; i<num_procs; i++){
        int numBoundsPerBlock = boundsPerBlockVec[i].size()/2;
        totalDivisions += numBoundsPerBlock;
        recvSizePerProc[i] = numBoundsPerBlock;

        if (i == 0)
          offsetBuffer[i] = 0;
        else
          offsetBuffer[i] = offsetBuffer[i-1] + recvSizePerProc[i-1];

      }
      compressedSizePerDiv = new int[totalDivisions];
    }

        //  send   recv  others
    MPI_Gatherv(sizeEncoding, numDivisions, MPI_INT,    compressedSizePerDiv, recvSizePerProc, offsetBuffer,MPI_INT,      0, MPI_COMM_WORLD); // all send to proc 0



    if (my_id == 0){
      for (int i=0; i<totalDivisions; i++)
        debug5 <<  "  0 div: " << i << " : " << compressedSizePerDiv[i] << endl;

      if (offsetBuffer != NULL) 
        delete []offsetBuffer;
      offsetBuffer = NULL;

      if (recvSizePerProc != NULL)  
        delete []recvSizePerProc;
      recvSizePerProc = NULL;
    }
  #endif
}


float avtImgCommunicator::clamp(float x){
  if (x > 1.0)
    x = 1.0;

  if (x < 0.0)
    x = 0.0;

  return x;
}

// ****************************************************************************
//  Method: avtImgCommunicator::
//
//  Purpose:
//
//  Programmer: Pascal Grosset
//  Creation: July 2013  
//
//  Modifications:
//
// ****************************************************************************
void avtImgCommunicator::gatherAndAssembleEncodedImages(int sizex, int sizey, int sizeSending, float *images, int numDivisions){
  #ifdef PARALLEL
    float *tempRecvBuffer = NULL;
    int *recvSizePerProc = NULL;
    int *offsetBuffer = NULL;
    int totalDivisions = 0;
    std::map<float,int> depthPartitions;  //float: z-value, int: division index

    // Only proc 0 receives data
    if (my_id == 0){
      int divIndex = 0;
      int totalSize = 0;
      recvSizePerProc = new int[num_procs]; 
      offsetBuffer = new int[num_procs];  

      for (int i=0; i<num_procs; i++){
        int numBoundsPerBlock = boundsPerBlockVec[i].size()/2;
        totalDivisions += numBoundsPerBlock;
        
        int sizeEncoded = 0;
        for (size_t j=0; j<boundsPerBlockVec[i].size(); j+=2){
          depthPartitions.insert( std::pair< float,int > ( (boundsPerBlockVec[i][j] + boundsPerBlockVec[i][j+1]) * 0.5, divIndex ) );   
          sizeEncoded += compressedSizePerDiv[divIndex]*5;
          divIndex++;
        }
        totalSize += sizeEncoded;
        recvSizePerProc[i] = sizeEncoded;

        if (i == 0)
          offsetBuffer[i] = 0;
        else
          offsetBuffer[i] = offsetBuffer[i-1] + recvSizePerProc[i-1];
      }

      tempRecvBuffer = new float[ totalSize ];
    }

        //  send   recv  others
    MPI_Gatherv(images, sizeSending, MPI_FLOAT,    tempRecvBuffer, recvSizePerProc, offsetBuffer,MPI_FLOAT,        0, MPI_COMM_WORLD);    // all send to proc 0

    if (my_id == 0){
      // Create a buffer to store the composited image
      imgBuffer = new float[sizex * sizey * 4];
      
      // Front to back
      for (int i=0; i<(sizex * sizey * 4); i+=4){
        imgBuffer[i+0] = background[0]/255.0; 
        imgBuffer[i+1] = background[1]/255.0; 
        imgBuffer[i+2] = background[2]/255.0; 
        imgBuffer[i+3] = 1.0;
      }

      int count = 0;
      int offset = 0;
      int index = 0;

      if (depthPartitions.size() > 0){
        std::map< float,int >::iterator it;
        it=depthPartitions.end();
        it=depthPartitions.end();
        do{
          --it;
          debug5 << it->first << " => " << it->second << "    compressedSizePerDiv[count]: " << compressedSizePerDiv[count] << std::endl;

          imageBuffer temp;
          temp.image = new float[sizex*sizey*4];
          index = it->second;

          if (index == 0)
            offset = 0;
          else{
            offset = 0;
            for (int k=0; k<index; k++)
              offset += compressedSizePerDiv[k];
          }

          rleDecode(compressedSizePerDiv[index], tempRecvBuffer, offset*5, temp.image);

              for (int j=0; j<sizey; j++){
            for (int k=0; k<sizex; k++){
              int imgIndex = sizex*4*j + k*4;                   // index in the image 

              // Back to front compositing
              imgBuffer[imgIndex+0] = clamp((imgBuffer[imgIndex+0] * (1.0 - temp.image[imgIndex+3])) + temp.image[imgIndex+0]);
              imgBuffer[imgIndex+1] = clamp((imgBuffer[imgIndex+1] * (1.0 - temp.image[imgIndex+3])) + temp.image[imgIndex+1]);
              imgBuffer[imgIndex+2] = clamp((imgBuffer[imgIndex+2] * (1.0 - temp.image[imgIndex+3])) + temp.image[imgIndex+2]);
            }
          }

              delete []temp.image;
              temp.image = NULL;

              count++;
        }while( it!=depthPartitions.begin() );
      }

      if (tempRecvBuffer != NULL)
        delete []tempRecvBuffer;
      tempRecvBuffer = NULL;

      if (offsetBuffer != NULL)
        delete []offsetBuffer;
      offsetBuffer = NULL;

      if (recvSizePerProc != NULL)
        delete []recvSizePerProc;
      recvSizePerProc = NULL;
    }

    syncAllProcs();

  #endif
}



// ****************************************************************************
//  Method: avtImgCommunicator::
//
//  Purpose:
//
//  Programmer: Pascal Grosset
//  Creation: July 2013  
//
//  Modifications:
//
// ****************************************************************************
void avtImgCommunicator::getcompositedImage(int imgBufferWidth, int imgBufferHeight, unsigned char *wholeImage){

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
//  Method: avtImgCommunicator::
//
//  Purpose:
//
//  Programmer: Pascal Grosset
//  Creation: July 2013  
//
//  Modifications:
//
// ****************************************************************************
imgMetaData avtImgCommunicator::setImg(int _inUse, int _procId, int _patchNumber, float dim_x, float dim_y, float screen_ll_x, float screen_ll_y, float screen_ur_x, float screen_ur_y, float _avg_z){
  imgMetaData temp;
  temp.inUse = _inUse;
  temp.procId = _procId;
  temp.destProcId = _procId;
  temp.patchNumber = _patchNumber;
  temp.dims[0] = dim_x;         temp.dims[1] = dim_y;
  temp.screen_ll[0] = screen_ll_x;    temp.screen_ll[1] = screen_ll_y;
  temp.screen_ur[0] = screen_ur_x;    temp.screen_ur[1] = screen_ur_y;
  temp.avg_z = _avg_z;
  
  return temp;
}


// ****************************************************************************
//  Method: avtImgCommunicator::
//
//  Purpose:
//
//  Programmer: Pascal Grosset
//  Creation: July 2013  
//
//  Modifications:
//
// ****************************************************************************
iotaMeta avtImgCommunicator::setIota(int _procId, int _patchNumber, int dim_x, int dim_y, int screen_ll_x, int screen_ll_y, float _avg_z){
  iotaMeta temp;
  temp.procId = _procId;
  temp.patchNumber = _patchNumber;
  temp.dims[0] = dim_x;         temp.dims[1] = dim_y;
  temp.screen_ll[0] = screen_ll_x;    temp.screen_ll[1] = screen_ll_y;
  temp.avg_z = _avg_z;
  
  return temp;
}





// ****************************************************************************
//  Method: avtImgCommunicator::
//
//  Purpose:
//
//  Programmer: 
//  Creation:   
//
//  Modifications:
//
// ****************************************************************************

bool compareColor(code x, float r, float g, float b, float a){
  if ((x.color[0] == r && x.color[1]==g) &&  (x.color[2] == b && x.color[3]==a))
    return true;
  else
    return false;
}


// ****************************************************************************
//  Method: avtImgCommunicator::
//
//  Purpose:
//
//  Programmer: 
//  Creation:   
//
//  Modifications:
//
// ****************************************************************************

code initCode(int count, float r, float g, float b, float a){
  code temp;
  temp.count = count;
  temp.color[0] = r;  temp.color[1]=g;  temp.color[2]=b;  temp.color[3]=a;
  return temp;  
}


// ****************************************************************************
//  Method: avtImgCommunicator::
//
//  Purpose:
//
//  Programmer: 
//  Creation:   
//
//  Modifications:
//
// ****************************************************************************

code incrCode(code x){
  x.count += 1;
  return x;
}



// ****************************************************************************
//  Method: avtImgCommunicator::rleEncodeAll
//
//  Purpose:
//    Encodes an image using RLE: Runlength, R, G, B, A
//
//  Arguments:
//    dimsX & dimsY : width & height of the image
//    imgArray    : array containing all the division composited by this processor
//    numDivs     : number of Z divisions
//
//    encoding      : the image compressed using RLE; it's a float so that it can be sent directly using MPI
//    sizeOfEncoding  : the compressed size of each image
//
//
//  Programmer: Pascal Grosset
//  Creation: August 23, 2013
//
//  Modifications:
//
// ****************************************************************************
int avtImgCommunicator::rleEncodeAll(int dimsX, int dimsY, int numDivs, float *imgArray,  float *& encoding, int *& sizeOfEncoding){
  std::vector<code> encodingVec;
  encodingVec.clear();
  code tempCode;
  sizeOfEncoding = new int[numDivs];
  int prev = 0;

  // Compress the data
  for (int j=0; j<numDivs; j++){
    int offset = dimsX*dimsY*4  *  j; // to get the next image in the array of images

    int i=0;
    tempCode = initCode(1, imgArray[offset + (i*4+0)],imgArray[offset + (i*4+1)],imgArray[offset + (i*4+2)],  imgArray[offset + (i*4+3)]);
    for (i=1; i<dimsX*dimsY; i++){
      if ( compareColor(tempCode, imgArray[offset + (i*4+0)],imgArray[offset + (i*4+1)],imgArray[offset + (i*4+2)],imgArray[offset + (i*4+3)]) )
        tempCode = incrCode(tempCode);
      else{
        encodingVec.push_back(tempCode);
        tempCode = initCode(1, imgArray[offset + (i*4+0)],imgArray[offset + (i*4+1)],imgArray[offset + (i*4+2)],imgArray[offset + (i*4+3)]);
      }
    }
    encodingVec.push_back(tempCode);

    if (j == 0)
      prev = sizeOfEncoding[j] = encodingVec.size();
    else{
      sizeOfEncoding[j] = encodingVec.size() - prev;
      prev = encodingVec.size();
    }

    debug5 <<  my_id << "  ~  encoding.size(): " << sizeOfEncoding[j] << "   offset: " << offset << "    original size: " << (dimsX * dimsY * 4) << "   size: " << encodingVec.size() << std::endl;
  }


  // Transfer the data to the encoding array
  int encSize = encodingVec.size();
  encoding = new float[encSize*5];
  
  int index = 0;
  for (int j=0; j<encSize; j++){
    encoding[index] = encodingVec[j].count; index++;
    encoding[index] = encodingVec[j].color[0];  index++;
    encoding[index] = encodingVec[j].color[1];  index++;
    encoding[index] = encodingVec[j].color[2];  index++;
    encoding[index] = encodingVec[j].color[3];  index++;  
  }
  encodingVec.clear();

  return encSize;   // size of the array
}



// ****************************************************************************
//  Method: avtImgCommunicator::rleDecode
//
//  Purpose:
//    Decodes rle encoded image to an RGBA image the size of the screen
//
//  Arguments:
//    encSize : size of compressed image
//    encoding: the image to be decoded
//    offset  : offset in the encoding array (all images from all procs are stored in 1 array)
//    img     : an array containing the RGBA decompressed image
//
//  Programmer: Pascal Grosset
//  Creation: August 23, 2013
//
//  Modifications:
//
// ****************************************************************************
void avtImgCommunicator::rleDecode(int encSize, float *encoding, int offset, float *& img){
  int imgIndex = 0; // index into the image to be decompressed

  //debug5 << " 0 ~ offset: " << offset  << " encSize: " << encSize << std::endl;

  for (int i=0; i<encSize; i++)
    for (int j=0; j<(int)encoding[offset + i*5 + 0]; j++) // *5 to offset into image; +0 run count
    {
      img[imgIndex*4 + 0] = encoding[offset + i*5 + 1]; // R
      img[imgIndex*4 + 1] = encoding[offset + i*5 + 2]; // G
      img[imgIndex*4 + 2] = encoding[offset + i*5 + 3]; // B
      img[imgIndex*4 + 3] = encoding[offset + i*5 + 4]; // A

      imgIndex++;
    }
}


// ****************************************************************************
//  Method: avtImgCommunicator::
//
//  Purpose:
//
//  Programmer: 
//  Creation:   
//
//  Modifications:
//
// ****************************************************************************
#ifdef PARALLEL
MPI_Datatype avtImgCommunicator::createMetaDataType(){
  MPI_Datatype _imgMeta_mpi;
  const int numItems = 7;
  int blockLengths[numItems] = {1, 1, 1, 2, 2, 2, 1};
  MPI_Datatype type[numItems] = { MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_FLOAT };
  MPI_Aint offsets[numItems] = {0, sizeof(int), sizeof(int)*2, sizeof(int)*3, sizeof(int)*5, sizeof(int)*7, sizeof(int)*9 };
  MPI_Type_struct(numItems, blockLengths,  offsets, type, &_imgMeta_mpi);
  
  return _imgMeta_mpi;
}
#endif
