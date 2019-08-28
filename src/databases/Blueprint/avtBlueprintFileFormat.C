/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
// avtBlueprintFileFormat.C
// ************************************************************************* //

#include "avtBlueprintFileFormat.h"

//-----------------------------------------------------------------------------
// visit includes
//-----------------------------------------------------------------------------
#include "avtDatabaseMetaData.h"
#include "avtResolutionSelection.h"

#include "DebugStream.h"
#include "StringHelpers.h"
#include "TimingsManager.h"

#include "DBOptionsAttributes.h"
#include "Expression.h"
#include "FileFunctions.h"
#include "InvalidVariableException.h"
#include "InvalidFilesException.h"
#include "UnexpectedValueException.h"

//-----------------------------------------------------------------------------
// std lib includes
//-----------------------------------------------------------------------------
#include <string>

//-----------------------------------------------------------------------------
// conduit includes
//-----------------------------------------------------------------------------
#include "conduit.hpp"
#include "conduit_relay.hpp"
#include "conduit_relay_io_hdf5.hpp"
#include "conduit_blueprint.hpp"

//-----------------------------------------------------------------------------
// mfem includes
//-----------------------------------------------------------------------------
#include "mfem.hpp"


#ifdef PARALLEL
#include <mpi.h>
#include <avtParallel.h>
#include "conduit_relay_mpi.hpp"
#endif


//-----------------------------------------------------------------------------
// bp visit plugin includes
//-----------------------------------------------------------------------------
#include "avtBlueprintLogging.h"
#include "avtBlueprintTreeCache.h"
#include "avtBlueprintDataAdaptor.h"


using std::string;
using namespace conduit;
using namespace mfem;


//-----------------------------------------------------------------------------
// These methods are used to re-wire conduit's default error handling
//-----------------------------------------------------------------------------
void
blueprint_plugin_print_msg(const std::string &msg,
                           const std::string &file,
                           int line)
{
    // Uncomment for very verbose traces:
    //
    // debug5 << "File:"    << file << std::endl
    //        << "Line:"    << line << std::endl
    //        << "Message:" << msg  << std::endl;
    debug5 << msg  << std::endl;
}

//-----------------------------------------------------------------------------
void
blueprint_plugin_info_handler(const std::string &msg,
                              const std::string &file,
                              int line)
{
    blueprint_plugin_print_msg(msg,file,line);
}


//-----------------------------------------------------------------------------
void
blueprint_plugin_warning_handler(const std::string &msg,
                                 const std::string &file,
                                 int line)
{
    blueprint_plugin_print_msg(msg,file,line);
}

//-----------------------------------------------------------------------------
void
blueprint_plugin_error_handler(const std::string &msg,
                               const std::string &file,
                               int line)
{
    std::ostringstream bp_err_oss;
    bp_err_oss << "[ERROR]"
               << "File:"    << file << std::endl
               << "Line:"    << line << std::endl
               << "Message:" << msg  << std::endl;

    debug1 << bp_err_oss.str();

    BP_PLUGIN_EXCEPTION1(InvalidVariableException, bp_err_oss.str());

}

// ****************************************************************************
void
split_mesh_and_topo(const std::string &name_name_full,
                    std::string &mesh_name,
                    std::string &topo_name)
{
    string mesh_base = FileFunctions::Basename(name_name_full);
    conduit::utils::rsplit_string(mesh_base,"_",topo_name,mesh_name);
}


// ****************************************************************************
std::string
sanitize_var_name(const std::string &varname)
{
    string res = StringHelpers::Replace(varname,":","_colon_");
    res = StringHelpers::Replace(res,"[","_lb_");
    res = StringHelpers::Replace(res,"]","_rb_");
    return res;
}


// ****************************************************************************
//  Method: avtBlueprintFileFormat constructor
//
//  Programmer: harrison37 -- generated by xml2avt
//  Creation:   Wed Jun 15 16:25:28 PST 2016
//
// ****************************************************************************
avtBlueprintFileFormat::avtBlueprintFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1),
      m_root_node(),
      m_protocol(""),
      m_tree_cache(NULL),
      m_selected_lod(0)
{
    m_tree_cache = new avtBlueprintTreeCache();

    // these redirect conduit info and warnings to debug 5
    conduit::utils::set_info_handler(blueprint_plugin_info_handler);
    conduit::utils::set_warning_handler(blueprint_plugin_warning_handler);
    // this catches any uncaught conduit errors, logs them to debug 1
    // and  converts them into a VisIt Exception
    conduit::utils::set_error_handler(blueprint_plugin_error_handler);
}

avtBlueprintFileFormat::~avtBlueprintFileFormat()
{
    delete m_tree_cache;
}


// ****************************************************************************
//  Method: avtBlueprintFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: harrison37 -- generated by xml2avt
//  Creation:   Wed Jun 15 16:25:28 PST 2016
//
// ****************************************************************************
void
avtBlueprintFileFormat::FreeUpResources(void)
{
    m_tree_cache->Release();
}

// ****************************************************************************
//  Method: avtBlueprintFileFormat::ReadBlueprintMesh
//
//  Purpose:
//      Reads a mesh domain into the `out` conduit Node.
//
//
//  Programmer: Cyrus Harrison and Mark Miller
//  Creation:   Fri Aug 12 13:45:34 PDT 2016
//
// ****************************************************************************
void
avtBlueprintFileFormat::ReadBlueprintMesh(int domain,
                                          const string &abs_meshname,
                                          Node &out)
{
    BP_PLUGIN_INFO("ReadBlueprintMesh: " << abs_meshname
                    << " [domain " << domain << "]");
    string mesh_name;
    string mesh_topo_name;
    split_mesh_and_topo(std::string(abs_meshname),
                        mesh_name,
                        mesh_topo_name);

    BP_PLUGIN_INFO("mesh name and topology name: "
                    << mesh_name << " " << mesh_topo_name);

    if (!m_root_node["blueprint_index"].has_child(mesh_name))
    {
        EXCEPTION1(InvalidVariableException, mesh_name);
    }


    string file_pattern = FileFunctions::Dirname(GetFilename())
                          + string("/")
                          + m_root_node["file_pattern"].as_string();
    string tree_pattern = m_root_node["tree_pattern"].as_string();

    const Node &bp_index_mesh_node = m_root_node["blueprint_index"][mesh_name];
    const Node &bp_index_topo_node = bp_index_mesh_node["topologies"][mesh_topo_name];

    string coordset_name = bp_index_topo_node["coordset"].as_string();
    string topo_path     = bp_index_topo_node["path"].as_string();
    string coords_path   = bp_index_mesh_node["coordsets"][coordset_name]["path"].as_string();

    BP_PLUGIN_INFO("coordset path " << coords_path);

    m_tree_cache->FetchBlueprintTree(domain,
                                     coords_path,
                                     out["coordsets"][coordset_name]);

    BP_PLUGIN_INFO("topology path " << topo_path);

    m_tree_cache->FetchBlueprintTree(domain,
                                     topo_path,
                                     out["topologies"][mesh_topo_name]);

    BP_PLUGIN_INFO("GetMesh: done loading conduit data for "
                    << abs_meshname << " [domain " <<domain << "]");

    // check for mfem case
    Node &topo_data_node = out["topologies"][mesh_topo_name];


    bool has_bndry_topo = topo_data_node.has_child("boundary_topology");
    string bndry_topo_name;

    // if we have an mfem mesh, load extra data necessary to construct
    // a mfem mesh object
    if(has_bndry_topo)
    {
        BP_PLUGIN_INFO(abs_meshname << " has a boundary topology");
        // mfem case, we need to fetch the boundary topo
        bndry_topo_name = topo_data_node["boundary_topology"].as_string();

        BP_PLUGIN_INFO("boundary_topology is named: " << bndry_topo_name);
        if(bp_index_mesh_node["topologies"].has_child(bndry_topo_name))
        {
            const Node &bp_index_boundary_topo_node =bp_index_mesh_node["topologies"][bndry_topo_name];

            string bnd_topo_path =bp_index_boundary_topo_node["path"].as_string();

            string bnd_topo_coordset_name = bp_index_boundary_topo_node["coordset"].as_string();


            // sanity check:
            // make sure the boundary topo is defined on the same coordset as the mesh
            if(bnd_topo_coordset_name != coordset_name)
            {
                BP_PLUGIN_WARNING("warning: boundary topology "
                                   << "'" << bndry_topo_name << "'"
                                   << "coordset (" << bnd_topo_coordset_name << ")"
                                   << " does not match main topology coordset (" << coordset_name << ")");

            }


            BP_PLUGIN_INFO("boundary topology path " << bnd_topo_path);
            m_tree_cache->FetchBlueprintTree(domain,
                                             bnd_topo_path,
                                             out["topologies"][bndry_topo_name]);
        }
        else
        {

            BP_PLUGIN_WARNING("boundary_topology '"
                               << bndry_topo_name
                               <<  "' not found in blueprint index");
        }

    }

    bool has_grid_func  = topo_data_node.has_child("grid_function");

    if(has_grid_func)
    {
        BP_PLUGIN_INFO(abs_meshname << " has a mfem grid fuction");
        // mfem case, we need to fetch the grid func
        string gf_name = topo_data_node["grid_function"].as_string();
        BP_PLUGIN_INFO("field for grid_function for topology is named: " << gf_name);
        if(!bp_index_mesh_node["fields"].has_child(gf_name))
        {
            BP_PLUGIN_WARNING("grid_function '" << gf_name << "' for topology not found in fields");
        }
        else
        {
           string gf_path = bp_index_mesh_node["fields"][gf_name]["path"].as_string();

           BP_PLUGIN_INFO("grid function path " << gf_path);
           m_tree_cache->FetchBlueprintTree(domain,
                                            gf_path,
                                            out["fields"][gf_name]);
        }
    }

    // to construct an mfem mesh object,
    // we may also need the mesh and boundary attribute fields
    // here we look for them by name,
    // in the future, we could have them explicitly listed

    if( bp_index_mesh_node.has_child("fields") )
    {
        const Node &bp_index_fields_node = bp_index_mesh_node["fields"];
        NodeConstIterator flds_itr = bp_index_fields_node.children();

        string mesh_att_path;
        string bndry_att_path;

        while(flds_itr.has_next() &&
              // keep going until we have found the path for both
              // the mesh and boundary att fields
              (mesh_att_path.empty() || bndry_att_path.empty()) )
        {
            const Node &fld = flds_itr.next();
            string cld_name = flds_itr.name();
            // see if the name contains "attribute"
            std::size_t att_loc = cld_name.find("attribute");

            // check if it is defined on the mesh topo or boundary topo
            if(att_loc != std::string::npos)
            {
                // we found a contender, see if its topo is the main mesh topo or
                // the boundary topo
                string fld_topo_name = fld["topology"].as_string();

                if(fld_topo_name == mesh_topo_name)
                {
                    mesh_att_path = bp_index_fields_node[cld_name]["path"].as_string();
                }
                else if(fld_topo_name == bndry_topo_name)
                {
                    bndry_att_path = bp_index_fields_node[cld_name]["path"].as_string();
                }

            }
        }


        if(!mesh_att_path.empty())
        {
            // load the data for the mesh att tree
            m_tree_cache->FetchBlueprintTree(domain,
                                             mesh_att_path,
                                             out["fields/mesh_attribute"]);
        }

        if(!bndry_att_path.empty())
        {
            // load the data for the bndry att tree
            m_tree_cache->FetchBlueprintTree(domain,
                                             bndry_att_path,
                                             out["fields/boundary_attribute"]);
        }
    }

}

// ****************************************************************************
//  Method: avtBlueprintFileFormat::ReadBlueprintMesh
//
//  Purpose:
//      Reads a field for the given domain into the `out` conduit Node.
//
//
//  Programmer: Cyrus Harrison and Mark Miller
//  Creation:   Fri Aug 12 13:45:34 PDT 2016
//
// ****************************************************************************

void
avtBlueprintFileFormat::ReadBlueprintField(int domain,
                                           const string &abs_varname,
                                           Node &out)
{
    BP_PLUGIN_INFO("ReadBlueprintField: " << abs_varname << " [domain " << domain << "]");

    string abs_varname_str(abs_varname);
    // replace colons, etc
    abs_varname_str = sanitize_var_name(abs_varname_str);
    string abs_meshname = metadata->MeshForVar(abs_varname_str);

    BP_PLUGIN_INFO("field " << abs_varname << " is defined on mesh " << abs_meshname);

    string mesh_name;
    string mesh_topo_name;
    split_mesh_and_topo(std::string(abs_meshname),
                        mesh_name,
                        mesh_topo_name);

    string varname  = FileFunctions::Basename(abs_varname);

    if (!m_root_node["blueprint_index"].has_child(mesh_name))
    {
        BP_PLUGIN_EXCEPTION1(InvalidVariableException,
                             "mesh " << mesh_name << " not found in blueprint index");
    }

    if (!m_root_node["blueprint_index"][mesh_name]["fields"].has_child(varname))
    {
        // only throw an error if element_coloring is not in the name
        // element_coloring won't be in the index, its automatic.
        if(varname.find("element_coloring") == std::string::npos)
        {
            BP_PLUGIN_EXCEPTION1(InvalidVariableException,
                                 "field " << varname << " not found in blueprint index");
        }
    }

    Node &bp_index_field = m_root_node["blueprint_index"][mesh_name]["fields"][varname];
    BP_PLUGIN_INFO(bp_index_field.to_json());

    string topo_tag  = bp_index_field["topology"].as_string();

    string file_pattern = FileFunctions::Dirname(GetFilename()) +
                          string("/") +
                          m_root_node["file_pattern"].as_string();

    string tree_pattern = m_root_node["tree_pattern"].as_string();
    string data_path    = bp_index_field["path"].as_string();


    m_tree_cache->FetchBlueprintTree(domain,
                                     data_path,
                                     out);

    BP_PLUGIN_INFO("done loading conduit data for " << abs_varname << " [domain "<< domain << "]" );
}



// ****************************************************************************
// helper method used to add the meta data for a blueprint mesh.
// ****************************************************************************
void
avtBlueprintFileFormat::AddBlueprintMeshAndFieldMetadata(avtDatabaseMetaData *md,
                                                         string const &mesh_name,
                                                         const Node &n_mesh_info)
{


    Node verify_info;
    if(!blueprint::mesh::index::verify(n_mesh_info,verify_info))
    {
        BP_PLUGIN_INFO("Skipping mesh named \"" << mesh_name << "\"" << endl
                       << "blueprint::mesh::index::verify failed " << endl
                       << verify_info.to_json());
        return;
    }

    BP_PLUGIN_INFO("Adding mesh named \"" << mesh_name << "\"");

    const Node &n_topos = n_mesh_info["topologies"];

    // holds the names of the topos we found
    // so we can do a sanity check when we add
    // fields
    std::map<std::string,int> topo_dims;

    //
    // loop over topologies
    //
    NodeConstIterator topos_itr = n_topos.children();

    while(topos_itr.has_next())
    {
        avtMeshType mt = AVT_UNKNOWN_MESH;

        const Node &n_topo = topos_itr.next();
        string topo_name = topos_itr.name();

        string mesh_topo_name = mesh_name + "_" + topo_name;


        bool is_mfem_mesh = false;

        if(n_topo.has_child("grid_function"))
        {
            BP_PLUGIN_INFO(mesh_topo_name << " is an mfem mesh");
            is_mfem_mesh = true;
        }

        string coordset_name = n_topo["coordset"].as_string();

        BP_PLUGIN_INFO("topology " << topo_name << " references 'coordset' "
               << coordset_name);

        if (n_topo["type"].as_string() == "uniform")
        {
            mt = AVT_RECTILINEAR_MESH;
            BP_PLUGIN_INFO(mesh_topo_name << " topology is uniform ");

        }
        else if (n_topo["type"].as_string() == "rectilinear")
        {
            mt = AVT_RECTILINEAR_MESH;
            BP_PLUGIN_INFO(mesh_topo_name << " topology is rectilinear ");
        }
        else if (n_topo["type"].as_string() == "structured")
        {
            mt = AVT_CURVILINEAR_MESH;
            BP_PLUGIN_INFO(mesh_topo_name << " topology is structured ");
        }
        else if (n_topo["type"].as_string() == "unstructured")
        {
            mt = AVT_UNSTRUCTURED_MESH;
            BP_PLUGIN_INFO(mesh_topo_name << " topology is unstructured ");
        }
        else
        {

            BP_PLUGIN_INFO( "Encountered unknown topology type, \""
                            << n_topo["type"].as_string() << "\"" << endl
                            << "Skipping this mesh for now");
            return;
        }

        //
        // Get number of blocks
        //
        const Node &n_state = n_mesh_info["state"];
        int nblocks = n_state["number_of_domains"].to_int();

        BP_PLUGIN_INFO("number_of_domains: " << nblocks);

        //
        // Get the mesh spatial dimensions
        //

        BP_PLUGIN_INFO("finding coordinate system");

        const Node &n_coordsets = n_mesh_info["coordsets"];
        const Node &n_coords = n_coordsets[coordset_name];

        int ndims = n_coords["coord_system/axes"].number_of_children();
        topo_dims[topo_name] = ndims;

        BP_PLUGIN_INFO("coordinate system: "
                       << n_coords["coord_system"].to_json()
                       << " (ndims=" << ndims << ")");

        avtMeshMetaData *mmd = new avtMeshMetaData(mesh_topo_name,
                                                   nblocks,
                                                   0, 0, 0,
                                                   ndims, ndims, mt);
        mmd->LODs = 20;
        md->Add(mmd);

        if(is_mfem_mesh)
        {
            // if we have a mfem mesh, add extra element_color variable
            md->Add(new avtScalarMetaData(mesh_topo_name + "/element_coloring",
                                          mesh_topo_name,
                                          AVT_ZONECENT));

            m_mfem_mesh_map[mesh_topo_name] = true;
        }
        else
        {
            m_mfem_mesh_map[mesh_topo_name] = false;
        }
    }

    BP_PLUGIN_INFO("adding field vars for " <<  mesh_name);

    //
    // Now, handle any fields defined for this mesh
    //



    if(n_mesh_info.has_child("fields"))
    {

        NodeConstIterator fields_itr = n_mesh_info["fields"].children();

        while (fields_itr.has_next())
        {
            const Node &n_field = fields_itr.next();
            string varname = fields_itr.name();
            string var_topo_name = n_field["topology"].as_string();
            string var_mesh_name = mesh_name + "_" + var_topo_name;
            string varname_wmesh = var_mesh_name + "/" + varname;

            if (topo_dims[var_topo_name] == 0)
            {
                BP_PLUGIN_WARNING("Field \"" << varname_wmesh
                                  << "\" defined on unknown topology=\""
                                  << n_field["topology"].as_string());
                continue;
            }

            int ncomps = n_field["number_of_components"].to_int();
            int ndims = topo_dims[var_topo_name];

            // note: this logic is ok b/c the mfem case
            // (w/ basis instead of assoc) will always be nodal
            avtCentering cent = AVT_NODECENT;

            if (n_field.has_child("association") &&
                n_field["association"].as_string() == "element")
            {
                cent = AVT_ZONECENT;
            }
            else if(n_field.has_child("basis"))
            {
                // if any of the fields are mfem grid funcs, we may have to
                // treat the mesh as an mfem mesh, even if it lacks a basis func

                m_mfem_mesh_map[var_topo_name] = true;
            }

            if (ncomps == 1)
                md->Add(new avtScalarMetaData(varname_wmesh, var_mesh_name, cent));
            else if (ndims == 2 && ncomps == 2)
                md->Add(new avtVectorMetaData(varname_wmesh, var_mesh_name, cent, ncomps));
            else if (ndims == 2 && ncomps == 3)
                md->Add(new avtSymmetricTensorMetaData(varname_wmesh, var_mesh_name, cent, ncomps));
            else if (ndims == 2 && ncomps == 4)
                md->Add(new avtTensorMetaData(varname_wmesh, var_mesh_name, cent, ncomps));
            else if (ndims == 3 && ncomps == 3)
                md->Add(new avtVectorMetaData(varname_wmesh, var_mesh_name, cent, ncomps));
            else if (ndims == 3 && ncomps == 6)
                md->Add(new avtSymmetricTensorMetaData(varname_wmesh, var_mesh_name, cent, ncomps));
            else if (ndims == 3 && ncomps == 9)
                md->Add(new avtTensorMetaData(varname_wmesh, var_mesh_name, cent, ncomps));
            else
                md->Add(new avtArrayMetaData(varname_wmesh, var_mesh_name, cent, ncomps));
        }
    }
}

// ****************************************************************************
//  Method: is_hdf5_file()
//
//  Purpose:  Check if passed path is an HDF5 file.
//
//  Note: This is a helper that will be moved into conduit in the future.
//
//  Programmer: Cyrus Harrison,
//  Creation:  Fri Aug 24 14:01:50 PDT 2018
// ****************************************************************************
bool
is_hdf5_file(const std::string &file_path)
{
    // callback used for hdf5 error interface
    H5E_auto2_t  herr_func;
    // data container for hdf5 error interface callback
    void         *herr_func_client_data;

    // mute hdf5 error stack handlers
    H5Eget_auto(H5E_DEFAULT,
                &herr_func,
                &herr_func_client_data);

    H5Eset_auto(H5E_DEFAULT,
                NULL,
                NULL);

    bool res = false;
    // open the hdf5 file for read + write
    hid_t h5_file_id = H5Fopen(file_path.c_str(),
                               H5F_ACC_RDWR,
                               H5P_DEFAULT);

    if( h5_file_id >= 0)
    {
        res = true;
        H5Fclose(h5_file_id);
    }

    // restore hdf5 error stack handlers
    H5Eset_auto(H5E_DEFAULT,
                herr_func,
                herr_func_client_data);

    return res;
}

// ****************************************************************************
//  Method: avtBlueprintFileFormat::ReadRootFile
//
//  Purpose: Read contents of the root file
//
//  Programmer: cyrush
//  Creation:   Fri Dec  8 14:55:23 PST 2017
//
//  Modifications:
//    Cyrus Harrison, Fri Aug 24 14:01:50 PDT 2018
//    Add extra check for valid HDF5 file and allow plugin to be used if
//    any valid mesh index is found.
//
// ****************************************************************************
void
avtBlueprintFileFormat::ReadRootFile()
{
        //
        // Read root file using conduit::relay
        //

        string root_fname = GetFilename();

        BP_PLUGIN_INFO("Opening root file " << root_fname);

        int error = 0;

        // assume hdf5, but check for json file
        std::string root_protocol = "hdf5";
        std::string error_msg = "";

// only check on proc-0
#ifdef PARALLEL
        if (PAR_Rank() == 0)
#endif
        {

            char buff[5] = {0,0,0,0,0};

            // heuristic, if json, we expect to see "{" in the first 5 chars of the file.
            ifstream ifs;
            ifs.open(root_fname.c_str());
            if(!ifs.is_open())
            {
               error =1;
            }
            ifs.read((char *)buff,5);
            ifs.close();

            std::string test_str(buff);

            if(test_str.find("{") != std::string::npos)
            {
               root_protocol = "json";
            }

            // note: ".root" may be associated with with binary files
            // that are not hdf5

            // if we are using the hdf5 protocol, first check if this
            // is an hdf5 file, and if so -- fast fail if we don't see
            // the "file_pattern" entry

            if(root_protocol.find("hdf5") != std::string::npos)
            {
               //if(relay::io::is_hdf5_file(root_fname))
               if(is_hdf5_file(root_fname))
               {

                   // fast fail check for if this is a valid blueprint root file
                   // (if this path doesn't exist, relay will throw an exception)

                   try
                   {
                      Node n_read_check;
                      relay::io::load(root_fname + ":file_pattern",
                                      root_protocol,
                                      n_read_check);
                   }
                   catch(conduit::Error &e)
                   {
                       error_msg = e.message();
                       error = 1;
                   }
               }
               else
               {
                  error_msg = root_fname + " is not a valid HDF5 file.\n" +
                              " Cannot open with 'hdf5' protocol.";
                  error = 1;
               }
            }
        }

// check for error reading root file
#ifdef PARALLEL
        Node n_in, n_out;
        n_in.set(error);
        conduit::relay::mpi::sum_all_reduce(n_in,
                                            n_out,
                                            VISIT_MPI_COMM);

        error = n_out.to_int();
#endif
       if(error != 0)
       {
           BP_PLUGIN_EXCEPTION1(InvalidFilesException,
                                "Error reading root file: '" << root_fname<<"'. "
                                <<error_msg);
       }

#ifdef PARALLEL
        if (PAR_Rank() == 0)
        {
            relay::io::load(root_fname, root_protocol, m_root_node);
        }

        conduit::relay::mpi::broadcast_using_schema(m_root_node,
                                                    0,
                                                    VISIT_MPI_COMM);
#else
        relay::io::load(root_fname, root_protocol, m_root_node);
#endif

        if(!m_root_node.has_child("file_pattern"))
        {
            BP_PLUGIN_EXCEPTION1(InvalidFilesException,
                                 "Root file missing 'file_pattern'");
        }

        if(!m_root_node.has_child("blueprint_index"))
        {
            BP_PLUGIN_EXCEPTION1(InvalidFilesException,
                                 "Root file missing 'blueprint_index'");
        }

        NodeConstIterator itr = m_root_node["blueprint_index"].children();
        Node n_verify_info;

        bool any_index_ok = false;

        while(itr.has_next())
        {
            const Node &curr = itr.next();
            std::string mesh_name = itr.name();
            if( blueprint::mesh::index::verify(curr,
                                               n_verify_info[mesh_name]))
            {
                any_index_ok = true;

                debug5 << "Success: Mesh Blueprint Index verify for "
                      << mesh_name << std::endl;
            }
        }

        if(!any_index_ok)
        {
            BP_PLUGIN_EXCEPTION1(InvalidFilesException,
                                 "Failed to find a valid Mesh Blueprint Index\n"
                                 << n_verify_info.to_json());
        }

}

// ****************************************************************************
//  Method: avtBlueprintFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: harrison37 -- generated by xml2avt
//  Creation:   Wed Jun 15 16:25:28 PST 2016
//
// ****************************************************************************

void
avtBlueprintFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    BP_PLUGIN_INFO("Begin avtBlueprintFileFormat::PopulateDatabaseMetaData");

    int t_pop_md = visitTimer->StartTimer();

    // clear any mfem mesh mappings
    m_mfem_mesh_map.clear();

    try
    {
        ReadRootFile();

        //std::cout << "Root file contents" << endl << m_root_node.to_json() << std::endl;
        BP_PLUGIN_INFO("Root file contents" << endl << m_root_node.to_json());

        m_protocol = "hdf5";

        if(m_root_node.has_child("protocol"))
        {
            m_protocol = m_root_node["protocol/name"].as_string();
        }

        if (DebugStream::Level5())
        {
            m_root_node.to_json_stream(DebugStream::Stream5());
        }

        BP_PLUGIN_INFO("Using protocol: " << m_protocol);


        string file_pattern = m_root_node["file_pattern"].as_string();

        // if file_pattern isn't an abs path, it needs to be relative to the
        // the root file
        if(file_pattern[0] !=  VISIT_SLASH_STRING[0])
        {
            string root_fname = GetFilename();
            string root_dir = FileFunctions::Dirname(root_fname);
            file_pattern  = root_dir + string(VISIT_SLASH_STRING) + file_pattern;
        }

        m_tree_cache->SetFilePattern(file_pattern);
        m_tree_cache->SetTreePattern(m_root_node["tree_pattern"].as_string());
        m_tree_cache->SetNumberOfFiles(m_root_node["number_of_files"].to_int());
        m_tree_cache->SetNumberOfTrees(m_root_node["number_of_trees"].to_int());
        m_tree_cache->SetProtocol(m_protocol);

        NodeConstIterator itr = m_root_node["blueprint_index"].children();

        while (itr.has_next())
        {
            const Node &n = itr.next();
            AddBlueprintMeshAndFieldMetadata(metadata, itr.name(), n);
        }
    }
    catch(conduit::Error &e)
    {
        std::ostringstream err_oss;
        err_oss <<  "Conduit Exception in Blueprint Plugin "
                    << "Populate Database MetaData: " << endl
                    << e.message();
        EXCEPTION1(InvalidFilesException, err_oss.str());
    }


    visitTimer->StopTimer(t_pop_md,"PopulateDatabaseMetaData");
    BP_PLUGIN_INFO("End avtBlueprintFileFormat::PopulateDatabaseMetaData");
}


// ****************************************************************************
//  Method: avtBlueprintFileFormat::GetCycle
//
//  Purpose:
//      Returns if we have the current cycle.
//
//  Programmer: Cyrus Harrison
//  Creation:   Thu Aug 11 10:15:05 PDT 2016
//
// ****************************************************************************
int
avtBlueprintFileFormat::GetCycle()
{
    // VisIt doesn't support diff times / cycles for meshes in STMD
    // we loop over all meshes and return the first valid cycle
    NodeConstIterator itr = m_root_node["blueprint_index"].children();

    while(itr.has_next())
    {
        const Node &mesh = itr.next();
        if(mesh.has_path("state/cycle"))
        {
            return mesh["state/cycle"].to_int();
        }
    }

    return avtFileFormat::INVALID_CYCLE;
}


// ****************************************************************************
//  Method: avtBlueprintFileFormat::GetTime
//
//  Purpose:
//      Returns if we have the current cycle.
//
//  Programmer: Cyrus Harrison
//  Creation:   Thu Aug 11 10:15:05 PDT 2016
//
// ****************************************************************************
double
avtBlueprintFileFormat::GetTime()
{
    // VisIt doesn't support diff times / cycles for meshes in STMD
    // we loop over all meshes and return the first valid time

    NodeConstIterator itr = m_root_node["blueprint_index"].children();

    while(itr.has_next())
    {
        const Node &mesh = itr.next();
        if(mesh.has_path("state/time"))
        {
            return mesh["state/time"].to_double();
        }
    }

    return avtFileFormat::INVALID_TIME;
}


// ****************************************************************************
//  Method: avtBlueprintFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: harrison37 -- generated by xml2avt
//  Creation:   Wed Jun 15 16:25:28 PST 2016
//
// ****************************************************************************
vtkDataSet *
avtBlueprintFileFormat::GetMesh(int domain, const char *abs_meshname)
{
    BP_PLUGIN_INFO("GetMesh: "
                   << abs_meshname << " [domain " << domain << "]");

    // read mesh data into conduit tree
    Node data;
    string abs_meshname_str(abs_meshname);
    // reads a single mesh into a blueprint conforming output
    ReadBlueprintMesh(domain, abs_meshname_str, data);

    Node verify_info;
    if(!blueprint::mesh::verify(data, verify_info))
    {
        BP_PLUGIN_INFO("blueprint::mesh::verify failed for mesh "
                       << abs_meshname << " [domain " << domain << "]" << endl
                       << "Verify Info " << endl
                       << verify_info.to_json() << endl
                       << "Data Schema " << endl
                       << data.schema().to_json());

        BP_PLUGIN_INFO("warning: "
                       "avtBlueprintFileFormat::GetMesh returning NULL "
                       << abs_meshname
                       << " [domain " << domain << "]"
                       << " will be missing" << endl);
        // TODO: Should we throw an error instead of blanking the domain?
        return NULL;
    }

    BP_PLUGIN_INFO(data.schema().to_json());

    // prepare result vtk dataset
    vtkDataSet *res = NULL;

    string mesh_name;
    string mesh_topo_name;
    split_mesh_and_topo(std::string(abs_meshname),
                        mesh_name,
                        mesh_topo_name);

    BP_PLUGIN_INFO("mesh name and topology name: "
                    << mesh_name << " " << mesh_topo_name);


    // check for the mfem case
    if( m_mfem_mesh_map[mesh_topo_name] )
    {
        BP_PLUGIN_INFO("mesh  " << mesh_topo_name << " is a mfem mesh");
        // use mfem to refine and create a vtk dataset
        mfem::Mesh *mesh = avtBlueprintDataAdaptor::MFEM::MeshToMFEM(data);
        res = avtBlueprintDataAdaptor::MFEM::RefineMeshToVTK(mesh, m_selected_lod+1);

        // cleanup the mfem mesh
        delete mesh;
    }
    else
    {
        BP_PLUGIN_INFO("mesh  " << mesh_topo_name << " is a standard mesh");
        // construct a vtk dataset directly from blueprint data
        // in a conduit tree
        res = avtBlueprintDataAdaptor::VTK::MeshToVTK(data);
    }

    BP_PLUGIN_INFO("avtBlueprintFileFormat::GetMesh Done");
    return res;
}

// ****************************************************************************
//  Method: avtBlueprintFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: harrison37 -- generated by xml2avt
//  Creation:   Wed Jun 15 16:25:28 PST 2016
//
// ****************************************************************************

vtkDataArray *
avtBlueprintFileFormat::GetVar(int domain, const char *abs_varname)
{
    BP_PLUGIN_INFO("GetVar: " << abs_varname << " [domain " << domain << "]");

    vtkDataArray *res = NULL;

    string abs_varname_str(abs_varname);

    // check for special var "element_coloring"
    if(abs_varname_str.find("element_coloring") != std::string::npos)
    {
        // TODO: we currently have to replace colons, brackets, etc
        // to fetch from the mesh metadata, is there a standard helper
        // util in VisIt that can take care of this for us?
        string abs_meshname = metadata->MeshForVar(sanitize_var_name(abs_varname_str));

        // element coloring is generated from the mesh
        Node n_mesh;
        // read the mesh data
        ReadBlueprintMesh(domain, abs_meshname, n_mesh);

        Node verify_info;
        if(!blueprint::mesh::verify(n_mesh,verify_info))
        {
            BP_PLUGIN_INFO("blueprint::mesh::verify failed for field "
                           << abs_meshname << " [domain " << domain << "]" << endl
                           << "Verify Info " << endl
                           << verify_info.to_json() << endl
                           << "Data Schema " << endl
                           << n_mesh.schema().to_json());
            return NULL;
        }

        // create an mfem mesh
        mfem::Mesh *mesh = avtBlueprintDataAdaptor::MFEM::MeshToMFEM(n_mesh);
        // refine the coloring to a vtk data array
        res = avtBlueprintDataAdaptor::MFEM::RefineElementColoringToVTK(mesh,
                                                                        domain,
                                                                        m_selected_lod+1);
        // clean up the mfem mesh
        delete mesh;

        // return the coloring result
        return res;
    }

    // else, normal field case

    Node n_field;
    ReadBlueprintField(domain,abs_varname_str,n_field);

    Node verify_info;
    if(!blueprint::mesh::field::verify(n_field,verify_info))
    {
        BP_PLUGIN_INFO("blueprint::mesh::field::verify failed for field "
                       << abs_varname_str << " [domain " << domain << "]" << endl
                       << "Verify Info " << endl
                       << verify_info.to_json() << endl
                       << "Data Schema " << endl
                       << n_field.schema().to_json());
        return NULL;
    }

    // if we have an association, this is a standard field
    if(n_field.has_child("association"))
    {
        // low-order case, use vtk
        res = avtBlueprintDataAdaptor::VTK::FieldToVTK(n_field);
    }
    // if we have a basis, this field is actually an mfem grid function
    else if(n_field.has_child("basis"))
    {
        // TODO: we currently have to replace colons, brackets, etc
        // to fetch from the mesh metadata, is there a standard helper
        // util in VisIt that can take care of this for us?
        string abs_meshname = metadata->MeshForVar(sanitize_var_name(abs_varname_str));

        // the grid function needs the mesh in order to refine

        // read the mesh data
        Node n_mesh;
        ReadBlueprintMesh(domain, abs_meshname, n_mesh);

        Node verify_info;
        if(!blueprint::mesh::verify(n_mesh,verify_info))
        {
            BP_PLUGIN_INFO("blueprint::mesh::verify failed for field "
                           << abs_meshname << " [domain " << domain << "]" << endl
                           << "Verify Info " << endl
                           << verify_info.to_json() << endl
                           << "Data Schema " << endl
                           << n_mesh.schema().to_json());
            return NULL;
        }

        // create an mfem mesh
        mfem::Mesh *mesh = avtBlueprintDataAdaptor::MFEM::MeshToMFEM(n_mesh);

        // create the grid fuction
        mfem::GridFunction *gf =  avtBlueprintDataAdaptor::MFEM::FieldToMFEM(mesh,
                                                                             n_field);
        // refine the grid function into a vtk data array
        res =  avtBlueprintDataAdaptor::MFEM::RefineGridFunctionToVTK(mesh,
                                                                      gf,
                                                                      m_selected_lod+1);

        // cleanup mfem data
        delete gf;
        delete mesh;
    }

    return res;
}


// ****************************************************************************
//  Method: avtBlueprintFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: harrison37 -- generated by xml2avt
//  Creation:   Wed Jun 15 16:25:28 PST 2016
//
// ****************************************************************************

vtkDataArray *
avtBlueprintFileFormat::GetVectorVar(int domain, const char *varname)
{
    // vector vars can simply use the normal GetVar logic
    return GetVar(domain,varname);
}


// ****************************************************************************
//  Method: avtBlueprintFileFormat::RegisterDataSelections
//
//  Purpose:
//   Used to support avtResolutionSelection & capture the selected lod.
//
//  Arguments:
//     sels:    data selection list from the pipeline
//     applied: pipeline handshaking for handling data selections
//
//
// ****************************************************************************
void
avtBlueprintFileFormat::RegisterDataSelections(
                                const std::vector<avtDataSelection_p>& sels,
                                std::vector<bool>* applied)
{
    for(size_t i=0; i < sels.size(); ++i)
    {
        if(strcmp(sels[i]->GetType(), "avtResolutionSelection") == 0)
        {
            const avtResolutionSelection* sel =
                static_cast<const avtResolutionSelection*>(*sels[i]);
            this->m_selected_lod = sel->resolution();
            (*applied)[i] = true;
        }
    }
}
