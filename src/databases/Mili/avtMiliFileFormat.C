/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                              avtMiliFileFormat.C                          //
// ************************************************************************* //

#include <avtMiliFileFormat.h>

#include <vector>
#include <string>
using std::getline;
#include <snprintf.h>
#include <visitstream.h>
#include <set>

extern "C" {
#include <mili_enum.h>
}

#include <vtkCellData.h>
#include <vtkCellTypes.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkFloatArray.h>

#include <Expression.h>

#include <avtCallback.h>
#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtGhostData.h>
#include <avtMaterial.h>
#include <avtVariableCache.h>
#include <avtUnstructuredPointBoundaries.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>
#include <UnexpectedValueException.h>

using std::vector;
using std::string;
using std::ifstream;

static const char *free_nodes_str = "free_nodes";
static const char *no_free_nodes_str = "no_free_nodes";
static const int free_nodes_strlen = strlen(free_nodes_str);
static const int no_free_nodes_strlen = strlen(no_free_nodes_str);

#define Warn(msg)          IssueWarning(msg, __LINE__)

// ****************************************************************************
//  Method:  avtMiliFileFormat::IssueWarning
//
//  Purpose: Convenience method to issue warning messages. Manages number of
//      times a given warning message will be output
//
//  Programmer:  Mark C. Miller 
//  Creation:    January 4, 2005 
//
// ****************************************************************************
void
avtMiliFileFormat::IssueWarning(const char *msg, int key)
{
    if (warn_map.find(key) == warn_map.end())
        warn_map[key] = 1;
    else
        warn_map[key]++;

    if (warn_map[key] <= 5)
    {
        if (!avtCallback::IssueWarning(msg))
            cerr << msg << endl;
    }

    if (warn_map[key] == 5)
    {
        const char *smsg = "\n\nFurther warnings will be suppresed";
        if (!avtCallback::IssueWarning(smsg))
            cerr << smsg << endl;
    }
}

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
//    Mark C. Miller, Mon Jul 18 13:41:13 PDT 2005
//    Added initialization of structures having to do with free nodes mesh
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

    free_nodes = 0;
    num_free_nodes = 0;
    free_nodes_ts = -1;
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
//    Mark C. Miller, Mon Jul 18 13:41:13 PDT 2005
//    Free structures having to do with free nodes mesh
//
//    Mark C. Miller, Wed Mar  8 08:40:55 PST 2006
//    Added code to cleanse Mili subrecords
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
    for (i = 0; i < sub_records.size(); ++i)
        for (j = 0; j < sub_records[i].size(); ++j)
            mc_cleanse_subrec(&sub_records[i][j]);

    //
    // Reset flags to indicate the meshes needs to be read in again.
    //
    for (i = 0; i < ndomains; ++i)
        readMesh[i] = false;

    delete [] famroot;
    if (fampath)
        delete [] fampath;

    if (free_nodes)
    {
        delete [] free_nodes;
        free_nodes = 0;
        num_free_nodes = 0;
        free_nodes_ts = -1;
    }
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
//  Modifications:
//
//    Mark C. Miller, Mon Jul 18 13:41:13 PDT 2005
//    Added logic to read "param arrays" via a different Mili API call. Note
//    that param arrays are always alloc'd by Mili
// ****************************************************************************

static void
read_results(Famid &dbid, int ts, int sr, int rank,
                      char **name, int vtype, int amount, float *buff)
{
    int  i;

    bool isParamArray = strncmp(*name, "params/", 7) == 0;

    void *buff_to_read_into = NULL;
    if (!isParamArray)
    {
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
    }

    int rval;
    if (isParamArray)
    {
        char tmpName[256];
        strcpy(tmpName, &(*name)[7]);
        rval = mc_read_param_array(dbid, tmpName, &buff_to_read_into);
        if (rval == OK && (vtype == M_FLOAT || vtype == M_FLOAT4))
        {
            float *pflt = (float *) buff_to_read_into;
            for (i = 0 ; i < amount ; i++)
                buff[i] = (float)(pflt[i]);
        }
    }
    else
        rval = mc_read_results(dbid, ts, sr, rank, name, buff_to_read_into);

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

    if (isParamArray)
        free(buff_to_read_into);
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
//    Hank Childs, Fri Aug 27 17:12:50 PDT 2004
//    Rename ghost data array.
//
//    Mark C. Miller, Mon Jul 18 13:41:13 PDT 2005
//    Added logic to read the "free nodes" mesh, too. Removed huge block of
//    unused #ifdef'd code having to do with ghost zones.
//
//    Mark C. Miller, Tue Jan  3 17:55:22 PST 2006
//    Added code to deal with case where nodal positions are time invariant.
//    They are not stored as "results" but instead part of the mesh read
//    in the ReadMesh() call.
//
//    Mark C. Miller, Wed Nov 15 01:46:16 PST 2006
//    Added a "no_free_nodes" mesh by ghost labeling sanded nodes. Added
//    the logic to label sanded nodes here.
//
//    Mark C. Miller, Tue Nov 21 10:16:42 PST 2006
//    Fixed leak of sand_arr. Made it request sand_arr only if the
//    no_free_nodes mesh was requested
// ****************************************************************************

vtkDataSet *
avtMiliFileFormat::GetMesh(int ts, int dom, const char *mesh)
{
    int i;

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
    char *check = 0;
    mesh_id = (int) strtol(mesh + 4, &check, 10);
    if (mesh_id == 0 || check == mesh + 4)
    {
        EXCEPTION1(InvalidVariableException, mesh)
    }
    --mesh_id;

    //
    // The connectivity does not change over time, so use the one we have
    // already calculated.
    //
    vtkUnstructuredGrid *rv = vtkUnstructuredGrid::New();
    rv->ShallowCopy(connectivity[dom][mesh_id]);

    //
    // The node positions are stored in 'nodpos'.
    //
    char *nodpos_str = "nodpos";
    int nodpos = -2;

    // Since this whole plugin assumes GetVariableIndex
    // handles throwing of invalid variable exception
    // we have to wrap this with TRY/CATCH to deal with
    // case where nodal positions are stored in params
    TRY
    {
        nodpos = GetVariableIndex(nodpos_str, mesh_id);
    }
    CATCH(InvalidVariableException)
    {
        nodpos = -1;
    }
    ENDTRY

    int subrec = -1;
    int vsize = M_FLOAT;
    for (i = 0 ; nodpos >= 0 && i < vars_valid[dom][nodpos].size() ; i++)
    {
        if (vars_valid[dom][nodpos][i])
        {
            subrec = sub_record_ids[dom][i];
            vsize = var_size[dom][nodpos][i];
            break;
        }
    }

    int amt = dims*nnodes[dom][mesh_id];
    float *fpts = 0;
    if (nodpos != -1)
    {
        if (subrec == -1)
        {
            if (rv->GetPoints() == 0)
            {
                char msg[1024];
                SNPRINTF(msg, sizeof(msg),
                    "Unable to find coords for domain %d. Skipping it", dom);
                Warn(msg);

                // null out the returned grid
                rv->Delete();
                rv = vtkUnstructuredGrid::New();
                return rv;
            }
        }
        else
        {
            fpts = new float[amt];
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

            //
            // Ghost out nodes that belong to zones that are "sanded"
            // Start by assuming all nodes are N/A and then remove the
            // N/A ghost node type for all those nodes that belong to
            // zones that are NOT sanded.
            //
            if (strstr(mesh, no_free_nodes_str))
            {
                vtkFloatArray *sand_arr = (vtkFloatArray *) GetVar(ts, dom, "sand");
                if (sand_arr)
                {
                    float *sand_vals = (float*) sand_arr->GetVoidPointer(0);

                    vtkUnsignedCharArray *ghost_nodes = vtkUnsignedCharArray::New();
                    ghost_nodes->SetName("avtGhostNodes");
                    ghost_nodes->SetNumberOfTuples(nnodes[dom][mesh_id]);
                    unsigned char *gnp = ghost_nodes->GetPointer(0);
                    for (i = 0 ; i < nnodes[dom][mesh_id]; i++)
                    {
                        gnp[i] = 0;
                        avtGhostData::AddGhostNodeType(gnp[i],
                            NODE_NOT_APPLICABLE_TO_PROBLEM);
                    }
                    for (int cell = 0; cell < ncells[dom][mesh_id]; cell++)
                    {
                        if (sand_vals[cell] > 0.5) // element status is "good"
                        {
                            vtkIdType npts = 0, *pts = 0;
                            rv->GetCellPoints(cell, npts, pts);
                            if (npts && pts)
                            {
                                for (int node = 0; node < npts; node++)
                                    avtGhostData::RemoveGhostNodeType(gnp[pts[node]],
                                        NODE_NOT_APPLICABLE_TO_PROBLEM);
                            }
                        }
                    }
                    sand_arr->Delete();
                    rv->GetPointData()->AddArray(ghost_nodes);
                    ghost_nodes->Delete();
                }
            }
        }
    }
    else
    {
        //
        // We can arrive here if there are no nodal positions results
        // but we have initial mesh positions from reading the mesh
        // header information (mc_load_nodes).
        //
        if (rv->GetPoints() == 0)
        {
            char msg[1024];
            SNPRINTF(msg, sizeof(msg),
                "Unable to find coords for domain %d. Skipping it", dom);
            Warn(msg);

            // null out the returned grid
            rv->Delete();
            rv = vtkUnstructuredGrid::New();
            return rv;
        }
    }

    //
    // If VisIt really asked for the free nodes mesh, compute that now,
    // otherwise, just return the mesh
    //
    if (strstr(mesh, no_free_nodes_str) ||
       !strstr(mesh, free_nodes_str))
    {
        if (fpts) delete [] fpts;
        return rv;
    }

    //
    // Element status' are stored in the "sand" variable 
    //
    vtkFloatArray *sand_arr = (vtkFloatArray *) GetVar(ts, dom, "sand");
    if (sand_arr->GetNumberOfTuples() != ncells[dom][mesh_id])
    {
        EXCEPTION2(UnexpectedValueException, sand_arr->GetNumberOfTuples(),
                                             ncells[dom][mesh_id]);
    }
    float *sand_vals = (float*) sand_arr->GetVoidPointer(0);

    const unsigned char MESH = 'm';
    const unsigned char FREE = 'f';
    unsigned char *ns = new unsigned char[nnodes[dom][mesh_id]];
    memset(ns, FREE, nnodes[dom][mesh_id]);

    //
    // Populate nodal status array based on element status'
    //
    int cell, node;
    int num_free = nnodes[dom][mesh_id];
    for (cell = 0; cell < ncells[dom][mesh_id]; cell++)
    {
        if (sand_vals[cell] > 0.5) // element status is "good"
        {
            vtkIdType npts = 0, *pts = 0;
            rv->GetCellPoints(cell, npts, pts);
            if (npts && pts)
            {
                for (node = 0; node < npts; node++)
                {
                    int nid = pts[node];
                    if (ns[nid] != MESH)
                    {
                        ns[nid] = MESH;
                        num_free--;
                    }
                }
            }
        }
    }
    sand_arr->Delete();


    vtkPoints *freepts = vtkPoints::New();
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    if (num_free > 0)
    {
        //
        // cache the list of nodes that are the free nodes
        //
        if (free_nodes)
            delete [] free_nodes;
        free_nodes = new int[num_free];
        num_free_nodes = num_free;
        free_nodes_ts = ts;

        freepts->SetNumberOfPoints(num_free);
        float *fptr_dst = (float *) freepts->GetVoidPointer(0);
        float *fptr_src = fpts; 
        int fnode = 0;
        for (node = 0; node < nnodes[dom][mesh_id]; node++)
        {
            if (ns[node] == FREE)
            {
                free_nodes[fnode/3] = node;
                fptr_dst[fnode++] = fptr_src[3*node+0];
                fptr_dst[fnode++] = fptr_src[3*node+1];
                fptr_dst[fnode++] = fptr_src[3*node+2];
            }
        }

        ugrid->SetPoints(freepts);
        ugrid->Allocate(num_free);
        vtkIdType onevertex[1];
        for (node = 0; node < num_free; node++)
        {
            onevertex[0] = node;
            ugrid->InsertNextCell(VTK_VERTEX, 1, onevertex);
        }
        freepts->Delete();
    }
    else
    {
        if (free_nodes)
            delete [] free_nodes;
        free_nodes = 0; 
        num_free_nodes = 0;
        free_nodes_ts = ts;
        freepts->SetNumberOfPoints(0);
        ugrid->SetPoints(freepts);
        freepts->Delete();
    }
    delete [] ns;
    delete [] fpts;
    rv->Delete();

    return ugrid;
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
//  Modifications:
//
//    Mark C. Miller, Mon Jul 18 13:41:13 PDT 2005
//    Added logic to deal with free nodes mesh variables
//
//    Mark C. Miller, Wed Nov 15 01:46:16 PST 2006
//    Changed names of free_node variables from 'xxx_free_nodes' to
//    'free_nodes/xxx' to put them in a submenu in GUI.
// ****************************************************************************

int
avtMiliFileFormat::GetVariableIndex(const char *varname)
{
    string tmpname = varname;
    char *p = strstr(varname, free_nodes_str);

    if (p)
        tmpname = string(varname, free_nodes_strlen+1,
                                  strlen(varname) - (free_nodes_strlen+1));

    for (int i = 0 ; i < vars.size() ; i++)
    {
        if (vars[i] == tmpname)
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
//  Modifications:
//
//    Mark C. Miller, Mon Jul 18 13:41:13 PDT 2005
//    Added logic to deal with free nodes mesh variables
//
//    Mark C. Miller, Wed Nov 15 01:46:16 PST 2006
//    Changed names of free_node variables from 'xxx_free_nodes' to
//    'free_nodes/xxx' to put them in a submenu in GUI.
// ****************************************************************************

int
avtMiliFileFormat::GetVariableIndex(const char *varname, int mesh_id)
{
    string tmpname = varname;
    char *p = strstr(varname, free_nodes_str);

    if (p)
        tmpname = string(varname, free_nodes_strlen+1,
                                  strlen(varname) - (free_nodes_strlen+1));

    for (int i = 0 ; i < vars.size() ; i++)
    {
        if (vars[i] == tmpname && var_mesh_associations[i] == mesh_id)
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
//    Hank Childs, Wed Aug 18 16:17:52 PDT 2004
//    Add some special handling for single domain families.
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
            {
                // Try putting in the domain number and see what happens...
                // We need this because makemili accepts it and there are
                // legacy .mili files that look like fam rather than fam000.
                char rootname[255];
                sprintf(rootname, "%s%.3d", famroot, dom);
                rval = mc_open(rootname, fampath, "r", &(dbid[dom]) );
            }
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
//    Mark C. Miller, Tue Jan  3 17:55:22 PST 2006
//    Added code to get initial nodal positions with mc_load_nodes()
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
        // Read initial nodal position information, if available
        //
        vtkPoints *pts = vtkPoints::New();
        pts->SetNumberOfPoints(nnodes[dom][mesh_id]);
        float *vpts = (float *) pts->GetVoidPointer(0);
        if (mc_load_nodes(dbid[dom], mesh_id, short_name, vpts) == 0)
        {
            //
            // We need to insert zeros if we're in 2D
            //
            if (dims == 2)
            {
                for (int p = nnodes[dom][mesh_id]-1; p >= 0; p--)
                {
                    int q = p*3, r = p*2;
                    vpts[q+0] = vpts[r+0];
                    vpts[q+1] = vpts[r+1];
                    vpts[q+2] = 0.0;
                }
            }
        }
        else
        {
            pts->Delete();
            pts = NULL;
        }

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

        //
        // Hook up points to mesh if we have 'em
        //
        if (pts)
        {
            connectivity[dom][mesh_id]->SetPoints(pts);
            pts->Delete();
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
//    Kathleen Bonnell, Wed Jul  6 14:27:42 PDT 2005 
//    Initialize sv with memset to remove free of invalid pointer when
//    mc_cleanse_st_variable is called.
//
//    Mark C. Miller, Mon Jul 18 13:41:13 PDT 2005
//    Added code to deal with param-array variables
//    Added memset call to zero-out Subrecord struct
//
//    Mark C. Miller, Mon Mar  6 14:25:49 PST 2006
//    Added call to cleanse subrec at end of loop to fix a memory leak
//
//    Mark C. Miller, Wed Mar  8 08:40:55 PST 2006
//    Moved code to cleanse subrec to destructor
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
            memset(&sr, 0, sizeof(sr));
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
                bool pushVal = strncmp(vars[vv].c_str(), "params/", 7) == 0;
                vars_valid[dom][vv].push_back(pushVal);
                var_size[dom][vv].push_back(M_FLOAT);
            }
            int index = sub_records[dom].size() - 1;
           
            for (int k = 0 ; k < sr.qty_svars ; k++)
            {
                 State_variable sv;
                 memset(&sv, 0, sizeof(sv));
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

                 mc_cleanse_st_variable(&sv);
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
//    Hank Childs, Fri Aug 20 15:31:30 PDT 2004
//    Increment the material number here to match what the meta-data says.
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
        sprintf(str, "mat%d", i+1);
        mat_names[i] = str;
    }

    avtMaterial * mat = new avtMaterial(nmaterials[meshId], mat_names, size, 
                                        mlist, 0, NULL, NULL, NULL, NULL);

    delete []mlist;
    return mat;
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::RestrictVarToFreeNodes
//
//  Purpose: Restrict a given variable to the free nodes mesh
//
//  Programmer:  Mark C. Miller
//  Creation:    July 18, 2005
//
// ****************************************************************************
vtkFloatArray *
avtMiliFileFormat::RestrictVarToFreeNodes(vtkFloatArray *src, int ts) const
{
    if (free_nodes_ts != ts)
    {
        EXCEPTION2(UnexpectedValueException, ts, free_nodes_ts);
    }

    int ncomps = src->GetNumberOfComponents();
    vtkFloatArray *dst = vtkFloatArray::New();
    dst->SetNumberOfComponents(ncomps);
    dst->SetNumberOfTuples(num_free_nodes);
    float *dstp = (float *) dst->GetVoidPointer(0);
    float *srcp = (float *) src->GetVoidPointer(0);
    for (int i = 0; i < num_free_nodes; i++)
        for (int j = 0; j < ncomps; j++)
            dstp[i*ncomps+j] = srcp[free_nodes[i]*ncomps+j];
    return dst;
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
//    Mark C. Miller, Mon Jul 18 13:41:13 PDT 2005
//    Added code to deal with param array variables
//    Added code to deal with variables defined on the free nodes mesh
//
//    Mark C. Miller, Wed Nov 15 01:46:16 PST 2006
//    Added "no_free_nodes" variants of variables. Changed names of
//    free_node variables from 'xxx_free_nodes' to 'free_nodes/xxx'
// ****************************************************************************

vtkDataArray *
avtMiliFileFormat::GetVar(int ts, int dom, const char *name)
{
    if (!readMesh[dom])
        ReadMesh(dom);
    if (!validateVars[dom])
        ValidateVariables(dom);

    bool isParamArray = strncmp(name, "params/", 7) == 0; 
    string usename = name;
    bool isFreeNodesVar = false;
    if (strstr(name, no_free_nodes_str))
    {
        usename = string(name, no_free_nodes_strlen+1,
                               strlen(name) - (no_free_nodes_strlen+1));
        isFreeNodesVar = false;
    }
    else if (strstr(name, free_nodes_str))
    {
        usename = string(name, free_nodes_strlen+1,
                               strlen(name) - (free_nodes_strlen+1));
        isFreeNodesVar = true;
    }

    string vname;
    int meshid = 0;
    if (nmeshes == 1)
        vname = usename;
    else
        DecodeMultiMeshVarname(usename, vname, meshid);
    
    int v_index = GetVariableIndex(vname.c_str(), meshid);
    int mesh_id = var_mesh_associations[v_index];

    vtkFloatArray *rv = 0;

    if (centering[v_index] == AVT_NODECENT)
    {
        int i;
        int nvars = 0;
        int sr_valid = -1;
        for (i = 0 ; i < vars_valid[dom][v_index].size() ; i++)
        {
            if (vars_valid[dom][v_index][i])
            {
                sr_valid = i;
                nvars++;
            }
        }
        if (!isParamArray && nvars != 1)
        {
            EXCEPTION1(InvalidVariableException, name);
        }
        int vsize = var_size[dom][v_index][sr_valid];

        // Since data in param arrays is constant over all time,
        // we just cache it here in the plugin. Lets look in the
        // cache *before* we try to read it (again).
        if (isParamArray)
        {
            rv = (vtkFloatArray*) cache->GetVTKObject(usename.c_str(),
                     avtVariableCache::SCALARS_NAME, -1, dom, "none");
        }

        if (rv == 0)
        {
            int amt = nnodes[dom][mesh_id];
            rv = vtkFloatArray::New();
            rv->SetNumberOfTuples(amt);
            float *p = (float *) rv->GetVoidPointer(0);
            char *tmp = (char *) usename.c_str();  // Bypass const
            read_results(dbid[dom], ts+1, sub_record_ids[dom][sr_valid], 1,
                            &tmp, vsize, amt, p);

            //
            // We explicitly cache param arrays at ts=-1
            //
            if (isParamArray)
            {
                cache->CacheVTKObject(usename.c_str(), avtVariableCache::SCALARS_NAME,
                                  -1, dom, "none", rv);
            }
        }
        else
        {
            // The reference count will be decremented by the generic database,
            // because it will assume it owns it.
            rv->Register(NULL);
        }

        //
        // Restrict variables on free nodes to the free nodes mesh
        //
        if (isFreeNodesVar)
        {
            vtkFloatArray *newrv = RestrictVarToFreeNodes(rv, ts);
            rv->Delete();
            rv = newrv;
        }
    }
    else
    {
        rv = vtkFloatArray::New();
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

                char *tmp = (char *) usename.c_str();  // Bypass const
                
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
//    Mark C. Miller, Mon Jul 18 13:41:13 PDT 2005
//    Added code to deal with variables defined on the free nodes mesh
//
//    Mark C. Miller, Wed Nov 15 01:46:16 PST 2006
//    Added "no_free_nodes" variants of variables. Changed names of
//    free_node variables from 'xxx_free_nodes' to 'free_nodes/xxx'
// ****************************************************************************

vtkDataArray *
avtMiliFileFormat::GetVectorVar(int ts, int dom, const char *name)
{
    if (!readMesh[dom])
        ReadMesh(dom);
    if (!validateVars[dom])
        ValidateVariables(dom);

    string usename = name;
    bool isFreeNodesVar = false;
    if (strstr(name, no_free_nodes_str))
    {
        usename = string(name, no_free_nodes_strlen+1,
                               strlen(name) - (no_free_nodes_strlen+1));
        isFreeNodesVar = false;
    }
    else if (strstr(name, free_nodes_str))
    {
        usename = string(name, free_nodes_strlen+1,
                               strlen(name) - (free_nodes_strlen+1));
        isFreeNodesVar = true;
    }

    string vname;
    int meshid = 0;
    if (nmeshes == 1)
        vname = usename;
    else
        DecodeMultiMeshVarname(usename, vname, meshid);
    
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
        char *tmp = (char *) usename.c_str();  // Bypass const
        read_results(dbid[dom], ts+1, sub_record_ids[dom][sr_valid], 1,
                        &tmp, vsize, amt*vdim, ptr);

        //
        // Restrict variables on free nodes to the free nodes mesh
        //
        if (isFreeNodesVar)
        {
            vtkFloatArray *newrv = RestrictVarToFreeNodes(rv, ts);
            rv->Delete();
            rv = newrv;
        }
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

                char *tmp = (char *) usename.c_str();  // Bypass const

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
            double orig_vals[6];
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
//    timeState  The time index to use (if metadata varies with time)
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
//    Hank Childs, Wed Aug 18 16:25:15 PDT 2004
//    Added new expressions for displacement and position.
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added timeState arg to satisfy new interface
//
//    Mark C. Miller, Mon Jul 18 13:41:13 PDT 2005
//    Added code to add free nodes mesh and variables
//
//    Mark C. Miller, Wed Nov 15 01:46:16 PST 2006
//    Added "no_free_nodes" variants of meshes, material and variables.
//    Changed names of free_node variables from 'xxx_free_nodes' to
//    'free_nodes/xxx'. Populated any node-centered expressions for both
//    original and free_node variants and zone-centered expressions for
//    both original and no_free_node variants. Changed cellOrigin to 1
//    to address off-by-one errors during pick. Bob Corey says that so far,
//    all clients that write mili data are Fortran clients. They expect to
//    get node/zone numbers from pick starting from '1'. 
//
//    Mark C. Miller, Wed Nov 29 12:08:49 PST 2006
//    Suppress creation of "no_free_nodes" flavors of expressions when
//    not needed
//    
//    Thomas R. Treadway, Tue Dec  5 15:14:11 PST 2006
//    Added a derived strain and displacement algorithms
//
// ****************************************************************************

void
avtMiliFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md,
    int timeState)
{
    int i;

    bool has_any_fn_mesh = false;
    vector<bool> has_fn_mesh;
    for (i = 0; i < nmeshes; ++i)
    {
        char meshname[32];
        char matname[32];
        sprintf(meshname, "mesh%d", i + 1);
        sprintf(matname, "materials%d", i + 1);
        const string fnmeshname = string(meshname) + "_" + string(free_nodes_str);
        const string nofnmeshname = string(meshname) + "_" + string(no_free_nodes_str);
        const string nofnmatname = string(matname) + "_" + string(no_free_nodes_str);
        avtMeshMetaData *mesh = new avtMeshMetaData;
        mesh->name = meshname;
        mesh->meshType = AVT_UNSTRUCTURED_MESH;
        mesh->numBlocks = ndomains;
        mesh->blockOrigin = 0;
        mesh->cellOrigin = 1; // Bob Corey says all mili writers so far are Fortran
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
        AddMaterialToMetaData(md, nofnmatname, nofnmeshname, nmaterials[i], mnames);

        //
        // Add the free-nodes and no-free-nodes meshes
        // if variable "sand" is defined on this mesh
        //
        has_fn_mesh.push_back(false);
        for (j = 0 ; j < vars.size() ; j++)
        {
            if (vars[j] == "sand" && var_mesh_associations[j] == i)
            {
                avtMeshMetaData *fnmesh = new avtMeshMetaData;
                fnmesh->name = fnmeshname; 
                fnmesh->meshType = AVT_POINT_MESH;
                fnmesh->numBlocks = ndomains;
                fnmesh->blockOrigin = 0;
                fnmesh->cellOrigin = 1; // All mili writers so far are Fortran
                fnmesh->spatialDimension = dims;
                fnmesh->topologicalDimension = 0;
                fnmesh->blockTitle = "processors";
                fnmesh->blockPieceName = "processor";
                fnmesh->hasSpatialExtents = false;
                md->Add(fnmesh);

                avtMeshMetaData *nofnmesh = new avtMeshMetaData;
                nofnmesh->name = nofnmeshname; 
                nofnmesh->meshType = AVT_UNSTRUCTURED_MESH;
                nofnmesh->numBlocks = ndomains;
                nofnmesh->blockOrigin = 0;
                nofnmesh->cellOrigin = 1; // All mili writers so far are Fortran
                nofnmesh->spatialDimension = dims;
                nofnmesh->topologicalDimension = dims;
                nofnmesh->blockTitle = "processors";
                nofnmesh->blockPieceName = "processor";
                nofnmesh->hasSpatialExtents = false;
                md->Add(nofnmesh);

                has_fn_mesh[i] = true;
                has_any_fn_mesh = true;
            }
        }
    }

    for (i = 0 ; i < vars.size() ; i++)
    {
        char meshname[32];
        sprintf(meshname, "mesh%d", var_mesh_associations[i] + 1);
        const string fnmeshname = string(meshname) + "_" + string(free_nodes_str);
        const string nofnmeshname = string(meshname) + "_" + string(no_free_nodes_str);

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

        bool do_fn_mesh_too = has_fn_mesh[var_mesh_associations[i]] && 
                              (centering[i] == AVT_NODECENT);
        bool do_nofn_mesh_too = has_fn_mesh[var_mesh_associations[i]];
        string fnvname = string(free_nodes_str) + "/" + *vname;
        string nofnvname = string(no_free_nodes_str) + "/" + *vname;
        
        switch (vartype[i])
        {
          case AVT_SCALAR_VAR:
            AddScalarVarToMetaData(md, *vname, meshname, centering[i]);
            if (do_fn_mesh_too)
                AddScalarVarToMetaData(md, fnvname, fnmeshname.c_str(), centering[i]);
            if (do_nofn_mesh_too)
                AddScalarVarToMetaData(md, nofnvname, nofnmeshname.c_str(), centering[i]);
            break;
          case AVT_VECTOR_VAR:
            AddVectorVarToMetaData(md, *vname, meshname, centering[i], dims);
            if (do_fn_mesh_too)
                AddVectorVarToMetaData(md, fnvname, fnmeshname.c_str(), centering[i], dims);
            if (do_nofn_mesh_too)
                AddVectorVarToMetaData(md, nofnvname, nofnmeshname.c_str(), centering[i], dims);
            break;
          case AVT_SYMMETRIC_TENSOR_VAR:
            AddSymmetricTensorVarToMetaData(md, *vname, meshname, centering[i],
                                            dims);
            if (do_fn_mesh_too)
                AddSymmetricTensorVarToMetaData(md, fnvname, fnmeshname.c_str(),
                                                centering[i], dims);
            if (do_nofn_mesh_too)
                AddSymmetricTensorVarToMetaData(md, nofnvname, nofnmeshname.c_str(),
                                                centering[i], dims);
            break;
          case AVT_TENSOR_VAR:
            AddTensorVarToMetaData(md, *vname, meshname, centering[i], dims);
            if (do_fn_mesh_too)
                AddTensorVarToMetaData(md, fnvname, fnmeshname.c_str(), centering[i], dims);
            if (do_nofn_mesh_too)
                AddTensorVarToMetaData(md, nofnvname, nofnmeshname.c_str(), centering[i], dims);
            break;
          default:
            break;
        }
    }

    //
    // By calling OpenDB for domain 0, it will populate the times.
    //
    OpenDB(0);

    vector<string> dirs;
    dirs.push_back("");
    if (has_any_fn_mesh)
        dirs.push_back(string(no_free_nodes_str) + "/");

    vector<string> ndirs;
    ndirs.push_back("");
    if (has_any_fn_mesh)
        ndirs.push_back(string(free_nodes_str) + "/");

    vector<string> nsuff; 
    nsuff.push_back("");
    if (has_any_fn_mesh)
        nsuff.push_back("_" + string(free_nodes_str));

    TRY
    {
        // This call throw an exception if stress does not exist.
        GetVariableIndex("stress");

        for (i = 0; i < dirs.size(); i++)
        {
            Expression pressure_expr;
            pressure_expr.SetName("derived/"+dirs[i]+"pressure");
            pressure_expr.SetDefinition("-trace(<"+dirs[i]+"stress>)/3");
            pressure_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&pressure_expr);

            Expression stressx_expr;
            stressx_expr.SetName("derived/"+dirs[i]+"stress/x");
            stressx_expr.SetDefinition("<"+dirs[i]+"stress>[0][0]");
            stressx_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&stressx_expr);

            Expression stressy_expr;
            stressy_expr.SetName("derived/"+dirs[i]+"stress/y");
            stressy_expr.SetDefinition("<"+dirs[i]+"stress>[1][1]");
            stressy_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&stressy_expr);

            Expression stressz_expr;
            stressz_expr.SetName("derived/"+dirs[i]+"stress/z");
            stressz_expr.SetDefinition("<"+dirs[i]+"stress>[2][2]");
            stressz_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&stressz_expr);

            Expression stressxy_expr;
            stressxy_expr.SetName("derived/"+dirs[i]+"stress/xy");
            stressxy_expr.SetDefinition("<"+dirs[i]+"stress>[0][1]");
            stressxy_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&stressxy_expr);

            Expression stressxz_expr;
            stressxz_expr.SetName("derived/"+dirs[i]+"stress/xz");
            stressxz_expr.SetDefinition("<"+dirs[i]+"stress>[0][2]");
            stressxz_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&stressxz_expr);

            Expression stressyz_expr;
            stressyz_expr.SetName("derived/"+dirs[i]+"stress/yz");
            stressyz_expr.SetDefinition("<"+dirs[i]+"stress>[1][2]");
            stressyz_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&stressyz_expr);

            Expression seff_expr;
            seff_expr.SetName("derived/"+dirs[i]+"eff_stress");
            seff_expr.SetDefinition("effective_tensor(<"+dirs[i]+"stress>)");
            seff_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&seff_expr);

            Expression p_dev_stress1_expr;
            p_dev_stress1_expr.SetName("derived/"+dirs[i]+"prin_dev_stress/1");
            p_dev_stress1_expr.SetDefinition
                ("principal_deviatoric_tensor(<"+dirs[i]+"stress>)[0]");
            p_dev_stress1_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&p_dev_stress1_expr);

            Expression p_dev_stress2_expr;
            p_dev_stress2_expr.SetName("derived/"+dirs[i]+"prin_dev_stress/2");
            p_dev_stress2_expr.SetDefinition
                ("principal_deviatoric_tensor(<"+dirs[i]+"stress>)[1]");
            p_dev_stress2_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&p_dev_stress2_expr);

            Expression p_dev_stress3_expr;
            p_dev_stress3_expr.SetName("derived/"+dirs[i]+"prin_dev_stress/3");
            p_dev_stress3_expr.SetDefinition
                ("principal_deviatoric_tensor(<"+dirs[i]+"stress>)[2]");
            p_dev_stress3_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&p_dev_stress3_expr);

            Expression maxshr_expr;
            maxshr_expr.SetName("derived/"+dirs[i]+"max_shear_stress");
            maxshr_expr.SetDefinition
                ("tensor_maximum_shear(<"+dirs[i]+"stress>)");
            maxshr_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&maxshr_expr);

            Expression prin_stress1_expr;
            prin_stress1_expr.SetName("derived/"+dirs[i]+"prin_stress/1");
            prin_stress1_expr.SetDefinition
                ("principal_tensor(<"+dirs[i]+"stress>)[0]");
            prin_stress1_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&prin_stress1_expr);

            Expression prin_stress2_expr;
            prin_stress2_expr.SetName("derived/"+dirs[i]+"prin_stress/2");
            prin_stress2_expr.SetDefinition
                ("principal_tensor(<"+dirs[i]+"stress>)[1]");
            prin_stress2_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&prin_stress2_expr);

            Expression prin_stress3_expr;
            prin_stress3_expr.SetName("derived/"+dirs[i]+"prin_stress/3");
            prin_stress3_expr.SetDefinition
                ("principal_tensor(<"+dirs[i]+"stress>)[2]");
            prin_stress3_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&prin_stress3_expr);
        }
    }
    CATCH(InvalidVariableException)
    {
    }
    ENDTRY

    TRY
    {
        // This call throw an exception if strain does not exist.
        GetVariableIndex("strain");

        for (i = 0; i < dirs.size(); i++)
        {
            Expression strainx_expr;
            strainx_expr.SetName("derived/"+dirs[i]+"strain/x");
            strainx_expr.SetDefinition("<"+dirs[i]+"strain>[0][0]");
            strainx_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&strainx_expr);

            Expression strainy_expr;
            strainy_expr.SetName("derived/"+dirs[i]+"strain/y");
            strainy_expr.SetDefinition("<"+dirs[i]+"strain>[1][1]");
            strainy_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&strainy_expr);

            Expression strainz_expr;
            strainz_expr.SetName("derived/"+dirs[i]+"strain/z");
            strainz_expr.SetDefinition("<"+dirs[i]+"strain>[2][2]");
            strainz_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&strainz_expr);

            Expression strainxy_expr;
            strainxy_expr.SetName("derived/"+dirs[i]+"strain/xy");
            strainxy_expr.SetDefinition("<"+dirs[i]+"strain>[0][1]");
            strainxy_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&strainxy_expr);

            Expression strainxz_expr;
            strainxz_expr.SetName("derived/"+dirs[i]+"strain/xz");
            strainxz_expr.SetDefinition("<"+dirs[i]+"strain>[0][2]");
            strainxz_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&strainxz_expr);

            Expression strainyz_expr;
            strainyz_expr.SetName("derived/"+dirs[i]+"strain/yz");
            strainyz_expr.SetDefinition("<"+dirs[i]+"strain>[1][2]");
            strainyz_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&strainyz_expr);

            Expression seff_expr;
            seff_expr.SetName("derived/"+dirs[i]+"eff_strain");
            seff_expr.SetDefinition("effective_tensor(<"+dirs[i]+"strain>)");
            seff_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&seff_expr);

            Expression p_dev_strain1_expr;
            p_dev_strain1_expr.SetName("derived/"+dirs[i]+"prin_dev_strain/1");
            p_dev_strain1_expr.SetDefinition
                ("principal_deviatoric_tensor(<"+dirs[i]+"strain>)[0]");
            p_dev_strain1_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&p_dev_strain1_expr);

            Expression p_dev_strain2_expr;
            p_dev_strain2_expr.SetName("derived/"+dirs[i]+"prin_dev_strain/2");
            p_dev_strain2_expr.SetDefinition
                ("principal_deviatoric_tensor(<"+dirs[i]+"strain>)[1]");
            p_dev_strain2_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&p_dev_strain2_expr);

            Expression p_dev_strain3_expr;
            p_dev_strain3_expr.SetName("derived/"+dirs[i]+"prin_dev_strain/3");
            p_dev_strain3_expr.SetDefinition
                ("principal_deviatoric_tensor(<"+dirs[i]+"strain>)[2]");
            p_dev_strain3_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&p_dev_strain3_expr);

            Expression maxshr_expr;
            maxshr_expr.SetName("derived/"+dirs[i]+"max_shear_strain");
            maxshr_expr.SetDefinition
                ("tensor_maximum_shear(<"+dirs[i]+"strain>)");
            maxshr_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&maxshr_expr);

            Expression prin_strain1_expr;
            prin_strain1_expr.SetName("derived/"+dirs[i]+"prin_strain/1");
            prin_strain1_expr.SetDefinition
                ("principal_tensor(<"+dirs[i]+"strain>)[0]");
            prin_strain1_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&prin_strain1_expr);

            Expression prin_strain2_expr;
            prin_strain2_expr.SetName("derived/"+dirs[i]+"prin_strain/2");
            prin_strain2_expr.SetDefinition
                ("principal_tensor(<"+dirs[i]+"strain>)[1]");
            prin_strain2_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&prin_strain2_expr);

            Expression prin_strain3_expr;
            prin_strain3_expr.SetName("derived/"+dirs[i]+"prin_strain/3");
            prin_strain3_expr.SetDefinition
                ("principal_tensor(<"+dirs[i]+"strain>)[2]");
            prin_strain3_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&prin_strain3_expr);
        }
    }
    CATCH(InvalidVariableException)
    {
    // If strain not given compute the strain at nodes given 
    // the current geometry and the initial configuration.
      for (i = 0; i < dirs.size(); i++)
      {
        string tmpmeshname = "mesh1";
        if (dirs[i] != "") 
        {
            tmpmeshname += "_"+string(no_free_nodes_str);
        }
        string tmpvarname = "derived/"+dirs[i]+"strain/initial_strain_coords";
        string tmpvelname = dirs[i]+"nodvel";
        Expression initial_coords_expr;
        initial_coords_expr.SetName(tmpvarname);
        initial_coords_expr.SetDefinition
            ("conn_cmfe(coord(<[0]i:"+tmpmeshname+">),"+tmpmeshname+")");
        initial_coords_expr.SetType(Expression::VectorMeshVar);
        initial_coords_expr.SetHidden(true);
        md->AddExpression(&initial_coords_expr);

        Expression strain_green_expr;
        strain_green_expr.SetName("derived/"+dirs[i]+"strain/green_lagrange");
        strain_green_expr.SetDefinition(
           "strain_green_lagrange("+tmpmeshname+",<"+tmpvarname+">)");
        strain_green_expr.SetType(Expression::TensorMeshVar);
        md->AddExpression(&strain_green_expr);

        Expression strain_infinitesimal_expr;
        strain_infinitesimal_expr.SetName
            ("derived/"+dirs[i]+"strain/infinitesimal");
        strain_infinitesimal_expr.SetDefinition(
           "strain_infinitesimal("+tmpmeshname+",<"+tmpvarname+">)");
        strain_infinitesimal_expr.SetType(Expression::TensorMeshVar);
        md->AddExpression(&strain_infinitesimal_expr);

        Expression strain_almansi_expr;
        strain_almansi_expr.SetName("derived/"+dirs[i]+"strain/almansi");
        strain_almansi_expr.SetDefinition(
           "strain_almansi("+tmpmeshname+",<"+tmpvarname+">)");
        strain_almansi_expr.SetType(Expression::TensorMeshVar);
        md->AddExpression(&strain_almansi_expr);

        Expression strain_rate_expr;
        strain_rate_expr.SetName("derived/"+dirs[i]+"strain/rate");
        strain_rate_expr.SetDefinition(
           "strain_rate("+tmpmeshname+",<"+tmpvelname+">)");
        strain_rate_expr.SetType(Expression::TensorMeshVar);
        md->AddExpression(&strain_rate_expr);

// green_lagrange strain
        Expression straingx_expr;
        straingx_expr.SetName
            ("derived/"+dirs[i]+"strain/green_lagrange_strain/x");
        straingx_expr.SetDefinition
            ("<derived/"+dirs[i]+"strain/green_lagrange>[0][0]");
        straingx_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&straingx_expr);

        Expression straingy_expr;
        straingy_expr.SetName
            ("derived/"+dirs[i]+"strain/green_lagrange_strain/y");
        straingy_expr.SetDefinition
            ("<derived/"+dirs[i]+"strain/green_lagrange>[1][1]");
        straingy_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&straingy_expr);

        Expression straingz_expr;
        straingz_expr.SetName
            ("derived/"+dirs[i]+"strain/green_lagrange_strain/z");
        straingz_expr.SetDefinition
            ("<derived/"+dirs[i]+"strain/green_lagrange>[2][2]");
        straingz_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&straingz_expr);

        Expression straingxy_expr;
        straingxy_expr.SetName
            ("derived/"+dirs[i]+"strain/green_lagrange_strain/xy");
        straingxy_expr.SetDefinition
            ("<derived/"+dirs[i]+"strain/green_lagrange>[0][1]");
        straingxy_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&straingxy_expr);

        Expression straingxz_expr;
        straingxz_expr.SetName
            ("derived/"+dirs[i]+"strain/green_lagrange_strain/xz");
        straingxz_expr.SetDefinition
            ("<derived/"+dirs[i]+"strain/green_lagrange>[0][2]");
        straingxz_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&straingxz_expr);

        Expression straingyz_expr;
        straingyz_expr.SetName
            ("derived/"+dirs[i]+"strain/green_lagrange_strain/yz");
        straingyz_expr.SetDefinition
            ("<derived/"+dirs[i]+"strain/green_lagrange>[1][2]");
        straingyz_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&straingyz_expr);

        Expression sgeff_expr;
        sgeff_expr.SetName
            ("derived/"+dirs[i]+"strain/green_lagrange_strain/eff_strain");
        sgeff_expr.SetDefinition
           ("effective_tensor(<derived/"+dirs[i]+"strain/green_lagrange>)");
        sgeff_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&sgeff_expr);

        Expression p_dev_straing1_expr;
        p_dev_straing1_expr.SetName
           ("derived/"+dirs[i]+"strain/green_lagrange_strain/prin_dev_strain/1");
        p_dev_straing1_expr.SetDefinition
           ("principal_deviatoric_tensor(<derived/"+dirs[i]+
           "strain/green_lagrange>)[0]");
        p_dev_straing1_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&p_dev_straing1_expr);

        Expression p_dev_straing2_expr;
        p_dev_straing2_expr.SetName
           ("derived/"+dirs[i]+
           "strain/green_lagrange_strain/prin_dev_strain/2");
        p_dev_straing2_expr.SetDefinition
           ("principal_deviatoric_tensor(<derived/"+dirs[i]+
           "strain/green_lagrange>)[1]");
        p_dev_straing2_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&p_dev_straing2_expr);

        Expression p_dev_straing3_expr;
        p_dev_straing3_expr.SetName
           ("derived/"+dirs[i]+
           "strain/green_lagrange_strain/prin_dev_strain/3");
        p_dev_straing3_expr.SetDefinition
           ("principal_deviatoric_tensor(<derived/"+dirs[i]+
           "strain/green_lagrange>)[2]");
        p_dev_straing3_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&p_dev_straing3_expr);

        Expression maxshrg_expr;
        maxshrg_expr.SetName
           ("derived/"+dirs[i]+"strain/green_lagrange_strain/max_shear_strain");
        maxshrg_expr.SetDefinition
           ("tensor_maximum_shear(<derived/"+dirs[i]+"strain/green_lagrange>)");
        maxshrg_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&maxshrg_expr);

        Expression prin_straing1_expr;
        prin_straing1_expr.SetName
           ("derived/"+dirs[i]+"strain/green_lagrange_strain/prin_strain/1");
        prin_straing1_expr.SetDefinition
           ("principal_tensor(<derived/"+dirs[i]+"strain/green_lagrange>)[0]");
        prin_straing1_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&prin_straing1_expr);

        Expression prin_straing2_expr;
        prin_straing2_expr.SetName
           ("derived/"+dirs[i]+"strain/green_lagrange_strain/prin_strain/2");
        prin_straing2_expr.SetDefinition
           ("principal_tensor(<derived/"+dirs[i]+"strain/green_lagrange>)[1]");
        prin_straing2_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&prin_straing2_expr);

        Expression prin_straing3_expr;
        prin_straing3_expr.SetName
           ("derived/"+dirs[i]+"strain/green_lagrange_strain/prin_strain/3");
        prin_straing3_expr.SetDefinition
           ("principal_tensor(<derived/"+dirs[i]+"strain/green_lagrange>)[2]");
        prin_straing3_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&prin_straing3_expr);

// infinitesimal strain
        Expression strainix_expr;
        strainix_expr.SetName
            ("derived/"+dirs[i]+"strain/infinitesimal_strain/x");
        strainix_expr.SetDefinition
            ("<derived/"+dirs[i]+"strain/infinitesimal>[0][0]");
        strainix_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&strainix_expr);

        Expression strainiy_expr;
        strainiy_expr.SetName
            ("derived/"+dirs[i]+"strain/infinitesimal_strain/y");
        strainiy_expr.SetDefinition
            ("<derived/"+dirs[i]+"strain/infinitesimal>[1][1]");
        strainiy_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&strainiy_expr);

        Expression strainiz_expr;
        strainiz_expr.SetName
            ("derived/"+dirs[i]+"strain/infinitesimal_strain/z");
        strainiz_expr.SetDefinition
            ("<derived/"+dirs[i]+"strain/infinitesimal>[2][2]");
        strainiz_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&strainiz_expr);

        Expression strainixy_expr;
        strainixy_expr.SetName
            ("derived/"+dirs[i]+"strain/infinitesimal_strain/xy");
        strainixy_expr.SetDefinition
            ("<derived/"+dirs[i]+"strain/infinitesimal>[0][1]");
        strainixy_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&strainixy_expr);

        Expression strainixz_expr;
        strainixz_expr.SetName
            ("derived/"+dirs[i]+"strain/infinitesimal_strain/xz");
        strainixz_expr.SetDefinition
            ("<derived/"+dirs[i]+"strain/infinitesimal>[0][2]");
        strainixz_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&strainixz_expr);

        Expression strainiyz_expr;
        strainiyz_expr.SetName
            ("derived/"+dirs[i]+"strain/infinitesimal_strain/yz");
        strainiyz_expr.SetDefinition
            ("<derived/"+dirs[i]+"strain/infinitesimal>[1][2]");
        strainiyz_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&strainiyz_expr);

        Expression sieff_expr;
        sieff_expr.SetName
            ("derived/"+dirs[i]+"strain/infinitesimal_strain/eff_strain");
        sieff_expr.SetDefinition
           ("effective_tensor(<derived/"+dirs[i]+"strain/infinitesimal>)");
        sieff_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&sieff_expr);

        Expression p_dev_straini1_expr;
        p_dev_straini1_expr.SetName
           ("derived/"+dirs[i]+"strain/infinitesimal_strain/prin_dev_strain/1");
        p_dev_straini1_expr.SetDefinition
           ("principal_deviatoric_tensor(<derived/"+dirs[i]+
           "strain/infinitesimal>)[0]");
        p_dev_straini1_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&p_dev_straini1_expr);

        Expression p_dev_straini2_expr;
        p_dev_straini2_expr.SetName
           ("derived/"+dirs[i]+
           "strain/infinitesimal_strain/prin_dev_strain/2");
        p_dev_straini2_expr.SetDefinition
           ("principal_deviatoric_tensor(<derived/"+dirs[i]+
           "strain/infinitesimal>)[1]");
        p_dev_straini2_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&p_dev_straini2_expr);

        Expression p_dev_straini3_expr;
        p_dev_straini3_expr.SetName
           ("derived/"+dirs[i]+
           "strain/infinitesimal_strain/prin_dev_strain/3");
        p_dev_straini3_expr.SetDefinition
           ("principal_deviatoric_tensor(<derived/"+dirs[i]+
           "strain/infinitesimal>)[2]");
        p_dev_straini3_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&p_dev_straini3_expr);

        Expression maxshri_expr;
        maxshri_expr.SetName
           ("derived/"+dirs[i]+
           "strain/infinitesimal_strain/max_shear_strain");
        maxshri_expr.SetDefinition
           ("tensor_maximum_shear(<derived/"+dirs[i]+
           "strain/infinitesimal>)");
        maxshri_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&maxshri_expr);

        Expression prin_straini1_expr;
        prin_straini1_expr.SetName
           ("derived/"+dirs[i]+
           "strain/infinitesimal_strain/prin_strain/1");
        prin_straini1_expr.SetDefinition
           ("principal_tensor(<derived/"+dirs[i]+
           "strain/infinitesimal>)[0]");
        prin_straini1_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&prin_straini1_expr);

        Expression prin_straini2_expr;
        prin_straini2_expr.SetName
           ("derived/"+dirs[i]+
           "strain/infinitesimal_strain/prin_strain/2");
        prin_straini2_expr.SetDefinition
           ("principal_tensor(<derived/"+dirs[i]+
           "strain/infinitesimal>)[1]");
        prin_straini2_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&prin_straini2_expr);

        Expression prin_straini3_expr;
        prin_straini3_expr.SetName
           ("derived/"+dirs[i]+"strain/infinitesimal_strain/prin_strain/3");
        prin_straini3_expr.SetDefinition
           ("principal_tensor(<derived/"+dirs[i]+"strain/infinitesimal>)[2]");
        prin_straini3_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&prin_straini3_expr);

// almansi strain
        Expression strainax_expr;
        strainax_expr.SetName("derived/"+dirs[i]+"strain/almansi_strain/x");
        strainax_expr.SetDefinition
            ("<derived/"+dirs[i]+"strain/almansi>[0][0]");
        strainax_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&strainax_expr);

        Expression strainay_expr;
        strainay_expr.SetName
            ("derived/"+dirs[i]+"strain/almansi_strain/y");
        strainay_expr.SetDefinition
            ("<derived/"+dirs[i]+"strain/almansi>[1][1]");
        strainay_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&strainay_expr);

        Expression strainaz_expr;
        strainaz_expr.SetName("derived/"+dirs[i]+"strain/almansi_strain/z");
        strainaz_expr.SetDefinition
            ("<derived/"+dirs[i]+"strain/almansi>[2][2]");
        strainaz_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&strainaz_expr);

        Expression strainaxy_expr;
        strainaxy_expr.SetName("derived/"+dirs[i]+"strain/almansi_strain/xy");
        strainaxy_expr.SetDefinition
            ("<derived/"+dirs[i]+"strain/almansi>[0][1]");
        strainaxy_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&strainaxy_expr);

        Expression strainaxz_expr;
        strainaxz_expr.SetName("derived/"+dirs[i]+"strain/almansi_strain/xz");
        strainaxz_expr.SetDefinition
            ("<derived/"+dirs[i]+"strain/almansi>[0][2]");
        strainaxz_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&strainaxz_expr);

        Expression strainayz_expr;
        strainayz_expr.SetName("derived/"+dirs[i]+"strain/almansi_strain/yz");
        strainayz_expr.SetDefinition
            ("<derived/"+dirs[i]+"strain/almansi>[1][2]");
        strainayz_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&strainayz_expr);

        Expression saeff_expr;
        saeff_expr.SetName
            ("derived/"+dirs[i]+"strain/almansi_strain/eff_strain");
        saeff_expr.SetDefinition
           ("effective_tensor(<derived/"+dirs[i]+"strain/almansi>)");
        saeff_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&saeff_expr);

        Expression p_dev_straina1_expr;
        p_dev_straina1_expr.SetName
           ("derived/"+dirs[i]+"strain/almansi_strain/prin_dev_strain/1");
        p_dev_straina1_expr.SetDefinition
           ("principal_deviatoric_tensor(<derived/"+dirs[i]+
           "strain/almansi>)[0]");
        p_dev_straina1_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&p_dev_straina1_expr);

        Expression p_dev_straina2_expr;
        p_dev_straina2_expr.SetName
           ("derived/"+dirs[i]+"strain/almansi_strain/prin_dev_strain/2");
        p_dev_straina2_expr.SetDefinition
           ("principal_deviatoric_tensor(<derived/"+dirs[i]+
           "strain/almansi>)[1]");
        p_dev_straina2_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&p_dev_straina2_expr);

        Expression p_dev_straina3_expr;
        p_dev_straina3_expr.SetName
           ("derived/"+dirs[i]+"strain/almansi_strain/prin_dev_strain/3");
        p_dev_straina3_expr.SetDefinition
           ("principal_deviatoric_tensor(<derived/"+dirs[i]+
           "strain/almansi>)[2]");
        p_dev_straina3_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&p_dev_straina3_expr);

        Expression maxshra_expr;
        maxshra_expr.SetName
           ("derived/"+dirs[i]+"strain/almansi_strain/max_shear_strain");
        maxshra_expr.SetDefinition
           ("tensor_maximum_shear(<derived/"+dirs[i]+"strain/almansi>)");
        maxshra_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&maxshra_expr);

        Expression prin_straina1_expr;
        prin_straina1_expr.SetName
           ("derived/"+dirs[i]+"strain/almansi_strain/prin_strain/1");
        prin_straina1_expr.SetDefinition
           ("principal_tensor(<derived/"+dirs[i]+"strain/almansi>)[0]");
        prin_straina1_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&prin_straina1_expr);

        Expression prin_straina2_expr;
        prin_straina2_expr.SetName
           ("derived/"+dirs[i]+"strain/almansi_strain/prin_strain/2");
        prin_straina2_expr.SetDefinition
           ("principal_tensor(<derived/"+dirs[i]+"strain/almansi>)[1]");
        prin_straina2_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&prin_straina2_expr);

        Expression prin_straina3_expr;
        prin_straina3_expr.SetName
           ("derived/"+dirs[i]+"strain/almansi_strain/prin_strain/3");
        prin_straina3_expr.SetDefinition
           ("principal_tensor(<derived/"+dirs[i]+"strain/almansi>)[2]");
        prin_straina3_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&prin_straina3_expr);

// Rate strain
        Expression strainrx_expr;
        strainrx_expr.SetName("derived/"+dirs[i]+"strain/rate_strain/x");
        strainrx_expr.SetDefinition("<derived/"+dirs[i]+"strain/rate>[0][0]");
        strainrx_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&strainrx_expr);

        Expression strainry_expr;
        strainry_expr.SetName("derived/"+dirs[i]+"strain/rate_strain/y");
        strainry_expr.SetDefinition("<derived/"+dirs[i]+"strain/rate>[1][1]");
        strainry_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&strainry_expr);

        Expression strainrz_expr;
        strainrz_expr.SetName("derived/"+dirs[i]+"strain/rate_strain/z");
        strainrz_expr.SetDefinition("<derived/"+dirs[i]+"strain/rate>[2][2]");
        strainrz_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&strainrz_expr);

        Expression strainrxy_expr;
        strainrxy_expr.SetName("derived/"+dirs[i]+"strain/rate_strain/xy");
        strainrxy_expr.SetDefinition("<derived/"+dirs[i]+"strain/rate>[0][1]");
        strainrxy_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&strainrxy_expr);

        Expression strainrxz_expr;
        strainrxz_expr.SetName("derived/"+dirs[i]+"strain/rate_strain/xz");
        strainrxz_expr.SetDefinition("<derived/"+dirs[i]+"strain/rate>[0][2]");
        strainrxz_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&strainrxz_expr);

        Expression strainryz_expr;
        strainryz_expr.SetName("derived/"+dirs[i]+"strain/rate_strain/yz");
        strainryz_expr.SetDefinition("<derived/"+dirs[i]+"strain/rate>[1][2]");
        strainryz_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&strainryz_expr);

        Expression sreff_expr;
        sreff_expr.SetName("derived/"+dirs[i]+"strain/rate_strain/eff_strain");
        sreff_expr.SetDefinition
           ("effective_tensor(<derived/"+dirs[i]+"strain/rate>)");
        sreff_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&sreff_expr);

        Expression p_dev_strainr1_expr;
        p_dev_strainr1_expr.SetName
           ("derived/"+dirs[i]+"strain/rate_strain/prin_dev_strain/1");
        p_dev_strainr1_expr.SetDefinition
           ("principal_deviatoric_tensor(<derived/"+dirs[i]+"strain/rate>)[0]");
        p_dev_strainr1_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&p_dev_strainr1_expr);

        Expression p_dev_strainr2_expr;
        p_dev_strainr2_expr.SetName
           ("derived/"+dirs[i]+"strain/rate_strain/prin_dev_strain/2");
        p_dev_strainr2_expr.SetDefinition
           ("principal_deviatoric_tensor(<derived/"+dirs[i]+"strain/rate>)[1]");
        p_dev_strainr2_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&p_dev_strainr2_expr);

        Expression p_dev_strainr3_expr;
        p_dev_strainr3_expr.SetName
           ("derived/"+dirs[i]+"strain/rate_strain/prin_dev_strain/3");
        p_dev_strainr3_expr.SetDefinition
           ("principal_deviatoric_tensor(<derived/"+dirs[i]+"strain/rate>)[2]");
        p_dev_strainr3_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&p_dev_strainr3_expr);

        Expression maxshrr_expr;
        maxshrr_expr.SetName
           ("derived/"+dirs[i]+"strain/rate_strain/max_shear_strain");
        maxshrr_expr.SetDefinition
           ("tensor_maximum_shear(<derived/"+dirs[i]+"strain/rate>)");
        maxshrr_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&maxshrr_expr);

        Expression prin_strainr1_expr;
        prin_strainr1_expr.SetName
           ("derived/"+dirs[i]+"strain/rate_strain/prin_strain/1");
        prin_strainr1_expr.SetDefinition
           ("principal_tensor(<derived/"+dirs[i]+"strain/rate>)[0]");
        prin_strainr1_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&prin_strainr1_expr);

        Expression prin_strainr2_expr;
        prin_strainr2_expr.SetName
           ("derived/"+dirs[i]+"strain/rate_strain/prin_strain/2");
        prin_strainr2_expr.SetDefinition
           ("principal_tensor(<derived/"+dirs[i]+"strain/rate>)[1]");
        prin_strainr2_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&prin_strainr2_expr);

        Expression prin_strainr3_expr;
        prin_strainr3_expr.SetName
           ("derived/"+dirs[i]+"strain/rate_strain/prin_strain/3");
        prin_strainr3_expr.SetDefinition
           ("principal_tensor(<derived/"+dirs[i]+"strain/rate>)[2]");
        prin_strainr3_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&prin_strainr3_expr);
      }
    }
    ENDTRY

    TRY
    {
        // This call throw an exception if nodvel does not exist.
        GetVariableIndex("nodvel");

        for (i = 0; i < ndirs.size(); i++)
        {
            Expression velx_expr;
            velx_expr.SetName("derived/"+ndirs[i]+"velocity/x");
            velx_expr.SetDefinition("<"+ndirs[i]+"nodvel>[0]");
            velx_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&velx_expr);

            Expression vely_expr;
            vely_expr.SetName("derived/"+ndirs[i]+"velocity/y");
            vely_expr.SetDefinition("<"+ndirs[i]+"nodvel>[1]");
            vely_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&vely_expr);

            Expression velz_expr;
            velz_expr.SetName("derived/"+ndirs[i]+"velocity/z");
            velz_expr.SetDefinition("<"+ndirs[i]+"nodvel>[2]");
            velz_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&velz_expr);

            Expression velmag_expr;
            velmag_expr.SetName("derived/"+ndirs[i]+"velocity/mag");
            velmag_expr.SetDefinition("magnitude(<"+ndirs[i]+"nodvel>)");
            velmag_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&velmag_expr);
        }
    }
    CATCH(InvalidVariableException)
    {
    }
    ENDTRY

    TRY
    {
        // This call throw an exception if nodacc does not exist.
        GetVariableIndex("nodacc");

        for (i = 0; i < ndirs.size(); i++)
        {
            Expression accx_expr;
            accx_expr.SetName("derived/"+ndirs[i]+"acceleration/x");
            accx_expr.SetDefinition("<"+ndirs[i]+"nodacc>[0]");
            accx_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&accx_expr);

            Expression accy_expr;
            accy_expr.SetName("derived/"+ndirs[i]+"acceleration/y");
            accy_expr.SetDefinition("<"+ndirs[i]+"nodacc>[1]");
            accy_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&accy_expr);

            Expression accz_expr;
            accz_expr.SetName("derived/"+ndirs[i]+"acceleration/z");
            accz_expr.SetDefinition("<"+ndirs[i]+"nodacc>[2]");
            accz_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&accz_expr);

            Expression accmag_expr;
            accmag_expr.SetName("derived/"+ndirs[i]+"acceleration/mag");
            accmag_expr.SetDefinition("magnitude(<"+ndirs[i]+"nodacc>)");
            accmag_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&accmag_expr);
        }
    }
    CATCH(InvalidVariableException)
    {
    }
    ENDTRY

    TRY
    {
        // This call throw an exception if noddisp does not exist.
        GetVariableIndex("noddisp");

        for (i = 0; i < ndirs.size(); i++)
        {
            Expression dispx_expr;
            dispx_expr.SetName("derived/"+ndirs[i]+"displacement/x");
            dispx_expr.SetDefinition("<"+ndirs[i]+"noddisp>[0]");
            dispx_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&dispx_expr);

            Expression dispy_expr;
            dispy_expr.SetName("derived/"+ndirs[i]+"displacement/y");
            dispy_expr.SetDefinition("<"+ndirs[i]+"noddisp>[1]");
            dispy_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&dispy_expr);

            Expression dispz_expr;
            dispz_expr.SetName("derived/"+ndirs[i]+"displacement/z");
            dispz_expr.SetDefinition("<"+ndirs[i]+"noddisp>[2]");
            dispz_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&dispz_expr);

            Expression dispmag_expr;
            dispmag_expr.SetName("derived/"+ndirs[i]+"displacement/mag");
            dispmag_expr.SetDefinition("magnitude(<"+ndirs[i]+"noddisp>)");
            dispmag_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&dispmag_expr);
        }
    }
    CATCH(InvalidVariableException)
    {
        for (i = 0; i < ndirs.size(); i++)
        {   
            string tmpmeshname = "mesh1";
            if (dirs[i] != "") 
                tmpmeshname += "_"+string(no_free_nodes_str);
            string tmpvarname = 
                "derived/"+dirs[i]+"displacement/initial_disp_coords";
                                                              
            Expression initial_disp_coords;              
            initial_disp_coords.SetName(tmpvarname);
            initial_disp_coords.SetDefinition
                ("conn_cmfe(coord(<[0]i:"+tmpmeshname+">),"+tmpmeshname+")");
            initial_disp_coords.SetType(Expression::VectorMeshVar);
            initial_disp_coords.SetHidden(true);              
            md->AddExpression(&initial_disp_coords);       
                                            
            Expression noddisp;
            noddisp.SetName("derived/"+dirs[i]+"displacement/vec");
            noddisp.SetDefinition( 
               "displacement("+tmpmeshname+",<"+tmpvarname+">)");
            noddisp.SetType(Expression::VectorMeshVar);
            noddisp.SetHidden(true);
            md->AddExpression(&noddisp);
       
            Expression dispx_expr;
            dispx_expr.SetName("derived/"+dirs[i]+"displacement/x");
            dispx_expr.SetDefinition
                ("<derived/"+dirs[i]+"displacement/vec>[0]");
            dispx_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&dispx_expr);
                                  
            Expression dispy_expr;                            
            dispy_expr.SetName("derived/"+dirs[i]+"displacement/y");
            dispy_expr.SetDefinition                      
                ("<derived/"+dirs[i]+"displacement/vec>[1]");
            dispy_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&dispy_expr);
                                                              
            Expression dispz_expr;                            
            dispz_expr.SetName("derived/"+dirs[i]+"displacement/z");
            dispz_expr.SetDefinition                          
                ("<derived/"+dirs[i]+"displacement/vec>[2]"); 
            dispz_expr.SetType(Expression::ScalarMeshVar);    
            md->AddExpression(&dispz_expr);                   
                                                              
            Expression dispmag_expr;                          
            dispmag_expr.SetName("derived/"+dirs[i]+"displacement/mag");    
            dispmag_expr.SetDefinition                        
                ("magnitude(<derived/"+dirs[i]+"displacement/vec>)");       
            dispmag_expr.SetType(Expression::ScalarMeshVar);  
            md->AddExpression(&dispmag_expr);                 
        }
    }
    ENDTRY

    if (nmeshes == 1)
    {
        for (i = 0; i < ndirs.size(); i++)
        {
            Expression posx_expr;
            posx_expr.SetName("derived/"+ndirs[i]+"nodpos/x");
            posx_expr.SetDefinition("coord(mesh1"+nsuff[i]+")[0]");
            posx_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&posx_expr);

            Expression posy_expr;
            posy_expr.SetName("derived/"+ndirs[i]+"nodpos/y");
            posy_expr.SetDefinition("coord(mesh1"+nsuff[i]+")[1]");
            posy_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&posy_expr);

            Expression posz_expr;
            posz_expr.SetName("derived/"+ndirs[i]+"nodpos/z");
            posz_expr.SetDefinition("coord(mesh1"+nsuff[i]+")[2]");
            posz_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&posz_expr);
        }
    }
    else
    {
        for (i = 0; i < nmeshes; ++i)
        {
            for (int j = 0; j < ndirs.size(); j++)
            {
                char meshname[32];
                char expr_name[128];
                char defn_name[128];
                sprintf(meshname, "mesh%d", i + 1);

                Expression posx_expr;
                sprintf(expr_name, "derived/%snodpos/%s/x", ndirs[i].c_str(), meshname);
                sprintf(defn_name, "coord(%s%s)[0]", meshname, nsuff[j].c_str());
                posx_expr.SetName(expr_name);
                posx_expr.SetDefinition(defn_name);
                posx_expr.SetType(Expression::ScalarMeshVar);
                md->AddExpression(&posx_expr);
        
                sprintf(expr_name, "derived/%snodpos/%s/y", ndirs[i].c_str(), meshname);
                sprintf(defn_name, "coord(%s%s)[1]", meshname, nsuff[j].c_str());
                Expression posy_expr;
                posy_expr.SetName(expr_name);
                posy_expr.SetDefinition(defn_name);
                posy_expr.SetType(Expression::ScalarMeshVar);
                md->AddExpression(&posy_expr);
        
                sprintf(expr_name, "derived/%snodpos/%s/z", ndirs[i].c_str(), meshname);
                sprintf(defn_name, "coord(%s%s)[2]", meshname, nsuff[j].c_str());
                Expression posz_expr;
                posz_expr.SetName(expr_name);
                posz_expr.SetDefinition(defn_name);
                posz_expr.SetType(Expression::ScalarMeshVar);
                md->AddExpression(&posz_expr);
            }
        }
    }

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
    if (check != NULL && check[0] != '\0' && !strstr(check, no_free_nodes_str))
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
// ****************************************************************************

void
avtMiliFileFormat::FreeUpResources()
{
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

bool
avtMiliFileFormat::CanCacheVariable(const char *varname)
{
    if (strncmp(varname, "params/", 7) == 0)
        return false;
    else
        return true;
}
