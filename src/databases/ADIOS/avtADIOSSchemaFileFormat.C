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
    fileObj = new ADIOSFileObject(nm);
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
    
    if (fileObj)
        delete fileObj;
    fileObj = NULL;
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
            //mesh->meshType = AVT_POINT_MESH;
            mesh->numBlocks = 1;
            mesh->blockOrigin = 0;
            mesh->spatialDimension = am->unstructured->nspaces;
            mesh->topologicalDimension = am->unstructured->nspaces;
            md->Add(mesh);
        }
    }

    std::map<std::string, ADIOS_VARINFO*>::const_iterator v;
    for (v = fileObj->variables.begin(); v != fileObj->variables.end(); v++)
    {
        string varNm = v->first;
        ADIOS_VARINFO *avi = v->second;
        ADIOS_MESH *am = fileObj->GetMeshInfo(avi);
        if (am == NULL)
            continue;

        avtScalarMetaData *smd = new avtScalarMetaData();
        smd->name = varNm;
        
        if (smd->name[0] == '/')
        {
            smd->originalName = smd->name;
            smd->name = string(&smd->name[1]);
        }

        smd->meshName = am->name;
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
    vtkDataArray *arr = NULL;
    fileObj->ReadScalarData(varname, timestate, &arr);
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
    if (!fileObj->Open())
        EXCEPTION0(ImproperUseException);
    if (initialized)
        return;

    bool firstIter = true;
    std::map<std::string, ADIOS_VARINFO*>::iterator vi;
    for (vi = fileObj->variables.begin(); vi != fileObj->variables.end(); ++vi)
    {
        ADIOS_VARINFO *avi = vi->second;
        if (avi->meshinfo == NULL)
            continue;
        
        ADIOS_MESH *am = fileObj->GetMeshInfo(avi);
        if (am == NULL)
            continue;

        if (meshes.find(am->name) == meshes.end())
            meshes[am->name] = am;

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
    }
    
    initialized = true;
}

//****************************************************************************
// Method:  avtADIOSSchemaFileFormat::MakeUniformMesh
//
// Purpose:
//   Construct a uniform mesh.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

vtkDataSet *
avtADIOSSchemaFileFormat::MakeUniformMesh(MESH_UNIFORM *m, int ts, int dom)
{
    vtkRectilinearGrid *rg = vtkRectilinearGrid::New();
    vtkFloatArray *xyz[3];
    for (int d = 0; d < 3; d++)
    {
        xyz[d] = vtkFloatArray::New();
        if (d < m->num_dimensions)
        {
            xyz[d]->SetNumberOfTuples(m->dimensions[d]);
            for (int i = 0; i < m->dimensions[d]; i++)
                xyz[d]->SetTuple1(i, m->origins[d] + i*m->spacings[d]);
        }
        else
        {
            xyz[d]->SetNumberOfTuples(1);
            xyz[d]->SetTuple1(0, 0.0f);
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
        dims[2] = m->dimensions[0];
        rg->SetXCoordinates(xyz[2]);
        rg->SetYCoordinates(xyz[1]);
        rg->SetZCoordinates(xyz[0]);
    }
    rg->SetDimensions(dims);

    xyz[0]->Delete();
    xyz[1]->Delete();
    xyz[2]->Delete();
    
    return rg;
}

//****************************************************************************
// Method:  avtADIOSSchemaFileFormat::MakeRectilinearMesh
//
// Purpose:
//   Construct a rectilinear mesh.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

vtkDataSet *
avtADIOSSchemaFileFormat::MakeRectilinearMesh(MESH_RECTILINEAR *m, int ts, int dom)
{
    vtkRectilinearGrid *rg = vtkRectilinearGrid::New();
    vtkDataArray *xyz[3] = {NULL, NULL, NULL};
    if (m->use_single_var == 1)
    {
        
    }
    else
    {
        for (int d = 0; d < 3; d++)
        {
            if (d < m->num_dimensions)
                fileObj->ReadScalarData(m->coordinates[d], ts, &(xyz[d]));
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
        dims[2] = m->dimensions[0];
        rg->SetXCoordinates(xyz[2]);
        rg->SetYCoordinates(xyz[1]);
        rg->SetZCoordinates(xyz[0]);
    }
    rg->SetDimensions(dims);

    xyz[0]->Delete();
    xyz[1]->Delete();
    xyz[2]->Delete();
    return rg;
}

//****************************************************************************
// Method:  avtADIOSSchemaFileFormat::MakeStructuredMesh
//
// Purpose:
//   Construct a structured mesh.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

vtkDataSet *
avtADIOSSchemaFileFormat::MakeStructuredMesh(MESH_STRUCTURED *m, int ts, int dom)
{
    vtkStructuredGrid *sg = vtkStructuredGrid::New();
    vtkPoints *pts = vtkPoints::New();
    int dims[3] = {1,1,1};

    if (m->num_dimensions == 1)
        dims[0] = m->dimensions[0];
    else if (m->num_dimensions == 2)
    {
        dims[0] = m->dimensions[1];
        dims[1] = m->dimensions[0];
    }
    else
    {
        dims[0] = m->dimensions[2];
        dims[1] = m->dimensions[1];
        dims[2] = m->dimensions[0];
    }

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
                fileObj->ReadScalarData(m->points[d], ts, &(xyz[d]));
            else
            {
                xyz[d] = vtkFloatArray::New();
                xyz[d]->SetNumberOfTuples(sz);
                for (int i = 0; i < sz; i++)
                    xyz[d]->SetTuple1(i, 0.0f);
            }
        }
    }

    pts->SetNumberOfPoints(sz);
    sg->SetDimensions(dims);

    int cnt = 0;
    for (int i = 0; i < dims[0]; i++)
        for (int j = 0; j < dims[1]; j++)
            for (int k = 0; k < dims[2]; k++)
            {
                int idx = cnt;
                pts->SetPoint(cnt,
                              xyz[0]->GetTuple1(idx),
                              xyz[1]->GetTuple1(idx),
                              xyz[2]->GetTuple1(idx));
                cnt++;
            }
    
    xyz[0]->Delete();
    xyz[1]->Delete();
    xyz[2]->Delete();

    sg->SetPoints(pts);
    pts->Delete();
    return sg;
}

//****************************************************************************
// Method:  avtADIOSSchemaFileFormat::MakeUnstructuredMesh
//
// Purpose:
//   Construct an unstructured mesh.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

vtkDataSet *
avtADIOSSchemaFileFormat::MakeUnstructuredMesh(MESH_UNSTRUCTURED *m, int ts, int dom)
{
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();

    //Read in the points.

    vtkPoints *pts = NULL;
    if (m->nvar_points == 1)
    {
        pts = fileObj->ReadCoordinates(m->points[0], ts, m->nspaces, m->npoints);
    }
    else
    {
        pts->SetNumberOfPoints(m->npoints);
        vtkDataArray *xyz[3] = {NULL, NULL, NULL};
        for (int d = 0; d < 3; d++)
        {
            if (d < m->nspaces)
                fileObj->ReadScalarData(m->points[d], ts, &(xyz[d]));
            else
            {
                xyz[d] = vtkFloatArray::New();
                xyz[d]->SetNumberOfTuples(m->npoints);
                for (int i = 0; i < m->npoints; i++)
                    xyz[d]->SetTuple1(i, 0.0f);
            }
        }
        for (int i = 0; i < m->npoints; i++)
            pts->SetPoint(i, xyz[0]->GetTuple1(i), xyz[1]->GetTuple1(i), xyz[2]->GetTuple1(i));
    }
    
    ugrid->SetPoints(pts);
    pts->Delete();

    
    //Read in the cells.
    for (int c = 0; c < m->ncsets; c++)
    {
        int nVerts = NumberOfVertices(m->ctypes[c]);
        ADIOS_CELL_TYPE cType = m->ctypes[c];
        
        vtkDataArray *cells = NULL;
        fileObj->ReadScalarData(m->cdata[c], ts, &cells);
        
        int cellType = GetCellType(m->ctypes[c]);
        vtkIdType *verts = new vtkIdType[nVerts];
        int idx = 0;
        for (int i = 0; i < m->ccounts[c]; i++)
        {
            for (int j = 0; j < nVerts; j++, idx++)
                verts[j] = cells->GetTuple1(idx);
            ugrid->InsertNextCell(cellType, nVerts, verts);
        }
        
        cells->Delete();
        delete [] verts;
    }

    return ugrid;
}

//****************************************************************************
// Method:  avtADIOSSchemaFileFormat::NumberOfVertices
//
// Purpose:
//   Cell type to number of vertices.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

int
avtADIOSSchemaFileFormat::NumberOfVertices(ADIOS_CELL_TYPE &ct)
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

//****************************************************************************
// Method:  avtADIOSSchemaFileFormat::GetCellType
//
// Purpose:
//   ADIOS cell type to VTK cell type.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

int
avtADIOSSchemaFileFormat::GetCellType(ADIOS_CELL_TYPE &ct)
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
