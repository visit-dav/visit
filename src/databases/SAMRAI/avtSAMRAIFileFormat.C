// ***************************************************************************
//                              avtSAMRAIFileFormat.C                         
//
//  Purpose:  Sample database which reads multi-domain multi-timestep files
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    July 7, 2003
//
// ****************************************************************************


#include <avtSAMRAIFileFormat.h>

#include <vector>
#include <string>
#include <stdlib.h>

#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkFloatArray.h>
#include <vtkPoints.h>

#include <avtDatabaseMetaData.h>
#include <avtIntervalTree.h>
#include <avtStructuredDomainBoundaries.h>
#include <avtVariableCache.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>


using std::vector;
using std::string;

static string GetDirName(const char *path);
static string GetNameLevel(const char *name_level_str, unsigned int &level, char link);


// ****************************************************************************
//  Constructor:  avtSAMRAIFileFormat::avtSAMRAIFileFormat
//
//  Arguments:
//    fname      the file name of the root metadata file
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    July 07, 2003
//
// ****************************************************************************
avtSAMRAIFileFormat::avtSAMRAIFileFormat(const char *fname)
    : avtSTMDFileFormat(&fname, 1)
{
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
    patch_map = NULL;
    child_array = NULL;
    child_pointer_array = NULL;
    parent_array = NULL;
    parent_pointer_array = NULL;

    num_patches = 0;


    dir_name = GetDirName(fname);
    file_name = fname;

    hid_t hfile = H5Fopen(file_name.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT); 

    if (hfile < 0)
    {
        debug1 << "Unable to open metadata file " << fname << endl;
    }
    else
    {
        ReadMetaDataFile(hfile);
    }
    H5Fclose(hfile);

    cached_patches = new vtkDataSet*[num_patches];
    for (int p=0; p<num_patches; p++)
        cached_patches[p] = NULL;
}

// ****************************************************************************
//  Destructor:  avtSAMRAIFileFormat::~avtSAMRAIFileFormat
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    June 19, 2003
//
// ****************************************************************************
avtSAMRAIFileFormat::~avtSAMRAIFileFormat()
{
    if (xlo != NULL)  delete[] xlo;
    xlo = NULL;

    if (dx != NULL)  delete[] dx;
    dx = NULL;

    if (num_patches_level != NULL) delete[] num_patches_level;
    num_patches_level = NULL;

    if (ratios_coarser_levels != NULL) delete[] ratios_coarser_levels;
    ratios_coarser_levels = NULL;

    if (var_cell_centered != NULL) delete[] var_cell_centered;
    var_cell_centered = NULL;

    if (patch_extents != NULL) delete[] patch_extents;
    patch_extents = NULL;
    
    if (var_extents != NULL) {
        for(int v=0; v<num_vars; v++) {
            delete[] var_extents[v];
        }
        delete[] var_extents;
    }
    var_extents = NULL;

    if (var_names != NULL) delete[] var_names;
    var_names = NULL;

    if (var_num_components != NULL) delete[] var_num_components;
    var_num_components = NULL;

    if (patch_map != NULL) delete[] patch_map;
    patch_map = NULL;

    if (child_array != NULL) delete[] child_array;
    child_array = NULL;

    if (child_pointer_array != NULL) delete[] child_pointer_array;
    child_pointer_array = NULL;

    if (parent_array != NULL) delete[] parent_array;
    parent_array = NULL;

    if (parent_pointer_array != NULL) delete[] parent_pointer_array;
    parent_pointer_array = NULL;

    if (cached_patches != NULL) {
        for (int p=0; p<num_patches; p++) {
            if (cached_patches[p] != NULL )
                // XXX is it necessary??
                cached_patches[p]->Delete();
        }
        delete[] cached_patches;
    }
    cached_patches = NULL;
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
// ****************************************************************************
vtkDataSet *
avtSAMRAIFileFormat::GetMesh(int patch, const char *level_name)
{
    unsigned level;
    string name = GetNameLevel(level_name, level, '_');
    if (level >= num_levels)
    {
        EXCEPTION1(InvalidVariableException, level_name);
    }

    unsigned offset = GetPatchOffset(level, patch);

    if (cached_patches[offset] != NULL)
    {
        // The reference count will be decremented by the generic database,
        // because it will assume it owns it.
        cached_patches[offset]->Register(NULL);
        return cached_patches[offset];
    }

    vtkDataSet *ds = ReadMesh(patch, level_name);
    cached_patches[offset] = ds;
    ds->Register(NULL);

    return ds;
}

// ****************************************************************************
//  Method:  avtSAMRAIFileFormat::ReadMesh
//
//  Purpose:
//    Reads the mesh with the given name for the given time step and
//    domain from the file.  NOTE: time step is ignored in this case
//    because the mesh does not change over time.
//
//  Arguments:
//    ts         the time step
//    dom        the domain number
//    mesh       the name of the mesh to read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    July 11, 2003
//
// ****************************************************************************
vtkDataSet *
avtSAMRAIFileFormat::ReadMesh(int patch, const char *level_name)
{
    // timestep (ts) is unused because the mesh is constant over time
    // should really interact with variable cache (see Exodus format)
    // so the mesh is only read once for all time steps

    unsigned level;
    string name = GetNameLevel(level_name, level, '_');
    if (level >= num_levels)
        EXCEPTION1(InvalidVariableException, level_name);
 

    unsigned offset = GetPatchOffset(level, patch);
    int dimensions[] = {1, 1, 1};
    float spacing[] = {0, 0, 0};
    float origin[] = {0, 0, 0};

    int dim = num_dim_problem < 3 ? num_dim_problem: 3;

    for (int i=0; i<dim; i++) {
        origin[i] = patch_extents[offset].xlo[i];
        dimensions[i] = patch_extents[offset].upper[i] -
                        patch_extents[offset].lower[i] + 2;
        spacing[i] = dx[level * num_dim_problem + i];
    }

    if (grid_type == "RECTILINEAR") {
        vtkFloatArray  *coords[3];
        for (int i = 0 ; i < 3 ; i++)
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
    
        return rGrid;
    }
    else if (grid_type == "ALE") {
        char var_name[100];
        sprintf(var_name, "Coords_%05d", level);
        vtkDataArray * array = GetVectorVar(patch, var_name);
        vtkStructuredGrid  *sGrid = vtkStructuredGrid::New(); 
        vtkPoints *points = vtkPoints::New();
        points->SetNumberOfPoints(array->GetNumberOfTuples());

        for (unsigned i=0; i<array->GetNumberOfTuples(); i++) {
            float p[3];
            array->GetTuple(i, p);
            points->SetPoint(i,p);
        }

        sGrid->SetDimensions(dimensions);
        sGrid->SetPoints(points);

        points->Delete();
        array->Delete();
        return sGrid;
    }
    else {
        EXCEPTION1(InvalidVariableException, level_name);
    }
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
// ****************************************************************************
vtkDataArray *
avtSAMRAIFileFormat::GetVar(int patch, const char *visit_var_name)
{
    unsigned int level;
    string var_name = GetNameLevel(visit_var_name, level, '_');    

    if (level >= num_levels)
        EXCEPTION1(InvalidVariableException, visit_var_name);

    if (patch < 0 || patch >= num_patches_level[level])
        EXCEPTION1(InvalidVariableException, visit_var_name);


    std::map<std::string, var_t>::const_iterator cur_var;
    cur_var = var_names_num_components.find(var_name);

    if (cur_var == var_names_num_components.end())
        EXCEPTION1(InvalidVariableException, visit_var_name);
    
    int num_components = (*cur_var).second.num_components;
    int cell_centered =  (*cur_var).second.cell_centered;
        
    if (num_components != 1 )
        EXCEPTION1(InvalidVariableException, visit_var_name);

    unsigned int offset = GetPatchOffset(level, patch);

    for (int v = 0; v < num_vars; v++) {
        if (var_names[v] == var_name) {
            if (!var_extents[v][offset].data_is_defined) {
                return NULL;
            }
            break;
        }
    }

    int extent = cell_centered == 1 ? 1 : 2;
    int dim = num_dim_problem < 3 ? num_dim_problem: 3;
    int num_data_samples = 1;
    for (int i=0; i<dim; i++) {
        num_data_samples *= patch_extents[offset].upper[i] - 
                            patch_extents[offset].lower[i] + extent;
    }

    char file[2048];   
    sprintf(file, "%sprocessor_cluster.%05d.samrai",
            dir_name.c_str(), patch_map[offset].file_cluster_number);

    char variable[100];
    sprintf(variable, "/processor.%05d/level.%05d/patch.%05d/%s",
            patch_map[offset].processor_number, level, patch, 
            var_name.c_str());


    hid_t h5f_file = H5Fopen(file, H5F_ACC_RDONLY, H5P_DEFAULT); 
    if (h5f_file < 0)
        EXCEPTION1(InvalidFilesException, file);

    hid_t h5d_variable = H5Dopen(h5f_file, variable);
    if (h5d_variable < 0)
        EXCEPTION1(InvalidFilesException, file);


    vtkFloatArray *scalars = vtkFloatArray::New();
    scalars->SetNumberOfTuples(num_data_samples);
    float *var_data = scalars->GetPointer(0);

    H5Dread(h5d_variable, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            var_data);

    H5Dclose(h5d_variable);      
    H5Fclose(h5f_file);

    return scalars;
}

// ****************************************************************************
//  Method:  avtSAMRAIFileFormat::GetVectorVar
//
//  Purpose:
//    Reads the vector variable with the given name for the given domain.
//
//  Note:  The Generic Database (which calls this function) already handles
//         caching for a single time step.
//
//  Arguments:
//    patch                the patch number
//    visit_var_name       the name of the variable to read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    July 18, 2003
//
// ****************************************************************************
vtkDataArray *
avtSAMRAIFileFormat::GetVectorVar(int patch, 
                                  const char *visit_var_name)
{
    unsigned int level;
    string var_name = GetNameLevel(visit_var_name, level, '_');    

    if (level >= num_levels)
        EXCEPTION1(InvalidVariableException, visit_var_name);

    if (patch < 0 || patch >= num_patches_level[level])
        EXCEPTION1(InvalidVariableException, visit_var_name);

    std::map<std::string, var_t>::const_iterator cur_var;
    cur_var = var_names_num_components.find(var_name);

    if (cur_var == var_names_num_components.end())
        EXCEPTION1(InvalidVariableException, visit_var_name);
    
    int num_components = (*cur_var).second.num_components;
    num_components = num_components <= 3 ? num_components : 3;
    int cell_centered =  (*cur_var).second.cell_centered;
        
    int extent = cell_centered == 1 ? 1 : 2;
    unsigned int offset = GetPatchOffset(level, patch);
    int dim = num_dim_problem < 3 ? num_dim_problem: 3;
    int num_data_samples = 1;
    for (int i=0; i<dim; i++) {
        num_data_samples *= patch_extents[offset].upper[i] - 
                            patch_extents[offset].lower[i] + extent;
    }

    char variable[100];
    sprintf(variable, "%s.00", var_name.c_str());

    for (int v = 0; v < num_vars; v++) {
        if (var_names[v] == variable) {
            if (!var_extents[v][offset].data_is_defined) {
                return NULL;
            }
            break;
        }
    }

    char file[2048];   
    sprintf(file, "%sprocessor_cluster.%05d.samrai",
            dir_name.c_str(), patch_map[offset].file_cluster_number);

    hid_t h5f_file = H5Fopen(file, H5F_ACC_RDONLY, H5P_DEFAULT); 
    if (h5f_file < 0)
        EXCEPTION1(InvalidFilesException, file);


    vtkFloatArray *vectors = vtkFloatArray::New();
    vectors->SetNumberOfComponents(3);
    vectors->SetNumberOfTuples(num_data_samples);
    for (int j = 0 ; j < num_data_samples ; j++) {
        vectors->SetComponent(j, 2, 0);
    }


//      vtkFloatArray *vectors = vtkFloatArray::New();
//      vectors->SetNumberOfComponents(num_components);
//      vectors->SetNumberOfTuples(num_data_samples);


    float *var_data = new float[num_data_samples];

    for (int i = 0 ; i < num_components ; i++) {
        sprintf(variable, "/processor.%05d/level.%05d/patch.%05d/%s.%02d",
                patch_map[offset].processor_number, level, patch, 
                var_name.c_str(), i);

        hid_t h5d_variable = H5Dopen(h5f_file, variable);
        if (h5d_variable < 0)
            EXCEPTION1(InvalidFilesException, file);
        
        H5Dread(h5d_variable, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
                var_data);
        H5Dclose(h5d_variable);      

        for (int j = 0 ; j < num_data_samples ; j++) {
            vectors->SetComponent(j, i, var_data[j]);
        }
    }

    delete[] var_data;
    H5Fclose(h5f_file);

    return vectors;
}


// ****************************************************************************
//  Method: avtBOVFileFormat::GetAuxiliaryData
//
//  Purpose:
//      Gets auxiliary data about the file format.
//
//  Programmer: Walter Herrera Jimenez
//  Creation:   July 21, 2003
//
// ****************************************************************************

void *
avtSAMRAIFileFormat::GetAuxiliaryData(const char *var, int domain,
                                      const char *type, void *,
                                      DestructorFunction &df)
{
    unsigned level;
    string name;
    void *rv = NULL;
    avtIntervalTree *itree = NULL;

    name = GetNameLevel(var, level, '_');    
    if (level >= num_levels)
        EXCEPTION1(InvalidVariableException, var);

    if (strcmp(type, AUXILIARY_DATA_DATA_EXTENTS) == 0) {
        
        std::map<std::string, var_t>::const_iterator cur_var;
        cur_var = var_names_num_components.find(name);
        if (cur_var == var_names_num_components.end())
            EXCEPTION1(InvalidVariableException, var);
    
        int num_components = (*cur_var).second.num_components;
        
        if (num_components == 1 ) {
          
            itree = new avtIntervalTree(num_patches_level[level], 1);
            for (int v = 0; v < num_vars; v++) {
                if (var_names[v] == name) {

                    for (int patch = 0 ; patch < num_patches_level[level] ; patch++) {
                        unsigned offset = GetPatchOffset(level, patch);

                        if (var_extents[v][offset].data_is_defined) {
                            float range[2] = { var_extents[v][offset].min, 
                                               var_extents[v][offset].max };
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
            itree = new avtIntervalTree(num_patches_level[level], 3);
            for (int patch = 0 ; patch < num_patches_level[level] ; patch++) {
                bool data_defined = true;
                unsigned int offset = GetPatchOffset(level, patch); 
                float range[6] = { 0, 0, 0, 0, 0, 0 };
                int dim = num_components <= 3 ? num_components : 3;
                for (int i=0; i<dim; i++) {
                    char variable[100];
                    sprintf(variable, "%s.%02d", name.c_str(), i);

                    for (int v = 0; v < num_vars; v++) {
                        if (var_names[v] == variable) {
                            if (var_extents[v][offset].data_is_defined) {
                                range[i*2] = var_extents[v][offset].min; 
                                range[i*2+1] = var_extents[v][offset].max;
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

        itree = new avtIntervalTree(num_patches_level[level], 3);
        for (int patch = 0 ; patch < num_patches_level[level] ; patch++) {
            unsigned offset = GetPatchOffset(level, patch);
            float bounds[] = {0, 0, 0, 0, 0, 0};
            int dim = num_dim_problem < 3 ? num_dim_problem: 3;

            for (int j=0; j<dim; j++) {
                bounds[j*2] = patch_extents[offset].xlo[j];
                bounds[j*2+1] = patch_extents[offset].xup[j];
            }
            itree->AddDomain(patch, bounds);
        }
        itree->Calculate(true);

        rv = (void *) itree;
        df = avtIntervalTree::Destruct;
    }

    return rv;
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
// ****************************************************************************
void
avtSAMRAIFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    for (int l=0; l<num_levels; l++) {
        char mesh_name[30];
        avtMeshMetaData *mesh = new avtMeshMetaData;
        sprintf(mesh_name, "Level_%05d", l);
        mesh->name = mesh_name;
        mesh->meshType = AVT_RECTILINEAR_MESH;
        mesh->numBlocks = num_patches_level[l];
        mesh->blockOrigin = 0;
        mesh->spatialDimension = 3;
        mesh->topologicalDimension = 3;
        mesh->hasSpatialExtents = false;
        md->Add(mesh);

        //char domain_name[50];
        //sprintf(domain_name, "domains(Level_%05d)", l);
        //AddDefaultPlotToMetaData(md, "subset", domain_name);

        int v=0;
        std::map<std::string, var_t>::const_iterator var_it;
        for (var_it = var_names_num_components.begin();
             var_it != var_names_num_components.end(); 
             var_it++) {

            if ((*var_it).first == "Coords") {
                continue;
            }

            char var_name[(*var_it).first.size() + 20];
            sprintf(var_name, "%s_%05d", (*var_it).first.c_str(), l);
          
            if ((*var_it).second.num_components == 1) { // scalar field
              if ( (*var_it).second.cell_centered == 1) {
                AddScalarVarToMetaData(md, var_name, mesh_name, AVT_ZONECENT);
              } else {
                AddScalarVarToMetaData(md, var_name, mesh_name, AVT_NODECENT);
              }
            }
            else if ((*var_it).second.num_components <= 3) { // vector field
              if ( (*var_it).second.cell_centered == 1) {
                AddVectorVarToMetaData(md, var_name, mesh_name, AVT_ZONECENT,
                                       //(*var_it).second.num_components);
                                       3);
              } else {
                AddVectorVarToMetaData(md, var_name, mesh_name, AVT_NODECENT,
                                       //(*var_it).second.num_components);
                                       3);
              }
            }
            else {
              EXCEPTION1(InvalidVariableException, (*var_it).first.c_str());
            }
        }
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
// ****************************************************************************
void
avtSAMRAIFileFormat::ReadMetaDataFile(hid_t &h5_file)
{
    ReadTimeStepNumber(h5_file);
    ReadTime(h5_file);
    ReadTimeOfDump(h5_file);

    ReadGridType(h5_file);
    ReadDataType(h5_file);
  
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
    hid_t h5_dataset = H5Dopen(h5_file, "/BASIC_INFO/time");
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

    H5Dread(h5_dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            &time);
    H5Dclose(h5_dataset);        
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
    hid_t h5_dataset = H5Dopen(h5_file,"/BASIC_INFO/time_step_number");
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

    H5Dread(h5_dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            &time_step_number);
    H5Dclose(h5_dataset);    
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
    hid_t h5_dataset = H5Dopen(h5_file, "/BASIC_INFO/time_of_dump");
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

    hid_t h5_disk_datatype = H5Tcopy(h5_dataset);
    int datatype_size = H5Tget_size(h5_disk_datatype);


    hid_t h5_mem_datatype = H5Tcopy(H5T_C_S1);
    H5Tset_size(h5_mem_datatype, datatype_size);  

    char buffer[datatype_size];

    H5Dread(h5_dataset, h5_mem_datatype, H5S_ALL,H5S_ALL, H5P_DEFAULT, buffer);

    H5Tclose(h5_mem_datatype);
    H5Tclose(h5_disk_datatype);
    H5Dclose(h5_dataset);        

    time_dump = buffer;
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
    hid_t h5_dataset = H5Dopen(h5_file,
                               "/BASIC_INFO/number_dimensions_of_problem");
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

    H5Dread(h5_dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            &num_dim_problem);
    H5Dclose(h5_dataset);    
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
    hid_t h5_dataset = H5Dopen(h5_file, "/BASIC_INFO/XLO");
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

    xlo = new double[num_dim_problem];

    H5Dread(h5_dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, xlo);
    H5Dclose(h5_dataset);    
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
    hid_t h5_dataset = H5Dopen(h5_file, "/BASIC_INFO/number_levels");
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

    H5Dread(h5_dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            &num_levels);
    H5Dclose(h5_dataset);    
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
    hid_t h5_dataset = H5Dopen(h5_file, "/BASIC_INFO/dx");
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

    dx = new double[num_levels * num_dim_problem];

    H5Dread(h5_dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, dx);
    H5Dclose(h5_dataset);    
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
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

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
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

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
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

    ratios_coarser_levels = new int[num_levels * num_dim_problem];

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
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

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
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

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
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

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
    hid_t h5_dataset = H5Dopen(h5_file,        "/BASIC_INFO/var_cell_centered");
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

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
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

    hid_t h5_disk_datatype = H5Tcopy(h5_dataset);
    int datatype_size = H5Tget_size(h5_disk_datatype);

    hid_t h5_mem_datatype = H5Tcopy(H5T_C_S1);
    H5Tset_size(h5_mem_datatype, datatype_size);  

    char buffer[datatype_size];

    H5Dread(h5_dataset, h5_mem_datatype, H5S_ALL,H5S_ALL, H5P_DEFAULT, buffer);

    H5Tclose(h5_mem_datatype);
    H5Tclose(h5_disk_datatype);
    H5Dclose(h5_dataset);        

    grid_type = buffer;
}


// ****************************************************************************
//  Method:  ReadDataType
//
//  Purpose:
//    Read the type of data of the variables
//
//  Arguments:
//    IN:  h5_file     the handle of the file to be read
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    August  20, 2003
//
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadDataType(hid_t &h5_file)
{
    hid_t h5_dataset = H5Dopen(h5_file, "/BASIC_INFO/data_type");
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

    hid_t h5_disk_datatype = H5Tcopy(h5_dataset);
    int datatype_size = H5Tget_size(h5_disk_datatype);

    hid_t h5_mem_datatype = H5Tcopy(H5T_C_S1);
    H5Tset_size(h5_mem_datatype, datatype_size);  

    char buffer[datatype_size];

    H5Dread(h5_dataset, h5_mem_datatype, H5S_ALL,H5S_ALL, H5P_DEFAULT, buffer);

    H5Tclose(h5_mem_datatype);
    H5Tclose(h5_disk_datatype);
    H5Dclose(h5_dataset);        

    data_type = buffer;
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
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadVarNames(hid_t &h5_file)
{
    hid_t h5_dataset = H5Dopen(h5_file, "/BASIC_INFO/var_names");
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

    hid_t h5_disk_datatype = H5Tcopy(h5_dataset);
    int datatype_size = H5Tget_size(h5_disk_datatype);

    hid_t h5_mem_datatype = H5Tcopy(H5T_C_S1);
    H5Tset_size(h5_mem_datatype, datatype_size);  

    char buffer[num_vars * datatype_size];
    var_names = new std::string[num_vars];

    H5Dread(h5_dataset, h5_mem_datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            buffer);

    H5Tclose(h5_mem_datatype);
    H5Tclose(h5_disk_datatype);
    H5Dclose(h5_dataset);        

    for (int v = 0; v < num_vars; v++) {
      var_names[v] = &buffer[v * datatype_size];
    }
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
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadVarNumComponents(hid_t &h5_file)
{
    hid_t h5_dataset = H5Dopen(h5_file, "/BASIC_INFO/var_number_components");
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

    var_num_components = new int[num_vars];

    H5Dread(h5_dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            var_num_components);
    H5Dclose(h5_dataset);    
    
    for (int v = 0; v < num_vars; v++) {
      unsigned level;
      string var_name = GetNameLevel(var_names[v].c_str(), level, '.');
      var_t var = { var_num_components[v], var_cell_centered[v] }; 
      var_names_num_components[var_name] = var;
    }
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
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadVarExtents(hid_t &h5_file)
{
    hid_t h5_mem_datatype = H5Tcreate (H5T_COMPOUND, sizeof(var_extents_t));
    H5Tinsert (h5_mem_datatype, "data_is_defined", 
               HOFFSET(var_extents_t,data_is_defined), H5T_NATIVE_CHAR);
    H5Tinsert (h5_mem_datatype, "min", 
               HOFFSET(var_extents_t,min), H5T_NATIVE_DOUBLE);
    H5Tinsert (h5_mem_datatype, "max", 
               HOFFSET(var_extents_t,max), H5T_NATIVE_DOUBLE);
              
    var_extents = new (var_extents_t*)[num_vars];
    for (int v = 0; v < num_vars; v++) {
      var_extents[v] = new var_extents_t[num_patches];
      
      char ds_name[50];
      sprintf(ds_name,"/extents/%s-Extents",var_names[v].c_str());
      
      hid_t h5_dataset = H5Dopen(h5_file, ds_name);
      if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

      H5Dread(h5_dataset, h5_mem_datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
              var_extents[v]);
      H5Dclose(h5_dataset);    
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
// ****************************************************************************
void 
avtSAMRAIFileFormat::ReadPatchExtents(hid_t &h5_file)
{
    hid_t h5_dataset = H5Dopen(h5_file, "/extents/patch_extents");
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

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
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

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
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

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
    hid_t h5_dataset = H5Dopen(h5_file,        "/BASIC_INFO/child_array");
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

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
    hid_t h5_dataset = H5Dopen(h5_file, "/BASIC_INFO/child_pointer_array");
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

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
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

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
    hid_t h5_dataset = H5Dopen(h5_file,        "/BASIC_INFO/parent_array");
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

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
    hid_t h5_dataset = H5Dopen(h5_file, "/BASIC_INFO/parent_pointer_array");
    if (h5_dataset < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());

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
//  Method:  GetPatchOffset
//
//  Purpose:
//    Returns the offset of one patch in the extent vectors
//
//  Arguments:
//    IN:  name_level_str     the string with name and level data
//    OUT: level              the level
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    July  11, 2003
//
// ****************************************************************************
unsigned int
avtSAMRAIFileFormat::GetPatchOffset(const int level, const int patch)
{
    int off=0;

    for (int i=0; i<level; i++)
        off += num_patches_level[i];
    off += patch;

    return off;
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
//  Method:  GetNameLevel
//
//  Purpose:
//    Returns the 'name' string and level number from a 'name.level' string
//
//  Arguments:
//    IN:  name_level_str     the string with name and level data
//    OUT: level              the level
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    July  11, 2003
//
// ****************************************************************************
string 
GetNameLevel(const char *name_level_str, unsigned int &level, char link)
{
    int len = strlen(name_level_str);
    const char *last = name_level_str + (len-1);
    while (*last != link && last > name_level_str)
    {
        last--;
    }

    if (*last != link)
    {
        level = 0;
        return name_level_str;
    }

    char str[1024];
    strcpy(str, name_level_str);
    str[last-name_level_str] = '\0';

    level = strtol(last+1,NULL,0);
    return str;
}






