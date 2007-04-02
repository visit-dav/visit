/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            avtCosmosPPFileFormat.C                        //
// ************************************************************************* //

#include <avtCosmosPPFileFormat.h>

#include <ctype.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>

#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkPoints.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>
#include <avtGhostData.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidDBTypeException.h>
#include <InvalidVariableException.h>

#include <hdf5.h>

using std::vector;
using std::string;

static string GetDirName(const char *path);

static int PointSorter(const void *arg1, const void *arg2);

typedef struct
{
    float pt[3];
    int   id;
} PointWithId;


// ****************************************************************************
//  Constructor:  avtCosmosPPFileFormat::avtCosmosPPFileFormat
//
//  Arguments:
//    fname      the file name of the master CosmosPP file
//
//  Programmer:  Hank Childs
//  Creation:    November 24, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Jul 19 16:57:59 PDT 2004
//    Initialize haveIssuedWarning.
//
// ****************************************************************************

avtCosmosPPFileFormat::avtCosmosPPFileFormat(const char *fname)
    : avtMTMDFileFormat(fname)
{
    haveIssuedWarning = false;
    dirname = GetDirName(fname);

    ifstream ifile(fname);

    if (ifile.fail())
    {
        EXCEPTION1(InvalidDBTypeException, "Unable to open cosmos file.");
    }
    
    ntimesteps = ndomains = rank = -1;

    int i, j;

    //
    // Parse the master file to find out the information that we need.
    // We're not too picky about the format as long as the necessary
    // parts are there, and that #DOMAINS is defined before the
    // grid and variable files.
    //

    for (;;)
    {
        string key;
        ReadString(ifile, key);

        if (ifile.eof() || ifile.fail())
                break;
        
        if (key == "#DOMAINS")
        {
            ifile >> ndomains;
            proc_names.resize(ndomains);
        }
        else if (key == "#TIMESTEPS")
        {
            ifile >> ntimesteps;
            cycles.resize(ntimesteps);
            times.resize(ntimesteps);
            dump_names.resize(ntimesteps);
            debug1 << "Reading timesteps = " << ntimesteps << endl;
        }
        else if (key == "#DUMP_CYCLE_TIME_FILENAME")
        {
            if (ntimesteps <= 0)
            {
                EXCEPTION1(InvalidDBTypeException, "Must declare ntimesteps "
                                                   "before timestep info.");    
            }
            for (i = 0 ; i < ntimesteps ; i++)
            {
                string dumpid;
                ReadString(ifile, dumpid);

                string c;
                ReadString(ifile, c);
                cycles[i] = atoi(c.c_str());

                string t;
                ReadString(ifile, t);
                times[i] = atof(t.c_str());

                string tmp;
                ReadString(ifile, tmp);
                dump_names[i] = tmp;
            }
        }
        else if (key == "#DIMENSIONS")
        {
            ifile >> rank;
        }
        else if (key == "#DIRECTORIES")
        {
            if (ndomains < 0)
                EXCEPTION1(InvalidDBTypeException, "Domains not defined "
                                                   "before data.");    
            string value;
            for (i = 0; i < ndomains; ++i)
            {
                ReadString(ifile, value);
                proc_names[i] = dirname + value;
            }
        }
        else if (key == "#SCALARFIELD")
        {
            if (ndomains < 0)
                EXCEPTION1(InvalidDBTypeException, "Domains not defined "
                                                   "before data.");    
            // Strip out leading white space.
            while (isspace(ifile.peek()))
                ifile.get();

            string value;
            char line[1024];
            ifile.getline(line, 1024);
            value = line;

            scalarVarNames.push_back(value);
        }
        else if (key == "#VECTORFIELD")
        {
            if (ndomains < 0)
                EXCEPTION1(InvalidDBTypeException, "Domains not defined "
                                                   "before data.");    
            // Strip out leading white space.
            while(isspace(ifile.peek()))
                ifile.get();

            string value;
            char line[1024];
            ifile.getline(line, 1024);
            value = line;

            vectorVarNames.push_back(value);
        }
        else if (key == "#NUMBEROFFIELDS")
        {
            // No action -- we don't need this.
            int numberOfFields;
            ifile >> numberOfFields;
        }
        else
        {
            debug1 << "Found bad key = " << key.c_str() << endl;
            EXCEPTION1(InvalidDBTypeException, "Badly formed cosmos file.");
        }
    }

    ifile.close();
    
    if (ntimesteps == -1)
        EXCEPTION1(InvalidDBTypeException, "Number of timesteps not defined.");
    if (ndomains == -1)
        EXCEPTION1(InvalidDBTypeException, "Number of domains not defined.");
    if (rank == -1)
        EXCEPTION1(InvalidDBTypeException, "Dimensions not defined.");

    readDataset.resize(ntimesteps);
    dataset.resize(ntimesteps);
    for (i = 0 ; i < ntimesteps ; i++)
    {
        readDataset[i].resize(ndomains);
        dataset[i].resize(ndomains);
        for (j = 0 ; j < ndomains ; j++)
        {
            readDataset[i][j] = false;
            dataset[i][j] = NULL;
        }
    }
    nscalars = scalarVarNames.size();
    nvectors = vectorVarNames.size();
}


// ****************************************************************************
//  Destructor:  avtCosmosPPFileFormat::~avtCosmosPPFileFormat
//
//  Programmer:  Hank Childs
//  Creation:    November 24, 2003
//
// ****************************************************************************

avtCosmosPPFileFormat::~avtCosmosPPFileFormat()
{
    for (int i = 0 ; i < ntimesteps ; i++)
        for (int j = 0 ; j < ndomains ; j++)
        {
            if (dataset[i][j] != NULL)
                dataset[i][j]->Delete();
        }
}


// ****************************************************************************
//  Method: avtCosmosPPFileFormat::ReadDataset
//
//  Purpose:
//      Reads a dataset in for a specific timestep and domain.
//
//  Programmer: Hank Childs
//  Creation:   December 31, 2003
//
//  Modifications:
//
//    Hank Childs, Fri May  7 15:31:07 PDT 2004
//    The VTK point locator is numerically sensitive if you give it lots of
//    points along a plane.  So we are going to code around it and dummy up
//    z-values as x-values.
//
//    Hank Childs, Fri Aug 27 16:54:45 PDT 2004
//    Rename ghost data array.
//
//    Hank Childs, Wed Sep 15 08:14:22 PDT 2004
//    Finally gave up using VTK's point locator and wrote our own.
//
// ****************************************************************************

void
avtCosmosPPFileFormat::ReadDataset(int ts, int dom)
{
    int   i, j;

    if (dataset[ts][dom] != NULL)
        return;

    dataset[ts][dom] = vtkUnstructuredGrid::New();

    string filename = proc_names[dom] + string("/") + dump_names[ts];
    hid_t file_handle = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);

    int nodes_per_zone = 0;
    switch (rank)
    {
       case 1:
         nodes_per_zone = 2; 
         break;
       case 2:
         nodes_per_zone = 4; 
         break;
       case 3:
         nodes_per_zone = 8; 
         break;
    }

    // The last vector is the node position, which we count separately.
    int num_comps = nscalars + (nvectors-1)*rank + nodes_per_zone*rank;

    // Figure out how big the array is, and from that determin how many zones
    // there are.
    int c_handle = H5Dopen(file_handle, "Cosmos++");
    int space_id = H5Dget_space(c_handle);
    hsize_t vdims[3];
    H5Sget_simple_extent_dims(space_id, vdims, NULL);
    if ((vdims[0] % num_comps) != 0)
    {
        EXCEPTION0(ImproperUseException);
    }
    int nzones = vdims[0] / num_comps;

    float *all_vars = new float[nzones*num_comps];
    H5Dread(c_handle, H5T_NATIVE_FLOAT, H5S_ALL, space_id, H5P_DEFAULT,
            all_vars);

    float *current = all_vars;
    for (i = 0 ; i < nscalars ; i++)
    {
        vtkFloatArray *arr = vtkFloatArray::New();
        arr->SetName(scalarVarNames[i].c_str());
        arr->SetNumberOfTuples(nzones);
        for (j = 0 ; j < nzones ; j++)
        {
            arr->SetTuple1(j, *current);
            current++;
        }
        dataset[ts][dom]->GetCellData()->AddArray(arr);
        arr->Delete();
    }
    for (i = 0 ; i < nvectors-1 ; i++)
    {
        vtkFloatArray *arr = vtkFloatArray::New();
        arr->SetName(vectorVarNames[i].c_str());
        arr->SetNumberOfComponents(3);
        arr->SetNumberOfTuples(nzones);
        for (j = 0 ; j < nzones ; j++)
        {
            arr->SetTuple3(j, current[0], current[1], 
                              (rank < 3 ? 0. : current[2]));
            current += rank;
        }
        dataset[ts][dom]->GetCellData()->AddArray(arr);
        arr->Delete();
    }

    vtkPoints *pts = vtkPoints::New();
    int npts = nodes_per_zone*nzones;
    float bounds[6];
    bounds[0] = +FLT_MAX;
    bounds[1] = -FLT_MAX;
    bounds[2] = +FLT_MAX;
    bounds[3] = -FLT_MAX;
    bounds[4] = +FLT_MAX;
    bounds[5] = -FLT_MAX;
   
    float *current_tmp = current;
    for (i = 0 ; i < npts ; i++)
    {
        float x = current_tmp[0];
        bounds[0] = (x < bounds[0] ? x : bounds[0]);
        bounds[1] = (x > bounds[1] ? x : bounds[1]);
        float y = current_tmp[0];
        bounds[2] = (y < bounds[2] ? y : bounds[2]);
        bounds[3] = (y > bounds[3] ? y : bounds[3]);
        if (rank == 3)
        {
            float z = current_tmp[2];
            bounds[4] = (z < bounds[4] ? z : bounds[4]);
            bounds[5] = (z > bounds[5] ? z : bounds[5]);
        }
        current_tmp += rank;
    }
    float x_diff = bounds[1] - bounds[0];
    bounds[0] -= x_diff*0.1;
    bounds[1] += x_diff*0.1;
    float y_diff = bounds[3] - bounds[2];
    bounds[2] -= y_diff*0.1;
    bounds[3] += y_diff*0.1;
    if (rank < 3)
    {
        bounds[4] = -(bounds[1]-bounds[0])*0.5;
        bounds[5] = +(bounds[1]-bounds[0])*0.5;
    }
    else
    {
        // HACK!  The VTK point locator doesn't work very well with 2D objects,
        // so make this appear to be 3D by using the X-coordinate.
        bounds[4] = bounds[0];
        bounds[5] = bounds[1];
    }

    PointWithId *pwid = new PointWithId[npts];
    current_tmp = current;
    for (i = 0 ; i < npts ; i++)
    {
        pwid[i].pt[0] = current_tmp[0];
        pwid[i].pt[1] = current_tmp[1];
        pwid[i].pt[2] = (rank < 3 ? 0 : current_tmp[2]);
        pwid[i].id = i;
        current_tmp += rank;
    }
 
    qsort(pwid, npts, sizeof(PointWithId), PointSorter);

    vector<int> ptIds(npts);
    ptIds[0] = 0;
    int nUniquePts = 1;
    for (i = 1 ; i < npts ; i++)
    {
        if ((pwid[i].pt[0] == pwid[i-1].pt[0]) && 
            (pwid[i].pt[1] == pwid[i-1].pt[1]) &&
            (pwid[i].pt[2] == pwid[i-1].pt[2]))
        {
            ptIds[pwid[i].id] = ptIds[pwid[i-1].id];
        }
        else
        {
            ptIds[pwid[i].id] = ptIds[pwid[i-1].id]+1;
            nUniquePts++;
        }
    }

    pts->SetNumberOfPoints(nUniquePts);
    float *ptr = (float *) pts->GetVoidPointer(0);
    for (i = 0 ; i < npts ; i++)
    {
        bool usePoint = false;
        if (i == 0)
            usePoint = true;
        else if (ptIds[pwid[i].id] != ptIds[pwid[i-1].id])
            usePoint = true;

        if (usePoint)
        {
            *ptr++ = pwid[i].pt[0];
            *ptr++ = pwid[i].pt[1];
            *ptr++ = pwid[i].pt[2];
        }
    }
    
    dataset[ts][dom]->SetPoints(pts);
    delete [] pwid;
    pts->Delete();
    dataset[ts][dom]->Allocate(nzones);
    for (i = 0 ; i < nzones ; i++)
    {
        if (rank == 2)
        {
            vtkIdType ids[4];
            ids[0] = ptIds[4*i];
            ids[1] = ptIds[4*i+1];
            ids[2] = ptIds[4*i+2];
            ids[3] = ptIds[4*i+3];
            dataset[ts][dom]->InsertNextCell(VTK_QUAD, 4, ids);
        }
        else
        {
            vtkIdType ids[8];
            ids[0] = ptIds[8*i];
            ids[1] = ptIds[8*i+1];
            ids[2] = ptIds[8*i+2];
            ids[3] = ptIds[8*i+3];
            ids[4] = ptIds[8*i+4];
            ids[5] = ptIds[8*i+5];
            ids[6] = ptIds[8*i+6];
            ids[7] = ptIds[8*i+7];
            dataset[ts][dom]->InsertNextCell(VTK_HEXAHEDRON, 8, ids);
        }
    }

    // retrieve internal zone size attribute
    int numInternalZones;
    int attr1  = H5Aopen_name(c_handle, "Number of Internal Zones");
    H5Aread(attr1, H5T_NATIVE_INT, &numInternalZones);

    vtkUnsignedCharArray *ghosts = vtkUnsignedCharArray::New();
    ghosts->SetName("avtGhostZones");
    ghosts->SetNumberOfTuples(nzones);
    if (numInternalZones > nzones)
    {
        if (!haveIssuedWarning)
        {
            char msg[1024];
            sprintf(msg, "The file %s claims to have more internal zones (%d)"
                         " than there are actual zones (%d).  This warning "
                         "will only be issued one time per session, even if "
                         "additional problems are found.", filename.c_str(),
                          numInternalZones, nzones);
            avtCallback::IssueWarning(msg);
            haveIssuedWarning = true;
        }
        numInternalZones = nzones;
    }

    for (i = 0 ; i < numInternalZones ; i++)
        ghosts->SetTuple1(i, 0);
    unsigned char ghostVal = 0;
    avtGhostData::AddGhostZoneType(ghostVal, 
                                   DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
    for (i = numInternalZones ; i < nzones ; i++)
        ghosts->SetTuple1(i, ghostVal);
    dataset[ts][dom]->GetCellData()->AddArray(ghosts);
    ghosts->Delete();

    // Clean up resources.
    H5Fclose(file_handle);
    delete [] all_vars;

    readDataset[ts][dom] = true;
}


// ****************************************************************************
//  Method:  avtCosmosPPFileFormat::GetMesh
//
//  Purpose:
//    Returns the mesh with the given name for the given time step and
//    domain.
//
//  Arguments:
//    ts         the time step
//    dom        the domain number
//    mesh       the name of the mesh to read
//
//  Programmer:  Hank Childs
//  Creation:    November 24, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Aug 27 16:54:45 PDT 2004
//    Rename ghost data array.
//
// ****************************************************************************

vtkDataSet *
avtCosmosPPFileFormat::GetMesh(int ts, int dom, const char *mesh)
{
    if ((ts < 0) || (ts >= ntimesteps))
    {
        EXCEPTION2(BadIndexException, ts, ntimesteps);
    }
    if ((dom < 0) || (dom >= ndomains))
    {
        EXCEPTION2(BadIndexException, dom, ndomains);
    }
 
    if (! readDataset[ts][dom])
    {
        ReadDataset(ts, dom);
    }

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->CopyStructure(dataset[ts][dom]);
    ugrid->GetCellData()->AddArray(
                  dataset[ts][dom]->GetCellData()->GetArray("avtGhostZones"));
    return ugrid;
}


// ****************************************************************************
//  Method:  avtCosmosPPFileFormat::GetVar
//
//  Purpose:
//    Reads the variable with the given name for the given time step and
//    domain. 
//
//  Arguments:
//    ts         the time step
//    dom        the domain number
//    name       the name of the variable to read
//
//  Programmer:  Hank Childs
//  Creation:    November 24, 2003
//
// ****************************************************************************

vtkDataArray *
avtCosmosPPFileFormat::GetVar(int ts, int dom, const char *name)
{
    if ((ts < 0) || (ts >= ntimesteps))
    {
        EXCEPTION2(BadIndexException, ts, ntimesteps);
    }
    if ((dom < 0) || (dom >= ndomains))
    {
        EXCEPTION2(BadIndexException, dom, ndomains);
    }
 
    if (! readDataset[ts][dom])
    {
        ReadDataset(ts, dom);
    }

    vtkDataArray *arr = dataset[ts][dom]->GetCellData()->GetArray(name);
    if (arr == NULL)
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    // The calling function will believe that it owns the memory.  Increment
    // it so that the reference count will be correct.
    arr->Register(NULL);
    return arr;
}


// ****************************************************************************
//  Method:  avtCosmosPPFileFormat::GetVectorVar
//
//  Purpose:
//    Reads the vector variable with the given name for the given time step and
//    domain. 
//
//  Arguments:
//    ts         the time step
//    dom        the domain number
//    name       the name of the variable to read
//
//  Programmer:  Hank Childs
//  Creation:    November 24, 2003
//
// ****************************************************************************

vtkDataArray *
avtCosmosPPFileFormat::GetVectorVar(int ts, int dom, const char *name)
{
    return GetVar(ts, dom, name);
}


// ****************************************************************************
//  Method:  avtCosmosPPFileFormat::GetCycles
//
//  Purpose:
//    Returns the actual cycle numbers for each time step.
//
//  Arguments:
//   c           the output vector of cycle numbers 
//
//  Programmer:  Hank Childs
//  Creation:    November 24, 2003
//
// ****************************************************************************

void
avtCosmosPPFileFormat::GetCycles(vector<int> &c)
{
    c = cycles;
}


// ****************************************************************************
//  Method:  avtCosmosPPFileFormat::GetTimes
//
//  Purpose:
//    Returns the actual time for each time step.
//
//  Arguments:
//   t           the output vector of times 
//
//  Programmer:  Hank Childs
//  Creation:    December 31, 2003
//
// ****************************************************************************

void
avtCosmosPPFileFormat::GetTimes(vector<double> &t)
{
    t = times;
}


// ****************************************************************************
//  Method:  avtCosmosPPFileFormat::GetNTimesteps
//
//  Purpose:
//    Returns the number of timesteps
//
//  Programmer:  Hank Childs
//  Creation:    November 24, 2003
//
// ****************************************************************************

int
avtCosmosPPFileFormat::GetNTimesteps()
{
    return ntimesteps;
}


// ****************************************************************************
//  Method:  avtCosmosPPFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//    Returns meta-data about the database.
//
//  Arguments:
//    md         The meta-data structure to populate
//    timeState  The time index to use (if metadata varies with time)
//
//  Programmer:  Hank Childs
//  Creation:    November 24, 2003
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added timeState arg to satisfy new interface
// ****************************************************************************

void
avtCosmosPPFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md,
    int timeState)
{
    int i;

    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "mesh";
    mesh->meshType = AVT_UNSTRUCTURED_MESH;
    mesh->blockTitle = "processors";
    mesh->blockPieceName = "processor";
    mesh->numBlocks = ndomains;
    mesh->blockOrigin = 0;
    mesh->spatialDimension = rank;
    mesh->topologicalDimension = rank;
    mesh->hasSpatialExtents = false;
    mesh->containsGhostZones = AVT_HAS_GHOSTS;
    md->Add(mesh);
    
    for (i = 0; i < nscalars; i++)
        md->Add(new avtScalarMetaData(scalarVarNames[i], "mesh", AVT_ZONECENT));
    for (i = 0; i < nvectors; i++)
    {
        if (vectorVarNames[i] == "nodePosition")
            continue;
        md->Add(new avtVectorMetaData(vectorVarNames[i], "mesh", 
                                      AVT_ZONECENT, rank));
    }
}


// ****************************************************************************
//  Method:  avtCosmosPPFileFormat::ReadString
//
//  Purpose:
//      Read in a string from a file. Works like you would expect
//      ifile >> str to work, except this will compile on all platforms.
//
//  Programmer:  Hank Childs
//  Creation:    November 24, 2003
//
// ****************************************************************************

void
avtCosmosPPFileFormat::ReadString(ifstream &ifile, std::string &str)
{
    char c;
    do
    {
        c = ifile.get();
        if (ifile.eof() || ifile.fail())
            return;
    }while (isspace(c));    

    str = c;

    while (!isspace(ifile.peek()))
    {
        if (ifile.eof() || ifile.fail())
            return;
        c = ifile.get();
        str += c;
    }
}

// ****************************************************************************
//  Method:  GetDirName
//
//  Purpose:
//    Returns the directory from a full path name
//
//  Arguments:
//    path       the full path name
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  7, 2003
//
// ****************************************************************************

string 
GetDirName(const char *path)
{
    string dir = "";

    int len = strlen(path);
    const char *last = path + (len-1);
    while (*last != '/' && last > path)
    {
        last--;
    }

    if (*last != '/')
    {
        return "";
    }

    char str[1024];
    strcpy(str, path);
    str[last-path+1] = '\0';

    return str;
}


// ****************************************************************************
//  Function: PointSorter
//
//  Purpose:
//      Sorts records of type PointWithId.  This is a routine that is then fed 
//      into qsort.  Its only value added is to provide the >, <, ==.
//
//  Arguments:
//      arg1    The first record.
//      arg2    The second record.
//
//  Returns:    <0 if arg1<arg2, 0 if arg1==arg2, >0 if arg2>arg1.
//
//  Programmer: Hank Childs
//  Creation:   September 15, 2004
//
// ****************************************************************************

int
PointSorter(const void *arg1, const void *arg2)
{
    const PointWithId *r1 = (const PointWithId *) arg1;
    const PointWithId *r2 = (const PointWithId *) arg2;

    if (r1->pt[0] < r2->pt[0])
        return -1;
    else if (r2->pt[0] < r1->pt[0])
        return 1;

    if (r1->pt[1] < r2->pt[1])
        return -1;
    else if (r2->pt[1] < r1->pt[1])
        return 1;

    if (r1->pt[2] < r2->pt[2])
        return -1;
    else if (r2->pt[2] < r1->pt[2])
        return 1;

    return 0;
}
