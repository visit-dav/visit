/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

// ***************************************************************************
//  Program: MakeMili
//
//  Purpose: 
//    Create a .mili file from a single or multi domained Mili dataset,
//    suitable for the Mili reader in VisIt. It will read the directory
//    (given by the path of the argument) to determine how many domains
//    there are.
//
//  Arguments:
//    Takes in a root name. Name can either be the Mili family name for
//    a single domained dataset, or the root of a Mili family name for
//    a multi domained dataset.
//
//    Also takes the optional arguments:
//      -dynapart <DynaPart partition file>
// 
//  Programmer:     Akira Haddox
//  Creation:       June 25, 2003
//
// ***************************************************************************

#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include <vector>
#include <string>

#include <visitstream.h>

extern "C"{
#include <mili.h>
#include <mili_enum.h>
}

#include <visit-config.h>

using namespace std;

#ifdef PARALLEL
#include <mpi.h>
#endif

//
// Global variables
//

// The rootname and path generated from the arguments.
char root[128];
int rootLen;
char *path;
char path_text[256];

// Flag: set if we're a single domain (and thus handle names differently).
bool singleDomainName = false;

// Process information. Default values for serial runs.
int numProcesses = 1;
int myRank = 0;

// Number of domains.
int ndomains;

// From avtTypes.h
#define     AVT_SCALAR_VAR      1
#define     AVT_VECTOR_VAR      2 
#define     AVT_UNKNOWN_TYPE    7  
#define     AVT_NODECENT        0
#define     AVT_ZONECENT        1

// ***************************************************************************
//  Struct: MiliInfo
//
//  Purpose:
//    Store information about a Mili dataset. Two different classifications
//    of data are stored: global data only used by the master process, and
//    collected data, which is potentially unique to each domain.
//
//  Modifications:
//    Akira Haddox, Fri Jul 25 10:49:57 PDT 2003
//    Added variable dimensionality.
//
// ***************************************************************************

struct MiliInfo
{
  // Used by master process:
    int                             ntimesteps;
    int                             ndomains;
    int                             dimensions;
    int                             numMeshes;
    
  // Unique to each domain
    vector<int>                     highestMaterial;
    vector<vector<int> >            varType;
    vector<vector<int> >            varCentering;
    vector<vector<string> >         varNames;
    vector<vector<int> >            varDimension;
};

//
// Prototypes
//

void DetermineRootAndPath(char *);
int GetNumDomains();
void ReadDomain(int, MiliInfo &);
void ReceiveInfo(vector<MiliInfo> &);
void SendInfo(MiliInfo &);
void CompileInfo(const vector<MiliInfo> &, MiliInfo &);
void PrintInfo(ostream &o, const MiliInfo &, bool);
int  VSSearch(const vector<string> &v, const string &s);
void FatalError(const string &);


// ***************************************************************************
//  Function: DetermineRootAndPath
//
//  Purpose:
//    Given an argument, parse out the rootname and the path to the root,
//    storing them in the global variables. Called by the master process.
//
//  Arguments:
//    fname     argument to the program
//
// ***************************************************************************

void DetermineRootAndPath(char *fname)
{
    //
    // Code from GRIZ.
    //
    const char *p_c, *p_src;
    const char *p_root_start, *p_root_end;
    char *p_dest;
 
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
    {
        /* No path preceding root name. Set to . */
        path_text[0] = '.';
        path_text[1] = '\0';
        path = path_text;
    }
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

    rootLen = strlen(root);
}


// ***************************************************************************
//  Function: GetNumDomains
//
//  Purpose:
//    Opens the directory specified in the global 'path', and looks for
//    files that match the global 'root'. Given this, it finds how many
//    datasets there are, or if it is a single domain problem. Called by
//    the master process.
//
//  Returns:
//    The number of domains, or 0 on failure.
//
//  Modifications:
//
//    Hank Childs, Mon Dec 20 11:12:24 PST 2004
//    If there is a choice between a multi domain set of files and a single
//    domain file, still continue to choose the multi domain set, but then
//    test to make sure it works.  If it doesn't declare it single domain.
//
// ***************************************************************************

int GetNumDomains()
{
    int lastdomain = -1;
    dirent *dp;
    DIR *dirp = opendir(path);
    while ((dp = readdir(dirp)) != NULL)
    {
        char const *f = dp->d_name;
        if (strstr(f, root) == f)
        {
            //
            // If it starts with our root, look at the next three
            // characters. If they're numbers, it's a multi domain.
            // If they're not, then it's a single domain.
            // 

            if (isdigit(f[rootLen]) && isdigit(f[rootLen + 1])
                                    && isdigit(f[rootLen + 2]))
            {
                int hundred = f[rootLen] - '0';
                int ten = f[rootLen + 1] - '0';
                int one = f[rootLen + 2] - '0';

                int dom = hundred * 100 + ten * 10 + one;
                if (dom > lastdomain)
                    lastdomain = dom;
            }
            else
            {
                // There exists a single domain name.
                singleDomainName = true;
            }
        }
    }
    closedir(dirp);

    // If we find multi domain names, take the multi domain name.
    if (lastdomain != -1)
        singleDomainName = false;

    if (!singleDomainName)
    {
        // Sometimes there will be additional files in the directory that
        // appear to be multiple domains, but are actually all part of the
        // same domain.  An mcopen will test whether or not there are actually
        // multiple domains.
        char rootname[255];
        sprintf(rootname, "%s%.3d", root, 0); 
    
        Famid dbid;
        int rval;
        rval = mc_open (rootname, path, "r", &dbid);
        if (rval != OK)
            singleDomainName = true;
    }
    
    if (singleDomainName)
        return 1;
    return lastdomain + 1;
}


// ***************************************************************************
//  Function: ReadDomain
//
//  Purpose:
//    Read domain specific information from a domain. Master process also
//    reads in global information.
//
//  Arguments:
//    dom       the domain
//    mi        the MiliInfo structure to fill.
//
//  Notes:
//    Will abort program if failure to open family or read variables.
//    Code based upon VisIt's avtMiliFileFormat.
//
//  Modifications:
//    Akira Haddox, Fri Jul 25 10:49:57 PDT 2003
//    Added variable dimensionality.
//
//    Mark C. Miller, Mon Jul 18 13:41:13 PDT 2005
//    Added logic to look for data in "param arrays"
// ***************************************************************************

void ReadDomain(int dom, MiliInfo &mi)
{
    //
    // Open the family.
    //
    char rootname[255];
    sprintf(rootname, "%s%.3d", root, dom); 
    
    Famid dbid;
    int rval;
    if (singleDomainName)
        rval = mc_open (root, path, "r", &dbid);
    else
        rval = mc_open (rootname, path, "r", &dbid);

    if (rval != OK)
    {
        char err[255];
        sprintf(err, "Unable to open family %s for domain %d.",
                singleDomainName ? root : rootname, dom);
        FatalError(err);
    }
    
    //
    // Get the number of meshes.
    //
    int nmeshes;
    mc_query_family(dbid, QTY_MESHES, NULL, NULL, (void *) &nmeshes);

    mi.numMeshes = nmeshes;

    mi.highestMaterial.resize(nmeshes);
    mi.varType.resize(nmeshes);
    mi.varCentering.resize(nmeshes);
    mi.varNames.resize(nmeshes);
    mi.varDimension.resize(nmeshes);
    
    int mesh_id;
    for (mesh_id = 0; mesh_id < nmeshes; ++mesh_id)
    {
        //
        // Read in the mesh connectivity to read in the materials.
        //
        char short_name[1024];
        char long_name[1024];

        const int n_elem_types = 8;
        static int elem_sclasses[n_elem_types] =
        {
            M_TRUSS, M_BEAM, M_TRI, M_QUAD, M_TET, M_PYRAMID, M_WEDGE, M_HEX
        };
        static int conn_count[n_elem_types] =
        {
            2, 3, 3, 4, 4, 5, 6, 8
        };

        vector < vector<int *> > mat_list;
        vector < vector<int> > list_size;
        list_size.resize(n_elem_types);
        mat_list.resize(n_elem_types);

        int i, j, k;
        for (i = 0 ; i < n_elem_types ; i++)
        {
            int args[2];
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
                
                mat_list[i].push_back(mat);
                list_size[i].push_back(nelems);
                delete [] part;
                delete [] conn;
            }
        }

        //
        // Using mat_list and list_size, find the highest material number
        // for this mesh.
        // 
        int size = 0;
        for (i = 0; i < list_size.size(); ++i)
            for (j = 0; j < list_size[i].size(); ++j)
                size += list_size[i][j];
        
        int elem, gr;
        int highestMat = 0;
        for (elem = 0; elem < mat_list.size(); ++elem)
        {
            for (gr = 0; gr < mat_list[elem].size(); ++gr)
            {
                int *ml = mat_list[elem][gr];
                for (i = 0; i < list_size[elem][gr]; ++i)
                {
                    int mat = ml[i]; 
                    if (highestMat < mat)
                        highestMat = mat;
                }
            }
        }

        mi.highestMaterial[mesh_id] = highestMat;

        for (i = 0; i < mat_list.size(); ++i)
            for (j = 0; j < mat_list[i].size(); ++j)
                delete[] (mat_list[i][j]);

        //
        // End read materials
        //
        // Begin read variables.
        //

        int srec_qty = 0;
        rval = mc_query_family(dbid, QTY_SREC_FMTS, NULL, NULL,
                               (void*) &srec_qty); 

        for (i = 0 ; i < srec_qty ; i++)
        {
            rval = mc_query_family(dbid, SREC_MESH, (void *) &i, NULL,
                                   (void *) &mesh_id);
            if (rval != OK)
            {
                char err[255];
                sprintf(err, "Failed to read variables for domain %d: %s.",
                        dom, singleDomainName ? root : rootname);
                FatalError(err);
            }

            int substates = 0;
            rval = mc_query_family(dbid, QTY_SUBRECS, (void *) &i, NULL,
                                   (void *) &substates);

            for (j = 0 ; j < substates ; j++)
            {
                Subrecord sr;
                rval = mc_get_subrec_def(dbid, i, j, &sr);

            //
            // glob vars are associated with the entire mesh.
            // mat vars deal with materials.
            // These are not variables we need to visualize. Skip them.
            //
            
                if (strcmp(sr.class_name, "glob") == 0)
                    continue;
                if (strcmp(sr.class_name, "mat") == 0)
                    continue;

                for (k = 0 ; k < sr.qty_svars ; k++)
                {
                     State_variable sv;
                     mc_get_svar_def(dbid, sr.svar_names[k], &sv);

                     mi.varNames[mesh_id].push_back(sv.short_name);

                     int cent;
                     if (strcmp(sr.class_name, "node") == 0)
                         cent = AVT_NODECENT;
                     else
                         cent = AVT_ZONECENT;
                     mi.varCentering[mesh_id].push_back(cent);
                     
                     int vartype;
                     if (sv.agg_type == SCALAR)
                         vartype = (AVT_SCALAR_VAR);
                     else if (sv.agg_type == VECTOR)
                         vartype = (AVT_VECTOR_VAR);
                     else
                         vartype = (AVT_UNKNOWN_TYPE);
                     mi.varType[mesh_id].push_back(vartype);

                    if (vartype == AVT_SCALAR_VAR)
                        mi.varDimension[mesh_id].push_back(1);
                    else
                        mi.varDimension[mesh_id].push_back(sv.vec_size);
                }
            }
        }

        //
        // Mili 1.09 defines some "param arrays" that may be read as mesh
        // variables. However, the interface to access them isn't as complete
        // as the interface for state vars. We don't know which mesh they
        // go with and we don't know their centering. Here, we use apriori
        // knowledge to specify centering and we just associate them
        // with every mesh.
        //
        static vector<int> paramVarTypes;
        static vector<string> paramVarNames;
        if (mesh_id == 0)
        {
            //
            // Ok, deal with known named "param arrays"
            //
            vector<string> knownpas;
            knownpas.push_back("Nodal Mass");
            knownpas.push_back("Nodal Volume");
            for (i = 0; i < knownpas.size(); i++)
            {
                //
                // Alas, Mili doesn't permit us to simply query for existence
                // of a param array. We need to actually attempt to read it and
                // if successful we will have done problem-sized I/O
                //
                void *values;
                if (mc_read_param_array(dbid, (char*) knownpas[i].c_str(), &values) == OK)
                {
                    free(values);
                    string paVarName = "params/" + knownpas[i];
                    paramVarNames.push_back(paVarName);
                    if (strstr(knownpas[i].c_str(), "Nodal") != 0)
                        paramVarTypes.push_back(AVT_NODECENT);
                    else
                        paramVarTypes.push_back(AVT_ZONECENT);
                }
            }
        }

        if (paramVarNames.size() > 0)
        {
            for (i = 0; i < paramVarNames.size(); i++)
            {
                mi.varNames[mesh_id].push_back(paramVarNames[i]);
                mi.varCentering[mesh_id].push_back(paramVarTypes[i]);
                mi.varType[mesh_id].push_back(AVT_SCALAR_VAR);
                mi.varDimension[mesh_id].push_back(1);
            }
        }
    }

    //
    // Master process looks up a few other items.
    //
    if (myRank == 0)
    {
        mi.ndomains = ndomains;
        mc_query_family(dbid, QTY_STATES, NULL, NULL, &(mi.ntimesteps));
        mc_query_family(dbid, QTY_DIMENSIONS, NULL, NULL, &(mi.dimensions));
    }
    mc_close(dbid);
}

// ***************************************************************************
//  Function: ReceiveInfo (parallel only)
//
//  Purpose:
//    Receive compiled information from other processes. Called by master
//    process. See SendInfo for routine.
//
//  Arguments:
//    info      vector of MiliInfo to fill
//
//  Modifications:
//    Akira Haddox, Fri Jul 25 10:49:57 PDT 2003
//    Added variable dimensionality.
//
// ***************************************************************************

void ReceiveInfo(vector<MiliInfo> &info)
{
#ifdef PARALLEL
    int p;
    for (p = 1; p < numProcesses; ++p)
    {
        MPI_Status st;
        MiliInfo &mi = info[p];

        // Recv numMeshes
        int nm;
        MPI_Recv(&nm, 1, MPI_INT, p, 0, MPI_COMM_WORLD, &st);
        mi.numMeshes = nm;

        mi.highestMaterial.resize(nm);
        mi.varNames.resize(nm);
        mi.varType.resize(nm);
        mi.varCentering.resize(nm);
        mi.varDimension.resize(nm);

        // Recv highest materials
        MPI_Recv(&(mi.highestMaterial[0]), nm, MPI_INT, p, 0, 
                                                        MPI_COMM_WORLD, &st);

        vector<int> numVars(nm);
        // Recv numVars
        MPI_Recv(&(numVars[0]), nm, MPI_INT, p, 0, MPI_COMM_WORLD, &st);

        int i;

        // Recv var types
        for (i = 0; i < nm; ++i)
        {
            mi.varType[i].resize(numVars[i]);
            MPI_Recv(&(mi.varType[i][0]), numVars[i], MPI_INT, p, 0,
                                                        MPI_COMM_WORLD, &st);
        }
        // Recv var centers
        for (i = 0; i < nm; ++i)
        {
            mi.varCentering[i].resize(numVars[i]);
            MPI_Recv(&(mi.varCentering[i][0]), numVars[i], MPI_INT, p, 0,
                                                        MPI_COMM_WORLD, &st);
        }

        // Recv var dimensions
        for (i = 0; i < nm; ++i)
        {
            mi.varDimension[i].resize(numVars[i]);
            MPI_Recv(&(mi.varDimension[i][0]), numVars[i], MPI_INT, p, 0,
                                                        MPI_COMM_WORLD, &st);
        }

        int maxLen;
        // Recv max_length
        MPI_Recv(&maxLen, 1, MPI_INT, p, 0, MPI_COMM_WORLD, &st);
        
        // Recv the var names
        char *str = new char[maxLen];
        for (i = 0; i < nm; ++i)
        {
            int j;
            for (j = 0; j < numVars[i]; ++j)
            {
                MPI_Recv(str, maxLen, MPI_CHAR, p, 0, MPI_COMM_WORLD, &st);
                mi.varNames[i].push_back(string(str));
            }
        }
        
        delete []str;
        
    }
#endif
}

// ***************************************************************************
//  Function: SendInfo (parallel only)
//
//  Purpose:
//    Send compiled information to the master process.
//
//  Arguments:
//    mi        MillInfo to transfer
//
//  Send structure:
//      maxLen is the maximum length of a varName string.
//  
//      What                    Type        Count
//    ----------------------------------------------------- 
//      number of meshes        int         1
//      highest materials       int[]       number of Meshes
//      number of variables     int[]       number of Meshes
//      for each mesh:
//          variable types      int[]       number of Meshes
//      for each mesh:
//          variable centers    int[]       number of Meshes
//      for each mesh:
//          variable dimenion   int[]       number of Meshes
//      for each mesh:
//          variable names      char[]      maxLen
//
//  Modifications:
//    Akira Haddox, Fri Jul 25 10:49:57 PDT 2003
//    Added variable dimensionality.
//
// ***************************************************************************

void SendInfo(MiliInfo &mi)
{
#ifdef PARALLEL
    int nm = mi.numMeshes;
    
    // Send numMeshes
    MPI_Send(&nm, 1, MPI_INT, 0, 0, MPI_COMM_WORLD); 

    // Send highest materials
    MPI_Send(&(mi.highestMaterial[0]), nm, MPI_INT, 0, 0, MPI_COMM_WORLD);

    vector<int> numVars(nm);
    int i;
    for (i = 0; i < nm; ++i)
    {
        numVars[i] = mi.varNames[i].size();
    }

    // Send number of variables
    MPI_Send(&(numVars[0]), nm, MPI_INT, 0, 0, MPI_COMM_WORLD);

    // Send the var types
    for (i = 0; i < nm; ++i)
        MPI_Send(&(mi.varType[i][0]), numVars[i], MPI_INT, 0, 0,
                                                        MPI_COMM_WORLD);
    
    // Send the var centers 
    for (i = 0; i < nm; ++i)
        MPI_Send(&(mi.varCentering[i][0]), numVars[i], MPI_INT, 0, 0,
                                                             MPI_COMM_WORLD);
    // Send the var dimension 
    for (i = 0; i < nm; ++i)
        MPI_Send(&(mi.varDimension[i][0]), numVars[i], MPI_INT, 0, 0,
                                                             MPI_COMM_WORLD);
    int maxLen = 0;
    for (i = 0; i < nm; ++i)
    {
        int j;
        for (j = 0; j < numVars[i]; ++j)
            if (mi.varNames[i][j].length() > maxLen)
                maxLen = mi.varNames[i][j].length();
    }
    ++maxLen;
    // Send the max_length
    MPI_Send(&maxLen, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    
    // Send the var names
    char *str = new char[maxLen];
    for (i = 0; i < nm; ++i)
    {
        int j;
        for (j = 0; j < numVars[i]; ++j)
        {
            strcpy(str, mi.varNames[i][j].c_str());
            MPI_Send(str, maxLen, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
        }
    }

    delete[] str;
#endif
}


// ***************************************************************************
//  Function: CompileInfo
//
//  Purpose:
//    Combine multi MiliInfo structures, removing redundant information.
//
//  Arguments:
//    info          the vector of MiliInfo structures to combine
//    result        the MiliInfo to fill
//
//  Notes:
//    The global dataset info is always taken from info[0], even though
//    it may be invalid. This ensures that when all the information is
//    finally compiled, the information will come through.
//
//  Modifications:
//    Akira Haddox, Fri Jul 25 10:49:57 PDT 2003
//    Added variable dimensionality.
//
// ***************************************************************************

void CompileInfo(const vector<MiliInfo> &info, MiliInfo &result)
{
    // Always take from the first record the master process info
    result.ntimesteps = info[0].ntimesteps;
    result.ndomains = info[0].ndomains;
    result.dimensions = info[0].dimensions;
    int nm = info[0].numMeshes;
    result.numMeshes = nm;
    
    result.highestMaterial.resize(nm, -1);
    result.varType.resize(nm);
    result.varCentering.resize(nm);
    result.varDimension.resize(nm);
    result.varNames.resize(nm);
   
    int i; 
    for (i = 0; i < info.size(); ++i)
    {
        const MiliInfo &mi = info[i]; 
        int m;
        for (m = 0; m < nm; ++m)
        {
            if (mi.highestMaterial[m] > result.highestMaterial[m])
                result.highestMaterial[m] = mi.highestMaterial[m];
            
            int v;
            for (v = 0; v < mi.varNames[m].size(); ++v)
            {
                const string &vn = mi.varNames[m][v];
                if (VSSearch(result.varNames[m], vn) == -1)
                {
                    result.varNames[m].push_back(vn);
                    result.varType[m].push_back(mi.varType[m][v]);
                    result.varCentering[m].push_back(mi.varCentering[m][v]);
                    result.varDimension[m].push_back(mi.varDimension[m][v]);
                } 
            }
        }
    }
}

// ***************************************************************************
//  Function: PrintInfo
//
//  Purpose:
//    Write a MiliInfo structure to a stream.
//
//  Arguments:
//    o         the stream
//    mi        the MiliInfo structure
//    verbose   if true, makes it human readable (unsuitable for .mili file)
//
//  Modifications:
//    Akira Haddox, Fri Jul 25 10:49:57 PDT 2003
//    Added variable dimensionality.
//
// ***************************************************************************

#define vp(a) if (verbose) o << a
void PrintInfo(ostream &o, const MiliInfo &mi, bool verbose)
{
    vp("MiliInfo: \n----------------------\n");
    vp("Domains:\t\t");
    o << mi.ndomains << endl;
    vp("Timesteps:\t\t");
    o << mi.ntimesteps << endl;
    vp("Dimensions:\t\t");
    o << mi.dimensions << endl;
    vp("Number of Meshes:\t");
    o << mi.numMeshes << endl;
    o << endl;

    int mesh;
    for (mesh = 0; mesh < mi.numMeshes; ++mesh)
    {
        vp("Mesh " << mesh << endl);
        vp("Number of Materials: ");
        o << mi.highestMaterial[mesh] + 1 << endl;
        int nVar = mi.varNames[mesh].size();
        vp("Number of Variables: " );
        o << nVar << endl;
        vp("\tType\tCenter\tDims\tName" << endl);
        int var;
        for (var = 0; var < mi.varNames[mesh].size(); ++var)
        {
            o << '\t' << mi.varType[mesh][var]  << '\t'
                      << mi.varCentering[mesh][var] << '\t'
                      << mi.varDimension[mesh][var] << '\t'
                      << mi.varNames[mesh][var] << endl;
        }
        o << endl;
    }
}


// ***************************************************************************
//  Function: VSSearch
//
//  Purpose:
//    Search a vector of strings.
//
//  Arguments:
//    v     the vector
//    s     the string to search for
//
//  Returns:
//    The index of s, or -1 if not found.
//
// ***************************************************************************
// 
int VSSearch(const vector<string> &v, const string &s)
{
    int i;
    for (i = 0; i < v.size(); ++i)
        if (v[i] == s)
            return i;
    return -1;
}

// ***************************************************************************
//  Function: FatalError
//
//  Purpose:
//    Prints an error message, and aborts program. The message is compacted
//    into one print in case multiple processes call this.
//
//  Arguments:
//    s         The reason for the error
//
// ***************************************************************************

void FatalError(const string &s)
{
    char error[1024];
    sprintf(error, "** Fatal error: %s Aborting program...\n", s.c_str());
    cerr << error;

#ifdef PARALLEL
    MPI_Abort(MPI_COMM_WORLD, -1);
#endif
    exit(-1);
}

// ***************************************************************************
//  Main
//
//  Arguments:
//      argv[1]     The root name of the dataset.
//
//  Modifications:
//    Akira Haddox, Tue Jul 22 08:12:57 PDT 2003
//    Added help on bad number of arguments.  
//
//    Akira Haddox, Fri Jul 25 11:21:38 PDT 2003
//    Added check to find rootname, ignoring hyphen-ed arguments.
//
//    Akira Haddox, Mon Aug 18 15:19:21 PDT 2003
//    Added -dynapart option.
//
// ***************************************************************************

int main(int argc, char* argv[])
{
#ifdef PARALLEL
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
#endif 

    char *dynapartFile = NULL;
    char * argument = NULL;
    int argi;
    for (argi = 1; argi < argc; ++argi)
    {
        if (!strcmp(argv[argi], "-dynapart"))
        {
            // Check the next argument for bad usage.
            if (argi + 1 >= argc || argv[argi + 1] == NULL
                                 || argv[argi + 1][0] == '-')
            {
                argument = NULL;
                break;
            }
            dynapartFile = argv[++argi];
        }
        else if (argv[argi][0] != '-' && !argument)
        {
            argument = argv[argi];
        }
    }

    if (argument == NULL)
    {
        if (myRank != 0)
            exit(-1);
        
        cout <<
"makemili help:\n"
"  Req. Argument:   the root name of a single or multi domained Mili dataset.\n"
"                   The path to the dataset may be included in the name.\n"
"                   The output .mili file will be written in the same\n"
"                   directory as the dataset.\n\n"
"  Opt. Argument:   -dynapart <DynaPart partition filename>\n" << endl;
 
        exit(-1);
    }

    DetermineRootAndPath(argument);
    
    //
    // Master process needs to determine how many domains there are.
    //
    if (myRank == 0)
    {
        ndomains = GetNumDomains();
        cout << "General: \n----------------------\n";
        cout << "Root: " << root << " \tPath: " << path << endl;
        if (dynapartFile)
            cout << "DynaPart file option: " << dynapartFile << endl;
        if (ndomains <= 0)
        {
            FatalError("Unable to find matching files.");
        }
        else
        {
            cout << "Domains found: " << ndomains;
            if (singleDomainName)
            {
                cout << " (single family: " << root << ")";
            }
            cout << endl << endl;
        }
    }
    
#ifdef PARALLEL
    //
    // Master process needs to tell children how many domains there are.
    //
    MPI_Bcast((void *)&ndomains, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif

    //
    // Now, find how many domains I am responsible for.
    //
    int startDomain;
    int endDomain;

    //
    // Special cases: If processes >= ndomains.
    // 
    if (numProcesses >= ndomains)
    {
#ifdef PARALLEL
        if (myRank == 0 && numProcesses > ndomains)
        {
            cout << "** More processes than domains. Restricting to "
                 << ndomains <<
                 (ndomains == 1 ? " process." : " processes.") << endl << endl;
        }
#endif
        if (myRank >= ndomains)
        {
#ifdef PARALLEL
            MPI_Finalize();
#endif
            return 0;
        }
        
        startDomain = myRank;
        endDomain = myRank;
        
        //
        // At this point, the number of processes may have changed.
        //
        numProcesses = ndomains;
    }
    else 
    {
        startDomain = (ndomains) * myRank / numProcesses;
        endDomain = (ndomains) * (myRank + 1) / numProcesses - 1; 
    }

    //
    // Make sure we always take care of the last domain.
    //
    if (myRank + 1 == numProcesses)
    {
        endDomain = ndomains - 1;
    }
    
    vector<MiliInfo> info(endDomain - startDomain + 1);
    
    int i;
    for (i = startDomain; i <= endDomain; ++i)
    {
        ReadDomain(i, info[i - startDomain]);
    }
    
    MiliInfo compiledInfo;
    CompileInfo(info, compiledInfo);
    
#ifdef PARALLEL
    if (myRank == 0)
    {
        vector<MiliInfo> gatheredInfo(numProcesses);
        gatheredInfo[0] = compiledInfo;
        // Receive domain info from all other processes
        ReceiveInfo(gatheredInfo);
        CompileInfo(gatheredInfo, compiledInfo);
    }
    else
    {
        SendInfo(compiledInfo);
    } 
#endif

    if (myRank == 0)
    {
        PrintInfo(cout, compiledInfo, true);
        if (dynapartFile)
            cout << "DynaPart file: " << dynapartFile << endl;

        char outfname[255];
        sprintf(outfname, "%s/%s.mili", path, root);
        ofstream out;
        out.open(outfname);
        if (out.fail())
            cerr << "** Error! Could not write to file: " << outfname << endl; 
        else
        {
            PrintInfo(out, compiledInfo, false);
            // Write out the dynapart file if we have one
            if (dynapartFile)
                out << dynapartFile << endl;
            cout << "File " << outfname << " successfully written." << endl; 
        }
        out.close();
    }

#ifdef PARALLEL
    MPI_Finalize();
#endif
}
