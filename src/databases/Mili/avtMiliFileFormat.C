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
    if (warn_map.find(key) == warn_map.end())
        warn_map[key] = 1;
    else
        warn_map[key]++;

    if (warn_map[key] <= 5)
    {
        if (!avtCallback::IssueWarning(msg))
            cerr << msg << endl;
    }

    if (warn_map[key] == 5)
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
//    fname      the file name of one of the Mili files.
//
//  Programmer:  Alister Maguire
//  Creation:    Jan 15, 2019
//
//  Modifications
//     
// ****************************************************************************

avtMiliFileFormat::avtMiliFileFormat(const char *fname)
    : avtMTMDFileFormat(fname)
{
    LoadMiliInfoJson(fname);

    string fnameStr(fname);

    //TODO: make sure this works for windows paths as well. 
    size_t fNPos   = fnameStr.find_last_of("\\/");
    string pthTmp  = fnameStr.substr(0, fNPos + 1);
    string root    = fnameStr.substr(fNPos + 1);

    //
    // Set the family path.
    //
    size_t pSize = pthTmp.size();
    fampath      = new char[pSize + 1];
    strcpy(fampath, pthTmp.c_str());
    fampath[pSize + 1] = '\0';

    //
    // Extract and set the family root. 
    //
    string lastSub = "";
    size_t lastPos = root.find_last_of(".");
    lastSub        = root.substr(lastPos + 1);
    string mExt    = "mili";

    if (lastSub == mExt)
        root = root.substr(0, lastPos);

    size_t rSize = root.size();
    famroot      = new char[rSize + 1];
    strcpy(famroot, root.c_str());
    famroot[rSize + 1] = '\0';
}


// ****************************************************************************
//  Destructor:  avtMiliFileFormat::~avtMiliFileFormat
//
//  Programmer:  Hank Childs
//  Creation:    April 11, 2003
//
//  Modifications:
//    Akira Haddox, Fri May 23 08:51:11 PDT 2003
//    Added in support for multiple meshes. Changed to MTMD.
//
//    Akira Haddox, Wed Jul 23 12:57:14 PDT 2003
//    Moved allocation of cached information to FreeUpResources.
//
//    Hank Childs, Tue Jul 27 10:40:44 PDT 2004
//    Sucked in code from FreeUpResources.
//
//    Mark C. Miller, Mon Jul 18 13:41:13 PDT 2005
//    Free structures having to do with free nodes mesh
//
//    Mark C. Miller, Wed Mar  8 08:40:55 PST 2006
//    Added code to cleanse Mili subrecords
// ****************************************************************************

avtMiliFileFormat::~avtMiliFileFormat()
{
    //
    // Close mili databases, and delete non-essential allocated memory.
    // Keep the original sizes of vectors though.
    //
    int i, j;
    for (i = 0; i < nDomains; ++i)
        if (dbid[i] != -1)
        {
            mc_close(dbid[i]);
            dbid[i] = -1;
        }
    for (i = 0; i < datasets.size(); ++i)
        for (j = 0; j < datasets[i].size(); ++j)
            if (datasets[i][j] != NULL)
            {
                datasets[i][j]->Delete();
                datasets[i][j] = NULL;
            }
    for (i = 0; i < materials.size(); ++i)
        for (j = 0; j < materials[i].size(); ++j)
            if (materials[i][j])
            {
                delete materials[i][j];
                materials[i][j] = NULL;
            }

    datasets.clear();
    materials.clear();

    //
    // Reset flags to indicate the meshes needs to be read in again.
    //
    for (i = 0; i < nDomains; ++i)
    {
        readMesh[i] = false;
    }

    //
    // Delete the mili meta data. 
    //
    for (int i = 0; i < nMeshes; ++i)
    {
        if (miliMetaData[i] != NULL)
            delete miliMetaData[i];
    }
    delete [] miliMetaData;

    delete [] famroot;
    if (fampath)
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
                buff[i] = (float)(pflt[i]);
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
//  Programmer:  Hank Childs
//  Creation:    April 11, 2003
//
//  Modifications:
//    Akira Haddox, Fri May 23 08:51:11 PDT 2003
//    Added in support for multiple meshes. Changed to MTMD. 
//
//    Akira Haddox, Tue Jul 22 08:09:28 PDT 2003
//    Fixed the try block. Properly dealt with cell variable blocks.
//
//    Akira Haddox, Mon Aug 18 14:33:15 PDT 2003
//    Commented out previous sand-based ghosts.
//
//    Hank Childs, Sat Jun 26 11:24:47 PDT 2004
//    Check for bad files where number of timesteps is incorrectly reported.
//
//    Hank Childs, Fri Aug 27 17:12:50 PDT 2004
//    Rename ghost data array.
//
//    Mark C. Miller, Mon Jul 18 13:41:13 PDT 2005
//    Added logic to read the "free nodes" mesh, too. Removed huge block of
//    unused #ifdef'd code having to do with ghost zones.
//
//    Mark C. Miller, Tue Jan  3 17:55:22 PST 2006
//    Added code to deal with case where nodal positions are time invariant.
//    They are not stored as "results" but instead part of the mesh read
//    in the ReadMesh() call.
//
//    Mark C. Miller, Wed Nov 15 01:46:16 PST 2006
//    Added a "no_free_nodes" mesh by ghost labeling sanded nodes. Added
//    the logic to label sanded nodes here.
//
//    Mark C. Miller, Tue Nov 21 10:16:42 PST 2006
//    Fixed leak of sand_arr. Made it request sand_arr only if the
//    no_free_nodes mesh was requested
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
    if (strstr(mesh, "mesh") != mesh)
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

    if (!readMesh[dom])
    {
        ReadMesh(dom);
    }
    if (!validateVars[dom])
    {
        ValidateVariables(dom, meshId);
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
    const char *npName = "nodpos";
    char *npNamePtr    = (char *) npName;
    int nodpos         = -2;
    nodpos             = miliMetaData[meshId]->GetMiliVariableMDIdx(npName);

    int subrec = -1;
    int vType  = M_FLOAT;
    vType  = miliMetaData[meshId]->
        GetMiliVariableMD(npName)->GetNumType();
    subrec = miliMetaData[meshId]->
        GetMiliVariableMD(npName)->GetSubrecordIds(dom)[0];

    int nNodes  = miliMetaData[meshId]->GetNumNodes(dom);
    int nPts    = dims * nNodes;
    float *fPts  = NULL;

    if (nodpos != -1)
    {
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
            fPts = new float[nPts];

            read_results(dbid[dom], ts+1, subrec, 1, 
                &npNamePtr, vType, nPts, fPts);

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
//  Method: avtMiliFileFormat::DecodeMultiMeshVarname
//
//  Purpose:
//      Takes in a variable name used to populate, and returns the
//      original variable name, and associated mesh id.
//
//  Programmer: Akira Haddox
//  Creation:   June 26, 2003
//
// ****************************************************************************

void
avtMiliFileFormat::DecodeMultiMeshVarname(const string &varname,
                                          string &decoded, int &meshId)
{
    //TODO: keep or rework?
    decoded = varname;
    meshId  = 0;

    char *ptr = &(decoded[0]);
    while(*ptr != '\0')
    {
        if(*ptr == '(')
            break;    
        ++ptr;
    }
    
    if (*ptr == '\0')
        return;

    char *check;
    meshId = (int) strtol(ptr + strlen("(mesh"), &check, 10);
    --meshId;

    *ptr = '\0';
}


// ****************************************************************************
//  Method: avtMiliFileFormat::DecodeMultiLevelVarname
//
//  Purpose:
//      Takes in a variable name used to populate, and returns the
//      original variable name less the directory path.
//
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019 
//
// ****************************************************************************

void
avtMiliFileFormat::DecodeMultiLevelVarname(const string &inname, string &decoded)
{
    size_t varPos  = inname.find_last_of("/");
    decoded        = inname.substr(varPos + 1);
}


// ****************************************************************************
//  Method: avtMiliFileFormat::OpenDB
//
//  Purpose:
//      Open up a family database for a given domain.
//
//  Programmer: Akira Haddox
//  Creation:   June 26, 2003
//
//  Modifications:
//
//    Akira Haddox, Tue Jul 22 15:34:40 PDT 2003
//    Added in setting of times.
//
//    Hank Childs, Mon Oct 20 10:03:58 PDT 2003
//    Made a new data member for storing times.  Populated that here.
//
//    Hank Childs, Wed Aug 18 16:17:52 PDT 2004
//    Add some special handling for single domain families.
//
//    Eric Brugger, Mon Sep 21 11:01:46 PDT 2015
//    The reader now returns the cycles and times in the meta data and 
//    marks them as accurate so that they are used where needed.
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
            rval = mc_open( famroot, fampath, rFlag, &(dbid[dom]) );

            if ( rval != OK )
                EXCEPTION1(InvalidFilesException, famroot);
        }
        else
        {
            int i; char famname[128];
            for (i = 0; i < 4; i++)
            {
                sprintf(famname, root_fmtstrs[i], famroot, dom);
                debug3 << "Attempting mc_open on root=\"" << famname 
                    << "\", path=\"" << fampath << "\"." << endl;
                rval = mc_open( famname, fampath, rFlag, &(dbid[dom]) );
                if (rval == OK) 
                    break;
            }
            if ( rval != OK )
                EXCEPTION1(InvalidFilesException, famname);
        }
    }
}


// ****************************************************************************
//  Method: avtMiliFileFormat::ReadMesh
//
//
//  Purpose:
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

        PopulateNodeLabels(dbid[dom], meshId, nodeSName, dom);

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

                //
                // TODO: do we still need the connectivity offset?
                //       we could also get it from SR.
                //
                miliMetaData[meshId]->GetMiliClassMD(shortName)->
                    SetConnectivityOffset(dom, offset);
                miliMetaData[meshId]->GetMiliClassMD(shortName)->
                    SetNumElements(dom, nCells);
                offset += nCells;
            }
        }

        miliMetaData[meshId]->SetNumCells(dom, nDomCells);

        // FIXME: should we set the num elements for classes not set
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
        int *matList  = new int[nDomCells];
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
                //FIXME: does mili handle deletion of these?
                int *conn = new int[nCells * connCount];
                int *mats = new int[nCells];
                int *part = new int[nCells];
                char shortName[classNames[cpcIdx].size() + 1];
                strcpy(shortName, classNames[cpcIdx].c_str());
                cpcIdx++;

                int rval = mc_load_conns(dbid[dom], meshId, 
                    shortName, conn, mats, part);

                delete [] part;

                if (rval != OK)
                {
                    delete [] conn;
                    delete [] mats;
                    EXCEPTION1(InvalidVariableException, shortName);
                }
    
                for (int k = 0 ; k < nCells ; ++k)
                {
                    matList[cellIdx] = mats[k];
                    cellIdx++;

                    vtkIdType verts[100];

                    for(int cc = 0; cc < connCount; ++cc)
                    {
                        verts[cc] = (vtkIdType)conn[cc];
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
                            if (conn[2] == conn[3] && conn[4] == conn[5] &&
                                conn[5] == conn[6] && conn[6] == conn[7])
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
                    conn += connCount;
                }

                PopulateZoneLabels(dbid[dom], meshId, shortName, dom, 
                                   nCells);
            }
        }
       
        //
        // Create our avtMaterial. 
        //
        int numMats           = miliMetaData[meshId]->GetNumMaterials();
        stringVector matNames = miliMetaData[meshId]->GetMaterialNames();
        avtMaterial *avtMat   = new avtMaterial(numMats, matNames,
                                                nDomCells, matList, 0, 
                                                NULL, NULL, 
                                                NULL, NULL);

        materials[dom][meshId] = avtMat;
        delete [] matList;

        //
        // Hook up points to mesh if we have 'em
        //
        if (vtkPts)
        {
            datasets[dom][meshId]->SetPoints(vtkPts);
            vtkPts->Delete();
        }

    }// end mesh reading loop

    readMesh[dom] = true;
}


// ****************************************************************************
//  Method: avtMiliFileFormat::ValidateVariables
//
//  Purpose:
//      Read in the information to determine which vars are valid for
//      which subrecords. Also read in subrecord info.
//
//  Programmer: Hank Childs (adapted by Akira Haddox)
//  Creation:   June 25, 2003
//
//  Modifications:
//    Akira Haddox, Wed Jul 23 09:47:30 PDT 2003
//    Adapted code to assume it knows all the variables (which are now
//    obtained from the .mili file). Set validate vars flag after run.
//    Changed sub_records to hold the mili Subrecord structure.
//
//    Kathleen Bonnell, Wed Jul  6 14:27:42 PDT 2005 
//    Initialize sv with memset to remove free of invalid pointer when
//    mc_cleanse_st_variable is called.
//
//    Mark C. Miller, Mon Jul 18 13:41:13 PDT 2005
//    Added code to deal with param-array variables
//    Added memset call to zero-out Subrecord struct
//
//    Mark C. Miller, Mon Mar  6 14:25:49 PST 2006
//    Added call to cleanse subrec at end of loop to fix a memory leak
//
//    Mark C. Miller, Wed Mar  8 08:40:55 PST 2006
//    Moved code to cleanse subrec to destructor
//
// ****************************************************************************
// TODO: refactor to retrieve missing info from the SRs, and add it to the
//       mili meta data. 
void
avtMiliFileFormat::ValidateVariables(int dom, int meshId)
{
    int rval;
    if (dbid[dom] == -1)
        OpenDB(dom);

    int srec_qty = 0;
    rval = mc_query_family(dbid[dom], QTY_SREC_FMTS, NULL, NULL,
                           (void*) &srec_qty);

    for (int i = 0 ; i < srec_qty ; i++)
    {
        int substates = 0;
        rval = mc_query_family(dbid[dom], QTY_SUBRECS, (void *) &i, NULL,
                               (void *) &substates);

        for (int srId = 0 ; srId < substates ; srId++)
        {
            Subrecord sr;
            memset(&sr, 0, sizeof(sr));
            rval = mc_get_subrec_def(dbid[dom], i, srId, &sr);
            //TODO: error check 
            
            miliMetaData[meshId]->AddSubrecord(srId, sr);
        
            //
            // glob vars are associated with the entire mesh.
            // mat vars deal with materials.
            //
            //TODO: we now need to visualize these
            //if (strcmp(sr.class_name, "glob") == 0)
            //{
            //    continue;
            //}
            //if (strcmp(sr.class_name, "mat") == 0)
            //{
            //    continue;
            //}
           
            for (int k = 0 ; k < sr.qty_svars ; k++)
            {
                 State_variable sv;
                 memset(&sv, 0, sizeof(sv));
                 mc_get_svar_def(dbid[dom], sr.svar_names[k], &sv);
                 
                 int sameAsVar = -1;
                     sameAsVar = miliMetaData[meshId]->
                         GetMiliVariableMDIdx(sv.short_name); 
                 if (sameAsVar != -1)
                 {
                     miliMetaData[meshId]->AddMiliVariableSubrecord(sameAsVar,
                                                                    dom,
                                                                    srId);
                 }
                 
                 //
                 // Assume all desired variables are in the mili file. 
                 //
                 if (sameAsVar == -1)
                 {
                     continue;
                 }

                 mc_cleanse_st_variable(&sv);
            }
        }
    }

    validateVars[dom] = true;
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
//  Programmer:  Hank Childs
//  Creation:    April 11, 2003
//
//  Modifications
//    Akira Haddox, Fri May 23 08:13:09 PDT 2003
//    Added in support for multiple meshes. Changed to MTMD.
//
//    Akira Haddox, Thu Jul 24 13:36:38 PDT 2003
//    Properly dealt with cell variable blocks.
//
//    Hank Childs, Tue Jul 20 15:53:30 PDT 2004
//    Add support for more data types (float, double, char, int, etc).
//
//    Mark C. Miller, Mon Jul 18 13:41:13 PDT 2005
//    Added code to deal with param array variables
//    Added code to deal with variables defined on the free nodes mesh
//
//    Mark C. Miller, Wed Nov 15 01:46:16 PST 2006
//    Added "no_free_nodes" variants of variables. Changed names of
//    free_node variables from 'xxx_free_nodes' to 'free_nodes/xxx'
//
//    Matt Larsen, Wed May 17 01:46:16 PST 2017
//    Added OriginalZoneLabels and OriginalNodeLabels
// ****************************************************************************

vtkDataArray *
avtMiliFileFormat::GetVar(int ts, int dom, const char *name)
{
    bool isParamArray = false;

    //
    // Decode the variable name so that we can retrieve its
    // meta data. 
    //
    string vNameStr;
    int meshId = 0;
    if (nMeshes != 1)
    {
        string noMesh;
        DecodeMultiMeshVarname(name, noMesh, meshId);
        DecodeMultiLevelVarname(noMesh, vNameStr);
    }
    else
    {
        DecodeMultiLevelVarname(name, vNameStr);
    }

    if (!readMesh[dom])
        ReadMesh(dom);
    if (!validateVars[dom])
        ValidateVariables(dom, meshId);

    vtkFloatArray *floatArr = NULL;

    if( strcmp("OriginalZoneLabels", name) == 0 )
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

    if( strcmp("OriginalNodeLabels", name) == 0 )
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

    if (strncmp(vNameStr.c_str(), "params/", 7) == 0)
        isParamArray = true; 

    //
    // We need to first retrieve the meta data for our variable. 
    //
    MiliVariableMetaData *varMD = 
        miliMetaData[meshId]->GetMiliVariableMD(vNameStr.c_str());

    if (varMD == NULL)
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    avtCentering centering     = varMD->GetCentering();
    meshId                     = varMD->GetMeshAssociation(); 
    vector<int> &varSRIds      = varMD->GetSubrecordIds(dom);
    vector<Subrecord *> varSRs = miliMetaData[meshId]->
        GetMiliVariableSubrecords(dom, vNameStr.c_str());

    //
    // Node centered variable. 
    //
    if (centering == AVT_NODECENT)
    {
        //TODO: is param array used anymore??
        if (!isParamArray && varSRs.size() != 1)
        {
            EXCEPTION1(InvalidVariableException, name);
        }
        int vType = varMD->GetNumType();

        //
        // Since data in param arrays is constant over all time,
        // we just cache it here in the plugin. Lets look in the
        // cache *before* we try to read it (again).
        //
        if (isParamArray)
        {
            floatArr = (vtkFloatArray*) cache->GetVTKObject(vNameStr.c_str(),
                     avtVariableCache::SCALARS_NAME, -1, dom, "none");
        }

        if (floatArr == 0)
        {
            int nNodes  = miliMetaData[meshId]->GetNumNodes(dom);
            floatArr    = vtkFloatArray::New();
            floatArr->SetNumberOfTuples(nNodes);
 
            float *fArrPtr = (float *) floatArr->GetVoidPointer(0);
            char *namePtr  = (char *) vNameStr.c_str();  

            read_results(dbid[dom], ts+1, varSRIds[0], 1,
                         &namePtr, vType, nNodes, fArrPtr);

            //
            // We explicitly cache param arrays at ts=-1
            //
            if (isParamArray)
            {
                cache->CacheVTKObject(vNameStr.c_str(), 
                                  avtVariableCache::SCALARS_NAME,
                                  -1, dom, "none", floatArr);
            }
        }
        else
        {
            // The reference count will be decremented by the generic database,
            // because it will assume it owns it.
            floatArr->Register(NULL);
        }
    }
    //
    // Cell centered variable. 
    //
    else
    {
        floatArr   = vtkFloatArray::New();
        int nCells = miliMetaData[meshId]->GetNumCells(dom);
        floatArr->SetNumberOfTuples(nCells);
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
        bool isMatVar = varMD->GetIsMatVar();
        bool isGlobal = varMD->GetIsGlobal();
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
            dataBuffer = (float *) floatArr->GetVoidPointer(0);
            dBuffSize  = nCells;
        }

        //
        // If our variable doesn't cover the entire dataset, we 
        // the "empty space" to be rendered grey. Nan values will
        // be mapped to grey. 
        //
        for (int i = 0 ; i < dBuffSize; i++)
        {
            dataBuffer[i] = std::numeric_limits<float>::quiet_NaN();
        }

        for (int i = 0 ; i < varSRs.size(); i++)
        {
            int vType        = varMD->GetNumType();
            int start        = 0;
            int nTargetCells = 0;

            start = miliMetaData[meshId]->
                GetMiliClassMD(varSRs[i]->class_name)->
                GetConnectivityOffset(dom);                                   
    
            //TODO: Are there cases where we don't have but
            //      need the offset? I don't believe so...
            if (start < 0)
            {
                start = 0;
            }

            //TODO: fill in missing meta data info
            //nTargetCells = miliMetaData[meshId]->
            //    GetMiliClassMD(varSRs[i].class_name)->
            //    GetNumElements(dom);  
            nTargetCells  = varSRs[i]->qty_objects;
            char *namePtr = (char *) vNameStr.c_str();  // Bypass const
            
            // Simple read in: one block 
            if (varSRs[i]->qty_blocks == 1)
            {
                // Adjust start
                start += (varSRs[i]->mo_blocks[0] - 1);
            
                read_results(dbid[dom], ts+1, varSRIds[i],
                             1, &namePtr, vType, nTargetCells, dataBuffer + start);
            }
            else
            {
                int nBlocks = varSRs[i]->qty_blocks;
                int *blocks = varSRs[i]->mo_blocks;

                int pSize = 0;
                for (int b = 0; b < nBlocks; ++b)
                    pSize += blocks[b * 2 + 1] - blocks[b * 2] + 1;

                float *mBBuffer = new float[pSize];

                read_results(dbid[dom], ts + 1, varSRIds[i],
                             1, &namePtr, vType, pSize, mBBuffer);

                //
                // Fill up the blocks into the array.
                //
                float *mbPtr = mBBuffer;
                for (int b = 0; b < nBlocks; ++b)
                {
                    for (int c = blocks[b * 2] - 1; 
                         c <= blocks[b * 2 + 1] - 1; ++c)
                    {
                        dataBuffer[c + start] = *(mbPtr++);
                    }
                }
                
                delete [] mBBuffer;
            }
        }

        //
        // If we have a material variable, we need to distribute the 
        // values across cells by material ID. 
        //
        if (isMatVar)
        {
            const int *matList = materials[dom][meshId]->GetMatlist();

            for (int i = 0; i < nCells; ++i)
            { 
                floatArr->SetTuple1(i, dataBuffer[matList[i]]);
            }

            delete [] dataBuffer;
        }
        //
        // If we have a global var, just apply it to all cells. 
        //
        else if (isGlobal)
        {
            for (int i = 0; i < nCells; ++i)
            { 
                floatArr->SetTuple1(i, dataBuffer[0]);
            }

            delete [] dataBuffer;
        }
    }

    return floatArr;
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::GetVectorVar
//
//  Purpose:
//      Gets variable 'var' for timestep 'ts'.
//
//  Arguments:
//    ts         the time step
//    var        the name of the variable to read
//
//  Programmer:  Hank Childs
//  Creation:    April 11, 2003
//
//  Modifications
//    Akira Haddox, Fri May 23 08:13:09 PDT 2003
//    Added in support for multiple meshes. Changed to MTMD.
//
//    Akira Haddox, Thu Jul 24 13:36:38 PDT 2003
//    Properly dealt with cell variable blocks.
//
//    Hank Childs, Mon Sep 22 07:36:48 PDT 2003
//    Add support for reading in tensors.
//
//    Hank Childs, Tue Jul 20 15:53:30 PDT 2004
//    Add support for more data types (float, double, char, int, etc).
//
//    Hank Childs, Tue Jul 27 12:42:12 PDT 2004
//    Fix problem with reading in double nodal vectors.
//
//    Mark C. Miller, Mon Jul 18 13:41:13 PDT 2005
//    Added code to deal with variables defined on the free nodes mesh
//
//    Mark C. Miller, Wed Nov 15 01:46:16 PST 2006
//    Added "no_free_nodes" variants of variables. Changed names of
//    free_node variables from 'xxx_free_nodes' to 'free_nodes/xxx'
// ****************************************************************************

vtkDataArray *
avtMiliFileFormat::GetVectorVar(int ts, int dom, const char *name)
{
    //TODO: is it even possible to arrive here without reading the mesh??
    string vNameStr;
    int meshId = 0;
    if (nMeshes != 1)
    {
        string noMesh;
        DecodeMultiMeshVarname(name, noMesh, meshId);
        DecodeMultiLevelVarname(noMesh, vNameStr);
    }
    else
    {
        DecodeMultiLevelVarname(name, vNameStr);
    }

    if (!readMesh[dom])
        ReadMesh(dom);
    if (!validateVars[dom])
        ValidateVariables(dom, meshId);
    
    MiliVariableMetaData *varMD = miliMetaData[meshId]->
        GetMiliVariableMD(vNameStr.c_str());

    if (varMD == NULL)
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    avtCentering centering     = varMD->GetCentering();
    meshId                     = varMD->GetMeshAssociation(); 
    vector<int> &varSRIds      = varMD->GetSubrecordIds(dom);
    vector<Subrecord *> varSRs = miliMetaData[meshId]->
        GetMiliVariableSubrecords(dom, vNameStr.c_str());

    //
    // Component dimensions will only be > 1 if we have
    // an element set. 
    //
    int vecSize   = varMD->GetVectorSize();
    int compDims  = varMD->GetComponentDims();
    int dataSize  = vecSize * compDims;

    vtkFloatArray *fltArray = vtkFloatArray::New();
    fltArray->SetNumberOfComponents(vecSize);

    //
    // Node centered variable. 
    //
    if (centering == AVT_NODECENT)
    {
        if (varSRs.size() != 1)
        {
            EXCEPTION1(InvalidVariableException, name);
        }

        int vType  = varMD->GetNumType();
        int nNodes = miliMetaData[meshId]->GetNumNodes(dom);

        fltArray->SetNumberOfTuples(nNodes);
        float *fArrPtr = (float *) fltArray->GetVoidPointer(0);
        char *namePtr  = (char *) vNameStr.c_str();  // Bypass const
        
        read_results(dbid[dom], ts+1, varSRIds[0], 1,
                     &namePtr, vType, nNodes * dataSize, fArrPtr);
    }
    //
    // Cell centered variable. 
    //
    else
    {
        int nCells      = miliMetaData[meshId]->GetNumCells(dom);
        int dBuffSize  = nCells * dataSize;

        fltArray->SetNumberOfTuples(nCells);

        //
        // If we have an element set, we need our data buffer to 
        // be larger than the final result to include integration 
        // points. Otherwise, just use the VTK pointer. 
        //
        float *dataBuffer = NULL;
        if (varMD->GetIsElementSet())
        {
            dataBuffer = new float[dBuffSize];
        }
        else
        {
            dataBuffer = (float *) fltArray->GetVoidPointer(0);
        }

        //
        // If our variable doesn't cover the entire dataset, we 
        // the "empty space" to be rendered grey. Nan values will
        // be mapped to grey. 
        //
        for (int i = 0 ; i < dBuffSize; i++)
        {
            dataBuffer[i] = std::numeric_limits<float>::quiet_NaN();
        }
        
        //TODO: what cases do variables have multiple sr associations. 
        //
        // Iterate through the variable's subrecords. 
        //
        for (int i = 0 ; i < varSRs.size(); i++)
        {
            int vType        = varMD->GetNumType();
            int start        = 0;
            int nTargetCells = 0;

            start = miliMetaData[meshId]->
                GetMiliClassMD(varSRs[i]->class_name)->
                GetConnectivityOffset(dom);                                   

            if (start < 0)
            {
                start = 0;
            }

            //TODO: this is stored in the mili meta data for most cases. 
            //      update the mmd to hold ALL cases when retrieving 
            //      subrecord info. 
            nTargetCells = varSRs[i]->qty_objects;

            string nameCpy = vNameStr;
            char *namePtr  = (char *) nameCpy.c_str();  // Bypass const

            //
            // This handles the case where all of our data is in a single
            // contiguous chunkc of cells. 
            // 
            if (varSRs[i]->qty_blocks == 1)
            {
                start         += (varSRs[i]->mo_blocks[0] - 1);
                int resultSize = nTargetCells * dataSize;
                float *dbPtr   = dataBuffer;

                read_results(dbid[dom], ts+1, varSRIds[i],
                             1, &namePtr, vType, resultSize,
                             dbPtr + (start * dataSize));
            }
            else
            {
                //TODO: need data that tests multiple blocks.
                int nBlocks = varSRs[i]->qty_blocks;
                int *blocks = varSRs[i]->mo_blocks;

                int totalBlocksSize = 0;
                for (int b = 0; b < nBlocks; ++b)
                {
                    totalBlocksSize += blocks[b * 2 + 1] - blocks[b * 2] + 1;
                }

                float *MBBuffer = new float[totalBlocksSize * dataSize];
                int resultSize  = totalBlocksSize * dataSize;

                read_results(dbid[dom], ts + 1, varSRIds[i],
                             1, &namePtr, vType, resultSize, MBBuffer);

                float *MBPtr = MBBuffer;

                //
                // Fill up the blocks into the array.
                //
                for (int b = 0; b < nBlocks; ++b)
                {
                    for (int c = blocks[b * 2] - 1; 
                         c <= blocks[b * 2 + 1] - 1; ++c)
                    {
                        for (int k = 0; k < dataSize; ++k)
                        {
                            int idx = (dataSize * (c + start)) + k;
                            dataBuffer[idx] = *(MBPtr++);
                        }
                    }
                }
                delete [] MBBuffer;
            } 
        }

        //TODO: Need to test the multiple block case. 
        //
        // If this is an element set, we need to extract the 
        // integration points, and copy then over to our 
        // VTK data array. 
        //
        if (varMD->GetIsElementSet())
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
            delete [] dataBuffer;
        }
    }

    //
    // If we have a symmetric tensor, put that in the form of a normal
    // tensor.
    //
    if (vecSize == 6)
    {
        vtkFloatArray *new_fltArray = vtkFloatArray::New();
        int ntups = fltArray->GetNumberOfTuples();
        new_fltArray->SetNumberOfComponents(9);
        new_fltArray->SetNumberOfTuples(ntups);
        for (int i = 0 ; i < ntups ; i++)
        {
            double orig_vals[6];
            float new_vals[9];
            fltArray->GetTuple(i, orig_vals);
            new_vals[0] = orig_vals[0];  // XX
            new_vals[1] = orig_vals[3];  // XY
            new_vals[2] = orig_vals[5];  // XZ
            new_vals[3] = orig_vals[3];  // YX
            new_vals[4] = orig_vals[1];  // YY
            new_vals[5] = orig_vals[4];  // YZ
            new_vals[6] = orig_vals[5];  // ZX
            new_vals[7] = orig_vals[4];  // ZY
            new_vals[8] = orig_vals[2];  // ZZ
            new_fltArray->SetTuple(i, new_vals);
        }
        fltArray->Delete();
        fltArray = new_fltArray;
    }

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
//  Method:  avtMiliFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//    Returns meta-data about the database.
//
//  Arguments:
//    md         The meta-data structure to populate
//    timeState  The time index to use (if metadata varies with time)
//
//  Programmer:  Hank Childs
//  Creation:    April 11, 2003
//
//  Modifications
//    Akira Haddox, Fri May 23 08:13:09 PDT 2003
//    Added in support for multiple meshes. Changed for MTMD.
//
//    Hank Childs, Sat Sep 20 08:15:54 PDT 2003
//    Added support for tensors and add some expressions based on tensors.
//
//    Hank Childs, Sat Oct 18 09:51:03 PDT 2003
//    Fix typo for strain/stress expressions.
//
//    Hank Childs, Sat Oct 18 10:53:51 PDT 2003
//    Do not read in the partition info if we are on the mdserver.
//
//    Hank Childs, Mon Oct 20 10:07:00 PDT 2003
//    Call OpenDB for domain 0 to populate the times.
//
//    Hank Childs, Sat Jun 26 10:28:45 PDT 2004
//    Make the materials start at "1" and go up.  Also make the domain 
//    decomposition say processor instead of block.
//
//    Hank Childs, Wed Aug 18 16:25:15 PDT 2004
//    Added new expressions for displacement and position.
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added timeState arg to satisfy new interface
//
//    Mark C. Miller, Mon Jul 18 13:41:13 PDT 2005
//    Added code to add free nodes mesh and variables
//
//    Mark C. Miller, Wed Nov 15 01:46:16 PST 2006
//    Added "no_free_nodes" variants of meshes, material and variables.
//    Changed names of free_node variables from 'xxx_free_nodes' to
//    'free_nodes/xxx'. Populated any node-centered expressions for both
//    original and free_node variants and zone-centered expressions for
//    both original and no_free_node variants. Changed cellOrigin to 1
//    to address off-by-one errors during pick. Bob Corey says that so far,
//    all clients that write mili data are Fortran clients. They expect to
//    get node/zone numbers from pick starting from '1'. 
//
//    Mark C. Miller, Wed Nov 29 12:08:49 PST 2006
//    Suppress creation of "no_free_nodes" flavors of expressions when
//    not needed
//    
//    Thomas R. Treadway, Tue Dec  5 15:14:11 PST 2006
//    Added a derived strain and displacement algorithms
//
//    Mark C. Miller, Tue Mar 27 08:39:55 PDT 2007
//    Added support for node origin
//
//    Eric Brugger, Mon Sep 21 11:01:46 PDT 2015
//    The reader now returns the cycles and times in the meta data and 
//    marks them as accurate so that they are used where needed.
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
        char meshname[32];
        char matname[32];
        sprintf(meshname, "mesh%d", meshId + 1);
        sprintf(matname, "materials%d", meshId + 1);
        avtMeshMetaData *mesh = new avtMeshMetaData;
        mesh->name = meshname;
        mesh->meshType = AVT_UNSTRUCTURED_MESH;
        mesh->numBlocks = nDomains;
        mesh->blockOrigin = 0;
        mesh->cellOrigin = 1; // Bob Corey says all mili writers so far are Fortran
        mesh->nodeOrigin = 1; // Bob Corey says all mili writers so far are Fortran
        mesh->spatialDimension = dims;
        mesh->topologicalDimension = dims;
        mesh->blockTitle = "processors";
        mesh->blockPieceName = "processor";
        mesh->hasSpatialExtents = false;
        md->Add(mesh);

        //TODO: need to add material color somehow. 
        //
        // Adding material to the meta data. 
        //
        int numMats           = miliMetaData[meshId]->GetNumMaterials();
        stringVector matNames = miliMetaData[meshId]->GetMaterialNames();
        AddMaterialToMetaData(md, matname, meshname, 
                              numMats, matNames);

        AddLabelVarToMetaData(md, 
                              "OriginalZoneLabels", 
                              meshname, 
                              AVT_ZONECENT, 
                              dims);
        //TODO: look into this. 
        // Visit is intercepting these labels and displaying something
        // else, so current work around is just to hide them
        bool hideFromGui = true;
        AddLabelVarToMetaData(md, 
                              "OriginalNodeLabels", 
                              meshname, 
                              AVT_NODECENT, 
                              dims, 
                              hideFromGui);

        //
        // Adding variables
        //
        int numVars = miliMetaData[meshId]->GetNumVariables();
        for (int i = 0 ; i < numVars; i++)
        {
            MiliVariableMetaData *miliVar = miliMetaData[meshId]->
                GetMiliVariableMD(i);
            char meshname[32];
            sprintf(meshname, "mesh%d", miliVar->GetMeshAssociation() + 1);

            int cellType           = miliVar->GetAvtCellType();
            avtCentering centering = miliVar->GetCentering();
            int vecSize            = miliVar->GetVectorSize();

            //
            // If the variable is a scalar, the vector size is designated 1, 
            // but there will be no vector components. 
            //
            vector<string> vComps  = miliVar->GetVectorComponents();
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

            string vPath;
            char multiVname[1024];
            if (nMeshes == 1)
            {
                vPath = miliVar->GetPath();
            }
            else
            {
                sprintf(multiVname, "%s(%s)(mesh%d)%s", 
                    miliVar->GetClassLongName().c_str(),
                    miliVar->GetClassShortName().c_str(), 
                    miliVar->GetMeshAssociation() + 1,
                    miliVar->GetShortName().c_str());
                vPath  = multiVname;
            }

            switch (cellTypeCast)
            {
                case AVT_SCALAR_VAR:
                {
                    AddScalarVarToMetaData(md, vPath, meshname, centering);
                    break;
                }
                case AVT_VECTOR_VAR:
                {
                    AddVectorVarToMetaData(md, vPath, meshname, 
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
                        meshname, centering, 9);

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
                        string singleName = vPath + "/" + miliVar->GetVectorComponent(j);
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
                        string multName = vPath + "/" + miliVar->GetVectorComponent(compIdx);
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
                        meshname, centering, nComps);

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
                        string singleName = vPath + "/" + miliVar->GetVectorComponent(j);
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
                        string multName = vPath + "/" + miliVar->GetVectorComponent(compIdx);
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
                        compNames.push_back(miliVar->GetVectorComponent(j));
                        char name[1024];
                        sprintf(name, "%s/%s", vPath.c_str(), 
                            miliVar->GetVectorComponent(j).c_str());
                        Expression expr = ScalarExpressionFromVec(vPath.c_str(),
                                              name, j);
                        md->AddExpression(&expr);
                    }

                    AddArrayVarToMetaData(md, vPath, compNames, meshname, 
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
avtMiliFileFormat::GetAuxiliaryData(const char *var, int ts, int dom, 
                                    const char * type, void *,
                                    DestructorFunction &df) 
{
    if (strcmp(type, AUXILIARY_DATA_MATERIAL) && strcmp(type, "AUXILIARY_DATA_IDENTIFIERS"))
    {
        return NULL;
    }
    if (!readMesh[dom])
        ReadMesh(dom);
    
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


// ****************************************************************************
//  Method: avtMiliFileFormat::FreeUpResources
//
//  Purpose:
//      Close databases and free up non-essential memory.
//
//  Programmer: Akira Haddox
//  Creation:   July 23, 2003
//
//  Modifications:
//
//    Hank Childs, Tue Jul 27 10:18:26 PDT 2004
//    Moved the code to free up resources to the destructor.
// ****************************************************************************

void
avtMiliFileFormat::FreeUpResources()
{
}


//TODO: move json extraction to miliMetaData
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

void 
avtMiliFileFormat::ExtractJsonVariable(MiliVariableMetaData *mVar,
                                       const Value &val,
                                       string name,
                                       string cShortName,
                                       string cLongName,
                                       int meshId)
{
    if (val.IsObject())
    {
        //
        // Extract our variable info. 
        //
        mVar->SetShortName(name);
        
        //
        // If the variable begins with es_, it is an element set. 
        //
        string esId    = "es_";
        string nameSub = name.substr(0, 3);

        if (esId == nameSub)
            mVar->SetIsElementSet(true); 

        if (val.HasMember("LongName"))
            mVar->SetLongName(val["LongName"].GetString());

        if (val.HasMember("Center"))
            mVar->SetCentering(val["Center"].GetInt());

        if (val.HasMember("VTK_TYPE"))
            mVar->SetAvtCellType(val["VTK_TYPE"].GetInt());

        if (val.HasMember("agg_type"))
            mVar->SetMiliCellType(val["agg_type"] .GetInt());

        if (val.HasMember("vector_size"))
            mVar->SetVectorSize(val["vector_size"].GetInt());

        if (val.HasMember("dims"))
        {
            int cDims = val["dims"].GetInt();
            if (cDims == 0)
                cDims = 1;

            mVar->SetComponentDims(cDims);
        }
  
        if (val.HasMember("num_type"))
            mVar->SetNumType(val["num_type"].GetInt());

        if (val.HasMember("vector_components"))
        {
            const Value &vComps = val["vector_components"];

            if (vComps.IsArray())
            {
                for (SizeType i = 0; i < vComps.Size(); ++i)
                {
                    mVar->AddVectorComponent(vComps[i].GetString());
                }
            }
        }

        mVar->SetClassShortName(cShortName);
        mVar->SetClassLongName(cLongName);
        mVar->SetMeshAssociation(meshId);
    }
}


//TODO: is this inherited? Needs doc
bool
avtMiliFileFormat::CanCacheVariable(const char *varname)
{
    if (strncmp(varname, "params/", 7) == 0)
        return false;
    else
        return true;
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
            // Store the class meta data. 
            //
            MiliClassMetaData *miliClass = 
                new MiliClassMetaData(nDomains);
            miliClass->SetShortName(sName);

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
            if (val.HasMember("LongName"))
            {
                lName = val["LongName"].GetString();
                miliClass->SetLongName(lName);
            }

            if (val.HasMember("ElementCount"))
                miliClass->SetTotalNumElements(val["ElementCount"].GetInt());
            else
            {
                char msg[256];
                sprintf(msg, "Mili file classes must contain ElementCount");
                EXCEPTION1(ImproperUseException, msg);
            }
               
            if (val.HasMember("SuperClass"))
            {
                int sClass = val["SuperClass"].GetInt();
                miliClass->SetSuperClassId(sClass);
            }
            else
            {
                //TODO: handle this?
                //char msg[256];
                //sprintf("%s", "Mili file classes must contain SuperClass ID");
                //EXCEPTION1(ImproperUseException, msg);
            }

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

                        miliClass->AddMiliVariable(varName);
                        MiliVariableMetaData *varMD = new MiliVariableMetaData;
                        
                        if (isMatVar)
                        {
                            varMD->SetIsMatVar(isMatVar);
                        }
                        else if (isGlobal)
                        {
                            varMD->SetIsGlobal(isGlobal);
                        }

                        ExtractJsonVariable(varMD, var, varName,
                            sName, lName, meshId);

                        miliMetaData[meshId]->AddMiliVariableMD(varIdx, varMD);
                        varIdx++;

                    } // end jVars.HasMember

                } // end for i in length of cVars
               
                //
                // Cache the mili class md. 
                //
                miliMetaData[meshId]->AddMiliClassMD(classIdx, miliClass);
                classIdx++;

            } // end val.HasMember("variables")

        }// end if val.IsObject()

    } // end for cItr in jClasses
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
void
avtMiliFileFormat::LoadMiliInfoJson(const char *fname)
{
    ifstream jfile;
    jfile.open(fname);

    IStreamWrapper isw(jfile);
    Document jDoc;
    jDoc.ParseStream(isw);

    string path = "";
    nDomains =  nMeshes = 1;

    //
    // Retrieve the file path. 
    //
    if (jDoc.HasMember("Path"))
    {
        //FIXME: do we even need this anymore?
        path = jDoc["Path"].GetString();

        //FIXME: filepath is only 512 chars long... is that really enough?
        //       we could just turn this into a string for safety. 
        strcpy(filepath, path.c_str());
        filepath[path.size() + 1] = '\0';
    }

    if (jDoc.HasMember("Domains"))
    {
        nDomains = jDoc["Domains"].GetInt(); 
    }

    if (jDoc.HasMember("Number_of_Meshes"))
        nMeshes = jDoc["Number_of_Meshes"].GetInt();

    if (jDoc.HasMember("Dimensions"))
        dims = jDoc["Dimensions"].GetInt();

    if (jDoc.HasMember("States"))
    {
        const Value &jStates = jDoc["States"];
        if (jStates.HasMember("count"))
            nTimesteps = jStates["count"].GetInt();
        //TODO:
        //else
        // RAISE ERROR 
    }
    
    setTimesteps = false;
    
    dbid.resize(nDomains, -1);
    readMesh.resize(nDomains, false);
    validateVars.resize(nDomains, false);
    datasets.resize(nDomains);

    zoneLabels.resize(nDomains);
    nodeLabels.resize(nDomains);
    zone_label_mappings.resize(nDomains);
    node_label_mappings.resize(nDomains);
    max_zone_label_lengths.resize(nDomains);
    max_node_label_lengths.resize(nDomains);

    for(int i = 0; i < nDomains; ++i)
    {
        max_zone_label_lengths[i] = 0;
        max_node_label_lengths[i] = 0;
    }
    materials.resize(nDomains);
    
    miliMetaData = new MiliMetaData *[nMeshes];

    for (int i = 0; i < nMeshes; ++i) 
        miliMetaData[i] = NULL;

    int dom;
    for (dom = 0; dom < nDomains; ++dom)
    {
        datasets[dom].resize(nMeshes, NULL);
        materials[dom].resize(nMeshes, NULL);
    }

    for (int meshId = 0; meshId < nMeshes; ++meshId)
    {

        miliMetaData[meshId] = new MiliMetaData(nDomains);

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
            for (Value::ConstMemberIterator jItr = jMats.MemberBegin();
                 jItr != jMats.MemberEnd(); ++jItr)
            { 
                string name = jItr->name.GetString();
                const Value &mat = jItr->value;
                  
                if (mat.IsObject())
                {
                    MiliMaterialMetaData miliMaterial;
                    if (mat.HasMember("name"))
                    {
                        miliMaterial.SetName(mat["name"].GetString());
                    }
                    //TODO: can we better handle this somehow?
                    else
                    {
                        miliMaterial.SetName("");
                    }

                    if (mat.HasMember("COLOR"))
                    {
                        const Value &mColors = mat["COLOR"];

                        if (mColors.IsArray())
                        {
                            float matColors[3];
                            setTimesteps = true;
                            for (SizeType i = 0; i < mColors.Size(); ++i)
                            {
                                matColors[i] = mColors[i].GetFloat();
                            }
                            miliMaterial.SetColor(matColors);
                        }

                        else
                        {
                            //FIXME: Raise error
                        }
                    }
                    //TODO: it may be better to let VisIt handle
                    //      this coloring...
                    else
                    {
                        //
                        // If the material doesn't have a color
                        // associated with it, generate a random color. 
                        //
                        float matColor[3];
                        for (int i = 0; i < 3; ++i)
                        {
                            float cChannel = static_cast<float> (rand()) /
                                             static_cast<float> (RAND_MAX);
                            matColor[i] = cChannel;
                        }

                        miliMaterial.SetColor(matColor);
                    }

                    miliMetaData[meshId]->AddMiliMaterialMD(miliMaterial);
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
                    setTimesteps = true;
                    for (SizeType i = 0; i < jTimes.Size(); ++i)
                    {
                        times.push_back(jTimes[i].GetDouble());
                        cycles.push_back(i);
                    }
                }
            }
        }
    }        

    jfile.close();
}



// ***************************************************************************
//  Function: PopulateZoneLabels
//
//  Purpose:
//      Populate data structures to implement a reverse mapping between
//      a zone label and a zone id
//  Arguments: 
//             fam_id - mili familiy id
//             meshId - id of the mesh associtated with the labels 
//             short_name - the short name of the mili class (e.g., "brick") 
//             dom - id of the domain 
//             num_zones - running count of the total number of zones in
//                         the mesh. Needed for the reverse mapping
//             elems_in_group - number of elements in this group
//           
//  Author: Matt Larsen May 10 2017
//
//  Modifications:
//
// ****************************************************************************
void 
avtMiliFileFormat::PopulateZoneLabels(const int fam_id, const int meshId, 
                                      char *short_name, const int dom, 
                                      const int elems_in_group)
{

    int num_zones = 0;
    int num_blocks = 0; 
    int **block_range = new int*[1];
    block_range[0] = NULL;
    int *elem_list = new int[elems_in_group];
    int *label_ids = new int[elems_in_group];
    
    //
    // Check for labels
    //
    int num_expected_labels = elems_in_group;
    mc_load_conn_labels(dbid[dom], meshId, short_name, 
                        num_expected_labels, &num_blocks, 
                        block_range, elem_list ,label_ids);
    
    if(num_blocks == 0)
    {
        debug4<<"mili block contains no labels\n";
        //Create default labels
        for(int elem_num = 1; elem_num <= elems_in_group; ++ elem_num)
        {
            // create the label strings for each cell
            std::stringstream sstream;
            sstream<<"";
            zoneLabels[dom].push_back(sstream.str());
            max_zone_label_lengths[dom] = 
                std::max(int(sstream.str().size()), max_zone_label_lengths[dom]);
        }
        num_zones += elems_in_group;
    }
    else
    {
        debug4<<"Mili labels found. There are "<<num_blocks<<" blocks in class "<<short_name<<" in dom "<<dom<<"\n";
        for(int el = 0; el < elems_in_group; ++el)
        {
            std::stringstream sstream;
            sstream<<short_name;
            sstream<<" "<<label_ids[el];
            zoneLabels[dom].push_back(sstream.str());
            max_zone_label_lengths[dom] = 
                std::max(int(sstream.str().size()), max_zone_label_lengths[dom]);
        }
        Label_mapping label_map;
        for(int block = 0; block < num_blocks; ++block)
        {
          int range_size = block_range[0][block * 2 + 1] - block_range[0][block * 2] + 1;
          label_map.label_ranges_begin.push_back(block_range[0][block*2]);
          label_map.label_ranges_end.push_back(block_range[0][block*2+1]);
          label_map.el_ids_begin.push_back(num_zones);
          label_map.el_ids_end.push_back(num_zones - 1 + range_size);

          num_zones += range_size;
        }

        zone_label_mappings[dom][std::string(short_name)] = label_map;

    }

    if(block_range[0]) delete[] block_range[0];
    delete[] block_range;

    delete [] label_ids;
    delete [] elem_list;
}

// ***************************************************************************
//  Function: PopulateNodeLabels
//
//  Purpose:
//      Populate data structures to implement a reverse mapping between
//      a zone label and a zone id
//  Arguments: 
//             fam_id - mili familiy id
//             meshId - id of the mesh associtated with the labels 
//             short_name - the short name of the mili class (e.g., "brick") 
//             dom - id of the domain 
//             num_zones - running count of the total number of zones in
//                         the mesh. Needed for the reverse mapping
//           
//  Author: Matt Larsen May 10 2017
//
//  Modifications:
//
// ****************************************************************************
void 
avtMiliFileFormat::PopulateNodeLabels(const int fam_id, const int meshId, 
                                      char *short_name, const int dom)
{

    int num_nodes = 0;
    int n_nodes = miliMetaData[meshId]->GetNumNodes(dom);
    max_node_label_lengths[dom] = 0;
    int num_blocks = 0; 
    int **block_range = new int*[1];
    block_range[0] = NULL;
    int *elem_list = new int[n_nodes];
    int *label_ids = new int[n_nodes];

    mc_load_node_labels(dbid[dom], meshId, short_name, 
                        &num_blocks,block_range,label_ids);


    if(num_blocks == 0)
    {
        debug4<<"Mili block does not contain node labels\n";
        
        //Create default labels
        for(int elem_num = 1; elem_num <= n_nodes; ++elem_num)
        {
            // create the label strings for each cell
            std::stringstream sstream;
            sstream<<"";
            zoneLabels[dom].push_back(sstream.str());
            max_node_label_lengths[dom] = 
                std::max(int(sstream.str().size()), max_node_label_lengths[dom]);
        }
        num_nodes += n_nodes;
    }
    else
    {
        debug4<<"Mili labels node found. There are "<<num_blocks<<" blocks in class "<<short_name<<" in dom "<<dom<<"\n";
        for(int el = 0; el < n_nodes; ++el)
        {
            std::stringstream sstream;
            sstream<<short_name;
            sstream<<" "<<label_ids[el];
            nodeLabels[dom].push_back(sstream.str());
            max_node_label_lengths[dom] = 
                std::max(int(sstream.str().size()), max_node_label_lengths[dom]);
        }
        Label_mapping label_map;
        for(int block = 0; block < num_blocks; ++block)
        {
          int range_size = block_range[0][block * 2 + 1] - block_range[0][block * 2] + 1;
          label_map.label_ranges_begin.push_back(block_range[0][block*2]);
          label_map.label_ranges_end.push_back(block_range[0][block*2+1]);
          label_map.el_ids_begin.push_back(num_nodes);
          label_map.el_ids_end.push_back(num_nodes - 1 + range_size);

           num_nodes += range_size;
        }

        node_label_mappings[dom][std::string(short_name)] = label_map;

    }
    
    //
    // cleanup
    //
    if(block_range[0]) delete[] block_range[0];
    delete[] block_range;

    delete [] label_ids;
    delete [] elem_list;

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

