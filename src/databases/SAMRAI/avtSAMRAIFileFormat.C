// ***************************************************************************
//                              avtSAMRAIFileFormat.C                         
//
//  Purpose:  Read SAMRAI data files 
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    July 7, 2003
//
//  Modifications:
//    Mark C. Miller
//    Eric Brugger
//    Kathleen Bonnell
//
// ****************************************************************************

#define USE_UNIQUE_SPECIES

#include <avtSAMRAIFileFormat.h>

#include <AutoArray.h>

#include <BJHash.h>

#include <vector>
#include <string>
#include <stdlib.h>

#include <vtkCellData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkPoints.h>

#include <avtDatabaseMetaData.h>
#include <avtGhostData.h>
#include <avtIntervalTree.h>
#include <avtIOInformation.h>
#include <avtMaterial.h>
#include <avtSpecies.h>
#include <avtStructuredDomainBoundaries.h>
#include <avtStructuredDomainNesting.h>
#include <avtVariableCache.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <UnexpectedValueException.h>
#include <Utility.h>
#include <DataNode.h>

#include <snprintf.h>

using std::vector;
using std::string;

// the version of the SAMRAI writer the current reader code matches 
static const float        expected_version_number = 2.0;
static const char        *inferredVoidMatName = "inferred void";
static const int MAX_GHOST_LAYERS = 2;
static const int MAX_GHOST_CODES = MAX_GHOST_LAYERS *
                                   MAX_GHOST_LAYERS *
                                   MAX_GHOST_LAYERS;

static string GetDirName(const char *path);

#define SAFE_DELETE(P)      \
   if (P != 0)              \
   {                        \
       delete[] P;          \
       P = 0;               \
   }

int avtSAMRAIFileFormat::objcnt = 0;

// ****************************************************************************
//  Function:  avtSAMRAIFileFormat::InitializeHDF5
//
//  Purpose:   Initialize interaction with the HDF5 library
//
//  Programmer:  Mark C. Miller 
//  Creation:    Decmeber 10, 2003 
//
// ****************************************************************************
void
avtSAMRAIFileFormat::InitializeHDF5(void)
{
    debug5 << "Initializing HDF5 Library" << endl;
    H5open();
    H5Eset_auto(NULL, NULL);
}

// ****************************************************************************
//  Function:  avtSAMRAIFileFormat::FinalizeHDF5
//
//  Purpose:   End interaction with the HDF5 library
//
//  Programmer:  Mark C. Miller 
//  Creation:    Decmeber 10, 2003 
//
// ****************************************************************************
void
avtSAMRAIFileFormat::FinalizeHDF5(void)
{
    debug5 << "Finalizing HDF5 Library" << endl;
    H5close();
}

// ****************************************************************************
//  Constructor:  avtSAMRAIFileFormat::avtSAMRAIFileFormat
//
//  Arguments:
//    fname      the file name of the root metadata file
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    July 07, 2003
//
//  Modifications:
//
//    Mark C. Miller, Thu Jan 29 11:21:41 PST 2004
//    Added initialization for var_max_ghost data member
//
//    Mark C. Miller, Mon Aug 23 14:17:55 PDT 2004
//    Added initialization for h5files data member
//
// ****************************************************************************
avtSAMRAIFileFormat::avtSAMRAIFileFormat(const char *fname)
    : avtSTMDFileFormat(&fname, 1)
{
    int i;

    // do HDF5 library initialization on consturction of first instance 
    if (avtSAMRAIFileFormat::objcnt == 0)
        InitializeHDF5();
    avtSAMRAIFileFormat::objcnt++;

    time_step_number = 0;

    cached_patches = NULL;
    xlo = NULL;
    dx = NULL;
    num_patches_level = NULL;
    ratios_coarser_levels = NULL;
    var_cell_centered = NULL;
    patch_extents = NULL;
    var_extents = NULL;
    var_names = NULL;
    var_num_components = NULL;
    var_num_ghosts = NULL;
    patch_map = NULL;
    child_array = NULL;
    child_pointer_array = NULL;
    parent_array = NULL;
    parent_pointer_array = NULL;

    num_vars = 0;
    num_patches = 0;
    child_array_length = 0;
    parent_array_length = 0;

    dir_name = GetDirName(fname);
    file_name = fname;

    last_patch = -1;

    has_ghost = false;
    ghosting_is_consistent = true;
    for (i = 0; i < 3; i++)
       var_max_ghosts[i] = 0;

    has_mats = false;
    inferVoidMaterial = false;
    num_mats = 0;
    mat_num_ghosts = 0;
    mat_names = 0;
    mat_var_num_components = 0;
    mat_var_names = 0;

    has_specs = false;
    nmatspec = 0;
    num_spec_vars = 0;
    spec_var_names = 0;

    h5files = new hid_t[MAX_FILES];
    for (int i = 0 ; i < MAX_FILES ; i++)
    {
        h5files[i] = -1;
    }

}

// ****************************************************************************
//  Destructor:  avtSAMRAIFileFormat::~avtSAMRAIFileFormat
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    June 19, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Aug 23 14:17:55 PDT 2004
//    Added deletion of h5files data member
//
// ****************************************************************************
avtSAMRAIFileFormat::~avtSAMRAIFileFormat()
{
    int i,v;

    SAFE_DELETE(xlo);
    SAFE_DELETE(dx);
    SAFE_DELETE(num_patches_level);
    SAFE_DELETE(ratios_coarser_levels)
    SAFE_DELETE(patch_extents);
    
    if (var_extents != NULL) {
        for(v=0; v<num_vars; v++) {
            delete[] var_extents[v];
        }
        delete[] var_extents;
    }
    var_extents = NULL;

    //
    // cleanup variable information
    //
    SAFE_DELETE(var_names);
    SAFE_DELETE(var_cell_centered);
    SAFE_DELETE(var_num_components);
    SAFE_DELETE(var_num_ghosts);

    //
    // cleanup parent/child information
    //
    SAFE_DELETE(patch_map);
    SAFE_DELETE(child_array);
    SAFE_DELETE(child_pointer_array);
    SAFE_DELETE(parent_array);
    SAFE_DELETE(parent_pointer_array);

    //
    // clean up stuff having to do with species
    //
    SAFE_DELETE(nmatspec);
    SAFE_DELETE(spec_var_names);
    for (i = 0; i < num_mats; i++)
        SAFE_DELETE(mat_specs[mat_names[i]]);

    std::map<std::string, std::map<std::string, matinfo_t*> >::iterator ms;
    for (ms  = mat_specs_matinfo.begin();
         ms != mat_specs_matinfo.end(); ms++)
    {
        std::map<std::string, matinfo_t*>::iterator mms;
        for (mms  = ms->second.begin();
             mms != ms->second.end(); mms++)
        {
            SAFE_DELETE(mms->second);
        }
    }

    //
    // clean up stuff having to do with materials 
    //
    std::map<std::string, matinfo_t*>::iterator m;
    for (m  = mat_names_matinfo.begin();
         m != mat_names_matinfo.end(); m++)
    {
        SAFE_DELETE(m->second);
    }

    std::map<std::string, std::map<std::string, matinfo_t*> >::iterator mv;
    for (mv  = mat_var_names_matinfo.begin();
         mv != mat_var_names_matinfo.end(); mv++)
    {
        std::map<std::string, matinfo_t*>::iterator mmv;
        for (mmv  = mv->second.begin();
             mmv != mv->second.end(); mmv++)
        {
            SAFE_DELETE(mmv->second);
        }
    }

    SAFE_DELETE(mat_names);
    SAFE_DELETE(mat_var_names);
    SAFE_DELETE(mat_num_ghosts);
    SAFE_DELETE(mat_var_num_components);

    //
    // cleanup the mesh cache
    //
    if (cached_patches != 0)
    {
        int p;
        for (p=0; p<num_patches; p++)
        {
            if (cached_patches[p] != 0)
            {
                if (has_ghost)
                {
                    int g;
                    for (g=0; g<MAX_GHOST_CODES; g++) {
                        // XXX is it necessary??
                        if (cached_patches[p][g] != 0)
                            cached_patches[p][g]->Delete();
                    }
                }
                else
                {
                    if (cached_patches[p][0] != 0)
                        cached_patches[p][0]->Delete();
                }
                delete[] cached_patches[p];
            }
        }
        delete[] cached_patches;
    }
    cached_patches = 0;

    SAFE_DELETE(h5files);

    // handle HDF5 library termination on descrution of last instance
    avtSAMRAIFileFormat::objcnt--;
    if (avtSAMRAIFileFormat::objcnt == 0)
        FinalizeHDF5();
}

// ****************************************************************************
//  Method:  avtSAMRAIFileFormat::OpenFile
//
//  Purpose:
//    Opens and registers or uses a named file and sets knowledge of the file
//    in the file format 
//
//  Programmer:  Mark C. Miller 
//  Creation:    August 19, 2004 
//
// ****************************************************************************
hid_t
avtSAMRAIFileFormat::OpenFile(const char *fileName)
{
    //
    // See if we've seen this file name already
    //
    int fileIndex = -1;
    for (int i = 0 ; i < nFiles ; i++)
    {
        if (strcmp(filenames[i], fileName) == 0)
        {
            fileIndex = i;
            break;
        }
    }
 
    if (fileIndex == -1)
    {
        //
        // We have asked for a previously unseen file.  Add it to the list and
        // continue.  AddFile will automatically take care of overflow issues.
        //
        fileIndex = AddFile(fileName);
    }

    //
    // Make sure this is in range.
    //
    if (fileIndex < 0 || fileIndex >= nFiles)
    {
        EXCEPTION2(BadIndexException, fileIndex, nFiles);
    }
 
    //
    // Check to see if the file is already open.
    //
    if (h5files[fileIndex] >= 0)
    {
        UsedFile(fileIndex);
        return h5files[fileIndex];
    }
 
    debug4 << "Opening HDF5 file " << filenames[fileIndex] << endl;
 
    //
    // Open the HDF5 file.
    //
    h5files[fileIndex] = H5Fopen(filenames[fileIndex], H5F_ACC_RDONLY, H5P_DEFAULT);
                        
    //
    // Check to see if we got a valid handle.
    //
    if (h5files[fileIndex] < 0)
    {
        EXCEPTION1(InvalidFilesException, filenames[fileIndex]);
    }
 
    RegisterFile(fileIndex);
 
    return h5files[fileIndex];
}

// ****************************************************************************
//  Method: avtSAMRAIlFileFormat::CloseFile
//
//  Purpose:
//      Implements the AVT interface's CloseFile method 
//
//  Programmer: Mark C. Miller 
//  Creation:   August 19, 2004
//
// ****************************************************************************
 
void
avtSAMRAIFileFormat::CloseFile(int f)
{
    if (h5files[f] >= 0)
    {
        debug4 << "Closing HDF5 file " << filenames[f] << endl;
        H5Fclose(h5files[f]);
        UnregisterFile(f);
        h5files[f] = -1;
    }
}

// ****************************************************************************
//  Method:  avtSAMRAIFileFormat::RegisterVariableList
//
//  Purpose:
//    Records the active variable name so per-variable ghosting can be applied
//    during GetMesh calls.
//
//  Programmer:  Mark C. Miller 
//  Creation:    December 9, 2003 
//
// ****************************************************************************
void
avtSAMRAIFileFormat::RegisterVariableList(const char *prim_var_name,
                                          const std::vector<CharStrRef> &)
{
    active_visit_var_name = prim_var_name;
}

// ****************************************************************************
//  Method:  avtSAMRAIFileFormat::CanCacheVariable
//
//  Purpose:
//    Determines if the given variable can be cached above the layer of the
//    plugin. In some cases, particularly where different ghosting is used
//    for different variables, the mesh variable cannot be cached.
//
//  Programmer:  Mark C. Miller 
//  Creation:    December 10, 2003 
//
// ****************************************************************************
bool
avtSAMRAIFileFormat::CanCacheVariable(const char *var_name)
{
   if (strncmp(var_name,"amr_mesh",8) == 0)
   {
      if (has_ghost == false)
          return true;
      else
          return false;
   }
   else
       return true;
}

// ****************************************************************************
//  Method:  avtSAMRAIFileFormat::GetMesh
//
//  Purpose:
//    Returns the mesh with the given name for the given time step and
//    domain.  This function will return a cached reference to the mesh
//    for this domain, since the mesh does not change over time.
//
//  Note:  The Generic Database (which calls this function) already handles
//         caching for a single time step.
//
//  Arguments:
//    ts         the time step
//    dom        the domain number
//    mesh       the name of the mesh to read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    June 19, 2003
//
//  Modifications
//
//     Mark C. Miller, Mon Nov 10 09:55:59 PST 2003
//     Added call to BuildDomainNestingInfo
//
//     Mark C. Miller, Tue Dec  9 10:45:46 PST 2003
//     Changed name of BuildDomainNestingInfo to BuildDomainAuxiliaryInfo
//
//     Brad Whitlock, Fri Mar 5 10:19:32 PDT 2004
//     Changed for Windows compiler.
//
// ****************************************************************************
vtkDataSet *
avtSAMRAIFileFormat::GetMesh(int patch, const char *)
{
    // Make sure the domain nest info object exists
    BuildDomainAuxiliaryInfo();

    int ghostCode = GetGhostCodeForVar(active_visit_var_name.c_str());

    if (cached_patches[patch][ghostCode] != NULL)
    {
        debug5 << "avtSAMRAIFileFormat::GetMesh returning cached value for \""
               << active_visit_var_name.c_str() << "\"" << endl;

        // The reference count will be decremented by the generic database,
        // because it will assume it owns it.
        cached_patches[patch][ghostCode]->Register(NULL);
        return cached_patches[patch][ghostCode];
    }

    vtkDataSet *ds = ReadMesh(patch);
    cached_patches[patch][ghostCode] = ds;
    ds->Register(NULL);

    return ds;
}

// ****************************************************************************
//  Method:  avtSAMRAIFileFormat::ReadMesh
//
//  Purpose:
//    Reads the patch with the given id for the given time step and
//    domain from the file.  NOTE: time step is ignored in this case
//    because the mesh does not change over time.
//
//  Arguments:
//    patch      the patch number
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    July 11, 2003
//
//  Modificatioons:
//    Mark C. Miller, Tue Dec  9 08:54:54 PST 2003
//    Added support for ghosting. Note that it can vary depending upon the
//    variable requested.
//
//    Mark C. Miller, Wed Jan  7 11:35:37 PST 2004
//    Elminated traversal of ALE coordinate data and replaced with a SetData
//    call
//
//    Brad Whitlock, Fri Mar 5 10:19:32 PDT 2004
//    Changed for Windows compiler.
//
//    Mark C. Miller, Tue Aug 24 20:11:52 PDT 2004
//    Added code to set up the magic 'base_index' array
//
//    Hank Childs, Fri Aug 27 17:16:05 PDT 2004
//    Rename ghost data array.
//
// ****************************************************************************
vtkDataSet *
avtSAMRAIFileFormat::ReadMesh(int patch)
{
    vtkDataSet *retval;

    debug5 << "avtSAMRAIFileFormat::ReadMesh for patch " << patch << endl;

    int dim = num_dim_problem < 3 ? num_dim_problem: 3;
    int level = patch_map[patch].level_number;

    bool should_ghost_this_patch = has_ghost;
    int num_ghosts[3] = {0,0,0};

    // set ghost information
    if (has_ghost)
    {

        //
        // lookup ghost layer information for the active variable
        //
        std::map<std::string, var_t>::const_iterator cur_var;
        cur_var = var_names_num_components.find(active_visit_var_name);

        if (active_visit_var_name == "amr_mesh")
        {
            num_ghosts[0] = var_max_ghosts[0];
            num_ghosts[1] = var_max_ghosts[1];
            num_ghosts[2] = var_max_ghosts[2];
            should_ghost_this_patch = true;
        }
        else if (cur_var == var_names_num_components.end())
        {
            num_ghosts[0] = 0;
            num_ghosts[1] = 0;
            num_ghosts[2] = 0;
            should_ghost_this_patch = false;
        }
        else
        {
            num_ghosts[0] = (*cur_var).second.num_ghosts[0];
            num_ghosts[1] = (*cur_var).second.num_ghosts[1];
            num_ghosts[2] = (*cur_var).second.num_ghosts[2];
            if ((num_ghosts[0] == 0) &&
                (num_ghosts[1] == 0) &&
                (num_ghosts[2] == 0))
                should_ghost_this_patch = false;
        }

        if (should_ghost_this_patch)
            debug5 << "avtSAMRAIFileFormat::ReadMesh has ghost layers " << 
                num_ghosts[0] << ", " << num_ghosts[1] << ", " << num_ghosts[2] << endl;

    }
 
    // compute logical size in each dimension of this patch
    int dimensions[] = {1, 1, 1};
    for (int i=0; i<dim; i++) {
        dimensions[i] = patch_extents[patch].upper[i] -
                        patch_extents[patch].lower[i] + 2 +
                        2 * num_ghosts[i];
    }

    if (grid_type == "RECTILINEAR" || grid_type == "CARTESIAN") 
    {

        // compute physical origin and cell-spacing
        float spacing[] = {0, 0, 0};
        float origin[] = {0, 0, 0};
        int i;
        for (i=0; i<dim; i++) {
            spacing[i] = dx[level * 3 + i];
            origin[i] = patch_extents[patch].xlo[i] - 
                        spacing[i] * num_ghosts[i];
        }

        vtkFloatArray  *coords[3];
        for (i = 0 ; i < 3 ; i++)
          {
            // Default number of components for an array is 1.
            coords[i] = vtkFloatArray::New();
            coords[i]->SetNumberOfTuples(dimensions[i]);
            for (int j = 0 ; j < dimensions[i] ; j++)
              {
                float c = origin[i] + j * spacing[i];
                coords[i]->SetComponent(j, 0, c);
              }
          }
   
        vtkRectilinearGrid  *rGrid = vtkRectilinearGrid::New(); 
        rGrid->SetDimensions(dimensions);
        rGrid->SetXCoordinates(coords[0]);
        coords[0]->Delete();
        rGrid->SetYCoordinates(coords[1]);
        coords[1]->Delete();
        rGrid->SetZCoordinates(coords[2]);
        coords[2]->Delete();

        retval = rGrid;
    }
    else if (grid_type == "ALE" || grid_type == "DEFORMED") 
    {
        char var_name[100];
        sprintf(var_name, "Coords");

        vtkDataArray * array = GetVectorVar(patch, var_name);

        vtkPoints *points = vtkPoints::New();
        points->SetData(array);
        array->Delete();

        vtkStructuredGrid  *sGrid = vtkStructuredGrid::New(); 
        sGrid->SetDimensions(dimensions);
        sGrid->SetPoints(points);
        points->Delete();

        retval = sGrid;
    }
    else {
        char dummyVarName[128];
        sprintf(dummyVarName,"amr_mesh, patch %d", patch);
        EXCEPTION1(InvalidVariableException, dummyVarName);
    }

    // Add ghost information if we should
    if (should_ghost_this_patch)
    {
        int nghost = 0;
        int ncells = 1;
        int i;
        for (i = 0; i < dim; i++)
            ncells *= (dimensions[i]-1);

        vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
        ghostCells->SetName("avtGhostZones");
        ghostCells->Allocate(ncells);

        // fill in ghost value (0/1) for each cell
        for (i = 0; i < ncells; i++)
        {
            bool in_ghost_layers = false;

            // determine if cell is in ghost layers
            int cell_idx = i;
            for (int j = 0; j < dim; j++)
            {
                int nj = dimensions[j] - 1;
                int jidx = cell_idx % nj;

                if ((jidx < num_ghosts[j]) || (jidx >= nj - num_ghosts[j]))
                {
                    in_ghost_layers = true;
                    break;
                }

                cell_idx = cell_idx / nj;
            }

            if (in_ghost_layers)
            {
                unsigned char v = 0;
                avtGhostData::AddGhostZoneType(v, 
                                          DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
                ghostCells->InsertNextValue(v);
                nghost++;
            }   
            else
                ghostCells->InsertNextValue((unsigned char)0);
        }

        // now, attach the ghost data to the grid
        retval->GetCellData()->AddArray(ghostCells);
        ghostCells->Delete();

        debug5 << "avtSAMRAIFileFormat::ReadMesh ghosted " <<
            100*nghost/ncells << "% of cells" << endl;
    }

    //
    // Emulate VisIt's notion of a block-structured grid by setting
    // the 'base_index' magic data array for this vtk grid
    //
    vtkIntArray *arr = vtkIntArray::New();
    arr->SetNumberOfTuples(3);
    for (int i=0; i<dim; i++)
       arr->SetValue(i, patch_extents[patch].lower[i] - num_ghosts[i]);
    for (int i=dim; i<3; i++)
       arr->SetValue(i, 0);
    arr->SetName("base_index");
    retval->GetFieldData()->AddArray(arr);
    arr->Delete();

    return retval;

}


// ****************************************************************************
//  Method:  avtSAMRAIFileFormat::GetVar
//
//  Purpose:
//    Reads the variable with the given name for the given time step and
//    domain.
//
//  Note:  The Generic Database (which calls this function) already handles
//         caching for a single time step.
//
//  Arguments:
//    ts         the time step
//    dom        the domain number
//    name       the name of the variable to read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    July 18, 2003
//
//  Modifications:
//    Kathleen Bonnell, Mon Dec 22 15:06:41 PST 2003
//    Added code to delete hdims, max_hdims.
//
//    Mark C. Miller, Tue Jan 13 14:23:38 PST 2004
//    Replaced with call to ReadVar to reduce duplication of code
//
// ****************************************************************************
vtkDataArray *
avtSAMRAIFileFormat::GetVar(int patch, const char *visit_var_name)
{
    return ReadVar(patch, visit_var_name);
}

vtkDataArray *
avtSAMRAIFileFormat::GetVectorVar(int patch, const char *visit_var_name)
{
    return ReadVar(patch, visit_var_name);
}

// ****************************************************************************
//  Method:  avtSAMRAIFileFormat::ReadVar
//
//  Purpose:
//    Reads a scalar, vector or tensor variable with the given name for the
//    given domain.
//
//  Arguments:
//    patch                the patch number
//    visit_var_name       the name of the variable to read
//    expected_num_comps   the expected number of components
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    July 18, 2003
//
//  Modifications:
//    Eric Brugger, Tue Dec 30 16:37:26 PST 2003
//    I modified the routine to properly read vector (and coordinate) data.
//    This involved file format as well as reader changes.  In this case
//    I modified the routine to handle the variable name without the extension
//    (.00, .01, .02) identifying the component.
//
//    Mark C. Miller, Wed Jan  7 11:35:37 PST 2004
//    I eliminated use of VTK SetComponent methods as well as extra buffer
//    copies. Now, HDF5 reads the data directly into the buffer space allocated
//    by VTK in the vtkFloatArray object.
//
//    Mark C. Miller, Tue Jan 13 13:45:31 PST 2004
//    Renamed from GetVectorVar to ReadVar
//
//    Brad Whitlock, Fri Mar 5 10:19:32 PDT 2004
//    Changed for Windows compiler.
//
//    Mark C. Miller, Wed May 19 10:56:11 PDT 2004
//    Re-factored code having to do with guessing variable type to avtTypes
//    function GuessVarTypeFromNumDimsAndComps
//
//    Mark C. Miller, Mon Aug 23 14:17:55 PDT 2004
//    Made it use OpenFile instead of H5Fopen directly. Removed call to
//    H5Fclose
//
// ****************************************************************************
vtkDataArray *
avtSAMRAIFileFormat::ReadVar(int patch, 
                             const char *visit_var_name)
{

    debug5 << "avtSAMRAIFileFormat::ReadVar for variable " << visit_var_name
           << "on patch " << patch << endl;

    string var_name = visit_var_name; 

    // find the variable name in our variable list
    std::map<std::string, var_t>::const_iterator cur_var;
    cur_var = var_names_num_components.find(var_name);
    if (cur_var == var_names_num_components.end())
    {
        EXCEPTION1(InvalidVariableException, visit_var_name);
    }
    
    // get component count, centering and ghost information for this variable
    int num_components = (*cur_var).second.num_components;
    int cell_centered =  (*cur_var).second.cell_centered;
    int num_ghosts[3] = {(*cur_var).second.num_ghosts[0],
                         (*cur_var).second.num_ghosts[1],
                         (*cur_var).second.num_ghosts[2]};
        
    // compute total number of samples
    int extent = cell_centered == 1 ? 1 : 2;
    int dim = num_dim_problem < 3 ? num_dim_problem: 3;
    int num_data_samples = 1;
    int i;
    for (i=0; i<dim; i++) {
        num_data_samples *= patch_extents[patch].upper[i] - 
                            patch_extents[patch].lower[i] + extent + 2 * num_ghosts[i];
    }

    // verify we actually have data on this patch
    for (int v = 0; v < num_vars; v++) {
        if (var_names[v] == var_name) {
            if (!var_extents[v][patch].data_is_defined) {
                return NULL;
            }
            break;
        }
    }

    // determine the name of the file that contains the variable and open it 
    char file[2048];   
    sprintf(file, "%sprocessor_cluster.%05d.samrai",
            dir_name.c_str(), patch_map[patch].file_cluster_number);
    hid_t h5f_file = OpenFile(file);
    if (h5f_file < 0)
    {
        EXCEPTION1(InvalidFilesException, file);
    }

    // guess var_type from problem dimensions and number of components
    avtVarType var_type = GuessVarTypeFromNumDimsAndComps(num_dim_problem, num_components);
    if (var_type == AVT_UNKNOWN_TYPE)
    {
        EXCEPTION1(InvalidVariableException, visit_var_name);
    }

    // determine number of components to allocate in VTK data array
    int num_alloc_comps = 0;
    switch (var_type)
    {
        case AVT_SCALAR_VAR: num_alloc_comps = 1; break;
        case AVT_VECTOR_VAR: num_alloc_comps = 3; break;
        case AVT_SYMMETRIC_TENSOR_VAR:
        case AVT_TENSOR_VAR: num_alloc_comps = 9; break;
        default: break;
    }
    if (num_alloc_comps == 0)
    {
        EXCEPTION2(UnexpectedValueException, num_alloc_comps, "a value other than zero");
    }

    // allocate VTK data array for this variable 
    vtkFloatArray *var_data = vtkFloatArray::New();
    var_data->SetNumberOfComponents(num_alloc_comps);
    var_data->SetNumberOfTuples(num_data_samples);
    float *fbuf = (float*) var_data->GetVoidPointer(0);

    for (i = 0 ; i < num_components ; i++)
    {
        // determine name of the HDF5 dataset and open it 
        char variable[256];
        if (var_type == AVT_SCALAR_VAR)
        {
            sprintf(variable, "/processor.%05d/level.%05d/patch.%05d/%s",
                    patch_map[patch].processor_number,
                    patch_map[patch].level_number,
                    patch_map[patch].patch_number, 
                    var_name.c_str());
        }
        else
        {
            sprintf(variable, "/processor.%05d/level.%05d/patch.%05d/%s.%02d",
                    patch_map[patch].processor_number,
                    patch_map[patch].level_number,
                    patch_map[patch].patch_number, 
                    var_name.c_str(), i);
        }
        hid_t h5d_variable = H5Dopen(h5f_file, variable);
        if (h5d_variable < 0)
        {
            EXCEPTION1(InvalidFilesException, file);
        }

        // check dataset size agrees with what we computed for num_data_samples
        hid_t h5d_space = H5Dget_space(h5d_variable);
        int hndims = H5Sget_simple_extent_ndims(h5d_space);
        hsize_t *hdims = new hsize_t[hndims];
        hsize_t *max_hdims = new hsize_t[hndims];
        H5Sget_simple_extent_dims(h5d_space, hdims, max_hdims);
        hsize_t hsum = 1;
        for (int j = 0; j < hndims; j++)
            hsum *= hdims[j];
        if ((hsize_t) num_data_samples != hsum)
        {
            EXCEPTION2(UnexpectedValueException, hsum, num_data_samples);
        }
        H5Sclose(h5d_space);
        delete [] hdims;
        delete [] max_hdims;

        // create dataspace and selection to read directly into fbuf
        hsize_t nvals = num_alloc_comps * num_data_samples;
        hid_t memspace = H5Screate_simple(1, &nvals, &nvals);
        hssize_t start = i;
        hsize_t stride = num_alloc_comps;
        hsize_t count = num_data_samples;
        H5Sselect_hyperslab(memspace, H5S_SELECT_SET, &start, &stride, &count, NULL);

        H5Dread(h5d_variable, H5T_NATIVE_FLOAT, memspace, H5S_ALL, H5P_DEFAULT, fbuf);

        H5Dclose(h5d_variable);      
        H5Sclose(memspace);
    }

    // fill in 0's when necessary
    if (num_alloc_comps != num_components)
    {
        int i;
        for (i=0; i < num_data_samples; i++)
        {
            int j;
            for (j = num_components; j < num_alloc_comps; j++)
                fbuf[i * num_alloc_comps + j] = 0.0;
        }
    }

    // update last patch/var information
    last_visit_var_name = visit_var_name;
    last_patch = patch;

    return var_data;
}

// ****************************************************************************
//  Method:  avtSAMRAIFileFormat::ReadMatSpecFractions
//
//  Purpose:
//    Reads material volume and species fraction arrays for a given material
//
//  Programmer:  Mark C. Miller, adapted from GetVar 
//  Creation:    December 12, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Mon Dec 22 15:06:41 PST 2003
//    Added code to delete hdims, max_hdims.
//
//    Brad Whitlock, Fri Mar 5 10:19:32 PDT 2004
//    Changed for Windows compiler.
//
//    Mark C. Miller, Mon Aug 23 14:17:55 PDT 2004
//    Made it use OpenFile instead of H5Fopen and removed call to H5Fclose
//
// ****************************************************************************
float *
avtSAMRAIFileFormat::ReadMatSpecFractions(int patch, string mat_name,
    string spec_name)
{
    int i;

    debug5 << "avtSAMRAIFileFormat::ReadMatSpecFractions for material "
           << mat_name.c_str() << ", on patch " << patch << endl;

    // if we ever get here expecting to actually read fractions for the
    // inferred void material, something is really wrong
    if (mat_name == inferredVoidMatName)
    {
        EXCEPTION2(UnexpectedValueException, mat_name, "something other than void");
    }

    int matNo = -1;
    for (i = 0; i < num_mats; i++)
    {
        if (mat_names[i] == mat_name)
        {
            matNo = i;
            break;
        }
    }
    
    if (matNo == -1)
    {
        EXCEPTION1(InvalidVariableException, mat_name.c_str());
    }

    int dim = num_dim_problem < 3 ? num_dim_problem: 3;
    int num_data_samples = 1;
    for (i=0; i<dim; i++) {
        num_data_samples *= patch_extents[patch].upper[i] - 
                            patch_extents[patch].lower[i] + 1 +
                            2 * mat_num_ghosts[i];
    }

    char file[512];   
    sprintf(file, "%sprocessor_cluster.%05d.samrai",
        dir_name.c_str(), patch_map[patch].file_cluster_number);

    char variable[1024];
    if (spec_name == "")
    {
        sprintf(variable, "/processor.%05d/level.%05d/patch.%05d/"
                      "materials/%s/%s-fractions",
            patch_map[patch].processor_number,
            patch_map[patch].level_number,
            patch_map[patch].patch_number, 
            mat_name.c_str(), mat_name.c_str());
    }
    else
    {
        sprintf(variable, "/processor.%05d/level.%05d/patch.%05d/"
                      "materials/%s/species/%s",
            patch_map[patch].processor_number,
            patch_map[patch].level_number,
            patch_map[patch].patch_number, 
            mat_name.c_str(), spec_name.c_str());
    }

    hid_t h5f_file = OpenFile(file);
    if (h5f_file < 0)
    {
        EXCEPTION1(InvalidFilesException, file);
    }

    hid_t h5d_variable = H5Dopen(h5f_file, variable);
    if (h5d_variable < 0)
    {
        EXCEPTION1(InvalidFilesException, file);
    }

    // check dataset size agrees with what we computed for num_data_samples
    hid_t h5d_space = H5Dget_space(h5d_variable);
    int hndims = H5Sget_simple_extent_ndims(h5d_space);
    hsize_t *hdims = new hsize_t[hndims];
    hsize_t *max_hdims = new hsize_t[hndims];
    H5Sget_simple_extent_dims(h5d_space, hdims, max_hdims);
    hsize_t hsum = 1;
    for (i = 0; i < hndims; i++)
        hsum *= hdims[i];
    if ((hsize_t) num_data_samples != hsum)
    {
        EXCEPTION2(UnexpectedValueException, hsum, num_data_samples);
    }
    H5Sclose(h5d_space);
    delete [] hdims;
    delete [] max_hdims;

    float *buffer = new float[num_data_samples];

    H5Dread(h5d_variable, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            buffer);

    H5Dclose(h5d_variable);      

    return buffer;
}


// ****************************************************************************
//  Method: avtSAMRAIFileFormat::GetMaterial
//
//  Purpose:
//      Gets an avtMaterial object for a given patch 
//
//  Programmer: Mark C. Miller 
//  Creation:   December 12, 2003 
//
//  Modifications:
//    Mark C. Miller, Wed Jan  7 11:35:37 PST 2004
//    Added stuff to compute compression achieved
//
//    Mark C. Miller, Thu May  6 22:07:32 PDT 2004
//    Used new material constructors
//
//    Eric Brugger, Wed May 12 13:37:29 PDT 2004
//    Modify the allocation of an array of float pointers so that it
//    compiles on all platforms.
//
// ****************************************************************************

avtMaterial *
avtSAMRAIFileFormat::GetMaterial(int patch, const char *matObjName)
{
    int i;

    double bytesInFile = 0.0;
    static double bytesInFileTotal = 0.0;
    double bytesInMem = 0.0;
    static double bytesInMemTotal = 0.0;

    debug5 << "avtSAMRAIFileFormat::GetMaterial getting materials on patch "
           << patch << endl;

    // if we don't have any materials, there is nothing to do
    if (has_mats == false)
    {
        EXCEPTION1(InvalidVariableException, matObjName);
    }

    // first, determine which material nos., if any, we actually have on this patch
    bool oneMat = false;
    std::vector<int> matList;
    for (i = 0; i < num_mats; i++)
    {
        // since the inferred "void" material doesn't really exist, we skip it
        if (mat_names[i] == inferredVoidMatName)
            continue;

        int matCompFlag = mat_names_matinfo[mat_names[i]][patch].mat_comp_flag;

        switch (matCompFlag)
        {
            case 0: // this material covers no part of the patch 
                 break;
            case 1: // this material covers the patch, wholly
                matList.push_back(i);
                oneMat = true;
                break;
            case 2: // this material covers the patch, partially
                matList.push_back(i);
                break;
            default:
            {
                EXCEPTION2(UnexpectedValueException, matCompFlag,
                    "a value for material_composition_flag of 0,1 or 2");
            }
        }
    }

    // if we get to here, we must have some material
    if (matList.size() == 0)
    {
        // if we didn't find any materials on this patch AND we
        // are NOT inferring a void material, it must be an error
        if (inferVoidMaterial == false)
        {
            EXCEPTION1(InvalidVariableException, matObjName);
        }
        else
        {
            // this is the case where we have a clean, inferred void
            // material
            oneMat = true;
            matList.push_back(num_mats-1);
        }
    }

    // if we encountered the 'one mat' case, above, make sure we only got
    // one material
    if ((oneMat == true) && (matList.size() > 1))
    {
        EXCEPTION2(UnexpectedValueException, matList.size(), 1);
    }

    // compute logical size in each dimension of this patch
    int dim = num_dim_problem < 3 ? num_dim_problem: 3;
    int dims[] = {1, 1, 1};
    int ncells = 1;
    for (i=0; i<dim; i++)
    {
        dims[i] = patch_extents[patch].upper[i] -
                  patch_extents[patch].lower[i] + 1 +
                  2 * mat_num_ghosts[i];
        ncells *= dims[i];
    }

    // re-format global mat numbers and names 
    int *matnos = new int[num_mats];
    char **matnames = new char*[num_mats];
    for (i = 0; i < num_mats; i++)
    {
        matnos[i] = i;
        matnames[i] = CXX_strdup(mat_names[i].c_str()); 
    }

    avtMaterial *mat = NULL;

    if (oneMat)
    {
        // create a matlist array for this single material
        int *matlist = new int[ncells];
        for (i = 0; i < ncells; i++)
            matlist[i] = matList[0];

        mat = new avtMaterial(num_mats,      //silomat->nmat,
                              matnos,        //silomat->matnos,
                              matnames,      //silomat->matnames,
                              dim,           //silomat->ndims,
                              dims,          //silomat->dims,
                              0,             //silomat->major_order,
                              matlist,       //silomat->matlist,
                              0,             //silomat->mixlen,
                              0,             //silomat->mix_mat,
                              0,             //silomat->mix_next,
                              0,             //silomat->mix_zone,
                              0              //silomat->mix_vf
                              );
     

        bytesInFile = 0.0;
        bytesInMem = (ncells * sizeof(int));

    }
    else
    {
       float **vfracs = new float*[num_mats];
       for (i = 0; i < num_mats; i++)
           vfracs[i] = 0;

       // read the volume fractions for each material
       for (i = 0; i < matList.size(); i++)
       {
           vfracs[matList[i]] = ReadMatSpecFractions(patch, mat_names[matList[i]]);
           bytesInFile += (ncells * sizeof(float));
       }

       // check to see if we've hit the case where we need to infer
       // the void material for this patch and that it is ok to do so
       if (matList.size() == 1)
       {
           if (inferVoidMaterial == false)
           {
               EXCEPTION2(UnexpectedValueException, 1, "2 or more materials");
           }

           // infer the void's volume fractions
           vfracs[num_mats-1] = new float[ncells];
           for (i = 0; i < ncells; i++)
               vfracs[num_mats-1][i] = 1.0 - vfracs[matList[0]][i];
       }

       //
       // Construct the object we came here for
       //
       char domName[256];
       SNPRINTF(domName, sizeof(domName),"%d", patch);
       mat = new avtMaterial(num_mats, matnos, matnames, dim, dims, 0,
                             vfracs, domName);

       for (i = 0; i < num_mats; i++)
           SAFE_DELETE(vfracs[i]);
       SAFE_DELETE(vfracs);

       bytesInMem = (ncells * sizeof(int) + mat->GetMixlen() * (3*sizeof(int)+sizeof(float)));

   }

   // free up the matnames and numbers
   SAFE_DELETE(matnos);
   for (i = 0; i < num_mats; i++)
       SAFE_DELETE(matnames[i]);

   SAFE_DELETE(matnames);

   bytesInFileTotal += bytesInFile;
   bytesInMemTotal += bytesInMem;

   debug5 << "compression of material data on patch " << patch << " is "
          << bytesInFile / bytesInMem << "x [cummulative = "
          << bytesInFileTotal / bytesInMemTotal << "x]" << endl;

   return mat;
}

// ****************************************************************************
//  Method: avtSAMRAIFileFormat::ConvertVolumeFractionFields
//
//  Purpose: Converts full-zonal arrays of volume fraction fields into AVT's
//  (and Silo's) material field plus mix arrays representation.
//
//  Programmer: Mark C. Miller 
//  Creation:   December 12, 2003 
//
// ****************************************************************************

void
avtSAMRAIFileFormat::ConvertVolumeFractionFields(vector<int> matIds,
    float **vfracs, int ncells, int* &matfield, int &mixlen, int* &mix_mat,
    int* &mix_next, int* &mix_zone, float* &mix_vf)
{
    int i,m,z;

    // compute a value to be used as the 'notSet' value in matfield array
    int notSet = matIds[0];
    for (m = 1; m < matIds.size(); m++)
    {
        if (notSet < matIds[m])
            notSet = matIds[m];
    }
    notSet++; // one more than largest mat number

    // allocate and fill matfield array with the 'notSet' value
    matfield = new int[ncells];
    for (i = 0; i < ncells; i++)
        matfield[i] = notSet;

    // pre-compute size of mix arrays
    mixlen = 0;
    for (m = 0; m < matIds.size(); m++)
    {
        for (z = 0; z < ncells; z++)
        {
            if ((vfracs[m][z] > 0.0) &&
                (vfracs[m][z] < 1.0))
                mixlen++;
        }
    }

    // allocate mix arrays
    mix_mat  = new int[mixlen];
    mix_vf   = new float[mixlen];
    mix_zone = new int[mixlen];
    mix_next = new int[mixlen];

    // loop over materials
    mixlen = 0;
    for (m = 0; m < matIds.size(); m++)
    {

        float *frac = vfracs[m];

        // loop over zones
        for (z = 0; z < ncells; z++)
        {
            if (frac[z] == 1.0)
            {
                if (matfield[z] != notSet)
                {
                    EXCEPTION2(UnexpectedValueException, matfield[z], "the 'notSet' value");
                }

                matfield[z] = matIds[m];
            }
            else if ((1.0 > frac[z]) && (frac[z] > 0.0))
            {

                if (matfield[z] == notSet)
                {
                    // put the first entry in the list for this zone
                    matfield[z] = -(mixlen+1); 
                    mix_mat [mixlen] = matIds[m]; 
                    mix_vf  [mixlen] = frac[z];
                    mix_zone[mixlen] = z;
                    mix_next[mixlen] = 0;
                }
                else
                {
                    if (matfield[z] >= 0)
                    {
                        EXCEPTION2(UnexpectedValueException, matfield[z], "a value < 0");
                    }

                    // walk forward through the list for this zone
                    int j = -(matfield[z]+1);
                    while (mix_next[j] != 0)
                        j = mix_next[j];

                    // link up last entry in list with the new entry
                    mix_next[j] = mixlen;

                    // put in the new entry
                    mix_mat [mixlen] = matIds[m];
                    mix_vf  [mixlen] = frac[z];
                    mix_zone[mixlen] = z;
                    mix_next[mixlen] = 0;
                }

                mixlen++;
            }
            else if (frac[z] != 0.0)
            {
                EXCEPTION2(UnexpectedValueException, frac[z],
                    "a value between 0.0 and 1.0");
            }
        }
    }

#if 0
    debug5 << DebugMixedMaterials(ncells, matfield, mix_next, mix_mat,
                  mix_vf, mix_zone) << endl;
#endif

}

// ****************************************************************************
//  Method: avtSAMRAIFileFormat::DebugMixedMaterials
//
//  Purpose:
//      Checks mixed material arrays for correctness 
//
//  Programmer: Mark C. Miller 
//  Creation:   December 12, 2003 
//
// ****************************************************************************

char
avtSAMRAIFileFormat::DebugMixedMaterials(int ncells, int* &matfield,
    int* &mix_next, int* &mix_mat, float* &mix_vf, int* &mix_zone)
{
    debug5 << "parsing mixed arrays..." << endl;

    int z;
    int nclean = 0;
    int nmixed = 0;

    for (z = 0; z < ncells; z++)
    {
        if (matfield[z] >= 0)
        {
            nclean++;

            debug5 << "zone " << z << " is clean in material " << matfield[z] << endl;
        }
        else
        {
            nmixed++;

            debug5 << "zone " << z << " is mixed with materials ";

            int mixidx = -(matfield[z]+1);
            float vfsum = 0.0;

            // deal with index into first entry
            if (mixidx == 0)
            {
                debug5 << mix_mat[0] << " (" << (int) (100*mix_vf[0]) << "%), ";
                vfsum += mix_vf[0];
                mixidx = mix_next[0];
            }

            while (mixidx != 0)
            {
                debug5 << mix_mat[mixidx] << " (" << (int) (100*mix_vf[mixidx]) << "%), ";
                vfsum += mix_vf[mixidx];
                mixidx = mix_next[mixidx];
            }

            debug5 << endl;

            if (vfsum != 1.0)
                debug5 << "***VOL-FRAC SUM ERROR***" << endl;
        }
    }

    debug5 << "clean/mixed = " << nclean << "/" << nmixed << endl;

    return ' ';

}

// ****************************************************************************
//  Method: avtSAMRAIFileFormat::GetSpecies
//
//  Purpose:
//      Gets an avtSpecies object for a given patch 
//
//  Programmer: Mark C. Miller 
//  Creation:   December 18, 2003 
//
//  Modifications:
//    Mark C. Miller, Wed Jan  7 11:35:37 PST 2004
//    Added stuff to compute amount of compression achieved
//
// ****************************************************************************

avtSpecies *
avtSAMRAIFileFormat::GetSpecies(int patch, const char *specObjName)
{
    int i;
    avtMaterial* mat = 0;

    double bytesInFile = 0.0;
    static double bytesInFileTotal = 0.0;
    double bytesInMem = 0.0;
    static double bytesInMemTotal = 0.0;

    if (has_specs == false)
    {
        EXCEPTION1(InvalidVariableException, specObjName);
    }

    //
    // we need the mixed material format computed during a GetMaterial call
    //
    void_ref_ptr vrTmp = cache->GetVoidRef("materials", AUXILIARY_DATA_MATERIAL,
                                  timestep, patch);

    if (*vrTmp == 0)
    {
        void *p = (void*) GetMaterial(patch, "materials");

        vrTmp = void_ref_ptr(p, avtMaterial::Destruct);

        cache->CacheVoidRef("materials", AUXILIARY_DATA_MATERIAL, timestep,
            patch, vrTmp);

    }

    mat = (avtMaterial*) *vrTmp;


    if (mat == 0)
    {
        char str[1024];
        sprintf(str,"Unable to obtain material object on patch %d to satisfy "
                    "cooresponding query for species object \"%s\"", patch,
                    specObjName);
        EXCEPTION1(InvalidFilesException, str);
    }

    debug5 << "avtSAMRAIFileFormat::GetSpecies getting species on patch "
           << patch << endl;

    //
    // scan the material "extents" info to determine which species to read 
    //
    int totalNumSpecs = 0;
    vector<int> matList;
    for (i = 0; i < num_mats; i++)
    {
        // since the inferred "void" material doesn't really exist, we skip it
        if (mat_names[i] == inferredVoidMatName)
            continue;

        int matCompFlag = mat_names_matinfo[mat_names[i]][patch].mat_comp_flag;

        switch (matCompFlag)
        {
            case 0: // this material covers no part of the patch 
                 break;
            case 1: // this material covers the patch, wholly
            case 2: // this material covers the patch, partially
            {
                matList.push_back(i);
                totalNumSpecs += nmatspec[i];
                break;
            }
            default:
            {
                EXCEPTION2(UnexpectedValueException, matCompFlag,
                    "a value for material_composition_flag of 0,1 or 2");
            }
        }
    }

    // compute logical size in each dimension of this patch
    int dim = num_dim_problem < 3 ? num_dim_problem: 3;
    int dims[] = {1, 1, 1};
    int ncells = 1;
    for (i=0; i<dim; i++)
    {
        dims[i] = patch_extents[patch].upper[i] -
                  patch_extents[patch].lower[i] + 1 +
                  2 * mat_num_ghosts[i];
        ncells *= dims[i];
    }

    // if this patch has no species, return a clean species object
    if (totalNumSpecs == 0)
    {
        int mixlen = mat->GetMixlen();

        int *mixList = new int[mixlen];
        for (i = 0; i < mixlen; i++)
            mixList[i] = 0;

        int *specList = new int[ncells];
        for (i = 0; i < ncells; i++)
            specList[i] = 0;

        avtSpecies *spec = new avtSpecies(num_mats, nmatspec, dim, dims, specList, mixlen,
                                          mixList, 0, NULL);

        bytesInFile = 0.0;
        bytesInMem = (mixlen + ncells) * sizeof(int);

        bytesInFileTotal += bytesInFile;
        bytesInMemTotal += bytesInMem;

        debug5 << "compression of species data on patch " << patch << " is "
               << bytesInFile / bytesInMem << "x [cummulative = "
               << bytesInFileTotal / bytesInMemTotal << "x]" << endl;

        return spec;
    }


    //
    // build an array, indexed by material number, with pointers to
    // mass fractions arrays. Only those materials that are ON this
    // patch AND have more than 0 species have non-NULL mass fraction
    // arrays
    //
    int currPatchMatId = 0;
    vector<float**> matSpecFracs;
    for (i = 0; i < num_mats; i++)
    {
        if (i == matList[currPatchMatId])
        {
            string matName = mat_names[i];
            int numSpecs = nmatspec[i];

            if (numSpecs > 0)
            {
                int j;
                float **specFracs = new float*[numSpecs];

                for (j = 0; j < numSpecs; j++)
                {
                    specFracs[j] = ReadMatSpecFractions(patch, matName,
                                       mat_specs[matName][j]);
                    bytesInFile += (ncells * sizeof(float));
                }
                matSpecFracs.push_back(specFracs);
            }
            else
            {
                matSpecFracs.push_back(NULL);
            }

            // inc. to next material id to be on the
            // the lookout for
            currPatchMatId++;
        }
        else
        {
            matSpecFracs.push_back(NULL);
        }
    }

    int *specList;
    int nspecies_mf;
    float *species_mf;
    int *mixList;

    ConvertMassFractionFields(matList, matSpecFracs, ncells, mat,
        specList, nspecies_mf, species_mf, mixList);

    avtSpecies *spec = new avtSpecies(num_mats, nmatspec, dim, dims, specList,
                                      mat->GetMixlen(), mixList, nspecies_mf,
                                      species_mf);

    //
    // free up species fields 
    //
    currPatchMatId = 0;
    for (i = 0; i < num_mats; i++)
    {
        if (i == matList[currPatchMatId])
        {
            int j;
            int numSpecs = nmatspec[i];
            if (numSpecs > 0)
            {
                for (j = 0; j < numSpecs; j++)
                    SAFE_DELETE(matSpecFracs[i][j]);
            }
        }
    }

    bytesInMem = (mat->GetMixlen() + ncells) * sizeof(int) + nspecies_mf * sizeof(float);

    bytesInFileTotal += bytesInFile;
    bytesInMemTotal += bytesInMem;

    debug5 << "compression of species data on patch " << patch << " is "
           << bytesInFile / bytesInMem << "x [cummulative = "
           << bytesInFileTotal / bytesInMemTotal << "x]" << endl;

    return spec;
}

// ****************************************************************************
//  Method: avtSAMRAIFileFormat::ConvertMassFractionFields
//
//  Purpose: Convert from field per species to Silo representation
//
//  Programmer: Mark C. Miller
//  Creation:   December 20, 2003 
//
//  Modifications:
//    Mark C. Miller, Wed Jan  7 11:35:37 PST 2004
//    Added stuff to compute unique species vectors for compression
//
//    Mark C. Miller, Wed May 19 21:31:28 PDT 2004
//    Corrected off by one error in mixed mat traversal
//
// ****************************************************************************

void
avtSAMRAIFileFormat::ConvertMassFractionFields(vector<int> matIds,
    vector<float**> matSpecFracs, int ncells, avtMaterial *mat,
    int* &speclist, int &nspecies_mf, float* &species_mf, int* &mixlist)
{
    int i,j;
    const int* matlist  = mat->GetMatlist();
    const int* mix_mat  = mat->GetMixMat();
    const int* mix_next = mat->GetMixNext();
    int mixlen          = mat->GetMixlen();

#ifdef USE_UNIQUE_SPECIES
    UniqueSpeciesMF uniq_species_mf(ncells,1.5);
#else
    AutoArray<float> aaspecies_mf(ncells,ncells,true);
#endif
    speclist = new int[ncells];
    mixlist  = new int[mixlen];

    nspecies_mf = 1;

    for (i = 0; i < ncells; i++)
    {
        int matNum = matlist[i];

        if (matNum >= 0) // clean zone
        {
            // if this material has species, it should have species mf pointers
            if ((nmatspec[matNum] != 0) && (matSpecFracs[matNum] == NULL))
            {
                EXCEPTION2(UnexpectedValueException, nmatspec[matNum], 0);
            }

            if (nmatspec[matNum] == 0)
                speclist[i] = 0;
            else
            {
#ifdef USE_UNIQUE_SPECIES
                float *mf_vec = new float[nmatspec[matNum]];
                for (j = 0; j < nmatspec[matNum]; j++)
                    mf_vec[j] = matSpecFracs[matNum][j][i];
                speclist[i] = uniq_species_mf.IndexOfMFVector(mf_vec, nmatspec[matNum]);
                delete [] mf_vec;
#else
                speclist[i] = nspecies_mf;
                for (j = 0; j < nmatspec[matNum]; j++)
                    aaspecies_mf[nspecies_mf++] = matSpecFracs[matNum][j][i];
#endif
            }
        }
        else // mixed zone
        {
            speclist[i] = -INT_MAX; 
            int mixidx = -(matlist[i]+1);
            while (true)
            {
                matNum = mix_mat[mixidx];

                // if this material has species, it should have species mf pointers
                if ((nmatspec[matNum] != 0) && (matSpecFracs[matNum] == NULL))
                {
                    EXCEPTION2(UnexpectedValueException, nmatspec[matNum], 0);
                }

#ifdef USE_UNIQUE_SPECIES
                float *mf_vec = new float[nmatspec[matNum]];
                for (j = 0; j < nmatspec[matNum]; j++)
                    mf_vec[j] = matSpecFracs[matNum][j][i];
                mixlist[mixidx] = uniq_species_mf.IndexOfMFVector(mf_vec, nmatspec[matNum]);
                delete [] mf_vec;
#else
                mixlist[mixidx] = nspecies_mf;
                for (j = 0; j < nmatspec[matNum]; j++)
                    aaspecies_mf[nspecies_mf++] = matSpecFracs[matNum][j][i];
#endif

                if (mix_next[mixidx] == 0)
                    break;
                mixidx = mix_next[mixidx]-1;
            }
        }
    }

#ifdef USE_UNIQUE_SPECIES
    species_mf = uniq_species_mf.GetData();
    nspecies_mf = uniq_species_mf.GetSize();
#else
    species_mf = aaspecies_mf.GetData();
    if (nspecies_mf != aaspecies_mf.GetSize())
    {
        EXCEPTION2(UnexpectedValueException, nspecies_mf, aaspecies_mf.GetSize());
    }
#endif
}

// ****************************************************************************
//  Method: avtSAMRAIFileFormat::GetAuxiliaryData
//
//  Purpose:
//      Gets auxiliary data about the file format.
//
//  Programmer: Walter Herrera Jimenez
//  Creation:   July 21, 2003
//
//  Modifications:
//    Eric Brugger, Tue Dec 30 16:37:26 PST 2003
//    I modified the routine to properly read vector (and coordinate) data.
//    This involved file format as well as reader changes.  In this case
//    I modified the way the routine built the interval tree from the
//    extents associated with a vector.
//
// ****************************************************************************

void *
avtSAMRAIFileFormat::GetAuxiliaryData(const char *var, int patch,
                                      const char *type, void *,
                                      DestructorFunction &df)
{
    string name = var;
    void *rv = NULL;
    avtIntervalTree *itree = NULL;

    if (strcmp(type, AUXILIARY_DATA_DATA_EXTENTS) == 0) {
        
        debug5 << "avtSAMRAIFileFormat::GetAuxiliaryData getting DATA_EXTENTS" << endl;

        std::map<std::string, var_t>::const_iterator cur_var;
        cur_var = var_names_num_components.find(name);
        if (cur_var == var_names_num_components.end())
        {
            EXCEPTION1(InvalidVariableException, var);
        }
    
        int num_components = (*cur_var).second.num_components;
        
        if (num_components == 1 ) {
          
            itree = new avtIntervalTree(num_patches, 1);
            for (int v = 0; v < num_vars; v++) {
                if (var_names[v] == name) {

                    for (int patch = 0 ; patch < num_patches ; patch++) {

                        if (var_extents[v][patch].data_is_defined) {
                            float range[2] = { var_extents[v][patch].min, 
                                               var_extents[v][patch].max };
                            itree->AddDomain(patch, range);
                        }
                        else {
                           // if the varible does no have values, what will we add?
                        }
                    }
                    break;
                }
            }

        } else {
            itree = new avtIntervalTree(num_patches, 3);
            for (int patch = 0 ; patch < num_patches; patch++) {
                bool data_defined = true;
                float range[6] = { 0, 0, 0, 0, 0, 0 };
                int dim = num_components <= 3 ? num_components : 3;
                for (int i=0; i<dim; i++) {
                    int ipatch = i * num_patches + patch;

                    for (int v = 0; v < num_vars; v++) {
                        if (var_names[v] == name) {
                            if (var_extents[v][ipatch].data_is_defined) {
                                range[i*2] = var_extents[v][ipatch].min; 
                                range[i*2+1] = var_extents[v][ipatch].max;
                            }
                            else {
                                data_defined = false;
                            }
                            break;
                        }
                    }
                }
                if (data_defined) {
                    itree->AddDomain(patch, range);
                }
                else {
                  // if the varible does no have values, what do we add?
                }
            }

        }
        itree->Calculate(true);
 
        rv = (void *) itree;
        df = avtIntervalTree::Destruct;

    }
    else if (strcmp(type, AUXILIARY_DATA_SPATIAL_EXTENTS) == 0) {

        debug5 << "avtSAMRAIFileFormat::GetAuxiliaryData getting SPATIAL_EXTENTS" << endl;

        itree = new avtIntervalTree(num_patches, 3);
        for (int patch = 0 ; patch < num_patches ; patch++) {
            float bounds[] = {0, 0, 0, 0, 0, 0};
            int dim = num_dim_problem < 3 ? num_dim_problem: 3;

            for (int j=0; j<dim; j++) {
                bounds[j*2] = patch_extents[patch].xlo[j];
                bounds[j*2+1] = patch_extents[patch].xup[j];
            }
            itree->AddDomain(patch, bounds);
        }
        itree->Calculate(true);

        rv = (void *) itree;
        df = avtIntervalTree::Destruct;
    }
    else if (strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        rv = (void *) GetMaterial(patch, var);
        df = avtMaterial::Destruct;
    }
    else if (strcmp(type, AUXILIARY_DATA_SPECIES) == 0)
    {
        rv = (void *) GetSpecies(patch, var);
        df = avtSpecies::Destruct;
    }

    return rv;
}


// ****************************************************************************
//  Method:  avtSAMRAIFileFormat::PopulateIOInformation
//
//  Purpose:
//    Popolates IO Information Hints
//
//  Programmer:  Mark C. Miller 
//  Creation:    August 19. 2004 
//
// ****************************************************************************

void
avtSAMRAIFileFormat::PopulateIOInformation(avtIOInformation &ioInfo)
{
    if (num_patches <= 1)
    {
        debug5 << "No need to do I/O optimization because there is only "
               << "one patch" << endl;
        ioInfo.SetNDomains(1);
        return;
    }

    if (num_clusters <= 1)
    {
        debug5 << "No need to do I/O optimization because there is only "
               << "one file for all patches" << endl;
        ioInfo.SetNDomains(num_patches);
        return;
    }

    //
    // create an initial vector for each file cluster
    //
    vector< vector<int> > groups;
    for (int i = 0; i < num_clusters; i++)
    {
        vector<int> dummy;
        groups.push_back(dummy);
    }

    //
    // use the patch map and stick each patch into its respective cluster group
    //
    for (int i = 0; i < num_patches; i++)
    {
        int clusterNum = patch_map[i].file_cluster_number;
        groups[clusterNum].push_back(i);
    }

    ioInfo.SetNDomains(num_patches);
    ioInfo.AddHints(groups);
}

// ****************************************************************************
//  Method:  avtSAMRAIFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//    Returns meta-data about the database.
//
//  Arguments:
//    md         The meta-data structure to populate
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    June 19, 2003
//
//  Modifications:
//
//    Hank Childs, Wed Apr 14 08:02:14 PDT 2004
//    Back out undocumented changes made by Mark Miller with adding material
//    numbers to material names.
//
//    Mark C. Miller, Mon Aug 23 14:17:55 PDT 2004
//    Added code to set cycle/time info
//
// ****************************************************************************

void
avtSAMRAIFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    int i;

    debug5 << "avtSAMRAIFileFormat::PopulateDatabaseMetaData getting data" << endl;

    {
        static const char *mesh_name = "amr_mesh";

        ReadMetaDataFile();

        //
        // Set cycle/time info
        //
        md->SetCycle(timestep, time_step_number);
        md->SetCycleIsAccurate(true, timestep);
        md->SetTime(timestep, time);
        md->SetTimeIsAccurate(true, timestep);

        avtMeshMetaData *mesh = new avtMeshMetaData;
        mesh->name = mesh_name;

        mesh->meshType = AVT_RECTILINEAR_MESH;
        mesh->topologicalDimension = num_dim_problem;
        mesh->spatialDimension = num_dim_problem; // should really be set rank of "Coords"
        mesh->hasSpatialExtents = false;

        // spoof a group/domain mesh
        mesh->numBlocks = num_patches;
        mesh->blockTitle = "patches";
        mesh->blockPieceName = "patch";
        mesh->numGroups = num_levels;
        mesh->groupTitle = "levels";
        mesh->groupPieceName = "level";
        vector<int> groupIds(num_patches);
        vector<string> blockPieceNames(num_patches);
        for (i = 0; i < num_patches; i++)
        {
           char tmpName[64];
           sprintf(tmpName,"level%d,patch%d",patch_map[i].level_number, patch_map[i].patch_number);

           groupIds[i] = patch_map[i].level_number;
           blockPieceNames[i] = tmpName;
        }
        mesh->blockNames = blockPieceNames;
        md->Add(mesh);
        md->AddGroupInformation(num_levels, num_patches, groupIds);

        //
        // add material information, if any
        //
        if (has_mats == true)
        {
            // re-format the material names for avt's API
            vector<string> matnames;
            for (i = 0; i < num_mats; i++)
                matnames.push_back(mat_names[i]);

            // add the material object
            avtMaterialMetaData *mmd = new avtMaterialMetaData("materials", mesh_name,
                                               num_mats, matnames);
            md->Add(mmd);

            //
            // add species information, if any
            //
            if (has_specs == true)
            {
                int j, k;
                vector<int> nmatspecVector;
                vector<vector<string> > spec_names;

                for (j = 0; j < num_mats; j++)
                {
                    vector<string>  tmp_string_vector;

                    nmatspecVector.push_back(nmatspec[j]); 

                    for (k = 0; k < nmatspec[j]; k++)
                        tmp_string_vector.push_back(mat_specs[mat_names[j]][k]);

                    spec_names.push_back(tmp_string_vector);
                }

                avtSpeciesMetaData *smd = new avtSpeciesMetaData("species",
                                               mesh_name, "materials", num_mats,
                                               nmatspecVector, spec_names);
                md->Add(smd);
            }
        }

        //
        // add default plot
        //
        avtDefaultPlotMetaData *plot = new avtDefaultPlotMetaData("Subset_1.0", "levels");
        char attribute[250];
        sprintf(attribute,"%d NULL ViewerPlot", INTERNAL_NODE);
        plot->AddAttribute(attribute);
        sprintf(attribute,"%d ViewerPlot SubsetAttributes", INTERNAL_NODE);
        plot->AddAttribute(attribute);
        sprintf(attribute,"%d SubsetAttributes lineWidth 1", INT_NODE);
        plot->AddAttribute(attribute);
        sprintf(attribute,"%d SubsetAttributes wireframe true", BOOL_NODE);
        plot->AddAttribute(attribute);
        md->Add(plot);
        
        //
        // add scalar and vector variables
        //
        std::map<std::string, var_t>::const_iterator var_it;
        for (var_it = var_names_num_components.begin();
             var_it != var_names_num_components.end(); 
             var_it++) {

            if ((*var_it).first == "Coords") {
                continue;
            }

            char *var_name = new char[(*var_it).first.size() + 20];
            sprintf(var_name, "%s", (*var_it).first.c_str());
          
            // set the centering
            avtCentering centering;
            if ( (*var_it).second.cell_centered == 1)
                centering = AVT_ZONECENT;
            else
                centering = AVT_NODECENT;

            if ((*var_it).second.num_components == 1)                    // scalar field
            {
                AddScalarVarToMetaData(md, var_name, mesh_name, centering);
            }
            else if ((*var_it).second.num_components == num_dim_problem) // vector field
            {
                AddVectorVarToMetaData(md, var_name, mesh_name, centering, 3);
            }
            else if (((num_dim_problem == 2) && ((*var_it).second.num_components == 3)) ||
                     ((num_dim_problem == 3) && ((*var_it).second.num_components == 6)))
            {
                AddSymmetricTensorVarToMetaData(md, var_name, mesh_name, centering,
                    (*var_it).second.num_components);
            }
            else if (((num_dim_problem == 2) && ((*var_it).second.num_components == 4)) ||
                     ((num_dim_problem == 3) && ((*var_it).second.num_components == 9)))
            {
                AddTensorVarToMetaData(md, var_name, mesh_name, centering,
                    (*var_it).second.num_components);
            }
            else {
              EXCEPTION1(InvalidVariableException, (*var_it).first.c_str());
            }

            delete [] var_name;
        }

        // the code below to create a custom SIL is currently disabled
#if 0
        // build the custom SIL
        avtSILMetaData *sil = new avtSILMetaData(mesh_name);

        // create the collection class for levels
        int levelCollId = sil->AddCollectionClass("levels", "level", num_levels);

        // create the collections of patches on each level
        int *tmpIndices = new int[num_patches];
        int globalPatchIndex = 0;
        for (int i = 0; i < num_levels; i++)
        {

           // find all patches in global patch map on this level
           int levelPatchIndex = 0;
           while (patch_map[globalPatchIndex].level_number == i)
              tmpIndices[levelPatchIndex++] = globalPatchIndex++;

           // add a collection of patches underneath this level
           int localPatchCollId =
               sil->AddCollection("patches", "patch", levelPatchIndex, levelCollId,
                                  i, -1, NULL);

        }
        delete [] tmpIndices;

        // create the collections of levels patches on each patch

        md->Add(sil);
#endif

    }
}

// ****************************************************************************
//  Method:  avtSAMRAIFileFormat::ReadMetaDataFile
//
//  Purpose:
//    Read the database meta-data (root) file
//
//  Arguments:
//    in         the open ifstream to read from
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    July 07, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Dec  8 12:03:06 PST 2003
//    Added call to ReadAndCheckVDRVersions(); 
//    Added call to ReadVarNumGhosts();
//
//    Brad Whitlock, Fri Mar 5 10:19:32 PDT 2004
//    Changed for Windows compiler.
//
//    Mark C. Miller, Mon Aug 23 14:17:55 PDT 2004
//    Made it use OpenFile instead of H5Fopen & removed call to H5Fclose
//
// ****************************************************************************
void
avtSAMRAIFileFormat::ReadMetaDataFile()
{
    debug5 << "avtSAMRAIFileFormat::ReadMetaDataFile reading SAMRAI summary "
        "file, \"" << file_name.c_str() << "\"" << endl;

    hid_t h5_file = OpenFile(file_name.c_str());

    if (h5_file < 0)
    {
        debug1 << "Unable to open metadata file " << file_name.c_str() << endl;
    }
    else
    {

        ReadAndCheckVDRVersion(h5_file);

        ReadTimeStepNumber(h5_file);
        ReadTime(h5_file);
        ReadTimeOfDump(h5_file);

        ReadGridType(h5_file);

        ReadNumDimensions(h5_file);
        ReadNumLevels(h5_file);

        ReadXLO(h5_file);
        ReadDX(h5_file);

        ReadNumPatches(h5_file);
        ReadNumPatchesLevel(h5_file);
        ReadRatiosCoarserLevels(h5_file);

        ReadNumClusters(h5_file);
        ReadNumProcessors(h5_file);

        ReadNumVariables(h5_file);
        ReadVarCellCentered(h5_file);
        ReadVarNames(h5_file);
        ReadVarNumGhosts(h5_file);
        ReadVarNumComponents(h5_file);

        ReadVarExtents(h5_file);
        ReadPatchExtents(h5_file);
        ReadPatchMap(h5_file);

        ReadChildArrayLength(h5_file);
        ReadChildArray(h5_file);
        ReadChildPointerArray(h5_file);
        ReadParentArrayLength(h5_file);
        ReadParentArray(h5_file);
        ReadParentPointerArray(h5_file);

        ReadMaterialInfo(h5_file);

        ReadSpeciesInfo(h5_file);

        cached_patches = new vtkDataSet**[num_patches];
        for (int p=0; p<num_patches; p++)
        {
            if (has_ghost)
            {
                cached_patches[p] = new vtkDataSet*[MAX_GHOST_CODES];
                for (int g = 0; g<MAX_GHOST_CODES; g++)
                    cached_patches[p][g] = NULL;
            }
            else
            {
                cached_patches[p] = new vtkDataSet*[1];
                cached_patches[p][0] = NULL;
            }
        }
    }
}



// ****************************************************************************
//  Method:  ReadTime
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadTime(hid_t &h5_file)
{

    bool isOptional = false;
    int numVals = 1;
    double *ptime = &time;
    ReadDataset(h5_file, "/BASIC_INFO/time", "double", 1, &numVals,
        (void**) &ptime, isOptional);
}


// ****************************************************************************
//  Method:  ReadAndCheckVDRVersion
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Mark C. Miller 
//  Creation:    December 8, 2003 
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadAndCheckVDRVersion(hid_t &h5_file)
{
//    cerr << "WARNING, SAMRAI VERSION CHECK DISABLED" << endl;
//    return;
   
    hid_t h5_dataset = H5Dopen(h5_file,"/BASIC_INFO/VDR_version_number");
    if (h5_dataset < 0) {
        char str[1024];
        sprintf(str, "%s::/BASIC_INFO/VDR_version_number does not exist. Unable "
            "to confirm file's version compatibility with reader plugin", file_name.c_str());
        EXCEPTION1(InvalidFilesException, str);
    }

    float obtained_version_number;
    H5Dread(h5_dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
            &obtained_version_number);
    H5Dclose(h5_dataset);

    if (expected_version_number != obtained_version_number)
    {
        char str[2048];
        sprintf(str, "The file \"%s\" appears to be a SAMRAI file "
            "written for input to VisIt. However, the version of the writer "
            "SAMRAI used to produce this file, %f, does not match the version of "
            "the VisIt reader plugin you are now trying to use to read it, %f",
             file_name.c_str(),obtained_version_number,expected_version_number);
        EXCEPTION1(InvalidFilesException, str);
    }

}

// ****************************************************************************
//  Method:  ReadTimeStepNumber
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadTimeStepNumber(hid_t &h5_file)
{
    bool isOptional = false;
    int numVals = 1;
    int *ptsn = &time_step_number;
    ReadDataset(h5_file, "/BASIC_INFO/time_step_number", "int", 1, &numVals,
        (void**) &ptsn, isOptional);
}

// ****************************************************************************
//  Method:  ReadTimeOfDump
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadTimeOfDump(hid_t &h5_file)
{
    bool isOptional = false;
    int numVals = 1;
    string *pstr = &time_dump;
    ReadDataset(h5_file, "/BASIC_INFO/time_of_dump", "string", 1, &numVals,
        (void**) &pstr, isOptional);
}

// ****************************************************************************
//  Method:  ReadNumDimensions
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadNumDimensions(hid_t &h5_file)
{
    bool isOptional = false;
    int numVals = 1;
    int *pnum_dim = &num_dim_problem;
    ReadDataset(h5_file, "/BASIC_INFO/number_dimensions_of_problem", "int", 1,
        &numVals, (void**) &pnum_dim, isOptional);
}


// ****************************************************************************
//  Method:  ReadXLO
//
//  Purpose:
//    Read the lower coordinates of the grid
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadXLO(hid_t &h5_file)
{
    bool isOptional = false;
    int numVals = 3;
    ReadDataset(h5_file, "/BASIC_INFO/XLO", "double", 1, &numVals,
        (void**) &xlo, isOptional);
}


// ****************************************************************************
//  Method:  ReadNumLevels
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadNumLevels(hid_t &h5_file)
{
    bool isOptional = false;
    int numVals = 1;
    int *pnl = &num_levels;
    ReadDataset(h5_file, "/BASIC_INFO/number_levels", "int", 1, &numVals,
        (void**) &pnl, isOptional);
}


// ****************************************************************************
//  Method:  ReadDx
//
//  Purpose:
//    Read the width of the cells on each level
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadDX(hid_t &h5_file)
{
    bool isOptional = false;
    int dims[2] = {num_levels,3};
    ReadDataset(h5_file, "/BASIC_INFO/dx", "double", 2, dims, (void**) &dx,
        isOptional);
}


// ****************************************************************************
//  Method:  ReadNumPatches
//
//  Purpose:
//    Read the number of global patches
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadNumPatches(hid_t &h5_file)
{
    hid_t h5_dataset = H5Dopen(h5_file, "/BASIC_INFO/number_global_patches");
    if (h5_dataset < 0) {
        char str[1024];
        sprintf(str, "%s::/BASIC_INFO/number_global_patches", file_name.c_str());
        EXCEPTION1(InvalidFilesException, str);
    }

    H5Dread(h5_dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            &num_patches);
    H5Dclose(h5_dataset);    
}


// ****************************************************************************
//  Method:  ReadNumPatchesLevel
//
//  Purpose:
//    Read the number of patches on each level
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadNumPatchesLevel(hid_t &h5_file)
{
    hid_t h5_dataset = H5Dopen(h5_file,"/BASIC_INFO/number_patches_at_level");
    if (h5_dataset < 0) {
        char str[1024];
        sprintf(str, "%s::/BASIC_INFO/number_patches_at_level", 
                file_name.c_str());
        EXCEPTION1(InvalidFilesException, str);
    }

    num_patches_level = new int[num_levels];

    H5Dread(h5_dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            num_patches_level);
    H5Dclose(h5_dataset);    
}


// ****************************************************************************
//  Method:  ReadRatiosCoarserLevels
//
//  Purpose:
//    Read the ratios to coarser levels
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadRatiosCoarserLevels(hid_t &h5_file)
{
    hid_t h5_dataset = H5Dopen(h5_file, 
                               "/BASIC_INFO/ratios_to_coarser_levels");
    if (h5_dataset < 0) {
        char str[1024];
        sprintf(str, "%s::/BASIC_INFO/ratios_to_coarser_levels", 
                file_name.c_str());
        EXCEPTION1(InvalidFilesException, str);
    }

    ratios_coarser_levels = new int[num_levels * 3];

    H5Dread(h5_dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            ratios_coarser_levels);
    H5Dclose(h5_dataset);    
}


// ****************************************************************************
//  Method:  ReadNumClusters
//
//  Purpose:
//    Read the number of clusters
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadNumClusters(hid_t &h5_file)
{
    hid_t h5_dataset = H5Dopen(h5_file, "/BASIC_INFO/number_file_clusters");
    if (h5_dataset < 0) {
        char str[1024];
        sprintf(str, "%s::/BASIC_INFO/number_file_clusters", 
                file_name.c_str());
        EXCEPTION1(InvalidFilesException, str);
    }

    H5Dread(h5_dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            &num_clusters);
    H5Dclose(h5_dataset);    
}

    
// ****************************************************************************
//  Method:  ReadNumProcessors
//
//  Purpose:
//    Read the number of processors
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadNumProcessors(hid_t &h5_file)
{
    hid_t h5_dataset = H5Dopen(h5_file, "/BASIC_INFO/number_processors");
    if (h5_dataset < 0) {
        char str[1024];
        sprintf(str, "%s::/BASIC_INFO/number_processors", file_name.c_str());
        EXCEPTION1(InvalidFilesException, str);
    }

    H5Dread(h5_dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            &num_procs);
    H5Dclose(h5_dataset);    
}

    
// ****************************************************************************
//  Method:  ReadNumVariables
//
//  Purpose:
//    Read the number of variables
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadNumVariables(hid_t &h5_file)
{
    hid_t h5_dataset = H5Dopen(h5_file, "/BASIC_INFO/number_visit_variables");
    if (h5_dataset < 0) {
        char str[1024];
        sprintf(str, "%s::/BASIC_INFO/number_visit_variables", 
                file_name.c_str());
        EXCEPTION1(InvalidFilesException, str);
    }

    H5Dread(h5_dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            &num_vars);
    H5Dclose(h5_dataset);    
}

    
// ****************************************************************************
//  Method:  ReadVarCellCentered
//
//  Purpose:
//    Read the cell centered flag for each variable
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadVarCellCentered(hid_t &h5_file)
{
    if (num_vars <= 0)
        return;

    hid_t h5_dataset = H5Dopen(h5_file, "/BASIC_INFO/var_cell_centered");
    if (h5_dataset < 0) {
        char str[1024];
        sprintf(str, "%s::/BASIC_INFO/var_cell_centered", file_name.c_str());
        EXCEPTION1(InvalidFilesException, str);
    }

    var_cell_centered = new int[num_vars];

    H5Dread(h5_dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            var_cell_centered);
    H5Dclose(h5_dataset);    
}


// ****************************************************************************
//  Method:  ReadGridType
//
//  Purpose:
//    Read the type of the grid
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadGridType(hid_t &h5_file)
{
    hid_t h5_dataset = H5Dopen(h5_file, "/BASIC_INFO/grid_type");
    if (h5_dataset < 0) {
        char str[1024];
        sprintf(str, "%s::/BASIC_INFO/grid_type", file_name.c_str());
        EXCEPTION1(InvalidFilesException, str);
    }

    hid_t h5_disk_datatype = H5Tcopy(h5_dataset);
    int datatype_size = H5Tget_size(h5_disk_datatype);

    hid_t h5_mem_datatype = H5Tcopy(H5T_C_S1);
    H5Tset_size(h5_mem_datatype, datatype_size);  

    char *buffer = new char[datatype_size];

    H5Dread(h5_dataset, h5_mem_datatype, H5S_ALL,H5S_ALL, H5P_DEFAULT, buffer);

    H5Tclose(h5_mem_datatype);
    H5Tclose(h5_disk_datatype);
    H5Dclose(h5_dataset);        

    grid_type = buffer;
    delete [] buffer;
}

// ****************************************************************************
//  Method:  ReadVarNames
//
//  Purpose:
//    Read the name of the variables
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
//  Modificaitons:
//    Mark C. Miller, changed to use ReadDataset helper function
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadVarNames(hid_t &h5_file)
{
    if (num_vars <= 0)
        return;

    bool isOptional = false;
    ReadDataset(h5_file, "/BASIC_INFO/var_names", "string", 1, &num_vars,
        (void**) &var_names, isOptional);
}


// ****************************************************************************
//  Method:  ReadVarNumComponents
//
//  Purpose:
//    Read the number of components of each variable
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
//  Modifications:
//    Mark C. Miller ,Thu Dec 11 14:43:24 PST 2003
//    Modified to be a little more general
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadVarNumComponents(hid_t &h5_file)
{
    bool isOptional = false;
    ReadDataset(h5_file, "/BASIC_INFO/var_number_components", "int",
        1, &num_vars, (void**) &var_num_components, isOptional);

    for (int v = 0; v < num_vars; v++) {
      string var_name = var_names[v].c_str();
      var_t var = { var_num_components[v], var_cell_centered[v],
                  { var_num_ghosts[v*3+0],
                    var_num_ghosts[v*3+1],
                    var_num_ghosts[v*3+2]}}; 
      var_names_num_components[var_name] = var;
    }

}


// ****************************************************************************
//  Method:  ReadVarNumGhosts
//
//  Purpose:
//    Read the number of ghost cells in each dimensio. Also, set flags
//    indicating if we have ghosting and if ghosting is consistent.
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Mark C. Miller
//  Creation:    December 8, 2003 
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadVarNumGhosts(hid_t &h5_file)
{
    if (num_vars <= 0)
        return;

    int dims[2] = {-1, 3};
    ReadDataset(h5_file, "/BASIC_INFO/var_number_ghosts", "int",
        2, dims, (void**) &var_num_ghosts);

    if (dims[0] == 0)
    {
        var_num_ghosts = new int[num_vars * 3];
        for (int v = 0; v < num_vars * 3; v++)
            var_num_ghosts[v] = 0;
        has_ghost = false;
        ghosting_is_consistent = true;
        return;
    }

    if (dims[0] != num_vars)
    {
        EXCEPTION2(UnexpectedValueException, dims[0], num_vars);
    }

    // When ghosting is not same for ALL variables in the file, we need
    // to tell VisIt NOT to cache pieces of the mesh above the plugin.
    // So, we examine the ghost information here for differences between
    // variables. If there are no differences, things can proceed as
    // normal. Otherwise, we need to turn off caching above the plugin.
    // We do this by setting a boolean indicating if ghosting is
    // consistent or not. Also, for the mesh 'variable', we need to know
    // the maximal amount of ghosting, so we compute that here too
    ghosting_is_consistent = true;
    for (int v = 1; v < num_vars; v++) {
        if ((var_num_ghosts[v*3+0] != var_num_ghosts[0*3+0]) ||
            (var_num_ghosts[v*3+1] != var_num_ghosts[0*3+1]) ||
            (var_num_ghosts[v*3+2] != var_num_ghosts[0*3+2]))
                ghosting_is_consistent = false;
        if (var_num_ghosts[v*3+0] > var_max_ghosts[0])
            var_max_ghosts[0] = var_num_ghosts[v*3+0];
        if (var_num_ghosts[v*3+1] > var_max_ghosts[1])
            var_max_ghosts[1] = var_num_ghosts[v*3+1];
        if (var_num_ghosts[v*3+2] > var_max_ghosts[2])
            var_max_ghosts[2] = var_num_ghosts[v*3+2];
    }

    // finally, lets just make sure we actually do have ghost zones
    if (ghosting_is_consistent && (var_num_ghosts[0] == 0) &&
                                  (var_num_ghosts[1] == 0) &&
                                  (var_num_ghosts[2] == 0))
        has_ghost = false;
    else
        has_ghost = true;


}

// ****************************************************************************
//  Method:  ReadVarExtents
//
//  Purpose:
//    Read the extents for each variable
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
//  Modifications:
//    Eric Brugger, Tue Dec 30 16:37:26 PST 2003
//    I modified the routine to properly read vector (and coordinate) data.
//    This involved file format as well as reader changes.  In this case
//    I modified the way the extents were read and stored for a vector.
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadVarExtents(hid_t &h5_file)
{
    if (num_vars <= 0)
        return;

    hid_t h5_mem_datatype = H5Tcreate (H5T_COMPOUND, sizeof(var_extents_t));
    H5Tinsert (h5_mem_datatype, "data_is_defined", 
               HOFFSET(var_extents_t,data_is_defined), H5T_NATIVE_CHAR);
    H5Tinsert (h5_mem_datatype, "min", 
               HOFFSET(var_extents_t,min), H5T_NATIVE_DOUBLE);
    H5Tinsert (h5_mem_datatype, "max", 
               HOFFSET(var_extents_t,max), H5T_NATIVE_DOUBLE);

    var_extents = new var_extents_t* [num_vars];
    for (int v = 0; v < num_vars; v++) {
        std::map<std::string, var_t>::const_iterator cur_var;
        cur_var = var_names_num_components.find(var_names[v]);
        if (cur_var == var_names_num_components.end())
        {
            EXCEPTION1(InvalidVariableException, var_names[v]);
        }
    
        int num_components = (*cur_var).second.num_components;

        var_extents[v] = new var_extents_t[num_patches*num_components];
      
        if (num_components == 1)
        {
            char ds_name[50];
            sprintf(ds_name, "/extents/%s-Extents", var_names[v].c_str());
      
            hid_t h5_dataset = H5Dopen(h5_file, ds_name);
            if (h5_dataset < 0) {
                char str[1024];
                sprintf(str, "%s::%s", file_name.c_str(), ds_name);
                EXCEPTION1(InvalidFilesException, str);
            }

            H5Dread(h5_dataset, h5_mem_datatype, H5S_ALL, H5S_ALL,
                    H5P_DEFAULT, var_extents[v]);
            H5Dclose(h5_dataset);    
        }
        else
        {
            for (int c = 0; c < num_components; c++)
            {
                char ds_name[50];
                sprintf(ds_name, "/extents/%s.%02d-Extents",
                        var_names[v].c_str(), c);
      
                hid_t h5_dataset = H5Dopen(h5_file, ds_name);
                if (h5_dataset < 0) {
                    char str[1024];
                    sprintf(str, "%s::%s", file_name.c_str(), ds_name);
                    EXCEPTION1(InvalidFilesException, str);
                }

                H5Dread(h5_dataset, h5_mem_datatype, H5S_ALL, H5S_ALL,
                        H5P_DEFAULT, &(var_extents[v][c*num_patches]));
                H5Dclose(h5_dataset);    
            }
        }
    }

    H5Tclose(h5_mem_datatype);
}


// ****************************************************************************
//  Method:  ReadPatchExtents
//
//  Purpose:
//    Read the spatial extents for each patch
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 12 17:29:19 PST 2004
//    Added code to override xlo/xup (spatial) extents of each patch with
//    knowledge of extents of coordinate array if its an ALE grid
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadPatchExtents(hid_t &h5_file)
{
    hid_t h5_dataset = H5Dopen(h5_file, "/extents/patch_extents");
    if (h5_dataset < 0) {
        char str[1024];
        sprintf(str, "%s::/extents/patch_extents", file_name.c_str());
        EXCEPTION1(InvalidFilesException, str);
    }

    patch_extents = new patch_extents_t[num_patches];

    hsize_t dim[] = {3};
    hid_t h5_lower_datatype = H5Tarray_create(H5T_NATIVE_INT, 1, dim, NULL);
    hid_t h5_upper_datatype = H5Tarray_create(H5T_NATIVE_INT, 1, dim, NULL);
    hid_t h5_xlo_datatype = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, dim, NULL);
    hid_t h5_xup_datatype = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, dim, NULL);

    int size = H5Tget_size(h5_lower_datatype) + 
               H5Tget_size(h5_upper_datatype) + 
               H5Tget_size(h5_xlo_datatype) + 
               H5Tget_size(h5_xup_datatype);
    hid_t h5_datatype = H5Tcreate (H5T_COMPOUND, size);

    /*
    int offset_lower = HOFFSET(patch_extents_t, lower);
    int offset_upper = HOFFSET(patch_extents_t, upper);
    int offset_xlo   = HOFFSET(patch_extents_t, xlo);
    int offset_xup   = HOFFSET(patch_extents_t, xup);
    */
    
    int offset_lower = 0;
    int offset_upper = H5Tget_size(h5_lower_datatype);
    int offset_xlo   = offset_upper + H5Tget_size(h5_upper_datatype);
    int offset_xup   = offset_xlo + H5Tget_size(h5_xlo_datatype);
    
    H5Tinsert(h5_datatype, "lower", offset_lower, h5_lower_datatype);
    H5Tinsert(h5_datatype, "upper", offset_upper, h5_upper_datatype);
    H5Tinsert(h5_datatype, "xlo", offset_xlo, h5_xlo_datatype);
    H5Tinsert(h5_datatype, "xup", offset_xup, h5_xup_datatype);

    H5Dread(h5_dataset, h5_datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            patch_extents);
    H5Dclose(h5_dataset);       

    H5Tclose(h5_lower_datatype);
    H5Tclose(h5_upper_datatype);
    H5Tclose(h5_xlo_datatype);
    H5Tclose(h5_xup_datatype);
    H5Tclose(h5_datatype);

    // override xlo/xup data with extents from the "Coord" variable
    if (grid_type == "ALE" || grid_type == "DEFORMED")
    {
        // find the index for the coordinate field
        int v;
        for (v = 0; v < num_vars; v++)
        {
            if (var_names[v] == "Coords")
                break;
        }

        // now move the var_extents data to patch_extents
        int p;
        for (p = 0; p < num_patches; p++)
        {
            patch_extents[p].xlo[0] = var_extents[v][0*num_patches].min;
            patch_extents[p].xup[0] = var_extents[v][0*num_patches].max;
            patch_extents[p].xlo[1] = var_extents[v][1*num_patches].min;
            patch_extents[p].xup[1] = var_extents[v][1*num_patches].max;
            patch_extents[p].xlo[2] = var_extents[v][2*num_patches].min;
            patch_extents[p].xup[2] = var_extents[v][2*num_patches].max;
        }
    }
}


// ****************************************************************************
//  Method:  ReadPatchMap
//
//  Purpose:
//    Read the patch map: cluster, processor, level and local number of patch
//    for each patch
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadPatchMap(hid_t &h5_file)
{
    hid_t h5_dataset = H5Dopen(h5_file, "/extents/patch_map");
    if (h5_dataset < 0) {
        char str[1024];
        sprintf(str, "%s::/extents/patch_map", file_name.c_str());
        EXCEPTION1(InvalidFilesException, str);
    }

    hid_t h5_datatype = H5Tcreate (H5T_COMPOUND, sizeof(patch_map_t));
    H5Tinsert (h5_datatype, "processor_number", 
               HOFFSET(patch_map_t,processor_number), H5T_NATIVE_INT);
    H5Tinsert (h5_datatype, "file_cluster_number", 
               HOFFSET(patch_map_t,file_cluster_number), H5T_NATIVE_INT);
    H5Tinsert (h5_datatype, "level_number", 
               HOFFSET(patch_map_t,level_number), H5T_NATIVE_INT);
    H5Tinsert (h5_datatype, "patch_number", 
               HOFFSET(patch_map_t,patch_number), H5T_NATIVE_INT);

    patch_map = new patch_map_t[num_patches];
      
    H5Dread(h5_dataset, h5_datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, patch_map);
    H5Dclose(h5_dataset);      
    H5Tclose(h5_datatype);
}


// ****************************************************************************
//  Method:  ReadChildArrayLength
//
//  Purpose:
//    Read the length of the array of children of each patch
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadChildArrayLength(hid_t &h5_file)
{
    hid_t h5_dataset = H5Dopen(h5_file, "/BASIC_INFO/child_array_length");
    if (h5_dataset < 0) {
        char str[1024];
        sprintf(str, "%s::/BASIC_INFO/child_array_length", file_name.c_str());
        EXCEPTION1(InvalidFilesException, str);
    }

    H5Dread(h5_dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            &child_array_length);
    H5Dclose(h5_dataset);    
}


// ****************************************************************************
//  Method:  ReadChildArray
//
//  Purpose:
//    Read the array of children of each patch
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadChildArray(hid_t &h5_file)
{
    if (child_array_length == 0)
        return;

    hid_t h5_dataset = H5Dopen(h5_file, "/BASIC_INFO/child_array");
    if (h5_dataset < 0) {
        char str[1024];
        sprintf(str, "%s::/BASIC_INFO/child_array", file_name.c_str());
        EXCEPTION1(InvalidFilesException, str);
    }

    child_array = new int[child_array_length];

    H5Dread(h5_dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            child_array);
    H5Dclose(h5_dataset);    
}


// ****************************************************************************
//  Method:  ReadChildPointerArray
//
//  Purpose:
//    Read the array of pointers to children of each patch
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadChildPointerArray(hid_t &h5_file)
{
    if (child_array_length == 0)
        return;

    hid_t h5_dataset = H5Dopen(h5_file, "/BASIC_INFO/child_pointer_array");
    if (h5_dataset < 0) {
        char str[1024];
        sprintf(str, "%s::/BASIC_INFO/child_pointer_array", file_name.c_str());
        EXCEPTION1(InvalidFilesException, str);
    }

    hid_t h5_datatype = H5Tcreate (H5T_COMPOUND, sizeof(child_t));
    H5Tinsert (h5_datatype, "offset", 
               HOFFSET(child_t,offset), H5T_NATIVE_INT);
    H5Tinsert (h5_datatype, "number_children", 
               HOFFSET(child_t,number_children), H5T_NATIVE_INT);

    child_pointer_array = new child_t[num_patches];
      
    H5Dread(h5_dataset, h5_datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            child_pointer_array);
    H5Dclose(h5_dataset);      
    H5Tclose(h5_datatype);
}


// ****************************************************************************
//  Method:  ReadParentArrayLength
//
//  Purpose:
//    Read the length of the array of parents of each patch
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadParentArrayLength(hid_t &h5_file)
{
    hid_t h5_dataset = H5Dopen(h5_file, "/BASIC_INFO/parent_array_length");
    if (h5_dataset < 0) {
        char str[1024];
        sprintf(str, "%s::/BASIC_INFO/parent_array_length", file_name.c_str());
        EXCEPTION1(InvalidFilesException, str);
    }

    H5Dread(h5_dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            &parent_array_length);
    H5Dclose(h5_dataset);    
}


// ****************************************************************************
//  Method:  ReadParentArray
//
//  Purpose:
//    Read the array of parents of each patch
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadParentArray(hid_t &h5_file)
{
    if (parent_array_length == 0)
        return;

    hid_t h5_dataset = H5Dopen(h5_file, "/BASIC_INFO/parent_array");
    if (h5_dataset < 0) {
        char str[1024];
        sprintf(str, "%s::/BASIC_INFO/parent_array", file_name.c_str());
        EXCEPTION1(InvalidFilesException, str);
    }

    parent_array = new int[parent_array_length];

    H5Dread(h5_dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            parent_array);
    H5Dclose(h5_dataset);    
}


// ****************************************************************************
//  Method:  ReadParentPointerArray
//
//  Purpose:
//    Read the array of pointers to parents of each patch
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadParentPointerArray(hid_t &h5_file)
{
    if (parent_array_length == 0)
        return;

    hid_t h5_dataset = H5Dopen(h5_file, "/BASIC_INFO/parent_pointer_array");
    if (h5_dataset < 0) {
        char str[1024];
        sprintf(str, "%s::/BASIC_INFO/parent_pointer_array", file_name.c_str());
        EXCEPTION1(InvalidFilesException, str);
    }

    hid_t h5_datatype = H5Tcreate (H5T_COMPOUND, sizeof(parent_t));
    H5Tinsert (h5_datatype, "offset", 
               HOFFSET(parent_t,offset), H5T_NATIVE_INT);
    H5Tinsert (h5_datatype, "number_parents", 
               HOFFSET(parent_t,number_parents), H5T_NATIVE_INT);

    parent_pointer_array = new parent_t[num_patches];      

    H5Dread(h5_dataset, h5_datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            parent_pointer_array);
    H5Dclose(h5_dataset);      
    H5Tclose(h5_datatype);
}


// ****************************************************************************
//  Method:  ReadMaterialInfo
//
//  Purpose:
//    Read material information from the metadata file 
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Mark C. Miller 
//  Creation:    December 12, 2003 
//
// ****************************************************************************
void
avtSAMRAIFileFormat::ReadMaterialInfo(hid_t &h5_file)
{
    // read material names
    num_mats = -1;
    bool isOptional = true;
    ReadDataset(h5_file, "/materials/material_names", "string", 1, &num_mats,
        (void**) &mat_names, isOptional);
    if (num_mats > 0)
        has_mats = true;
    else
        has_mats = false;

    // if we don't have any materials, we're done
    if (has_mats == false)
        return;

    // read names of material-specific variables
    num_mat_vars = -1;
    mat_var_names = 0;
    ReadDataset(h5_file, "/materials/material_variable_names", "string",
        1, &num_mat_vars, (void**) &mat_var_names, isOptional);
    if (num_mat_vars == -1)
        num_mat_vars = 0;

    // if we do have materials, all the remaining information is required 
    isOptional = false;

    // read information on ghosting for material variables
    int numVals = 3;
    ReadDataset(h5_file, "/materials/material_number_ghosts", "int",
        1, &numVals, (void**) &mat_num_ghosts, isOptional);

    // read information on components counts of material-specific variables
    ReadDataset(h5_file, "/materials/material_variable_number_components", "int",
        1, &num_mat_vars, (void**) &mat_var_num_components, isOptional);

    for (int mv = 0; mv < num_mat_vars; mv++) {
      string mat_var_name = mat_var_names[mv].c_str();
      var_t var = { var_num_components[mv], 1,
                  { mat_num_ghosts[0],
                    mat_num_ghosts[1],
                    mat_num_ghosts[2]}}; 
      mat_var_names_num_components[mat_var_name] = var;
    }

    // Read "extents" information for each material 
    for (int m = 0; m < num_mats; m++)
    {
        matinfo_t *tmpInfo = 0;

        char dsName[1024];
        sprintf(dsName,"/extents/materials/%s/%s-Fractions",
            mat_names[m].c_str(), mat_names[m].c_str());
        ReadDataset(h5_file, dsName, "matinfo_t", 1, &num_patches,
            (void**) &tmpInfo, isOptional);

        mat_names_matinfo[mat_names[m]] = tmpInfo;

        for (int mv = 0; mv < num_mat_vars; mv++)
        {
            tmpInfo = 0;
            char dsName[1024];
            sprintf(dsName,"/extents/materials/%s/material_variables/%s",
                mat_names[m].c_str(), mat_var_names[mv].c_str());
            ReadDataset(h5_file, dsName, "matinfo_t", 1, &num_patches,
                (void**) &tmpInfo, isOptional);
            
            mat_var_names_matinfo[mat_names[m]][mat_var_names[mv]] = tmpInfo;
        }
    }

    // check if we'll ever need to infer the "void" material for this database
    inferVoidMaterial = false;
    int p;
    for (p = 0; p < num_patches; p++)
    {
        int i, n0 = 0, n1 = 0, n2 = 0;
        for (i = 0; i < num_mats; i++)
        {
            int matCompFlag = mat_names_matinfo[mat_names[i]][p].mat_comp_flag;
            switch (matCompFlag)
            {
                case 0: // this material covers no part of the patch 
                    n0++;
                    break;
                case 1: // this material covers the patch, wholly
                    n1++;
                    break;
                case 2: // this material covers the patch, partially
                    n2++;
                    break;
            }
        }

        if (n1 == 0 && n2 == 1)
        {
            inferVoidMaterial = true;
            break;
        }
    }

    // if we need to infer the "void" material, adjust the material count/names 
    if (inferVoidMaterial)
    {
        string *new_mat_names = new string[num_mats+1];

        // copy over the existing names
        int i;
        for (i = 0; i < num_mats; i++)
            new_mat_names[i] = mat_names[i];
        new_mat_names[num_mats] = inferredVoidMatName; 

        SAFE_DELETE(mat_names);
        mat_names = new_mat_names;

        num_mats++;
    }
}

// ****************************************************************************
//  Method:  ReadSpeciesInfo
//
//  Purpose:
//    Read species information from the metadata file 
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Mark C. Miller 
//  Creation:    December 18, 2003 
//
// ****************************************************************************
void
avtSAMRAIFileFormat::ReadSpeciesInfo(hid_t &h5_file)
{
    // read species variable names
    num_spec_vars = -1;
    bool isOptional = true;

    ReadDataset(h5_file, "/materials/species_variable_names", "string", 1, &num_spec_vars,
        (void**) &spec_var_names, isOptional);

    if (num_spec_vars > 0)
        has_specs = true;
    else
        has_specs = false;

    // if we don't have any species, we're done
    if (has_specs == false)
        return;

    nmatspec = new int[num_mats];

    // for each material, try to obtain the names of its constituent species 
    int i;
    bool atLeastOneMaterialHasSpecies = false;
    for (i = 0; i < num_mats; i++)
    {
        char matSpecName[512];
        sprintf(matSpecName,"/materials/species/%s", mat_names[i].c_str());

        string* tmpString = 0;
        if (mat_names[i] != inferredVoidMatName)
        {
            nmatspec[i] = -1;
            ReadDataset(h5_file, matSpecName, "string", 1, &nmatspec[i],
                (void**) &tmpString, isOptional);
        }
        else
            nmatspec[i] = 0;

        // if the given material has no constituent species, its count is 0
        if (nmatspec[i] == 0)
        {
            mat_specs[mat_names[i]] = 0;
        }
        else
        {
            mat_specs[mat_names[i]] = tmpString;
            atLeastOneMaterialHasSpecies = true;
        }
    }

    // if no material was found to have constituent species, ignore species data
    if (atLeastOneMaterialHasSpecies == false)
        return;

    // if we do have species, all the remaining information is required 
    isOptional = false;

    // Read "extents" information for each species 
    for (int m = 0; m < num_mats; m++)
    {
        int s;
        for (s = 0; s < nmatspec[m]; s++)
        {
            matinfo_t *tmpInfo = 0;
            char dsName[1024];

            string matName = mat_names[m];
            string specName = mat_specs[matName][s];

            sprintf(dsName,"/extents/materials/%s/species/%s",
                matName.c_str(), specName.c_str());

            ReadDataset(h5_file, dsName, "matinfo_t", 1, &num_patches,
                (void**) &tmpInfo, isOptional);
            
            mat_specs_matinfo[matName][specName] = tmpInfo;
        }
    }
}

// ****************************************************************************
//  Method:  BuildDomainAuxiliaryInfo 
//
//  Purpose:
//    Builds the domain nesting information object and caches it 
//
//  Programmer:  Mark C. Miller 
//  Creation:    October 13, 2003 
//
//  Modifications:
//
//     Mark C. Miller, Mon Nov 10 09:55:59 PST 2003
//     Added code to check to see if its in cache before we try to build it.
//     This makes it ok to include a call to BuildDomainAuxiliaryInfo() in the
//     GetMesh method without serious performance drawbacks.
//
//     Hank Childs, Wed Nov 12 17:58:16 PST 2003
//     Also create the rectilinear domain boundaries structure.
//
//     Mark C. Miller, Tue Dec  9 10:45:46 PST 2003
//     Changed name from BuildDomainNestingInfo to BuildDomainAuxiliaryInfo
//     since it does more than build domain nesting information
//
//     Mark C. Miller, Mon Jan 12 17:29:19 PST 2004
//     I switched to use avtCurvilinearDomainBoundaries when its in ALE
//     mesh
//
//     Brad Whitlock, Fri Mar 5 10:19:32 PDT 2004
//     Changed for Windows compiler.
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::BuildDomainAuxiliaryInfo()
{

    // first, look to see if we don't already have it cached
    void_ref_ptr vrTmp = cache->GetVoidRef("any_mesh",
                                   AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
                                  timestep, -1);

    if ((*vrTmp == NULL) && (child_array_length > 0))
    {

        //
        // build the avtDomainNesting object
        //
        avtStructuredDomainNesting *dn =
            new avtStructuredDomainNesting(num_patches, num_levels);

        dn->SetNumDimensions(num_dim_problem);

        //
        // Set refinement level ratio information
        //
        vector<int> ratios(3,1);
        dn->SetLevelRefinementRatios(0, ratios);
        int i;
        for (i = 1; i < num_levels; i++)
        {
           vector<int> ratios(3);
           ratios[0] = ratios_coarser_levels[3*i+0];
           ratios[1] = ratios_coarser_levels[3*i+1];
           ratios[2] = ratios_coarser_levels[3*i+2];
           dn->SetLevelRefinementRatios(i, ratios);
        }

        //
        // set each domain's level, children and logical extents
        //
        for (i = 0; i < num_patches; i++)
        {
            vector<int> childPatches;
            for (int j = 0; j < child_pointer_array[i].number_children; j++)
            {
               int offset = child_pointer_array[i].offset;
               childPatches.push_back(child_array[offset+j]);
            }

            vector<int> logExts(6);
            logExts[0] = patch_extents[i].lower[0];
            logExts[1] = patch_extents[i].lower[1];
            logExts[2] = patch_extents[i].lower[2];
            logExts[3] = patch_extents[i].upper[0];
            logExts[4] = patch_extents[i].upper[1];
            logExts[5] = patch_extents[i].upper[2];

            dn->SetNestingForDomain(i, patch_map[i].level_number,
                childPatches, logExts);
        }

        void_ref_ptr vr = void_ref_ptr(dn, avtStructuredDomainNesting::Destruct);

        cache->CacheVoidRef("any_mesh", AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
            timestep, -1, vr);
    }

    void_ref_ptr dbTmp = cache->GetVoidRef("any_mesh",
                                           AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION,
                                           timestep, -1);
    if ((*dbTmp == NULL) && !has_ghost)
    {
        bool canComputeNeighborsFromExtents = true;
        avtStructuredDomainBoundaries *sdb = 0;

        if (grid_type == "ALE" || grid_type == "DEFORMED")
            sdb = new avtCurvilinearDomainBoundaries(canComputeNeighborsFromExtents);
        else
            sdb = new avtRectilinearDomainBoundaries(canComputeNeighborsFromExtents);

        sdb->SetNumDomains(num_patches);
        for (int i = 0 ; i < num_patches ; i++)
        {
            int e[6];
            e[0] = patch_extents[i].lower[0];
            e[1] = patch_extents[i].upper[0]+1;
            e[2] = patch_extents[i].lower[1];
            e[3] = patch_extents[i].upper[1]+1;
            e[4] = patch_extents[i].lower[2];
            e[5] = patch_extents[i].upper[2]+1;
            sdb->SetIndicesForAMRPatch(i, patch_map[i].level_number, e);
        }
        sdb->CalculateBoundaries();
        void_ref_ptr vsdb = void_ref_ptr(sdb,avtStructuredDomainBoundaries::Destruct);
        cache->CacheVoidRef("any_mesh",
                            AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION,
                            timestep, -1, vsdb);

    }
}

// ****************************************************************************
//  Method:  GetGhostCodeForVar
//
//  Purpose:
//    Returns the ghost code for a given variable. The ghost code is a simple
//    conversion from a base-MAX_GHOST_LAYERS integer of num_dim_problem digits
//    to a base-10 integer.
//
//  Programmer:  Mark C. Miller 
//  Creation:    December 9, 2003 
//
// ****************************************************************************
int
avtSAMRAIFileFormat::GetGhostCodeForVar(const char *visit_var_name)
{
    if (!has_ghost)
        return 0;

    string var_name = visit_var_name;
    int num_ghosts[3];

    if (var_name == "amr_mesh")
    {
        num_ghosts[0] = var_max_ghosts[0];
        num_ghosts[1] = var_max_ghosts[1];
        num_ghosts[2] = var_max_ghosts[2];
    }
    else
    {
        std::map<std::string, var_t>::const_iterator cur_var;
        cur_var = var_names_num_components.find(var_name);
 
        if (cur_var == var_names_num_components.end())
            return 0;

        num_ghosts[0] = (*cur_var).second.num_ghosts[0];
        num_ghosts[1] = (*cur_var).second.num_ghosts[1];
        num_ghosts[2] = (*cur_var).second.num_ghosts[2];
    }

    int code = 0;
    int powr = 1;
    for (int i = 0; i < num_dim_problem; i++)
    {
        code += num_ghosts[i] * powr; 
        powr *= MAX_GHOST_LAYERS;
    }

    return code;
}


// ****************************************************************************
//  Method:  ReadDataset
//
//  Purpose:
//    Read a dataset that is a 1D array of string-values and return it as
//    a vector of strings
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Mark C. Miller, adapted from Walter Herrar Jimenez
//  Creation:    December 11, 2003 
//
//  Modifications:
//     Kathleen Bonnell, Mon Dec 22 15:06:41 PST 2003
//     Added code dynamically allocate hdims max_hdims, so that code will build
//     on IRIX. 
//
//     Brad Whitlock, Fri Mar 5 10:19:32 PDT 2004
//     Changed for Windows compiler.
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadDataset(hid_t &hdfFile, const char *dsPath,
    const char *typeName, int ndims, int *dims, void **data, bool isOptional) 
{

    // confirm total number of entries > 0
    int i;
    for (i = 0; i < ndims; i++)
    {
        if (dims[i] == 0)
            return;
    }

    hid_t h5_dataset = H5Dopen(hdfFile, dsPath);
    if (h5_dataset < 0)
    {
        if (isOptional)
        {
            for (i = 0; i < ndims; i++)
                dims[i] = 0;
            *data = 0;
            return;
        }
        else
        {
            char str[1024];
            sprintf(str, "Required dataset \"%s\", not present in file \"%s\"",
                dsPath, file_name.c_str());
            EXCEPTION1(InvalidFilesException, str);
        }
    }

    // determine # entries in dataset
    hid_t h5d_space = H5Dget_space(h5_dataset);
    int hndims = H5Sget_simple_extent_ndims(h5d_space);
    if (hndims != ndims)
    {
        H5Dclose(h5_dataset);
        EXCEPTION2(UnexpectedValueException, hndims, ndims);
    }
    hsize_t *hdims = new hsize_t[hndims];
    hsize_t *max_hdims = new hsize_t[hndims];
    H5Sget_simple_extent_dims(h5d_space, hdims, max_hdims);
    H5Sclose(h5d_space);

    int num_vals = 1;
    for (i = 0; i < hndims; i++)
    {
        if (dims[i] == -1)
            dims[i] = hdims[i];
        else
        {
            if (dims[i] != hdims[i])
            {
                H5Dclose(h5_dataset);
                EXCEPTION2(UnexpectedValueException, hdims[i], dims[i]);
            }
        }

        num_vals *= dims[i];
    }
    delete [] hdims;
    delete [] max_hdims;

    // if we don't have anything to read or we've only be called to return
    // the size, just return now
    if ((num_vals == 0) || (data == 0))
    {
        if (data != 0)
            *data = 0;
        H5Dclose(h5_dataset);
        return;
    }

    if (strncmp(typeName,"string",6) == 0)
    {

        hid_t h5_disk_datatype = H5Tcopy(h5_dataset);
        int datatype_size = H5Tget_size(h5_disk_datatype);

        hid_t h5_mem_datatype = H5Tcopy(H5T_C_S1);
        H5Tset_size(h5_mem_datatype, datatype_size);  

        char *buffer = new char[num_vals * datatype_size];
        string *tmp_data;

        if (*data == 0)
            tmp_data = new std::string[num_vals];
        else
            tmp_data = (string*) *data;

        // read the data
        H5Dread(h5_dataset, h5_mem_datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
                buffer);

        // close up everything
        H5Tclose(h5_mem_datatype);
        H5Tclose(h5_disk_datatype);

        for (int v = 0; v < num_vals; v++) {
          tmp_data[v] = &buffer[v * datatype_size];
        }

        *data = (void*) tmp_data;

        delete [] buffer;
    }
    else if (strncmp(typeName,"int",3) == 0)
    {
        int *tmp_data;

        if (*data == 0)
            tmp_data = new int[num_vals];
        else
            tmp_data = (int *) *data;

        H5Dread(h5_dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            tmp_data);

        *data = (void *) tmp_data;
    }
    else if (strncmp(typeName,"double",6) == 0)
    {
        double *tmp_data;

        if (*data == 0)
            tmp_data = new double[num_vals];
        else
            tmp_data = (double *) *data;

        H5Dread(h5_dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            tmp_data);

        *data = (void *) tmp_data;
    }
    else if (strncmp(typeName,"float",6) == 0)
    {
        float *tmp_data;

        if (*data == 0)
            tmp_data = new float[num_vals];
        else
            tmp_data = (float *) *data;

        H5Dread(h5_dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            tmp_data);

        *data = (void *) tmp_data;
    }
    else if (strncmp(typeName,"matinfo_t",9) == 0)
    {
        // build the hdf5 type
        hid_t h5_mem_datatype = H5Tcreate (H5T_COMPOUND, sizeof(matinfo_t));
        H5Tinsert (h5_mem_datatype, "data_is_defined", 
                   HOFFSET(matinfo_t,data_is_defined), H5T_NATIVE_UCHAR);
        H5Tinsert (h5_mem_datatype, "material_composition_flag", 
                   HOFFSET(matinfo_t,mat_comp_flag), H5T_NATIVE_INT);
        H5Tinsert (h5_mem_datatype, "species_composition_flag", 
                   HOFFSET(matinfo_t,spec_comp_flag), H5T_NATIVE_INT);
        H5Tinsert (h5_mem_datatype, "min", 
                   HOFFSET(matinfo_t,min), H5T_NATIVE_DOUBLE);
        H5Tinsert (h5_mem_datatype, "max", 
                   HOFFSET(matinfo_t,max), H5T_NATIVE_DOUBLE);

        matinfo_t *tmp_data;

        if (*data == 0)
            tmp_data = new matinfo_t[num_vals];
        else
            tmp_data = (matinfo_t*) *data;

        H5Dread(h5_dataset, h5_mem_datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            tmp_data);

        H5Tclose(h5_mem_datatype);

        *data = (void *) tmp_data;
    }


    H5Dclose(h5_dataset);    

}

// ****************************************************************************
//  Function:  GetDirName
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


int   UniqueSpeciesMF::DefaultSize = DEFAULT_SIZE;
float UniqueSpeciesMF::DefaultMinc = DEFAULT_MINC;

// ****************************************************************************
//  Constructor:  UniqueSpeciesMF
//
//  Programmer:  Mark C. Miller 
//  Creation:    January 6, 2004
//
// ****************************************************************************
UniqueSpeciesMF::UniqueSpeciesMF(int initSize, float _minc)
{
    // sanity checks
    if (initSize < 0)
        initSize = DefaultSize;
    if (_minc < 1.0)
        _minc = DefaultMinc;

    dataRetrieved = false;

    max_nspecies_mf = initSize;
    minc = _minc;

    nspecies_mf = 0;
    species_mf = (float *) malloc(max_nspecies_mf * sizeof(float));
}

// ****************************************************************************
//  Destructor:  ~UniqueSpeciesMF
//
//  Programmer:  Mark C. Miller 
//  Creation:    January 6, 2004
//
// ****************************************************************************
UniqueSpeciesMF::~UniqueSpeciesMF()
{
    if (!dataRetrieved && species_mf != 0)
        free(species_mf);
}

// ****************************************************************************
//  Method:  UniqueSpeciesMF::GetSize
//
//  Purpose: Returns the current size of the species_mf array 
//
//  Programmer:  Mark C. Miller 
//  Creation:    January 6, 2004
//
// ****************************************************************************
int
UniqueSpeciesMF::GetSize()
{
    return nspecies_mf;
}

// ****************************************************************************
//  Method:  UniqueSpeciesMF::GetMaxSize
//
//  Purpose: Returns the current max size of the species_mf array 
//
//  Programmer:  Mark C. Miller 
//  Creation:    January 6, 2004
//
// ****************************************************************************
int
UniqueSpeciesMF::GetMaxSize()
{
    return max_nspecies_mf;
}

// ****************************************************************************
//  Method:  UniqueSpeciesMF::GetData
//
//  Purpose: Returns the species_mf array 
//
//  Programmer:  Mark C. Miller 
//  Creation:    January 6, 2004
//
// ****************************************************************************
float *
UniqueSpeciesMF::GetData()
{
    if (dataRetrieved)
        return 0;

    dataRetrieved = true;

    // downsize the array as necessary
    if (nspecies_mf < max_nspecies_mf)
    {
        species_mf = (float*) realloc((void*)species_mf,
                                      nspecies_mf * sizeof(float)); 
        max_nspecies_mf = nspecies_mf;
    }

    float *retval = species_mf;
    species_mf = 0;

    return retval;
}

// ****************************************************************************
//  Method:  UniqueSpeciesMF::IndexOfMFVector
//
//  Purpose: Installs a new species vector in the species_mf array and returns
//  its index or returns the index of an existing matching vector.
//
//  Programmer:  Mark C. Miller 
//  Creation:    January 6, 2004
//
// ****************************************************************************
int
UniqueSpeciesMF::IndexOfMFVector(float *mf_vector, int nvals)
{
    if (dataRetrieved)
        return -1;

    // compute a key based on Bob Burtle's hashing algorithm
    unsigned int hval = BJHash::Hash((unsigned char*)mf_vector,
                                     nvals * sizeof(float), 0xdeadbeef);

    // look up the key in the map, if it isn't found STL will create the entry
    // using the default constructor
    int retval = vmap[hval].idx;

    // if the key wasn't found, we've got a new species mf vector, so add it
    if (retval == -1)
    {
        // increase size of species_mf array if necessary
        if ((nspecies_mf + nvals) >= max_nspecies_mf)
        {
              max_nspecies_mf = (int) (max_nspecies_mf * minc);
            species_mf = (float*) realloc((void*)species_mf,
                                          max_nspecies_mf * sizeof(float)); 
        }

        // put the index for this mf vector into the map 
        vmap[hval].idx = nspecies_mf;
        retval = nspecies_mf;

        // put the mf vector in the species_mf array
        int i;
        for (i = 0; i < nvals; i++)
            species_mf[nspecies_mf++] = mf_vector[i];
    }

    return retval;
    
}
