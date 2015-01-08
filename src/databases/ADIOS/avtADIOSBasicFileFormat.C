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
#include <ADIOSFileObject.h>
#include <avtADIOSBasicFileFormat.h>
#include <vtkRectilinearGrid.h>
#include <vtkFloatArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPointData.h>
#include <vtkIdList.h>
#include <vtkCellData.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <avtDatabaseMetaData.h>
#include <DBOptionsAttributes.h>
#include <InvalidDBTypeException.h>
#include <InvalidVariableException.h>
#include <DebugStream.h>
#include <avtDatabase.h>
#include <sys/types.h>

#include <map>
#include <string>
#include <vector>

using namespace std;

// ****************************************************************************
//  Method: avtEMSTDFileFormat::Identify
//
//  Purpose:
//      Determine if this file is of this flavor.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Feb 10 15:55:14 EST 2010
//
// ****************************************************************************

bool
avtADIOSBasicFileFormat::Identify(const char *fname)
{
    //Any bp file is a basic file format.
    return true;
}


// ****************************************************************************
//  Method: avtEMSTDFileFormat::CreateInterface
//
//  Purpose:
//      Create an interface for this reader.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Feb 10 15:55:14 EST 2010
//
// ****************************************************************************

avtFileFormatInterface *
avtADIOSBasicFileFormat::CreateInterface(const char *const *llist,
                                         int nList,
                                         int nBlock)
{
    int nTimestepGroups = nList / nBlock;
    avtMTMDFileFormat **ffl = new avtMTMDFileFormat*[nTimestepGroups];
    for (int i = 0 ; i < nTimestepGroups ; i++)
    {
        ffl[i] = new avtADIOSBasicFileFormat(llist[i*nBlock]);
    }
    
    return new avtMTMDFileFormatInterface(ffl, nTimestepGroups);
}

// ****************************************************************************
//  Method: avtADIOSBasicFileFormat constructor
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

avtADIOSBasicFileFormat::avtADIOSBasicFileFormat(const char *nm)
    : avtMTMDFileFormat(nm)
{
    fileObj = new ADIOSFileObject(nm);
    initialized = false;
}

// ****************************************************************************
//  Method: avtADIOSBasicFileFormat destructor
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

avtADIOSBasicFileFormat::~avtADIOSBasicFileFormat()
{
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
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

int
avtADIOSBasicFileFormat::GetNTimesteps()
{
    return fileObj->NumTimeSteps();
}

// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetCycles
//
//  Purpose:
//      
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

void
avtADIOSBasicFileFormat::GetCycles(std::vector<int> &cycles)
{
    int nt = fileObj->NumTimeSteps();
    cycles.resize(nt);
    for (int i = 0; i < nt; i++)
        cycles[i] = i;
}


// ****************************************************************************
//  Method: avtADIOSBasicFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

void
avtADIOSBasicFileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method: avtADIOSBasicFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
//  Modifications:
//   Dave Pugmire, Tue Mar  9 12:40:15 EST 2010
//   Use name/originalName for names starting with /.
//
// ****************************************************************************

void
avtADIOSBasicFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    debug5<<"avtADIOSBasicFileFormat::PopulateDatabaseMetaData()"<<endl;
    Initialize();
    //md->SetFormatCanDoDomainDecomposition(true);

    int nBlocks = 1;
    std::map<std::string, meshInfo>::const_iterator m;
    for (m = meshes.begin(); m != meshes.end(); m++)
    {
        int n = m->second.blocks.size();
        if (n > nBlocks)
            nBlocks = n;
    }
    
    // Add 2D/3D mesh metadata
    for (m = meshes.begin(); m != meshes.end(); m++)
    {
        avtMeshMetaData *mesh = new avtMeshMetaData;
        mesh->name = m->first;
        mesh->meshType = AVT_RECTILINEAR_MESH;
        mesh->numBlocks = nBlocks;
        mesh->blockOrigin = 0;
        mesh->spatialDimension = m->second.dim;
        mesh->topologicalDimension = m->second.dim;
        mesh->blockTitle = "blocks";
        mesh->blockPieceName = "block";
        mesh->hasSpatialExtents = false;
        md->Add(mesh);
        
        debug5 <<"added metadata: mesh "<<m->first<<endl;
    }

    // Add variables' metadata
    std::map<std::string, ADIOS_VARINFO*>::const_iterator v;
    for (v = fileObj->variables.begin(); v != fileObj->variables.end(); v++)
    {
        if (v->second->ndim == 1)
        { 
            // define as a curve
            avtCurveMetaData *curve = new avtCurveMetaData;
            curve->name = v->first;
            if (curve->name[0] == '/')
            {
                curve->originalName = curve->name;
                curve->name = std::string(&curve->name[1]);
            }
            md->Add(curve);
            debug5<< "added metadata: curve " << v->first << endl;

        } 
        else
        {
            std::string meshName = GenerateMeshName(v->second);
            string originalName = v->first, name = v->first;
            if (name[0] == '/')
                name = string(&name[1]);
            
            //Complex variables...
            if (v->second->type == adios_complex ||
                v->second->type == adios_double_complex)
            {
                avtScalarMetaData *smd_r = new avtScalarMetaData();

                smd_r->originalName = originalName + string("_real");
                smd_r->name = name + string("_real");
                smd_r->meshName = meshName;
                smd_r->centering = AVT_ZONECENT;
                md->Add(smd_r);

                avtScalarMetaData *smd_i = new avtScalarMetaData();
                smd_i->originalName = originalName + string("_imag");
                smd_i->name = name + string("_imag");
                smd_i->meshName = meshName;
                smd_i->centering = AVT_ZONECENT;
                md->Add(smd_i);

                //name: psi_real
                //orig: /psi_real
                //map: /psi_real -> /psi
                complexVarMap[smd_r->originalName] = v->first;
                complexVarMap[smd_i->originalName] = v->first;
            }
            else
            {
                avtScalarMetaData *smd = new avtScalarMetaData();
                smd->name = name;
                smd->originalName = originalName;
                smd->meshName = meshName;
                smd->centering = AVT_ZONECENT;
                md->Add(smd);
            }
            debug5 << "added metadata: var "<<v->first<<" on mesh "<<meshName<<endl;
        }
    }
}


// ****************************************************************************
//  Method: avtADIOSBasicFileFormat::GetMesh
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
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

vtkDataSet *
avtADIOSBasicFileFormat::GetMesh(int timestate, int domain, const char *meshname)
{
    //debug1 << "avtADIOSBasicFileFormat::GetMesh " << meshname << endl;
    Initialize();

    // Look it up in the mesh table.
    std::map<std::string,meshInfo>::iterator m = meshes.find(meshname);
    if (m != meshes.end())
    {
        vtkRectilinearGrid *grid = NULL;
        grid = CreateUniformGrid(m->second, timestate, domain);
        if (grid)
            grid->Register(NULL);
        return grid;
    }
}


// ****************************************************************************
//  Method: avtADIOSBasicFileFormat::GetVar
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
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

vtkDataArray *
avtADIOSBasicFileFormat::GetVar(int timestate, int domain, const char *vn)
{
    string varname = vn;
    debug1 << "avtADIOSBasicFileFormat::GetVar " << varname << endl;
    Initialize();

    //See if we have a complex variable, and get the complex varname in the file.
    map<string,string>::const_iterator ci = complexVarMap.find(varname);
    if (ci != complexVarMap.end())
        varname = ci->second;

    std::map<std::string, ADIOS_VARINFO*>::iterator vi = fileObj->variables.find(varname);
    if (vi == fileObj->variables.end())
        EXCEPTION1(InvalidVariableException, varname);
    
    //Get the domain bounds.
    ADIOS_VARINFO *avi = vi->second;
    uint64_t s[3], c[3];
    bool gFlags[6];
    GetDomBounds(avi->sum_nblocks, avi->ndim, avi->dims,
                 avi->blockinfo[domain].start, avi->blockinfo[domain].count,
                 s, c, gFlags);

    ADIOS_SELECTION *sel = adios_selection_boundingbox(avi->ndim, s, c);
    
    //Read the variable...
    vtkDataArray *arr = NULL;
    if (avi->type == adios_complex)
    {
        string nm = ci->first;
        if (nm.substr(nm.length()-5, 5) == "_real")
        {
            if (!fileObj->ReadComplexRealData(nm, timestate, sel, &arr))
                EXCEPTION1(InvalidVariableException, varname);
        }
        else if (nm.substr(nm.length()-5, 5) == "_imag")
        {
            if (!fileObj->ReadComplexImagData(nm, timestate, sel, &arr))
                EXCEPTION1(InvalidVariableException, varname);
        }
    }
    else
        if (!fileObj->ReadScalarData(varname, timestate, sel, &arr))
            EXCEPTION1(InvalidVariableException, varname);
    adios_selection_delete(sel);
    
    return arr;
}


// ****************************************************************************
//  Method: avtADIOSBasicFileFormat::GetVectorVar
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
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

vtkDataArray *
avtADIOSBasicFileFormat::GetVectorVar(int timestate, int domain, const char *varname)
{
    return NULL;
}

// ****************************************************************************
//  Method: avtADIOSBasicFileFormat::GenerateMeshName
//
//  Purpose:
//      Create a generic mesh name from the dimensions of the variable.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Feb 10 16:08:01 EST 2010
//
// ****************************************************************************

std::string
avtADIOSBasicFileFormat::GenerateMeshName(const ADIOS_VARINFO *avi)
{
    std::vector<int64_t> dimT, dims;
    std::string meshname = "mesh_";

    for (int i=0; i<avi->ndim; i++)
        dims.insert(dims.begin(), avi->dims[i]);

    for (int i=0; i <dims.size(); i++)
    {
        std::stringstream ss;
        ss<<dims[i];
        meshname += ss.str();
        if (i<dims.size()-1)
            meshname += "x";
    }
    return meshname;
}

// ****************************************************************************
//  Method: avtADIOSBasicFileFormat::Initialize
//
//  Purpose:
//      Initialize the reader.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

void
avtADIOSBasicFileFormat::Initialize()
{
    if (!fileObj->Open())
        EXCEPTION0(ImproperUseException);

    if (initialized)
        return;

    //Create meshes for each variable.
    std::map<std::string, ADIOS_VARINFO*>::iterator vi;
    for (vi = fileObj->variables.begin(); vi != fileObj->variables.end(); ++vi)
    {
        ADIOS_VARINFO *avi = vi->second;
        std::string meshname = GenerateMeshName(avi);

        //Add mesh, if not found...
        if (meshes.find(meshname) == meshes.end())
            meshes[meshname] = meshInfo(avi);
    }
    
    initialized = true;
}

// ****************************************************************************
//  Method: avtADIOSBasicFileFormat::CreateUniformGrid
//
//  Purpose: 
//      Make a 0..x-1,0..y-1,0..z-1 uniform grid with stepping 1
//      Used for both 2D/3D variables (mesh) and 1D variables (curve)
//      y/z must be 1 at least since vtkRectilinearGrid is always a 3D structure
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
//  Modifications:
//   Dave Pugmire, Tue Mar  9 12:40:15 EST 2010
//   Use uint64_t for start/count arrays.
//
// ****************************************************************************

vtkRectilinearGrid *
avtADIOSBasicFileFormat::CreateUniformGrid(meshInfo &mi, int ts, int dom)
{
    if (dom > mi.blocks.size()-1)
        return NULL;
    
    uint64_t sIn[3]={0,0,0}, cIn[3]={0,0,0}, dims[3]={1,1,1}, s[3], c[3];
    bool g[6];
    block &b = mi.blocks[dom];
    for (int i = 0; i < 3; i++)
        if (i < mi.dim)
        {
            sIn[i] = b.start[i];
            cIn[i] = b.count[i];
            dims[i] = mi.dims[i];
        }
    
    //ADIOS is fortran ordering, so swap the mesh dims.
    if (mi.dim == 2)
    {
        std::swap(sIn[0], sIn[1]);
        std::swap(cIn[0], cIn[1]);
        std::swap(dims[0], dims[1]);
    }
    else
    {
        std::swap(sIn[0], sIn[2]);
        std::swap(cIn[0], cIn[2]);
        std::swap(dims[0], dims[2]);
    }

    bool doGhosts = GetDomBounds(mi.blocks.size(), mi.dim, dims,
                                 sIn, cIn, s, c, g);
    vtkRectilinearGrid *grid = vtkRectilinearGrid::New();
    vtkFloatArray *coords[3] = {NULL,NULL,NULL};

    int d[3] = {1,1,1};
    for (int i = 0; i < mi.dim; i++)
        d[i] = c[i]+1;
    grid->SetDimensions(d);

    for (int i = 0; i < 3; i++)
    {
        coords[i] = vtkFloatArray::New();
        if (i < mi.dim)
        {
            coords[i]->SetNumberOfTuples(d[i]);
            float *data = (float *)coords[i]->GetVoidPointer(0);
            int x = s[i];
            for (int j = 0; j < d[i]; j++, x++)
                data[j] = (float)x;
        }
        else
        {
            coords[i]->SetNumberOfTuples(1);
            coords[i]->SetTuple1(0, 0.0);
        }
    }
    grid->SetXCoordinates(coords[0]);
    grid->SetYCoordinates(coords[1]);
    grid->SetZCoordinates(coords[2]);
    for (int i = 0; i<3; i++)
        coords[i]->Delete();

    if (doGhosts)
        AddGhostZones(grid, g);
    return grid;
}

//****************************************************************************
// Method:  avtADIOSBasicFileFormat::GetDomBounds
//
// Purpose:
//   Get start/count indices, including ghost values.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

bool
avtADIOSBasicFileFormat::GetDomBounds(int nBlocks, int dim, uint64_t *dims,
                                      uint64_t *sIn, uint64_t *cIn,
                                      uint64_t *sOut, uint64_t *cOut, bool *gFlags)
{
    for (int i = 0; i < 3; i++)
    {
        sOut[i] = 0;
        cOut[i] = 0;
        gFlags[i*2+0] = false;
        gFlags[i*2+1] = false;
        if (i < dim)
        {
            sOut[i] = sIn[i];
            cOut[i] = cIn[i];//+1;
        }
    }

    if (nBlocks == 1)
        return false;
    
    for (int i = 0; i < dim; i++)
    {
        if (sOut[i] > 0)
            gFlags[i*2 +0] = true;
        if (sOut[i]+cOut[i] < dims[i])
            gFlags[i*2+1] = true;
        if (gFlags[i*2+0])
        {
            sOut[i]--;
            cOut[i]++;
        }
        if (gFlags[i*2+1])
            cOut[i]++;
    }
    
    //cout<<"GDB: sI: "<<sIn[0]<<" "<<sIn[1]<<" cI: "<<cIn[0]<<" "<<cIn[1]<<" --> ("<<sOut[0]<<" "<<sOut[1]<<") ("<<cOut[0]<<" "<<cOut[1]<<")"<<endl;
    
    return true;
}

//****************************************************************************
// Method:  avtADIOSBasicFileFormat::AddGhostZones
//
// Purpose:
//   Add ghost zones to a rectilinear grid.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

void
avtADIOSBasicFileFormat::AddGhostZones(vtkRectilinearGrid *grid, bool *g)
{
    //Add ghost zones.
    int ncells = grid->GetNumberOfCells();
    vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
    ghostCells->SetName("avtGhostZones");
    ghostCells->SetNumberOfTuples(ncells);
    unsigned char realVal = 0, ghostVal = 0;
    avtGhostData::AddGhostZoneType(ghostVal,
                                   DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);

    int nNodes[3];
    grid->GetDimensions(nNodes);
    for (int i = 0; i < 3; i++)
        if (nNodes[i] > 1)
            nNodes[i]--;
    
    int index = 0;
    for (int k = 0; k < nNodes[2]; k++)
        for (int j = 0; j < nNodes[1]; j++)
            for (int i = 0; i < nNodes[0]; i++)
            {
                if ((i == 0 && g[0*2 +0]) ||
                    (j == 0 && g[1*2 +0]) ||
                    (k == 0 && g[2*2 +0]) ||
                    (i == nNodes[0]-1 && g[0*2 +1]) ||
                    (j == nNodes[1]-1 && g[1*2 +1]) ||
                    (k == nNodes[2]-1 && g[2*2 +1]))
                {
                    ghostCells->SetTuple1(index, ghostVal);
                }
                else
                    ghostCells->SetTuple1(index, realVal);
                index++;
            }
    
    grid->GetCellData()->AddArray(ghostCells);
    vtkStreamingDemandDrivenPipeline::SetUpdateGhostLevel(grid->GetInformation(), 0);
    ghostCells->Delete();
}
