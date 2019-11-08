// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
#include <avtCommonDataFunctions.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>
#include <UnexpectedValueException.h>
#include <NonCompliantFileException.h>

#include <TimingsManager.h>


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
            //
            // No copy needed. 
            //
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
    datasets   = NULL;
    materials  = NULL;

    LoadMiliInfoJson(fpath);

    string fnamePth(fpath);

    size_t fNPos  = fnamePth.find_last_of("\\/");
    string pthTmp = fnamePth.substr(0, fNPos + 1);
    string root   = fnamePth.substr(fNPos + 1);

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
    string lastSub = "";
    size_t lastPos = root.find_last_of(".");
    lastSub        = root.substr(lastPos + 1);
    string mExt    = "mili";

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
    // Clean up our dataset memory.
    //
    if (datasets != NULL)
    {
        for (int i = 0; i < nDomains; ++i)
        {
            for (int j = 0; j < nMeshes; ++j) 
            {
                if (datasets[i][j] != NULL)
                {
                    datasets[i][j]->Delete();
                    datasets[i][j] = NULL;
                }
            }
            delete [] datasets[i];
        }

        delete [] datasets;
    }

    //
    // Clean up our material memory.
    //
    if (materials != NULL)
    {
        for (int i = 0; i < nDomains; ++i)
        {
            for (int j = 0; j < nMeshes; ++j) 
            {
                if (materials[i][j] != NULL)
                {
                    delete materials[i][j];
                    materials[i][j] = NULL;
                }
            }
            delete [] materials[i];
        }

        delete [] materials;
    }


    //
    // Delete the mili meta data. 
    //
    if (miliMetaData != NULL)
    {
        for (int i = 0; i < nMeshes; ++i)
        {
            if (miliMetaData[i] != NULL)
            {
                delete miliMetaData[i];
            }
        }
        delete [] miliMetaData;
    }

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
//  method:  avtmilifileformat::getcycles
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

    //
    // Multi-proc databases have larger numbers attached to their names. 
    //
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
            debug3 << "MILI: Attempting mc_open on root=\"" << famroot << 
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
                snprintf(famname, 128, root_fmtstrs[i], famroot, dom);
                debug3 << "MILI: Attempting mc_open on root=\"" << famname 
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
//  Method:  avtMiliFileFormat::GetNodePositions
//
//  Purpose:
//      Retrieve the node positions for the given timestep.
//
//  Arguments:
//    timestep   The timestep of interest.
//    dom        The domain of interested.
//    meshId     The mesh id of interest. 
//
//  Programmer:  Alister Maguire
//  Creation:    
//
//  Modifications:
//
// ****************************************************************************

vtkPoints *
avtMiliFileFormat::GetNodePositions(int timestep, 
                                    int dom, 
                                    int meshId)
{
    //
    // The node positions are stored in 'nodpos'.
    //
    char npChar[128];
    snprintf(npChar, 128, "nodpos");
    char *npCharPtr = (char *)npChar;

    MiliVariableMetaData *nodpos = miliMetaData[meshId]->
        GetVarMDByShortName("nodpos", "node");

    vtkPoints *vtkNodePos = NULL;

    //
    // There are datasets whose nodes remain constant throughout time. 
    // In these cases, we will not have a "nodpos" to retrieve and
    // will instead rely on the initial positions retrieved during
    // ReadMesh. 
    //
    if (nodpos != NULL)
    {
        int numNodes = miliMetaData[meshId]->GetNumNodes(dom);
        int nPts     = dims * numNodes;
        int vType    = nodpos->GetNumType();
        int subrec   = nodpos->GetSubrecIds(dom)[0];
        float *fPts = new float[nPts];

        ReadMiliResults(dbid[dom], timestep+1, subrec, 1, 
            &npCharPtr, vType, nPts, fPts);

        vtkNodePos = vtkPoints::New();
        vtkNodePos->SetNumberOfPoints(numNodes);

        float *vtkNPPtr = (float *) vtkNodePos->GetVoidPointer(0);
        float *fPtsPtr  = fPts; 

        for (int pt = 0 ; pt < numNodes; pt++)
        {
            *(vtkNPPtr++) = *(fPtsPtr++);
            *(vtkNPPtr++) = *(fPtsPtr++);
            if (dims >= 3)
            {
                *(vtkNPPtr++) = *(fPtsPtr++);
            }
            else
            {
                *(vtkNPPtr++) = 0.;
            }
        }

        delete [] fPts;
    }

    return vtkNodePos;
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::GetMesh
//
//  Purpose:
//      Retrieve the mesh for the given timestep.
//
//  Arguments:
//    timestep   The timestep of interest.
//    dom        The domain of interested.
//    mesh       the name of the mesh to read
//
//  Programmer:  Alister Maguire
//  Creation:    Jan 16, 2019
//
//  Modifications:
//
//    Alister Maguire, Fri Mar  6 10:55:34 PST 2020
//    Adding ghost zones to aid the pick operator.
//
// ****************************************************************************

vtkDataSet *
avtMiliFileFormat::GetMesh(int timestep, int dom, const char *mesh)
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

    if (isSandMesh && !miliMetaData[meshId]->ContainsSand())
    {
        debug1 << "MILI: The user has requested a sand mesh that doesn't exist!"
               << "This shouldn't be possible...";
        char msg[128];
        snprintf(msg, 128, "Cannot view a non-existent sand mesh!");
        EXCEPTION1(ImproperUseException, msg);
    }

    vtkPoints *nodePosPts = GetNodePositions(timestep, dom, meshId);

    vtkUnstructuredGrid *rv = vtkUnstructuredGrid::New();
    rv->ShallowCopy(datasets[dom][meshId]);

    if (nodePosPts != NULL)
    {
        rv->SetPoints(nodePosPts);
        nodePosPts->Delete();
    }
    else if (datasets[dom][meshId] != NULL)
    {
        if (datasets[dom][meshId]->GetPoints() == NULL)
        {
            debug1 << "MILI: Unable to find nodes! This shouldn't happen..";
            char msg[128];
            snprintf(msg, 128, "Unable to load nodes from Mili!");
            EXCEPTION1(ImproperUseException, msg);
        }
    }

    //
    // If our dataset contains sand, and the user has not requested
    // the sand mesh, we need to ghost out the sanded elements. 
    // FYI: sand elements are those that have been "destroyed" 
    // during the simulation. 
    //
    if (!isSandMesh && miliMetaData[meshId]->ContainsSand())
    {
        SubrecInfo *SRInfo = miliMetaData[meshId]->GetSubrecInfo(dom);

        if (SRInfo == NULL)
        {
            char msg[512];
            snprintf(msg, 512, "Unable to retrieve sand info from mili.");
            EXCEPTION1(ImproperUseException, msg);
        }

        int numVars = miliMetaData[meshId]->GetNumVariables();
        int nCells  = miliMetaData[meshId]->GetNumCells(dom);
        int nNodes  = miliMetaData[meshId]->GetNumNodes(dom);
        
        float *sandBuffer = new float[nCells];

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
                    debug1 << "MILI: Sanded variable is not " << 
                        "zone centered?!?" << endl;
                    continue;
                }

                string varName  = varMD->GetShortName();
                intVector SRIds = varMD->GetSubrecIds(dom);
                int vType       = varMD->GetNumType();

                string className = varMD->GetClassShortName(); 
                int start = miliMetaData[meshId]->
                    GetClassMDByShortName(className.c_str())->
                    GetConnectivityOffset(dom);

                //
                // Create a copy of our name to pass into mili. 
                //
                char charName[128];
                snprintf(charName, 128, "%s", varName.c_str());
                char *namePtr = (char *) charName;

                ReadMiliVarToBuffer(namePtr, SRIds, SRInfo, start,
                    vType, 1, timestep + 1, dom, sandBuffer);
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

        vtkUnsignedCharArray *ghostZones = vtkUnsignedCharArray::New();
        ghostZones->SetName("avtGhostZones");
        ghostZones->SetNumberOfTuples(nCells);

        unsigned char *ghostZonePtr = ghostZones->GetPointer(0);

        for (int i = 0; i < nNodes; ++i)
        {
            ghostNodePtr[i] = 0;
            avtGhostData::AddGhostNodeType(ghostNodePtr[i],
                NODE_NOT_APPLICABLE_TO_PROBLEM);
        }
    
        vtkNew<vtkIdList> cell;
        for (int i = 0; i < nCells; ++i)
        {
            ghostZonePtr[i] = 0;

            //
            // Element status > .5 is good. 
            //
            if (sandBuffer[i] > 0.5)
            {
                rv->GetCellPoints(i, cell);
                const vtkIdType nCellPts = cell->GetNumberOfIds();
                const vtkIdType *cellPts = cell->GetPointer(0);

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
            else
            {
                avtGhostData::AddGhostZoneType(ghostZonePtr[i],
                    ZONE_NOT_APPLICABLE_TO_PROBLEM);
            }
        }
    
        delete [] sandBuffer; 

        rv->GetPointData()->AddArray(ghostNodes);
        rv->GetCellData()->AddArray(ghostZones);
        ghostNodes->Delete();
        ghostZones->Delete();
    }
    

    visitTimer->StopTimer(gmTimer, "MILI: Getting Mesh");
    visitTimer->DumpTimings();

    return rv;
}


// ****************************************************************************
//  Method: avtMiliFileFormat::ExtractMeshIdFromPath
//
//  Purpose:
//      Extract the mesh id from a variable path. 
//
//  Arguments:
//      varPath    The variable path. 
//
//  Returns:
//      The mesh id found from the path. 
//
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019 
//
// ****************************************************************************

int
avtMiliFileFormat::ExtractMeshIdFromPath(const string &varPath)
{
    if (nMeshes > 1)
    {
        size_t varPos = varPath.find("mesh");
        string first  = varPath.substr(varPos, 5);
        char cNum     = first.back();
        return (int) (cNum - '0');
    }
    return 0;
}


// ****************************************************************************
//  Method: avtMiliFileFormat::ReadMesh
//
//  Purpose:
//      Read the given domain of the mesh. 
//
//  Arguments:
//      dom    The domain of interest. 
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
            char msg[512];
            snprintf(msg, 512, "Unable to retrieve %s from mili", shortName);
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
                classNames.push_back(string(shortName));
                nDomCells += nCells;

                miliMetaData[meshId]->GetClassMDByShortName(shortName)->
                    SetConnectivityOffset(dom, offset);
                miliMetaData[meshId]->GetClassMDByShortName(shortName)->
                    SetNumElements(dom, nCells);
                offset += nCells;
            }
        }

        miliMetaData[meshId]->SetNumCells(dom, nDomCells);

        //
        // Global variables are applied to the entire mesh. 
        //
        MiliClassMetaData *globalClass = miliMetaData[meshId]->GetClassMDByShortName("glob");
        if (globalClass != NULL)
        {
            globalClass->SetNumElements(dom, nDomCells);
        }

        //
        // Material variables are applied to all materials (technically, each element should
        // belong to a material, right?). 
        //
        MiliClassMetaData *matClass = miliMetaData[meshId]->GetClassMDByShortName("mat");
        if (matClass != NULL)
        {
            matClass->SetNumElements(dom, nDomCells);
        }
       
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
        int *matList = new int[nDomCells];

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
                int *conn       = new int[nCells * connCount];
                int *mats       = new int[nCells];
                int *part       = new int[nCells];
                char *shortName = new char[classNames[cpcIdx].size() + 1];
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
                            debug1 << "MILI: Unable to add cell of type "
                                << miliCellTypes[i] << endl;
                            break;
                        }
                    }
                    connPtr += connCount;
                }

                //
                // Retrieve label info for this class. 
                //
                RetrieveZoneLabelInfo(meshId, shortName, dom,
                                      nCells);
                delete [] conn;
                delete [] mats;
                delete [] part;
                delete [] shortName;
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
        delete [] matList;

        //
        // Read initial nodal position information, if available. 
        //
        vtkPoints *initialNodePos = vtkPoints::New();
        initialNodePos->SetNumberOfPoints(nNodes);
        float *nodePosPtr  = (float *) initialNodePos->GetVoidPointer(0);

        if (mc_load_nodes(dbid[dom], meshId, nodeSName, nodePosPtr) == OK)
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
                    nodePosPtr[q+2] = 0.0;
                    nodePosPtr[q+1] = nodePosPtr[r+1];
                    nodePosPtr[q+0] = nodePosPtr[r+0];
                }
            }

            datasets[dom][meshId]->SetPoints(initialNodePos);
            initialNodePos->Delete();
        }
        else
        {
            initialNodePos->Delete();
            initialNodePos = NULL;
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
        debug1 << "MILI: Cannot query QTY_SREC_FMTS! This is bad..." << endl;
        return;
    }
    else if (srec_qty > 1) 
    {
        //
        // According to Kevin, the state record format count is never
        // actually greater than 1. 
        //
        debug1 << "MILI: Encountered multiple state record formats! Is this in "
               << "use now???";
        char msg[512];
        snprintf(msg, 512, "The Mili plugin is not set-up to handle multiple "
            "state record formats. Bailing.");
        EXCEPTION1(ImproperUseException, msg);
    }

    int sRFId     = 0;
    int substates = 0;
    rval = mc_query_family(dbid[dom], QTY_SUBRECS, (void *) &sRFId, NULL,
                           (void *) &substates);
    
    if (rval != OK)
    {
        debug1 << "MILI: Cannot query QTY_SUBRECS! This is bad..." << endl;
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
//      Retrieve the data for the given varPath. 
//
//  Arguments:
//    timestep   The time step of interest.
//    dom        The domain of interest. 
//    varPath    The variable path of interest.         
//
//  Programmer:  Alister Maguire
//  Creation:    Jan 16, 2019
//
//  Modifications
//
// ****************************************************************************

vtkDataArray *
avtMiliFileFormat::GetVar(int timestep, 
                          int dom, 
                          const char *varPath)
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

        const stringVector *miliZoneLabels = 
            miliMetaData[meshId]->GetZoneBasedLabelsPtr(dom);

        std::vector<MiliClassMetaData *> cellMD;
        miliMetaData[meshId]->GetCellBasedClassMD(cellMD);

        return GenerateLabelArray(dom, maxSize, miliZoneLabels, cellMD);
    }

    else if (strcmp("OriginalNodeLabels", varPath) == 0)
    {
        int maxSize = miliMetaData[meshId]->GetMaxNodeLabelLength(dom) + 1;

        const stringVector *miliNodeLabels = 
            miliMetaData[meshId]->GetNodeBasedLabelsPtr(dom);

        std::vector<MiliClassMetaData *> nodeMD;
        miliMetaData[meshId]->GetNodeBasedClassMD(nodeMD);

        return GenerateLabelArray(dom, maxSize, miliNodeLabels, nodeMD);
    }

    MiliVariableMetaData *varMD = 
        miliMetaData[meshId]->GetVarMDByPath(varPath);

    if (varMD == NULL)
    {
        EXCEPTION1(InvalidVariableException, varPath);
    }

    int numTuples = 0;
    if (varMD->GetCentering() == AVT_NODECENT)
    {
        numTuples = miliMetaData[meshId]->GetNumNodes(dom);
    }
    else
    {
        numTuples = miliMetaData[meshId]->GetNumCells(dom);
    }

    vtkFloatArray *fltArray = vtkFloatArray::New();
    fltArray->SetNumberOfTuples(numTuples);

    //
    // If our variable doesn't cover the entire dataset, we want
    // the "empty space" to be rendered grey. Nan values will
    // be mapped to grey. 
    //
    for (int i = 0; i < numTuples; ++i)
    { 
        fltArray->SetTuple1(i, std::numeric_limits<float>::quiet_NaN());
    }

    string shortName = "";
    bool isShared    = false;

    if (varMD->IsElementSet())     
    {
        //
        // If this is an element set, we need to retrieve the requested
        // group info. 
        //
        int gIdx = 
            ((MiliElementSetMetaData *)varMD)->GetGroupIdxByPath(varPath);
        shortName = 
            ((MiliElementSetMetaData *)varMD)->GetGroupShortName(gIdx);
        isShared =
            ((MiliElementSetMetaData *)varMD)->GroupIsShared(gIdx);
    }
    else
    {
        shortName = varMD->GetShortName();
        isShared  = varMD->IsShared();
    }

    if (isShared)
    {
        //
        // If this is a shared variable, we need to retrieve data from 
        // each of the classes that share this variable. 
        //
        SharedVariableInfo *sharedInfo = 
            miliMetaData[meshId]->GetSharedVariableInfo(shortName.c_str());

        if (sharedInfo != NULL)
        {
            std::vector<int> *varIdxs = &(sharedInfo->variableIndicies);

            for (std::vector<int>::iterator itr = varIdxs->begin();
                 itr != varIdxs->end(); ++itr)
            {
                int curIdx = (*itr); 
                MiliVariableMetaData *sharedVarMD = 
                    miliMetaData[meshId]->GetVarMDByIdx(curIdx);

                if (sharedVarMD->IsElementSet())
                {
                    GetElementSetVar(timestep, 
                                     dom, 
                                     meshId, 
                                     shortName,
                                     sharedVarMD, 
                                     fltArray);
                }
                else
                {
                    GetVar(timestep, 
                           dom, 
                           meshId, 
                           sharedVarMD, 
                           fltArray);
                }
              
            }

            visitTimer->StopTimer(gvTimer, "MILI: GetVar");

            return fltArray;
        }

        debug1 << "MILI: Missing shared variable info?!?"; 
        debug1 << "MILI: returning incomplete array...";

        return fltArray;
    }
    else if (varMD->IsElementSet())
    {
        //
        // We can arrive here when an element set has one shared group
        // and another non-shared group that is to be treated as a 
        // scalar. 
        //
        GetElementSetVar(timestep, 
                         dom, 
                         meshId, 
                         shortName,
                         varMD, 
                         fltArray);

        visitTimer->StopTimer(gvTimer, "MILI: GetVar");
        return fltArray;
    }

    //
    // If we're here, we must have a regular scalar that isn't 
    // shared. Just get the variable. 
    //
    GetVar(timestep, dom, meshId, varMD, fltArray);

    visitTimer->StopTimer(gvTimer, "MILI: GetVar");
    return fltArray;
}

                
// ****************************************************************************
//  Method:  avtMiliFileFormat::GetVar
//
//  Purpose:
//      Retrieve the data for the given variable. 
//
//  Arguments:
//    timestep   The time step of interest.
//    dom        The domain of interest. 
//    meshId     The mesh id for this variable. 
//    varMD      The variable metadata. 
//    fltArray   The vtk array to read into. 
//
//  Programmer:  Alister Maguire
//  Creation:    April 15, 2019
//
//  Modifications
//
// ****************************************************************************

void
avtMiliFileFormat::GetVar(int timestep, 
                          int dom, 
                          int meshId, 
                          MiliVariableMetaData *varMD,
                          vtkFloatArray *fltArray)
{
    if (fltArray == NULL)
    {
        debug1 << "MILI: GetVar recieved a null array?! "
               << "This shouldn't happen...";
        char msg[128];
        snprintf(msg, 128, "Data array must be initialized!");
        EXCEPTION1(ImproperUseException, msg);
    }

    SubrecInfo *SRInfo = miliMetaData[meshId]->GetSubrecInfo(dom);

    if (SRInfo == NULL)
    {
        EXCEPTION1(InvalidVariableException, varMD->GetLongName());
    }

    intVector SRIds    = varMD->GetSubrecIds(dom);
    int nSRs           = SRIds.size();
    int vType          = varMD->GetNumType();
    string vShortName  = varMD->GetShortName();

    //
    // Create a copy of our name to pass into mili. 
    //
    char charName[128];
    snprintf(charName, 128, "%s", vShortName.c_str());
    char *namePtr = (char *) charName;

    if (varMD->GetCentering() == AVT_NODECENT)
    {
        int nNodes = miliMetaData[meshId]->GetNumNodes(dom);

        float *fArrPtr = (float *) fltArray->GetVoidPointer(0);

        ReadMiliResults(dbid[dom], timestep + 1, SRIds[0], 1,
            &namePtr, vType, nNodes, fArrPtr);
    }
    else
    {
        //
        // Cell centered variable. 
        //
        int nCells = miliMetaData[meshId]->GetNumCells(dom);
        
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
        // Nans for empty space (redered grey).
        //
        for (int i = 0 ; i < dBuffSize; i++)
        {
            dataBuffer[i] = std::numeric_limits<float>::quiet_NaN();
        }

        //
        // Read the data into our buffer. 
        //
        string className = varMD->GetClassShortName(); 
        int start = miliMetaData[meshId]->
            GetClassMDByShortName(className.c_str())->
            GetConnectivityOffset(dom);

        ReadMiliVarToBuffer(namePtr, SRIds, SRInfo, start, 
            vType, 1, timestep + 1, dom, dataBuffer);

        if (isMatVar)
        {
            //
            // This is a material variable. We need to distribute the 
            // values across cells by material ID. 
            //
            const int *matList = materials[dom][meshId]->GetMatlist();

            for (int i = 0; i < nCells; ++i)
            { 
                if (!visitIsNan(dataBuffer[matList[i]]))
                {
                    fltArray->SetTuple1(i, dataBuffer[matList[i]]);
                }
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
                if (!visitIsNan(dataBuffer[i]))
                {
                    fltArray->SetTuple1(i, dataBuffer[i]);
                }
            }
        }

        if (dataBuffer != NULL)
        {
            delete [] dataBuffer;
        }
    }
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::GetVectorVar
//
//  Purpose:
//      Retrieve the variable data from the given varPath. 
//
//  Arguments:
//    timestep   The time step of interest. 
//    dom        The domain of interest. 
//    varPath    The variable path to look up and retrieve. 
//
//  Programmer:  Alister Maguire
//  Creation:    Jan 16, 2019
//
//  Modifications
//
// ****************************************************************************

vtkDataArray *
avtMiliFileFormat::GetVectorVar(int timestep, 
                                int dom, 
                                const char *varPath)
{
    int gvvTimer = visitTimer->StartTimer();
    int meshId   = ExtractMeshIdFromPath(varPath);
    
    MiliVariableMetaData *varMD = miliMetaData[meshId]->
        GetVarMDByPath(varPath);

    if (varMD == NULL)
    {
        EXCEPTION1(InvalidVariableException, varPath);
    }

    int vecSize      = 0;
    string shortName = "";

    if (varMD->IsElementSet())
    {
        int gIdx = 
            ((MiliElementSetMetaData *)varMD)->GetGroupIdxByPath(varPath);
        shortName = 
            ((MiliElementSetMetaData *)varMD)->GetGroupShortName(gIdx);
        vecSize =
            ((MiliElementSetMetaData *)varMD)->GetGroupVecSize(gIdx);
    }
    else
    {
        vecSize   = varMD->GetVectorSize();
        shortName = varMD->GetShortName();
    }    

    int numTuples = 0;
    if (varMD->GetCentering() == AVT_NODECENT)
    {
        numTuples = miliMetaData[meshId]->GetNumNodes(dom);
    }
    else
    {
        numTuples = miliMetaData[meshId]->GetNumCells(dom);
    }

    vtkFloatArray *fltArray = vtkFloatArray::New();
    fltArray->SetNumberOfComponents(vecSize);
    fltArray->SetNumberOfTuples(numTuples);

    float *fArrPtr = (float *) fltArray->GetVoidPointer(0);
    int arrSize    = numTuples * vecSize;

    //
    // If our variable doesn't cover the entire dataset, we want
    // the "empty space" to be rendered grey. Nan values will
    // be mapped to grey. 
    //
    for (int i = 0; i < arrSize; ++i)
    {
        fArrPtr[i] = std::numeric_limits<float>::quiet_NaN();
    }

    if (varMD->IsShared())
    {
        //
        // If this is a shared variable, we need to retrieve data from 
        // each of the classes that share this variable. 
        //
        SharedVariableInfo *sharedInfo = 
            miliMetaData[meshId]->GetSharedVariableInfo(shortName.c_str());

        if (sharedInfo != NULL)
        {
            std::vector<int> *varIdxs = &(sharedInfo->variableIndicies);

            for (std::vector<int>::iterator itr = varIdxs->begin();
                 itr != varIdxs->end(); ++itr)
            {
                int curIdx = (*itr); 
                MiliVariableMetaData *sharedVarMD = 
                    miliMetaData[meshId]->GetVarMDByIdx(curIdx);

                if (sharedVarMD->IsElementSet())
                {
                    GetElementSetVar(timestep, 
                                     dom, 
                                     meshId, 
                                     shortName,
                                     sharedVarMD, 
                                     fltArray);

                }
                else
                {
                    GetVectorVar(timestep, 
                                 dom, 
                                 meshId, 
                                 sharedVarMD, 
                                 fltArray);
                }
            }
        }
        else
        { 
            debug1 << "MILI: Missing shared variable info?!?"; 
            debug1 << "MILI: returning incomplete array...";

            return fltArray;
        }
    }
    else
    {
        GetVectorVar(timestep, dom, meshId, varMD, fltArray);
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
//  Method:  avtMiliFileFormat::GetVectorVar
//
//  Purpose:
//      Retrieve a vector variable from mili. 
//
//  Arguments:
//    timestep   The time step of interest. 
//    dom        The domain of interest. 
//    meshId     The mesh id associated with this variable. 
//    varMD      The variable meta data. 
//    fltArray   The vtk array to read into. 
//
//  Programmer:  Alister Maguire
//  Creation:    April 15, 2019
//
//  Modifications
//
// ****************************************************************************

void
avtMiliFileFormat::GetVectorVar(int timestep, 
                                int dom, 
                                int meshId, 
                                MiliVariableMetaData *varMD,
                                vtkFloatArray *fltArray)
{
    SubrecInfo *SRInfo = miliMetaData[meshId]->GetSubrecInfo(dom);
    intVector SRIds    = varMD->GetSubrecIds(dom);

    if (varMD->GetComponentDims() > 1)
    {
        debug1 << "Mili: an element set is not being handled properly!";
        debug1 << "Mili: it should be directed to GetElementSetVar";
        return; 
    }
  
    //
    // Create a copy of our name to pass into mili. 
    //
    char charName[128];
    snprintf(charName, 128, "%s", varMD->GetShortName().c_str());
    char *namePtr = (char *) charName;

    int vecSize   = varMD->GetVectorSize();
    int vType     = varMD->GetNumType();
    bool isGlobal = varMD->IsGlobal();

    if (varMD->GetCentering() == AVT_NODECENT)
    {
        //
        // Node centered variables should span a single element (nodes...).
        //
        if (SRIds.size() != 1)
        {
            string path = varMD->GetPath();
            EXCEPTION1(InvalidVariableException, path.c_str());
        }

        int nNodes     = miliMetaData[meshId]->GetNumNodes(dom);
        float *fArrPtr = (float *) fltArray->GetVoidPointer(0);
        
        ReadMiliResults(dbid[dom], timestep+1, SRIds[0], 1,
            &namePtr, vType, nNodes * vecSize, fArrPtr);
    }
    else
    {
        //
        // Cell centered variable. 
        //
        int nCells    = miliMetaData[meshId]->GetNumCells(dom);
        int dBuffSize = 0;;

        float *dataBuffer = NULL;
        if (isGlobal)
        {
            //
            // A global vector will be a single vector of size vecSize. 
            //
            dBuffSize  = vecSize;
            dataBuffer = new float[dBuffSize]; 
        }
        else
        {
            dBuffSize  = nCells * vecSize;
            dataBuffer = new float[dBuffSize];
        }

        //
        // Nans for empty space (rendered grey). 
        //
        for (int i = 0 ; i < dBuffSize; i++)
        {
            dataBuffer[i] = std::numeric_limits<float>::quiet_NaN();
        }

        //
        // Read the data into our buffer. 
        //
        string className = varMD->GetClassShortName(); 
        int start = miliMetaData[meshId]->
            GetClassMDByShortName(className.c_str())->
            GetConnectivityOffset(dom);

        ReadMiliVarToBuffer(namePtr, SRIds, SRInfo, start,
            vType, vecSize, timestep + 1, dom, dataBuffer);

        if (isGlobal)
        {
            //
            // This vector is global. Just apply it to every cell. 
            //
            float *fltArrayPtr = (float *) fltArray->GetVoidPointer(0);
            for (int i = 0; i < nCells; ++i)
            { 
                for (int j = 0; j < vecSize; ++j)
                {
                    fltArrayPtr[i*vecSize + j] = dataBuffer[j];
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
                if (!visitIsNan(dataBuffer[i]))
                {
                    fltArrayPtr[i] = dataBuffer[i];
                }
            }
        }

        if (dataBuffer != NULL)
        {
            delete [] dataBuffer;
        }
    }
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::GetElementSetVar
//
//  Purpose:
//      Retrieve and element set variable from mili. 
//
//      Element Sets (ESs) are special cases that need particular 
//      attention. There are two details of importance:
//
//      1. An element set is basically a 3D array, but we collapse
//         it down to a 2D array. Consider an element set es_x, which
//         consists of an array of length 4 and 3 integration points. 
//         es_x[i][j][k] would retrieve the kth integration point from
//         the jth component of the ith cell, where 0 <= i < numCells,
//         0 <= j < 4, and 0 <= k < 3. Since we only wish to display 
//         one integration point at a time, the user decides which 
//         one to display, and we collapse arr[][][] down to arr[][][c], 
//         where c is the index of the desired integration point. 
//
//      2. An element set can contain multiple "groups" within its data.
//         If we consider again es_x from above, we might find that 
//         the components defined in the range 0 <= j < 3 belong to a 
//         group named "stress" and that the remaining component, j = 3, 
//         belongs to a group named "eps". Even though es_x is considered
//         a single variable, we visualize these groups separetely (which
//         also means that each group can be shared separately). In reality
//         we treat each group as if it is a distinct variable of its own. 
//
//  Arguments:
//    timestep       The time step of interest. 
//    dom            The domain of interest. 
//    meshId         The mesh id associated with this variable. 
//    groupName      The name of the element set's group of interest. 
//    varMD          The variable meta data. 
//    fltArray       The vtk array to read into. 
//
//  Programmer:  Alister Maguire
//  Creation:    May 8, 2019
//
//  Modifications
//
// ****************************************************************************

void
avtMiliFileFormat::GetElementSetVar(int timestep, 
                                    int dom, 
                                    int meshId, 
                                    string groupName,
                                    MiliVariableMetaData *varMD,
                                    vtkFloatArray *fltArray)
{
    //
    // As of now, element sets are always cell centered. 
    //
    if (varMD->GetCentering() != AVT_ZONECENT)
    {
        debug1 << "Mili: Encountered a node centered element set!";
        debug1 << "Mili: We aren't equiped for this... bailing";
        return; 
    }

    SubrecInfo *SRInfo = miliMetaData[meshId]->GetSubrecInfo(dom);
    intVector SRIds    = varMD->GetSubrecIds(dom);

    //
    // Create a copy of our name to pass into mili. 
    //
    char charName[128];
    snprintf(charName, 128, "%s", varMD->GetShortName().c_str());
    char *namePtr = (char *) charName;

    int compDims      = varMD->GetComponentDims();
    int dataSize      = varMD->GetVectorSize() * compDims;
    int vType         = varMD->GetNumType();
    int nCells        = miliMetaData[meshId]->GetNumCells(dom);
    int dBuffSize     = nCells * dataSize;
    float *dataBuffer = new float[dBuffSize];

    //
    // Nans for empty space (rendered grey).   
    //
    for (int i = 0 ; i < dBuffSize; i++)
    {
        dataBuffer[i] = std::numeric_limits<float>::quiet_NaN();
    }

    //
    // Read the data into our buffer. 
    //
    string className = varMD->GetClassShortName(); 
    int start = miliMetaData[meshId]->
        GetClassMDByShortName(className.c_str())->
        GetConnectivityOffset(dom);

    ReadMiliVarToBuffer(namePtr, SRIds, SRInfo, start,
        vType, dataSize, timestep + 1, dom, dataBuffer);

    //
    // We need to determine which of the components have been asked for, 
    // as element sets contain groups that are visualized separately. 
    //
    intVector compIdxs = 
        ((MiliElementSetMetaData *) varMD)->GetGroupComponentIdxs(
        groupName);

    int retVecSize = compIdxs.size();

    //
    //TODO: we need to determine how to let the user choose
    //      the integration point. 
    //      Let's take the mid integration point for now.
    //
    int targetIP = (int)(compDims / 2);

    for (int i = 0 ; i < nCells; i++)
    {
        float *vecPts = new float[retVecSize];
        bool nanFound = false;

        //
        // Element sets are specialized vectors such that each
        // element in the vector is a list of integration points. 
        //
        for (int j = 0; j < compIdxs.size(); ++j)
        {
            int idx = (i * dataSize) + (compIdxs[j] * compDims);
            idx += targetIP;
            if (visitIsNan(dataBuffer[idx]))
            { 
                nanFound = true; 
                break;
            }
            vecPts[j] = dataBuffer[idx];
        }

        if (!nanFound)
        {
            fltArray->SetTuple(i, vecPts);
        }

        delete [] vecPts;
    }

    delete [] dataBuffer;
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
//      Alister Maguire, Thu Dec 19 11:13:35 MST 2019
//      Make sure that we're not overriding the input start value.
//
// ****************************************************************************

void
avtMiliFileFormat::ReadMiliVarToBuffer(char *varName,
                                       const intVector &SRIds,
                                       SubrecInfo *SRInfo, 
                                       int start,
                                       int vType,
                                       int varSize,
                                       int ts,
                                       int dom,
                                       float *dataBuffer)
{
    if (SRInfo == NULL || dataBuffer == NULL)
    {
        return;
    }

    //
    // Loop over the subrecords, and retreive the variable
    // data from mili. 
    //
    for (int i = 0 ; i < SRIds.size(); i++)
    {
        int nTargetEl = 0;
        int nBlocks   = 0;
        int SRId      = SRIds[i];
        intVector blockRanges;

        SRInfo->GetSubrec(SRId,
                          nTargetEl,
                          nBlocks,
                          blockRanges);

        //
        // We only have one block to read. 
        //
        if (nBlocks == 1)
        {
            //
            // Adjust the start.
            //
            int curStart   = start + blockRanges[0] - 1;
            int resultSize = nTargetEl * varSize;
            float *dbPtr   = dataBuffer;

            ReadMiliResults(dbid[dom], ts, SRId,
                1, &varName, vType, resultSize, 
                dbPtr + (curStart * varSize));
        }
        else if (nBlocks > 1)
        {
            int totalBlocksSize = 0;
            for (int b = 0; b < nBlocks; ++b)
            {
                int curStart = blockRanges[b * 2];
                int stop     = blockRanges[b * 2 + 1];
                totalBlocksSize += stop - curStart + 1;
            }

            float *MBBuffer = new float[totalBlocksSize * varSize];
            int resultSize = totalBlocksSize * varSize;

            ReadMiliResults(dbid[dom], ts, SRId,
                1, &varName, vType, resultSize, MBBuffer);

            float *MBPtr = MBBuffer;

            //
            // Fill up the blocks into the array.
            //
            for (int b = 0; b < nBlocks; ++b)
            {
                for (int c = blockRanges[b * 2] - 1; 
                     c <= blockRanges[b * 2 + 1] - 1; ++c)
                {
                    for (int k = 0; k < varSize; ++k)
                    {
                        int idx = (varSize * (c + start)) + k;
                        dataBuffer[idx] = *(MBPtr++);
                    }
                }
            }

            delete [] MBBuffer;
        }
        else
        {
            debug1 << "MILI: ReadMiliVarToBuffer found no data blocks?!?!";
        }
    }
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::AddMiliVariableToMetaData
//
//  Purpose:
//      Add a mili variable to visit's meta-data structures.  
//
//  Arguments:
//    avtMD          The meta-data structure to populate
//    meshId         The associated mesh.
//    avtType        The variable avt type (material, vector, etc).
//    doSand         Whether or not to build the sand mesh. 
//    varPath        The variable's visit path. 
//    centering      The variable centering (node/zone). 
//    compIdxs       The vector component indicies (for element sets,
//                   we may only use some of the total available). 
//    vComps         The vector component names.
//
//  Programmer:  Alister Maguire
//  Creation:    May 8, 2019
//
//  Modifications
//
// ****************************************************************************

void 
avtMiliFileFormat::AddMiliVariableToMetaData(avtDatabaseMetaData *avtMD,
                                             int                  meshId,
                                             int                  avtType,
                                             bool                 doSand,  
                                             string               varPath,
                                             avtCentering         centering,
                                             const intVector     &compIdxs,
                                             const stringVector  &vComps)
{
    //
    // Create a container for all mesh paths that the current
    // variable must be added to. 
    //
    stringVector meshPaths;
    stringVector meshNames;
    int numMeshes = 0;

    //
    // We always add the variable to the default non-sanded mesh.
    //
    meshPaths.push_back(varPath);

    char meshName[32];
    snprintf(meshName, 32, "mesh%d", meshId + 1);
    meshNames.push_back(meshName);
    numMeshes++;

    if (doSand)
    {
        string sandDir  = miliMetaData[meshId]->GetSandDir();
        string sandPath = sandDir + "/" + varPath;
        meshPaths.push_back(sandPath);

        char meshName[32];
        snprintf(meshName, 32, "sand_mesh%d", meshId + 1);
        meshNames.push_back(meshName);
        numMeshes++;
    }

    int vecSize = compIdxs.size();

    //
    // If we have a higher dim vector, determine how we should
    // treat it.
    //
    int refAvtType         = avtType;
    bool mayNeedTypeReform = (avtType == AVT_VECTOR_VAR ||
                              avtType == AVT_MATERIAL);
    if (mayNeedTypeReform && vecSize != dims)
    {
        if (dims == 3)
        {
            if (vecSize == 1)
            {
                refAvtType = AVT_SCALAR_VAR;
            } 
            else if (vecSize == 6)
            {
                refAvtType = AVT_SYMMETRIC_TENSOR_VAR;
            }
            else if (vecSize == 9)
            {
                refAvtType = AVT_TENSOR_VAR;
            }
            else
            {
                //
                // Default to treating it as an array and 
                // just display the components. 
                //
                refAvtType = AVT_ARRAY_VAR;
            }
        }
        else if (dims == 2)      
        {
            if (vecSize == 1)
            {
                refAvtType = AVT_SCALAR_VAR;
            } 
            else if (vecSize == 3)
            {
                refAvtType = AVT_SYMMETRIC_TENSOR_VAR;
            }
            else if (vecSize == 4)
            {
                refAvtType = AVT_TENSOR_VAR;
            }
            else
            {
                //
                // Default to treating it as an array and 
                // just display the components. 
                //
                refAvtType = AVT_ARRAY_VAR;
            }
        }
        else
        {
            debug1 << "Mili: unusual mesh dims.. dims: " << dims;
            return;
        }
    }
    
    switch (refAvtType)
    {
        case AVT_SCALAR_VAR:
        {
            for (int i = 0; i < numMeshes; ++i)
            {
                AddScalarVarToMetaData(avtMD, meshPaths[i], 
                    meshNames[i], centering);
            }
            break;
        }
        case AVT_VECTOR_VAR:
        {
            for (int i = 0; i < numMeshes; ++i)
            {
                string mPath = meshPaths[i];
                AddVectorVarToMetaData(avtMD, mPath, meshNames[i], 
                    centering, vecSize);

                //
                // Add expressions for displaying each component. 
                //
                for (intVector::const_iterator idxItr = compIdxs.begin();
                     idxItr != compIdxs.end(); ++idxItr)
                {
                    //
                    // Add the x, y, z expressions. 
                    //
                    char name[1024];
                    snprintf(name, 1024, "%s/%s", mPath.c_str(), 
                        vComps[(*idxItr)].c_str());
                    Expression expr = ScalarExpressionFromVec(mPath.c_str(),
                                                              name, 
                                                              (*idxItr));
                    avtMD->AddExpression(&expr);
                }
            }
            break;
        }
        case AVT_SYMMETRIC_TENSOR_VAR:
        {
            for (int i = 0; i < numMeshes; ++i)
            {
                string mPath = meshPaths[i];

                //
                // When we come across a vector of length 6, we change it 
                // to a normal vector of length 9 and render it as a 
                // symmetric tensor. 
                //
                AddSymmetricTensorVarToMetaData(avtMD, mPath,
                    meshNames[i], centering, 9);

                //
                // Now we add the individual components. 
                //
                int multDimIdxs[] = {1, 2, 0};

                for (int j = 0; j < 3; ++j)
                {
                    int cIdx = compIdxs[j];

                    // 
                    // First, get the "single-dim" values: xx, yy, zz. 
                    // 
                    Expression singleDim;
                    singleDim.SetType(Expression::ScalarMeshVar);
          
                    char singleDef[256];
                    snprintf(singleDef, 256, "<%s>[%d][%d]", 
                        mPath.c_str(), cIdx, cIdx);
                    singleDim.SetDefinition(singleDef);

                    string singleName = mPath + "/" + vComps[cIdx];
                    singleDim.SetName(singleName);

                    avtMD->AddExpression(&singleDim);

                    //
                    // Next, get the "multi-dim" values: xy, yz, zx
                    //
                    int mltDIdx = cIdx + 3;
                    Expression multDim;
                    multDim.SetType(Expression::ScalarMeshVar);

                    char multDef[256];
                    snprintf(multDef, 256, "<%s>[%d][%d]", mPath.c_str(), 
                        cIdx, multDimIdxs[cIdx]);
                    multDim.SetDefinition(multDef);

                    string multName = mPath + "/" + vComps[mltDIdx];
                    multDim.SetName(multName);

                    avtMD->AddExpression(&multDim);
                }
            }
            break;
        }
        case AVT_TENSOR_VAR:
        {
            for (int i = 0; i < numMeshes; ++i)
            {
                string mPath = meshPaths[i];

                AddTensorVarToMetaData(avtMD, mPath, 
                    meshNames[i], centering, vecSize);

                //
                // Now we add the individual components. 
                //
                int multDimIdxs[] = {1, 2, 0};

                for (int j = 0; j < 3; ++j)
                {
                    int cIdx = compIdxs[j];
 
                    // 
                    // First, get the "single-dim" values: xx, yy, zz. 
                    // 
                    Expression singleDim;
                    singleDim.SetType(Expression::ScalarMeshVar);

                    char singleDef[256];
                    snprintf(singleDef, 256, "<%s>[%d][%d]", 
                        mPath.c_str(), cIdx, cIdx);
                    singleDim.SetDefinition(singleDef);

                    string singleName = mPath + "/" + vComps[cIdx];
                    singleDim.SetName(singleName);

                    avtMD->AddExpression(&singleDim);

                    //
                    // Next, get the "multi-dim" values: xy, yz, zx
                    //
                    int mltDIdx = cIdx + 3;
                    Expression multDim;
                    char multDef[256];
                    snprintf(multDef, 256, "<%s>[%d][%d]", mPath.c_str(), 
                        cIdx, multDimIdxs[cIdx]);
                    string multName = mPath + "/" + vComps[mltDIdx];
                    multDim.SetName(multName);
                    multDim.SetDefinition(multDef);
                    multDim.SetType(Expression::ScalarMeshVar);
                    avtMD->AddExpression(&multDim);
                }
            }
            break;
        }
        case AVT_ARRAY_VAR:
        {
            for (int i = 0; i < numMeshes; ++i)
            {
                string mPath = meshPaths[i];

                //
                // For array vars, we just want to display the 
                // individual components. 
                //
                stringVector compNames;

                for (int j = 0; j < vecSize; ++j)
                {
                    int cIdx = compIdxs[j];
                    compNames.push_back(vComps[cIdx]);
                    char name[1024];
                    snprintf(name, 1024, "%s/%s", mPath.c_str(), 
                        vComps[cIdx].c_str());
                    Expression expr = ScalarExpressionFromVec(
                                          mPath.c_str(),
                                          name, 
                                          cIdx);
                    avtMD->AddExpression(&expr);
                }

                AddArrayVarToMetaData(avtMD, mPath, compNames, meshNames[i],
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


// ****************************************************************************
//  Method:  avtMiliFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Retrieves metadata about the database. 
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
//      Alister Maguire, Fri Jun 28 15:01:24 PDT 2019
//      Added checks to determine if shared variables have been added
//      or not.  
//
//      Alister Maguire, Fri Sep 13 08:37:45 MST 2019
//      We don't need to add the OriginalZone/NodeLabels for the 
//      sand mesh. 
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

        snprintf(meshName, 32, "mesh%d", meshId + 1);
        snprintf(sandMeshName, 64, "sand_mesh%d", meshId + 1);
        snprintf(matName, 32, "materials%d", meshId + 1);

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
                debug1 << "MILI: Missing variable meta data???";
                continue;
            }

            int avtType            = varMD->GetAvtVarType();
            avtCentering centering = varMD->GetCentering();
            stringVector vComps    = varMD->GetVectorComponents();

            if (varMD->IsElementSet())
            {
                const boolVector groupShared = 
                    ((MiliElementSetMetaData *)varMD)->GetGroupIsShared();
                
                //
                // Element sets can contain groups of vector components
                // that are treated as distinct variables. We need to 
                // check for this and add them individually. 
                //
                for (int j = 0; j < groupShared.size(); ++j)
                {
                    bool addVarNow = true;
                    SharedVariableInfo *sharedInfo = NULL;

                    if (groupShared[j])
                    {
                        string sName = 
                            ((MiliElementSetMetaData *)varMD)->GetGroupShortName(j);

                        sharedInfo = 
                            miliMetaData[meshId]->GetSharedVariableInfo(
                            sName.c_str());

                        //
                        // If this ES is shared with non-ES variables, let's
                        // be lazy and wait for the non-ES to add them. 
                        //
                        if (!sharedInfo->isAllES)
                        {
                            addVarNow = false;
                        }
                    }

                    if (addVarNow)
                    {
                        string varPath = 
                            ((MiliElementSetMetaData *)varMD)->GetGroupPath(j);
                        intVector compIdxs = 
                            ((MiliElementSetMetaData *)varMD)->
                            GetGroupComponentIdxs(j);

                        AddMiliVariableToMetaData(md,
                                                  meshId,
                                                  avtType,
                                                  doSand,  
                                                  varPath,
                                                  centering,
                                                  compIdxs,
                                                  vComps);

                        //
                        // If this is a shared variable, let's signify that we
                        // no longer need to add it. 
                        //
                        if (sharedInfo != NULL)
                        {
                            sharedInfo->isLive = true;
                        }
                    }
                }
            }
            else
            {
                string varPath = varMD->GetPath();
                intVector compIdxs; 
                compIdxs.reserve(vComps.size());

                const int vSize = vComps.size();
                for (int i = 0; i < vSize; ++i)
                {
                    compIdxs.push_back(i);
                }

                if (varMD->IsShared())
                {
                    SharedVariableInfo *sharedInfo = 
                        miliMetaData[meshId]->GetSharedVariableInfo(
                        varMD->GetShortName().c_str());

                    //
                    // Check to see if one of the shared vars has already been
                    // added. It only needs to occur once. 
                    //
                    if (!sharedInfo->isLive)
                    {
                        sharedInfo->isLive = true;
                        AddMiliVariableToMetaData(md,
                                                  meshId,
                                                  avtType,
                                                  doSand,  
                                                  varPath,
                                                  centering,
                                                  compIdxs,
                                                  vComps);
                    }
                }
                else
                {
                    AddMiliVariableToMetaData(md,
                                              meshId,
                                              avtType,
                                              doSand,  
                                              varPath,
                                              centering,
                                              compIdxs,
                                              vComps);
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
        string strVarName(varName);
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
//      jDoc         The json document (needed for element sets). 
//      val          A json Value type to extract from.
//      shortName    The variable's short name.        
//      cShortName   The variable's class' short name. 
//      cLongName    The Variable's class' long name. 
//      meshId       The associated mesh ID. 
//      isMatVar     Is this a material variable?
//      isGlobal     Is this a global variable?
//      sharedMap    Map variable names to shared status. 
//           
//  Author: Alister Maguire
//  Date:   January 29, 2019
//
//  Modifications:
//
// ****************************************************************************

MiliVariableMetaData * 
avtMiliFileFormat::ExtractJsonVariable(const rapidjson::Document &jDoc,
                                       const rapidjson::Value &val,
                                       string shortName,
                                       string cShortName,
                                       string cLongName,
                                       int meshId,
                                       bool isMatVar,
                                       bool isGlobal,
                                       StrToIntMap &sharedMap)
{
    if (val.IsObject())
    {
        string longName        = "";
        avtCentering centering = AVT_NODECENT;
        int avtType            = -1;
        int aggType            = -1;
        int vecSize            = 0;
        int compDims           = 1;
        int numType            = M_FLOAT;
        bool isShared          = false;
        bool isMulti           = (nMeshes > 1);

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
            const rapidjson::Value &vComps = val["vector_components"];

            if (vComps.IsArray())
            {
                for (rapidjson::SizeType i = 0; i < vComps.Size(); ++i)
                {
                    comps.push_back(vComps[i].GetString());
                }
            }
        }

        //
        // "real_names" should only really appear with element sets,
        // but older datasets treated element sets as separate variables
        // (name_in, name_mid, name_out). In those older cases, we need
        // to treat them like normal variables, but they still have 
        // "real_names" defined. 
        //
        if (val.HasMember("real_names") && 
            avtMiliMetaData::ContainsESFlag(shortName.c_str(), 
                                            shortName.size()))
        {
            const rapidjson::Value &rN    = val["real_names"];
            const rapidjson::Value &jVars = jDoc["Variables"];

            stringVector groupNames;
            intVector    groupAvtTypes;
            intVector    groupAggTypes;
            intVector    groupVectorSizes;
            boolVector   groupIsShared;
 
            if (rN.IsArray())
            {
                for (rapidjson::SizeType i = 0; i < rN.Size(); ++i)
                {
                    string groupName = rN[i].GetString();
                    groupNames.push_back(groupName);  

                    //
                    // Determine if this group is shared or not.  
                    //
                    if (sharedMap[groupName] > 1)
                    {
                        groupIsShared.push_back(true);
                        isShared = true;
                    }
                    else
                    {
                        groupIsShared.push_back(false);
                    }

                    if (jVars.HasMember(groupName.c_str()))
                    {
                        const rapidjson::Value &var = jVars[groupName.c_str()];

                        if (var.HasMember("VTK_TYPE"))
                        {
                            groupAvtTypes.push_back(var["VTK_TYPE"].GetInt());
                        }

                        if (var.HasMember("agg_type"))
                        {
                            groupAggTypes.push_back(var["agg_type"].GetInt());
                        }

                        if (var.HasMember("vector_size"))
                        {
                            groupVectorSizes.push_back(
                                var["vector_size"].GetInt());
                        }
                    }
                }
            }
            return new MiliElementSetMetaData(shortName, 
                                              longName,
                                              cShortName,
                                              cLongName,
                                              isMulti,
                                              isMatVar,
                                              isGlobal,
                                              isShared,
                                              centering,
                                              nDomains,
                                              meshId,
                                              avtType,
                                              aggType,
                                              numType,
                                              vecSize,
                                              compDims,
                                              comps,
                                              groupNames,
                                              groupVectorSizes,
                                              groupAvtTypes,
                                              groupAggTypes,
                                              groupIsShared);
        }

        //
        // Determine if this variable is shared or not.  
        //
        if (sharedMap[shortName] > 1)
        {
            isShared = true; 
        }

        return new MiliVariableMetaData(shortName, 
                                        longName,
                                        cShortName,
                                        cLongName,
                                        isMulti,
                                        isMatVar,
                                        isGlobal,
                                        isShared,
                                        false,    //ES status
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
//      mili Classes within the .mili json file, and count the 
//      occurences of shared variables. The sharedMap will
//      map the variable short name to its number of occurences
//      (a single count for its own existence).  
//
//  Arguments: 
//      jDoc         The json document. 
//      sharedCount  A map which identifies which variables are shared. 
//           
//  Author: Alister Maguire
//  Date:   January 29, 2019
//
//  Modifications:
//
// ****************************************************************************

int
avtMiliFileFormat::CountJsonClassVariables(const rapidjson::Document &jDoc,
                                           StrToIntMap &sharedMap)
{
    const rapidjson::Value &jClasses = jDoc["Classes"]; 
    int numClassVars = 0;
    for (rapidjson::Value::ConstMemberIterator cItr = jClasses.MemberBegin();
         cItr != jClasses.MemberEnd(); ++cItr)
    { 
        const rapidjson::Value &val = cItr->value;
        if (val.IsObject())
        { 
            if (val.HasMember("variables"))
            {
                const rapidjson::Value &cVars = val["variables"];

                if (cVars.IsArray())
                {
                    numClassVars += cVars.Size(); 

                    for (rapidjson::SizeType i = 0; i < cVars.Size(); ++i)
                    {
                        const rapidjson::Value &jVars = jDoc["Variables"];
                        const rapidjson::Value &var   = jVars[cVars[i]];

                        if (jVars.HasMember(cVars[i].GetString()))
                        {
                            string varName = cVars[i].GetString();

                            if (var.HasMember("vector_components"))
                            {
                                std::vector<string> comps;

                                const rapidjson::Value &vC = 
                                    var["vector_components"];

                                if (vC.IsArray())
                                {
                                    for (rapidjson::SizeType i = 0; 
                                         i < vC.Size(); ++i)
                                    {
                                        comps.push_back(vC[i].GetString());
                                    }
                                }
                            }
                           
                            //
                            // Element sets are often comprised of multiple
                            // variables. We need to check for this. 
                            //
                            const char *cName = varName.c_str();
                            int nameSize      = varName.size(); 

                            if (var.HasMember("real_names") &&
                                avtMiliMetaData::ContainsESFlag(cName,
                                                                nameSize))
                            {
                                const rapidjson::Value &rN = var["real_names"];
 
                                if (rN.IsArray())
                                {
                                    for (rapidjson::SizeType i = 0; 
                                         i < rN.Size(); ++i)
                                    {
                                        string name = rN[i].GetString(); 

                                        if (sharedMap.find(name) ==
                                            sharedMap.end())
                                        {
                                            sharedMap[name] = 1;
                                        }
                                        else
                                        {
                                            sharedMap[name]++;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (sharedMap.find(varName) == sharedMap.end())
                                {
                                    sharedMap[varName] = 1;
                                }
                                else
                                {
                                    sharedMap[varName]++;
                                }
                            }
                        }
                    }
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
//      jDoc        The json document. 
//      meshId      The current mesh ID. 
//      sharedMap   A map that associates mili variables with the number of
//                  times they are shared (1 is shared only with itself). 
//           
//  Author: Alister Maguire
//  Date:   January 29, 2019
//
//  Modifications:
//
// ****************************************************************************

void 
avtMiliFileFormat::ExtractJsonClasses(rapidjson::Document &jDoc,
                                      int meshId,
                                      StrToIntMap &sharedMap)
{
    const rapidjson::Value &jClasses = jDoc["Classes"]; 

    //
    // Keep track of how many variables and classes 
    // we've added. 
    //
    int varIdx   = 0;
    int classIdx = 0;

    int cCount = 0;
    if (jClasses.HasMember("count"))
    {
        //
        // Counts may be in scientific notation. We need to retrieve 
        // as a double and cast to int. 
        //
        double dbl = jClasses["count"].GetDouble();
        cCount = (int) dbl;
    }

    //
    // Set the number of classes and initialize our
    // class container. 
    //
    miliMetaData[meshId]->SetNumClasses(cCount);

    rapidjson::Value::ConstMemberIterator cItr;
    for (cItr = jClasses.MemberBegin(); cItr != jClasses.MemberEnd(); ++cItr)
    { 
        const rapidjson::Value &val = cItr->value;
        string sName = cItr->name.GetString();

        if (val.IsObject())
        { 
            //
            // Check for special cases. As of now, these are material
            // and global variables. 
            //
            string matFlag  = "mat";
            string globFlag = "glob";
            bool isGlobal   = false;
            bool isMatVar   = false;

            if (sName == matFlag)
            {
                isMatVar = true;
            }
            else if (sName == globFlag)
            {
                isGlobal = true;
            }
 
            string lName = "";
            int scID     = -1;
            int elCount  = 0;

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
                const rapidjson::Value &cVars = val["variables"];
                
                //
                // Extract the class variable meta data. 
                //
                for (rapidjson::SizeType i = 0; i < cVars.Size(); ++i)
                {
                    const rapidjson::Value &jVars = jDoc["Variables"];

                    if (jVars.HasMember(cVars[i].GetString()))
                    {
                        string varName = cVars[i].GetString();
                        const rapidjson::Value &var = jVars[cVars[i]];
                        
                        MiliVariableMetaData *varMD = ExtractJsonVariable(jDoc,
                            var, varName, sName, lName, meshId, isMatVar, 
                            isGlobal, sharedMap);

                        if (varMD != NULL)
                        {
                            miliClass->AddMiliVariable(varName);
                            miliMetaData[meshId]->AddVarMD(varIdx, varMD);
          
                            varIdx++;
                        }
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
//      Alister Maguire, Tue Jul  9 13:31:59 PDT 2019
//      Check that we have the correct file format. JSON tends to hang when
//      it tries to open non-json files. 
//
// ****************************************************************************
void
avtMiliFileFormat::LoadMiliInfoJson(const char *fpath)
{
    int jsonTimer1 = visitTimer->StartTimer(); 

    //
    // First, we need to open the file and make sure that it's the 
    // newer JSON format.
    //
    std::ifstream jfile(fpath);

    char first;
    jfile.get(first);
    if (first != '{')
    {
        char msg[512];
        snprintf(msg, 512, "Invalid Mili file. You are likely using an outdated "
            "format. To update your format, use the makemili_driver located "
            "in the bin directory. ");

        debug1 << "MILI: " <<  msg;
        EXCEPTION2(NonCompliantFileException, "Mili", msg);
    }
    jfile.clear();
    jfile.seekg(0, ios::beg);

    rapidjson::IStreamWrapper isw(jfile);
    rapidjson::Document jDoc;
    jDoc.ParseStream(isw);

    if (jDoc.HasMember("Domains"))
    {
        //
        // Counts may be in scientific notation. We need to retrieve 
        // as a double and cast to int. 
        //
        double dbl = jDoc["Domains"].GetDouble(); 
        nDomains   = (int) dbl;
    }
    else
    {
        debug1 << "MILI: Mili file missing domains?!?!?!\n";
        char msg[128];
        snprintf(msg, 128, "Mili file is missing domains!!");
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
        debug1 << "MILI: Mili file missing dims?!?!?!\n";
        char msg[128];
        snprintf(msg, 128, "Mili file is missing dims!!");
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
            const rapidjson::Value &jMats = jDoc["Materials"]; 
            int   nMats = 0;

            if (jMats.HasMember("count"))
            {
                //
                // Counts may be in scientific notation. We need to retrieve 
                // as a double and cast to int. 
                //
                double dbl = jMats["count"].GetDouble();
                nMats      = (int) dbl;
            }

            miliMetaData[meshId]->SetNumMaterials(nMats);

            //
            // Retrieve material meta-data. 
            //
            int matCount = 0;
            rapidjson::Value::ConstMemberIterator jItr;
            for (jItr = jMats.MemberBegin(); jItr != jMats.MemberEnd(); ++jItr)
            { 
                string name = jItr->name.GetString();
                const rapidjson::Value &mat = jItr->value;

                string matName = "";
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
                        snprintf(buff, 128, "%d", matCount);
                        matName = string(buff);
                    }

                    if (mat.HasMember("COLOR"))
                    {
                        const rapidjson::Value &mColors = mat["COLOR"];

                        //
                        // Mili stores its mat color as a float rgb. We
                        // need to convert this to a hex string. 
                        //
                        if (mColors.IsArray())
                        {
                            for (rapidjson::SizeType i = 0; 
                                 i < mColors.Size(); ++i)
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
                                                 string(colorSS.str()));

                    miliMetaData[meshId]->AddMaterialMD(matCount++, 
                                                        miliMaterial);
                }
            }
        }

        if (jDoc.HasMember("Variables"))
        {
            StrToIntMap sharedMap;
            int numClassVars = CountJsonClassVariables(jDoc, sharedMap);

            miliMetaData[meshId]->SetNumVariables(numClassVars);

            ExtractJsonClasses(jDoc, meshId, sharedMap);
        }

        if (jDoc.HasMember("States"))
        {
            const rapidjson::Value &jStates = jDoc["States"];
            if (jStates.HasMember("count"))
            {
                //
                // Counts may be in scientific notation. We need to retrieve 
                // as a double and cast to int. 
                //
                double dbl = jStates["count"].GetDouble();
                nTimesteps = (int) dbl;
            }

            if (jStates.HasMember("times"))
            {

                const rapidjson::Value &jTimes = jStates["times"];
                times.clear();
                cycles.clear();

                if (jTimes.IsArray())
                {
                    for (rapidjson::SizeType i = 0; i < jTimes.Size(); ++i)
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

    datasets  = new vtkUnstructuredGrid**[nDomains];
    materials = new avtMaterial**[nDomains];

    for (int dom = 0; dom < nDomains; ++dom)
    {
        datasets[dom]  = new vtkUnstructuredGrid*[nMeshes];
        materials[dom] = new avtMaterial*[nMeshes];

        for (int m = 0; m < nMeshes; ++m)
        {
            datasets[dom][m]  = NULL;
            materials[dom][m] = NULL;
        }
    }

    jfile.close();
    
    visitTimer->StopTimer(jsonTimer1, "MILI: Loading Mili Json Data");
    visitTimer->DumpTimings();
}



// ***************************************************************************
//  Function: RetrieveZoneLabelInfo
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
avtMiliFileFormat::RetrieveZoneLabelInfo(const int meshId, 
                                         char *shortName, 
                                         const int dom, 
                                         const int nElemsInClass)
{
    int numBlocks    = 0; 
    int *blockRanges = NULL;
    int *elemList    = new int[nElemsInClass];
    int *labelIds    = new int[nElemsInClass];

    for (int i = 0; i < nElemsInClass; ++i)
    {
        elemList[i] = -1;
        labelIds[i] = -1;
    }
    
    int loadZoneLabels = visitTimer->StartTimer();

    //
    // Check for labels
    //
    int nExpectedLabels = nElemsInClass;
    int rval = mc_load_conn_labels(dbid[dom], meshId, shortName, 
                                   nExpectedLabels, &numBlocks, 
                                   &blockRanges, elemList, labelIds);

    if (rval != OK)
    {
        debug1 << "MILI: mc_load_conn_labels failed at " << shortName << "!\n";
        numBlocks   = 0; 
        blockRanges = NULL;
    }

    MiliClassMetaData *miliClass = 
        miliMetaData[meshId]->GetClassMDByShortName(shortName);
    miliClass->PopulateLabelIds(dom, 
                                labelIds,
                                numBlocks,
                                blockRanges);

    //
    // Mili mallocs blockRanges using C style. 
    //
    if (blockRanges != NULL)
    {
        free(blockRanges);
    }
    delete [] elemList;
    delete [] labelIds;
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
avtMiliFileFormat::RetrieveNodeLabelInfo(const int meshId, 
                                         char *shortName, 
                                         const int dom)
{
    int nLabeledNodes = 0;
    int nNodes        = miliMetaData[meshId]->GetNumNodes(dom);
    int numBlocks     = 0; 
    int *blockRanges  = NULL;
    int *elemList     = new int[nNodes];
    int *labelIds     = new int[nNodes];

    for (int i = 0; i < nNodes; ++i)
    {
        labelIds[i] = -1;
        elemList[i] = -1;
    }

    int rval = mc_load_node_labels(dbid[dom], meshId, shortName, 
                                   &numBlocks, &blockRanges, labelIds);

    if (rval != OK)
    {
        debug1 << "MILI: mc_load_node_labels failed!\n";
        numBlocks   = 0; 
        blockRanges = NULL;
    }

    MiliClassMetaData *miliClass = 
        miliMetaData[meshId]->GetClassMDByShortName(shortName);
    miliClass->PopulateLabelIds(dom, 
                                labelIds,
                                numBlocks,
                                blockRanges);

    //
    // Mili mallocs blockRanges using C style. 
    //
    if (blockRanges != NULL)
    {
        free(blockRanges);
    }
    delete [] elemList;
    delete [] labelIds;
}


// ***************************************************************************
//  Function: avtMiliFileFormat::GenerateLabelArray
//
//  Purpose:
//      Generate a vtkElementLabelArray that contains mili labels. 
//
//  Arguments: 
//      dom              The domain of interest. 
//      maxLabelLength   The maximum label lenght. 
//      labels           The labels to add. 
//
//  Returns:
//      A vtkElementLabelArray. 
//
//  Author: Alister Maguire
//  Date:   May 20, 2019
//
//  Modifications:
//
// ****************************************************************************

vtkElementLabelArray *
avtMiliFileFormat::GenerateLabelArray(int dom,
                                      int maxLabelLength,
                                      const stringVector *labels,
                                      std::vector<MiliClassMetaData *> labelMD)
{
    vtkElementLabelArray *vtkLabels = 0;
    vtkLabels = vtkElementLabelArray::New();
    vtkLabels->SetNumberOfComponents(maxLabelLength);

    const int nEls = labels->size();
    vtkLabels->SetNumberOfTuples(nEls);

    char *ptr = (char *) vtkLabels->GetVoidPointer(0);

    for(int i = 0; i < nEls; ++i)
    {
        const int offset     = i * maxLabelLength;
        const char * elLabel = (*labels)[i].c_str();
        const int cSize      = (*labels)[i].size();
        for(int j = 0; j < maxLabelLength; ++j)
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
    // Add the data so we can do reverse lookups.
    //
    std::vector<MiliClassMetaData *>::iterator mdItr;
    for (mdItr = labelMD.begin(); mdItr != labelMD.end(); ++mdItr)
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
    snprintf(def, 256, "<%s>[%d]", vecPath, dim);
    Expression::ExprType eType = Expression::ScalarMeshVar;
    return CreateGenericExpression(scalarPath, def, eType);
}
