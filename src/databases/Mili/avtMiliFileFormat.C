/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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
//                              avtMiliFileFormat.C                          //
// ************************************************************************* //

#include <avtMiliFileFormat.h>

#include <limits>
#include <visitstream.h>

#include <vtkCellData.h>
#include <vtkCellTypes.h>
#include <vtkElementLabelArray.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkFloatArray.h>

#include <avtCallback.h>
#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtVariableCache.h>
#include <avtUnstructuredPointBoundaries.h>
#include <avtGhostData.h>
#include <avtMaterial.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>
#include <UnexpectedValueException.h>

#include <TimingsManager.h>

using namespace rapidjson;


// ****************************************************************************
//  Constructor:  avtMiliFileFormat::avtMiliFileFormat
//
//  This method was re-written from its original version. 
//
//  Arguments:
//    fpath      The path to a .mili json file. 
//
//  Programmer:  Alister Maguire
//  Creation:    Jan 15, 2019
//
//  Modifications
//     
// ****************************************************************************

avtMiliFileFormat::avtMiliFileFormat(const char *fpath)
    : avtMTMDFileFormat(fpath)
{
    dims       = 0;
    nDomains   = 0;
    nMeshes    = 0;
    nTimesteps = 0;

    LoadMiliInfoJson(fpath);

    std::string fnamePth(fpath);

    //TODO: make sure this works for windows paths as well. 
    size_t fNPos       = fnamePth.find_last_of("\\/");
    std::string pthTmp = fnamePth.substr(0, fNPos + 1);
    std::string root   = fnamePth.substr(fNPos + 1);

    //
    // Set the family path.
    //
    size_t pSize = pthTmp.size();
    fampath      = new char[pSize + 1];
    strcpy(fampath, pthTmp.c_str());
    fampath[pSize] = '\0';

    //
    // Extract and set the family root. 
    //
    std::string lastSub = "";
    size_t lastPos      = root.find_last_of(".");
    lastSub             = root.substr(lastPos + 1);
    std::string mExt    = "mili";

    if (lastSub == mExt)
    {
        root = root.substr(0, lastPos);
    }

    size_t rSize = root.size();
    famroot      = new char[rSize + 1];
    strcpy(famroot, root.c_str());
    famroot[rSize] = '\0';
}


// ****************************************************************************
//  Destructor:  avtMiliFileFormat::~avtMiliFileFormat
//
//  Programmer:  Alister Maguire
//  Creation:    Jan 16, 2019
//
//  Modifications:
//
// ****************************************************************************

avtMiliFileFormat::~avtMiliFileFormat()
{
    //
    // Close the open databases. 
    //
    for (int i = 0; i < nDomains; ++i)
    {
        if (dbid[i] != -1)
        {
            mc_close(dbid[i]);
            dbid[i] = -1;
        }
    }

    //
    // Delete whatever datasets we have created. 
    //
    for (int i = 0; i < datasets.size(); ++i)
    {
        for (int j = 0; j < datasets[i].size(); ++j)
        {
            if (datasets[i][j] != NULL)
            {
                datasets[i][j]->Delete();
                datasets[i][j] = NULL;
            }
        }
    }
    datasets.clear();

    //
    // Delete whatever materials we have created. 
    //
    for (int i = 0; i < materials.size(); ++i)
    {
        for (int j = 0; j < materials[i].size(); ++j)
        {
            if (materials[i][j])
            {
                delete materials[i][j];
                materials[i][j] = NULL;
            }
        }
    }
    materials.clear();

    //
    // Reset flags to indicate the meshes needs to be read in again.
    //
    for (int i = 0; i < nDomains; ++i)
    {
        meshRead[i] = false;
    }

    //
    // Delete the mili meta data. 
    //
    for (int i = 0; i < nMeshes; ++i)
    {
        if (miliMetaData[i] != NULL)
        {
            delete miliMetaData[i];
        }
    }
    delete [] miliMetaData;

    if (famroot != NULL)
    {
        delete [] famroot;
    }

    if (fampath != NULL)
    {
        delete [] fampath;
    }
}


// ****************************************************************************
//  Function: ReadMiliResults
//
//  Purpose:
//      A wrapper around mc_read_results that handles multiple types (floats,
//      doubles, etc.).
//
//  Arguments:
//      dbid         The database id. 
//      ts           The desired timestate. 
//      srId         The subrecord ID. 
//      numVars      The number of requested state variables. 
//      shortNames   The state variable short names. 
//      vType        The variable type (int/double/etc.).
//      dataSize     The size of the data to be retrieved. 
//      dataBuffer   The buffer store the retrieved data. 
//
//  Notes:
//      This method is largely based off of the one originally 
//      written by Hank Childs in 2004. 
//
//  Programmer: Alister Maguire
//  Creation:   April 9, 2019
//
//  Modifications:
//
// ****************************************************************************

static void
ReadMiliResults(Famid  &dbid, 
                int     ts, 
                int     srId, 
                int     numVars,
                char  **shortNames, 
                int     vType, 
                int     dataSize, 
                float  *dataBuff)
{
    void *readBuff = NULL;
    switch (vType)
    {
        case M_STRING:
        {
            readBuff = new char[dataSize];
            break;
        }
        case M_FLOAT:
        {
            //
            // Intentional fallthrough.
            //
        }
        case M_FLOAT4:
        {
            readBuff = dataBuff;
            break;
        }
        case M_FLOAT8:
        {
            readBuff = new double[dataSize];
            break;
        }
        case M_INT:
        {
            //
            // Intentional fallthrough.
            //
        }
        case M_INT4:
        {
            readBuff = new int[dataSize];
            break;
        }
        case M_INT8:
        {
            readBuff = new long[dataSize];
            break;
        }
    }
    
    int rval = mc_read_results(dbid, ts, srId, numVars, 
                               shortNames, readBuff);

    if (rval != OK)
    {
        EXCEPTION1(InvalidVariableException, shortNames[0]);
    }

    char   *cTmp = NULL;
    double *dTmp = NULL;
    int    *iTmp = NULL;
    long   *lTmp = NULL;
    switch (vType)
    {
       case M_STRING:
       {
           cTmp = (char *) readBuff;
           for (int i = 0 ; i < dataSize ; i++)
           {
               dataBuff[i] = (float)(cTmp[i]);
           }
           delete [] cTmp;
           break;
       }
       case M_FLOAT:
       {
            //
            // Intentional fallthrough.
            //
       }
       case M_FLOAT4:
       {
         break;
       }
       case M_FLOAT8:
       {
           dTmp = (double *) readBuff;
           for (int i = 0 ; i < dataSize ; i++)
           {
               dataBuff[i] = (float)(dTmp[i]);
           }
           delete [] dTmp;
           break;
       }
       case M_INT:
       {
            //
            // Intentional fallthrough.
            //
       }
       case M_INT4:
       {
         iTmp = (int *) readBuff;
         for (int i = 0 ; i < dataSize ; i++)
         {
             dataBuff[i] = (float)(iTmp[i]);
         }
         delete [] iTmp;
         break;
       }
       case M_INT8:
       {
           lTmp = (long *) readBuff;
           for (int i = 0 ; i < dataSize ; i++)
           {
               dataBuff[i] = (float)(lTmp[i]);
           }
           delete [] lTmp;
           break;
       }
       default:
       {
           break;
       }
    }
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::ReadMiliVarToBuffer
//
//  Purpose:
//      Read mili variable data into a given buffer. 
//
//      IMPORTANT: The caller is responsible for all memory management of 
//      the buffer. This includes allocating the appropriate amount of
//      space to hold the variable data. 
//
//  Arguments:
//      varName      The variable's short name. 
//      SRIds        The variable's subrecord ids. 
//      SRInfo       The subrecord info that includes this variable. 
//      start        The start position of the data. 
//      vType        The type of value (int, float, etc). 
//      varSize      The size of the variable (vec vs single, etc).
//      ts           The timestep (in mili format => one based). 
//      dom          The variable's domain.
//      dataBuffer   The buffer to read into. 
//
//  Programmer:  Alister Maguire
//  Creation:    Feb 25, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtMiliFileFormat::ReadMiliVarToBuffer(char *varName,
                                       const intVector &SRIds,
                                       const SubrecInfo &SRInfo, 
                                       int start,
                                       int vType,
                                       int varSize,
                                       int ts,
                                       int dom,
                                       float *dataBuffer)
{
    //
    // Loop over the subrecords, and retreive the variable
    // data from mili. 
    //
    for (int i = 0 ; i < SRIds.size(); i++)
    {
        int nTargetCells = 0;
        int SRId         = SRIds[i];
        nTargetCells     = SRInfo.nElements[SRId];
        
        //
        // Simple read in: one block 
        //
        if (SRInfo.nDataBlocks[SRId] == 1)
        {
            //
            // Adjust start
            //
            start         += (SRInfo.dataBlockRanges[SRId][0] - 1);
            int resultSize = nTargetCells * varSize;
            float *dbPtr   = dataBuffer;

            ReadMiliResults(dbid[dom], ts, SRId,
                1, &varName, vType, resultSize, 
                dbPtr + (start * varSize));
        }
        else if (SRInfo.nDataBlocks[SRId] > 1)
        {
            int nBlocks               = SRInfo.nDataBlocks[SRId];
            const intVector *blocks   = &SRInfo.dataBlockRanges[SRId];

            int totalBlocksSize = 0;
            for (int b = 0; b < nBlocks; ++b)
            {
                int start = (*blocks)[b * 2];
                int stop  = (*blocks)[b * 2 + 1]; 
                totalBlocksSize += stop - start + 1;
            }

            float MBBuffer[totalBlocksSize * varSize];
            int resultSize  = totalBlocksSize * varSize;

            ReadMiliResults(dbid[dom], ts, SRId,
                1, &varName, vType, resultSize, MBBuffer);

            float *MBPtr = MBBuffer;

            //
            // Fill up the blocks into the array.
            //
            
            for (int b = 0; b < nBlocks; ++b)
            {
                for (int c = (*blocks)[b * 2] - 1; 
                     c <= (*blocks)[b * 2 + 1] - 1; ++c)
                {
                    for (int k = 0; k < varSize; ++k)
                    {
                        int idx = (varSize * (c + start)) + k;
                        dataBuffer[idx] = *(MBPtr++);
                    }
                }
            }
        }
        else
        {
            debug1 << "ReadMiliVarToBuffer found no data blocks?!?!";
        }
    }
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh for timestep 'ts' and domain 'dom'.
//
//  Arguments:
//    ts         the time step
//    dom        the domain
//    mesh       the name of the mesh to read
//
//  Programmer:  Alister Maguire
//  Creation:    Jan 16, 2019
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtMiliFileFormat::GetMesh(int ts, int dom, const char *mesh)
{
    int gmTimer = visitTimer->StartTimer();

    //
    // The valid meshnames are meshX or sand_meshX, where X is an int > 0.
    // We need to verify the name, and get the meshId.
    //
    bool isSandMesh = false;
    if (strstr(mesh, "sand_mesh") == mesh)
    {
        isSandMesh = true;
    }
    else if (strstr(mesh, "mesh") != mesh)
    {
        EXCEPTION1(InvalidVariableException, mesh);
    }
    
    char *check = 0;
    int meshId;
    int offset = 4;
    if (isSandMesh)
    {
        offset = 9;
    }

    //
    // Do a checked conversion to integer.
    //
    meshId = (int) strtol(mesh + offset, &check, 10);
    if (meshId == 0 || check == mesh + offset)
    {
        EXCEPTION1(InvalidVariableException, mesh)
    }
    --meshId;

    //
    // Actually read in the mesh if we haven't already.  
    //
    if (!meshRead[dom])
    {
        ReadMesh(dom);
    }

    if (!miliMetaData[meshId]->ContainsSand())
    {
        debug1 << "The user has requested a sand mesh that doesn't exist!"
               << "This shouldn't be possible...";
        char msg[1024];
        sprintf(msg, "Cannot view a non-existent sand mesh!");
        EXCEPTION1(ImproperUseException, msg);
    }

    //
    // The connectivity does not change over time, so use the one we have
    // already calculated.
    //
    vtkUnstructuredGrid *rv = vtkUnstructuredGrid::New();
    rv->ShallowCopy(datasets[dom][meshId]);

    //
    // The node positions are stored in 'nodpos'.
    //
    char npChar[128];
    sprintf(npChar, "nodpos");
    char *npCharPtr = (char *)npChar;

    MiliVariableMetaData *nodpos = miliMetaData[meshId]->
        GetVarMDByShortName("nodpos", "node");

    int nNodes   = miliMetaData[meshId]->GetNumNodes(dom);
    int nPts     = dims * nNodes;
    int subrec   = -1;
    int vType    = M_FLOAT;

    if (nodpos != NULL)
    {
        vType  = nodpos->GetNumType();
        subrec = nodpos->GetSubrecIds(dom)[0];
        if (subrec == -1)
        {
            if (rv->GetPoints() == 0)
            {
                debug1 << "Unable to find coords for domain " << dom;

                //
                // null out the returned grid
                //
                rv->Delete();
                rv = vtkUnstructuredGrid::New();
                return rv;
            }
        }
        else
        {
            float fPts[nPts];
            ReadMiliResults(dbid[dom], ts+1, subrec, 1, 
                &npCharPtr, vType, nPts, fPts);

            vtkPoints *vtkPts = vtkPoints::New();
            vtkPts->SetNumberOfPoints(nNodes);

            float *vtkPtsPtr = (float *) vtkPts->GetVoidPointer(0);
            float *fPtsPtr   = fPts; 

            for (int pt = 0 ; pt < nNodes; pt++)
            {
                *(vtkPtsPtr++) = *(fPtsPtr++);
                *(vtkPtsPtr++) = *(fPtsPtr++);
                if (dims >= 3)
                {
                    *(vtkPtsPtr++) = *(fPtsPtr++);
                }
                else
                {
                    *(vtkPtsPtr++) = 0.;
                }
            }

            rv->SetPoints(vtkPts);
            vtkPts->Delete();

            //
            // Only ghost out sand nodes if the mesh isn't the sand mesh. 
            //
            if (!isSandMesh)
            {
                SubrecInfo SRInfo = miliMetaData[meshId]->GetSubrecInfo(dom);
                int numVars       = miliMetaData[meshId]->GetNumVariables();
                int nCells        = miliMetaData[meshId]->GetNumCells(dom);
                int nNodes        = miliMetaData[meshId]->GetNumNodes(dom);
                
                float sandBuffer[nCells];

                //
                // Begin by assuming the status of every cell is good. 
                //
                for (int i = 0; i < nCells; ++i)
                {
                    sandBuffer[i] = 1.0;
                }

                //
                // Because sand can appear on multiple variables, we need
                // to check them all and populate the buffer iteratively.  
                //
                for (int i = 0 ; i < numVars; i++)
                {
                    MiliVariableMetaData *varMD = miliMetaData[meshId]->
                        GetVarMDByIdx(i);

                    if (varMD == NULL)
                    {
                        continue;
                    }
                    
                    if (varMD->IsSand())
                    {
                        avtCentering centering = varMD->GetCentering();

                        if (centering != AVT_ZONECENT)
                        {
                            debug1 << "Sanded variable is not " << 
                                "zone centered?!?" << endl;
                            continue;
                        }

                        std::string varName = varMD->GetShortName();
                        intVector SRIds     = varMD->GetSubrecIds(dom);
                        int vType           = varMD->GetNumType();

                        std::string className = varMD->GetClassShortName(); 
                        int start = miliMetaData[meshId]->
                            GetClassMDByShortName(className.c_str())->
                            GetConnectivityOffset(dom);                                   

                        //
                        // Create a copy of our name to pass into mili. 
                        //
                        char charName[1024];
                        sprintf(charName, varName.c_str());
                        char *namePtr = (char *) charName;

                        ReadMiliVarToBuffer(namePtr, SRIds, SRInfo, start,
                            vType, 1, ts + 1, dom, sandBuffer);
                    }
                }

                vtkUnsignedCharArray *ghostNodes = vtkUnsignedCharArray::New();
                ghostNodes->SetName("avtGhostNodes");
                ghostNodes->SetNumberOfTuples(nNodes);

                unsigned char *ghostNodePtr = ghostNodes->GetPointer(0);

                for (int i = 0; i < nNodes; ++i)
                {
                    ghostNodePtr[i] = 0;
                    avtGhostData::AddGhostNodeType(ghostNodePtr[i], 
                        NODE_NOT_APPLICABLE_TO_PROBLEM);
                }
     
                //
                // FYI: sand elements are those that have been 
                // "destroyed" during the simulation. We ghost them
                // out by default. 
                //
                for (int i = 0; i < nCells; ++i)
                {
                    //
                    // Element status > .5 is good. 
                    //
                    if (sandBuffer[i] > 0.5)
                    {
                        vtkIdType nCellPts = 0;
                        vtkIdType *cellPts = NULL;

                        rv->GetCellPoints(i, nCellPts, cellPts);
                        
                        if (nCellPts && cellPts)
                        {
                            for (int j = 0; j < nCellPts; ++j)
                            {
                                avtGhostData::RemoveGhostNodeType(
                                    ghostNodePtr[cellPts[j]],
                                    NODE_NOT_APPLICABLE_TO_PROBLEM);
                            }
                        }
                    }
                }
          
                rv->GetPointData()->AddArray(ghostNodes);
                ghostNodes->Delete();
            }
        }
    }
    else
    {
        //
        // We can arrive here if there are no nodal positions results
        // but we have initial mesh positions from reading the mesh
        // header information (mc_load_nodes).
        //
        if (rv->GetPoints() == 0)
        {
            debug1 << "Unable to find coords for domain " << dom;

            //
            // null out the returned grid
            //
            rv->Delete();
            rv = vtkUnstructuredGrid::New();
            return rv;
        }
    }
    visitTimer->StopTimer(gmTimer, "MILI: Getting Mesh");
    visitTimer->DumpTimings();

    return rv;
}


// ****************************************************************************
//  Method: avtMiliFileFormat::ExtractMeshIdFromPath
//
//  Purpose:
//
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019 
//
// ****************************************************************************

int
avtMiliFileFormat::ExtractMeshIdFromPath(const std::string &varPath)
{
    if (nMeshes > 1)
    {
        size_t varPos     = varPath.find("mesh");
        std::string first = varPath.substr(varPos, 5);
        char cNum         = first.back();
        return (int) (cNum - '0');
    }
    return 0;
}


// ****************************************************************************
//  Method: avtMiliFileFormat::OpenDB
//
//  Purpose:
//      Open up a family database for a given domain.
//
//  Programmer: Alister Maguire
//  Creation:   Jan 16, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtMiliFileFormat::OpenDB(int dom)
{
    int openDBTimer = visitTimer->StartTimer();

    //TODO: do the filenames still have this much variety?
    char const * const root_fmtstrs[] = {
        "%s%.3d",
        "%s%.4d",
        "%s%.5d",
        "%s%.6d",
    };

    char rFlag[] = "r";

    if (dbid[dom] == -1)
    {
        int rval;
        if (nDomains == 1)
        {
            debug3 << "Attempting mc_open on root=\"" << famroot << 
                "\", path=\"" << fampath << "\"." << endl;

            rval = mc_open(famroot, fampath, rFlag, &(dbid[dom]) );

            if ( rval != OK )
            {
                EXCEPTION1(InvalidFilesException, famroot);
            }
        }
        else
        {
            char famname[128];
            for (int i = 0; i < 4; i++)
            {
                sprintf(famname, root_fmtstrs[i], famroot, dom);
                debug3 << "Attempting mc_open on root=\"" << famname 
                    << "\", path=\"" << fampath << "\"." << endl;

                rval = mc_open(famname, fampath, rFlag, &(dbid[dom]) );
                if (rval == OK) 
                {
                    break;
                }
            }
            if ( rval != OK )
            {
                EXCEPTION1(InvalidFilesException, famname);
            }
        }
    }

    visitTimer->StopTimer(openDBTimer, "MILI: Opening database");
}


// ****************************************************************************
//  Method: avtMiliFileFormat::ReadMesh
//
//  Purpose:
//      Read the given domain of the mesh. 
//
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications
//
// ****************************************************************************

void
avtMiliFileFormat::ReadMesh(int dom)
{
    int readMeshTimer = visitTimer->StartTimer();

    if (dbid[dom] == -1)
    {
        OpenDB(dom);
    }

    //
    // Read in the meshes.
    //
    for (int meshId = 0; meshId < nMeshes; ++meshId)
    {
        //
        // Perform an mc call to retrieve the number of nodes
        // on this domain, and update our meta data. 
        //
        char nodeSName[] = "node";
        int classIdx     = 0;
        int nNodes       = 0;
        char shortName[1024];
        char longName[1024];

        int rval = mc_get_class_info(dbid[dom], meshId, M_NODE,
             classIdx, shortName, longName, &nNodes);

        if (rval != OK)
        {
            char msg[1024];
            sprintf(msg, "Unable to retrieve %s from mili", shortName);
            EXCEPTION1(ImproperUseException, msg);
        }

        miliMetaData[meshId]->SetNumNodes(dom, nNodes);

        //
        // Might as well retrieve the node label info here. Note 
        // that this does NOT create the labels, as that tends 
        // to be very expensive. We save that for when requested. 
        //
        RetrieveNodeLabelInfo(meshId, nodeSName, dom);

        //
        // Read initial nodal position information, if available. 
        //
        vtkPoints *vtkPts = vtkPoints::New();
        vtkPts->SetNumberOfPoints(nNodes);
        float *vtkPtsPtr  = (float *) vtkPts->GetVoidPointer(0);

        if (mc_load_nodes(dbid[dom], meshId, nodeSName, vtkPtsPtr) == OK)
        {
            if (dims == 2)
            {
                 //
                 // We need to insert zeros if we're in 2D
                 //
                 for (int p = nNodes - 1; p >= 0; p--)
                 {
                     int q = p*3, r = p*2;
                     //
                     // Store the coordinates in reverse so we 
                     // don't mess up at node 1.
                     //
                     vtkPtsPtr[q+2] = 0.0;
                     vtkPtsPtr[q+1] = vtkPtsPtr[r+1];
                     vtkPtsPtr[q+0] = vtkPtsPtr[r+0];
                 }
             }
        }
        else
        {
            vtkPts->Delete();
            vtkPts = NULL;
        }

        //
        // Mili has its own definitions for cell types. The
        // heirarchy is CellType->ClassType->...
        // We need to extract data one Class type at a time. 
        //
        intVector numClassesPerCellType;
        intVector miliCellTypes;
        miliMetaData[meshId]->GetCellTypeCounts(
            miliCellTypes, numClassesPerCellType);

        int numCellTypes = miliCellTypes.size();

        //
        // Now determine the number of cells for each class and 
        // get some basic info for future mc calls. 
        //
        int offset    = 0;
        int nDomCells = 0;
        intVector    numCellsPerClass;
        stringVector classNames;
        for (int i = 0 ; i < numCellTypes ; i++)
        {
            for (int j = 0 ; j < numClassesPerCellType[i]; j++)
            {
                int nCells = 0;
                char shortName[1024];
                char longName[1024];

                int rval = mc_get_class_info(dbid[dom], meshId, 
                                             miliCellTypes[i], j,
                                             shortName, longName, &nCells);

                //
                // If we're working with multi-domain data, the current 
                // cell type might not exist on this domain. 
                //
                if (rval != OK)
                {
                    numClassesPerCellType[i] = 0;
                    continue; 
                }

                numCellsPerClass.push_back(nCells);
                classNames.push_back(std::string(shortName));
                nDomCells += nCells;

                miliMetaData[meshId]->GetClassMDByShortName(shortName)->
                    SetConnectivityOffset(dom, offset);
                miliMetaData[meshId]->GetClassMDByShortName(shortName)->
                    SetNumElements(dom, nCells);
                offset += nCells;
            }
        }

        // TODO: should we set the num elements for classes not set
        // above? (element sets, mat variables, glob variables...)
        miliMetaData[meshId]->SetNumCells(dom, nDomCells);
       
        //
        // Allocate an appropriately sized dataset using that connectivity
        // information.
        //
        datasets[dom][meshId] = vtkUnstructuredGrid::New();
        datasets[dom][meshId]->Allocate(nDomCells);

        //
        // Now that we have the needed counts, retrieve the connectivity
        // and build our dataset. 
        //
        int cellIdx = 0;
        int cpcIdx  = 0;
        int matList[nDomCells];

        for (int i = 0; i < numCellTypes; ++i)
        {
            for (int j = 0; j < numClassesPerCellType[i]; ++j)
            {
                //
                // class_conns is a mili defined array which maps mili
                // cell types to their number of nodes. 
                //
                int connCount = class_conns[miliCellTypes[i]];
                int nCells    = numCellsPerClass[cpcIdx];

                //
                // Load all of the info for a single mili cell type. 
                //
                // conn: list of nodes defining each element.
                // mat:  materials that each element belongs to. 
                // part: Not used...
                //
                int conn[nCells * connCount];
                int mats[nCells];
                int part[nCells];
                char shortName[classNames[cpcIdx].size() + 1];
                strcpy(shortName, classNames[cpcIdx].c_str());
                cpcIdx++;

                int rval = mc_load_conns(dbid[dom], meshId, 
                    shortName, conn, mats, part);

                if (rval != OK)
                {
                    EXCEPTION1(InvalidVariableException, shortName);
                }

                //
                // Create a conn pointer that we can move around
                // in memory without offsetting the original.
                //
                int *connPtr = conn;
    
                for (int k = 0 ; k < nCells ; ++k)
                {
                    matList[cellIdx] = mats[k];
                    cellIdx++;

                    vtkIdType verts[100];

                    for(int cc = 0; cc < connCount; ++cc)
                    {
                        verts[cc] = (vtkIdType)connPtr[cc];
                    }
                    
                    switch (miliCellTypes[i])
                    {
                        case M_TRUSS:
                        {
                            datasets[dom][meshId]->InsertNextCell(VTK_LINE,
                                                     connCount, verts);
                            break;
                        }
                        case M_BEAM:
                        {
                            //
                            // Beams are lines that have a third point to 
                            // define the normal. Since we don't need to 
                            // visualize it, we just drop the normal point.
                            //
                            datasets[dom][meshId]->InsertNextCell(VTK_LINE,
                                                     2, verts);
                            break;
                        }
                        case M_TRI:
                        {
                            datasets[dom][meshId]->InsertNextCell(VTK_TRIANGLE,
                                                     connCount, verts);
                            break;
                        }
                        case M_QUAD:
                        {
                            datasets[dom][meshId]->InsertNextCell(VTK_QUAD,
                                                     connCount, verts);
                            break;
                        }
                        case M_TET:
                        {
                            datasets[dom][meshId]->InsertNextCell(VTK_TETRA,
                                                     connCount, verts);
                            break;
                        }
                        case M_PYRAMID:
                        {
                            datasets[dom][meshId]->InsertNextCell(VTK_PYRAMID,
                                                     connCount, verts);
                            break;
                        }
                        case M_WEDGE:
                        {
                            datasets[dom][meshId]->InsertNextCell(VTK_WEDGE,
                                                     connCount, verts);
                            break;
                        }
                        case M_HEX:
                        {
                            //
                            // If all of the node ids are equal, this is 
                            // actually a particle (Mili hack). 
                            //
                            bool isParticle = true;
                            int prevNode    = connPtr[0];
                            for (int nIdx = 0; nIdx < 8; ++nIdx)
                            {
                                if (prevNode != connPtr[nIdx])
                                {
                                    isParticle = false;
                                    break;
                                }
                                prevNode = connPtr[nIdx];
                            }
                            
                            if (isParticle)
                            {
                                vtkIdType vert[1];
                                vert[0] = connPtr[0];
                                datasets[dom][meshId]->InsertNextCell(
                                                         VTK_VERTEX,
                                                         1, vert);
                            }
                            else if (connPtr[2] == connPtr[3] && 
                                     connPtr[4] == connPtr[5] &&
                                     connPtr[5] == connPtr[6] && 
                                     connPtr[6] == connPtr[7])
                            {
                                vtkIdType tet[4];
                                tet[0] = verts[0]; tet[1] = verts[1];
                                tet[2] = verts[2]; tet[3] = verts[4];
                                datasets[dom][meshId]->InsertNextCell(
                                                         VTK_TETRA,
                                                         4, tet);
                            }
                            else
                            {
                                datasets[dom][meshId]->InsertNextCell(
                                                         VTK_HEXAHEDRON,
                                                         connCount, verts);
                            }
                            break;
                        }
                        case M_PARTICLE:
                        {
                            datasets[dom][meshId]->InsertNextCell(
                                                     VTK_VERTEX,
                                                     connCount, verts);
                            break;
                        }
                        default:
                        {
                            debug1 << "Unable to add cell of type "
                                << miliCellTypes[i] << endl;
                            break;
                        }
                    }
                    connPtr += connCount;
                }

                //
                // Retrieve label info for this class. 
                //
                RetrieveCellLabelInfo(meshId, shortName, dom, 
                                      nCells);
            }
        }

        //
        // Create our avtMaterial. 
        //
        int numMats = miliMetaData[meshId]->GetNumMaterials();
        stringVector matNames;
        miliMetaData[meshId]->GetMaterialNames(matNames);
        avtMaterial *avtMat = new avtMaterial(numMats, matNames,
                                              nDomCells, matList, 0, 
                                              NULL, NULL, 
                                              NULL, NULL);

        materials[dom][meshId] = avtMat;

        //
        // Hook up points to mesh if we have 'em
        //
        if (vtkPts)
        {
            datasets[dom][meshId]->SetPoints(vtkPts);
            vtkPts->Delete();
        }

        //
        // Grab the subrecord info while we're here. 
        //
        PopulateSubrecordInfo(dom, meshId);

    }// end mesh reading loop

    meshRead[dom] = true;

    visitTimer->StopTimer(readMeshTimer, "MILI: Reading Mesh");
}


// ****************************************************************************
//  Method: avtMiliFileFormat::PopulateSubrecordInfo
//
//  Purpose:
//      Retrieve the needed information from the subrecords, and
//      store it for later requests. 
//
//  Programmer: Alister Maguire
//  Creation:   January 28, 2019 
//
//  Modifications:
//
// ****************************************************************************

void
avtMiliFileFormat::PopulateSubrecordInfo(int dom, int meshId)
{
    int popSubrecTimer = visitTimer->StartTimer();

    int rval;
    int srec_qty = 0;
    rval = mc_query_family(dbid[dom], QTY_SREC_FMTS, NULL, NULL,
                           (void*) &srec_qty);

    if (rval != OK)
    {
        debug1 << "Cannot query QTY_SREC_FMTS! This is bad..." << endl;
        return;
    }
    else if (srec_qty > 1) 
    {
        //
        // According to Kevin, the state record format count is never
        // actually greater than 1. 
        //
        debug1 << "Encountered multiple state record formats! Is this in "
               << "use now???";
        char msg[1024];
        sprintf(msg, "The Mili plugin is not set-up to handle multiple ",
            "state record formats. Bailing.");
        EXCEPTION1(ImproperUseException, msg);
    }

    int sRFId     = 0;
    int substates = 0;
    rval = mc_query_family(dbid[dom], QTY_SUBRECS, (void *) &sRFId, NULL,
                           (void *) &substates);
    
    if (rval != OK)
    {
        debug1 << "Cannot query QTY_SUBRECS! This is bad..." << endl;
        return;
    }

    //
    // This seems like it should be terrible in performance, but it's
    // actually pretty fast. 
    //
    for (int srId = 0 ; srId < substates ; srId++)
    {
        Subrecord sr;
        memset(&sr, 0, sizeof(sr));
        rval = mc_get_subrec_def(dbid[dom], sRFId, srId, &sr);

        if (rval != OK)
        {
            continue;
        }
        
        for (int varIdx = 0 ; varIdx < sr.qty_svars ; varIdx++)
        {
             State_variable sv;
             memset(&sv, 0, sizeof(sv));

             mc_get_svar_def(dbid[dom], sr.svar_names[varIdx], &sv);
             
             //
             // Look for this variable in our metadata. 
             //
             int MDVarIdx = miliMetaData[meshId]->
                 GetVarMDIdxByShortName(sv.short_name, sr.class_name); 

             if (MDVarIdx != -1)
             {
                 //
                 // Add the subrecord info to our metadata. 
                 //
                 miliMetaData[meshId]->AddVarSubrecInfo(MDVarIdx,
                                                        dom,
                                                        srId,
                                                        &sr);
             }

             //
             // Mili calls needed to free memory. 
             //
             mc_cleanse_st_variable(&sv);
        }

        //
        // Mili calls needed to free memory. 
        //
        mc_cleanse_subrec(&sr);
    }

    visitTimer->StopTimer(popSubrecTimer, "MILI: PopulateSubrecordInfo");
}

                
// ****************************************************************************
//  Method:  avtMiliFileFormat::GetVar
//
//  Purpose:
//      Gets variable 'var' for timestep 'ts'.
//
//  Arguments:
//    ts         the time step
//    var        the name of the variable to read
//
//    Note: zone and node label handling was only slightly refactored from 
//          it's original implementation by Matt Larsen. 
//
//  Programmer:  Alister Maguire
//  Creation:    Jan 16, 2019
//
//  Modifications
//
// ****************************************************************************

vtkDataArray *
avtMiliFileFormat::GetVar(int ts, int dom, const char *varPath)
{
    int gvTimer = visitTimer->StartTimer();

    int meshId  = ExtractMeshIdFromPath(varPath);

    //
    // Labels are special cases that we need to handle 
    // separately. 
    //
    if (strcmp("OriginalZoneLabels", varPath) == 0)
    {
        int maxSize = miliMetaData[meshId]->GetMaxZoneLabelLength(dom) + 1;

        vtkElementLabelArray *vtkLabels = 0;
        vtkLabels = vtkElementLabelArray::New();
        vtkLabels->SetNumberOfComponents(maxSize);

        const stringVector *miliZoneLabels = 
            miliMetaData[meshId]->GetZoneBasedLabelsPtr(dom);

        const int nEls = miliZoneLabels->size();
        vtkLabels->SetNumberOfTuples(nEls);

        char *ptr = (char *) vtkLabels->GetVoidPointer(0);

        for(int i = 0; i < nEls; ++i)
        {
            const int offset     = i * maxSize;
            const char * elLabel = (*miliZoneLabels)[i].c_str();
            const int cSize      = (*miliZoneLabels)[i].size();
            for(int j = 0; j < maxSize; ++j)
            {   
                if(j < cSize)
                {
                    ptr[offset + j] = elLabel[j];
                }
                else
                {
                    ptr[offset + j] = '\0';
                }
            }
        }
        
        //
        // Add the data so we can do reverse lookups
        //
        std::vector<MiliClassMetaData *> cellMD;
        miliMetaData[meshId]->GetCellBasedClassMD(cellMD);

        std::vector<MiliClassMetaData *>::iterator mdItr;
        for (mdItr = cellMD.begin(); mdItr != cellMD.end(); ++mdItr)
        {
            const LabelPositionInfo *posInfo = 
                (*mdItr)->GetLabelPositionInfoPtr(dom);
            vtkLabels->AddName((*mdItr)->GetShortName(),
                               posInfo->rangesBegin,
                               posInfo->rangesEnd,
                               posInfo->idsBegin,
                               posInfo->idsEnd);
        }

        return vtkLabels;

    }

    else if (strcmp("OriginalNodeLabels", varPath) == 0)
    {
        int maxSize = miliMetaData[meshId]->GetMaxNodeLabelLength(dom) + 1;
 
        vtkElementLabelArray *vtkLabels = 0;
        vtkLabels = vtkElementLabelArray::New();
        vtkLabels->SetNumberOfComponents(maxSize);

        const stringVector *miliNodeLabels = 
            miliMetaData[meshId]->GetNodeBasedLabelsPtr(dom);

        const int nEls = (*miliNodeLabels).size();
        vtkLabels->SetNumberOfTuples(nEls);

        char * ptr = (char *) vtkLabels->GetVoidPointer(0);

        for(int i = 0; i < nEls; ++i)
        {
            const int offset     = i * maxSize;
            const char *elLabel  = (*miliNodeLabels)[i].c_str();
            const int cSize      = (*miliNodeLabels)[i].size();
            for(int j = 0; j < maxSize; ++j)
            {   
                if(j < cSize)
                {
                    ptr[offset + j] = elLabel[j];
                }
                else
                {
                    ptr[offset + j] = '\0';
                }
            }
        }

        //
        // Add the data so we can do reverse lookups
        //
        std::vector<MiliClassMetaData *> nodeMD;
        miliMetaData[meshId]->GetNodeBasedClassMD(nodeMD);

        std::vector<MiliClassMetaData *>::iterator mdItr;
        for (mdItr = nodeMD.begin(); mdItr != nodeMD.end(); ++mdItr)
        {
            const LabelPositionInfo *posInfo = 
                (*mdItr)->GetLabelPositionInfoPtr(dom);
            vtkLabels->AddName((*mdItr)->GetShortName(),
                               posInfo->rangesBegin,
                               posInfo->rangesEnd,
                               posInfo->idsBegin,
                               posInfo->idsEnd);
        }

        return vtkLabels;
    }

    //
    // We need to first retrieve the meta data for our variable. 
    // NOTE: We need to get the MD by PATH instead of name because
    // variables can be associated with multiple classes. 
    //
    MiliVariableMetaData *varMD = 
        miliMetaData[meshId]->GetVarMDByPath(varPath);
    SubrecInfo SRInfo = miliMetaData[meshId]->GetSubrecInfo(dom);

    if (varMD == NULL)
    {
        EXCEPTION1(InvalidVariableException, varPath);
    }

    avtCentering centering = varMD->GetCentering();
    intVector SRIds        = varMD->GetSubrecIds(dom);
    int nSRs               = SRIds.size();
    int vType              = varMD->GetNumType();
    std::string vShortName = varMD->GetShortName();

    //
    // Create a copy of our name to pass into mili. 
    //
    char charName[1024];
    sprintf(charName, vShortName.c_str());
    char *namePtr = (char *) charName;

    vtkFloatArray *fltArray = NULL;

    if (centering == AVT_NODECENT)
    {
        //
        // Node centered variable. 
        //

        if (fltArray == 0)
        {
            int nNodes = miliMetaData[meshId]->GetNumNodes(dom);
            fltArray   = vtkFloatArray::New();
            fltArray->SetNumberOfTuples(nNodes);
 
            float *fArrPtr = (float *) fltArray->GetVoidPointer(0);

            ReadMiliResults(dbid[dom], ts+1, SRIds[0], 1,
                &namePtr, vType, nNodes, fArrPtr);
        }
        else
        {
            //
            // The reference count will be decremented by the generic database,
            // because it will assume it owns it.
            //
            fltArray->Register(NULL);
        }
    }
    else
    {
        //
        // Cell centered variable. 
        //

        fltArray   = vtkFloatArray::New();
        int nCells = miliMetaData[meshId]->GetNumCells(dom);
        fltArray->SetNumberOfTuples(nCells);
        float *dataBuffer = NULL;

        //
        // We have two special cases to consider.
        // Material variables: 
        //   We receive a single value for a given number of materials, 
        //   and we must apply each of those values to their respective 
        //   materials. 
        // Global variables:
        //   We recieve a single value that is applied to all cells. 
        //
        int dBuffSize = 0;
        bool isMatVar = varMD->IsMatVar();
        bool isGlobal = varMD->IsGlobal();
        if (isMatVar)
        {
            dBuffSize  = materials[dom][meshId]->GetNMaterials();
            dataBuffer = new float[dBuffSize]; 
        }
        else if (isGlobal)
        {
            dBuffSize  = 1;
            dataBuffer = new float[1]; 
        }
        else
        {
            dBuffSize  = nCells;
            dataBuffer = new float[dBuffSize];
        }

        //
        // If our variable doesn't cover the entire dataset, we want
        // the "empty space" to be rendered grey. Nan values will
        // be mapped to grey. 
        //
        for (int i = 0 ; i < dBuffSize; i++)
        {
            dataBuffer[i] = std::numeric_limits<float>::quiet_NaN();
        }

        //
        // Read the data into our buffer. 
        //
        std::string className = varMD->GetClassShortName(); 
        int start = miliMetaData[meshId]->
            GetClassMDByShortName(className.c_str())->
            GetConnectivityOffset(dom);                                   

        ReadMiliVarToBuffer(namePtr, SRIds, SRInfo, start, 
            vType, 1, ts + 1, dom, dataBuffer);

        if (isMatVar)
        {
            //
            // This is a material variable. We need to distribute the 
            // values across cells by material ID. 
            //
            const int *matList = materials[dom][meshId]->GetMatlist();

            for (int i = 0; i < nCells; ++i)
            { 
                fltArray->SetTuple1(i, dataBuffer[matList[i]]);
            }
        }
        else if (isGlobal)
        {
            //
            // This is a global var. Just apply it to all cells. 
            //
            for (int i = 0; i < nCells; ++i)
            { 
                fltArray->SetTuple1(i, dataBuffer[0]);
            }
        }
        else
        {
            //
            // Nothing special here. Just copy the values over. 
            //
            for (int i = 0; i < nCells; ++i)
            { 
                fltArray->SetTuple1(i, dataBuffer[i]);
            }
        }

        delete [] dataBuffer;
       
    }

    visitTimer->StopTimer(gvTimer, "MILI: GetVar");

    return fltArray;
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::GetVectorVar
//
//  Purpose:
//      Gets variable 'var' for timestep 'ts'.
//
//      Note: code for transferring symmetric tensors to normal
//            tensors has been kept as originally written by
//            Hank Childs in 2004. 
//
//  Arguments:
//    ts         the time step
//    var        the name of the variable to read
//
//  Programmer:  Alister Maguire
//  Creation:    Jan 16, 2019
//
//  Modifications
//
// ****************************************************************************

vtkDataArray *
avtMiliFileFormat::GetVectorVar(int ts, int dom, const char *varPath)
{
    int gvvTimer = visitTimer->StartTimer();

    int meshId   = ExtractMeshIdFromPath(varPath);
    
    //
    // NOTE: We need to get the MD by PATH instead of name because
    // variables can be associated with multiple classes. 
    //
    MiliVariableMetaData *varMD = miliMetaData[meshId]->
        GetVarMDByPath(varPath);
    SubrecInfo SRInfo = miliMetaData[meshId]->GetSubrecInfo(dom);

    if (varMD == NULL)
    {
        EXCEPTION1(InvalidVariableException, varPath);
    }

    avtCentering centering = varMD->GetCentering();
    intVector SRIds        = varMD->GetSubrecIds(dom);
    std::string vShortName = varMD->GetShortName();

    //
    // Component dimensions will only be > 1 if we have
    // an element set. 
    //
    int vecSize   = varMD->GetVectorSize();
    int compDims  = varMD->GetComponentDims();
    int dataSize  = vecSize * compDims;

    int vType     = varMD->GetNumType();
    bool isGlobal = varMD->IsGlobal();
    bool isES     = varMD->IsElementSet();

    //
    // Create a copy of our name to pass into mili. 
    //
    char charName[1024];
    sprintf(charName, vShortName.c_str());
    char *namePtr = (char *) charName;

    vtkFloatArray *fltArray = vtkFloatArray::New();
    fltArray->SetNumberOfComponents(vecSize);

    if (centering == AVT_NODECENT)
    {
        //
        // Node centered variable. 
        //
        
        if (SRIds.size() != 1)
        {
            EXCEPTION1(InvalidVariableException, varPath);
        }

        int nNodes = miliMetaData[meshId]->GetNumNodes(dom);

        fltArray->SetNumberOfTuples(nNodes);
        float *fArrPtr = (float *) fltArray->GetVoidPointer(0);
        
        ReadMiliResults(dbid[dom], ts+1, SRIds[0], 1,
            &namePtr, vType, nNodes * dataSize, fArrPtr);
    }
    else
    {
        //
        // Cell centered variable. 
        //

        int nCells    = miliMetaData[meshId]->GetNumCells(dom);
        int dBuffSize = 0;;

        fltArray->SetNumberOfTuples(nCells);

        float *dataBuffer = NULL;
        if (isGlobal)
        {
            //
            // A global vector will be a single vector of size dataSize. 
            //
            dBuffSize  = dataSize;
            dataBuffer = new float[dBuffSize]; 
        }
        else
        {
            dBuffSize  = nCells * dataSize;
            dataBuffer = new float[dBuffSize];
        }

        //
        // If our variable doesn't cover the entire dataset, we want
        // the "empty space" to be rendered grey. Nan values will
        // be mapped to grey. 
        //
        for (int i = 0 ; i < dBuffSize; i++)
        {
            dataBuffer[i] = std::numeric_limits<float>::quiet_NaN();
        }

        //
        // Read the data into our buffer. 
        //
        std::string className = varMD->GetClassShortName(); 
        int start = miliMetaData[meshId]->
            GetClassMDByShortName(className.c_str())->
            GetConnectivityOffset(dom);                                   

        ReadMiliVarToBuffer(namePtr, SRIds, SRInfo, start,
            vType, dataSize, ts + 1, dom, dataBuffer);

        //
        // If this is an element set, we need to extract the 
        // integration points, and copy then over to our 
        // VTK data array. 
        //
        if (isES)
        {
            //
            // Let's take the mid integration point for now.
            //
            int targetIP = (int)(compDims / 2);

            for (int i = 0 ; i < nCells; i++)
            {
                float vecPts[vecSize];

                //TODO: we should somehow store all values and let
                //      the user choose which one they want to display. 
                //
                // Element sets are specialized vectors such that each
                // element in the vector is a list of integration points. 
                // For now, we choose the middle integration point. 
                //
                for (int j = 0; j < vecSize; ++j)
                {
                    int idx   = (i * dataSize) + (j * compDims) + targetIP;
                    vecPts[j] = dataBuffer[idx];
                }

                fltArray->SetTuple(i, vecPts);
            }
        }
        else if (isGlobal)
        {
            //
            // This vector is global. Just apply it to every cell. 
            //
            float *fltArrayPtr = (float *) fltArray->GetVoidPointer(0);
            for (int i = 0; i < nCells; ++i)
            { 
                for (int j = 0; j < dataSize; ++j)
                {
                    fltArrayPtr[i*dataSize + j] = dataBuffer[j];
                }
            }
        }
        else
        {
            //
            // Nothing special here. Just copy over the elements. 
            //
            float *fltArrayPtr = (float *) fltArray->GetVoidPointer(0);
            for (int i = 0 ; i < dBuffSize; i++)
            {
                fltArrayPtr[i] = dataBuffer[i];
            }
        }

        delete [] dataBuffer;
    }

    //
    // If we have a symmetric tensor, put that in the form of a normal
    // tensor.
    //
    if (vecSize == 6)
    {
        vtkFloatArray *newFltArray = vtkFloatArray::New();
        int nTups = fltArray->GetNumberOfTuples();
        newFltArray->SetNumberOfComponents(9);
        newFltArray->SetNumberOfTuples(nTups);
        for (int i = 0 ; i < nTups ; i++)
        {
            double origVals[6];
            float newVals[9];
            fltArray->GetTuple(i, origVals);
            newVals[0] = origVals[0];  // XX
            newVals[1] = origVals[3];  // XY
            newVals[2] = origVals[5];  // XZ
            newVals[3] = origVals[3];  // YX
            newVals[4] = origVals[1];  // YY
            newVals[5] = origVals[4];  // YZ
            newVals[6] = origVals[5];  // ZX
            newVals[7] = origVals[4];  // ZY
            newVals[8] = origVals[2];  // ZZ
            newFltArray->SetTuple(i, newVals);
        }
        fltArray->Delete();
        fltArray = newFltArray;
    }

    visitTimer->StopTimer(gvvTimer, "MILI: GetVectorVar");

    return fltArray;
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::GetCycles
//
//  Purpose:
//      Returns the actual cycle numbers for each time step.
//
//  Arguments:
//   cycles      the output vector of cycle numbers 
//
//  Programmer:  Alister Maguire
//  Creation:    April 9, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtMiliFileFormat::GetCycles(intVector &outCycles)
{
    outCycles = cycles;
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::GetTimes
//
//  Purpose:
//      Returns the actual times for each time step.
//
//  Arguments:
//   outTimes   the output vector of times 
//
//  Programmer:  Alister Maguire
//  Creation:    April 9, 2019
//
// ****************************************************************************

void
avtMiliFileFormat::GetTimes(doubleVector &outTimes)
{
    outTimes = times;
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::GetNTimesteps
//
//  Purpose:
//      Returns the number of timesteps
//
//  Programmer:  Alister Maguire
//  Creation:    April 9, 2019
//
// ****************************************************************************

int
avtMiliFileFormat::GetNTimesteps()
{
    return nTimesteps;
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::CanCacheVariable
//
//  Purpose:
//      Determine if a variable can be cached. 
//
//  Programmer:  Alister Maguire
//  Creation:    January 29, 2019
//
// ****************************************************************************

bool
avtMiliFileFormat::CanCacheVariable(const char *varname)
{
    return false;
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//    Returns meta-data about the database.
//
//  Arguments:
//    md         The meta-data structure to populate
//    timeState  The time index to use (if metadata varies with time)
//
//  Programmer:  Alister Maguire
//  Creation:    Jan 16, 2019
//
//  Modifications
//
// ****************************************************************************

void
avtMiliFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md,
                                            int timeState)
{
    int pdmdTimer = visitTimer->StartTimer();

    for (int meshId = 0; meshId < nMeshes; ++meshId)
    {
        char meshName[32];
        char sandMeshName[64];
        char matName[32];

        sprintf(meshName, "mesh%d", meshId + 1);
        sprintf(sandMeshName, "sand_mesh%d", meshId + 1);
        sprintf(matName, "materials%d", meshId + 1);

        avtMeshMetaData *mesh      = new avtMeshMetaData;
        mesh->name                 = meshName;
        mesh->meshType             = AVT_UNSTRUCTURED_MESH;
        mesh->numBlocks            = nDomains;
        mesh->blockOrigin          = 0;
        //
        // mili indexing is 1 based.
        //
        mesh->cellOrigin           = 1; 
        mesh->nodeOrigin           = 1; 
        mesh->spatialDimension     = dims;
        mesh->topologicalDimension = dims;
        mesh->blockTitle           = "processors";
        mesh->blockPieceName       = "processor";
        mesh->hasSpatialExtents    = false;
        md->Add(mesh);

        //
        // Adding material to the meta data. 
        //
        int numMats = miliMetaData[meshId]->GetNumMaterials();
        stringVector matColors;
        miliMetaData[meshId]->GetMaterialColors(matColors);

        stringVector matNames;
        miliMetaData[meshId]->GetMaterialNames(matNames);
        AddMaterialToMetaData(md, 
                              matName, 
                              meshName, 
                              numMats, 
                              matNames, 
                              matColors);

        AddLabelVarToMetaData(md, 
                              "OriginalZoneLabels", 
                              meshName, 
                              AVT_ZONECENT, 
                              dims);

        bool hideFromGui = false;
        AddLabelVarToMetaData(md, 
                              "OriginalNodeLabels", 
                              meshName, 
                              AVT_NODECENT, 
                              dims, 
                              hideFromGui);

        //
        // If the dataset contains sanded elements, the default mesh
        // will have them ghosted, but we need to give the option
        // of a mesh that does contain them. 
        //
        bool doSand = miliMetaData[meshId]->ContainsSand();
        if  (doSand)
        {
            avtMeshMetaData *sandMesh      = new avtMeshMetaData;
            sandMesh->name                 = sandMeshName;
            sandMesh->meshType             = AVT_UNSTRUCTURED_MESH;
            sandMesh->numBlocks            = nDomains;
            sandMesh->blockOrigin          = 0;
            //
            // mili indexing is 1 based.
            //
            sandMesh->cellOrigin           = 1;
            sandMesh->nodeOrigin           = 1; 
            sandMesh->spatialDimension     = dims;
            sandMesh->topologicalDimension = dims;
            sandMesh->blockTitle           = "processors";
            sandMesh->blockPieceName       = "processor";
            sandMesh->hasSpatialExtents    = false;

            md->Add(sandMesh);

            AddMaterialToMetaData(md, 
                                  matName, 
                                  sandMeshName, 
                                  numMats, 
                                  matNames, 
                                  matColors);

            AddLabelVarToMetaData(md, 
                                  "OriginalZoneLabels", 
                                  sandMeshName, 
                                  AVT_ZONECENT, 
                                  dims);

            bool hideFromGui = false;
            AddLabelVarToMetaData(md, 
                                  "OriginalNodeLabels", 
                                  sandMeshName, 
                                  AVT_NODECENT, 
                                  dims, 
                                  hideFromGui);
        }

        //
        // Adding variables
        //
        int numVars = miliMetaData[meshId]->GetNumVariables();

        for (int i = 0 ; i < numVars; i++)
        {
            MiliVariableMetaData *varMD = miliMetaData[meshId]->
                GetVarMDByIdx(i);

            if (varMD == NULL)
            {
                debug1 << "Missing variable meta data???";
                continue;
            }

            //
            // If this variable is sand or cause, we only add it to 
            // the sand mesh.
            //
            bool addToDefault = !(varMD->IsSand() || varMD->IsCause());
            if (!addToDefault && !doSand)
            {
                continue;
            }

            //
            // Create a container for all mesh paths that the current
            // variable must be added to. 
            //
            stringVector meshPaths;
            if (addToDefault)
            {  
                meshPaths.push_back(varMD->GetPath());
            }
            if (doSand)
            {
                std::string sandDir  = miliMetaData[meshId]->GetSandDir();
                std::string sandPath = sandDir + "/" + varMD->GetPath();
                meshPaths.push_back(sandPath);
            }

            char meshName[32];
            sprintf(meshName, "mesh%d", varMD->GetMeshAssociation() + 1);

            int cellType           = varMD->GetAvtCellType();
            avtCentering centering = varMD->GetCentering();
            int vecSize            = varMD->GetVectorSize();

            //
            // If the variable is a scalar, the vector size is designated 1, 
            // but there will be no vector components. 
            //
            stringVector vComps  = varMD->GetVectorComponents();
            int nComps           = vComps.size();

            //
            // If we have a higher dim vector, determine how we should
            // treat it.
            //
            int cellTypeCast       = cellType;
            bool mayHaveHigherDims = (cellType == AVT_VECTOR_VAR ||
                                      cellType == AVT_MATERIAL);
            if (mayHaveHigherDims && vecSize != dims)
            {
                if (dims == 3)
                {
                    if (vecSize == 6)
                    {
                        cellTypeCast = AVT_SYMMETRIC_TENSOR_VAR;
                    }
                    else if (vecSize == 9)
                    {
                        cellTypeCast = AVT_TENSOR_VAR;
                    }
                    else
                    {
                        //
                        // Default to treating it as an array and 
                        // just display the components. 
                        //
                        cellTypeCast = AVT_ARRAY_VAR;
                    }

                }
                else if (dims == 2)      
                {
                    if (vecSize == 3)
                    {
                        cellTypeCast = AVT_SYMMETRIC_TENSOR_VAR;
                    }
                    else if (vecSize == 4)
                    {
                        cellTypeCast = AVT_TENSOR_VAR;
                    }
                    else
                    {
                        //
                        // Default to treating it as an array and 
                        // just display the components. 
                        //
                        cellTypeCast = AVT_ARRAY_VAR;
                    }
                }
                else
                {
                    continue;
                }
            }
           
            switch (cellTypeCast)
            {
                case AVT_SCALAR_VAR:
                {
                    for (stringVector::iterator pathIt = meshPaths.begin();
                         pathIt != meshPaths.end(); ++pathIt)
                    {
                        AddScalarVarToMetaData(md, (*pathIt), 
                            meshName, centering);
                    }
                    break;
                }
                case AVT_VECTOR_VAR:
                {
                    for (stringVector::iterator pathIt = meshPaths.begin();
                         pathIt != meshPaths.end(); ++pathIt)
                    {
                        AddVectorVarToMetaData(md, (*pathIt), meshName, 
                            centering, nComps);

                        //
                        // Add expressions for displaying each component. 
                        //
                        for (int j = 0; j < nComps; ++j)
                        {
                            //
                            // Add the x, y, z expressions. 
                            //
                            char name[1024];
                            sprintf(name, "%s/%s", (*pathIt).c_str(), 
                                vComps[j].c_str());
                            Expression expr = ScalarExpressionFromVec(
                                                  (*pathIt).c_str(),
                                                  name, 
                                                  j);
                            md->AddExpression(&expr);
                        }
                    }
                    break;
                }
                case AVT_SYMMETRIC_TENSOR_VAR:
                {
                    for (stringVector::iterator pathIt = meshPaths.begin();
                         pathIt != meshPaths.end(); ++pathIt)
                    {
                        //
                        // When we come across a vector of length 6, we change it 
		        // to a normal vector of length 9 and render it as a 
		        // symmetric tensor. 
                        //
                        AddSymmetricTensorVarToMetaData(md, (*pathIt), 
                            meshName, centering, 9);

                        //
                        // Now we add the individual components. 
                        //
                        int multDimIdxs[] = {1, 2, 0};

                        for (int j = 0; j < 3; ++j)
                        {
                            // 
                            // First, get the "single-dim" values: xx, yy, zz. 
                            // 
                            Expression singleDim;
                            singleDim.SetType(Expression::ScalarMeshVar);
                  
                            char singleDef[256];
                            sprintf(singleDef, "<%s>[%d][%d]", 
                                (*pathIt).c_str(), j, j);
                            singleDim.SetDefinition(singleDef);

                            std::string singleName = (*pathIt) + "/" + 
                                varMD->GetVectorComponent(j);
                            singleDim.SetName(singleName);

                            md->AddExpression(&singleDim);

                            //
                            // Next, get the "multi-dim" values: xy, yz, zx
                            //
                            int compIdx = j + 3;
                            Expression multDim;
                            multDim.SetType(Expression::ScalarMeshVar);

                            char multDef[256];
                            sprintf(multDef, "<%s>[%d][%d]", (*pathIt).c_str(), 
                                j, multDimIdxs[j]);
                            multDim.SetDefinition(multDef);

                            std::string multName = (*pathIt) + "/" + 
                                varMD->GetVectorComponent(compIdx);
                            multDim.SetName(multName);

                            md->AddExpression(&multDim);
                        }
                    }
                    break;
                }
                case AVT_TENSOR_VAR:
                {
                    for (stringVector::iterator pathIt = meshPaths.begin();
                         pathIt != meshPaths.end(); ++pathIt)
                    {
                        AddTensorVarToMetaData(md, (*pathIt), 
                            meshName, centering, nComps);

                        //
                        // Now we add the individual components. 
                        //
                        int multDimIdxs[] = {1, 2, 0};

                        for (int j = 0; j < 3; ++j)
                        {
                            // 
                            // First, get the "single-dim" values: xx, yy, zz. 
                            // 
                            Expression singleDim;
                            singleDim.SetType(Expression::ScalarMeshVar);

                            char singleDef[256];
                            sprintf(singleDef, "<%s>[%d][%d]", 
                                (*pathIt).c_str(), j, j);
                            singleDim.SetDefinition(singleDef);

                            std::string singleName = (*pathIt) + "/" + 
                                varMD->GetVectorComponent(j);
                            singleDim.SetName(singleName);

                            md->AddExpression(&singleDim);

                            //
                            // Next, get the "multi-dim" values: xy, yz, zx
                            //
                            int compIdx = j + 3;
                            Expression multDim;
                            char multDef[256];
                            sprintf(multDef, "<%s>[%d][%d]", (*pathIt).c_str(), 
                                j, multDimIdxs[j]);
                            std::string multName = (*pathIt) + "/" + 
                                varMD->GetVectorComponent(compIdx);
                            multDim.SetName(multName);
                            multDim.SetDefinition(multDef);
                            multDim.SetType(Expression::ScalarMeshVar);
                            md->AddExpression(&multDim);
                        }
                    }
                    break;
                }
                case AVT_ARRAY_VAR:
                {
                    for (stringVector::iterator pathIt = meshPaths.begin();
                         pathIt != meshPaths.end(); ++pathIt)
                    {
                        //
                        // For array vars, we just want to display the 
                        // individual components. 
                        //
                        stringVector compNames;

                        for (int j = 0; j < nComps; ++j)
                        {
                            compNames.push_back(varMD->GetVectorComponent(j));
                            char name[1024];
                            sprintf(name, "%s/%s", (*pathIt).c_str(), 
                                varMD->GetVectorComponent(j).c_str());
                            Expression expr = ScalarExpressionFromVec(
                                                  (*pathIt).c_str(),
                                                  name, 
                                                  j);
                            md->AddExpression(&expr);
                        }

                        AddArrayVarToMetaData(md, (*pathIt), compNames, meshName, 
                            centering); 
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        //TODO: add derived types when given the OK. 
    }

    //
    // Set the cycle and time information.
    //
    md->SetCyclesAreAccurate(true);
    md->SetCycles(cycles);
    md->SetTimesAreAccurate(true);
    md->SetTimes(times);

    visitTimer->StopTimer(pdmdTimer, "MILI: PopulateDatabaseMetaData timer");
    visitTimer->DumpTimings();
}


// ****************************************************************************
//  Method: avtMiliFileFormat::GetAuxiliaryData
//
//  Purpose:
//      Gets the auxiliary data specified.
//
//      Note: much of this has remained from the original plugin. 
//
//  Arguments:
//      varName    The variable of interest.
//      ts         The timestep of interest.
//      dom        The domain of interest.
//      auxType    The type of auxiliary data.
//      typeARgs   The arguments for that type -- not used.
//      df         Destructor function.
//
//  Returns:    The auxiliary data.
//
//  Programmer: Alister Maguire
//  Creation:   April 9, 2019
//
//  Modifications
//
// ****************************************************************************
 
void *
avtMiliFileFormat::GetAuxiliaryData(const char *varName, 
                                    int         ts, 
                                    int         dom, 
                                    const char *auxType, 
                                    void       *typeArgs,
                                    DestructorFunction &df) 
{
    //
    // If we know that the request can't be handled, just 
    // leave. 
    //
    if ( (strcmp(auxType, AUXILIARY_DATA_MATERIAL) != 0) && 
         (strcmp(auxType, "AUXILIARY_DATA_IDENTIFIERS") != 0) )
    {
        return NULL;
    }

    if (!meshRead[dom])
    {
        ReadMesh(dom);
    }
    
    if (strcmp(auxType, "AUXILIARY_DATA_IDENTIFIERS") == 0)
    {
        //
        // Retrieve the node/zone labels. 
        //
        std::string strVarName(varName);
        if(strVarName != "OriginalZoneLabels" && 
           strVarName != "OriginalNodeLabels" )
        {
            EXCEPTION1(InvalidVariableException, varName);
        }

        df = vtkElementLabelArray::Destruct;
        return (void *)this->GetVar(ts, dom, varName);
    }
    else if (strcmp(auxType, AUXILIARY_DATA_MATERIAL) == 0)
    {
        //
        // Retrieve the materials. 
        //

        if (strstr(varName, "materials") != varName)
        {
            EXCEPTION1(InvalidVariableException, varName);
        }

        //
        // Do a checked conversion to integer.
        //
        char *check;
        int meshId = (int) strtol(varName + strlen("materials"), &check, 10);
        if (check != NULL && check[0] != '\0')
        {
            EXCEPTION1(InvalidVariableException, varName)
        }
        --meshId;
 
        avtMaterial *myCopy = materials[dom][meshId];
        avtMaterial *mat    = new avtMaterial(myCopy->GetNMaterials(),
                                              myCopy->GetMaterials(),
                                              myCopy->GetNZones(),
                                              myCopy->GetMatlist(),
                                              myCopy->GetMixlen(),
                                              myCopy->GetMixMat(),
                                              myCopy->GetMixNext(),
                                              myCopy->GetMixZone(),
                                              myCopy->GetMixVF());
        df = avtMaterial::Destruct;

        return (void*) mat;
    }

    return NULL;
}


// ***************************************************************************
//  Function: avtMiliFileFormat::ExtractJsonVariable
//
//  Purpose:
//      Extract a mili variable from our .mili json file. 
//
//  Arguments: 
//      val          A json Value type to extract from.
//      shortName    The variable's short name.        
//      cShortName   The variable's class' short name. 
//      cLongName    The Variable's class' long name. 
//      meshId       The associated mesh ID. 
//      isMatVar     Is this a material variable?
//      isGlobal     Is this a global variable?
//           
//  Author: Alister Maguire
//  Date:   January 29, 2019
//
//  Modifications:
//
// ****************************************************************************

MiliVariableMetaData * 
avtMiliFileFormat::ExtractJsonVariable(const Value &val,
                                       std::string shortName,
                                       std::string cShortName,
                                       std::string cLongName,
                                       int meshId,
                                       bool isMatVar,
                                       bool isGlobal)
{
    if (val.IsObject())
    {
        std::string longName   = "";
        avtCentering centering = AVT_NODECENT;
        int avtType            = -1;
        int aggType            = -1;
        int vecSize            = 0;
        int compDims           = 1;
        int numType            = M_FLOAT;
        stringVector comps;

        if (val.HasMember("LongName"))
        {
            longName = val["LongName"].GetString();
        }

        if (val.HasMember("Center"))
        {
            centering = avtCentering(val["Center"].GetInt());
        }

        if (val.HasMember("VTK_TYPE"))
        {
            avtType = val["VTK_TYPE"].GetInt();
        }

        if (val.HasMember("agg_type"))
        {
            aggType = val["agg_type"].GetInt();
        }

        if (val.HasMember("vector_size"))
        {
            vecSize = val["vector_size"].GetInt(); 
        }

        if (val.HasMember("dims"))
        {
            compDims = val["dims"].GetInt();
            if (compDims == 0)
            {
                compDims = 1;
            }
        }
  
        if (val.HasMember("num_type"))
        {
            numType = val["num_type"].GetInt();
        }

        if (val.HasMember("vector_components"))
        {
            const Value &vComps = val["vector_components"];

            if (vComps.IsArray())
            {
                for (SizeType i = 0; i < vComps.Size(); ++i)
                {
                    comps.push_back(vComps[i].GetString());
                }
            }
        }

        bool isMulti = (nMeshes > 1);

        return new MiliVariableMetaData(shortName, 
                                        longName,
                                        cShortName,
                                        cLongName,
                                        isMulti,
                                        isMatVar,
                                        isGlobal,
                                        centering,
                                        nDomains,
                                        meshId,
                                        avtType,
                                        aggType,
                                        numType,
                                        vecSize,
                                        compDims,
                                        comps);
    }

    return NULL;
}


// ***************************************************************************
//  Function: avtMiliFileFormat::CountJsonClassVariables
//
//  Purpose:
//      Count the total number of variables that belong to all 
//      mili Classes within the .mili json file. 
//
//  Arguments: 
//      jDoc    The json document. 
//           
//  Author: Alister Maguire
//  Date:   January 29, 2019
//
//  Modifications:
//
// ****************************************************************************

int
avtMiliFileFormat::CountJsonClassVariables(const Document &jDoc)
{
    const Value &jClasses = jDoc["Classes"]; 
    int numClassVars = 0;
    for (Value::ConstMemberIterator cItr = jClasses.MemberBegin();
         cItr != jClasses.MemberEnd(); ++cItr)
    { 
        const Value &val = cItr->value;
        if (val.IsObject())
        { 
            if (val.HasMember("variables"))
            {
                const Value &cVars = val["variables"];
                if (cVars.IsArray())
                {
                    numClassVars += cVars.Size(); 
                }
            }
        }
    }
    return numClassVars;
}


// ***************************************************************************
//  Function: avtMiliFileFormat::ExtractJsonClasses
//
//  Purpose:
//      Extract mili Class information from our .mili json file. 
//
//  Arguments: 
//      jDoc    The json document. 
//      meshId  The current mesh ID. 
//           
//  Author: Alister Maguire
//  Date:   January 29, 2019
//
//  Modifications:
//
// ****************************************************************************

void 
avtMiliFileFormat::ExtractJsonClasses(Document &jDoc,
                                      int meshId) 
{
    const Value &jClasses = jDoc["Classes"]; 

    //
    // Keep track of how many variables and classes 
    // we've added. 
    //
    int varIdx   = 0;
    int classIdx = 0;

    int cCount = 0;
    if (jClasses.HasMember("count"))
    {
        cCount = jClasses["count"].GetInt();
    }

    //
    // Set the number of classes and initialize our
    // class container. 
    //
    miliMetaData[meshId]->SetNumClasses(cCount);

    for (Value::ConstMemberIterator cItr = jClasses.MemberBegin();
         cItr != jClasses.MemberEnd(); ++cItr)
    { 
        const Value &val  = cItr->value;
        std::string sName = cItr->name.GetString();

        if (val.IsObject())
        { 
            //
            // Check for special cases. As of now, these are material
            // and global variables. 
            //
            std::string matFlag  = "mat";
            std::string globFlag = "glob";
            bool isGlobal        = false;
            bool isMatVar        = false;

            if (sName == matFlag)
            {
                isMatVar = true;
            }
            else if (sName == globFlag)
            {
                isGlobal = true;
            }
 
            std::string lName = "";
            int scID          = -1;
            int elCount       = 0;

            if (val.HasMember("LongName"))
            {
                lName = val["LongName"].GetString();
            }
               
            if (val.HasMember("SuperClass"))
            {
                scID= val["SuperClass"].GetInt();
            }
            else
            {
                EXCEPTION2(UnexpectedValueException, "Superclass ID", "");
            }

            MiliClassMetaData *miliClass = 
                new MiliClassMetaData(sName,
                                      lName,
                                      scID,
                                      nDomains);
       
            if (val.HasMember("variables"))
            {
                const Value &cVars = val["variables"];
                
                //
                // Extract the class variable meta data. 
                //
                for (SizeType i = 0; i < cVars.Size(); ++i)
                {
                    const Value &jVars = jDoc["Variables"];

                    if (jVars.HasMember(cVars[i].GetString()))
                    {
                        const Value &var    = jVars[cVars[i]];
                        std::string varName = cVars[i].GetString();
 
                        MiliVariableMetaData *varMD = ExtractJsonVariable(var,
                            varName, sName, lName, meshId, isMatVar, isGlobal);

                        if (varMD != NULL)
                        {
                            miliClass->AddMiliVariable(varName);
                            miliMetaData[meshId]->AddVarMD(varIdx, varMD);
                            varIdx++;
                        }
                    }
                }
               
                //
                // Cache the mili class md. 
                //
                miliMetaData[meshId]->AddClassMD(classIdx, miliClass);
                classIdx++;
            }
        }
    } 
}


// ***************************************************************************
//  Function: avtMiliFileFormat::LoadMiliInfoJson
//
//  Purpose:
//      Extract mili meta data from a json file. 
//
//  Arguments: 
//      fpath    The name and path of the file to open. 
//           
//  Author: Alister Maguire
//  Date:   April 9, 2019
//
//  Modifications:
//
// ****************************************************************************
void
avtMiliFileFormat::LoadMiliInfoJson(const char *fpath)
{
    int jsonTimer1 = visitTimer->StartTimer(); 

    std::ifstream jfile;

    jfile.open(fpath);

    IStreamWrapper isw(jfile);
    Document jDoc;
    jDoc.ParseStream(isw);

    if (jDoc.HasMember("Domains"))
    {
        nDomains = jDoc["Domains"].GetInt(); 
    }
    else
    {
        debug1 << "Mili file missing domains?!?!?!\n";
        char msg[1024];
        sprintf(msg, "Mili file is missing domains!!");
        EXCEPTION1(ImproperUseException, msg);
    }

    if (jDoc.HasMember("Number_of_Meshes"))
    {
        nMeshes = jDoc["Number_of_Meshes"].GetInt();
    }
    else
    {
        //
        // Older formats don't seem to include this info. 
        //
        nMeshes = 1;
    }

    if (jDoc.HasMember("Dimensions"))
    {
        dims = jDoc["Dimensions"].GetInt();
    }
    else
    {
        debug1 << "Mili file missing dims?!?!?!\n";
        char msg[1024];
        sprintf(msg, "Mili file is missing dims!!");
        EXCEPTION1(ImproperUseException, msg);
    }
    
    miliMetaData = new avtMiliMetaData *[nMeshes];

    for (int i = 0; i < nMeshes; ++i) 
    {
        miliMetaData[i] = NULL;
    }

    //
    //TODO: It's yet unlcear how meshes will be distinguished in 
    //      the mili files. According to Kevin, this might not
    //      even be possible for mili to contain multiple meshes
    //      right now. We're putting the logistics of this on 
    //      the backburner. 
    //
    for (int meshId = 0; meshId < nMeshes; ++meshId)
    {
        miliMetaData[meshId] = new avtMiliMetaData(nDomains);

        if (jDoc.HasMember("Materials"))
        {
            //
            // We only need space for 3 floating points (RGB). 
            //
            const Value &jMats = jDoc["Materials"]; 
            int   nMats = 0;

            if (jMats.HasMember("count"))
            {
                nMats = jMats["count"].GetInt();
            }

            miliMetaData[meshId]->SetNumMaterials(nMats);

            //
            // Retrieve material meta-data. 
            //
            int matCount = 0;
            for (Value::ConstMemberIterator jItr = jMats.MemberBegin();
                 jItr != jMats.MemberEnd(); ++jItr)
            { 
                std::string name = jItr->name.GetString();
                const Value &mat = jItr->value;

                std::string matName = "";
                std::stringstream colorSS;
                colorSS << "#";
                  
                if (mat.IsObject())
                {
                    if (mat.HasMember("name"))
                    {
                        matName = mat["name"].GetString();
                    }
                    else
                    {
                        char buff[128];
                        sprintf(buff, "%d", matCount);
                        matName = std::string(buff);
                    }

                    if (mat.HasMember("COLOR"))
                    {
                        const Value &mColors = mat["COLOR"];

                        //
                        // Mili stores its mat color as a float rgb. We
                        // need to convert this to a hex string. 
                        //
                        if (mColors.IsArray())
                        {
                            for (SizeType i = 0; i < mColors.Size(); ++i)
                            {
                                int iVal = (int) floor(
                                    mColors[i].GetFloat() * 256);

                                colorSS << std::hex << 
                                    std::max(0, std::min(255, iVal));
                            }
                            
                        }
                    }
                    else
                    {
                        //
                        // If this material doesn't have a color, 
                        // assign a random color. 
                        //
                        for (int i = 0; i < 3; ++i)
                        {
                            int randInt = (rand() % static_cast<int> (256));
                            colorSS << std::hex << randInt;
                        }
                    }

                    MiliMaterialMetaData *miliMaterial = 
                        new MiliMaterialMetaData(matName,
                                                 std::string(colorSS.str()));

                    miliMetaData[meshId]->AddMaterialMD(matCount++, miliMaterial);
                }
            }
        }

        if (jDoc.HasMember("Variables"))
        {
            miliMetaData[meshId]->SetNumVariables(CountJsonClassVariables(jDoc));
            ExtractJsonClasses(jDoc, meshId);
        }

        if (jDoc.HasMember("States"))
        {
            const Value &jStates = jDoc["States"];
            if (jStates.HasMember("count"))
            {
                nTimesteps = jStates["count"].GetInt();
            }

            if (jStates.HasMember("times"))
            {

                const Value &jTimes = jStates["times"];
                times.clear();
                cycles.clear();

                if (jTimes.IsArray())
                {
                    for (SizeType i = 0; i < jTimes.Size(); ++i)
                    {
                        times.push_back(jTimes[i].GetDouble());
                        cycles.push_back(i);
                    }
                }
            }
        }
    }        

    dbid.resize(nDomains, -1);
    meshRead.resize(nDomains, false);

    datasets.resize(nDomains);
    materials.resize(nDomains);

    for (int dom = 0; dom < nDomains; ++dom)
    {
        datasets[dom].resize(nMeshes, NULL);
        materials[dom].resize(nMeshes, NULL);
    }

    jfile.close();
    
    visitTimer->StopTimer(jsonTimer1, "MILI: Loading Mili Json Data");
    visitTimer->DumpTimings();
}



// ***************************************************************************
//  Function: RetrieveCellLabelInfo
//
//  Purpose:
//      Retrieve info about cell labels from mili, and populate
//      data structures to implement a reverse mapping between
//      a zone label and a zone id.
//
//  Arguments: 
//      meshId          The mesh ID. 
//      shortName       The class shortname.  
//      dom             The domain ID. 
//      nElemsInClass   The number of elements in this mili class. 
//
//  Author: Alister Maguire
//  Date:   April 9, 2019
//
//  Modifications:
//
// ****************************************************************************

void 
avtMiliFileFormat::RetrieveCellLabelInfo(const int meshId, 
                                         char *shortName, 
                                         const int dom, 
                                         const int nElemsInClass)
{
    int numBlocks    = 0; 
    int *blockRanges = NULL;
    int elemList[nElemsInClass];
    int labelIds[nElemsInClass];

    for (int i = 0; i < nElemsInClass; ++i)
    {
        elemList[i] = -1;
        labelIds[i] = -1;
    }
    
    int loadZoneLabels = visitTimer->StartTimer();

    //
    // Check for labels
    //
    //TODO: see if memory errors from this call were fixed. 
    int nExpectedLabels = nElemsInClass;
    int rval = mc_load_conn_labels(dbid[dom], meshId, shortName, 
                                   nExpectedLabels, &numBlocks, 
                                   &blockRanges, elemList, labelIds);

    if (rval != OK)
    {
        debug1 << "mc_load_conn_labels failed at " << shortName << "!\n";
        numBlocks   = 0; 
        blockRanges = NULL;
    }

    MiliClassMetaData *miliClass = 
        miliMetaData[meshId]->GetClassMDByShortName(shortName);
    miliClass->PopulateLabelIds(dom, 
                                labelIds,
                                numBlocks,
                                blockRanges);
}


// ***************************************************************************
//  Function: RetrieveNodeLabelInfo
//
//  Purpose:
//      Retrieve node label info and populate data structures 
//      to implement a reverse mapping between a node label 
//      and a node id
//
//  Arguments: 
//      meshId        The mesh ID. 
//      shortName     The class shortname. 
//      dom           The domain of interest. 
//           
//  Notes:
//      Much of this method is based on a method from the original
//      plugin that was authored by Matt Larsen in 2017.  
//
//  Author: Alister Maguire
//  Date:   April 9, 2019
//
//  Modifications:
//
// ****************************************************************************

void 
avtMiliFileFormat::RetrieveNodeLabelInfo(const int meshId, char *shortName, 
                                         const int dom)
{

    int nLabeledNodes = 0;
    int nNodes        = miliMetaData[meshId]->GetNumNodes(dom);
    int numBlocks     = 0; 
    int *blockRanges  = NULL;
    int elemList[nNodes];
    int labelIds[nNodes];

    for (int i = 0; i < nNodes; ++i)
    {
        labelIds[i] = -1;
        elemList[i] = -1;
    }

    int rval = mc_load_node_labels(dbid[dom], meshId, shortName, 
                                   &numBlocks, &blockRanges, labelIds);

    if (rval != OK)
    {
        debug1 << "mc_load_node_labels failed!\n";
        numBlocks   = 0; 
        blockRanges = NULL;
    }

    MiliClassMetaData *miliClass = 
        miliMetaData[meshId]->GetClassMDByShortName(shortName);
    miliClass->PopulateLabelIds(dom, 
                                labelIds,
                                numBlocks,
                                blockRanges);
}


// ***************************************************************************
//  Function: avtMiliFileFormat::CreateGenericExpression
//
//  Purpose:
//      Helper function for creating generic expressions. 
//
//  Arguments: 
//      name        The expression's name. 
//      definition  The expression's definition. 
//      eType       The type of expression.
//           
//  Author: Alister Maguire
//  Date:   April 9, 2019
//
//  Modifications:
//
// ****************************************************************************
Expression
avtMiliFileFormat::CreateGenericExpression(const char *name,
                                           const char *definition,
                                           Expression::ExprType eType)
{
    Expression exp;
    exp.SetName(name);
    exp.SetDefinition(definition);
    exp.SetType(eType);
    return exp;
}


// ***************************************************************************
//  Function: avtMiliFileFormat::ScalarExpressionFromVec
//
//  Purpose:
//      Create a scalar expression from a vector. 
//
//  Arguments: 
//      vecPath      The vector's path in the visit menu.
//      scalarPath   The new path to use for the scalar.   
//           
//  Author: Alister Maguire
//  Date:   April 9, 2019
//
//  Modifications:
//
// ****************************************************************************
Expression
avtMiliFileFormat::ScalarExpressionFromVec(const char *vecPath, 
                                           const char *scalarPath, 
                                           int dim)
{
    char def[256];
    sprintf(def, "<%s>[%d]", vecPath, dim);
    Expression::ExprType eType = Expression::ScalarMeshVar;
    return CreateGenericExpression(scalarPath, def, eType);
}
