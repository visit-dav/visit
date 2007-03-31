// ************************************************************************* //
//                              avtMiliFileFormat.C                          //
// ************************************************************************* //


#include <avtMiliFileFormat.h>

#include <vector>
#include <string>
#include <fstream>

extern "C" {
#include <mili_enum.h>
}

#include <vtkUnstructuredGrid.h>
#include <vtkFloatArray.h>

#include <avtDatabaseMetaData.h>
#include <avtMaterial.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>

using std::vector;
using std::string;
using std::ifstream;

// ****************************************************************************
//  Constructor:  avtMiliFileFormat::avtMiliFileFormat
//
//  Arguments:
//    fname      the file name of one of the Mili files.
//
//  Programmer:  Hank Childs
//  Creation:    April 11, 2003
//
//  Modifications
//    Akira Haddox, Fri May 23 08:13:09 PDT 2003
//    Added in support for multiple meshes. Changed to MTMD.
//    Changed to read in .mili format files.
//
//    Akira Haddox, Tue Jul 22 15:34:40 PDT 2003
//    Initialized setTimesteps.
//
// ****************************************************************************

avtMiliFileFormat::avtMiliFileFormat(const char *fname)
    : avtMTMDFileFormat(fname)
{
    ifstream in;
    in.open(fname);

    ndomains = nmeshes = 0;
    setTimesteps = false;
    
    in >> ndomains;
    in >> ntimesteps;
    in >> dims;
    in >> nmeshes;
    
    dbid.resize(ndomains, -1);
    readMesh.resize(ndomains, false);
    validateVars.resize(ndomains, false);
    nnodes.resize(ndomains);
    ncells.resize(ndomains);
    connectivity.resize(ndomains);
    sub_records.resize(ndomains);
    sub_record_ids.resize(ndomains);
    element_group_name.resize(ndomains);
    connectivity_offset.resize(ndomains);
    group_mesh_associations.resize(ndomains);
    materials.resize(ndomains);
    
    nmaterials.resize(nmeshes);

    int dom;
    for (dom = 0; dom < ndomains; ++dom)
    {
        nnodes[dom].resize(nmeshes, 0);
        ncells[dom].resize(nmeshes, 0);
        connectivity[dom].resize(nmeshes, NULL);
        materials[dom].resize(nmeshes, NULL);
    }

    int mesh_id;
    for (mesh_id = 0; mesh_id < nmeshes; ++mesh_id)
    {
        in >> nmaterials[mesh_id];
        int nVars;
        in >> nVars;
        int v;
        for (v = 0; v < nVars; ++v)
        {
            int type, center;
            string name;
            in >> type >> center;

            // Strip out leading white space.
            while(isspace(in.peek()))
                in.get();
            
            getline(in, name);

            vars.push_back(name);
            centering.push_back(avtCentering(center));
            vartype.push_back(avtVarType(type));
            var_mesh_associations.push_back(mesh_id);
        }
    }

    vars_valid.resize(ndomains);
    for (dom = 0; dom < ndomains; ++dom)
        vars_valid[dom].resize(vars.size());

    if (in.fail())
        EXCEPTION1(InvalidFilesException, fname);
    
    in.close();

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
 
    //
    // If it ends in .m or .mili, strip it off.
    //
    int len = strlen(root);
    if (len > 4 && strcmp(&(root[len - 5]), ".mili") == 0)
    {
        root[len - 5] = '\0';
    }
    else if (len > 1 && strcmp(&(root[len - 2]), ".m") == 0)
    {
        root[len - 2] = '\0';
    }
    else
    {
        EXCEPTION1(InvalidFilesException, fname);
    }

    famroot = new char[strlen(root) + 1];
    strcpy(famroot, root);
    if (path)
    {
        fampath = new char[strlen(path) + 1];
        strcpy(fampath, path);
    }
    else
       fampath = NULL; 
}


// ****************************************************************************
//  Destructor:  avtMiliFileFormat::~avtMiliFileFormat
//
//  Programmer:  Hank Childs
//  Creation:    April 11, 2003
//
//  Modifications:
//    Akira Haddox, Fri May 23 08:51:11 PDT 2003
//    Added in support for multiple meshes. Changed to MTMD.
//
//    Akira Haddox, Wed Jul 23 12:57:14 PDT 2003
//    Moved allocation of cached information to FreeUpResources.
//
// ****************************************************************************

avtMiliFileFormat::~avtMiliFileFormat()
{
    FreeUpResources();

    delete []famroot;
    if (fampath)
    delete []fampath;
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh for timestep 'ts' and domain 'dom'.
//
//  Arguments:
//    ts         the time step
//    dom        the domain
//    mesh       the name of the mesh to read
//
//  Programmer:  Hank Childs
//  Creation:    April 11, 2003
//
//  Modifications:
//    Akira Haddox, Fri May 23 08:51:11 PDT 2003
//    Added in support for multiple meshes. Changed to MTMD. 
//
//    Akira Haddox, Tue Jul 22 08:09:28 PDT 2003
//    Fixed the try block. Properly dealt with cell variable blocks.
//
// ****************************************************************************

vtkDataSet *
avtMiliFileFormat::GetMesh(int ts, int dom, const char *mesh)
{
    debug5 << "Reading in " << mesh << " for domain/ts : " << dom << ',' << ts
           << endl;
    if (!readMesh[dom])
        ReadMesh(dom);
    if (!validateVars[dom])
        ValidateVariables(dom);

    int mesh_id;

    //
    // The valid meshnames are meshX, where X is an int > 0.
    // We need to verify the name, and get the meshId.
    //
    if (strstr(mesh, "mesh") != mesh)
    {
        EXCEPTION1(InvalidVariableException, mesh);
    }
    
    //
    // Do a checked conversion to integer.
    //
    char *check;
    mesh_id = (int) strtol(mesh + 4, &check, 10);
    if (check != NULL && check[0] != '\0')
    {
        EXCEPTION1(InvalidVariableException, mesh)
    }
    --mesh_id;
    
    char *nodpos_str = "nodpos";

    //
    // The connectivity does not change over time, so use the one we have
    // already calculated.
    //
    vtkUnstructuredGrid *rv = vtkUnstructuredGrid::New();
    rv->ShallowCopy(connectivity[dom][mesh_id]);

    //
    // The node positions are stored in 'nodpos'.
    //
    int nodpos = GetVariableIndex(nodpos_str, mesh_id);

    int subrec = -1;
    for (int i = 0 ; i < vars_valid[dom][nodpos].size() ; i++)
    {
        if (vars_valid[dom][nodpos][i])
        {
            subrec = sub_record_ids[dom][i];
            break;
        }
    }
    if (subrec == -1)
    {
        EXCEPTION0(ImproperUseException);
    }

    float *fpts = new float[dims*nnodes[dom][mesh_id]];
    int rval = mc_read_results(dbid[dom], ts+1, subrec, 1, &nodpos_str, fpts);
    if (rval != OK)
    {
        EXCEPTION1(InvalidFilesException, filename);
    }

    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(nnodes[dom][mesh_id]);
    float *vpts = (float *) pts->GetVoidPointer(0);
    float *tmp = fpts;
    for (int pt = 0 ; pt < nnodes[dom][mesh_id] ; pt++)
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

    //
    // Add in the ghost levels (from sand variables), if it has any.
    //
   
    int v_index = -1;
    const char *name = "sand";
    
    TRY
    {
        v_index = GetVariableIndex(name, mesh_id);
    }
    CATCH(InvalidVariableException)
    {
        v_index = -1;
    }
    ENDTRY

    if (v_index >= 0)
    {
        bool someValid = false;
        vtkFloatArray *sandLevels = vtkFloatArray::New();
        sandLevels->SetNumberOfTuples(ncells[dom][mesh_id]);
        float *p = (float *) sandLevels->GetVoidPointer(0);
        int i;
        for (i = 0 ; i < ncells[dom][mesh_id] ; i++)
            p[i] = 1.;
        for (i = 0 ; i < vars_valid[dom][v_index].size() ; i++)
        {
            if (vars_valid[dom][v_index][i])
            {
                int start = 0;
                int csize = 0;
                GetSizeInfoForGroup(sub_records[dom][i].class_name, start, 
                                    csize, dom);
                char *tmp = (char *) name;  // Bypass const

                // Simple read in: one block 
                if (sub_records[dom][i].qty_blocks == 1)
                {
                    // Adjust start
                    start += (sub_records[dom][i].mo_blocks[0] - 1);
                
                    int rval = mc_read_results(dbid[dom], ts+1,
                                 sub_record_ids[dom][i],
                                 1, &tmp, p + start);
                    if (rval != OK)
                    {
                        EXCEPTION1(InvalidVariableException, name);
                    }
                }
                else
                {
                    int nBlocks = sub_records[dom][i].qty_blocks;
                    int *blocks = sub_records[dom][i].mo_blocks;

                    int pSize = 0;
                    int b;
                    for (b = 0; b < nBlocks; ++b)
                        pSize += blocks[b * 2 + 1] - blocks[b * 2] + 1;

                    float *arr = new float[pSize];

                    int rval = mc_read_results(dbid[dom], ts + 1,
                                    sub_record_ids[dom][i],
                                    1, &tmp, arr);

                    if (rval != OK)
                    {
                        delete [] arr;
                        EXCEPTION1(InvalidVariableException, name);
                    }
                    

                    float *ptr = arr;
                    // Fill up the blocks into the array.
                    for (b = 0; b < nBlocks; ++b)
                    {
                        int c;
                        for (c = blocks[b * 2] - 1; c <= blocks[b * 2 + 1] - 1;
                                                    ++c)
                            p[c + start] = *(ptr++);
                    }
                    
                    delete [] arr;
                }
            }
        }        
      
        vtkUnsignedCharArray *ghostLevels = vtkUnsignedCharArray::New(); 
        ghostLevels->SetNumberOfTuples(ncells[dom][mesh_id]);
        unsigned char *ptr = (unsigned char *)ghostLevels->GetVoidPointer(0);
        for (i = 0; i < ncells[dom][mesh_id]; ++i)
        {
            // If the sand value is 0, we want to drop it as a ghost zone.
            if (p[i] < 0.5)
            {
                ptr[i] = 1;
                someValid = true;
            }
            else
            {
                ptr[i] = 0;
            }
        }

        if (someValid)
        {
            ghostLevels->SetName("vtkGhostLevels");
            rv->GetCellData()->AddArray(ghostLevels);
        }
        sandLevels->Delete();
        ghostLevels->Delete();
        rv->SetUpdateGhostLevel(0);
    } 

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
//  Method: avtMiliFileFormat::GetVariableIndex
//
//  Purpose:
//      Gets the index of a variable that is associated with a particular
//      mesh.
//
//  Programmer: Akira Haddox
//  Creation:   May 23, 2003
//
// ****************************************************************************

int
avtMiliFileFormat::GetVariableIndex(const char *varname, int mesh_id)
{
    for (int i = 0 ; i < vars.size() ; i++)
    {
        if (vars[i] == varname && var_mesh_associations[i] == mesh_id)
        {
            return i;
        }
    }

    EXCEPTION1(InvalidVariableException, varname);
}

// ****************************************************************************
//  Method: avtMiliFileFormat::DecodeMultiMeshVarname
//
//  Purpose:
//      Takes in a variable name used to populate, and returns the
//      original variable name, and associated mesh id.
//
//  Programmer: Akira Haddox
//  Creation:   June 26, 2003
//
// ****************************************************************************

void
avtMiliFileFormat::DecodeMultiMeshVarname(const string &varname,
                                          string &decoded, int &meshid)
{
    decoded = varname;
    meshid = 0;

    char *ptr = &(decoded[0]);
    while(*ptr != '\0')
    {
        if(*ptr == '(')
            break;    
        ++ptr;
    }
    
    if (*ptr == '\0')
        return;

    char *check;
    meshid = (int) strtol(ptr + strlen("(mesh"), &check, 10);
    --meshid;

    *ptr = '\0';
}

// ****************************************************************************
//  Method: avtMiliFileFormat::OpenDB
//
//  Purpose:
//      Open up a family database for a given domain.
//
//  Programmer: Akira Haddox
//  Creation:   June 26, 2003
//
//  Modifications:
//    Akira Haddox, Tue Jul 22 15:34:40 PDT 2003
//    Added in setting of times.
//
// ****************************************************************************

void
avtMiliFileFormat::OpenDB(int dom)
{
    if (dbid[dom] == -1)
    {
        int rval;
        if (ndomains == 1)
        {
            rval = mc_open( famroot, fampath, "r", &(dbid[dom]) );
            if ( rval != OK )
                EXCEPTION1(InvalidFilesException, famroot);
        }
        else
        {
            char famname[128];
            sprintf(famname, "%s%.3d", famroot, dom);
            rval = mc_open( famname, fampath, "r", &(dbid[dom]) );
            if ( rval != OK )
                EXCEPTION1(InvalidFilesException, famname);
        }

        //
        // The first domain that we open, we use to find the times.
        //
        if (!setTimesteps)
        {
            setTimesteps = true;
            //
            // First entry is how many timesteps we want (all of them),
            // the following entries are which timesteps we want.
            // The index is 1 based.
            //
            vector<int> timeVars(ntimesteps + 1);
            timeVars[0] = ntimesteps;
            int i;
            for (i = 1; i <= ntimesteps; ++i)
                timeVars[i] = i;

            vector<float> times(ntimesteps);
            rval = mc_query_family(dbid[dom], MULTIPLE_TIMES, &(timeVars[0]),
                                    0, &(times[0]));
            if (rval == OK)
            {
                for (i = 0; i < ntimesteps; ++i)
                    metadata->SetTime(i, times[i]); 
            }
        }
    }
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
//  Modifications
//    Akira Haddox, Fri May 23 08:13:09 PDT 2003
//    Added in support for multiple meshes. Changed for MTMD.
//
// ****************************************************************************

void
avtMiliFileFormat::GetSizeInfoForGroup(const char *group_name, int &offset,
                                       int &g_size, int dom)
{
    if (!readMesh[dom])
        ReadMesh(dom);

    int g_index = -1;
    for (int i = 0 ; i < element_group_name[dom].size() ; i++)
    {
        if (element_group_name[dom][i] == group_name)
        {
            g_index = i;
            break;
        }
    }
    if (g_index == -1)
    {
        EXCEPTION0(ImproperUseException);
    }
    int mesh_id = group_mesh_associations[dom][g_index];

    offset = connectivity_offset[dom][g_index];
    if (g_index == (connectivity_offset[dom].size()-1))
    {
        g_size = ncells[dom][mesh_id] - connectivity_offset[dom][g_index];
    }
    else
    {
        g_size = connectivity_offset[dom][g_index+1] 
                 - connectivity_offset[dom][g_index];
    }
}


// ****************************************************************************
//  Method: avtMiliFileFormat::ReadMesh
//
//  Purpose:
//      Read the connectivity for the meshes in a certain domain.
//
//  Programmer: Hank Childs (adapted by Akira Haddox)
//  Creation:   June 25, 2003
//
//  Modifications:
//    Akira Haddox, Tue Jul 22 09:21:39 PDT 2003
//    Changed ConstructMaterials call to match new signature.
//
// ****************************************************************************

void
avtMiliFileFormat::ReadMesh(int dom)
{
    if (dbid[dom] == -1)
        OpenDB(dom);

    char short_name[1024];
    char long_name[1024];

    //
    // Read in the meshes.
    //

    int mesh_id;
    for (mesh_id = 0; mesh_id < nmeshes; ++mesh_id)
    {
        //
        // Determine the number of nodes.
        //
        int node_id = 0;
        mc_get_class_info(dbid[dom], mesh_id, M_NODE, node_id, short_name, 
                          long_name, &nnodes[dom][mesh_id]);

        //
        // Determine the connectivity.  This will also calculate the number of
        // cells.
        //
        connectivity[dom][mesh_id] = vtkUnstructuredGrid::New();

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
        ncells[dom][mesh_id] = 0;
        int ncoords = 0;
        for (i = 0 ; i < n_elem_types ; i++)
        {
            int args[2];
            args[0] = mesh_id;
            args[1] = elem_sclasses[i];
            int ngroups = 0;
            mc_query_family(dbid[dom], QTY_CLASS_IN_SCLASS, (void*) args, NULL,
                            (void*) &ngroups);
            for (j = 0 ; j < ngroups ; j++)
            {
                int nelems;
                mc_get_class_info(dbid[dom], mesh_id, elem_sclasses[i], j,
                                  short_name, long_name, &nelems);
                  
                int *conn = new int[nelems * conn_count[i]];
                int *mat = new int[nelems];
                int *part = new int[nelems];
                mc_load_conns(dbid[dom], mesh_id, short_name, conn, mat, part);
                
                conn_list[i].push_back(conn);
                mat_list[i].push_back(mat);
                list_size[i].push_back(nelems);
                connectivity_offset[dom].push_back(ncells[dom][mesh_id]);
                element_group_name[dom].push_back(short_name);
                group_mesh_associations[dom].push_back(mesh_id);
                ncells[dom][mesh_id] += list_size[i][j];
                ncoords += list_size[i][j] * (conn_count[i] + 1);
                delete [] part;
            }
        }

        //
        // Allocate an appropriately sized dataset using that connectivity
        // information.
        //
        connectivity[dom][mesh_id]->Allocate(ncells[dom][mesh_id], ncoords);

        //
        // The materials are in a format that is not AVT friendly.  Convert it
        // now.
        //
        materials[dom][mesh_id] = ConstructMaterials(mat_list, list_size, 
                                                     mesh_id);

        //
        // Now that we have our avtMaterial, we can deallocate the 
        // old pure material data.
        //
        for (i = 0; i < mat_list.size(); ++i)
            for (j = 0; j < mat_list[i].size(); ++j)
                delete[] (mat_list[i][j]);

        //
        // Now construct the connectivity in a VTK dataset.
        //
        for (i = 0 ; i < n_elem_types ; i++)
        {
            for (j = 0 ; j < conn_list[i].size() ; j++)
            {
                int *conn = conn_list[i][j];
                int nelems = list_size[i][j];
                int *newconn;
                for (k = 0 ; k < nelems ; k++)
                {
                    switch (elem_sclasses[i])
                    {
                      case M_TRUSS:
                        connectivity[dom][mesh_id]->InsertNextCell(VTK_LINE,
                                                 conn_count[i], conn);
                        break;
                      case M_BEAM:
                        newconn = new int[2];
                        newconn[0] = conn[0];
                        newconn[1] = conn[1];
                        delete[] conn;
                        connectivity[dom][mesh_id]->InsertNextCell(VTK_LINE,
                                                 conn_count[i], newconn);
                        break;
                      case M_TRI:
                        connectivity[dom][mesh_id]->InsertNextCell(VTK_TRIANGLE,
                                                 conn_count[i], conn);
                        break;
                      case M_QUAD:
                        connectivity[dom][mesh_id]->InsertNextCell(VTK_QUAD,
                                                     conn_count[i], conn);
                        break;
                      case M_TET:
                        connectivity[dom][mesh_id]->InsertNextCell(VTK_TETRA,
                                                     conn_count[i], conn);
                        break;
                      case M_PYRAMID:
                        connectivity[dom][mesh_id]->InsertNextCell(VTK_PYRAMID,
                                                     conn_count[i], conn);
                        break;
                      case M_WEDGE:
                        connectivity[dom][mesh_id]->InsertNextCell(VTK_WEDGE,
                                                     conn_count[i], conn);
                        break;
                      case M_HEX:
                        connectivity[dom][mesh_id]->InsertNextCell(VTK_HEXAHEDRON,
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
    }// end mesh reading loop

    readMesh[dom] = true;
}

// ****************************************************************************
//  Method: avtMiliFileFormat::ValidateVariables
//
//  Purpose:
//      Read in the information to determine which vars are valid for
//      which subrecords. Also read in subrecord info.
//
//  Programmer: Hank Childs (adapted by Akira Haddox)
//  Creation:   June 25, 2003
//
//  Modifications:
//    Akira Haddox, Wed Jul 23 09:47:30 PDT 2003
//    Adapted code to assume it knows all the variables (which are now
//    obtained from the .mili file). Set validate vars flag after run.
//    Changed sub_records to hold the mili Subrecord structure.
//
// ****************************************************************************

void
avtMiliFileFormat::ValidateVariables(int dom)
{
    int rval;
    if (dbid[dom] == -1)
        OpenDB(dom);

    int srec_qty = 0;
    rval = mc_query_family(dbid[dom], QTY_SREC_FMTS, NULL, NULL,
                           (void*) &srec_qty);

    int i;
    for (i = 0 ; i < srec_qty ; i++)
    {
        int substates = 0;
        rval = mc_query_family(dbid[dom], QTY_SUBRECS, (void *) &i, NULL,
                               (void *) &substates);

        for (int j = 0 ; j < substates ; j++)
        {
            Subrecord sr;
            rval = mc_get_subrec_def(dbid[dom], i, j, &sr);
        
            //
            // glob vars are associated with the entire mesh.
            // mat vars deal with materials.
            // These are not variables we need to visualize. Skip them.
            //
            
            if (strcmp(sr.class_name, "glob") == 0)
            {
                continue;
            }
            if (strcmp(sr.class_name, "mat") == 0)
            {
                continue;
            }

            sub_records[dom].push_back(sr);
            sub_record_ids[dom].push_back(j);

            //
            // To date, we believe none of the variables are valid for this
            // subrecord.  This will change as we look through its variable
            // list.
            //
            for (int vv = 0 ; vv < vars.size() ; vv++)
                 vars_valid[dom][vv].push_back(false);
            int index = sub_records[dom].size() - 1;
           
            for (int k = 0 ; k < sr.qty_svars ; k++)
            {
                 State_variable sv;
                 mc_get_svar_def(dbid[dom], sr.svar_names[k], &sv);

                 int sameAsVar = -1;
                 for (int v = 0 ; v < vars.size() ; v++)
                     if (vars[v] == sv.short_name)
                     {
                         sameAsVar = v;
                         break;
                     }

                 //
                 // If we didn't find the variable, then something has gone
                 // wrong - We should have known it from the .mili file.
                 //
                 if (sameAsVar == -1)
                 {
                     EXCEPTION1(InvalidVariableException, sv.short_name);
                 }

                 vars_valid[dom][sameAsVar][index] = true;
            }
        }
    }

    validateVars[dom] = true;
}


// ****************************************************************************
//  Method: avtMiliFileFormat::ConstructMaterials
//
//  Purpose:
//      Constructs a material list from the partial material lists.
//
//  Programmer: Akira Haddox
//  Creation:   May 22, 2003
//
//  Modifications:
//    Akira Haddox, Tue Jul 22 09:21:39 PDT 2003
//    Find number of materials from global mesh information.
//
// ****************************************************************************

avtMaterial *
avtMiliFileFormat::ConstructMaterials(vector< vector<int *> > &mat_list,
                                      vector< vector<int> > &list_size, 
                                      int meshId)
{
    int size = 0;
    int i, j;
    for (i = 0; i < list_size.size(); ++i)
        for (j = 0; j < list_size[i].size(); ++j)
            size += list_size[i][j];

    int * mlist = new int[size];

    //
    // Fill in the material type sequentially for each zone. We go in
    // order of increasing element type.
    // 
    int elem, gr;
    int count = 0;
    for (elem = 0; elem < mat_list.size(); ++elem)
    {
        for (gr = 0; gr < mat_list[elem].size(); ++gr)
        {
            int *ml = mat_list[elem][gr];
            for (i = 0; i < list_size[elem][gr]; ++i)
            {
                int mat = ml[i]; 
                mlist[count++] = mat;
            }
        }
    }
    
    vector<string> mat_names(nmaterials[meshId]);
    char str[32];
    for (i = 0; i < mat_names.size(); ++i)
    {
        sprintf(str, "mat%d", i);
        mat_names[i] = str;
    }

    avtMaterial * mat = new avtMaterial(nmaterials[meshId], mat_names, size, 
                                        mlist, 0, NULL, NULL, NULL, NULL);

    delete []mlist;
    return mat;
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
//  Modifications
//    Akira Haddox, Fri May 23 08:13:09 PDT 2003
//    Added in support for multiple meshes. Changed to MTMD.
//
//    Akira Haddox, Thu Jul 24 13:36:38 PDT 2003
//    Properly dealt with cell variable blocks.
//
// ****************************************************************************

vtkDataArray *
avtMiliFileFormat::GetVar(int ts, int dom, const char *name)
{
    if (!readMesh[dom])
        ReadMesh(dom);
    if (!validateVars[dom])
        ValidateVariables(dom);

    string vname;
    int meshid = 0;
    if (nmeshes == 1)
        vname = name;
    else
        DecodeMultiMeshVarname(name, vname, meshid);
    
    vtkFloatArray *rv = vtkFloatArray::New();

    int v_index = GetVariableIndex(vname.c_str(), meshid);
    int mesh_id = var_mesh_associations[v_index];

    if (centering[v_index] == AVT_NODECENT)
    {
        int nvars = 0;
        int sr_valid = -1;
        for (int i = 0 ; i < vars_valid[dom][v_index].size() ; i++)
        {
            if (vars_valid[dom][v_index][i])
            {
                sr_valid = i;
                nvars++;
            }
        }
        if (nvars != 1)
        {
            EXCEPTION1(InvalidVariableException, name);
        }

        rv->SetNumberOfTuples(nnodes[dom][mesh_id]);
        float *p = (float *) rv->GetVoidPointer(0);
        char *tmp = (char *) name;  // Bypass const
        int rval = mc_read_results(dbid[dom], ts+1, sub_record_ids[dom][sr_valid],
                                    1, &tmp, p);
        if (rval != OK)
        {
            EXCEPTION1(InvalidVariableException, name);
        }
    }
    else
    {
        rv->SetNumberOfTuples(ncells[dom][mesh_id]);
        float *p = (float *) rv->GetVoidPointer(0);
        int i;
        for (i = 0 ; i < ncells[dom][mesh_id] ; i++)
            p[i] = 0.;
        for (i = 0 ; i < vars_valid[dom][v_index].size() ; i++)
        {
            if (vars_valid[dom][v_index][i])
            {
                int start = 0;
                int csize = 0;
                GetSizeInfoForGroup(sub_records[dom][i].class_name, start,
                                    csize, dom);

                char *tmp = (char *) name;  // Bypass const
                
                // Simple read in: one block 
                if (sub_records[dom][i].qty_blocks == 1)
                {
                    // Adjust start
                    start += (sub_records[dom][i].mo_blocks[0] - 1);
                
                    int rval = mc_read_results(dbid[dom], ts+1,
                                 sub_record_ids[dom][i],
                                 1, &tmp, p + start);
                    if (rval != OK)
                    {
                        EXCEPTION1(InvalidVariableException, name);
                    }
                }
                else
                {
                    int nBlocks = sub_records[dom][i].qty_blocks;
                    int *blocks = sub_records[dom][i].mo_blocks;

                    int pSize = 0;
                    int b;
                    for (b = 0; b < nBlocks; ++b)
                        pSize += blocks[b * 2 + 1] - blocks[b * 2] + 1;

                    float *arr = new float[pSize];

                    int rval = mc_read_results(dbid[dom], ts + 1,
                                    sub_record_ids[dom][i],
                                    1, &tmp, arr);

                    if (rval != OK)
                    {
                        delete [] arr;
                        EXCEPTION1(InvalidVariableException, name);
                    }
                    

                    float *ptr = arr;
                    // Fill up the blocks into the array.
                    for (b = 0; b < nBlocks; ++b)
                    {
                        int c;
                        for (c = blocks[b * 2] - 1; c <= blocks[b * 2 + 1] - 1;
                                                    ++c)
                            p[c + start] = *(ptr++);
                    }
                    
                    delete [] arr;
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
//  Modifications
//    Akira Haddox, Fri May 23 08:13:09 PDT 2003
//    Added in support for multiple meshes. Changed to MTMD.
//
//    Akira Haddox, Thu Jul 24 13:36:38 PDT 2003
//    Properly dealt with cell variable blocks.
//
// ****************************************************************************

vtkDataArray *
avtMiliFileFormat::GetVectorVar(int ts, int dom, const char *name)
{
    if (!readMesh[dom])
        ReadMesh(dom);
    if (!validateVars[dom])
        ValidateVariables(dom);

    string vname;
    int meshid = 0;
    if (nmeshes == 1)
        vname = name;
    else
        DecodeMultiMeshVarname(name, vname, meshid);
    
    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(dims);

    int v_index = GetVariableIndex(vname.c_str(), meshid);
    int mesh_id = var_mesh_associations[v_index];

    if (centering[v_index] == AVT_NODECENT)
    {
        int nvars = 0;
        int sr_valid = -1;
        for (int i = 0 ; i < vars_valid[dom][v_index].size() ; i++)
        {
            if (vars_valid[dom][v_index][i])
            {
                sr_valid = i;
                nvars++;
            }
        }
        if (nvars != 1)
        {
            EXCEPTION1(InvalidVariableException, name);
        }

        rv->SetNumberOfTuples(nnodes[dom][mesh_id]);
        float *ptr = (float *) rv->GetVoidPointer(0);
        char *tmp = (char *) name;  // Bypass const
        int rval = mc_read_results(dbid[dom], ts+1, sub_record_ids[dom][sr_valid],
                                    1, &tmp, ptr);
        if (rval != OK)
        {
            EXCEPTION1(InvalidVariableException, name);
        }
    }
    else
    {
        rv->SetNumberOfTuples(ncells[dom][mesh_id]);
        float *p = (float *) rv->GetVoidPointer(0);
        int i;
        int nvals = ncells[dom][mesh_id] * dims;
        for (i = 0 ; i < nvals ; i++)
            p[i] = 0.;
        for (int i = 0 ; i < vars_valid[dom][v_index].size() ; i++)
        {
            if (vars_valid[dom][v_index][i])
            {
                int start = 0;
                int csize = 0;
                GetSizeInfoForGroup(sub_records[dom][i].class_name, start, 
                                    csize, dom);

                char *tmp = (char *) name;  // Bypass const

                // Simple read in: one block 
                if (sub_records[dom][i].qty_blocks == 1)
                {
                    // Adjust start
                    start += (sub_records[dom][i].mo_blocks[0] - 1);
                
                    int rval = mc_read_results(dbid[dom], ts+1,
                                 sub_record_ids[dom][i],
                                 1, &tmp, p + dims * start);
                    if (rval != OK)
                    {
                        EXCEPTION1(InvalidVariableException, name);
                    }
                }
                else
                {
                    int nBlocks = sub_records[dom][i].qty_blocks;
                    int *blocks = sub_records[dom][i].mo_blocks;

                    int pSize = 0;
                    int b;
                    for (b = 0; b < nBlocks; ++b)
                        pSize += blocks[b * 2 + 1] - blocks[b * 2] + 1;

                    float *arr = new float[pSize * dims];

                    int rval = mc_read_results(dbid[dom], ts + 1,
                                    sub_record_ids[dom][i],
                                    1, &tmp, arr);

                    if (rval != OK)
                    {
                        delete [] arr;
                        EXCEPTION1(InvalidVariableException, name);
                    }
                    

                    float *ptr = arr;
                    // Fill up the blocks into the array.
                    for (b = 0; b < nBlocks; ++b)
                    {
                        int c, k;
                        for (c = blocks[b * 2] - 1; c <= blocks[b * 2 + 1] - 1;
                                                    ++c)
                            for (k = 0; k < dims; ++k)
                                p[dims * (c + start) + k] = *(ptr++);
                    }
                    
                    delete [] arr;
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
//  Modifications
//    Akira Haddox, Fri May 23 08:13:09 PDT 2003
//    Added in support for multiple meshes. Changed for MTMD.
//
// ****************************************************************************

void
avtMiliFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    int i;
    for (i = 0; i < nmeshes; ++i)
    {
        char meshname[32];
        char matname[32];
        sprintf(meshname, "mesh%d", i + 1);
        sprintf(matname, "materials%d", i + 1);
        AddMeshToMetaData(md, meshname, AVT_UNSTRUCTURED_MESH, NULL, ndomains,
                          0, dims, dims);
        vector<string> mnames(nmaterials[i]);
        int j;
        char str[32];
        for (j = 0; j < nmaterials[i]; ++j)
        {
            sprintf(str, "mat%d", j);
            mnames[j] = str;
        }
        AddMaterialToMetaData(md, matname, meshname, nmaterials[i], mnames);
    }

    for (i = 0 ; i < vars.size() ; i++)
    {
        char meshname[32];
        sprintf(meshname, "mesh%d", var_mesh_associations[i] + 1);

        //
        // Don't list the node position variable.
        //
        if (vars[i] == "nodpos")
            continue;
        
        string *vname;
        string mvnStr;
        char multiVname[64];
        if (nmeshes == 1)
            vname = &(vars[i]);
        else
        {
            sprintf(multiVname, "%s(mesh%d)", vars[i].c_str(), 
                                              var_mesh_associations[i] + 1);
            mvnStr = multiVname;
            vname = &mvnStr;
        }
        
        switch (vartype[i])
        {
          case AVT_SCALAR_VAR:
            AddScalarVarToMetaData(md, *vname, meshname, centering[i]);
            break;
          case AVT_VECTOR_VAR:
            AddVectorVarToMetaData(md, *vname, meshname, centering[i], dims);
            break;
          default:
            break;
        }
    }
}

// ****************************************************************************
//  Method: avtMiliFileFormat::GetAuxiliaryData
//
//  Purpose:
//      Gets the auxiliary data specified.
//
//  Arguments:
//      var        The variable of interest.
//      ts         The timestep of interest.
//      dom        The domain of interest.
//      type       The type of auxiliary data.
//      <unnamed>  The arguments for that type -- not used.
//      df         Destructor function.
//
//  Returns:    The auxiliary data.
//
//  Programmer: Akira Haddox
//  Creation:   May 23, 2003
//
//  Modifications
//    Akira Haddox, Fri May 23 08:13:09 PDT 2003
//    Added in support for multiple meshes. Changed for MTMD.
//
// ****************************************************************************
 
void *
avtMiliFileFormat::GetAuxiliaryData(const char *var, int ts, int dom, 
                                    const char * type, void *,
                                    DestructorFunction &df) 
{
    if (strcmp(type, AUXILIARY_DATA_MATERIAL))
        return NULL;

    if (!readMesh[dom])
        ReadMesh(dom);
    
    //
    // The valid variables are meshX, where X is an int > 0.
    // We need to verify the name, and get the meshId.
    //
    if (strstr(var, "materials") != var)
    {
        EXCEPTION1(InvalidVariableException, var);
    }
 
    //
    // Do a checked conversion to integer.
    //
    char *check;
    int mesh_id = (int) strtol(var + strlen("materials"), &check, 10);
    if (check != NULL && check[0] != '\0')
    {
        EXCEPTION1(InvalidVariableException, var)
    }
    --mesh_id;
 
    avtMaterial *myCopy = materials[dom][mesh_id];
    avtMaterial *mat = new avtMaterial(myCopy->GetNMaterials(),
                                       myCopy->GetMaterials(),
                                       myCopy->GetNZones(),
                                       myCopy->GetMatlist(),
                                       myCopy->GetMixlen(),
                                       myCopy->GetMixMat(),
                                       myCopy->GetMixNext(),
                                       myCopy->GetMixZone(),
                                       myCopy->GetMixVF());
    df = avtMaterial::Destruct;
    return (void*) mat;
}


// ****************************************************************************
//  Method: avtMiliFileFormat::FreeUpResources
//
//  Purpose:
//      Close databases and free up non-essential memory.
//
//  Programmer: Akira Haddox
//  Creation:   July 23, 2003
//
// ****************************************************************************

void
avtMiliFileFormat::FreeUpResources()
{
    //
    // Close mili databases, and delete non-essential allocated memory.
    // Keep the original sizes of vectors though.
    //
    int i, j;
    for (i = 0; i < ndomains; ++i)
        if (dbid[i] != -1)
        {
            mc_close(dbid[i]);
            dbid[i] = -1;
        }
    for (i = 0; i < connectivity.size(); ++i)
        for (j = 0; j < connectivity[i].size(); ++j)
            if (connectivity[i][j] != NULL)
            {
                connectivity[i][j]->Delete();
                connectivity[i][j] = NULL;
            }
    for (i = 0; i < materials.size(); ++i)
        for (j = 0; j < materials[i].size(); ++j)
            if (materials[i][j])
            {
                delete materials[i][j];
                materials[i][j] = NULL;
            }

    //
    // Reset flags to indicate the meshes needs to be read in again.
    //
    for (i = 0; i < ndomains; ++i)
    {
        readMesh[i] = false;
    }
}
