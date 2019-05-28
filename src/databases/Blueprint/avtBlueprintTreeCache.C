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
// avtBlueprintTreeCache.C
// ************************************************************************* //

#include "avtBlueprintTreeCache.h"

//
// //-----------------------------------------------------------------------------
// // visit includes
// //-----------------------------------------------------------------------------
#include <StringHelpers.h>
#include <TimingsManager.h>

//-----------------------------------------------------------------------------
// std lib includes
//-----------------------------------------------------------------------------
#include <map>

//-----------------------------------------------------------------------------
// conduit includes
//-----------------------------------------------------------------------------
#include "conduit.hpp"
#include "conduit_relay.hpp"
#include "conduit_relay_io_hdf5.hpp"

//-----------------------------------------------------------------------------
// mfem includes
//-----------------------------------------------------------------------------
#include "mfem.hpp"

//-----------------------------------------------------------------------------
// bp visit plugin includes
//-----------------------------------------------------------------------------
#include "avtBlueprintLogging.h"

using std::string;
using namespace conduit;
using namespace mfem;

//----------------------------------------------------------------------------/
///
/// avtBlueprintTreeCache::CacheMap Interface
///
//----------------------------------------------------------------------------/
class avtBlueprintTreeCache::CacheMap
{
  public:
         CacheMap();
        ~CacheMap();

        Node    &FetchTree(int domain_id);
        Node    &FetchSidreMetaTree(int domain_id);

        hid_t    FetchHDF5Id(const std::string &file_path);

        uint64   TotalSize() const;
        uint64   TotalHDF5Ids() const;
        
        void     Release();

  private:
      std::map<int,Node>          m_nodes;
      std::map<int,Node>          m_sidre_nodes;
      std::map<std::string,hid_t> m_h5_ids;

};

//----------------------------------------------------------------------------/
///
/// avtBlueprintTreeCache::IO Methods
///
//----------------------------------------------------------------------------/

//----------------------------------------------------------------------------/
avtBlueprintTreeCache::CacheMap::CacheMap()
: m_nodes(),
  m_sidre_nodes(),
  m_h5_ids()
{}

//----------------------------------------------------------------------------/
avtBlueprintTreeCache::CacheMap::~CacheMap()
{
    Release();
}


//----------------------------------------------------------------------------/
void
avtBlueprintTreeCache::CacheMap::Release()
{
    // this will free all cached tree data
    m_nodes.clear();
    m_sidre_nodes.clear();
    
    // this will close all of the hdf5 file handles
    
    std::map<std::string, hid_t>::const_iterator itr;
    for(itr = m_h5_ids.begin(); itr != m_h5_ids.end(); ++itr)
    {
     
         hid_t h5_file_id = (*itr).second;
         // close the hdf5 file
         CHECK_HDF5_ERROR(H5Fclose(h5_file_id),
                           "Error closing HDF5 file handle: " << h5_file_id);
    }

    m_h5_ids.clear();

}

//----------------------------------------------------------------------------/
Node &
avtBlueprintTreeCache::CacheMap::FetchTree(int domain_id)
{
    return m_nodes[domain_id];
}

//----------------------------------------------------------------------------/
Node &
avtBlueprintTreeCache::CacheMap::FetchSidreMetaTree(int domain_id)
{
    return m_sidre_nodes[domain_id];
}

//----------------------------------------------------------------------------/
hid_t
avtBlueprintTreeCache::CacheMap::FetchHDF5Id(const std::string &file_path)
{
    hid_t h5_file_id = -1;
    if ( m_h5_ids.find(file_path) == m_h5_ids.end() )
    {
        // assume fetch_path points to a hdf5 dataset
        // open the hdf5 file for reading
        h5_file_id = H5Fopen(file_path.c_str(),
                             H5F_ACC_RDONLY,
                             H5P_DEFAULT);
        CHECK_HDF5_ERROR(h5_file_id,
                         "Error opening HDF5 file for reading: "  << file_path);
        BP_PLUGIN_INFO("opened " << file_path << "  hdf5 id = " << h5_file_id);
        m_h5_ids[file_path] = h5_file_id;
    }
    else
    {
        h5_file_id =  m_h5_ids[file_path];
    }

    return h5_file_id;
}


//----------------------------------------------------------------------------/
uint64
avtBlueprintTreeCache::CacheMap::TotalSize() const
{
    uint64 res = 0;
    // count cached blueprint data
    std::map<int, Node>::const_iterator itr;
    for(itr  = m_nodes.begin(); 
        itr != m_nodes.end();
        itr++)
    {
        res += itr->second.total_bytes_allocated();
    }
    
    // also count cached sidre metadata
    for(itr  = m_sidre_nodes.begin(); 
        itr != m_sidre_nodes.end();
        itr++)
    {
        res += itr->second.total_bytes_allocated();
    }

    return res;
}

//----------------------------------------------------------------------------/
uint64
avtBlueprintTreeCache::CacheMap::TotalHDF5Ids() const
{
    return (uint64) m_h5_ids.size();
}

//----------------------------------------------------------------------------/
///
/// avtBlueprintTreeCache::IO Interface
///
//----------------------------------------------------------------------------/


class avtBlueprintTreeCache::IO
{
public:
    
    //-----------------------------------------------------------------------//
    // Generic Entry point (handles all protocols)
    //-----------------------------------------------------------------------//
    
    static void LoadBlueprintTree(avtBlueprintTreeCache &tcache,
                                  int tree_id,
                                  const std::string &protocol,
                                  const std::string &file_path,
                                  const std::string &tree_root,
                                  const std::string &tree_path,
                                  conduit::Node &out);

    //-----------------------------------------------------------------------//
    static bool ReadHDF5Slab(hid_t h5_file_id,
                             const std::string &fetch_path,
                             const DataType &dtype,
                             void *data_ptr);

private:
    //-----------------------------------------------------------------------//
    // Sidre Specific Read Helpers
    //-----------------------------------------------------------------------//

    //-----------------------------------------------------------------------//
    static void LoadSidreView(conduit::Node &sidre_meta_view,
                              avtBlueprintTreeCache &tcache,
                              int tree_id,
                              const std::string &tree_root,
                              const std::string &view_path,
                              conduit::Node &out);

    //-----------------------------------------------------------------------//
    static void LoadSidreGroup(conduit::Node &sidre_meta,
                               avtBlueprintTreeCache &tcache,
                               int tree_id,
                               const std::string &tree_root,
                               const std::string &group_path,
                               conduit::Node &out);

    //-----------------------------------------------------------------------//
    static void LoadSidreTree(conduit::Node &sidre_meta,
                              avtBlueprintTreeCache &tcache,
                              int tree_id,
                              const std::string &tree_root,
                              const std::string &tree_path,
                              const std::string &curr_path,
                              conduit::Node &out);

    //-----------------------------------------------------------------------//
    static std::string CreateSidreMetaGroupPath(const std::string &tree_path);


    //-----------------------------------------------------------------------//
    static std::string CreateSidreMetaViewPath(const std::string &tree_path);
    



};

//---------------------------------------------------------------------------//
// Helper that expands a sidre groups's meta path to the expected hdf5 path 
//---------------------------------------------------------------------------//
std::string
avtBlueprintTreeCache::IO::CreateSidreMetaGroupPath(const std::string &tree_path)
{
    // sidre/groups/ga/groups/gb/groups/gc/.../groups/gn
    
    std::ostringstream oss;
    
    std::string t_path  = tree_path;
    std::string t_curr;
    std::string t_next;

    while(t_path != "")
    {
        conduit::utils::split_path(t_path,
                                   t_curr,
                                   t_next);
                                   
        oss << "groups/" << t_curr;
        if( t_next != "")
        {
            oss << "/";
        }

        t_path = t_next;
   
    }

    return oss.str();

}

//---------------------------------------------------------------------------//
// Helper that expands a sidre view's meta path to the expected hdf5 path 
//---------------------------------------------------------------------------//
std::string
avtBlueprintTreeCache::IO::CreateSidreMetaViewPath(const std::string &tree_path)
{
    // sidre/groups/ga/groups/gb/groups/gc/.../views/gn
    
    std::ostringstream oss;
    
    std::string t_path  = tree_path;
    std::string t_curr;
    std::string t_next;

    while(t_path != "")
    {
        conduit::utils::split_path(t_path,
                                   t_curr,
                                   t_next);
        if(t_next == "")
        {
            oss << "/views/" << t_curr;
        }
        else
        {
            oss << "groups/" << t_curr << "/";
        }

        t_path = t_next;
   
    }

    return oss.str();

   
    std::string curr,next;
    
    
}



//---------------------------------------------------------------------------//
// Main Blueprint IO Load Method (HDF5 Variant)
//---------------------------------------------------------------------------//
void
avtBlueprintTreeCache::IO::LoadBlueprintTree(avtBlueprintTreeCache &tree_cache,
                                             int tree_id,
                                             const std::string &protocol,
                                             const std::string &file_path,
                                             const std::string &tree_root,
                                             const std::string &tree_path,
                                             Node &out)
{
    int t_load_bp_tree = visitTimer->StartTimer();
    
    // hid_t h5_file_id = tree_cache.Cache().FetchHDF5Id(file_path);
    
    // non sidre case
    if(protocol.find("sidre") == std::string::npos)
    {
        std::string fetch_path = tree_root + tree_path;
        BP_PLUGIN_INFO("tree cache read " 
                        << "domain " << tree_id 
                        << " : "
                        << fetch_path);
        int t_tree_read = visitTimer->StartTimer();
        
        tree_cache.Read(tree_id,fetch_path,out);
        visitTimer->StopTimer(t_tree_read, "tree read");
    }
    else if( protocol == "sidre_hdf5" )
    {
        Node &sidre_meta = tree_cache.Cache().FetchSidreMetaTree(tree_id);
        
        int t_sidre_meta_read = visitTimer->StartTimer();
       
        // this path will either be a sidre group or a sidre view
        // check if either exists cached
        BP_PLUGIN_INFO("fetch sidre tree: "<< tree_path);
      
        std::string sidre_mtree_view  = CreateSidreMetaViewPath(tree_path);
        std::string sidre_mtree_group = CreateSidreMetaGroupPath(tree_path);
        
        if( !sidre_meta.has_path(sidre_mtree_group) ||
            !sidre_meta.has_path(sidre_mtree_view) )
        {
    
            // check to see if we have a group or view
            if( tree_cache.HasPath(tree_id,
                                   tree_root + "/sidre/" + sidre_mtree_group) )
            {
                   // we have a group, read the meta data
                   tree_cache.Read(tree_id,
                                   tree_root + "/sidre/" + sidre_mtree_group,
                                   sidre_meta[sidre_mtree_group]);

            }
            else if( tree_cache.HasPath(tree_id,
                                        tree_root + "/sidre/" + sidre_mtree_view) )
            {
                   // we have a view, read the meta data
                   tree_cache.Read(tree_id,
                                   tree_root + "/sidre/" + sidre_mtree_view,
                                   sidre_meta[sidre_mtree_view]);
            }
            else
            {
                BP_PLUGIN_EXCEPTION1( InvalidVariableException, 
                                     "Failed to read tree path: " << tree_path
                                     << std::endl
                                     << "Expected to find Sidre Group: "
                                     << tree_root << "/sidre/" << sidre_mtree_group
                                     << " or "
                                     << "Sidre View: " 
                                     << tree_root << "/sidre/" << sidre_mtree_view);
            }

            visitTimer->StopTimer(t_sidre_meta_read, "fetch sidre meta data from hdf5");
        }
        else
        {
            visitTimer->StopTimer(t_sidre_meta_read, "fetch sidre meta data from cache");
        }
        
        
        int t_sidre_tree = visitTimer->StartTimer();
        
        // start a top level traversal 
        LoadSidreTree(sidre_meta,
                      tree_cache,
                      tree_id,
                      tree_root,
                      tree_path,
                      "",
                      out);
        visitTimer->StopTimer(t_sidre_tree, "LoadSidreTree");
    }
    else
    {
         BP_PLUGIN_EXCEPTION1( InvalidVariableException,
                               "unknown protocol" << protocol);
    }

    visitTimer->StopTimer(t_load_bp_tree, "IO::LoadBlueprintTree");
}



//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
//
// Helpers for reading sidre style hdf5 data. 
//
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//


//----------------------------------------------------------------------------/
// TODO CONST FOR INPUT 
void
avtBlueprintTreeCache::IO::LoadSidreView(Node &sidre_meta_view,
                                         avtBlueprintTreeCache &tree_cache,
                                         int tree_id,
                                         const std::string &tree_root,
                                         const std::string &view_path,
                                         Node &out)
{
    // view load cases:
    //   the view is a scalar or string
    //     simply copy the "value" from the meta view
    //
    //   the view is attached to a buffer
    //     in this case we need to get the info about the buffer the view is 
    //     attached to and read the proper slab of that buffer's hdf5 dataset
    //     into a new compact node.
    //
    //   the view is has external data
    //     for this case we can follow the "tree_path" in the sidre external
    //     data tree, and fetch the hdf5 dataset that was written there.
    //
    string view_state = sidre_meta_view["state"].as_string();

    if( view_state == "STRING")
    {
        BP_PLUGIN_INFO("loading " << view_path << " as sidre string view");
        out.set(sidre_meta_view["value"]);
    }
    else if(view_state == "SCALAR")
    {
        BP_PLUGIN_INFO("loading " << view_path << " as sidre scalar view");
        out.set(sidre_meta_view["value"]);
    }
    else if( view_state == "BUFFER" )
    {
        BP_PLUGIN_INFO("loading " << view_path << " as sidre view linked to a buffer");
        // we need to fetch the buffer
        int buffer_id = sidre_meta_view["buffer_id"].to_int();

        std::ostringstream buffer_fetch_path_oss;
        buffer_fetch_path_oss << tree_root  
                       << "sidre/buffers/buffer_id_" << buffer_id;
               
        // buffer data path
        std::string buffer_data_fetch_path   = buffer_fetch_path_oss.str() + "/data";
        // we also need the buffer's schema 
        std::string buffer_schema_fetch_path = buffer_fetch_path_oss.str() + "/schema";

        Node n_buffer_schema_str;
        
        // TODO: We aren't caching this, but the final result is cached, not sure
        // we need to cache.
        
        tree_cache.Read(tree_id,buffer_schema_fetch_path,n_buffer_schema_str);
       
        string buffer_schema_str = n_buffer_schema_str.as_string();
        Schema buffer_schema(buffer_schema_str);

        BP_PLUGIN_INFO("sidre buffer schema: " << buffer_schema.to_json());
        BP_PLUGIN_INFO("sidre buffer data path " << buffer_data_fetch_path);


        string view_schema_str =  sidre_meta_view["schema"].as_string();
        // create the schema we want for this view
        // it describes how the view relates to the buffer in the hdf5 file
        Schema view_schema(view_schema_str);
        BP_PLUGIN_INFO("sidre view schema: " << view_schema.to_json());
        
        // if the schema isn't compact, or if we are reading 
        // less elements than the entire buffer, 
        // we need to read a subset of the hdf5 dataset
      
        if(   !view_schema.is_compact() || 
            ( view_schema.dtype().number_of_elements() <
              buffer_schema.dtype().number_of_elements() )
          )
        {
            BP_PLUGIN_INFO("Sidre View from Buffer Slab Fetch Case");
            //
            // Create a compact schema to describe our desired output data
            //
            Schema view_schema_compact;
            view_schema.compact_to(view_schema_compact);
            // setup and allocate the output node
            out.set(view_schema_compact);

            // ---------------------------------------------------------------
            // BUFFER-SLAB FETCH
            // ---------------------------------------------------------------
            //
            // we can use hdf5 slab fetch if the the dtype.id() of the buffer
            // and the view are the same. 
            // 
            //  otherwise, we will have to fetch the entire buffer since
            //  hdf5 doesn't support byte level striding.

            if(
                tree_cache.Read(tree_id,
                                buffer_data_fetch_path,
                                view_schema.dtype(),
                                out)
                )
            {
                BP_PLUGIN_INFO("Sidre View from Buffer Slab Fetch Case Failed");
                // ---------------------------------------------------------------
                // Fall back to Non BUFFER-SLAB FETCH
                // ---------------------------------------------------------------
                // this reads the entire buffer to get the proper subset
                Node n_buff;
                Node n_view;

                tree_cache.Read(tree_id,
                                buffer_data_fetch_path,
                                n_buff);
                // create our view on the buffer
                n_view.set_external(view_schema,n_buff.data_ptr());
                // compact the view to our output
                n_view.compact_to(out);
            }
            else
            {
                BP_PLUGIN_INFO("Sidre View from Buffer Slab Fetch Case Successful");
            }
        }
        else
        {
            
            tree_cache.Read(tree_id,
                            buffer_data_fetch_path,
                            out);
        }
    }
    else if( view_state == "EXTERNAL" )
    {
        BP_PLUGIN_INFO("loading " << view_path << " as sidre external view");

        std::string fetch_path = tree_root + "sidre/external/" + view_path;

        

        BP_PLUGIN_INFO("relay:io::hdf5_read " 
                       << "domain " << tree_id 
                       << " : "
                       << fetch_path);
        
        tree_cache.Read(tree_id,
                        fetch_path,
                        out);

    }
    else
    {
        BP_PLUGIN_EXCEPTION1( InvalidVariableException,
                              "unsupported sidre view state: " << view_state );
    }
}


//----------------------------------------------------------------------------/
void
avtBlueprintTreeCache::IO::LoadSidreGroup(Node &sidre_meta,
                                          avtBlueprintTreeCache &tree_cache,
                                          int tree_id,
                                          const std::string &tree_root,
                                          const std::string &group_path,
                                          Node &out)
{
    // load this group's children groups and views
    NodeIterator g_itr = sidre_meta["groups"].children();
    while(g_itr.has_next())
    {
        Node &g = g_itr.next();
        string g_name = g_itr.name();
        BP_PLUGIN_INFO("loading " << group_path << g_name << " as group");
        std::string cld_path = group_path + g_name;
        LoadSidreGroup(g,
                       tree_cache,
                       tree_id,
                       tree_root,
                       cld_path + "/",
                       out[g_name]);
    }    
    
    NodeIterator v_itr = sidre_meta["views"].children();
    while(v_itr.has_next())
    {
        Node &v = v_itr.next();
        string v_name = v_itr.name();
        BP_PLUGIN_INFO("loading " << group_path << v_name << " as view");
        std::string cld_path = group_path + v_name;
         LoadSidreView(v,
                       tree_cache,
                       tree_id,
                       tree_root,
                       cld_path,
                       out[v_name]);
        
    }
}


//----------------------------------------------------------------------------/
void
avtBlueprintTreeCache::IO::LoadSidreTree(Node &sidre_meta,
                                         avtBlueprintTreeCache &tree_cache,
                                         int tree_id,
                                         const std::string &tree_root,
                                         const std::string &tree_path,
                                         const std::string &curr_path,
                                         Node &out)
{
    // we want to pull out a sub-tree of the sidre group hierarchy 
    //
    // descend down to "tree_path" in sidre meta
    
    string tree_curr;
    string tree_next;
    conduit::utils::split_path(tree_path,tree_curr,tree_next);
    
    if( sidre_meta["groups"].has_path(tree_curr) )
    {
        BP_PLUGIN_INFO(curr_path << tree_curr << " is a group");
        if(tree_next.size() == 0)
        {
            LoadSidreGroup(sidre_meta["groups"][tree_curr],
                           tree_cache,
                           tree_id,
                           tree_root,
                           curr_path + tree_curr  + "/",
                           out);
        }
        else // keep descending 
        {
            LoadSidreTree(sidre_meta["groups"][tree_curr],
                          tree_cache,
                          tree_id,
                          tree_root,
                          tree_next,
                          curr_path + tree_curr  + "/",
                          out);
        }
    }
    else if( sidre_meta["view"].has_path(tree_curr) )
    {
        BP_PLUGIN_INFO(curr_path << tree_curr << " is a view");
        if(tree_next.size() != 0)
        {
            BP_PLUGIN_EXCEPTION1( InvalidVariableException,
                                  "path extends beyond sidre view (views are leaves)");
        }
        else
        {
            LoadSidreView(sidre_meta["view"][tree_curr],
                          tree_cache,
                          tree_id,
                          tree_root,
                          curr_path + tree_curr  + "/",
                          out);
        }
    }
    else
    {
        BP_PLUGIN_EXCEPTION1( InvalidVariableException,
                              "sidre tree path " << tree_curr << " does not exist");
    }
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
//
// Helper for reading HDF5 data 
// (for cases beyond what conduit::relay provides)
//
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//


// ****************************************************************************
bool
avtBlueprintTreeCache::IO::ReadHDF5Slab(hid_t h5_file_id,
                                        const std::string &fetch_path,
                                        const DataType &dtype,
                                        void *data_ptr)
{
    // open the dataset
    hid_t h5_dset_id = H5Dopen( h5_file_id, fetch_path.c_str(),H5P_DEFAULT);

    CHECK_HDF5_ERROR(h5_dset_id,
                     "Error opening HDF5 dataset at: " << fetch_path);


    // get info about the dataset
    hid_t h5_dspace_id = H5Dget_space(h5_dset_id);
    CHECK_HDF5_ERROR(h5_dspace_id,
                     "Error reading HDF5 Dataspace: " << h5_dset_id);

    // check for empty case
    if(H5Sget_simple_extent_type(h5_dspace_id) == H5S_NULL)
    {
        // we have an error, but to try to clean up the hdf5 handles
        // before we issue the error.
        
        CHECK_HDF5_ERROR(H5Sclose(h5_dspace_id),
                          "Error closing HDF5 data space: " << fetch_path);

        CHECK_HDF5_ERROR(H5Dclose(h5_dset_id),
                          "Error closing HDF5 dataset: " << fetch_path);
        BP_PLUGIN_EXCEPTION1( InvalidVariableException,
                              "Can't slab fetch from an empty hdf5 data set.");
    }

    hid_t h5_dtype_id  = H5Dget_type(h5_dset_id);

    CHECK_HDF5_ERROR(h5_dtype_id,
                     "Error reading HDF5 Datatype: "
                     << h5_dset_id);

    // ounds check  (check that we are fetching a subset of the elems)
    index_t  h5_nelems = H5Sget_simple_extent_npoints(h5_dspace_id);
    if( dtype.number_of_elements() > h5_nelems)
    {
        // we have an error, but to try to clean up the hdf5 handles
        // before we issue the error.
        
        CHECK_HDF5_ERROR(H5Sclose(h5_dspace_id),
                          "Error closing HDF5 data space: " << fetch_path);

        CHECK_HDF5_ERROR(H5Dclose(h5_dset_id),
                          "Error closing HDF5 dataset: " << fetch_path);

        BP_PLUGIN_EXCEPTION1( InvalidVariableException,
                              "Can't slab fetch a buffer larger than the source"
                              " hdf5 data set. Requested number of elements" 
                              << dtype.number_of_elements()
                              << " hdf5 dataset number of elements" << h5_nelems);
    }


    // we need to compute an offset, stride, and element bytes
    // that will work for reading in the general case
    // right now we assume the dest type of data and the hdf5 datasets
    // data type are compatible  
    
    // conduit's offsets, strides, are all in terms of bytes
    // hdf5's are in terms of elements
    
    // what we really want is a way to read bytes from the hdf5 dset with
    // out any type conversion, but that doesn't exist.

    // general support would include reading a a view of one type that
    //  points to a buffer of another
    // (for example a view of doubles that is defined on a buffer of bytes)

    // but hdf5 doens't support slab fetch across datatypes
    // so for now we make sure the datatype is consistent. 

    // TODO: Note we now have access the buffer datatype outside of this
    //  method, so we may be able to refactor at a higher level to catch
    // this case.  
    
    DataType h5_dt = conduit::relay::io::hdf5_dtype_to_conduit_dtype(h5_dtype_id,1);
    
    if( h5_dt.id() != dtype.id() )
    {
        // cleanup hdf5 handles 
        
        // close the dspace handle
        CHECK_HDF5_ERROR(H5Sclose(h5_dspace_id),
                          "Error closing HDF5 data space: " << fetch_path);

        // close the dataset
        CHECK_HDF5_ERROR(H5Dclose(h5_dset_id),
                           "Error closing HDF5 dataset: " << fetch_path);
        
        // close the hdf5 file
        CHECK_HDF5_ERROR(H5Fclose(h5_file_id),
                         "Error closing HDF5 file: " << fetch_path);
                         
        BP_PLUGIN_INFO("Cannot fetch hdf5 slab of buffer and view are"
                        "different data types.")
        return false;
    }
    
    /// adj endianness if necessary 

    if(!h5_dt.endianness_matches_machine())
    {
        // if they don't match, modify the dt
        // and get the proper hdf5 data type handle
        h5_dt.set_endianness(Endianness::machine_default());

        // clean up our old handle
        CHECK_HDF5_ERROR(H5Tclose(h5_dtype_id),
                         "Error closing HDF5 Datatype: " << h5_dtype_id);

        // get ref to standard variant of this dtype
        h5_dtype_id  = conduit::relay::io::conduit_dtype_to_hdf5_dtype(h5_dt);

        CHECK_HDF5_ERROR(h5_dtype_id,
                         "Error creating HDF5 Datatype");

        // copy this handle, b/c clean up code later will close it
        h5_dtype_id  = H5Tcopy(h5_dtype_id);
        CHECK_HDF5_ERROR(h5_dtype_id,
                         "Error copying HDF5 Datatype: " << h5_dtype_id);
    }

    hid_t h5_status    = 0;

    hsize_t elem_bytes = dtype.element_bytes();
    hsize_t offset  = dtype.offset() / elem_bytes; // in bytes, convert to elems
    hsize_t stride  = dtype.stride() / elem_bytes; // in bytes, convert to elems
    hsize_t num_ele = dtype.number_of_elements();
    
    BP_PLUGIN_INFO("slab dtype: " << dtype.to_json());
    
    BP_PLUGIN_INFO("hdf5 slab: "  <<
                   " element_offset: " << offset <<
                   " element_stride: " << stride <<
                   " number_of_elements: " << num_ele);
    
    h5_status = H5Sselect_hyperslab(h5_dspace_id,
                                    H5S_SELECT_SET,
                                    &offset,
                                    &stride,
                                    &num_ele,
                                    0); // 0 here means NULL pointers; HDF5 *knows* dimension is 1
    // check subset sel
    CHECK_HDF5_ERROR(h5_status,
                      "Error selecting hyper slab from HDF5 dataspace: " << h5_dspace_id);

    hid_t h5_dspace_compact_id = H5Screate_simple(1,
                                                  &num_ele,
                                                  NULL);

    CHECK_HDF5_ERROR(h5_dspace_id,"Failed to create HDF5 Dataspace");

    h5_status = H5Dread(h5_dset_id, // data set id
                        h5_dtype_id, // memory type id  // use same data type
                        h5_dspace_compact_id,  // memory space id ...
                        h5_dspace_id, // file space id
                        H5P_DEFAULT,
                        data_ptr);

    // check read
    CHECK_HDF5_ERROR(h5_status,
                      "Error reading bytes from HDF5 dataset: " << h5_dset_id);

    // close the data space 
    CHECK_HDF5_ERROR(H5Sclose(h5_dspace_id),
                      "Error closing HDF5 data space: " << fetch_path);

    // close the compact data space 
    CHECK_HDF5_ERROR(H5Sclose(h5_dspace_compact_id),
                      "Error closing HDF5 compact memory data space" << fetch_path);

    // close the data type
    CHECK_HDF5_ERROR(H5Tclose(h5_dtype_id),
                     "Error closing HDF5 Datatype: " << h5_dtype_id);

    // close the dataset
    CHECK_HDF5_ERROR(H5Dclose(h5_dset_id),
                      "Error closing HDF5 dataset: " << fetch_path);

    return true;
}


//----------------------------------------------------------------------------/
///
/// avtBlueprintTreeCache Methods
///
//----------------------------------------------------------------------------/


//----------------------------------------------------------------------------/
avtBlueprintTreeCache::avtBlueprintTreeCache()
: m_file_pattern(""),
  m_tree_pattern(""),
  m_num_files(0),
  m_num_trees(0),
  m_protocol("hdf5"),
  m_cache_map(NULL)
{
    m_cache_map = new CacheMap();
    
}

//----------------------------------------------------------------------------/
avtBlueprintTreeCache::~avtBlueprintTreeCache()
{
    delete m_cache_map;
}

//----------------------------------------------------------------------------/
void
avtBlueprintTreeCache::Release()
{
    m_cache_map->Release();
}

//----------------------------------------------------------------------------/
std::string
avtBlueprintTreeCache::Expand(const std::string pattern,
                              int idx) const
{
    //
    // Note: This currently only handles format strings :
    // "%05d" "%06d" "%07d"
    //

    std::size_t idx_pattern = pattern.find("%05d");

    if(idx_pattern != std::string::npos)
    {
        char buff[16];
        SNPRINTF(buff,16,"%05d",idx);
        return StringHelpers::Replace(pattern,
                                      "%05d",
                                      std::string(buff));
    }

    idx_pattern = pattern.find("%06d");

    if(idx_pattern != std::string::npos)
    {
        char buff[16];
        SNPRINTF(buff,16,"%06d",idx);    
        return StringHelpers::Replace(pattern,
                                      "%06d",
                                      std::string(buff));
    }

    idx_pattern = pattern.find("%07d");

    if(idx_pattern != std::string::npos)
    {
        char buff[16];
        SNPRINTF(buff,16,"%07d",idx);    
        return StringHelpers::Replace(pattern,
                                      "%07d",
                                      std::string(buff));
    }
    return pattern;
}


//----------------------------------------------------------------------------/
void
avtBlueprintTreeCache::SetFilePattern(const std::string &file_pattern)
{
    m_file_pattern = file_pattern;
}

void
avtBlueprintTreeCache::SetTreePattern(const std::string &tree_pattern)
{
    m_tree_pattern = tree_pattern;
}

//-------------------------------------------------------------------//
void
avtBlueprintTreeCache::SetNumberOfFiles(int num_files)
{
    m_num_files = num_files;
}

//-------------------------------------------------------------------//
void
avtBlueprintTreeCache::SetNumberOfTrees(int num_trees)
{
    m_num_trees = num_trees;
}

//-------------------------------------------------------------------//
void
avtBlueprintTreeCache::SetProtocol(const std::string &protocol)
{
    m_protocol = protocol;
}


//-------------------------------------------------------------------//
std::string
avtBlueprintTreeCache::GenerateFilePath(int tree_id) const
{
    // for now, we only support 1 tree per file.
    int file_id = tree_id;
    return Expand(m_file_pattern,file_id);
}

//-------------------------------------------------------------------//
std::string
avtBlueprintTreeCache::GenerateTreePath(int tree_id) const
{
    // the tree path should always end in a /
    std::string res = Expand(m_tree_pattern,tree_id);
    if( (res.size() > 0) && (res[res.size()-1] != '/') )
    {
        res += "/";
    }
    return res;
}


//---------------------------------------------------------------------------//
void
avtBlueprintTreeCache::FetchBlueprintTree(int tree_id, 
                                          const std::string &sub_tree_path,
                                          conduit::Node &out)
{
    int t_fetch_tree = visitTimer->StartTimer();
    
    Node &cache = m_cache_map->FetchTree(tree_id);

    if( cache.has_path(sub_tree_path) )
    {
        out.set_external(cache[sub_tree_path]);
    }
    else
    {
        Node &cache_sub_tree = cache[sub_tree_path];
        IO::LoadBlueprintTree(*this,
                              tree_id,
                              m_protocol,
                              GenerateFilePath(tree_id),
                              GenerateTreePath(tree_id),
                              sub_tree_path,
                              cache_sub_tree);

        out.set_external(cache_sub_tree);
    }

    BP_PLUGIN_INFO("Blueprint Tree Cache Size (Post fetch of"
                   << " domain: " << tree_id
                   << " sub path: " << sub_tree_path << ") = "
                   << m_cache_map->TotalSize() << " bytes");
    visitTimer->StopTimer(t_fetch_tree, "FetchBlueprintTree");

}


//---------------------------------------------------------------------------//
avtBlueprintTreeCache::CacheMap &
avtBlueprintTreeCache::Cache()
{
    return *m_cache_map;
}


//-----------------------------------------------------------------------//
void
avtBlueprintTreeCache::Read(int tree_id,
                            const std::string &path,
                            conduit::Node &out)
{
    // switch on protocol
    if(m_protocol.find("hdf5") != std::string::npos)
    {
        hid_t h5_file_id = m_cache_map->FetchHDF5Id(GenerateFilePath(tree_id));
        conduit::relay::io::hdf5_read(h5_file_id,path,out);
    }
    else
    {
        // TODO proper sub tree, or read and cache at higher lvl
        Node n_full;
        conduit::relay::io::load(GenerateFilePath(tree_id),m_protocol,n_full);
        out = n_full[path];
    }
}


    
//-----------------------------------------------------------------------//
// Note: Only used by sidre_hdf5
bool
avtBlueprintTreeCache::HasPath(int tree_id,
                               const std::string &path)
{
    hid_t h5_file_id = m_cache_map->FetchHDF5Id(GenerateFilePath(tree_id));
    return conduit::relay::io::hdf5_has_path(h5_file_id,path);
}

//-----------------------------------------------------------------------//
// Note: Only used by sidre_hdf5
bool
avtBlueprintTreeCache::Read(int tree_id,
                            const std::string &path,
                            const conduit::DataType &dtype,
                            conduit::Node &out)
{
    hid_t h5_file_id = m_cache_map->FetchHDF5Id(GenerateFilePath(tree_id));
    
    void *data_ptr = out.data_ptr();
    bool res = IO::ReadHDF5Slab(h5_file_id,
                                path,
                                dtype,
                                data_ptr);
    
    return res;
}


