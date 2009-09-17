/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                            avtAdiosFileFormat.C                           //
// ************************************************************************* //

#include <avtAdiosFileFormat.h>

#include <string>
#include <vector>
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
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPointData.h>
#include <vtkVisItUtility.h>
#include <avtDatabaseMetaData.h>
#include <DBOptionsAttributes.h>
#include <Expression.h>
#include <InvalidDBTypeException.h>
#include <InvalidVariableException.h>
#include <DebugStream.h>
#include <avtParallel.h>
#include <avtDatabase.h>

#include <sys/types.h>
extern "C"
{
#include <adios_read.h>
#include <adios_types.h>
}

using     std::string;

static std::string
IntToString(int value)
{
    std::stringstream out;
    out << value;
    return out.str();
}


// ****************************************************************************
//  Method: avtAdiosFileFormat constructor
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

avtAdiosFileFormat::avtAdiosFileFormat(const char *nm)
    : avtMTMDFileFormat(nm)
{
    filename = nm;
    fileOpened = false;

    fh = -1;
    numTimeSteps = 0;
    ginfos = NULL;
    ghs = NULL;
}

// ****************************************************************************
//  Method: avtAdiosFileFormat destructor
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

avtAdiosFileFormat::~avtAdiosFileFormat()
{
    CloseFile();
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
avtAdiosFileFormat::GetNTimesteps(void)
{
    OpenFile();
    return numTimeSteps;
}

void
avtAdiosFileFormat::GetCycles(std::vector<int> &cycles)
{
    OpenFile();
    
    for(int i = finfo.tidx_start; i <= finfo.tidx_stop; ++i)
        cycles.push_back(i);
}


// ****************************************************************************
//  Method: avtAdiosFileFormat::FreeUpResources
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
avtAdiosFileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method: avtAdiosFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

void
avtAdiosFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    debug1 << "avtAdiosFileFormat::PopulateMetadata:  begin:" << endl;
    OpenFile();

    md->SetFormatCanDoDomainDecomposition(true);

    // Add 2D/3D mesh metadata
    map<string,meshInfo>::const_iterator m;
    for (m = meshes.begin(); m != meshes.end(); m++)
    {
        avtMeshMetaData *mesh = new avtMeshMetaData;
        mesh->name = m->first;
        mesh->meshType = AVT_RECTILINEAR_MESH;
        mesh->numBlocks = 1; // must be 1 for automatic decomposition.
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
    map<string,varInfo>::const_iterator v;
    for (v = variables.begin(); v != variables.end(); v++)
    {
        if (v->second.dim == 1) 
        { 
            // define as a curve
            avtCurveMetaData *curve = new avtCurveMetaData;
            curve->name = v->first;
            md->Add(curve);
            debug5<< "added metadata: curve " << v->first << endl;

        } 
        else
        {
            avtCentering cent = AVT_NODECENT;
            AddScalarVarToMetaData(md, v->first, v->second.meshName, cent);
            debug5 << "added metadata: var "<<v->first<<" on mesh "<<v->second.meshName<<endl;
        }
    }
}


// ****************************************************************************
//  Method: avtAdiosFileFormat::GetMesh
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
avtAdiosFileFormat::GetMesh(int timestate, int domain, const char *meshname)
{
    debug1 << "avtAdiosFileFormat::GetMesh " << meshname << endl;
    OpenFile();

    // Look it up in the mesh table.
    map<string,meshInfo>::const_iterator m = meshes.find(meshname);
    if (m != meshes.end())
    {
        vtkRectilinearGrid *grid = NULL;
        grid = CreateUniformGrid(m->second);
        grid->Register(NULL);
        
        return grid;
    }

    //It might be a curve.
    map<string,varInfo>::const_iterator v = variables.find(meshname);
    if (v != variables.end() && v->second.dim == 1)
    {
        vtkRectilinearGrid *grid = NULL;
        grid = CreateUniformGrid(v->second);

        vtkDataArray *vals = GetADIOSVar(timestate, v->first.c_str());
        vals->SetName(meshname);
        grid->GetPointData()->SetScalars(vals);
        vals->Delete();
        grid->Register(NULL);
        return grid;
    }
    
    debug1<<meshname<<" not found"<<endl;
    EXCEPTION1(InvalidVariableException, meshname);
}


// ****************************************************************************
//  Method: avtAdiosFileFormat::GetVar
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
avtAdiosFileFormat::GetVar(int timestate, int domain, const char *varname)
{
    debug1 << "avtAdiosFileFormat::GetVar " << varname << endl;
    OpenFile();

    vtkDataArray *vtkarray = GetADIOSVar(timestate, varname);
    return vtkarray;
}


// ****************************************************************************
//  Method: avtAdiosFileFormat::GetVectorVar
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
avtAdiosFileFormat::GetVectorVar(int timestate, int domain, const char *varname)
{
    OpenFile();
    return NULL;
}


// ****************************************************************************
//  Method: avtEMSTDFileFormat::OpenFile
//
//  Purpose:
//      Open the file.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

void
avtAdiosFileFormat::OpenFile()
{
    if (fileOpened)
        return;

    // INITIALIZE DATA MEMBERS
    int     gr, vr, i, j;             // loop vars
    int     status;
    int     vartype, ndims, dims[16]; // info about one variable 
    int     hastimesteps;             // variable is spread among timesteps in the file
    int mpiComm = 0;
#ifdef PARALLEL
    mpiComm = VISIT_MPI_COMM;
#endif

    // open the BP file
    status = adios_fopen (&fh, filename.c_str(), mpiComm);
    if (status != 0)
        EXCEPTION1(InvalidDBTypeException, "The file could not be opened");
    
    // get number of groups, variables, timesteps, and attributes 
    // all parameters are integers, 
    // besides the last parameter, which is an array of strings for holding the list of group names
    adios_init_fileinfo( &finfo, 1);
    adios_inq_file(fh, &finfo);

    numTimeSteps = finfo.tidx_stop - finfo.tidx_start + 1;
    
    debug5 << "ADIOS BP file: " << filename << endl;
    debug5 << "# of groups: " << finfo.groups_count << endl;
    debug5 << "# of variables: " << finfo.vars_count << endl;
    debug5 << "# of attributes:" << finfo.attrs_count << endl;
    debug5 << "time steps: " << finfo.tidx_start << "-" << finfo.tidx_stop << endl;


    meshes.clear();
    variables.clear();
    ginfos = (BP_GROUP_INFO *) malloc(finfo.groups_count * sizeof(BP_GROUP_INFO));
    if (ginfos == NULL)
        EXCEPTION1(InvalidDBTypeException, "The file could not be opened");
    
    ghs = (int64_t *) malloc (finfo.groups_count * sizeof(int64_t));
    if (ginfos == NULL)
        EXCEPTION1(InvalidDBTypeException, "The file could not be opened");

    for (int gr=0; gr<finfo.groups_count; gr++)
    {
        debug5 <<  "  group " << finfo.group_namelist[gr] << ":" << endl;
        adios_gopen(fh, &(ghs[gr]), finfo.group_namelist[gr]);
        adios_init_groupinfo( &(ginfos[gr]), 1);
        // get variable info from group
        adios_inq_group (ghs[gr], &(ginfos[gr]));
        
        for (int vr=0; vr<ginfos[gr].vars_count; vr++)
        {
            adios_inq_var(ghs[gr], ginfos[gr].var_namelist[vr], &vartype, &ndims, &hastimesteps, dims);
            if (ndims == 0 || ndims > 3)
            {
                debug5<<"Skipping variable"<<ginfos[gr].var_namelist[vr]<<" of dimension: "<<ndims<<endl;
                continue;
            }
            
            // add variable to map, map id = variable path without the '/' in the beginning
            varInfo vi;
            vi.name = ginfos[gr].var_namelist[vr]+1;
            vi.dim = ndims;
            
            for (int i=0; i<3; i++)
            {
                vi.start[i] = 0;
                vi.count[i] = 1;
                vi.global[i] = 1;
                
                if (i<ndims)
                    vi.count[i] = vi.global[i] = dims[i];
            }

            vi.type = vartype;
            vi.timeVarying = hastimesteps;
            vi.groupIdx = gr;

            // Define a mesh for this variable's dimensions
            // name is like mesh_23x59, mesh_100x200x300
            string meshname = "mesh_";
            for (int i=0; i<ndims; i++)
            {
                meshname += IntToString(dims[i]);
                if (i<ndims-1)
                    meshname += "x";
            }
            vi.meshName = meshname;

            //Add mesh, if not found...
            if (meshes.find(meshname) == meshes.end())
            {
                meshInfo mi;
                for (int i=0; i<3; i++)
                {
                    mi.start[i] = vi.start[i];
                    mi.count[i] = vi.count[i];
                    mi.global[i] = vi.global[i];
                }
                mi.dim = vi.dim;
                mi.name = meshname;
                meshes[meshname] = mi;
                
                debug5 << "  added mesh " << meshname << endl;
            }

            // add variable to map
            variables[vi.name] = vi;
            debug5 << "  added variable " << vi.name<<" with mesh "<<vi.meshName<<endl;
            
        } // end var
    } // end group

    DoDomainDecomposition();
    
    fileOpened = true;
}

// ****************************************************************************
//  Method: avtAdiosFileFormat::CloseFile
//
//  Purpose:
//      Open the file.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

void
avtAdiosFileFormat::CloseFile()
{
    if (!fileOpened)
        return;
    
    if (ginfos)
        free(ginfos);
    if (ghs)
        free(ghs);
    adios_fclose(fh);
    meshes.clear();
    variables.clear();

    fileOpened = false;
}

// ****************************************************************************
//  Method: avtAdiosFileFormat::DoDomainDecomposition
//
//  Purpose:
//      Decompose across processor set.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

void
avtAdiosFileFormat::DoDomainDecomposition()
{
    map<string,meshInfo>::iterator m;
    for (m = meshes.begin(); m != meshes.end(); m++)
    {
#ifdef PARALLEL
        int zoneDims[3] = {1,1,1};
        for (int i = 0; i < 3; i++)
            zoneDims[i] = m->second.global[i];

        // Compute the zone-oriented domain decomposition
        int domCount[3] = {0, 0, 0};
        avtDatabase::ComputeRectilinearDecomposition(m->second.dim, PAR_Size(),
                                                     zoneDims[0], zoneDims[1], zoneDims[2],
                                                     &domCount[0], &domCount[1], &domCount[2]);
        
        // Determine this processor's logical domain (e.g. domain ijk) indices
        int domLogicalCoords[3] = {0, 0, 0};
        avtDatabase::ComputeDomainLogicalCoords(m->second.dim, domCount, PAR_Rank(),
                                                domLogicalCoords);

        debug5<<m->second.name<<" "<<zoneDims[0]<<" "<<zoneDims[1]<<" "<<zoneDims[2]<<" ";
        debug5 << "Processor " << PAR_Rank() << " DLC:["
               << domLogicalCoords[0] << ", " << domLogicalCoords[1] << ", "
               << domLogicalCoords[2] <<"] ";
        
        // Compute domain bounds.
        for (int i = 0; i < 3; i++)
        {
            avtDatabase::ComputeDomainBounds(zoneDims[i], domCount[i], domLogicalCoords[i],
                                             &(m->second.start[i]),
                                             &(m->second.count[i]));
            
            m->second.count[i]++;
            if (m->second.start[i]+m->second.count[i] >= m->second.global[i])
                m->second.count[i]--;
            
        }
#endif

        debug5 << "Domain offsets: [ ";
        for (int i = 0; i<3; i++)
            debug5<<"("<<m->second.start[i]<<"+"<<m->second.count[i]<<"), ";
        debug5<<"]"<<endl;
            
    }
}

// ****************************************************************************
//  Method: avtAdiosFileFormat::GetADIOSVar
//
//  Purpose:
//      Read in a variable from the file and return as vtkDataArray.
//      Used by GetVar() and GetMesh() (latter to read a 1D variable).
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      varname    The name of the variable requested.
//      groupidx   The group id of the variable (stored in VarInfo)
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

vtkDataArray *
avtAdiosFileFormat::GetADIOSVar(int timestate, const char *varname)
{
    debug5<<"avtAdiosFileFormat::GetADIOSVar "<<varname<<endl;
    
    map<string,varInfo>::const_iterator v = variables.find(varname);
    if (v == variables.end())
        EXCEPTION1(InvalidVariableException, varname);
    
    map<string,meshInfo>::const_iterator m = meshes.find(v->second.meshName);
    if (m == meshes.end())
        EXCEPTION1(InvalidVariableException, varname);

    int tupleSz;
    vtkDataArray *adiosArr = NULL;
    // determine tuple size and VTK array type from adios data type
    switch(v->second.type)
    {
        case adios_unsigned_byte:
             tupleSz = 1;
             adiosArr = vtkCharArray::New(); 
             break;
        case adios_byte:
             tupleSz = 1;
             adiosArr = vtkUnsignedCharArray::New(); 
             break;
        case adios_string:
             tupleSz = 1;
             adiosArr = vtkCharArray::New(); 
             break;
               
        case adios_unsigned_short:
             tupleSz = 2;
             adiosArr = vtkUnsignedShortArray::New(); 
             break;
        case adios_short:
             tupleSz = 2;
             adiosArr = vtkShortArray::New(); 
             break;
 
        case adios_unsigned_integer:
             tupleSz = 4;
             adiosArr = vtkUnsignedIntArray::New(); 
             break;
        case adios_integer:
             tupleSz = 4;
             adiosArr = vtkIntArray::New(); 
             break;
    
        case adios_unsigned_long:
             tupleSz = 8;
             adiosArr = vtkUnsignedLongArray::New(); 
             break;
        case adios_long:
             tupleSz = 8;
             adiosArr = vtkLongArray::New(); 
             break;
    
        case adios_real:
             tupleSz = 4;
             adiosArr = vtkFloatArray::New(); 
             break;

        case adios_double:
             tupleSz = 8;
             adiosArr = vtkDoubleArray::New(); 
             break;
             
        case adios_long_double: // 16 bytes
        case adios_complex:     //  8 bytes
        case adios_double_complex: // 16 bytes
        default:
          EXCEPTION1(InvalidVariableException, varname);
          break;
    }

    //Get the offsets....
    int ntuples = 1;
    int start[3] = {0,0,0}, count[3] = {1,1,1};

    for (int i=0; i<3; i++)
    {
        ntuples *= (m->second.count[i]);
        start[i] = m->second.start[i];
        count[i] = m->second.count[i];
    }
    
    adiosArr->SetNumberOfTuples(ntuples);
    void *data = adiosArr->GetVoidPointer(0);

    if (adios_get_var(ghs[v->second.groupIdx], (char*)varname, data, start, count, finfo.tidx_start+timestate) < 0)
        EXCEPTION1(InvalidVariableException, varname);
    
    debug5<<"adios_get_var ["<<start[0]<<"+"<<count[0]<<", "<<start[1]<<"+"<<count[1]<<", "<<start[2]<<"+"<<count[2]<<"] nt= "<<ntuples<<endl;

    //We need to transpose the array.
    vtkDataArray *arr = adiosArr->NewInstance();
    arr->SetNumberOfComponents(adiosArr->GetNumberOfComponents());
    arr->SetNumberOfTuples(adiosArr->GetNumberOfTuples());
    arr->SetName(adiosArr->GetName());

    int destIdx = 0;
    for(int k = 0; k < count[2]; k++)
        for(int j = 0; j < count[1]; j++)
            for(int i = 0; i < count[0]; i++)
            {
                int srcIdx = k + count[2] * (j + count[1] * i);
                arr->SetTuple(destIdx, adiosArr->GetTuple(srcIdx));
                destIdx++;
            }
    
    adiosArr->Delete();

    return arr;
}


// ****************************************************************************
//  Method: avtAdiosFileFormat::CreateUniformGrid
//
//  Purpose: 
//      Make a 0..x-1,0..y-1,0..z-1 uniform grid with stepping 1
//      Used for both 2D/3D variables (mesh) and 1D variables (curve)
//      y/z must be 1 at least since vtkRectilinearGrid is always a 3D structure
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

vtkRectilinearGrid *
avtAdiosFileFormat::CreateUniformGrid(const meshInfo &mi)
{
    vtkRectilinearGrid *grid = vtkRectilinearGrid::New();
    vtkFloatArray *coords[3] = {NULL,NULL,NULL};
    
    int dims[3] = {1,1,1};
    for (int i = 0; i<3; i++)
    {
        dims[i] = mi.count[i];

        coords[i] = vtkFloatArray::New();
        coords[i]->SetNumberOfTuples(dims[i]);
        float *data = (float *)coords[i]->GetVoidPointer(0);

        int x = mi.start[i];

        for (int j = 0; j < mi.count[i]; j++, x++)
            data[j] = (float)x;
    }
    grid->SetDimensions(dims);
    grid->SetXCoordinates(coords[0]);
    grid->SetYCoordinates(coords[1]);
    grid->SetZCoordinates(coords[2]);
    
    for (int i = 0; i<3; i++)
        coords[i]->Delete();

    debug1 <<"  create mesh: " << coords[0]->GetNumberOfTuples() << "x"
           <<coords[1]->GetNumberOfTuples() << "x"
           <<coords[2]->GetNumberOfTuples() << endl;

    return grid;
}
