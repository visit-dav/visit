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
    void        SetNumberOfFiles(int num_files);

    //-----------------------------------------------------------------------//
    void        SetNumberOfTrees(int num_trees);

    //-----------------------------------------------------------------------//
    void        SetProtocol( const std::string &protocol);

    //-----------------------------------------------------------------------//
    void        FetchBlueprintTree(int tree_id, 
                                   const std::string &sub_tree_path,
                                   conduit::Node &out);



    //-----------------------------------------------------------------------//
    std::string Expand(const std::string pattern,
                       int idx) const;

    //-----------------------------------------------------------------------//
    std::string GenerateFilePath(int tree_id) const;

    //-----------------------------------------------------------------------//
    std::string GenerateTreePath(int tree_id) const;


    
    //-----------------------------------------------------------------------//
    bool        HasPath(int tree_id,
                        const std::string &path);

    //-----------------------------------------------------------------------//
    void        Read(int tree_id,
                     const std::string &path,
                     conduit::Node &out);
    
    //-----------------------------------------------------------------------//
    // Read using schema (supports reading slabs)
    //-----------------------------------------------------------------------//
    bool        Read(int domain_id,
                     const std::string &path,
                     const conduit::DataType &dtype,
                     conduit::Node &out);

    //-----------------------------------------------------------------------//
    CacheMap   &Cache();

private: 
    

    
    
    std::string   m_file_pattern;
    std::string   m_tree_pattern;
    int           m_num_files;
    int           m_num_trees;
    std::string   m_protocol;
    avtBlueprintTreeCache::CacheMap     *m_cache_map;

};


#endif
