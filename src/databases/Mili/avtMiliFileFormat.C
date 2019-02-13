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
#include <avtMiliMetaData.h>

#include <vector>
#include <string>
#include <sstream> 
#include <snprintf.h>
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
#include <avtGhostData.h>
#include <avtMaterial.h>
#include <avtVariableCache.h>
#include <avtUnstructuredPointBoundaries.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>
#include <UnexpectedValueException.h>
#include <sstream>

using namespace rapidjson;
using std::ifstream;
using std::map;
using std::string;
using std::vector;

#define Warn(msg)          IssueWarning(msg, __LINE__)

// ****************************************************************************
//  Method:  avtMiliFileFormat::IssueWarning
//
//  Purpose: Convenience method to issue warning messages. Manages number of
//      times a given warning message will be output
//
//  Programmer:  Mark C. Miller 
//  Creation:    January 4, 2005 
//
// ****************************************************************************
void
avtMiliFileFormat::IssueWarning(const char *msg, int key)
{
    if (warnMap.find(key) == warnMap.end())
        warnMap[key] = 1;
    else
        warnMap[key]++;

    if (warnMap[key] <= 5)
    {
        if (!avtCallback::IssueWarning(msg))
            cerr << msg << endl;
    }

    if (warnMap[key] == 5)
    {
        const char *smsg = "\n\nFurther warnings will be suppresed";
        if (!avtCallback::IssueWarning(smsg))
            cerr << smsg << endl;
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
    dims     = 0;
    nDomains = 0;
    nMeshes  = 0;
    LoadMiliInfoJson(fpath);

    string fnamePth(fpath);

    //TODO: make sure this works for windows paths as well. 
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
//  Function: read_results
//
//  Purpose:
//      A wrapper around mc_read_results that handles multiple types (floats,
//      doubles, etc.).
//
//  Programmer: Hank Childs
//  Creation:   July 20, 2004
//
//  Modifications:
//
//    Mark C. Miller, Mon Jul 18 13:41:13 PDT 2005
//    Added logic to read "param arrays" via a different Mili API call. Note
//    that param arrays are always alloc'd by Mili
// ****************************************************************************

static void
read_results(Famid &dbid, int ts, int sr, int rank,
             char **name, int vtype, int amount, float *buff)
{
    bool isParamArray = strncmp(*name, "params/", 7) == 0;

    void *buff_to_read_into = NULL;
    if (!isParamArray)
    {
        switch (vtype)
        {
          case M_STRING:
            buff_to_read_into = new char[amount];
            break;
          case M_FLOAT:
          case M_FLOAT4:
            buff_to_read_into = buff;
            break;
          case M_FLOAT8:
            buff_to_read_into = new double[amount];
            break;
          case M_INT:
          case M_INT4:
            buff_to_read_into = new int[amount];
            break;
          case M_INT8:
            buff_to_read_into = new long[amount];
            break;
        }
    }

    int rval;
    //TODO: can we get rid of param arrays?
    if (isParamArray)
    {
        char tmpName[256];
        strcpy(tmpName, &(*name)[7]);
        rval = mc_read_param_array(dbid, tmpName, &buff_to_read_into);

        if (rval == OK && (vtype == M_FLOAT || vtype == M_FLOAT4))
        {
            float *pflt = (float *) buff_to_read_into;
            for (int i = 0 ; i < amount ; i++)
            {
                buff[i] = (float)(pflt[i]);
            }
        }
    }
    else
    {
        //TODO: the rank variable is very missleading. This is supposed to be
        // the number of state variables we wish to retrieve. Change this. 
        rval = mc_read_results(dbid, ts, sr, rank, name, buff_to_read_into);
    }

    if (rval != OK)
    {
        EXCEPTION1(InvalidVariableException, name[0]);
    }

    char   *c_tmp = NULL;
    double *d_tmp = NULL;
    int    *i_tmp = NULL;
    long   *l_tmp = NULL;
    switch (vtype)
    {
       case M_STRING:
         c_tmp = (char *) buff_to_read_into;
         for (int i = 0 ; i < amount ; i++)
             buff[i] = (float)(c_tmp[i]);
         delete [] c_tmp;
         break;
       case M_FLOAT:
       case M_FLOAT4:
         break;
       case M_FLOAT8:
         d_tmp = (double *) buff_to_read_into;
         for (int i = 0 ; i < amount ; i++)
             buff[i] = (float)(d_tmp[i]);
         delete [] d_tmp;
         break;
       case M_INT:
       case M_INT4:
         i_tmp = (int *) buff_to_read_into;
         for (int i = 0 ; i < amount ; i++)
             buff[i] = (float)(i_tmp[i]);
         delete [] i_tmp;
         break;
       case M_INT8:
         l_tmp = (long *) buff_to_read_into;
         for (int i = 0 ; i < amount ; i++)
             buff[i] = (float)(l_tmp[i]);
         delete [] l_tmp;
         break;
       default:
         break;
    }

    if (isParamArray)
    {
        free(buff_to_read_into);
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
    debug5 << "Reading in " << mesh << " for domain/ts : " << 
        dom << ',' << ts << endl;

    //
    // The valid meshnames are meshX, where X is an int > 0.
    // We need to verify the name, and get the meshId.
    //
    //if (strstr(mesh, "mesh") != mesh)
    //{
    //    EXCEPTION1(InvalidVariableException, mesh);
    //}

    bool isSandMesh = false;
    if (strstr(mesh, "sand_mesh") == mesh)
    {
        isSandMesh == true;
    }
    else if (strstr(mesh, "mesh") != mesh)
    {
        EXCEPTION1(InvalidVariableException, mesh);
    }
    
    //
    // Do a checked conversion to integer.
    //
    char *check = 0;
    int meshId;
    meshId = (int) strtol(mesh + 4, &check, 10);
    if (meshId == 0 || check == mesh + 4)
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
        GetVarMDByShortName("nodpos");

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
                char msg[1024];
                SNPRINTF(msg, sizeof(msg),
                    "Unable to find coords for domain %d. Skipping it", dom);
                Warn(msg);

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
            read_results(dbid[dom], ts+1, subrec, 1, 
                &npCharPtr, vType, nPts, fPts);

            vtkPoints *vtkPts = vtkPoints::New();
            vtkPts->SetNumberOfPoints(nNodes);
            float *vtkPtsPtr = (float *) vtkPts->GetVoidPointer(0);
            float *fPtsPtr = fPts; 

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

            //TODO: handle sanded elements. 
            if (isSandMesh)
            {
                //Get a list of indicies corresponding with 
                //sand variables?
                //OR, we could add an option to GetVar that 
                //allows us to retrieve the sand class? Is that
                //possible, though? Nope. Don't think so...
                //MiliVariableMetaData *sand = 
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
            char msg[1024];
            SNPRINTF(msg, sizeof(msg),
                "Unable to find coords for domain %d. Skipping it", dom);
            Warn(msg);

            //
            // null out the returned grid
            //
            rv->Delete();
            rv = vtkUnstructuredGrid::New();
            return rv;
        }
    }

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
                    break;
            }
            if ( rval != OK )
            {
                EXCEPTION1(InvalidFilesException, famname);
            }
        }
    }
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
        int nodeId       = 0;
        int nNodes       = 0;
        char shortName[1024];
        char longName[1024];

        int rval = mc_get_class_info(dbid[dom], meshId, M_NODE,
             nodeId, shortName, longName, &nNodes);
        if (rval != OK)
        {
            char msg[1024];
            sprintf(msg, "Unable to retrieve %s from mili", shortName);
            EXCEPTION1(ImproperUseException, msg);
        }

        miliMetaData[meshId]->SetNumNodes(dom, nNodes);

        //PopulateNodeLabels(dbid[dom], meshId, nodeSName, dom);//FIXME: 

        //
        // Read initial nodal position information, if available. 
        //
        vtkPoints *vtkPts = vtkPoints::New();
        vtkPts->SetNumberOfPoints(nNodes);
        float *vtkPtsPtr  = (float *) vtkPts->GetVoidPointer(0);
        bool havePts = false;
        if (dims == 2)
        {
             if (mc_load_nodes(dbid[dom], meshId, nodeSName, vtkPtsPtr) == 0)
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
                 havePts = true;
             }
        }
        if (!havePts)
        {
            vtkPts->Delete();
            vtkPts = NULL;
        }

        //
        // Mili has its own definitions for cell types. The
        // heirarchy is CellType->ClassType->...
        // We need to extract data one Class type at a time. 
        //
        vector<int> nClassesPerCellType;
        vector<int> miliCellTypes;
        miliMetaData[meshId]->GetCellTypeCounts(
            miliCellTypes, nClassesPerCellType);
        int nCellTypes = miliCellTypes.size();

        //
        // Now determine the number of cells for each class and 
        // get some basic info for future mc calls. 
        //
        int offset    = 0;
        int nDomCells = 0;
        vector<int>    numCellsPerClass;
        vector<string> classNames;
        for (int i = 0 ; i < nCellTypes ; i++)
        {
            for (int j = 0 ; j < nClassesPerCellType[i]; j++)
            {
                int nCells = 0;
                char shortName[1024];
                char longName[1024];

                mc_get_class_info(dbid[dom], meshId, miliCellTypes[i], j,
                                  shortName, longName, &nCells);

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

        // TODO: should we set the num elements for classes not set
        // above? (element sets, mat variables, glob variables...)
       
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
        int cellIdx   = 0;
        int cpcIdx    = 0;
        int matList[nDomCells];
        max_zone_label_lengths[dom] = 0;

        for (int i = 0; i < nCellTypes; ++i)
        {
            for (int j = 0; j < nClassesPerCellType[i]; ++j)
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

                //
                // Create a conn pointer that we can move around
                // in memory without offsetting the original.
                //
                int *connPtr = conn;

                if (rval != OK)
                {
                    EXCEPTION1(InvalidVariableException, shortName);
                }
    
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
                            datasets[dom][meshId]->InsertNextCell(VTK_LINE,
                                                     connCount, verts);
                            break;
                        case M_BEAM:
                            //
                            // Beams are lines that have a third point to 
                            // define the normal. Since we don't need to 
                            // visualize it, we just drop the normal point.
                            //
                            datasets[dom][meshId]->InsertNextCell(VTK_LINE,
                                                     2, verts);
                            break;
                        case M_TRI:
                            datasets[dom][meshId]->InsertNextCell(VTK_TRIANGLE,
                                                     connCount, verts);
                            break;
                        case M_QUAD:
                            datasets[dom][meshId]->InsertNextCell(VTK_QUAD,
                                                     connCount, verts);
                            break;
                        case M_TET:
                            datasets[dom][meshId]->InsertNextCell(VTK_TETRA,
                                                     connCount, verts);
                            break;
                        case M_PYRAMID:
                            datasets[dom][meshId]->InsertNextCell(VTK_PYRAMID,
                                                     connCount, verts);
                            break;
                        case M_WEDGE:
                            datasets[dom][meshId]->InsertNextCell(VTK_WEDGE,
                                                     connCount, verts);
                            break;
                        case M_HEX:
                            if (connPtr[2] == connPtr[3] && 
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
                        default:
                            debug1 << "Unable to add cell of type "
                                << miliCellTypes[i] << endl;
                            break;
                    }
                    connPtr += connCount;
                }

                //PopulateZoneLabels(dbid[dom], meshId, shortName, dom, 
                //                   nCells);//FIXME
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
}


// ****************************************************************************
//  Method: avtMiliFileFormat::PopulateSubrecordInfo
//
//  Purpose:
//      Retrieve needed info from the subrecords. 
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
    //TODO: This is pretty horrible and slow. Once Kevin puts the subrecord
    //      info in the mili file, we should be able to refactor this and 
    //      make it much faster. 
    //
    int rval;
    int srec_qty = 0;
    rval = mc_query_family(dbid[dom], QTY_SREC_FMTS, NULL, NULL,
                           (void*) &srec_qty);

    if (rval != OK)
    {
        debug1 << "Cannot query mili family! This is bad..." << endl;
        return;
    }

    for (int i = 0 ; i < srec_qty ; i++)
    {
        int substates = 0;
        rval = mc_query_family(dbid[dom], QTY_SUBRECS, (void *) &i, NULL,
                               (void *) &substates);
       
        if (rval != OK)
        {
            continue;
        }

        for (int srId = 0 ; srId < substates ; srId++)
        {
            Subrecord sr;
            memset(&sr, 0, sizeof(sr));
            rval = mc_get_subrec_def(dbid[dom], i, srId, &sr);

            if (rval != OK)
            {
                continue;
            }
            
            for (int k = 0 ; k < sr.qty_svars ; k++)
            {
                 State_variable sv;
                 memset(&sv, 0, sizeof(sv));
                 mc_get_svar_def(dbid[dom], sr.svar_names[k], &sv);
                 
                 int MDVarIdx = -1;
                 MDVarIdx     = miliMetaData[meshId]->
                     GetVarMDIdxByShortName(sv.short_name); 
                 if (MDVarIdx != -1)
                 {
                     miliMetaData[meshId]->AddVarSubrecInfo(MDVarIdx,
                                                            dom,
                                                            srId,
                                                            &sr);
                 }
                 
                 //
                 // Assume all desired variables are in the mili file. 
                 //
                 if (MDVarIdx == -1)
                 {
                     continue;
                 }

                 mc_cleanse_st_variable(&sv);
            }

            mc_cleanse_subrec(&sr);
        }
    }
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
//
//      Note: zone and node label handling was kept as originally written
//            by Matt Larsen in 2017.
//
//  Programmer:  Alister Maguire
//  Creation:    Jan 16, 2019
//
//  Modifications
//
//    Matt Larsen, Wed May 17 01:46:16 PST 2017
//    Added OriginalZoneLabels and OriginalNodeLabels
//
// ****************************************************************************

vtkDataArray *
avtMiliFileFormat::GetVar(int ts, int dom, const char *varPath)
{
    //TODO: determine if we still need param arrays. 
    bool isParamArray = false;
    int meshId = ExtractMeshIdFromPath(varPath);

    vtkFloatArray *fltArray = NULL;

    if( strcmp("OriginalZoneLabels", varPath) == 0 )
    {
        int max_size = max_zone_label_lengths[dom] + 1;
        vtkElementLabelArray *names = 0;
        names = vtkElementLabelArray::New();
        names->SetNumberOfComponents(max_size);
        const int nels = zoneLabels[dom].size();
        names->SetNumberOfTuples(nels);
        char * ptr = (char *) names->GetVoidPointer(0);
        for(int i = 0; i < nels; ++i)
        {
            const int offset = i * max_size;
            const char * el_label = zoneLabels[dom][i].c_str();
            const int c_size = zoneLabels[dom][i].size();
            for(int j = 0; j < max_size; ++j)
            {   
                if(j < c_size)
                    ptr[offset + j] = el_label[j];
                else
                    ptr[offset + j] = '\0';
            }

        }

        //
        // Add the data so we can do reverse lookups
        //
        std::map<std::string,Label_mapping>::iterator it;
        for(it = zone_label_mappings[dom].begin(); 
            it != zone_label_mappings[dom].end(); ++it)
        {
             std::string name = it->first;
             Label_mapping label_map = it->second;
             names->AddName(name,
                            label_map.label_ranges_begin,
                            label_map.label_ranges_end,
                            label_map.el_ids_begin,
                            label_map.el_ids_end);
        }
        return names;

    }

    if( strcmp("OriginalNodeLabels", varPath) == 0 )
    {
        int max_size = max_node_label_lengths[dom] + 1;
        vtkElementLabelArray *names = 0;
        names = vtkElementLabelArray::New();
        names->SetNumberOfComponents(max_size);
        const int nels = nodeLabels[dom].size();
        names->SetNumberOfTuples(nels);
        char * ptr = (char *) names->GetVoidPointer(0);
        for(int i = 0; i < nels; ++i)
        {
            const int offset = i * max_size;
            const char * el_label = nodeLabels[dom][i].c_str();
            const int c_size = nodeLabels[dom][i].size();
            for(int j = 0; j < max_size; ++j)
            {   
                if(j < c_size)
                    ptr[offset + j] = el_label[j];
                else
                    ptr[offset + j] = '\0';
            }

        }
        //
        // Add the data so we can do reverse lookups
        //
        std::map<std::string,Label_mapping>::iterator it;
        for(it = node_label_mappings[dom].begin(); 
            it != node_label_mappings[dom].end(); ++it)
        {
             std::string name = it->first;
             Label_mapping label_map = it->second;
             names->AddName(name,
                            label_map.label_ranges_begin,
                            label_map.label_ranges_end,
                            label_map.el_ids_begin,
                            label_map.el_ids_end);
        }
        return names;
    }

    //
    // We need to first retrieve the meta data for our variable. 
    // NOTE: We need to get the MD by PATH instead of name because
    // variables can share names. 
    //
    MiliVariableMetaData *varMD = 
        miliMetaData[meshId]->GetVarMDByPath(varPath);
    SubrecInfo SRInfo = miliMetaData[meshId]->GetSubrecInfo(dom);

    if (varMD == NULL)
    {
        EXCEPTION1(InvalidVariableException, varPath);
    }

    avtCentering centering = varMD->GetCentering();
    vector<int> SRIds      = varMD->GetSubrecIds(dom);
    int nSRs               = SRIds.size();
    string vShortName      = varMD->GetShortName();

    //
    // Create a copy of our name to pass into mili. 
    //
    char charName[1024];
    sprintf(charName, vShortName.c_str());
    char *namePtr = (char *) charName;

    if (strncmp(vShortName.c_str(), "params/", 7) == 0)
        isParamArray = true; 

    //
    // Node centered variable. 
    //
    if (centering == AVT_NODECENT)
    {
        if (!isParamArray && nSRs != 1)
        {
            EXCEPTION1(InvalidVariableException, varPath);
        }
        int vType = varMD->GetNumType();

        //
        // Since data in param arrays is constant over all time,
        // we just cache it here in the plugin. Lets look in the
        // cache *before* we try to read it (again).
        //
        if (isParamArray)
        {
            fltArray = (vtkFloatArray*) cache->GetVTKObject(vShortName.c_str(),
                     avtVariableCache::SCALARS_NAME, -1, dom, "none");
        }

        if (fltArray == 0)
        {
            int nNodes = miliMetaData[meshId]->GetNumNodes(dom);
            fltArray   = vtkFloatArray::New();
            fltArray->SetNumberOfTuples(nNodes);
 
            float *fArrPtr = (float *) fltArray->GetVoidPointer(0);

            read_results(dbid[dom], ts+1, SRIds[0], 1,
                         &namePtr, vType, nNodes, fArrPtr);

            //
            // We explicitly cache param arrays at ts=-1
            //
            if (isParamArray)
            {
                cache->CacheVTKObject(vShortName.c_str(), 
                                  avtVariableCache::SCALARS_NAME,
                                  -1, dom, "none", fltArray);
            }
        }
        else
        {
            // The reference count will be decremented by the generic database,
            // because it will assume it owns it.
            fltArray->Register(NULL);
        }
    }
    //
    // Cell centered variable. 
    //
    else
    {
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

        for (int i = 0 ; i < SRIds.size(); i++)
        {
            int vType        = varMD->GetNumType();
            int start        = 0;
            int nTargetCells = 0;
            string className = varMD->GetClassShortName();
            int SRId         = SRIds[i];

            start = miliMetaData[meshId]->
                GetClassMDByShortName(className.c_str())->
                GetConnectivityOffset(dom);                                   

            nTargetCells = SRInfo.nElements[SRId];
            
            // Simple read in: one block 
            if (SRInfo.nDataBlocks[SRId] == 1)
            {
                // Adjust start
                start += (SRInfo.dataBlockRanges[SRId][0] - 1);
            
                read_results(dbid[dom], ts+1, SRId,
                             1, &namePtr, vType, nTargetCells, 
                             dataBuffer + start);
            }
            else
            {
                int nBlocks         = SRInfo.nDataBlocks[SRId];
                vector<int> *blocks = &SRInfo.dataBlockRanges[SRId];

                int totalBlocksSize = 0;
                for (int b = 0; b < nBlocks; ++b)
                {
                    int start = (*blocks)[b * 2 + 1]; 
                    int stop  = (*blocks)[b * 2];
                    totalBlocksSize += start - stop + 1;
                }

                float MBBuffer[totalBlocksSize];

                read_results(dbid[dom], ts + 1, SRId,
                             1, &namePtr, vType, totalBlocksSize, MBBuffer);

                //
                // Fill up the blocks into the array.
                //
                float *mbPtr = MBBuffer;
                for (int b = 0; b < nBlocks; ++b)
                {
                    for (int c = (*blocks)[b * 2] - 1; 
                         c <= (*blocks)[b * 2 + 1] - 1; ++c)
                    {
                        dataBuffer[c + start] = *(mbPtr++);
                    }
                }
            }
        }

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
//    Hank Childs, Mon Sep 22 07:36:48 PDT 2003
//    Add support for reading in tensors.
//
// ****************************************************************************

vtkDataArray *
avtMiliFileFormat::GetVectorVar(int ts, int dom, const char *varPath)
{
    int meshId = ExtractMeshIdFromPath(varPath);
    
    //
    // NOTE: We need to get the MD by PATH instead of name because
    // variables can share names. 
    //
    MiliVariableMetaData *varMD = miliMetaData[meshId]->
        GetVarMDByPath(varPath);
    SubrecInfo SRInfo = miliMetaData[meshId]->GetSubrecInfo(dom);

    if (varMD == NULL)
    {
        EXCEPTION1(InvalidVariableException, varPath);
    }

    avtCentering centering = varMD->GetCentering();
    vector<int> SRIds      = varMD->GetSubrecIds(dom);
    string vShortName      = varMD->GetShortName();

    //
    // Component dimensions will only be > 1 if we have
    // an element set. 
    //
    int vecSize   = varMD->GetVectorSize();
    int compDims  = varMD->GetComponentDims();
    int dataSize  = vecSize * compDims;

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

    //
    // Node centered variable. 
    //
    if (centering == AVT_NODECENT)
    {
        if (SRIds.size() != 1)
        {
            EXCEPTION1(InvalidVariableException, varPath);
        }

        int vType  = varMD->GetNumType();
        int nNodes = miliMetaData[meshId]->GetNumNodes(dom);

        fltArray->SetNumberOfTuples(nNodes);
        float *fArrPtr = (float *) fltArray->GetVoidPointer(0);
        
        read_results(dbid[dom], ts+1, SRIds[0], 1,
                     &namePtr, vType, nNodes * dataSize, fArrPtr);
    }
    //
    // Cell centered variable. 
    //
    else
    {
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
        // Iterate through the variable's subrecords. 
        //
        for (int i = 0 ; i < SRIds.size(); i++)
        {
            int vType        = varMD->GetNumType();
            int start        = 0;
            int nTargetCells = 0;
            string className = varMD->GetClassShortName();
            int SRId         = SRIds[i];

            start = miliMetaData[meshId]->
                GetClassMDByShortName(className.c_str())->
                GetConnectivityOffset(dom);                                   

            nTargetCells = SRInfo.nElements[SRId];

            //
            // This handles the case where all of our data is in a single
            // contiguous chunk of cells. 
            // 
            if (SRInfo.nDataBlocks[SRId] == 1)
            {
                start         += (SRInfo.dataBlockRanges[SRId][0] - 1);
                int resultSize = nTargetCells * dataSize;
                float *dbPtr   = dataBuffer;

                read_results(dbid[dom], ts+1, SRId,
                             1, &namePtr, vType, resultSize,
                             dbPtr + (start * dataSize));

            }
            else
            {
                //TODO: need data that tests multiple blocks.
                int nBlocks         = SRInfo.nDataBlocks[SRId];
                vector<int> *blocks = &SRInfo.dataBlockRanges[SRId];

                int totalBlocksSize = 0;
                for (int b = 0; b < nBlocks; ++b)
                {
                    int start = (*blocks)[b * 2 + 1]; 
                    int stop  = (*blocks)[b * 2];
                    totalBlocksSize += start - stop + 1;
                }

                float MBBuffer[totalBlocksSize * dataSize];
                int resultSize  = totalBlocksSize * dataSize;

                read_results(dbid[dom], ts + 1, SRId,
                             1, &namePtr, vType, resultSize, MBBuffer);

                float *MBPtr = MBBuffer;

                //
                // Fill up the blocks into the array.
                //
                for (int b = 0; b < nBlocks; ++b)
                {
                    for (int c = (*blocks)[b * 2] - 1; 
                         c <= (*blocks)[b * 2 + 1] - 1; ++c)
                    {
                        for (int k = 0; k < dataSize; ++k)
                        {
                            int idx = (dataSize * (c + start)) + k;
                            dataBuffer[idx] = *(MBPtr++);
                        }
                    }
                }
            } 
        }

        //TODO: Need to test the multiple block case. 
        //
        // If this is an element set, we need to extract the 
        // integration points, and copy then over to our 
        // VTK data array. 
        //
        if (isES)
        {
            //
            // Let's take the mid integration point. 
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

    return fltArray;
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::GetCycles
//
//  Purpose:
//      Returns the actual cycle numbers for each time step.
//
//      Note: unchanged from original filter.
//
//  Arguments:
//   cycles      the output vector of cycle numbers 
//
//  Programmer:  Hank Childs
//  Creation:    April 11, 2003
//
//  Modifications:
//    Eric Brugger, Mon Sep 21 11:01:46 PDT 2015
//    The reader now returns the cycles and times in the meta data and 
//    marks them as accurate so that they are used where needed.
//
// ****************************************************************************

void
avtMiliFileFormat::GetCycles(vector<int> &out_cycles)
{
    out_cycles = cycles;
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::GetTimes
//
//  Purpose:
//      Returns the actual times for each time step.
//
//      Note: unchanged from original filter.
//
//  Arguments:
//   out_times   the output vector of times 
//
//  Programmer:  Hank Childs
//  Creation:    October 20, 2003
//
// ****************************************************************************

void
avtMiliFileFormat::GetTimes(vector<double> &out_times)
{
    out_times = times;
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::GetNTimesteps
//
//  Purpose:
//      Returns the number of timesteps
//
//      Note: unchanged from original filter.
//
//  Programmer:  Hank Childs
//  Creation:    April 11, 2003
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
//      Note: unchanged from original filter.
//
//  Programmer:  Alister Maguire
//  Creation:    January 29, 2019
//
// ****************************************************************************

bool
avtMiliFileFormat::CanCacheVariable(const char *varname)
{
    if (strncmp(varname, "params/", 7) == 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//    Returns meta-data about the database.
//
//    Note: label handling has been kept as originally written by Matt Larsen
//          in 2017.
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
//    Matt Larsen, Fri May 26 07:45:12 PDT 2017
//    Adding zone and node labels to meta data
//
// ****************************************************************************

void
avtMiliFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md,
    int timeState)
{
    for (int meshId = 0; meshId < nMeshes; ++meshId)
    {
        char meshName[32];
        char matName[32];
        sprintf(meshName, "mesh%d", meshId + 1);
        sprintf(matName, "materials%d", meshId + 1);
        avtMeshMetaData *mesh      = new avtMeshMetaData;
        mesh->name                 = meshName;
        mesh->meshType             = AVT_UNSTRUCTURED_MESH;
        mesh->numBlocks            = nDomains;
        mesh->blockOrigin          = 0;
        mesh->cellOrigin           = 1; // mili writers are Fortran
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
        vector<string> matColors;
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

        //TODO: look into this. 
        // Visit is intercepting these labels and displaying something
        // else, so current work around is just to hide them
        bool hideFromGui = true;
        AddLabelVarToMetaData(md, 
                              "OriginalNodeLabels", 
                              meshName, 
                              AVT_NODECENT, 
                              dims, 
                              hideFromGui);

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

            char meshName[32];
            sprintf(meshName, "mesh%d", varMD->GetMeshAssociation() + 1);

            int cellType           = varMD->GetAvtCellType();
            avtCentering centering = varMD->GetCentering();
            int vecSize            = varMD->GetVectorSize();

            //
            // If the variable is a scalar, the vector size is designated 1, 
            // but there will be no vector components. 
            //
            vector<string> vComps  = varMD->GetVectorComponents();
            int nComps             = vComps.size();

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

            string vPath = varMD->GetPath();
           
            switch (cellTypeCast)
            {
                case AVT_SCALAR_VAR:
                {
                    AddScalarVarToMetaData(md, vPath, meshName, centering);
                    break;
                }
                case AVT_VECTOR_VAR:
                {
                    AddVectorVarToMetaData(md, vPath, meshName, 
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
                        sprintf(name, "%s/%s", vPath.c_str(), 
                            vComps[j].c_str());
                        Expression expr = ScalarExpressionFromVec(vPath.c_str(),
                                              name, j);
                        md->AddExpression(&expr);
                    }
                    break;
                }
                case AVT_SYMMETRIC_TENSOR_VAR:
                {
                    //
                    // When we come across a vector of length 6, we change it 
		    // to a normal vector of length 9 and render it as a 
		    // symmetric tensor. 
                    //
                    AddSymmetricTensorVarToMetaData(md, vPath, 
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
                        char singleDef[256];
                        sprintf(singleDef, "<%s>[%d][%d]", vPath.c_str(), j, j);
                        string singleName = vPath + "/" + varMD->GetVectorComponent(j);
                        singleDim.SetName(singleName);
                        singleDim.SetDefinition(singleDef);
                        singleDim.SetType(Expression::ScalarMeshVar);
                        md->AddExpression(&singleDim);

                        //
                        // Next, get the "multi-dim" values: xy, yz, zx
                        //
                        int compIdx = j + 3;
                        Expression multDim;
                        char multDef[256];
                        sprintf(multDef, "<%s>[%d][%d]", vPath.c_str(), j, multDimIdxs[j]);
                        string multName = vPath + "/" + varMD->GetVectorComponent(compIdx);
                        multDim.SetName(multName);
                        multDim.SetDefinition(multDef);
                        multDim.SetType(Expression::ScalarMeshVar);
                        md->AddExpression(&multDim);
                    }

                    break;
                }
                case AVT_TENSOR_VAR:
                {
                    AddTensorVarToMetaData(md, vPath, 
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
                        char singleDef[256];
                        sprintf(singleDef, "<%s>[%d][%d]", vPath.c_str(), j, j);
                        string singleName = vPath + "/" + varMD->GetVectorComponent(j);
                        singleDim.SetName(singleName);
                        singleDim.SetDefinition(singleDef);
                        singleDim.SetType(Expression::ScalarMeshVar);
                        md->AddExpression(&singleDim);

                        //
                        // Next, get the "multi-dim" values: xy, yz, zx
                        //
                        int compIdx = j + 3;
                        Expression multDim;
                        char multDef[256];
                        sprintf(multDef, "<%s>[%d][%d]", vPath.c_str(), j, multDimIdxs[j]);
                        string multName = vPath + "/" + varMD->GetVectorComponent(compIdx);
                        multDim.SetName(multName);
                        multDim.SetDefinition(multDef);
                        multDim.SetType(Expression::ScalarMeshVar);
                        md->AddExpression(&multDim);
                    }
                    break;
                }
                case AVT_ARRAY_VAR:
                {
                    //
                    // For array vars, we just want to display the 
                    // individual components. 
                    //
                    vector<string> compNames;

                    for (int j = 0; j < nComps; ++j)
                    {
                        compNames.push_back(varMD->GetVectorComponent(j));
                        char name[1024];
                        sprintf(name, "%s/%s", vPath.c_str(), 
                            varMD->GetVectorComponent(j).c_str());
                        Expression expr = ScalarExpressionFromVec(vPath.c_str(),
                                              name, j);
                        md->AddExpression(&expr);
                    }

                    AddArrayVarToMetaData(md, vPath, compNames, meshName, 
                        centering); 
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

}


// ****************************************************************************
//  Method: avtMiliFileFormat::GetAuxiliaryData
//
//  Purpose:
//      Gets the auxiliary data specified.
//
//      Note: unchanged from original filter. 
//
//  Arguments:
//      var        The variable of interest.
//      ts         The timestep of interest.
//      dom        The domain of interest.
//      type       The type of auxiliary data.
//      <unnamed>  The arguments for that type -- not used.
//      df         Destructor function.
//
//  Returns:    The auxiliary data.
//
//  Programmer: Akira Haddox
//  Creation:   May 23, 2003
//
//  Modifications
//    Akira Haddox, Fri May 23 08:13:09 PDT 2003
//    Added in support for multiple meshes. Changed for MTMD.
//
//    Matt Larsen, Fri May 4 08:13:09 PDT 2017
//    Added in support for Zone and Node labels.
//
// ****************************************************************************
 
void *
avtMiliFileFormat::GetAuxiliaryData(const char *var, 
                                    int ts, 
                                    int dom, 
                                    const char *type, 
                                    void *,
                                    DestructorFunction &df) 
{
    if (strcmp(type, AUXILIARY_DATA_MATERIAL) && 
        strcmp(type, "AUXILIARY_DATA_IDENTIFIERS"))
    {
        return NULL;
    }

    if (!meshRead[dom])
    {
        ReadMesh(dom);
    }
    
    if(!strcmp(type, "AUXILIARY_DATA_IDENTIFIERS"))
    {
        std::string varName(var);
        if(varName != "OriginalZoneLabels" && 
           varName != "OriginalNodeLabels" )
        {
            EXCEPTION1(InvalidVariableException, var);
        }

        df = vtkElementLabelArray::Destruct;
        return (void *)this->GetVar(ts, dom, var);
    }

    if (strcmp(type, AUXILIARY_DATA_MATERIAL))
    {
        return NULL;
    }

    //
    // The valid variables are meshX, where X is an int > 0.
    // We need to verify the name, and get the meshId.
    //
    if (strstr(var, "materials") != var)
    {
        EXCEPTION1(InvalidVariableException, var);
    }
 
    //
    // Do a checked conversion to integer.
    //
    char *check;
    int meshId = (int) strtol(var + strlen("materials"), &check, 10);
    if (check != NULL && check[0] != '\0')
    {
        EXCEPTION1(InvalidVariableException, var)
    }
    --meshId;
 
    avtMaterial *myCopy = materials[dom][meshId];
    avtMaterial *mat = new avtMaterial(myCopy->GetNMaterials(),
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
                                       string shortName,
                                       string cShortName,
                                       string cLongName,
                                       int meshId,
                                       bool isMatVar,
                                       bool isGlobal)
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
        vector<string> comps;

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
        const Value &val = cItr->value;
        string sName     = cItr->name.GetString();

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

            if (val.HasMember("ElementCount"))
            {
                elCount = val["ElementCount"].GetInt();
            }
            else
            {
                char msg[256];
                sprintf(msg, "Mili file classes must contain ElementCount");
                EXCEPTION1(ImproperUseException, msg);
            }
               
            if (val.HasMember("SuperClass"))
            {
                scID= val["SuperClass"].GetInt();
            }
            else
            {
                
                EXCEPTION2(UnexpectedValueException, "Superclass ID",
                    "");
            }

            MiliClassMetaData *miliClass = 
                new MiliClassMetaData(sName,
                                      lName,
                                      scID,
                                      elCount,
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
                        const Value &var = jVars[cVars[i]];
                        string varName   = cVars[i].GetString();
 
                        MiliVariableMetaData *varMD = ExtractJsonVariable(var, 
                            varName, sName, lName, meshId, isMatVar, isGlobal);

                        if (varMD != NULL)
                        {
                            miliClass->AddMiliVariable(varName);
                            miliMetaData[meshId]->AddVarMD(varIdx, varMD);
                            varIdx++;
                        }

                    } // end jVars.HasMember

                } // end for i in length of cVars
               
                //
                // Cache the mili class md. 
                //
                miliMetaData[meshId]->AddClassMD(classIdx, miliClass);
                classIdx++;

            } // end val.HasMember("variables")

        }// end if val.IsObject()

    } // end for cItr in jClasses
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
//  Author:
//
//  Modifications:
//
// ****************************************************************************
void
avtMiliFileFormat::LoadMiliInfoJson(const char *fpath)
{
    ifstream jfile;
    jfile.open(fpath);

    IStreamWrapper isw(jfile);
    Document jDoc;
    jDoc.ParseStream(isw);

    if (jDoc.HasMember("Domains"))
    {
        nDomains = jDoc["Domains"].GetInt(); 
    }

    if (jDoc.HasMember("Number_of_Meshes"))
    {
        nMeshes = jDoc["Number_of_Meshes"].GetInt();
    }

    if (jDoc.HasMember("Dimensions"))
    {
        dims = jDoc["Dimensions"].GetInt();
    }

    if (jDoc.HasMember("States"))
    {
        const Value &jStates = jDoc["States"];
        if (jStates.HasMember("count"))
        {
            nTimesteps = jStates["count"].GetInt();
        }
    }
    
    miliMetaData = new avtMiliMetaData *[nMeshes];

    for (int i = 0; i < nMeshes; ++i) 
    {
        miliMetaData[i] = NULL;
    }

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
                string name      = jItr->name.GetString();
                const Value &mat = jItr->value;

                string matName = "";
                std::stringstream colorSS;
                colorSS << "#";
                  
                if (mat.IsObject())
                {
                    if (mat.HasMember("name"))
                    {
                        matName = mat["name"].GetString();
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
                                                 string(colorSS.str()));

                    miliMetaData[meshId]->AddMaterialMD(matCount++, miliMaterial);
                }// end mat.IsObject

            }// end for jItr in jMats
        }

        if (jDoc.HasMember("Variables"))
        {
            miliMetaData[meshId]->SetNumVariables(CountJsonClassVariables(jDoc));
            ExtractJsonClasses(jDoc, meshId);
        }

        if (jDoc.HasMember("States"))
        {
            const Value &jStates = jDoc["States"]; 

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

    zoneLabels.resize(nDomains);
    nodeLabels.resize(nDomains);
    zone_label_mappings.resize(nDomains);
    node_label_mappings.resize(nDomains);
    max_zone_label_lengths.resize(nDomains);
    max_node_label_lengths.resize(nDomains);

    materials.resize(nDomains);
    for (int dom = 0; dom < nDomains; ++dom)
    {
        datasets[dom].resize(nMeshes, NULL);
        materials[dom].resize(nMeshes, NULL);
    }

    for(int i = 0; i < nDomains; ++i)
    {
        max_zone_label_lengths[i] = 0;
        max_node_label_lengths[i] = 0;
    }

    jfile.close();
}



// ***************************************************************************
//  Function: PopulateZoneLabels
//
//  Purpose:
//      Populate data structures to implement a reverse mapping between
//      a zone label and a zone id
//
//      Note: unchanged from original filter. 
//
//  Arguments: 
//             famId - mili familiy id
//             meshId - id of the mesh associtated with the labels 
//             shortName - the short name of the mili class (e.g., "brick") 
//             dom - id of the domain 
//             num_zones - running count of the total number of zones in
//                         the mesh. Needed for the reverse mapping
//             nElemsInGroup - number of elements in this group
//           
//  Author: Matt Larsen May 10 2017
//
//  Modifications:
//
// ****************************************************************************
void 
avtMiliFileFormat::PopulateZoneLabels(const int famId, const int meshId, 
                                      char *shortName, const int dom, 
                                      const int nElemsInGroup)
{

    int num_zones   = 0;
    int numBlocks   = 0; 
    int *blockRange = NULL;
    int elem_list[nElemsInGroup];
    int label_ids[nElemsInGroup];
    
    //
    // Check for labels
    //
    int nExpectedLabels = nElemsInGroup;
    int rval = mc_load_conn_labels(dbid[dom], meshId, shortName, 
                                   nExpectedLabels, &numBlocks, 
                                   &blockRange, elem_list, label_ids);

    if (rval != OK)
    {
        debug1 << "mc_load_conn_labels failed at " << shortName << "!\n";
        num_zones    = 0;
        numBlocks   = 0; 
        blockRange  = NULL;
    }
    
    if (numBlocks == 0)
    {
        debug4 << "mili block contains no labels\n";
        //Create default labels
        for(int elem_num = 1; elem_num <= nElemsInGroup; ++ elem_num)
        {
            // create the label strings for each cell
            std::stringstream sstream;
            sstream<<"";
            zoneLabels[dom].push_back(sstream.str());
            max_zone_label_lengths[dom] = 
                std::max(int(sstream.str().size()), max_zone_label_lengths[dom]);
        }
        num_zones += nElemsInGroup;
    }
    else
    {
        debug4 << "Mili labels found. There are " << numBlocks
               << " blocks in class " << shortName << " in dom " 
               << dom << "\n";
        for(int el = 0; el < nElemsInGroup; ++el)
        {
            std::stringstream sstream;
            sstream<<shortName;
            sstream<<" "<<label_ids[el];
            zoneLabels[dom].push_back(sstream.str());
            max_zone_label_lengths[dom] = 
                std::max(int(sstream.str().size()), max_zone_label_lengths[dom]);
        }
        Label_mapping label_map;
        for(int block = 0; block < numBlocks; ++block)
        {
          int rangeSize = blockRange[block * 2 + 1] - blockRange[block * 2] + 1;
          label_map.label_ranges_begin.push_back(blockRange[block * 2]);
          label_map.label_ranges_end.push_back(blockRange[block * 2 + 1]);
          label_map.el_ids_begin.push_back(num_zones);
          label_map.el_ids_end.push_back(num_zones - 1 + rangeSize);

          num_zones += rangeSize;
        }

        zone_label_mappings[dom][std::string(shortName)] = label_map;

    }
}

// ***************************************************************************
//  Function: PopulateNodeLabels
//
//  Purpose:
//      Populate data structures to implement a reverse mapping between
//      a zone label and a zone id
//
//      Note: unchanged from original filter. 
//
//  Arguments: 
//             famId - mili familiy id
//             meshId - id of the mesh associtated with the labels 
//             shortNamshortName the short name of the mili class (e.g., "brick") 
//             dom - id of the domain 
//             num_zones - running count of the total number of zones in
//                         the mesh. Needed for the reverse mapping
//           
//  Author: Matt Larsen May 10 2017
//
//  Modifications:
//
// ****************************************************************************
// FIXME: famId isn't used??
void 
avtMiliFileFormat::PopulateNodeLabels(const int famId, const int meshId, 
                                      char *shortName, const int dom)
{

    int nLabeledNodes = 0;
    int nNodes        = miliMetaData[meshId]->GetNumNodes(dom);
    max_node_label_lengths[dom] = 0;

    int numBlocks   = 0; 
    int *blockRange = NULL;
    int elemList[nNodes];
    int labelIds[nNodes];

    int rval = mc_load_node_labels(dbid[dom], meshId, shortName, 
                                   &numBlocks, &blockRange, labelIds);

    if (rval != OK)
    {
        debug1 << "mc_load_conn_labels failed at " << shortName << "!\n";
        numBlocks   = 0; 
        blockRange  = NULL;
    }

    if (numBlocks == 0)
    {
        debug4 << "Mili block does not contain node labels\n";
        
        //Create default labels
        for(int elemNum = 1; elemNum <= nNodes; ++elemNum)
        {
            // create the label strings for each cell
            std::stringstream sstream;
            sstream<<"";
            zoneLabels[dom].push_back(sstream.str());
            max_node_label_lengths[dom] = 
                std::max(int(sstream.str().size()), 
                max_node_label_lengths[dom]);
        }
        nLabeledNodes += nNodes;
    }
    else
    {
        debug4 << "Mili labels node found. There are " << numBlocks 
               << " blocks in class " << shortName << " in dom " 
               << dom << "\n";
        for(int el = 0; el < nNodes; ++el)
        {
            std::stringstream sstream;
            sstream << shortName;
            sstream << " " << labelIds[el];
            nodeLabels[dom].push_back(sstream.str());
            max_node_label_lengths[dom] = 
                std::max(int(sstream.str().size()), 
                max_node_label_lengths[dom]);
        }
        Label_mapping label_map;
        for(int block = 0; block < numBlocks; ++block)
        {
            int rangeSize = (blockRange[block * 2 + 1] - 
                blockRange[block * 2] + 1);
            label_map.label_ranges_begin.push_back(blockRange[block * 2]);
            label_map.label_ranges_end.push_back(blockRange[block * 2 + 1]);
            label_map.el_ids_begin.push_back(nLabeledNodes);
            label_map.el_ids_end.push_back(nLabeledNodes - 1 + rangeSize);

            nLabeledNodes += rangeSize;
        }

        node_label_mappings[dom][std::string(shortName)] = label_map;
    }
}


// ***************************************************************************
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
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
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
Expression
avtMiliFileFormat::ScalarExpressionFromVec(const char *vecPath, 
                                           const char *varPath, 
                                           int dim)
{
    char def[256];
    sprintf(def, "<%s>[%d]", vecPath, dim);
    Expression::ExprType eType = Expression::ScalarMeshVar;
    return CreateGenericExpression(varPath, def, eType);
}



// ***************************************************************************
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
Expression
avtMiliFileFormat::ScalarExpressionFromElementSet(const char *esPath, 
                                                  const char *varPath, 
                                                  int *dims)
{
    char def[256];
    sprintf(def, "<%s>[%d][%d]", esPath, dims[0], dims[1]);
    Expression::ExprType eType = Expression::ScalarMeshVar;
    return CreateGenericExpression(varPath, def, eType);
}

