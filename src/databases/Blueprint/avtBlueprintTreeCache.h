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
