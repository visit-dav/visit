// ************************************************************************* //
//                              avtMiliFileFormat.C                          //
// ************************************************************************* //

#include <avtMiliFileFormat.h>

#include <vector>
#include <string>
#include <fstream>
#include <set>

extern "C" {
#include <mili_enum.h>
}

#include <vtkCellData.h>
#include <vtkCellTypes.h>
#include <vtkUnstructuredGrid.h>
#include <vtkFloatArray.h>

#include <Expression.h>

#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtMaterial.h>
#include <avtVariableCache.h>
#include <avtUnstructuredPointBoundaries.h>

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
//    Akira Haddox, Fri Jul 25 11:09:13 PDT 2003
//    Added reading in of variable dimensions.
//
//    Akira Haddox, Mon Aug 18 14:33:15 PDT 2003
//    Added partition file support.
//
//    Hank Childs, Tue Jul 20 15:53:30 PDT 2004
//    Add support for more data types (float, double, char, int, etc).
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
            int type, center, v_dimension;
            string name;
            in >> type >> center >> v_dimension;

            // Strip out leading white space.
            while(isspace(in.peek()))
                in.get();
            
            getline(in, name);

            vars.push_back(name);
            centering.push_back(avtCentering(center));
            vartype.push_back(avtVarType(type));
            var_dimension.push_back(v_dimension);
            var_mesh_associations.push_back(mesh_id);
        }
    }

    vars_valid.resize(ndomains);
    var_size.resize(ndomains);
    for (dom = 0; dom < ndomains; ++dom)
    {
        vars_valid[dom].resize(vars.size());
        var_size[dom].resize(vars.size());
    }

    if (in.fail())
        EXCEPTION1(InvalidFilesException, fname);

    // Read part file, if it exists.
    dynaPartFilename = "";
    in >> dynaPartFilename;
    if (dynaPartFilename != "")
        readPartInfo = false;
    else
        readPartInfo = true;
    
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
//    Hank Childs, Tue Jul 27 10:40:44 PDT 2004
//    Sucked in code from FreeUpResources.
//
// ****************************************************************************

avtMiliFileFormat::~avtMiliFileFormat()
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
    connectivity.clear();
    materials.clear();

    //
    // Reset flags to indicate the meshes needs to be read in again.
    //
    for (i = 0; i < ndomains; ++i)
        readMesh[i] = false;

    delete [] famroot;
    if (fampath)
        delete [] fampath;
}


// ****************************************************************************
//  Function: read_results
//
//  Purpose:
//      A wrapper around mc_read_results that handles multiple types (floats,
//      doubles, etc.).
//
//  Programmer: Hank Childs
//  Creation:   July 20, 2004
//
// ****************************************************************************

static void
read_results(Famid &dbid, int ts, int sr, int rank,
                      char **name, int vtype, int amount, float *buff)
{
    int  i;

    void *buff_to_read_into = NULL;
    switch (vtype)
    {
      case M_STRING:
        buff_to_read_into = new char[amount];
        break;
      case M_FLOAT:
      case M_FLOAT4:
        buff_to_read_into = buff;
        break;
      case M_FLOAT8:
        buff_to_read_into = new double[amount];
        break;
      case M_INT:
      case M_INT4:
        buff_to_read_into = new int[amount];
        break;
      case M_INT8:
        buff_to_read_into = new long[amount];
        break;
    }

    int rval = mc_read_results(dbid, ts, sr, rank, name, buff_to_read_into);
    if (rval != OK)
    {
        EXCEPTION1(InvalidVariableException, name[0]);
    }

    char   *c_tmp = NULL;
    double *d_tmp = NULL;
    int    *i_tmp = NULL;
    long   *l_tmp = NULL;
    switch (vtype)
    {
       case M_STRING:
         c_tmp = (char *) buff_to_read_into;
         for (i = 0 ; i < amount ; i++)
             buff[i] = (float)(c_tmp[i]);
         delete [] c_tmp;
         break;
       case M_FLOAT:
       case M_FLOAT4:
         break;
       case M_FLOAT8:
         d_tmp = (double *) buff_to_read_into;
         for (i = 0 ; i < amount ; i++)
             buff[i] = (float)(d_tmp[i]);
         delete [] d_tmp;
         break;
       case M_INT:
       case M_INT4:
         i_tmp = (int *) buff_to_read_into;
         for (i = 0 ; i < amount ; i++)
             buff[i] = (float)(i_tmp[i]);
         delete [] i_tmp;
         break;
       case M_INT8:
         l_tmp = (long *) buff_to_read_into;
         for (i = 0 ; i < amount ; i++)
             buff[i] = (float)(l_tmp[i]);
         delete [] l_tmp;
         break;
    }
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
//    Akira Haddox, Mon Aug 18 14:33:15 PDT 2003
//    Commented out previous sand-based ghosts.
//
//    Hank Childs, Sat Jun 26 11:24:47 PDT 2004
//    Check for bad files where number of timesteps is incorrectly reported.
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
    int vsize = M_FLOAT;
    for (int i = 0 ; i < vars_valid[dom][nodpos].size() ; i++)
    {
        if (vars_valid[dom][nodpos][i])
        {
            subrec = sub_record_ids[dom][i];
            vsize = var_size[dom][nodpos][i];
            break;
        }
    }
    if (subrec == -1)
    {
        EXCEPTION0(ImproperUseException);
    }

    int amt = dims*nnodes[dom][mesh_id];
    float *fpts = new float[amt];
    read_results(dbid[dom], ts+1, subrec, 1, &nodpos_str, vsize, amt, fpts);

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
    // This code for ghost levels from sand variables has been commented
    // out, since they conflict with the real ghost zones that we can
    // generate.
    //
#if 0
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
#endif

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
//
//    Akira Haddox, Tue Jul 22 15:34:40 PDT 2003
//    Added in setting of times.
//
//    Hank Childs, Mon Oct 20 10:03:58 PDT 2003
//    Made a new data member for storing times.  Populated that here.
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

            vector<float> ttimes(ntimesteps);
            rval = mc_query_family(dbid[dom], MULTIPLE_TIMES, &(timeVars[0]),
                                    0, &(ttimes[0]));

            //
            // Some Mili files are written out incorrectly -- they have an
            // extra timestep at the end with no data.  Detect and ignore.
            //
            if (ntimesteps >= 2)
                if (ttimes[ntimesteps-1] == ttimes[ntimesteps-2])
                    ntimesteps--;

            times.clear();
            if (rval == OK)
            {
                for (i = 0; i < ntimesteps; ++i)
                {
                    times.push_back(ttimes[i]);
                }
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
    {
        ReadMesh(dom);
    }

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
//    Akira Haddox, Thu Aug  7 10:07:40 PDT 2003
//    Fixed beam support.
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
                for (k = 0 ; k < nelems ; k++)
                {
                    switch (elem_sclasses[i])
                    {
                      case M_TRUSS:
                        connectivity[dom][mesh_id]->InsertNextCell(VTK_LINE,
                                                 conn_count[i], conn);
                        break;
                      case M_BEAM:
                        // Beams are lines that have a third point to define
                        // the normal. Since we don't need to visualize it,
                        // we just drop the normal point.
                        connectivity[dom][mesh_id]->InsertNextCell(VTK_LINE,
                                                 2, conn);
                        break;
                      case M_TRI:
                        connectivity[dom][mesh_id]->InsertNextCell(
                                                                  VTK_TRIANGLE,
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
                        connectivity[dom][mesh_id]->InsertNextCell(
                                                                VTK_HEXAHEDRON,
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
            {
                vars_valid[dom][vv].push_back(false);
                var_size[dom][vv].push_back(M_FLOAT);
            }
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
                 var_size[dom][sameAsVar][index] = sv.num_type;
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
//    Hank Childs, Tue Jul 20 15:53:30 PDT 2004
//    Add support for more data types (float, double, char, int, etc).
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
        int vsize = var_size[dom][v_index][sr_valid];

        int amt = nnodes[dom][mesh_id];
        rv->SetNumberOfTuples(amt);
        float *p = (float *) rv->GetVoidPointer(0);
        char *tmp = (char *) name;  // Bypass const
        read_results(dbid[dom], ts+1, sub_record_ids[dom][sr_valid], 1,
                        &tmp, vsize, amt, p);
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
                int vsize = var_size[dom][v_index][i];
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
                
                    read_results(dbid[dom], ts+1, sub_record_ids[dom][i],
                                 1, &tmp, vsize, csize, p + start);
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

                    read_results(dbid[dom], ts + 1, sub_record_ids[dom][i],
                                 1, &tmp, vsize, pSize, arr);

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
//    Hank Childs, Mon Sep 22 07:36:48 PDT 2003
//    Add support for reading in tensors.
//
//    Hank Childs, Tue Jul 20 15:53:30 PDT 2004
//    Add support for more data types (float, double, char, int, etc).
//
//    Hank Childs, Tue Jul 27 12:42:12 PDT 2004
//    Fix problem with reading in double nodal vectors.
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
    
    int v_index = GetVariableIndex(vname.c_str(), meshid);
    int mesh_id = var_mesh_associations[v_index];

    //
    // We stuff tensors into the vector field, so explicitly look up the
    // dimension of vector (3 for vector, 6 for symm tensor, 9 for tensor).
    //
    int vdim = var_dimension[v_index];

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(vdim);

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

        int vsize = var_size[dom][v_index][sr_valid];
        int amt = nnodes[dom][mesh_id];
        rv->SetNumberOfTuples(amt);
        float *ptr = (float *) rv->GetVoidPointer(0);
        char *tmp = (char *) name;  // Bypass const
        read_results(dbid[dom], ts+1, sub_record_ids[dom][sr_valid], 1,
                        &tmp, vsize, amt*vdim, ptr);
    }
    else
    {
        rv->SetNumberOfTuples(ncells[dom][mesh_id]);
        float *p = (float *) rv->GetVoidPointer(0);
        int i;
        int nvals = ncells[dom][mesh_id] * vdim;
        for (i = 0 ; i < nvals ; i++)
            p[i] = 0.;
        for (int i = 0 ; i < vars_valid[dom][v_index].size() ; i++)
        {
            if (vars_valid[dom][v_index][i])
            {
                int vsize = var_size[dom][v_index][i];
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
                
                    read_results(dbid[dom], ts+1, sub_record_ids[dom][i],
                                 1, &tmp, vsize, csize*vdim, p + vdim * start);
                }
                else
                {
                    int nBlocks = sub_records[dom][i].qty_blocks;
                    int *blocks = sub_records[dom][i].mo_blocks;

                    int pSize = 0;
                    int b;
                    for (b = 0; b < nBlocks; ++b)
                        pSize += blocks[b * 2 + 1] - blocks[b * 2] + 1;

                    float *arr = new float[pSize * vdim];

                    read_results(dbid[dom], ts + 1, sub_record_ids[dom][i],
                                    1, &tmp, vsize, pSize*vdim, arr);

                    float *ptr = arr;
                    // Fill up the blocks into the array.
                    for (b = 0; b < nBlocks; ++b)
                    {
                        int c, k;
                        for (c = blocks[b * 2] - 1; c <= blocks[b * 2 + 1] - 1;
                                                    ++c)
                            for (k = 0; k < vdim; ++k)
                                p[vdim * (c + start) + k] = *(ptr++);
                    }
                    
                    delete [] arr;
                }
            }
        }
    }

    //
    // If we have a symmetric tensor, put that in the form of a normal
    // tensor.
    //
    if (vdim == 6)
    {
        vtkFloatArray *new_rv = vtkFloatArray::New();
        int ntups = rv->GetNumberOfTuples();
        new_rv->SetNumberOfComponents(9);
        new_rv->SetNumberOfTuples(ntups);
        for (int i = 0 ; i < ntups ; i++)
        {
            float orig_vals[6];
            float new_vals[9];
            rv->GetTuple(i, orig_vals);
            new_vals[0] = orig_vals[0];  // XX
            new_vals[1] = orig_vals[3];  // XY
            new_vals[2] = orig_vals[5];  // XZ
            new_vals[3] = orig_vals[3];  // YX
            new_vals[4] = orig_vals[1];  // YY
            new_vals[5] = orig_vals[4];  // YZ
            new_vals[6] = orig_vals[5];  // ZX
            new_vals[7] = orig_vals[4];  // ZY
            new_vals[8] = orig_vals[2];  // ZZ
            new_rv->SetTuple(i, new_vals);
        }
        rv->Delete();
        rv = new_rv;
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
//  Method:  avtMiliFileFormat::GetTimes
//
//  Purpose:
//      Returns the actual times for each time step.
//
//  Arguments:
//   out_times   the output vector of times 
//
//  Programmer:  Hank Childs
//  Creation:    October 20, 2003
//
// ****************************************************************************

void
avtMiliFileFormat::GetTimes(vector<double> &out_times)
{
    out_times = times;
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
//    Hank Childs, Sat Sep 20 08:15:54 PDT 2003
//    Added support for tensors and add some expressions based on tensors.
//
//    Hank Childs, Sat Oct 18 09:51:03 PDT 2003
//    Fix typo for strain/stress expressions.
//
//    Hank Childs, Sat Oct 18 10:53:51 PDT 2003
//    Do not read in the partition info if we are on the mdserver.
//
//    Hank Childs, Mon Oct 20 10:07:00 PDT 2003
//    Call OpenDB for domain 0 to populate the times.
//
//    Hank Childs, Sat Jun 26 10:28:45 PDT 2004
//    Make the materials start at "1" and go up.  Also make the domain 
//    decomposition say processor instead of block.
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
        avtMeshMetaData *mesh = new avtMeshMetaData;
        mesh->name = meshname;
        mesh->meshType = AVT_UNSTRUCTURED_MESH;
        mesh->numBlocks = ndomains;
        mesh->blockOrigin = 0;
        mesh->cellOrigin = 0;
        mesh->spatialDimension = dims;
        mesh->topologicalDimension = dims;
        mesh->blockTitle = "processors";
        mesh->blockPieceName = "processor";
        mesh->hasSpatialExtents = false;
        md->Add(mesh);

        vector<string> mnames(nmaterials[i]);
        int j;
        char str[32];
        for (j = 0; j < nmaterials[i]; ++j)
        {
            sprintf(str, "mat%d", j+1);
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

        //
        // Determine if this is a tensor or a symmetric tensor.
        //
        if (vartype[i] == AVT_VECTOR_VAR && var_dimension[i] != dims)
        {
            if (dims == 3)
            {
                if (var_dimension[i] == 6)
                    vartype[i] = AVT_SYMMETRIC_TENSOR_VAR;
                else if (var_dimension[i] == 9)
                    vartype[i] = AVT_TENSOR_VAR;
                else
                    continue;
            }
            else if (dims == 2)      
            {
                if (var_dimension[i] == 3)
                    vartype[i] = AVT_SYMMETRIC_TENSOR_VAR;
                else if (var_dimension[i] == 4)
                    vartype[i] = AVT_TENSOR_VAR;
                else
                    continue;
            }
            else
                continue;
        }
        
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
          case AVT_SYMMETRIC_TENSOR_VAR:
            AddSymmetricTensorVarToMetaData(md, *vname, meshname, centering[i],
                                            dims);
            break;
          case AVT_TENSOR_VAR:
            AddTensorVarToMetaData(md, *vname, meshname, centering[i], dims);
            break;
          default:
            break;
        }
    }

    //
    // By calling OpenDB for domain 0, it will populate the times.
    //
    OpenDB(0);

    TRY
    {
        // This call throw an exception if stress does not exist.
        GetVariableIndex("stress");

        Expression pressure_expr;
        pressure_expr.SetName("derived/pressure");
        pressure_expr.SetDefinition("-trace(stress)/3");
        pressure_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&pressure_expr);

        Expression stressx_expr;
        stressx_expr.SetName("derived/stress_x");
        stressx_expr.SetDefinition("stress[0][0]");
        stressx_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&stressx_expr);

        Expression stressy_expr;
        stressy_expr.SetName("derived/stress_y");
        stressy_expr.SetDefinition("stress[1][1]");
        stressy_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&stressy_expr);

        Expression stressz_expr;
        stressz_expr.SetName("derived/stress_z");
        stressz_expr.SetDefinition("stress[2][2]");
        stressz_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&stressz_expr);

        Expression stressxy_expr;
        stressxy_expr.SetName("derived/stress_xy");
        stressxy_expr.SetDefinition("stress[0][1]");
        stressxy_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&stressxy_expr);

        Expression stressxz_expr;
        stressxz_expr.SetName("derived/stress_xz");
        stressxz_expr.SetDefinition("stress[0][2]");
        stressxz_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&stressxz_expr);

        Expression stressyz_expr;
        stressyz_expr.SetName("derived/stress_yz");
        stressyz_expr.SetDefinition("stress[1][2]");
        stressyz_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&stressyz_expr);

        Expression seff_expr;
        seff_expr.SetName("derived/eff_stress");
        seff_expr.SetDefinition("effective_tensor(stress)");
        seff_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&seff_expr);

        Expression p_dev_stress1_expr;
        p_dev_stress1_expr.SetName("derived/prin_dev_stress_1");
        p_dev_stress1_expr.SetDefinition(
                                     "principal_deviatoric_tensor(stress)[0]");
        p_dev_stress1_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&p_dev_stress1_expr);

        Expression p_dev_stress2_expr;
        p_dev_stress2_expr.SetName("derived/prin_dev_stress_2");
        p_dev_stress2_expr.SetDefinition(
                                     "principal_deviatoric_tensor(stress)[1]");
        p_dev_stress2_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&p_dev_stress2_expr);

        Expression p_dev_stress3_expr;
        p_dev_stress3_expr.SetName("derived/prin_dev_stress_3");
        p_dev_stress3_expr.SetDefinition(
                                     "principal_deviatoric_tensor(stress)[2]");
        p_dev_stress3_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&p_dev_stress3_expr);

        Expression maxshr_expr;
        maxshr_expr.SetName("derived/max_shear_stress");
        maxshr_expr.SetDefinition("tensor_maximum_shear(stress)");
        maxshr_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&maxshr_expr);

        Expression prin_stress1_expr;
        prin_stress1_expr.SetName("derived/prin_stress_1");
        prin_stress1_expr.SetDefinition("principal_tensor(stress)[0]");
        prin_stress1_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&prin_stress1_expr);

        Expression prin_stress2_expr;
        prin_stress2_expr.SetName("derived/prin_stress_2");
        prin_stress2_expr.SetDefinition("principal_tensor(stress)[1]");
        prin_stress2_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&prin_stress2_expr);

        Expression prin_stress3_expr;
        prin_stress3_expr.SetName("derived/prin_stress_3");
        prin_stress3_expr.SetDefinition("principal_tensor(stress)[2]");
        prin_stress3_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&prin_stress3_expr);
    }
    CATCH(InvalidVariableException)
    {
    }
    ENDTRY

    TRY
    {
        // This call throw an exception if strain does not exist.
        GetVariableIndex("strain");

        Expression strainx_expr;
        strainx_expr.SetName("derived/strain_x");
        strainx_expr.SetDefinition("strain[0][0]");
        strainx_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&strainx_expr);

        Expression strainy_expr;
        strainy_expr.SetName("derived/strain_y");
        strainy_expr.SetDefinition("strain[1][1]");
        strainy_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&strainy_expr);

        Expression strainz_expr;
        strainz_expr.SetName("derived/strain_z");
        strainz_expr.SetDefinition("strain[2][2]");
        strainz_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&strainz_expr);

        Expression strainxy_expr;
        strainxy_expr.SetName("derived/strain_xy");
        strainxy_expr.SetDefinition("strain[0][1]");
        strainxy_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&strainxy_expr);

        Expression strainxz_expr;
        strainxz_expr.SetName("derived/strain_xz");
        strainxz_expr.SetDefinition("strain[0][2]");
        strainxz_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&strainxz_expr);

        Expression strainyz_expr;
        strainyz_expr.SetName("derived/strain_yz");
        strainyz_expr.SetDefinition("strain[1][2]");
        strainyz_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&strainyz_expr);

        Expression seff_expr;
        seff_expr.SetName("derived/eff_strain");
        seff_expr.SetDefinition("effective_tensor(strain)");
        seff_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&seff_expr);

        Expression p_dev_strain1_expr;
        p_dev_strain1_expr.SetName("derived/prin_dev_strain_1");
        p_dev_strain1_expr.SetDefinition(
                                     "principal_deviatoric_tensor(strain)[0]");
        p_dev_strain1_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&p_dev_strain1_expr);

        Expression p_dev_strain2_expr;
        p_dev_strain2_expr.SetName("derived/prin_dev_strain_2");
        p_dev_strain2_expr.SetDefinition(
                                     "principal_deviatoric_tensor(strain)[1]");
        p_dev_strain2_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&p_dev_strain2_expr);

        Expression p_dev_strain3_expr;
        p_dev_strain3_expr.SetName("derived/prin_dev_strain_3");
        p_dev_strain3_expr.SetDefinition(
                                     "principal_deviatoric_tensor(strain)[2]");
        p_dev_strain3_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&p_dev_strain3_expr);

        Expression maxshr_expr;
        maxshr_expr.SetName("derived/max_shear_strain");
        maxshr_expr.SetDefinition("tensor_maximum_shear(strain)");
        maxshr_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&maxshr_expr);

        Expression prin_strain1_expr;
        prin_strain1_expr.SetName("derived/prin_strain_1");
        prin_strain1_expr.SetDefinition("principal_tensor(strain)[0]");
        prin_strain1_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&prin_strain1_expr);

        Expression prin_strain2_expr;
        prin_strain2_expr.SetName("derived/prin_strain_2");
        prin_strain2_expr.SetDefinition("principal_tensor(strain)[1]");
        prin_strain2_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&prin_strain2_expr);

        Expression prin_strain3_expr;
        prin_strain3_expr.SetName("derived/prin_strain_3");
        prin_strain3_expr.SetDefinition("principal_tensor(strain)[2]");
        prin_strain3_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&prin_strain3_expr);
    }
    CATCH(InvalidVariableException)
    {
    }
    ENDTRY

    TRY
    {
        // This call throw an exception if nodvel does not exist.
        GetVariableIndex("nodvel");

        Expression velx_expr;
        velx_expr.SetName("derived/velocity_x");
        velx_expr.SetDefinition("nodvel[0]");
        velx_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&velx_expr);

        Expression vely_expr;
        vely_expr.SetName("derived/velocity_y");
        vely_expr.SetDefinition("nodvel[1]");
        vely_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&vely_expr);

        Expression velz_expr;
        velz_expr.SetName("derived/velocity_z");
        velz_expr.SetDefinition("nodvel[2]");
        velz_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&velz_expr);

        Expression velmag_expr;
        velmag_expr.SetName("derived/velocity_mag");
        velmag_expr.SetDefinition("magnitude(nodvel)");
        velmag_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&velmag_expr);
    }
    CATCH(InvalidVariableException)
    {
    }
    ENDTRY

    TRY
    {
        // This call throw an exception if nodacc does not exist.
        GetVariableIndex("nodacc");

        Expression accx_expr;
        accx_expr.SetName("derived/acceleration_x");
        accx_expr.SetDefinition("nodacc[0]");
        accx_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&accx_expr);

        Expression accy_expr;
        accy_expr.SetName("derived/acceleration_y");
        accy_expr.SetDefinition("nodacc[1]");
        accy_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&accy_expr);

        Expression accz_expr;
        accz_expr.SetName("derived/acceleration_z");
        accz_expr.SetDefinition("nodacc[2]");
        accz_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&accz_expr);

        Expression accmag_expr;
        accmag_expr.SetName("derived/acceleration_mag");
        accmag_expr.SetDefinition("magnitude(nodacc)");
        accmag_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&accmag_expr);
    }
    CATCH(InvalidVariableException)
    {
    }
    ENDTRY

    if (!readPartInfo && !avtDatabase::OnlyServeUpMetaData())
        ParseDynaPart();
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
//  Modifications:
//
//    Hank Childs, Tue Jul 27 10:18:26 PDT 2004
//    Moved the code to free up resources to the destructor.
//
// ****************************************************************************

void
avtMiliFileFormat::FreeUpResources()
{
    // Do not free anything up, since we are a multi-timestep format and
    // we need the information for the next timestep...
}


// ****************************************************************************
//  Method: avtMiliFileFormat::ParseDynaPart
//
//  Purpose:
//    Read though a DynaPart output file to gather information about
//    shared nodes for generating ghostzones.
//
//  Programmer: Akira Haddox
//  Creation:   August 6, 2003
//
// ****************************************************************************

#define READ_THROUGH_NEXT_COMMENT(_in) while(_in.get() != '#') ;\
                                       _in.getline(buf, 1024)
#define READ_VECTOR(_in, _dest) \
    for (_macro_i = 0; _macro_i < _dest.size(); ++_macro_i) \
        _in >> _dest[_macro_i]

void
avtMiliFileFormat::ParseDynaPart()
{
    readPartInfo = true;
    int i;
    int _macro_i;

    string fname = fampath;
    fname += "/" + dynaPartFilename;

    ifstream in;
    in.open(fname.c_str());

    if (in.fail())
        EXCEPTION1(InvalidFilesException, fname.c_str());

    char buf[1024];

    // Read the header line
    in.getline(buf, 1024);
    
    // Skip through the version and initial comments
    do
    {
        in.getline(buf, 1024);
    }while (buf[0] == '#');

    // Get the number of discrete elements
    READ_THROUGH_NEXT_COMMENT(in);
    in.getline(buf, 1024);

    // Get the number of each type
    READ_THROUGH_NEXT_COMMENT(in);
    int nNodal;
    int nHexs;
    int nBeams;
    int nShells;
    int nThickShells;
    int nProc;
    in >> nNodal >> nHexs >> nBeams >> nShells >> nThickShells >> nProc;

    // Get nodes per processor
    vector<int> nNodalPerProc(nProc, 0);
    READ_THROUGH_NEXT_COMMENT(in);
    READ_VECTOR(in, nNodalPerProc);

    // Get hexs per processor
    vector<int> nHexsPerProc(nProc, 0);
    if (nHexs)
    {
        READ_THROUGH_NEXT_COMMENT(in);
        READ_VECTOR(in, nHexsPerProc);
    }

    // Get Beams per processor
    vector<int> nBeamsPerProc(nProc, 0);
    if (nBeams)
    {
        READ_THROUGH_NEXT_COMMENT(in);
        READ_VECTOR(in, nBeamsPerProc);
    }

    // Get shells per processor
    vector<int> nShellsPerProc(nProc, 0);
    if (nShells)
    {
        READ_THROUGH_NEXT_COMMENT(in);
        READ_VECTOR(in, nShellsPerProc);
    }
    
    // Get thick shells per processor
    vector<int> nThickShellsPerProc(nProc, 0);
    if (nThickShells)
    {
        READ_THROUGH_NEXT_COMMENT(in);
        READ_VECTOR(in, nThickShellsPerProc);
    }
    

    // Get number of shared nodes per processor
    vector<int> nSharedNodes(nProc, 0);
    READ_THROUGH_NEXT_COMMENT(in);
    READ_VECTOR(in, nSharedNodes);

    // Get the number of processors a processor shares with
    vector<int> nProcComm(nProc, 0);
    READ_THROUGH_NEXT_COMMENT(in);
    READ_VECTOR(in, nProcComm);

    // Get the node divisions per processor
    vector<vector<int> > nodesProcMap(nProc);
    for (i = 0; i < nProc; ++i)
    {
        nodesProcMap[i].resize(nNodalPerProc[i]);
        READ_THROUGH_NEXT_COMMENT(in);
        READ_VECTOR(in, nodesProcMap[i]);
    }

    //
    // Now we need to skip through all the sections that define what
    // the partitioning is.
    //
    int skippedSections = 0;
    for (i = 0; i < nProc; ++i)
    {
//        if (nNodalPerProc[i]) ++skippedSections;
        if (nHexsPerProc[i]) ++skippedSections;
        if (nBeamsPerProc[i]) ++skippedSections;
        if (nShellsPerProc[i]) ++skippedSections;
        if (nThickShellsPerProc[i]) ++skippedSections;
    }

    for (i = 0; i < skippedSections; ++i)
        READ_THROUGH_NEXT_COMMENT(in);
    // The next READ_THROUGH_NEXT_COMMENT queues us up to the right point

    vector<int> nAdjacentProc(nProc, 0);
    vector<vector<int> > adjacentProc(nProc);
    vector<vector<int> > nSharedNodesPerProc(nProc);

    // Indexed: [domain] shares with [domain]
    vector<vector<vector< int > > >     sharedNodes;

    sharedNodes.resize(nProc);
    for (i = 0; i < nProc; ++i)
    {
        sharedNodes[i].resize(nProc);

        // Get the adjacent processors
        READ_THROUGH_NEXT_COMMENT(in);
        int adp;
        for (;;)
        {
            in >> adp;
            if (in.fail())
                break;
            adjacentProc[i].push_back(adp);
        }
        in.clear();
        nAdjacentProc[i] = adjacentProc[i].size();

        // We may have stripped the '#' out of the comment
        // use a getline this time.
        in.getline(buf, 1024);
        
        // Read in how many shared nodes there are for each shared processor
        nSharedNodesPerProc[i].resize(nAdjacentProc[i]);
        READ_VECTOR(in, nSharedNodesPerProc[i]);

        int j;
        READ_THROUGH_NEXT_COMMENT(in);
        for (j = 0; j < nAdjacentProc[i]; ++j)
        {
            int index = adjacentProc[i][j];
            sharedNodes[i][index].resize(nSharedNodesPerProc[i][j]);
            READ_VECTOR(in, sharedNodes[i][index]);
        }
    }
    
    // Remapp the shared node ids to the ones we store
    vector<vector<vector<pair<int,int> > > > mappings(nProc);
    for (i = 0; i < nProc; ++i)
    {
        mappings[i].resize(nProc);
        int j;
        for (j = 0; j < nProc; ++j)
        {
            int iPtr = 0;
            int k;
            for (k = 0; k < sharedNodes[i][j].size(); ++k)
            {
                int relative = sharedNodes[i][j][k];
                // Proc i is sharing with proc j
                while (nodesProcMap[i][iPtr] != relative)
                    // Incriment iPtr. This check shouldn't be
                    // necessary really.
                    if (++iPtr > nodesProcMap[j].size())
                        break;

                int jPtr;
                for (jPtr = 0; nodesProcMap[j][jPtr] != relative; ++jPtr)
                    ;
                
                sharedNodes[i][j][k] = iPtr;
                mappings[i][j].push_back(pair<int, int>(iPtr, jPtr));
            }
        }
    }

    in.close();

    avtUnstructuredPointBoundaries *upb = new avtUnstructuredPointBoundaries;
    
    for (i = 0; i < ndomains; ++i)
    {
        int j;
        for (j = 0; j < ndomains; ++j)
        {
            if (i == j)
                continue;

            if (sharedNodes[i][j].size() == 0)
                continue;

            if (i < j)
            {
                vector<int> d1pts;
                vector<int> d2pts;
                
                for (int k = 0; k < mappings[i][j].size(); ++k)
                {
                    d1pts.push_back(mappings[i][j][k].first);
                    d2pts.push_back(mappings[i][j][k].second);
                }
                
                upb->SetSharedPoints(i, j, d1pts, d2pts);
            }
        }
    }

    upb->SetTotalNumberOfDomains(ndomains);

    void_ref_ptr vr = void_ref_ptr(upb, 
                                   avtUnstructuredPointBoundaries::Destruct); 

    cache->CacheVoidRef("any_mesh", AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION,
                        -1, -1, vr);
}
