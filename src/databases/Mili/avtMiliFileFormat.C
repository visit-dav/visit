// ************************************************************************* //
//                              avtMiliFileFormat.C                          //
// ************************************************************************* //


#include <avtMiliFileFormat.h>

#include <vector>
#include <string>

extern "C" {
#include <mili_enum.h>
}

#include <vtkUnstructuredGrid.h>
#include <vtkFloatArray.h>

#include <avtDatabaseMetaData.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>

using std::vector;
using std::string;


// ****************************************************************************
//  Constructor:  avtMiliFileFormat::avtMiliFileFormat
//
//  Arguments:
//    fname      the file name of one of the Mili files.
//
//  Programmer:  Hank Childs
//  Creation:    April 11, 2003
//
// ****************************************************************************

avtMiliFileFormat::avtMiliFileFormat(const char *fname)
    : avtMTSDFileFormat(&fname, 1)
{
    ntimesteps = 1;
    gottenGeneralInfo = false;
    connectivity = NULL;
    dims = 3;
    nnodes = 0;
    ncells = 0;

    // 
    // Akira:
    //
    // Should we really be opening the file now, or should we wait until later?
    //
    // There is a file descriptor management scheme that we are not hooking
    // into here.
    //

    //
    // Code from GRIZ.
    //
    const char *p_c, *p_src;
    const char *p_root_start, *p_root_end;
    char *p_dest;
    char *path;
    char root[128];
    char path_text[256];
 
    /* Scan forward to end of name string. */
    for ( p_c = fname ; *p_c != '\0'; p_c++ );
 
    /* Scan backward to last non-slash character. */
    for ( p_c--; *p_c == '/' && p_c != fname; p_c-- );
    p_root_end = p_c;
 
    /* Scan backward to last slash character preceding last non-slash char. */
    for ( ; *p_c != '/' && p_c != fname; p_c-- );
 
    p_root_start = ( *p_c == '/' ) ? p_c + 1 : p_c;
 
    /* Generate the path argument to mc_open(). */
    if ( p_root_start == fname )
        /* No path preceding root name. */
        path = NULL;
    else
    {
        /* Copy path (less last slash). */
 
        path = path_text;
 
        for ( p_src = fname, p_dest = path_text;
              p_src < p_root_start - 1;
              *p_dest++ = *p_src++ );
 
        if ( p_src == fname )
            /* Path must be just "/".  If that's what the app wants... */
            *p_dest++ = *fname;
 
        *p_dest = '\0';
    }
 
    /* Generate root name argument to mc_open(). */
    for ( p_src = p_root_start, p_dest = root;
          p_src <= p_root_end;
          *p_dest++ = *p_src++ );
    *p_dest = '\0';
 
/*
    //
    // The GRIZ code assumes that we have a root, but we really have a
    // filename.  Take off the trailing characters to create a root.
    // Akira: This probably requires more thought than I putting into this.
    //
    int len = strlen(root);
    if (isdigit(root[len-1]))
    {
        root[len-2] = '\0'; // Take off the last two digits.
    }
    else if (isalpha(root[len-1]))
    {
        root[len-1] = '\0'; // Take off the 'A', 'B', etc.
    }
    else
    {
        EXCEPTION1(InvalidFilesException, fname);
    }
 */

    //
    // For the time being, we will just take off the .m or .mili extension.
    //
    int len = strlen(root);
    if (strstr(root, ".mili") != NULL)
    {
        root[len - strlen(".mili")] = '\0';
    }
    else if (strstr(root, ".m") != NULL)
    {
        root[len - strlen(".m")] = '\0';
    }
    else
    {
        EXCEPTION1(InvalidFilesException, fname);
    }

    int rval = mc_open( root, path, "r", &dbid );
    if ( rval != OK )
    {
        EXCEPTION1(InvalidFilesException, fname);
    }
}


// ****************************************************************************
//  Destructor:  avtMiliFileFormat::~avtMiliFileFormat
//
//  Programmer:  Hank Childs
//  Creation:    April 11, 2003
//
// ****************************************************************************

avtMiliFileFormat::~avtMiliFileFormat()
{
    mc_close(dbid);
    if (connectivity != NULL)
    {
        connectivity->Delete();
    }
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh for timestep 'ts'.
//
//  Arguments:
//    ts         the time step
//    mesh       the name of the mesh to read
//
//  Programmer:  Hank Childs
//  Creation:    April 11, 2003
//
// ****************************************************************************

vtkDataSet *
avtMiliFileFormat::GetMesh(int ts, const char *mesh)
{
    if (!gottenGeneralInfo)
    {
        GetGeneralInfo();
    }

    char *nodpos_str = "nodpos";

    //
    // The connectivity does not change over time, so use the one we have
    // already calculated.
    //
    vtkUnstructuredGrid *rv = vtkUnstructuredGrid::New();
    rv->ShallowCopy(connectivity);

    //
    // The node positions are stored in 'nodpos'.
    //
    int nodpos = GetVariableIndex(nodpos_str);

    int subrec = -1;
    for (int i = 0 ; i < vars_valid[nodpos].size() ; i++)
    {
        if (vars_valid[nodpos][i])
        {
            subrec = sub_record_ids[i];
            break;
        }
    }
    if (subrec == -1)
    {
        EXCEPTION0(ImproperUseException);
    }

    float *fpts = new float[dims*nnodes];
    int rval = mc_read_results(dbid, ts+1, subrec, 1, &nodpos_str, fpts);
    if (rval != OK)
    {
        EXCEPTION1(InvalidFilesException, filenames[0]);
    }

    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(nnodes);
    float *vpts = (float *) pts->GetVoidPointer(0);
    float *tmp = fpts;
    for (int pt = 0 ; pt < nnodes ; pt++)
    {
        *(vpts++) = *(tmp++);
        *(vpts++) = *(tmp++);
        if (dims >= 3)
            *(vpts++) = *(tmp++);
        else
            *(vpts++) = 0.;
    }

    rv->SetPoints(pts);
    pts->Delete();
    delete [] fpts;

    return rv;
}


// ****************************************************************************
//  Method: avtMiliFileFormat::GetVariableIndex
//
//  Purpose:
//      Gets the index of a variable.
//
//  Programmer: Hank Childs
//  Creation:   April 16, 2003
//
// ****************************************************************************

int
avtMiliFileFormat::GetVariableIndex(const char *varname)
{
    for (int i = 0 ; i < vars.size() ; i++)
    {
        if (vars[i] == varname)
        {
            return i;
        }
    }

    EXCEPTION1(InvalidVariableException, varname);
}


// ****************************************************************************
//  Method: avtMiliFileFormat::GetSizeInfoForGroup
//
//  Purpose:
//      Returns the number of cells and where they start in the connectivity
//      index.
//
//  Programmer: Hank Childs
//  Creation:   April 16, 2003
//
// ****************************************************************************

void
avtMiliFileFormat::GetSizeInfoForGroup(const char *group_name, int &offset,
                                       int &g_size)
{
    int g_index = -1;
    for (int i = 0 ; i < element_group_name.size() ; i++)
    {
        if (element_group_name[i] == group_name)
        {
            g_index = i;
            break;
        }
    }
    if (g_index == -1)
    {
        EXCEPTION0(ImproperUseException);
    }

    offset = connectivity_offset[g_index];
    if (g_index == (connectivity_offset.size()-1))
    {
        g_size = ncells - connectivity_offset[g_index];
    }
    else
    {
        g_size = connectivity_offset[g_index+1] - connectivity_offset[g_index];
    }
}


// ****************************************************************************
//  Method: avtMiliFileFormat::GetGeneralInfo
//
//  Purpose:
//      Gets general information about the dataset.
//
//  Programmer: Hank Childs
//  Creation:   April 16, 2003
//
// ****************************************************************************

void
avtMiliFileFormat::GetGeneralInfo(void)
{
    char short_name[1024];
    char long_name[1024];

    //
    // Determine how many timesteps there are.
    //
    int rval = mc_query_family(dbid, QTY_STATES, NULL, NULL,
                               (void *) &ntimesteps);
    if (rval != OK)
    {
        EXCEPTION1(InvalidFilesException, filenames[0]);
    }

    //
    // Find the dimensions.
    //
    mc_query_family(dbid, QTY_DIMENSIONS, NULL, NULL, &dims);

    //
    // Determine the number of nodes.
    //
    int mesh_id = 0;
    int node_id = 0;
    mc_get_class_info(dbid, mesh_id, M_NODE, node_id, short_name, long_name,
                    &nnodes);

    //
    // Determine the connectivity.  This will also calculate the number of
    // cells.
    //
    connectivity = vtkUnstructuredGrid::New();

    const int n_elem_types = 8;
    static int elem_sclasses[n_elem_types] =
    {
        M_TRUSS, M_BEAM, M_TRI, M_QUAD, M_TET, M_PYRAMID, M_WEDGE, M_HEX
    };
    static int conn_count[n_elem_types] =
    {
        2, 3, 3, 4, 4, 5, 6, 8
    };
    
    //
    // Make one pass through the data and read all of the connectivity
    // information.
    //
    vector < vector<int *> > conn_list;
    vector < vector<int *> > mat_list;
    vector < vector<int> > list_size;
    conn_list.resize(n_elem_types);
    list_size.resize(n_elem_types);
    mat_list.resize(n_elem_types);
    int i, j, k;
    ncells = 0;
    int ncoords = 0;
    for (i = 0 ; i < n_elem_types ; i++)
    {
        int args[2];
        int mesh_id = 0;
        args[0] = mesh_id;
        args[1] = elem_sclasses[i];
        int ngroups = 0;
        mc_query_family(dbid, QTY_CLASS_IN_SCLASS, (void*) args, NULL,
                        (void*) &ngroups);
        for (j = 0 ; j < ngroups ; j++)
        {
            int nelems;
            mc_get_class_info(dbid, mesh_id, elem_sclasses[i], j,
                              short_name, long_name, &nelems);
 
            int *conn = new int[nelems * conn_count[i]];
            int *mat = new int[nelems];
            int *part = new int[nelems];
            mc_load_conns(dbid, mesh_id, short_name, conn, mat, part);
            
            conn_list[i].push_back(conn);
            mat_list[i].push_back(mat);
            list_size[i].push_back(nelems);
            connectivity_offset.push_back(ncells);
            element_group_name.push_back(short_name);
            ncells += list_size[i][j];
            ncoords += list_size[i][j] * (conn_count[i] + 1);
            delete [] mat;
            delete [] part;
        }
    }

    //
    // Allocate an appropriately sized dataset using that connectivity
    // information.
    //
    connectivity->Allocate(ncells, ncoords);

    //
    // The materials are in a format that is not AVT friendly.  Convert it
    // now.
    //
    ConstructMaterials(mat_list, list_size);

    //
    // Now construct the connectivity in a VTK dataset.
    //
    for (i = 0 ; i < n_elem_types ; i++)
    {
        for (j = 0 ; j < conn_list[i].size() ; j++)
        {
            int *conn = conn_list[i][j];
            int nelems = list_size[i][j];
            for (k = 0 ; k < nelems ; k++)
            {
                switch (elem_sclasses[i])
                {
                  case M_TRI:
                    connectivity->InsertNextCell(VTK_TRIANGLE,
                                                 conn_count[i], conn);
                    break;
                  case M_QUAD:
                    connectivity->InsertNextCell(VTK_QUAD,
                                                 conn_count[i], conn);
                    break;
                  case M_TET:
                    connectivity->InsertNextCell(VTK_TETRA,
                                                 conn_count[i], conn);
                    break;
                  case M_PYRAMID:
                    connectivity->InsertNextCell(VTK_PYRAMID,
                                                 conn_count[i], conn);
                    break;
                  case M_WEDGE:
                    connectivity->InsertNextCell(VTK_WEDGE,
                                                 conn_count[i], conn);
                    break;
                  case M_HEX:
                    connectivity->InsertNextCell(VTK_HEXAHEDRON,
                                                 conn_count[i], conn);
                    break;
                  default:
                    debug1 << "Unable to add cell" << endl;
                    break;
                }
 
                conn += conn_count[i];
            }
            conn = conn_list[i][j];
            delete [] conn;
        }
    }

    //
    // Right now we are assuming that there is only one mesh.
    //
    int nmeshes = 0;
    rval = mc_query_family(dbid, QTY_MESHES, NULL, NULL, (void *) &nmeshes);
    if (rval != OK)
    {
        EXCEPTION1(InvalidFilesException, filenames[0]);
    }
    if (nmeshes != 1)
    {
        EXCEPTION1(InvalidFilesException, filenames[0]);
    }

    //
    // Finally, read in information about the variables.
    //
    int srec_qty = 0;
    rval = mc_query_family(dbid, QTY_SREC_FMTS, NULL, NULL, (void*) &srec_qty);

    for (i = 0 ; i < srec_qty ; i++)
    {
        int mesh_id;
        rval = mc_query_family(dbid, SREC_MESH, (void *) &i, NULL,
                               (void *) &mesh_id);
        if (rval != OK)
        {
            EXCEPTION1(InvalidFilesException, filenames[0]);
        }

        int substates = 0;
        rval = mc_query_family(dbid, QTY_SUBRECS, (void *) &i, NULL,
                               (void *) &substates);

        for (int j = 0 ; j < substates ; j++)
        {
            Subrecord sr;
            rval = mc_get_subrec_def(dbid, i, j, &sr);

            if (strcmp(sr.class_name, "glob") == 0)
            {
                continue;
            }
            if (strcmp(sr.class_name, "mat") == 0)
            {
                continue;
            }
            if (strcmp(sr.class_name, "sand") == 0)
            {
                continue;
            }

            sub_records.push_back(sr.class_name);
            sub_record_ids.push_back(j);

            //
            // To date, we believe none of the variables are valid for this
            // subrecord.  This will change as we look through its variable
            // list.
            //
            for (int vv = 0 ; vv < vars.size() ; vv++)
                 vars_valid[vv].push_back(false);
            int index = sub_records.size() - 1;
           
            for (int k = 0 ; k < sr.qty_svars ; k++)
            {
                 State_variable sv;
                 mc_get_svar_def(dbid, sr.svar_names[k], &sv);

                 int sameAsVar = -1;
                 for (int v = 0 ; v < vars.size() ; v++)
                     if (vars[v] == sv.short_name)
                     {
                         sameAsVar = v;
                         break;
                     }

                 if (sameAsVar != -1)
                 {
                     vars_valid[sameAsVar][index] = true;
                     continue;
                 }

                 vars.push_back(sv.short_name);
                 vector<bool> tmp(sub_records.size(), false);
                 tmp[index] = true;
                 vars_valid.push_back(tmp);

                 avtCentering cent;
                 if (strcmp(sr.class_name, "node") == 0)
                     cent = AVT_NODECENT;
                 else
                     cent = AVT_ZONECENT;
                 centering.push_back(cent);
                 if (sv.agg_type == SCALAR)
                     vartype.push_back(AVT_SCALAR_VAR);
                 else if (sv.agg_type == VECTOR && sv.vec_size == dims)
                     vartype.push_back(AVT_VECTOR_VAR);
                 else
                     vartype.push_back(AVT_UNKNOWN_TYPE);
            }
        }
    }

    gottenGeneralInfo = true;
}


// ****************************************************************************
//  Method: avtMiliFileFormat::ConstructMaterials
//
//  Purpose:
//      Constructs a material list from the partial material lists.
//
//  Programmer: Hank Childs
//  Creation:   April 18, 2003
//
// ****************************************************************************

void
avtMiliFileFormat::ConstructMaterials(vector< vector<int *> > &mat_list,
                                      vector< vector<int> > &list_size)
{
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
// ****************************************************************************

vtkDataArray *
avtMiliFileFormat::GetVar(int ts, const char *name)
{
    if (!gottenGeneralInfo)
    {
        GetGeneralInfo();
    }

    vtkFloatArray *rv = vtkFloatArray::New();

    int v_index = GetVariableIndex(name);

    if (centering[v_index] == AVT_NODECENT)
    {
        int nvars = 0;
        int sr_valid = -1;
        for (int i = 0 ; i < vars_valid[v_index].size() ; i++)
        {
            if (vars_valid[v_index][i])
            {
                sr_valid = i;
                nvars++;
            }
        }
        if (nvars != 1)
        {
            EXCEPTION1(InvalidVariableException, name);
        }

        rv->SetNumberOfTuples(nnodes);
        float *p = (float *) rv->GetVoidPointer(0);
        char *tmp = (char *) name;  // Bypass const
        int rval = mc_read_results(dbid, ts+1, sub_record_ids[sr_valid],
                                    1, &tmp, p);
        if (rval != OK)
        {
            EXCEPTION1(InvalidVariableException, name);
        }
    }
    else
    {
        rv->SetNumberOfTuples(ncells);
        float *p = (float *) rv->GetVoidPointer(0);
        int i;
        for (i = 0 ; i < ncells ; i++)
            p[i] = 0.;
        for (i = 0 ; i < vars_valid[v_index].size() ; i++)
        {
            if (vars_valid[v_index][i])
            {
                int start = 0;
                int csize = 0;
                GetSizeInfoForGroup(sub_records[i].c_str(), start, csize);
                char *tmp = (char *) name;  // Bypass const
                int rval = mc_read_results(dbid, ts+1, sub_record_ids[i],
                                           1, &tmp, p + start);
                if (rval != OK)
                {
                    EXCEPTION1(InvalidVariableException, name);
                }
            }
        }
    }

    return rv;
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
// ****************************************************************************

vtkDataArray *
avtMiliFileFormat::GetVectorVar(int ts, const char *name)
{
    if (!gottenGeneralInfo)
    {
        GetGeneralInfo();
    }

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(dims);

    int v_index = GetVariableIndex(name);

    if (centering[v_index] == AVT_NODECENT)
    {
        int nvars = 0;
        int sr_valid = -1;
        for (int i = 0 ; i < vars_valid[v_index].size() ; i++)
        {
            if (vars_valid[v_index][i])
            {
                sr_valid = i;
                nvars++;
            }
        }
        if (nvars != 1)
        {
            EXCEPTION1(InvalidVariableException, name);
        }

        rv->SetNumberOfTuples(nnodes);
        float *ptr = (float *) rv->GetVoidPointer(0);
        char *tmp = (char *) name;  // Bypass const
        int rval = mc_read_results(dbid, ts+1, sub_record_ids[sr_valid],
                                    1, &tmp, ptr);
        if (rval != OK)
        {
            EXCEPTION1(InvalidVariableException, name);
        }
    }
    else
    {
        rv->SetNumberOfTuples(ncells);
        float *p = (float *) rv->GetVoidPointer(0);
        int i;
        int nvals = ncells*dims;
        for (i = 0 ; i < nvals ; i++)
            p[i] = 0.;
        for (int i = 0 ; i < vars_valid[v_index].size() ; i++)
        {
            if (vars_valid[v_index][i])
            {
                int start = 0;
                int csize = 0;
                GetSizeInfoForGroup(sub_records[i].c_str(), start, csize);
                char *tmp = (char *) name;  // Bypass const
                int rval = mc_read_results(dbid, ts+1, sub_record_ids[i],
                                           1, &tmp, p + start*dims);
                if (rval != OK)
                {
                    EXCEPTION1(InvalidVariableException, name);
                }
            }
        }
    }

    return rv;
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
// ****************************************************************************

void
avtMiliFileFormat::GetCycles(vector<int> &cycles)
{
    int nTimesteps = GetNTimesteps();

    cycles.resize(nTimesteps);
    for (int i = 0 ; i < nTimesteps ; i++)
    {
        cycles[i] = i;
    }
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
    return ntimesteps;
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//    Returns meta-data about the database.
//
//  Arguments:
//    md         The meta-data structure to populate
//
//  Programmer:  Hank Childs
//  Creation:    April 11, 2003
//
// ****************************************************************************

void
avtMiliFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    if (!gottenGeneralInfo)
    {
        GetGeneralInfo();
    }

    AddMeshToMetaData(md, "mesh", AVT_UNSTRUCTURED_MESH, NULL, 1, 0, 
                      dims, dims);

    for (int i = 0 ; i < vars.size() ; i++)
    {
        switch (vartype[i])
        {
          case AVT_SCALAR_VAR:
            AddScalarVarToMetaData(md, vars[i], "mesh", centering[i]);
            break;
          case AVT_VECTOR_VAR:
            AddVectorVarToMetaData(md, vars[i], "mesh", centering[i], dims);
            break;
        }
    }
}


