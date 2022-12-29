// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
// avtBlueprintTreeCache.C
// ************************************************************************* //

#ifndef AVT_BLUEPRINT_TREE_CACHE_H
#define AVT_BLUEPRINT_TREE_CACHE_H

#include <string>
#include "conduit.hpp"

//---------------------------------------------------------------------------//
//
//  avtBlueprintTreeCache Class
//  Encapsulates common logic for fetching and caching data from file + hdf5 
//  paths.
//
//---------------------------------------------------------------------------//
class avtBlueprintTreeCache
{
public:
    
    class IO;
    class CacheMap;
    
    //-----------------------------------------------------------------------//
    avtBlueprintTreeCache();

    //-----------------------------------------------------------------------//
   ~avtBlueprintTreeCache();

    //-----------------------------------------------------------------------//
    void        Release();

    //-----------------------------------------------------------------------//
    void        SetFilePattern(const std::string &file_pattern);

    //-----------------------------------------------------------------------//
    void        SetTreePattern(const std::string &tree_pattern);

    //-----------------------------------------------------------------------//
    void        SetRootDir(const std::string &root_dir);

    //-----------------------------------------------------------------------//
    void        SetNumberOfFiles(int num_files);

    //-----------------------------------------------------------------------//
    void        SetNumberOfTrees(int num_trees);

    //-----------------------------------------------------------------------//
    void        SetProtocol( const std::string &protocol);

    void        AddMeshParitionMap(const std::string   &mesh_name,
                                   const std::string   &partition_pattern);

    void        AddMeshParitionMap(const std::string   &mesh_name,
                                   const std::string   &partition_pattern,
                                   const conduit::Node &partition_map);

    //-----------------------------------------------------------------------//
    void        FetchBlueprintTree(int tree_id, 
                                   const std::string &mesh_name,
                                   const std::string &sub_tree_path,
                                   conduit::Node &out);

    // //-----------------------------------------------------------------------//
    // void        FetchBlueprintTree(int tree_id,
    //                                const std::string &sub_tree_path,
    //                                conduit::Node &out);

private:
    //-----------------------------------------------------------------------//
    std::string Expand(const std::string pattern,
                       int idx) const;

    std::string GeneratePartitionMapFullPath(const conduit::Node &mesh_part_info,
                                             int tree_id) const;

    //-----------------------------------------------------------------------//
    std::string GenerateFilePath(const std::string &mesh_name,
                                 int tree_id) const;

    //-----------------------------------------------------------------------//
    std::string GenerateTreePath(const std::string &mesh_name,
                                 int tree_id) const;

    //-----------------------------------------------------------------------//
    bool        HasPath(const std::string &file_path,
                        const std::string &fetch_path);

    //-----------------------------------------------------------------------//
    void        Read(const std::string &file_path,
                     const std::string &fetch_path,
                     conduit::Node &out);

    //-----------------------------------------------------------------------//
    // Read using schema (supports reading slabs)
    //-----------------------------------------------------------------------//
    bool        Read(const std::string &file_path,
                     const std::string &fetch_path,
                     const conduit::DataType &dtype,
                     conduit::Node &out);

    //-----------------------------------------------------------------------//
    CacheMap   &Cache();

private: 

    // file flavor
    std::string   m_protocol;

    // legacy style blueprint index
    std::string   m_file_pattern;
    std::string   m_tree_pattern;
    int           m_num_files;
    int           m_num_trees;

    // partition map style blueprint index
    conduit::Node m_part_maps;
    std::string   m_root_dir;

    // cached results
    avtBlueprintTreeCache::CacheMap     *m_cache_map;

};


#endif
