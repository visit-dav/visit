// ***************************************************************************
//                              avtSAMRAIFileFormat.C                         
//
//  Purpose:  Sample database which reads multi-domain multi-timestep files
//
//  Programmer:  Jeremy Meredith, Walter Herrera-Jimenez
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
static string GetNameLevel(const char *name_level_str, unsigned int &level);


// ****************************************************************************
//  Constructor:  avtSAMRAIFileFormat::avtSAMRAIFileFormat
//
//  Arguments:
//    fname      the file name of the root metadata file
//
//  Programmer:  Walter Herrera-Jimenez
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
//  Programmer:  Jeremy Meredith
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
//  Programmer:  Jeremy Meredith
//  Creation:    June 19, 2003
//
// ****************************************************************************
vtkDataSet *
avtSAMRAIFileFormat::GetMesh(int patch, const char *level_name)
{
    unsigned level;
    string name = GetNameLevel(level_name, level);
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
//  Programmer:  Jeremy Meredith, Walter Herrera Jimenez
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
    string name = GetNameLevel(level_name, level);
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

    if (is_rectilinear) {
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
    else {
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
    string var_name = GetNameLevel(visit_var_name, level);    

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

    int extent = cell_centered == 1 ? 1 : 2;
    unsigned int offset = GetPatchOffset(level, patch);
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
    sprintf(variable, "/processor.%05d/level.%05d/patch.%05d/%s.00",
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
    string var_name = GetNameLevel(visit_var_name, level);    

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

    float *var_data = new float[num_data_samples];

    for (int i = 0 ; i < num_components ; i++) {
        char variable[100];
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

    name = GetNameLevel(var, level);    
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
            for (int patch = 0 ; patch < num_patches_level[level] ; patch++) {
                char variable[100];
                sprintf(variable, "%s.00", name.c_str());

                for (int v = 0; v < num_vars; v++) {
                    if (var_names[v] == variable) {
                        float range[2] = { var_extents[v]->min, 
                                           var_extents[v]->max };
                        itree->AddDomain(patch, range);
                        break;
                    }
                }
            }
        } else {

            itree = new avtIntervalTree(num_patches_level[level], 3);
            for (int patch = 0 ; patch < num_patches_level[level] ; patch++) {
                float range[6] = { 0, 0, 0, 0, 0, 0 };
                int dim = num_components <= 3 ? num_components : 3;
                for (int i=0; i<dim; i++) {
                    char variable[100];
                    sprintf(variable, "%s.%02d", name.c_str(), i);

                    for (int v = 0; v < num_vars; v++) {
                        if (var_names[v] == variable) {
                            range[i*2] = var_extents[v]->min; 
                            range[i*2+1] = var_extents[v]->max;
                            itree->AddDomain(patch, range);
                            break;
                        }
                    }
                }

                itree->AddDomain(patch, range);
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
//  Programmer:  Jeremy Meredith
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
                                       (*var_it).second.num_components);
              } else {
                AddVectorVarToMetaData(md, var_name, mesh_name, AVT_NODECENT,
                                       (*var_it).second.num_components);
              }
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
//  Programmer:  Walter Herrera-Jimenez
//  Creation:    July 07, 2003
//
// ****************************************************************************
void
avtSAMRAIFileFormat::ReadMetaDataFile(hid_t &h5f_file)
{
    // Dump number
    //
    hid_t h5d_dump_number = H5Dopen(h5f_file,"/BASIC_INFO/dump_number");
    if (h5d_dump_number < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());
    H5Dread(h5d_dump_number, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            &dump_number);
    H5Dclose(h5d_dump_number);    


    // Number of dimensions of the problem 
    //
    hid_t h5d_dim_problem =H5Dopen(h5f_file,
                                   "/BASIC_INFO/number_dimensions_of_problem");
    if (h5d_dim_problem < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());
    H5Dread(h5d_dim_problem, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            &num_dim_problem);
    H5Dclose(h5d_dim_problem);    


    // Lower spatial coordinate (origin)   
    //
    xlo = new double[num_dim_problem];
    hid_t h5d_xlo = H5Dopen(h5f_file, "/BASIC_INFO/XLO");
    if (h5d_xlo < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());
    H5Dread(h5d_xlo, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, xlo);
    H5Dclose(h5d_xlo);    


    // Number of levels   
    //
    hid_t h5d_levels = H5Dopen(h5f_file, "/BASIC_INFO/number_levels");
    if (h5d_levels < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());
    H5Dread(h5d_levels, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            &num_levels);
    H5Dclose(h5d_levels);    


    // Width of a cell on each level   
    //
    dx = new double[num_levels * num_dim_problem];
    hid_t h5d_dx = H5Dopen(h5f_file, "/BASIC_INFO/dx");
    if (h5d_dx < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());
    H5Dread(h5d_dx, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, dx);
    H5Dclose(h5d_dx);    


    // Number of patches at each level   
    //
    num_patches_level = new int[num_levels];
    hid_t h5d_patches_level = H5Dopen(h5f_file,
                                      "/BASIC_INFO/number_patches_at_level");
    if (h5d_patches_level < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());
    H5Dread(h5d_patches_level, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            num_patches_level);
    H5Dclose(h5d_patches_level);    

    num_patches = 0;
    for (int l=0; l<num_levels; l++) {
        num_patches += num_patches_level[l];
    }


    // Ratios to coarser levels   
    //
    ratios_coarser_levels = new int[num_levels * num_dim_problem];
    hid_t h5d_ratios_levels = H5Dopen(h5f_file, 
                                      "/BASIC_INFO/ratios_to_coarser_levels");
    if (h5d_ratios_levels < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());
    H5Dread(h5d_ratios_levels, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, 
            H5P_DEFAULT, ratios_coarser_levels);
    H5Dclose(h5d_ratios_levels);    


    // Number of clusters   
    //
    hid_t h5d_clusters = H5Dopen(h5f_file, "/BASIC_INFO/number_file_clusters");
    if (h5d_clusters < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());
    H5Dread(h5d_clusters, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            &num_clusters);
    H5Dclose(h5d_clusters);    


    // Number of processors   
    //
    hid_t h5d_procs = H5Dopen(h5f_file, "/BASIC_INFO/number_processors");
    if (h5d_procs < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());
    H5Dread(h5d_procs, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            &num_procs);
    H5Dclose(h5d_procs);    


    // Number of variables   
    //
    hid_t h5d_vars = H5Dopen(h5f_file, "/BASIC_INFO/number_visit_variables");
    if (h5d_vars < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());
    H5Dread(h5d_vars, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            &num_vars);
    H5Dclose(h5d_vars);    


    // Cell centered of each variable   
    //
    var_cell_centered = new int[num_vars];
    hid_t h5d_cell_centered = H5Dopen(h5f_file,
                                      "/BASIC_INFO/var_cell_centered");
    if (h5d_cell_centered < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());
    H5Dread(h5d_cell_centered, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            var_cell_centered);
    H5Dclose(h5d_cell_centered);    


    // Time
    //   
    hid_t h5d_time = H5Dopen(h5f_file, "/BASIC_INFO/time");
    if (h5d_time < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());
    H5Dread(h5d_time, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            &time);
    H5Dclose(h5d_time);    

    
    // Name of each variable   
    //
    hid_t h5d_var_names = H5Dopen(h5f_file, "/BASIC_INFO/var_names");
    if (h5d_var_names < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());
    hid_t h5t_var_name = H5Tcopy(h5d_var_names);
    int name_size = H5Tget_size(h5t_var_name);
    char names_buffer[num_vars * name_size];
    var_names = new std::string[num_vars];

    H5Dread(h5d_var_names, h5t_var_name, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            names_buffer);
    H5Tclose(h5t_var_name);
    H5Dclose(h5d_var_names);        

    is_rectilinear = true;
    for (int v=0; v<num_vars; v++) {
        unsigned level;
        var_names[v] = &names_buffer[v * name_size];
        string var_name = GetNameLevel(var_names[v].c_str(), level);
        if (var_name == "Coords") {
            is_rectilinear = false;
        }
    }


    // Num components of each variable   
    //
    var_num_components = new int[num_vars];
    hid_t h5d_num_components = H5Dopen(h5f_file, 
                                       "/BASIC_INFO/var_number_components");
    if (h5d_num_components < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());
    H5Dread(h5d_num_components, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
            var_num_components);
    H5Dclose(h5d_num_components);    

    for (int v=0; v<num_vars; v++) {
      unsigned level;
      string var_name = GetNameLevel(var_names[v].c_str(), level);
      var_t var = { var_num_components[v], var_cell_centered[v] }; 
      var_names_num_components[var_name] = var;
    }

    
    // Extents for each variable   
    //
    hid_t h5t_extents = H5Tcreate (H5T_COMPOUND, sizeof(var_extents_t));
    H5Tinsert (h5t_extents,"min",HOFFSET(var_extents_t,min), H5T_NATIVE_FLOAT);
    H5Tinsert (h5t_extents,"max",HOFFSET(var_extents_t,max), H5T_NATIVE_FLOAT);

    var_extents = new (var_extents_t*)[num_vars];
    for (int v=0; v<num_vars; v++) {
        var_extents[v] = new var_extents_t[num_patches];
   
        char ds_name[50];
        sprintf(ds_name,"/extents/%s-Extents",var_names[v].c_str());

        hid_t h5d_var_extents = H5Dopen(h5f_file, ds_name);
        H5Dread(h5d_var_extents, h5t_extents, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                var_extents[v]);
        H5Dclose(h5d_var_extents);    
        
    }

    H5Tclose(h5t_extents);

    
    // Spatial extents for each patch   
    //
    hid_t h5d_patch_extents = H5Dopen(h5f_file, "/extents/patch_extents");
    if (h5d_patch_extents < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());
    hid_t h5t_patch_extent = H5Tcopy(h5d_patch_extents);
    int patch_extent_size = H5Tget_size(h5t_patch_extent);
    char extents_buffer[num_patches * patch_extent_size];
    patch_extents = new patch_extents_t[num_patches];

    H5Dread(h5d_patch_extents, h5t_patch_extent, H5S_ALL, H5S_ALL, H5P_DEFAULT,
            extents_buffer);
    H5Dclose(h5d_patch_extents);        

    for (int p=0; p<num_patches; p++) {
      int *lower = (int*)(&extents_buffer[p*patch_extent_size] + 
                          H5Tget_member_offset(h5t_patch_extent, 0));
      int *upper = (int*)(&extents_buffer[p*patch_extent_size] + 
                          H5Tget_member_offset(h5t_patch_extent, 1));
      float *xlo = (float*)(&extents_buffer[p*patch_extent_size] + 
                            H5Tget_member_offset(h5t_patch_extent, 2));
      float *xup = (float*)(&extents_buffer[p*patch_extent_size] + 
                            H5Tget_member_offset(h5t_patch_extent, 3));

      for (int d=0; d<num_dim_problem; d++) {
        patch_extents[p].lower.push_back(lower[d]);
        patch_extents[p].upper.push_back(upper[d]);
        patch_extents[p].xlo.push_back(xlo[d]);
        patch_extents[p].xup.push_back(xup[d]);
      }
    }

    H5Tclose(h5t_patch_extent);


    // Cluster, processor, level and number of patch for each patch   
    //
    hid_t h5t_patch_map = H5Tcreate (H5T_COMPOUND, sizeof(patch_map_t));
    H5Tinsert (h5t_patch_map, "processor_number", 
               HOFFSET(patch_map_t,processor_number), H5T_NATIVE_INT);
    H5Tinsert (h5t_patch_map, "file_cluster_number", 
               HOFFSET(patch_map_t,file_cluster_number), H5T_NATIVE_INT);
    H5Tinsert (h5t_patch_map, "level_number", 
               HOFFSET(patch_map_t,level_number), H5T_NATIVE_INT);
    H5Tinsert (h5t_patch_map, "patch_number", 
               HOFFSET(patch_map_t,patch_number), H5T_NATIVE_INT);
    patch_map = new patch_map_t[num_patches];
      
    hid_t h5d_patch_map = H5Dopen(h5f_file, "/extents/patch_map");
    if (h5d_patch_map < 0)
        EXCEPTION1(InvalidFilesException, file_name.c_str());
    H5Dread(h5d_patch_map, h5t_patch_map, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
            patch_map);
    H5Dclose(h5d_patch_map);      
    H5Tclose(h5t_patch_map);

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
GetNameLevel(const char *name_level_str, unsigned int &level)
{
    int len = strlen(name_level_str);
    const char *last = name_level_str + (len-1);
    while (*last != '.' && *last != '_' && last > name_level_str)
    {
        last--;
    }

    if (*last != '.' && *last != '_')
    {
        level = 0;
        return "";
    }

    char str[1024];
    strcpy(str, name_level_str);
    str[last-name_level_str] = '\0';

    level = strtol(last+1,NULL,0);
    return str;
}






