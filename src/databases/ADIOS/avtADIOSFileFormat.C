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
//                            avtADIOSFileFormat.C                           //
// ************************************************************************* //

#include <avtParallel.h>
#include <avtADIOSFileFormat.h>

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
Int64ToString(int64_t value)
{
    std::stringstream out;
    out << value;
    return out.str();
}

#define MIN(x,y) (x < y ? x : y)


// ****************************************************************************
//  Method: avtADIOSFileFormat constructor
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

avtADIOSFileFormat::avtADIOSFileFormat(const char *nm)
    : avtMTMDFileFormat(nm)
{
    filename = nm;
    fileOpened = false;

    fp = NULL;
    numTimeSteps = 0;
    gps = NULL;
}

// ****************************************************************************
//  Method: avtADIOSFileFormat destructor
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

avtADIOSFileFormat::~avtADIOSFileFormat()
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
avtADIOSFileFormat::GetNTimesteps(void)
{
    OpenFile();
    return numTimeSteps;
}

void
avtADIOSFileFormat::GetCycles(std::vector<int> &cycles)
{
    OpenFile();
    
    for(int i = 0; i < numTimeSteps; ++i)
        cycles.push_back(fp->tidx_start + i);
}


// ****************************************************************************
//  Method: avtADIOSFileFormat::FreeUpResources
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
avtADIOSFileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method: avtADIOSFileFormat::PopulateDatabaseMetaData
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
avtADIOSFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    debug1 << "avtADIOSFileFormat::PopulateMetadata:  begin:" << endl;
    OpenFile();

    md->SetFormatCanDoDomainDecomposition(true);

    // Add 2D/3D mesh metadata
    std::map<std::string,meshInfo>::const_iterator m;
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
    std::map<std::string,varInfo>::const_iterator v;
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
//  Method: avtADIOSFileFormat::GetMesh
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
avtADIOSFileFormat::GetMesh(int timestate, int domain, const char *meshname)
{
    debug1 << "avtADIOSFileFormat::GetMesh " << meshname << endl;
    OpenFile();

    // Look it up in the mesh table.
    std::map<std::string,meshInfo>::const_iterator m = meshes.find(meshname);
    if (m != meshes.end())
    {
        vtkRectilinearGrid *grid = NULL;
        grid = CreateUniformGrid(m->second);
        grid->Register(NULL);
        
        return grid;
    }

    //It might be a curve.
    std::map<std::string,varInfo>::const_iterator v = variables.find(meshname);
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
//  Method: avtADIOSFileFormat::GetVar
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
avtADIOSFileFormat::GetVar(int timestate, int domain, const char *varname)
{
    debug1 << "avtADIOSFileFormat::GetVar " << varname << endl;
    OpenFile();

    vtkDataArray *vtkarray = GetADIOSVar(timestate, varname);
    return vtkarray;
}


// ****************************************************************************
//  Method: avtADIOSFileFormat::GetVectorVar
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
avtADIOSFileFormat::GetVectorVar(int timestate, int domain, const char *varname)
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
avtADIOSFileFormat::OpenFile()
{
    if (fileOpened)
        return;

    // INITIALIZE DATA MEMBERS
    int     gr, vr, i, j;             // loop vars
    ADIOS_VARINFO *avi;
#ifdef PARALLEL
    fp = adios_fopen(filename.c_str(), VISIT_MPI_COMM);
#else
    fp = adios_fopen(filename.c_str(), 0);
#endif

    // open the BP file
    // get number of groups, variables, timesteps, and attributes 
    // Internally, only rank=0 reads in anything and distributes index to other processes

    if (fp == NULL) {
        sprintf(errmsg, "Error opening bp file %s:\n%s", filename.c_str(), adios_errmsg());
        EXCEPTION1(InvalidDBTypeException, errmsg);
    }
    
    numTimeSteps = fp->ntimesteps;
    
    debug5 << "ADIOS BP file: " << filename << endl;
    debug5 << "# of groups: " << fp->groups_count << endl;
    debug5 << "# of variables: " << fp->vars_count << endl;
    debug5 << "# of attributes:" << fp->attrs_count << endl;
    debug5 << "time steps: " << fp->ntimesteps << " from " << fp->tidx_start << endl;


    meshes.clear();
    variables.clear();
    gps = (ADIOS_GROUP **) malloc(fp->groups_count * sizeof(ADIOS_GROUP *));
    if (gps == NULL)
        EXCEPTION1(InvalidDBTypeException, "The file could not be opened. Not enough memory");
    
    for (int gr=0; gr<fp->groups_count; gr++)
    {
        debug5 <<  "  group " << fp->group_namelist[gr] << ":" << endl;
        gps[gr] = adios_gopen_byid(fp, gr);
        if (gps[gr] == NULL) {
            sprintf(errmsg, "Error opening group %s in bp file %s:\n%s", fp->group_namelist[gr], filename.c_str(), adios_errmsg());
            EXCEPTION1(InvalidDBTypeException, errmsg);
        }
        
        for (int vr=0; vr<gps[gr]->vars_count; vr++)
        {
            avi = adios_inq_var_byid (gps[gr], vr);
            if (avi == NULL) {
                sprintf(errmsg, "Error opening inquiring variable %s in group %s of bp file %s:\n%s", 
                    gps[gr]->var_namelist[vr], fp->group_namelist[gr], filename.c_str(), adios_errmsg());
                EXCEPTION1(InvalidDBTypeException, errmsg);
            }

            // Skip scalars, or variables with > 3 real dimensions (besides time)
            if ( avi->ndim == 0 ||                        // scalar value in file
                (avi->ndim == 1 && avi->timedim >= 0) ||  // scalar with time
                (avi->ndim > 3 && avi->timedim == -1) ||  // >3D array with no time
                (avi->ndim > 4 && avi->timedim >= 0))     // >3D array with time
            {
                debug5<<"Skipping variable"<<gps[gr]->var_namelist[vr]<<" of dimension: "<<avi->ndim<<endl;
                continue;
            }
            
            // Skip arrays whose type is not supported in VisIt            
            if (avi->type == adios_long_double ||
                avi->type == adios_complex || 
                avi->type == adios_double_complex) 
            {
                debug5<<"Skipping variable"<<gps[gr]->var_namelist[vr]<<" of unsupported type: "<<adios_type_to_string(avi->type)<<endl;
                continue;
            }

            // add variable to map, map id = variable path without the '/' in the beginning
            varInfo vi;
            if (gps[gr]->var_namelist[vr][0] == '/')
            {
                vi.name = gps[gr]->var_namelist[vr]+1;
            }
            else
            {
                vi.name = gps[gr]->var_namelist[vr];
            }

            vi.type = avi->type;
            vi.timedim = avi->timedim;
            vi.groupIdx = gr;
            vi.varid = avi->varid;
            
            // VisIt variable is max 3D and has no time dimension
            // ADIOS variable's dimensions include the time dimension 
            //  (which is the first dim in C/C++, i.e. avi->timedim == 0 or -1 if it has no time)
            if (avi->timedim == -1)
                vi.dim = avi->ndim;
            else
                vi.dim = avi->ndim - 1;
                    
            i = 0; // avi's index
            j = 0; // vi's index
            // 1. process dimensions before the time dimension
            // Note that this is empty loop with current ADIOS/C++ (timedim = 0 or -1)
            for (; i < MIN(avi->timedim,3); i++)
            {
                vi.start[j] = 0;
                vi.count[j] = 1;
                vi.global[j] = 1;
                if (i<avi->ndim)
                    vi.count[j] = vi.global[j] = (int) avi->dims[i];
                j++;
            }
            // 2. skip time dimension if it has one
            if (avi->timedim >= 0)
                i++; 
            // 3. process dimensions after the time dimension
            for (; i < (avi->timedim == -1 ? 3 : 4); i++)
            {
                vi.start[j] = 0;
                vi.count[j] = 1;
                vi.global[j] = 1;
                if (i<avi->ndim)
                    vi.count[j] = vi.global[j] = (int) avi->dims[i];
                j++;
            }

            vi.SwapIndices();

            // Define a mesh for this variable's dimensions
            // name is like mesh_23x59, mesh_100x200x300

            //Build this list backwards (adios uses ZYX indexing).
            string meshname = "mesh_";
            vector<int64_t> dimT, dims;
            for (int i=0; i<avi->ndim; i++)
            {
                if (i != avi->timedim)
                    dims.insert(dims.begin(), avi->dims[i]);
            }

            for (int i=0; i <dims.size(); i++)
            {
                meshname += Int64ToString(dims[i]);
                if (i<dims.size()-1)
                    meshname += "x";
            }
            vi.meshName = meshname;
            debug5<<"MESH: "<<meshname<<endl;

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
                
                debug5<<"Add mesh "<<meshname<<" ["<<mi.count[0]<<" "<<mi.count[1]<<" "<<mi.count[2]<<"]"<<endl;
            }

            // add variable to map
            variables[vi.name] = vi;
            debug5 << "  added variable " << vi.name<<" with mesh "<<vi.meshName<<endl;

            // Note: avi has gmin and gmax / or value that we did not use here
            adios_free_varinfo(avi);
            
        } // end var
    } // end group

    DoDomainDecomposition();
    
    fileOpened = true;
}

// ****************************************************************************
//  Method: avtADIOSFileFormat::CloseFile
//
//  Purpose:
//      Open the file.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

void
avtADIOSFileFormat::CloseFile()
{
    if (!fileOpened)
        return;
    
    if (gps) {
        for (int gr=0; gr<fp->groups_count; gr++)
            adios_gclose(gps[gr]);
        free(gps);
    }
    adios_fclose(fp);
    meshes.clear();
    variables.clear();

    fileOpened = false;
}

// ****************************************************************************
//  Method: avtADIOSFileFormat::DoDomainDecomposition
//
//  Purpose:
//      Decompose across processor set.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

void
avtADIOSFileFormat::DoDomainDecomposition()
{
    std::map<std::string,meshInfo>::iterator m;
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
//  Method: avtADIOSFileFormat::GetADIOSVar
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
avtADIOSFileFormat::GetADIOSVar(int timestate, const char *varname)
{
    debug5<<"avtADIOSFileFormat::GetADIOSVar "<<varname<<endl;
    
    std::map<std::string,varInfo>::const_iterator v = variables.find(varname);
    if (v == variables.end())
        EXCEPTION1(InvalidVariableException, varname);
    
    std::map<std::string,meshInfo>::const_iterator m = meshes.find(v->second.meshName);
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

    //Get the offsets.... Max 4D can be here as max 3D vars are allowed plus time
    int ntuples = 1;
    uint64_t start[4] = {0,0,0,0}, count[4] = {0,0,0,0};
    int i=0;  // VisIt var dimension index
    int j=0;  // adios var dimension index

    // timedim=-1 for non-timed variables, 0..n for others
    // 1. up to time index, or max 3
    // This loop is empty with current ADIOS/C++ (timedim = -1 or 0)
    for (; i<MIN(v->second.timedim,3); i++)
    {
        ntuples *= (m->second.count[i]);
        start[j] = (uint64_t) m->second.start[i];
        count[j] = (uint64_t) m->second.count[i];
        j++;
    }
    // 2. handle time index if the variable has time
    if (v->second.timedim >= 0) 
    {
        start[j] = timestate;
        count[j] = 1;
        j++;
    }
    // 3. the rest of indices (all if no time dimension)
    for (; i<3; i++)
    {
        ntuples *= (m->second.count[i]);
        start[j] = (uint64_t) m->second.start[i];
        count[j] = (uint64_t) m->second.count[i];
        j++;
    }
    
    adiosArr->SetNumberOfTuples(ntuples);
    void *data = adiosArr->GetVoidPointer(0);

    SwapIndices(m->second.dim, start);
    SwapIndices(m->second.dim, count);
    
    debug5<<"s "<<start[0]<<" "<<start[1]<<" "<<start[2]<<" "<<start[3]<<endl;
    debug5<<"c "<<count[0]<<" "<<count[1]<<" "<<count[2]<<" "<<count[3]<<endl;
    debug5<<"adios_read_var ["<<start[0]<<"+"<<count[0]<<", "<<start[1]<<"+"<<count[1]<<", "<<start[2]<<"+"<<count[2]<<"] nt= "<<ntuples<<endl;
    for(int i = 0; i < ntuples; i++) ((float *)data)[i] = 0.0;
    
    if (adios_read_var_byid(gps[v->second.groupIdx], v->second.varid, start, count, data) < 0) {
        sprintf(errmsg, "Error when reading variable %s:\n%s", v->second.name.c_str(), adios_errmsg());
        EXCEPTION1(InvalidVariableException, errmsg);
    }

    return adiosArr;
}


// ****************************************************************************
//  Method: avtADIOSFileFormat::CreateUniformGrid
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
avtADIOSFileFormat::CreateUniformGrid(const meshInfo &mi)
{
    vtkRectilinearGrid *grid = vtkRectilinearGrid::New();
    vtkFloatArray *coords[3] = {NULL,NULL,NULL};
    
    int dims[3] = {1,1,1};
    for (int i = 0; i<3; i++)
    {
        dims[i] = (int)mi.count[i];

        coords[i] = vtkFloatArray::New();
        coords[i]->SetNumberOfTuples(dims[i]);
        float *data = (float *)coords[i]->GetVoidPointer(0);

        int x = (int)mi.start[i];

        debug5<<"I= "<<i<<endl;
        for (int j = 0; j < mi.count[i]; j++, x++)
        {
            data[j] = (float)x;
            debug5<<"  "<<x;
        }
        debug5<<endl;
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
