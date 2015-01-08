/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

//
// Creates a graphviz (dot) output file of a Silo AMR mrgtree.
//

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <silo.h>

using namespace std;


//---------------------------------------------------------------------------//
void ERROR(const std::string &err)
{
    cout << "<err: " << err <<  ">" << endl;
    exit(-1);
}

//---------------------------------------------------------------------------//
void
create_dot_output(int nlvls,
                  const std::vector<int> &lvls_map,
                  const std::vector<std::string> &nodes,
                  const std::vector<int> &edges_to,
                  const std::vector<int> &edges_fro,
                  const std::string &ofname)
{
    cout << "[creating: " << ofname << "]" <<endl;
    std::ofstream ofs(ofname.c_str());
    ofs << "digraph mrgTree {" << endl;

    for(size_t i=0; i < edges_fro.size(); i++)
    {
        ofs  << " \"" << nodes[edges_to[i]] << "\" -> " " \""
             << nodes[edges_fro[i]] << "\";" <<endl;
    }
    ofs << endl;

    for(int i=0; i < nlvls; i++)
    {
        ofs << " { rank = same; " <<endl;
        for(size_t j=0; j < lvls_map.size(); j++)
        {
            if(lvls_map[j] == i)
            {
                ofs << " \"" << nodes[j] << "\";" <<endl;
            }
        }
        ofs << " }" <<endl;
        ofs << endl;
    }

    ofs  << "}" << endl;
    ofs << endl;
}

//---------------------------------------------------------------------------//
// Taken from avtSiloFileFormat.C
//---------------------------------------------------------------------------//
DBgroupelmap *
GetCondensedGroupelMap(DBfile *dbfile,
                       DBmrgtnode *rootNode,
                       int dontForceSingle=0)
{
    int i,k,q,pass;
    DBgroupelmap *retval = 0;

    // We do this to prevent Silo for re-interpreting integer data in
    // groupel maps
    DBForceSingle(0);

    if (rootNode->num_children == 1 && rootNode->children[0]->narray == 0)
    {
        retval = DBAllocGroupelmap(0, DB_NOTYPE);
    }
    else if ((rootNode->num_children == 1 && rootNode->children[0]->narray > 0) ||
             (rootNode->num_children > 1 && rootNode->maps_name))
    {
        int nseg_mult = 1;
        DBmrgtnode *mapNode;
        if (rootNode->num_children == 1 && rootNode->children[0]->narray > 0)
        {
            nseg_mult = rootNode->children[0]->narray;
            mapNode = rootNode->children[0];
        }
        else
            mapNode = rootNode;

        //
        // Get the groupel map.
        //
        string mapsName = mapNode->maps_name;
        DBgroupelmap *gm = DBGetGroupelmap(dbfile, mapsName.c_str());

        //
        // One pass to count parts of map we'll be needing and a 2nd 
        // pass to allocate and transfer those parts to the returned map.
        //
        for (pass = 0; pass < 2; pass++)
        {
            if (pass == 1) /* allocate on 2nd pass */
            {
                retval = DBAllocGroupelmap(q, DB_NOTYPE);
                /* We won't need segment_ids because the map is condensed */ 
                free(retval->segment_ids);
                retval->segment_ids = 0;
            }

            q = 0;
            for (k = 0; k < mapNode->nsegs * nseg_mult; k++)
            {
                for (i = 0; i < gm->num_segments; i++)
                {
                    int gm_seg_id = gm->segment_ids ? gm->segment_ids[i] : i;
                    int tnode_seg_id = mapNode->seg_ids ? mapNode->seg_ids[k] : k;
                    int gm_seg_type = gm->groupel_types[i];
                    int tnode_seg_type = mapNode->seg_types[k];
                    if (gm_seg_id != tnode_seg_id ||
                        tnode_seg_type != DB_BLOCKCENT ||
                        gm_seg_type != DB_BLOCKCENT)
                        continue;

                    if (pass == 1) /* populate on 2nd pass */
                    {
                        retval->groupel_types[q] = DB_BLOCKCENT;
                        retval->segment_lengths[q] = gm->segment_lengths[tnode_seg_id];
                        /* Transfer ownership of segment_data to the condensed map */
                        retval->segment_data[q] = gm->segment_data[tnode_seg_id];
                        gm->segment_data[tnode_seg_id] = 0;
                    }

                    q++;
                }
            }
        }
        DBFreeGroupelmap(gm);
    }
    else
    {
        //
        // Multiple groupel maps are stored, one for each node
        //
        retval = DBAllocGroupelmap(rootNode->num_children, DB_NOTYPE);
        for (q = 0; q < rootNode->num_children; q++)
        {
            DBmrgtnode *rootChild = rootNode->children[q];
            string mapsName = rootChild->maps_name;
            DBgroupelmap *gm = DBGetGroupelmap(dbfile, mapsName.c_str());
            for (k = 0; k < rootChild->nsegs; k++)
            {
                for (i = 0; i < gm->num_segments; i++)
                {
                    int gm_seg_id = gm->segment_ids ? gm->segment_ids[i] : i;
                    int tnode_seg_id = rootChild->seg_ids ? rootChild->seg_ids[k] : k;
                    int gm_seg_type = gm->groupel_types[i];
                    int tnode_seg_type = rootChild->seg_types[k];
                    if (gm_seg_id != tnode_seg_id ||
                        tnode_seg_type != DB_BLOCKCENT ||
                        gm_seg_type != DB_BLOCKCENT)
                        continue;

                    retval->groupel_types[q] = DB_BLOCKCENT;
                    retval->segment_lengths[q] = gm->segment_lengths[i];
                    retval->segment_data[q] = gm->segment_data[i];
                    gm->segment_data[i] = 0;
                }
            }
            DBFreeGroupelmap(gm);
        }
    }

    DBForceSingle(!dontForceSingle);
    return retval;
}

//---------------------------------------------------------------------------//
// Adapted from from avtSiloFileFormat.C
//---------------------------------------------------------------------------//
void
extract_mrgtree(const std::string &root_fname,
                const std::string &mmesh_name,
                const std::string &odot_fname)
{

    std::vector<std::string> nodes;
    std::vector<int> edges_fro;
    std::vector<int> edges_to;

    int num_levels, num_patches, num_dims;
    int i,j;

    cout << "[opening: " << root_fname << "]" <<endl;

    DBfile *dbfile = DBOpen(root_fname.c_str(), DB_HDF5, DB_READ);

    cout << "[fetching: " << mmesh_name << "]" <<endl;
    // get the mmesh
    DBmultimesh *mm = DBGetMultimesh(dbfile, mmesh_name.c_str());

    cout << "[mrgtree name: " << mm->mrgtree_name << "]" <<endl;

    // get the mrgTree
    DBmrgtree *mrgTree = DBGetMrgtree(dbfile, mm->mrgtree_name);


    // Look through all the mrgtree's variable object names to see if
    // any define 'ratios' or 'ijk' extents. They are needed to
    // compute domain nesting and neighbor information.
    //
    char **vname = mrgTree ->mrgvar_onames;
    string ratioVarName;
    string ijkExtsVarName;
    while (vname && *vname != 0)
    {
        string vnameTmp = *vname;
        for (size_t k = 0; k < vnameTmp.size(); k++)
            vnameTmp[k] = tolower(vnameTmp[k]);

        if (vnameTmp.find("ratio") != string::npos)
            ratioVarName = *vname;
        if (vnameTmp.find("ijk") != string::npos)
            ijkExtsVarName = *vname;

        vname++;
    }
    if (ratioVarName == "")    ERROR("ratioVarName");
    if (ijkExtsVarName  == "") ERROR("ijkExtsVarName ");
    if (DBSetCwr(mrgTree, "amr_decomp") < 0) ERROR("cd amr_decomp");
    if (DBSetCwr(mrgTree, "levels") < 0)     ERROR("cd levels");

    DBmrgtnode *levelsNode = mrgTree->cwr;

    //
    // Get level grouping information from tree
    //
    DBgroupelmap *lvlgm = GetCondensedGroupelMap(dbfile, levelsNode);
    num_levels = lvlgm->num_segments;
    //cout << "num_levels = " << num_levels << endl;
    vector<int> levelId;
    levelId.resize(mm->nblocks,-1);
    for (i = 0; i < lvlgm->num_segments; i++)
    {
        for (j = 0; j < lvlgm->segment_lengths[i]; j++)
        {
            int patch_no = ((int**) lvlgm->segment_data)[i][j];
            levelId[patch_no] = i; 
        }
    }
    DBFreeGroupelmap(lvlgm);

    //
    // Try to go to the patches part of the amr_decomp
    //
    if (DBSetCwr(mrgTree, "..") < 0)       ERROR ("cd ..");
    if (DBSetCwr(mrgTree, "patches") < 0)  ERROR ("cd patches");

    DBmrgtnode *childsNode = mrgTree->cwr;

    //
    // Get Parent/Child maps
    //
    DBgroupelmap *chldgm = GetCondensedGroupelMap(dbfile, childsNode);

    //
    // Read the ratios variable (on the levels) and the parent/child
    // map.
    //
    DBForceSingle(0);
    DBmrgvar *ratvar = DBGetMrgvar(dbfile, ratioVarName.c_str());
    DBmrgvar *ijkvar = DBGetMrgvar(dbfile, ijkExtsVarName.c_str());
    DBForceSingle(!0);

    //
    // The number of patches can be inferred from the size of the child groupel map.
    //
    num_patches = chldgm->num_segments;

    //
    // The number of dimensions can be inferred from the number of components in
    // the ratios variable.
    //
    num_dims = ratvar->ncomps;


    //
    // If we need refinement level ratio information:
    //
    vector<int> ratios(3,1);
    for (i = 1; i < num_levels; i++)
    {
        int **ratvar_data = (int **) ratvar->data;
        ratios[0] = ratvar_data[0][i]; 
        ratios[1] = ratvar_data[1][i];
        ratios[2] = num_dims == 3 ? (int) ratvar_data[2][i]: 0;
        // cout << "ratios = " << ratios[0] << ", " << ratios[1] << ", " << ratios[2] << endl;
    }

    for (i = 0; i < num_patches; i++)
    {
        ostringstream oss;
        oss << "patch" << i;
        nodes.push_back(oss.str());

        for (j = 0; j < chldgm->segment_lengths[i]; j++)
        {
            int cid = chldgm->segment_data[i][j];
            edges_fro.push_back(i);
            edges_to.push_back(cid);
        }

        // If we need logical extents info:
        vector<int> logExts(6,0);
        int **ijkvar_data = (int **) ijkvar->data;
        logExts[0] = (int) ijkvar_data[0][i];
        logExts[1] = (int) ijkvar_data[2][i];
        logExts[2] = num_dims == 3 ? (int) ijkvar_data[4][i] : 0;
        logExts[3] = (int) ijkvar_data[1][i];
        logExts[4] = (int) ijkvar_data[3][i];
        logExts[5] = num_dims == 3 ? (int) ijkvar_data[5][i] : 0;
        //cout << "logExts = " << logExts[0] << ", " << logExts[1] << ", " << logExts[2] << endl;
        //cout << "          " << logExts[3] << ", " << logExts[4] << ", " << logExts[5] << endl;
    }


    if (ratvar)
       DBFreeMrgvar(ratvar);
    if (ijkvar)
       DBFreeMrgvar(ijkvar);
    if (chldgm)
       DBFreeGroupelmap(chldgm);

    DBFreeMrgtree(mrgTree);
    DBFreeMultimesh(mm);
    DBClose(dbfile);

    create_dot_output(num_levels,levelId, nodes,edges_fro,edges_to,odot_fname);
}




//---------------------------------------------------------------------------//
int
main(int argc, char **argv)
{
    if(argc < 4)
    {
        cout << "usage: mrgtree2dot [silo.root] [multimesh_name] [output.dot]" << endl;
        exit(-1);
    }

    std::string root_fname(argv[1]);
    std::string mmesh_name(argv[2]);
    std::string odot_fname(argv[3]);

    extract_mrgtree(root_fname,mmesh_name,odot_fname);

}

