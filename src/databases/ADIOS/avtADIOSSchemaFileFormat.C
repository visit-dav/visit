/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
//                            avtADIOSFileFormat.C                           //
// ************************************************************************* //

#include <avtMTMDFileFormatInterface.h>
#include <avtParallel.h>
#include "ADIOSFileObject.h"
#include "avtADIOSSchemaFileFormat.h"
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <avtDatabaseMetaData.h>
#include <DBOptionsAttributes.h>
#include <InvalidDBTypeException.h>
#include <InvalidVariableException.h>
#include <DebugStream.h>
#include <avtDatabase.h>
#include <vtkUnstructuredGrid.h>
#include <vtkTriangle.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>

#include <vtkCharArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkShortArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkIntArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkLongArray.h>
#include <vtkUnsignedLongArray.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>

#include <set>
#include <string>
#include <vector>

extern "C"
{
#include <adios_read.h>
}

using namespace std;

static vtkDataArray *AllocateArray(ADIOS_DATATYPES &t, int nt, int nc=1);
static int NumberOfVertices(ADIOS_CELL_TYPE &ct);
static int GetCellType(ADIOS_CELL_TYPE &ct);

// ****************************************************************************
//  Method: avtADIOSSchemaFileFormat::Identify
//
//  Purpose:
//      Determine if this file is of this flavor.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

bool
avtADIOSSchemaFileFormat::Identify(const char *fname)
{
    ADIOS_FILE *fp;
#ifdef PARALLEL
    fp = adios_read_open_file(fname, ADIOS_READ_METHOD_BP, (MPI_Comm)VISIT_MPI_COMM);
#else
    MPI_Comm comm_dummy = 0;
    fp = adios_read_open_file(fname, ADIOS_READ_METHOD_BP, comm_dummy);
#endif

    cout<<"nmeshes= "<<fp->nmeshes<<endl;
    bool isSchema = false;
    for (int i = 0; i < fp->nvars && !isSchema; i++)
    {
        ADIOS_VARINFO *avi = adios_inq_var_byid(fp, i);
        if (adios_inq_var_meshinfo(fp, avi) != 0)
            continue;
        if (avi->meshinfo == NULL)
            continue;
        ADIOS_MESH *am = adios_inq_mesh_byid(fp, avi->meshinfo->meshid);
        if (am != NULL)
            isSchema = true;
        adios_free_varinfo(avi);
        adios_free_meshinfo(am);
    }

    adios_read_close(fp);

    return isSchema;
}


// ****************************************************************************
//  Method: avtADIOSSchemaFileFormat::CreateInterface
//
//  Purpose:
//      Create an interface for this reader.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

avtFileFormatInterface *
avtADIOSSchemaFileFormat::CreateInterface(const char *const *list,
                                      int nList,
                                      int nBlock)
{
    int nTimestepGroups = nList / nBlock;
    avtMTMDFileFormat **ffl = new avtMTMDFileFormat*[nTimestepGroups];
    for (int i = 0 ; i < nTimestepGroups ; i++)
        ffl[i] = new avtADIOSSchemaFileFormat(list[i*nBlock]);
    
    return new avtMTMDFileFormatInterface(ffl, nTimestepGroups);
}

// ****************************************************************************
//  Method: avtADIOSSchemaFileFormat constructor
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

avtADIOSSchemaFileFormat::avtADIOSSchemaFileFormat(const char *nm)
    : avtMTMDFileFormat(nm)
{
    fp = NULL;
    filename = nm;
    initialized = false;
    numTimes = 0;
}

// ****************************************************************************
//  Method: avtADIOSSchemaFileFormat destructor
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

avtADIOSSchemaFileFormat::~avtADIOSSchemaFileFormat()
{
    map<string, ADIOS_MESH *>::iterator mit;
    for (mit = meshes.begin(); mit != meshes.end(); mit++)
        adios_free_meshinfo(mit->second);
    map<string, ADIOS_VARINFO *>::iterator vit;
    for (vit = vars.begin(); vit != vars.end(); vit++)
        adios_free_varinfo(vit->second);
    meshes.clear();
    vars.clear();
    varMeshes.clear();
    
    if (fp)
        adios_read_close(fp);
    fp = NULL;
}


// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

int
avtADIOSSchemaFileFormat::GetNTimesteps()
{
    Initialize();
    return numTimes;
}

// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetCycles
//
//  Purpose:
//      
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

void
avtADIOSSchemaFileFormat::GetCycles(std::vector<int> &cycles)
{
    for (int i = 0; i < numTimes; i++)
        cycles.push_back(i);
}


// ****************************************************************************
// Method:  avtADIOSSchemaFileFormat::GetTimes
//
// Purpose:
//   
// Programmer:  Dave Pugmire
// Creation:    January 26, 2011
//
// ****************************************************************************

void
avtADIOSSchemaFileFormat::GetTimes(std::vector<double> &times)
{
    for (int i = 0; i < numTimes; i++)
        times.push_back(i);
}

// ****************************************************************************
//  Method: avtADIOSSchemaFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

void
avtADIOSSchemaFileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method: avtADIOSSchemaFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
//  Modifications:
//
//  Dave Pugmire, Wed Mar 24 16:43:32 EDT 2010
//  Add expressions.
//
//   Dave Pugmire, Thu Jan 27 11:39:46 EST 2011
//   Support for new Pixle file format.
//
// ****************************************************************************

void
avtADIOSSchemaFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    Initialize();

    map<string, ADIOS_MESH *>::const_iterator mit;
    for (mit = meshes.begin(); mit != meshes.end(); mit++)
    {
        ADIOS_MESH *am = mit->second;

        if (am->type == ADIOS_MESH_UNIFORM || am->type == ADIOS_MESH_RECTILINEAR)
        {
            avtMeshMetaData *mesh = new avtMeshMetaData;
            mesh->name = am->name;
            mesh->meshType = AVT_RECTILINEAR_MESH;
            mesh->numBlocks = PAR_Size();
            mesh->blockOrigin = 0;
            if (am->type == ADIOS_MESH_UNIFORM)
            {
                mesh->spatialDimension = am->uniform->num_dimensions;
                mesh->topologicalDimension = am->uniform->num_dimensions;
            }
            else
            {
                mesh->spatialDimension = am->rectilinear->num_dimensions;
                mesh->topologicalDimension = am->rectilinear->num_dimensions;
            }
            md->Add(mesh);
        }
        else if (am->type == ADIOS_MESH_STRUCTURED)
        {
            avtMeshMetaData *mesh = new avtMeshMetaData;
            mesh->name = am->name;
            mesh->meshType = AVT_CURVILINEAR_MESH;
            mesh->numBlocks = PAR_Size();
            mesh->blockOrigin = 0;
            mesh->spatialDimension = am->structured->num_dimensions;
            mesh->topologicalDimension = am->structured->num_dimensions;
            md->Add(mesh);
        }
        else if (am->type == ADIOS_MESH_UNSTRUCTURED)
        {
            avtMeshMetaData *mesh = new avtMeshMetaData;
            mesh->name = am->name;
            mesh->meshType = AVT_UNSTRUCTURED_MESH;
            mesh->numBlocks = PAR_Size();
            mesh->blockOrigin = 0;
            mesh->spatialDimension = am->unstructured->nspaces;
            mesh->topologicalDimension = am->unstructured->nspaces;
            md->Add(mesh);
        }
    }

    map<string, ADIOS_VARINFO *>::const_iterator vit;
    for (vit = vars.begin(); vit != vars.end(); vit++)
    {
        string varNm = vit->first;
        ADIOS_VARINFO *avi = vit->second;
        string meshNm = varMeshes.find(varNm)->second;
        ADIOS_MESH *m = meshes.find(meshNm)->second;
        
        avtScalarMetaData *smd = new avtScalarMetaData();
        smd->name = varNm;
        if (smd->name[0] == '/')
        {
            smd->originalName = smd->name;
            smd->name = string(&smd->name[1]);
        }

        smd->meshName = meshNm;
        if (avi->meshinfo->centering == point)
            smd->centering = AVT_NODECENT;
        else
            smd->centering = AVT_ZONECENT;
        md->Add(smd);
    }
}


// ****************************************************************************
//  Method: avtADIOSSchemaFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
//  Modifications
//   Dave Pugmire, Thu Jan 27 11:39:46 EST 2011
//   Support for new Pixle file format.
//
// ****************************************************************************

vtkDataSet *
avtADIOSSchemaFileFormat::GetMesh(int timestate, int domain, const char *meshname)
{
    Initialize();
    
    map<string,ADIOS_MESH *>::const_iterator it = meshes.find(meshname);
    ADIOS_MESH *am = it->second;
    if (am == NULL)
        return NULL;

    if (am->type == ADIOS_MESH_UNIFORM)
        return MakeUniformMesh(am->uniform, timestate, domain);
    else if (am->type == ADIOS_MESH_RECTILINEAR)
        return MakeRectilinearMesh(am->rectilinear, timestate, domain);
    else if (am->type == ADIOS_MESH_STRUCTURED)
        return MakeStructuredMesh(am->structured, timestate, domain);
    else if (am->type == ADIOS_MESH_UNSTRUCTURED)
        return MakeUnstructuredMesh(am->unstructured, timestate, domain);

    return NULL;
}


// ****************************************************************************
//  Method: avtADIOSSchemaFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      varname    The name of the variable requested.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
//  Modifications:
//
//  Dave Pugmire, Wed Mar 24 16:43:32 EDT 2010
//  Handle time varying variables correctly.
//
// ****************************************************************************

vtkDataArray *
avtADIOSSchemaFileFormat::GetVar(int timestate, int domain, const char *varname)
{
    Initialize();
    
    ADIOS_VARINFO *avi = vars.find(varname)->second;

    int sz = 1;
    for (int i = 0; i < avi->ndim; i++)
        sz *= avi->dims[i];
    
    //cout<<"GetVar "<<varname<<" sz= "<<sz<<" mesh= "<<varMeshes.find(varname)->second<<endl;
    vtkDataArray *arr = AllocateArray(avi->type, sz);
    ReadData(avi, timestate, arr->GetVoidPointer(0));

    return arr;
}


// ****************************************************************************
//  Method: avtSpecFEMFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      varname    The name of the variable requested.
//
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

vtkDataArray *
avtADIOSSchemaFileFormat::GetVectorVar(int timestate, int domain, const char *varname)
{
    return NULL;
}

// ****************************************************************************
//  Method: avtADIOSSchemaFileFormat::Initialize
//
//  Purpose:
//      Initialize the reader.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
//  Modifications:
//
//  Dave Pugmire, Wed Mar 24 16:43:32 EDT 2010
//  Read in expressions.
//
//   Dave Pugmire, Thu Jan 27 11:39:46 EST 2011
//   Support for new Pixle file format.
//
// ****************************************************************************

void
avtADIOSSchemaFileFormat::Initialize()
{
    if (initialized)
        return;

#ifdef PARALLEL
    fp = adios_read_open_file(filename.c_str(), ADIOS_READ_METHOD_BP, (MPI_Comm)VISIT_MPI_COMM);
#else
    MPI_Comm comm_dummy = 0;
    fp = adios_read_open_file(filename.c_str(), ADIOS_READ_METHOD_BP, comm_dummy);
#endif
    
    //Read vars.
    bool firstIter = true;
    for (int i = 0; i < fp->nvars; i++)
    {
        //cout<<i<<" "<<fp->var_namelist[i]<<endl;
        ADIOS_VARINFO *avi = adios_inq_var_byid(fp, i);
        if (adios_inq_var_meshinfo(fp, avi) != 0)
            continue;
        if (avi->meshinfo == NULL)
            continue;
        //cout<<"GET MESH, id= "<<avi->meshinfo->meshid<<endl;
        ADIOS_MESH *am = adios_inq_mesh_byid(fp, avi->meshinfo->meshid);
        if (am == NULL)
            continue;
        adios_inq_var_blockinfo(fp, avi);
        if (meshes.find(am->name) == meshes.end())
            meshes[am->name] = am;
        vars[fp->var_namelist[i]] = avi;
        varMeshes[fp->var_namelist[i]] = am->name;
        if (firstIter)
        {
            numTimes = avi->nsteps;
            firstIter = false;
        }
        else
        {
            if (numTimes != avi->nsteps)
                EXCEPTION1(InvalidVariableException, "Variable inconsitency");
        }
        //print some info
        //cout<<i<<" "<<fp->var_namelist[i]<<" mesh= "<<am->name;
        cout<<" dims: ";
        for (int i = 0; i < avi->ndim; i++)
            cout<<avi->dims[i]<<" ";
        cout<<endl;
        /*
        for (int i = 0; i < avi->nsteps; i++)
        {
            cout<<"Step: "<<i<<endl;
            for (int j = 0; j < avi->nblocks[i]; j++)
            {
                cout<<" Block: "<<j<<" ";
                cout<<"s: ";
                for (int k = 0; k < avi->ndim; k++)
                    cout<<avi->blockinfo[j].start[k]<<" ";
                cout<<"c: ";
                for (int k = 0; k < avi->ndim; k++)
                    cout<<avi->blockinfo[j].count[k]<<" ";
                cout<<endl;
            }
        }
        */
    }

    initialized = true;
}


vtkDataSet *
avtADIOSSchemaFileFormat::MakeUniformMesh(MESH_UNIFORM *m, int ts, int dom)
{
    vtkRectilinearGrid *rg = vtkRectilinearGrid::New();
    vtkFloatArray *xyz[3];
    for (int d = 0; d < 3; d++)
    {
        xyz[d] = vtkFloatArray::New();
        if (m->dimensions[d] == 0)
        {
            xyz[d]->SetNumberOfTuples(1);
            xyz[d]->SetTuple1(0, 0.0f);
        }
        else
        {
            xyz[d]->SetNumberOfTuples(m->dimensions[d]);
            for (int i = 0; i < m->dimensions[d]; i++)
                xyz[d]->SetTuple1(i, m->origins[d] + i*m->spacings[d]);
        }
    }
    
    int dims[3] = {1,1,1};
    if (m->num_dimensions == 1)
    {
        dims[0] = m->dimensions[0];
        rg->SetXCoordinates(xyz[0]);
        rg->SetYCoordinates(xyz[1]);
        rg->SetZCoordinates(xyz[2]);
    }
    else if (m->num_dimensions == 2)
    {
        dims[0] = m->dimensions[1];
        dims[1] = m->dimensions[0];
        rg->SetXCoordinates(xyz[1]);
        rg->SetYCoordinates(xyz[0]);
        rg->SetZCoordinates(xyz[2]);
    }
    else if (m->num_dimensions == 3)
    {
        dims[0] = m->dimensions[2];
        dims[1] = m->dimensions[1];
        dims[0] = m->dimensions[0];
        rg->SetXCoordinates(xyz[2]);
        rg->SetYCoordinates(xyz[1]);
        rg->SetZCoordinates(xyz[0]);
    }
    rg->SetDimensions(dims);
    //cout<<"Uniform Mesh dims: "<<dims[0]<<" "<<dims[1]<<" "<<dims[2]<<endl;

    xyz[0]->Delete();
    xyz[1]->Delete();
    xyz[2]->Delete();
    
    return rg;
}

vtkDataSet *
avtADIOSSchemaFileFormat::MakeRectilinearMesh(MESH_RECTILINEAR *m, int ts, int dom)
{
    vtkRectilinearGrid *rg = vtkRectilinearGrid::New();
    vtkDataArray *xyz[3];
    if (m->use_single_var == 1)
    {
        
    }
    else
    {
        for (int d = 0; d < 3; d++)
        {
            if (d < m->num_dimensions)
            {
                ADIOS_VARINFO *avi = adios_inq_var(fp, m->coordinates[d]);
                xyz[d] = AllocateArray(avi->type, m->dimensions[d]);
                ReadData(avi, ts, xyz[d]->GetVoidPointer(0));
            }
            else
            {
                xyz[d] = vtkFloatArray::New();
                xyz[d]->SetNumberOfTuples(1);
                xyz[d]->SetTuple1(0, 0.0f);
            }
        }
    }
    int dims[3] = {1,1,1};
    if (m->num_dimensions == 1)
    {
        dims[0] = m->dimensions[0];
        rg->SetXCoordinates(xyz[0]);
        rg->SetYCoordinates(xyz[1]);
        rg->SetZCoordinates(xyz[2]);
    }
    else if (m->num_dimensions == 2)
    {
        dims[0] = m->dimensions[1];
        dims[1] = m->dimensions[0];
        rg->SetXCoordinates(xyz[1]);
        rg->SetYCoordinates(xyz[0]);
        rg->SetZCoordinates(xyz[2]);
    }
    else if (m->num_dimensions == 3)
    {
        dims[0] = m->dimensions[2];
        dims[1] = m->dimensions[1];
        dims[0] = m->dimensions[0];
        rg->SetXCoordinates(xyz[2]);
        rg->SetYCoordinates(xyz[1]);
        rg->SetZCoordinates(xyz[0]);
    }
    //cout<<"Rect Mesh dims: "<<dims[0]<<" "<<dims[1]<<" "<<dims[2]<<endl;
    rg->SetDimensions(dims);

    xyz[0]->Delete();
    xyz[1]->Delete();
    xyz[2]->Delete();
    return rg;
}

vtkDataSet *
avtADIOSSchemaFileFormat::MakeStructuredMesh(MESH_STRUCTURED *m, int ts, int dom)
{
    vtkStructuredGrid *sg = vtkStructuredGrid::New();
    vtkPoints *pts = vtkPoints::New();
    int dims[3] = {1,1,1};
    for (int i = 0; i < m->num_dimensions; i++)
        dims[i] = m->dimensions[i];
    cout<<"STRUCTURED: dims= "<<dims[0]<<" "<<dims[1]<<" "<<dims[2]<<endl;
    
    int sz = dims[0]*dims[1]*dims[2];
    vtkDataArray *xyz[3];
    if (m->use_single_var == 1)
    {
    }
    else
    {
        for (int d = 0; d < 3; d++)
        {
            if (d < m->num_dimensions)
            {
                ADIOS_VARINFO *avi = adios_inq_var(fp, m->points[d]);
                xyz[d] = AllocateArray(avi->type, sz);
                ReadData(avi, ts, xyz[d]->GetVoidPointer(0));
            }
            else
            {
                xyz[d] = vtkFloatArray::New();
                xyz[d]->SetNumberOfTuples(sz);
                for (int i = 0; i < sz; i++)
                    xyz[d]->SetTuple1(i, 0.0f);
            }
        }
    }

    //DRP FIX THIS!!!!!
    pts->SetNumberOfPoints(sz);
    dims[0] = 129;
    dims[1] = 65;
    sg->SetDimensions(dims);

    int cnt = 0;
    for (int i = 0; i < dims[0]; i++)
        for (int j = 0; j < dims[1]; j++)
            for (int k = 0; k < dims[2]; k++)
            {
                int idx = cnt; //i*dims[1] + j;
                pts->SetPoint(cnt,
                              xyz[0]->GetTuple1(idx),
                              xyz[1]->GetTuple1(idx),
                              xyz[2]->GetTuple1(idx));
                cout<<cnt<<": "<<xyz[0]->GetTuple1(idx)<<" "<<xyz[1]->GetTuple1(idx)<<endl;
                cnt++;
            }
    cout<<"CNT= "<<cnt<<" sz= "<<sz<<endl;
                              
    xyz[0]->Delete();
    xyz[1]->Delete();
    xyz[2]->Delete();

    sg->SetPoints(pts);
    pts->Delete();


    cout<<__LINE__<<endl;

    return sg;
}

vtkDataSet *
avtADIOSSchemaFileFormat::MakeUnstructuredMesh(MESH_UNSTRUCTURED *m, int ts, int dom)
{
    cout<<__LINE__<<endl;
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();

    vtkDataArray *xyz[3];
    if (m->nvar_points == 1)
    {
        ADIOS_VARINFO *avi = adios_inq_var(fp, m->points[0]);
        vtkDataArray *p =  AllocateArray(avi->type, m->npoints, avi->ndim);
        ReadData(avi, ts, p->GetVoidPointer(0));

        for (int d = 0; d < 3; d++)
        {
            xyz[d] = AllocateArray(avi->type, m->npoints);
            if (d < avi->ndim)
                for (int i = 0; i < m->npoints; i++)
                    xyz[d]->SetTuple1(i, p->GetComponent(i, d));
            else
                for (int i = 0; i < m->npoints; i++)
                    xyz[d]->SetTuple1(i, 0.0);
        }
        p->Delete();
    }
    else
    {
        for (int d = 0; d < 3; d++)
        {
            if (d < m->nspaces)
            {
                ADIOS_VARINFO *avi = adios_inq_var(fp, m->points[d]);
                xyz[d] = AllocateArray(avi->type, m->npoints);
                ReadData(avi, ts, xyz[d]->GetVoidPointer(0));
            }
            else
            {
                xyz[d] = vtkFloatArray::New();
                xyz[d]->SetNumberOfTuples(m->npoints);
                for (int i = 0; i < m->npoints; i++)
                    xyz[d]->SetTuple1(i, 0.0f);
            }
        }
    }
    
    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(m->npoints);
    cout<<"nPts= "<<m->npoints<<endl;
    ugrid->SetPoints(pts);
    pts->Delete();

    for (int i = 0; i < m->npoints; i++)
    {
        pts->SetPoint(i, 
                      xyz[0]->GetTuple1(i),
                      xyz[1]->GetTuple1(i),
                      xyz[2]->GetTuple1(i));
        cout<<i<<": "<<xyz[0]->GetTuple1(i)<<" "<<xyz[1]->GetTuple1(i)<<" "<<xyz[2]->GetTuple1(i)<<endl;
    }

    for (int c = 0; c < m->ncsets; c++)
    {
        ADIOS_VARINFO *avi = adios_inq_var(fp, m->cdata[c]);
        cout<<"CELL TYPE = "<<m->ctypes[c]<<endl;
        //m->ctypes[c] = ADIOS_CELL_TRI;
        int nVerts = NumberOfVertices(m->ctypes[c]);
        vtkDataArray *cells = AllocateArray(avi->type, m->ccounts[c], nVerts);
        ReadData(avi, ts, cells->GetVoidPointer(0));
        ADIOS_CELL_TYPE cType = m->ctypes[c];
        
        int cellType = GetCellType(m->ctypes[c]);
        vtkIdType *verts = new vtkIdType[nVerts];
        cout<<"CELLS: "<<endl;
        for (int i = 0; i < m->ccounts[c]; i++)
        {
            for (int j = 0; j < nVerts; j++)
                verts[j] = cells->GetComponent(i, j);
            
            cout<<" "<<i<<" :"<<cellType<<" [";
            for (int j = 0; j < nVerts; j++)
                cout<<verts[j]<<" ";
            cout<<"]"<<endl;
            
            ugrid->InsertNextCell(cellType, nVerts, verts);
        }
        cells->Delete();
        delete [] verts;
    }

    return ugrid;
}

void
avtADIOSSchemaFileFormat::ReadData(ADIOS_VARINFO *avi, int ts, void *data)
{
    ADIOS_SELECTION *sel;
    uint64_t start[4] = {0,0,0,0}, count[4] = {0,0,0,0};
    for (int i = 0; i < avi->ndim; i++)
        count[i] = avi->dims[i];
    //cout<<"READS: "<<start[0]<<" "<<start[1]<<" "<<start[2]<<" "<<start[3]<<endl;
    //cout<<"READC: "<<count[0]<<" "<<count[1]<<" "<<count[2]<<" "<<count[3]<<endl;
    sel = adios_selection_boundingbox(avi->ndim, start, count);
    adios_schedule_read_byid(fp, sel, avi->varid, ts, 1, data);
    int retval = adios_perform_reads(fp, 1);
    adios_selection_delete(sel);
}


static vtkDataArray *
AllocateArray(ADIOS_DATATYPES &t, int nt, int nc)
{
    vtkDataArray *array = NULL;
    switch (t)
    {
      case adios_unsigned_byte:
        array = vtkCharArray::New();
        break;
      case adios_byte:
        array = vtkUnsignedCharArray::New();
        break;
      case adios_string:
        array = vtkCharArray::New();
        break;
        
      case adios_unsigned_short:
        array = vtkUnsignedShortArray::New();
        break;
      case adios_short:
        array = vtkShortArray::New();
        break;
        
      case adios_unsigned_integer:
        array = vtkUnsignedIntArray::New(); 
        break;
      case adios_integer:
        array = vtkIntArray::New(); 
        break;
        
      case adios_unsigned_long:
        array = vtkUnsignedLongArray::New(); 
        break;
      case adios_long:
        array = vtkLongArray::New(); 
        break;
        
      case adios_real:
        array = vtkFloatArray::New(); 
        break;
        
      case adios_double:
        array = vtkDoubleArray::New(); 
        break;
        
      case adios_long_double: // 16 bytes
      case adios_complex:     //  8 bytes
      case adios_double_complex: // 16 bytes
      default:
        std::string str = "Inavlid variable type";
        EXCEPTION1(InvalidVariableException, str);
        break;
    }

    array->SetNumberOfComponents(nc);
    array->SetNumberOfTuples(nt);
    return array;
}


static int
NumberOfVertices(ADIOS_CELL_TYPE &ct)
{
    if (ct == ADIOS_CELL_PT)
        return 1;
    else if (ct == ADIOS_CELL_LINE)
        return 2;
    else if (ct == ADIOS_CELL_TRI)
        return 3;
    else if (ct == ADIOS_CELL_QUAD)
        return 4;
    else if (ct == ADIOS_CELL_HEX)
        return 8;
    else if (ct == ADIOS_CELL_PRI)
        return 6;
    else if (ct == ADIOS_CELL_TET)
        return 4;
    else if (ct == ADIOS_CELL_PYR)
        return 5;
}

static int
GetCellType(ADIOS_CELL_TYPE &ct)
{
    if (ct == ADIOS_CELL_PT)
        return VTK_VERTEX;
    else if (ct == ADIOS_CELL_LINE)
        return VTK_LINE;
    else if (ct == ADIOS_CELL_TRI)
        return VTK_TRIANGLE;
    else if (ct == ADIOS_CELL_QUAD)
        return VTK_QUAD;
    else if (ct == ADIOS_CELL_HEX)
        return VTK_HEXAHEDRON;
    else if (ct == ADIOS_CELL_PRI)
        return VTK_WEDGE;
    else if (ct == ADIOS_CELL_TET)
        return VTK_TETRA;
    else if (ct == ADIOS_CELL_PYR)
        return VTK_PYRAMID;
}
