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
//                      avtSubsetBlockMergeFilter.C                          //
// ************************************************************************* //

#include <avtSubsetBlockMergeFilter.h>
#include <avtParallel.h>

#include <DebugStream.h>

#include <vtkAppendPolyData.h>
#include <vtkCleanPolyData.h>
#include <vtkDataSetWriter.h>
#include <vtkPolyDataReader.h>
#include <vtkCharArray.h>

#ifdef PARALLEL
    #include <mpi.h>

    const int SIZE_TAG = GetUniqueMessageTag();
    const int BLOCKID_TAG = GetUniqueMessageTag();
    const int DATA_TAG = GetUniqueMessageTag();
#endif

using std::string;
using std::vector;
using std::map;

// ****************************************************************************
//  Method: avtSubsetBlockMergeFilter constructor
//
//  Purpose:
//      Defines the constructor.
//
//  Programmer: Kevin Griffin
//  Creation:   October 10, 2014
//
// ****************************************************************************

avtSubsetBlockMergeFilter::avtSubsetBlockMergeFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtSubsetBlockMergeFilter destructor
//
//  Purpose:
//      Defines the destructor.
//
//  Programmer: Kevin Griffin
//  Creation:   October 10, 2014
//
// ****************************************************************************

avtSubsetBlockMergeFilter::~avtSubsetBlockMergeFilter()
{
    ;
}

// ****************************************************************************
//  Method: avtSubsetBlockMergeFilter::Execute
//
//  Purpose: Merge all datasets into one dataset for each block
//
//  Programmer: Kevin Griffin
//  Creation:   October 10, 2014
//
//  Modifications:
//
//
// ****************************************************************************

void
avtSubsetBlockMergeFilter::Execute()
{
#ifdef PARALLEL
    // Map processor to list of datasets (blocks) that its responsible for
    map<int, vector<BlockIdDatasetPair> > procDatasetMap;
#endif
    
    avtDataTree_p inputTree = GetInputDataTree();
    
    // Get the number of incoming datasets
    int numInDataSets;
    vtkDataSet **dataSets = inputTree->GetAllLeaves(numInDataSets);
    
    // Get all of the labels
    vector<string> labels;
    vector<string> uniqueLabels;
    inputTree->GetAllLabels(labels);
    inputTree->GetAllUniqueLabels(uniqueLabels);
    
    // Merge datasets for each block
    int outSize = uniqueLabels.size();
    
    vtkDataSet **outDataSets = new vtkDataSet *[outSize];
    vtkAppendPolyData **appender = new vtkAppendPolyData *[outSize];
    vtkCleanPolyData **cleaner = new vtkCleanPolyData *[outSize];
    
    for(int i=0; i<outSize; i++)
    {
        appender[i] = vtkAppendPolyData::New();
        cleaner[i] = vtkCleanPolyData::New();
    }
    
    // :ASSUMPTION: Assuming that datasets are in the same order as labels
    for(int i=0; i<labels.size(); i++)
    {
        int index = GetIndexFromBlockId(labels[i], uniqueLabels);
        appender[index]->AddInputData(dynamic_cast<vtkPolyData *>(dataSets[i]));
    }
    
    // Clean all merged data sets
    for(int i=0; i<outSize; i++)
    {
        cleaner[i]->SetInputConnection(appender[i]->GetOutputPort());
        cleaner[i]->Update();
        outDataSets[i] = cleaner[i]->GetOutput();
        
        appender[i]->Delete();
        
#ifdef PARALLEL
        AddDatasetToMap(procDatasetMap, outDataSets[i], uniqueLabels[i]);
#endif
    }
    
#ifdef PARALLEL
    map<string, vtkAppendPolyData *> blockAppenderMap;
    int myRank = PAR_Rank();
    
    for(int rank=0; rank<PAR_Size(); rank++)
    {
        if(myRank == rank)
        {
            Send(procDatasetMap, myRank);
        }
        else
        {
            Receive(blockAppenderMap);
        }
    }
    
    // Add Own Data if applicable
    map<int, vector<BlockIdDatasetPair> >::iterator iter = procDatasetMap.find(myRank);
    
    if(iter != procDatasetMap.end())
    {
        for(int j=0; j<iter->second.size(); j++)
        {
            map<string, vtkAppendPolyData *>::iterator iter2 = blockAppenderMap.find(iter->second[j].blockId);
            if(iter2 != blockAppenderMap.end())
            {
                iter2->second->AddInputData(dynamic_cast<vtkPolyData *>(iter->second[j].dataSet));
            }
            else {
                vtkAppendPolyData *appender = vtkAppendPolyData::New();
                appender->AddInputData(dynamic_cast<vtkPolyData *>(iter->second[j].dataSet));
                blockAppenderMap[iter->second[j].blockId] = appender;
            }
        }
    }
    
    // Merge datasets from other processors then create output tree
    if(blockAppenderMap.size() > 0)
    {
        SetOutputDataTree(CreateOutputDataTree(blockAppenderMap));
    }
    else
    {
        avtDataTree_p dummy = new avtDataTree();
        SetOutputDataTree(dummy);
    }
    
    // Clean Up
    for(map<string, vtkAppendPolyData *>::iterator iter = blockAppenderMap.begin(); iter != blockAppenderMap.end(); iter++)
    {
        iter->second->Delete();
    }
#else
    avtDataTree_p outDataTree = new avtDataTree(outSize, outDataSets, 0, uniqueLabels);
    SetOutputDataTree(outDataTree);
#endif
    
    // Clean Up
    delete [] appender;
    delete [] outDataSets;
    
    for(int i=0; i<outSize; i++)
    {
        cleaner[i]->Delete();
    }
    delete [] cleaner;
}

// ****************************************************************************
//  Method: avtSubsetBlockMergeFilter::CreateOutputDataTree
//
//  Purpose: Create the output data tree for this filter
//
//  Arguments:
//      blockAppenderMap    maps blockId to the vtkAppendPolyData class used
//                          to merge all individual datasets of a particular
//                          block.
//
//  Returns:                The output data tree
//
//  Programmer: Kevin Griffin
//  Creation:   October 10, 2014
//
//  Modifications:
//
//
// ****************************************************************************
avtDataTree_p
avtSubsetBlockMergeFilter::CreateOutputDataTree(map<string, vtkAppendPolyData *> &blockAppenderMap)
{
    vector<string> labels;
    int numChildren = blockAppenderMap.size();
    vtkDataSet **outDataSets = new vtkDataSet *[numChildren];
    vtkCleanPolyData **cleaners = new vtkCleanPolyData *[numChildren];
    
    int i=0;
    for(map<string, vtkAppendPolyData *>::iterator iter = blockAppenderMap.begin(); iter != blockAppenderMap.end(); iter++)
    {
        labels.push_back(iter->first);
        
        cleaners[i] = vtkCleanPolyData::New();
        cleaners[i]->SetInputConnection(iter->second->GetOutputPort());
        cleaners[i]->Update();
        
        outDataSets[i] = cleaners[i]->GetOutput();
        
        ++i;
    }
    
    avtDataTree_p tree = new avtDataTree(numChildren, outDataSets, 0, labels);
    
    // Cleanup
    delete [] outDataSets;
    
    for(int i=0; i<numChildren; i++)
    {
        cleaners[i]->Delete();
    }
    delete [] cleaners;
    
    return tree;
}

// ****************************************************************************
//  Method: avtSubsetBlockMergeFilter::AddDatasetToMap
//
//  Purpose: Add the locally merged datset (block) to the the list of the
//           processor that's responsible for the global dataset (block)
//           merge.
//
//  Arguments:
//      procDatasetMap  Mapping of processor to locally merged datasets (blocks)
//      dataSet         the merged dataset (block) to add to the map
//      blockId         the block id
//
//
//  Programmer: Kevin Griffin
//  Creation:   October 10, 2014
//
//  Modifications:
//
//
// ****************************************************************************
void
avtSubsetBlockMergeFilter::AddDatasetToMap(map<int, vector<BlockIdDatasetPair> > &procDatasetMap, vtkDataSet *dataSet, const string blockId)
{
    BlockIdDatasetPair sendData;
    sendData.blockId = blockId;
    sendData.dataSet = dataSet;
    
    int processorId = GetProcessorIdFromBlockId(atoi(blockId.c_str()));
    
    // Add sendData to map
    if(!procDatasetMap.empty())
    {
        map<int, vector<BlockIdDatasetPair> >::iterator iter = procDatasetMap.find(processorId);
        
        if(iter != procDatasetMap.end())
        {
            iter->second.push_back(sendData);
        }
        else
        {
            vector<BlockIdDatasetPair> newVector;
            newVector.push_back(sendData);
            procDatasetMap[processorId] = newVector;
        }
    }
    else
    {
        vector<BlockIdDatasetPair> newVector;
        newVector.push_back(sendData);
        procDatasetMap[processorId] = newVector;
    }
}

// ****************************************************************************
//  Method: avtSubsetBlockMergeFilter::Receive
//
//  Purpose:    Receive local block-merged dataset(s) from other processor(s)
//              that this processor is responsible for merging into global
//              block-merged dataset(s).
//
//  Arguments:
//      outMap  Mapping of block id to a vtkAppendPolyData class that is
//              responsible for merging all of the separate datasets belonging
//              to the same block
//
//
//  Programmer: Kevin Griffin
//  Creation:   October 10, 2014
//
//  Modifications:
//
//
// ****************************************************************************
void
avtSubsetBlockMergeFilter::Receive(map<string, vtkAppendPolyData *> &outMap)
{
#ifdef PARALLEL
    const int CHAR_BUFFER_SIZE = 50;
    MPI_Status mpiStatus;
    char buffer[CHAR_BUFFER_SIZE];
    int myRank = PAR_Rank();
    
    // First Get the number of datasets being sent to receiveProcessor
    int count;
    MPI_Recv(&count, 1, MPI_INT, MPI_ANY_SOURCE, SIZE_TAG, VISIT_MPI_COMM, &mpiStatus);
    debug5 << myRank << ": Dataset Count = " << count << endl;
    
    if(count > 0)
    {
        int source = mpiStatus.MPI_SOURCE;
        debug5 << myRank << ": Source is: " << source << endl;
        
        for(int i=0; i<count; i++)
        {
            // Get Block ID
            int blockId;
            MPI_Recv(&blockId, 1, MPI_INT, source, BLOCKID_TAG, VISIT_MPI_COMM, &mpiStatus);
                    
            int blockIdLen = SNPRINTF(buffer, CHAR_BUFFER_SIZE, "%d", blockId);
            string blockIdStr = string(buffer, blockIdLen);
            
            // Get Dataset
            int dataSize;
            MPI_Recv(&dataSize, 1, MPI_INT, source, SIZE_TAG, VISIT_MPI_COMM, &mpiStatus);
            debug5 << myRank << ": Data Size = " << dataSize << endl;
                    
            char *recvBinary = new char[dataSize];
            MPI_Recv(recvBinary, dataSize, MPI_CHAR, source, DATA_TAG, VISIT_MPI_COMM, &mpiStatus);
                    
            vtkCharArray *charArray = vtkCharArray::New();
            charArray->SetArray(recvBinary, dataSize, 1);
                    
            vtkPolyDataReader *polyReader = vtkPolyDataReader::New();
            polyReader->SetReadFromInputString(1);
            polyReader->SetInputArray(charArray);
            
            map<string, vtkAppendPolyData *>::iterator iter = outMap.find(blockIdStr);
            if(iter != outMap.end())
            {
                iter->second->AddInputConnection(polyReader->GetOutputPort());
                iter->second->Update();
            }
            else
            {
                vtkAppendPolyData *appender = vtkAppendPolyData::New();
                appender->AddInputConnection(polyReader->GetOutputPort());
                appender->Update();
                outMap[blockIdStr] = appender;
            }
                    
            // Cleanup
            delete [] recvBinary;
            charArray->Delete();
            polyReader->Delete();
        }
    }
#endif
}

// ****************************************************************************
//  Method: avtSubsetBlockMergeFilter::Send
//
//  Purpose:    Send the locally block merged datasets to the processor
//              responsible for the global block merging
//
//  Arguments:
//      procDatasetMap  Mapping of global merge processor to locally block
//                      merged datasets
//      source          The processor sending the data
//
//
//  Programmer: Kevin Griffin
//  Creation:   October 10, 2014
//
//  Modifications:
//
//
// ****************************************************************************
void
avtSubsetBlockMergeFilter::Send(map<int, vector<BlockIdDatasetPair> > &procDatasetMap, const int source)
{
#ifdef PARALLEL
    for(int dest=0; dest<PAR_Size(); dest++)
    {
        if(dest != source)
        {
            map<int, vector<BlockIdDatasetPair> >::iterator iter = procDatasetMap.find(dest);
            int count;
            
            if(iter != procDatasetMap.end())
            {
                vector<BlockIdDatasetPair> *sendDataList = &iter->second;
                count = sendDataList->size();
                MPI_Send(&count, 1, MPI_INT, dest, SIZE_TAG, VISIT_MPI_COMM);  // Send # of datasets
                
                for(int dataIdx=0; dataIdx<count; dataIdx++)
                {
                    // Send Block ID
                    int blockId = atoi((*sendDataList)[dataIdx].blockId.c_str());
                    MPI_Send(&blockId, 1, MPI_INT, dest, BLOCKID_TAG, VISIT_MPI_COMM);
                    
                    // Send Dataset
                    vtkDataSetWriter *serializer = vtkDataSetWriter::New();
                    serializer->SetInputData((*sendDataList)[dataIdx].dataSet);
                    serializer->SetWriteToOutputString(1);
                    serializer->SetFileTypeToBinary();
                    serializer->Write();
                    
                    int dataSendCnt = serializer->GetOutputStringLength();
                    char *sendData = serializer->RegisterAndGetOutputString();
                    
                    MPI_Send(&dataSendCnt, 1, MPI_INT, dest, SIZE_TAG, VISIT_MPI_COMM);
                    MPI_Send(sendData, dataSendCnt, MPI_CHAR, dest, DATA_TAG, VISIT_MPI_COMM);
                    
                    // Cleanup
                    serializer->Delete();
                    delete sendData;
                    
                    debug5 << source << ": Sent " << dataSendCnt << " Bytes to " << dest << endl;
                }
                
                
            }
            else
            {
                count = 0;
                MPI_Send(&count, 1, MPI_INT, dest, SIZE_TAG, VISIT_MPI_COMM);
                debug5 << source << ": Sent 0 bytes to " << dest << endl;
            }
        }
    }
#endif
}

// ****************************************************************************
//  Method: avtSubsetBlockMergeFilter::GetProcessorIdFromBlockId
//
//  Purpose:    Determine the processor that will do the global block merge of
//              the block identified by blockId
//
//  Arguments:
//      blockId The block ID
//
//  Returns:    The processor number responsible for doing the global block
//              merge of the block identified by blockId
//
//
//  Programmer: Kevin Griffin
//  Creation:   October 10, 2014
//
//  Modifications:
//
//
// ****************************************************************************
const int
avtSubsetBlockMergeFilter::GetProcessorIdFromBlockId(const int blockId) const
{
    int nprocs = PAR_Size();
    
    if(blockId < nprocs)
    {
        return blockId;
    }
    else
    {
        return (blockId % nprocs);
    }
}

// ****************************************************************************
//  Method: avtSubsetBlockMergeFilter::GetIndexFromBlockId
//
//  Purpose:    Determine the array index that will be used for a block
//              identified by blockId
//
//  Arguments:
//      blockId The block ID
//      labels  The list of block IDs
//
//  Returns:    The array index used for a specific block or -1 if the block ID 
//              is not contained in the label list.
//  
//
//  Programmer: Kevin Griffin
//  Creation:   October 10, 2014
//
//  Modifications:
//
//
// ****************************************************************************
const int
avtSubsetBlockMergeFilter::GetIndexFromBlockId(const std::string blockId, const std::vector<std::string> &labels) const
{
    for(int i=0; i<labels.size(); i++)
    {
        if(blockId.compare(labels[i]) == 0)
        {
            return i;
        }
    }
    
    return -1;
}

// ****************************************************************************
//  Method: avtSubsetBlockMergeFilter::PostExecute
//
//  Purpose:    Once the new avtDataTree is created with new labels, this 
//              is needed so everything works correctly in parallel.
//
//
//  Programmer: Kevin Griffin
//  Creation:   November 18, 2014
//
//  Modifications:
//
//
// ****************************************************************************
void
avtSubsetBlockMergeFilter::PostExecute()
{
   vector<string> treeLabels;
   GetDataTree()->GetAllUniqueLabels(treeLabels);
   GetOutput()->GetInfo().GetAttributes().SetLabels(treeLabels);
}
