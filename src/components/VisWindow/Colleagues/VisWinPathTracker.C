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
//                             VisWinPathTracker.C                           //
// ************************************************************************* //

#include <VisWinPathTracker.h>

#include <VisWindow.h>

#include <string>
#include <vector>
#include <map>

using std::string;
using std::vector;
using std::map;

//
// Storage for static data elements.
//

VisWinPathTracker *VisWinPathTracker::instance=0;


// ****************************************************************************
//  Method: VisWinPathTracker constructor
//
//  Programmer: Cyrus Harrison
//  Creation:   Sunday June 17, 2007
//
// ****************************************************************************

VisWinPathTracker::VisWinPathTracker()
{
    ;
}

// ****************************************************************************
//  Method: VisWinPathTracker destructor
//
//  Programmer: Cyrus Harrison
//  Creation:   Sunday June 17, 2007
//
// ****************************************************************************

VisWinPathTracker::~VisWinPathTracker()
{
   ;
}

// ****************************************************************************
//  Method: VisWinPathTracker::Instance
//
//  Purpose:
//    Returns a pointer to the singleton instance of the VisWinPathTracker
//    class.
//
//  Programmer: Cyrus Harrison
//  Creation:   Sunday June 17, 2007
//
// ****************************************************************************

VisWinPathTracker *
VisWinPathTracker::Instance()
{
    // If the singleton instance has not been instantiated, create it.
    if (instance == 0)
    {instance = new VisWinPathTracker;}

    return instance;
}


// ****************************************************************************
//  Method: VisWinPathTracker::AddPath
//
//  Purpose:
//    Adds a reference to the passed path.
//
//  Programmer: Cyrus Harrison
//  Creation:   Sunday June 17, 2007
//
// ****************************************************************************

void
VisWinPathTracker::AddPath(const std::string &path)
{
    std::map<std::string, Entry>::iterator itr;
    itr = entires.find(path);
    // if path exists, increment its counter,
    if(itr != entires.end())
    {entires[path].IncrementRefCount();}
    else // otherwise create new map entry with ref count of one
    {
        // create a new path entry
        entires[path] = Entry(path);
        // update smart paths
        UpdatePaths();
    }
}

// ****************************************************************************
//  Method: VisWinPathTracker::RemovePath
//
//  Purpose:
//    Removes a reference to the passed path.
//
//  Programmer: Cyrus Harrison
//  Creation:   Sunday June 17, 2007
//
// ****************************************************************************

void
VisWinPathTracker::RemovePath(const std::string &path)
{
    // see if the path has an entry
    std::map<std::string, Entry>::iterator itr;
    itr = entires.find(path);
    if(itr != entires.end())
    {
        // if path exists, decrement its counter
        if(!itr->second.DecrementRefCount())
        {
            // ref count == 0, remove from active paths and update paths
            entires.erase(path);
            UpdatePaths();
        }
    }
}

// ****************************************************************************
//  Method: VisWinPathTracker::GetFileName
//
//  Purpose:
//    Gets the file name of the input path
//
//  Programmer: Cyrus Harrison
//  Creation:   Sunday June 17, 2007
//
// ****************************************************************************

std::string
VisWinPathTracker::GetFileName(const std::string &path)
{
    // get the path's entry
    std::map<std::string, Entry>::iterator itr;
    itr = entires.find(path);
    if(itr != entires.end())
    {
        // return the smart path
        return entires[path].GetFileName();
    }
    // if the path does not have an entry, simply return the full path
    return path;
}

// ****************************************************************************
//  Method: VisWinPathTracker::GetDirectory
//
//  Purpose:
//    Gets the directory of the input path
//
//  Programmer: Cyrus Harrison
//  Creation:   Sunday June 17, 2007
//
// ****************************************************************************

std::string
VisWinPathTracker::GetDirectory(const std::string &path)
{
    // get the path's entry
    std::map<std::string, Entry>::iterator itr;
    itr = entires.find(path);
    if(itr != entires.end())
    {
        // return the smart path
        return entires[path].GetDirectory();
    }
    // if the path does not have an entry, simply return the full path
    return path;
}


// ****************************************************************************
//  Method: VisWinPathTracker::GetSmartPath
//
//  Purpose:
//    Gets the smart expanded path for the input path
//
//  Programmer: Cyrus Harrison
//  Creation:   Sunday June 17, 2007
//
// ****************************************************************************

std::string
VisWinPathTracker::GetSmartPath(const std::string &path)
{
    // get the path's entry
    std::map<std::string, Entry>::iterator itr;
    itr = entires.find(path);
    if(itr != entires.end())
    {
        // return the smart path
        return entires[path].GetSmartPath();
    }
    // if the path does not have an entry, simply return the full path
    return path;
}


// ****************************************************************************
//  Method: VisWinPathTracker::GetSmartDirectory
//
//  Purpose:
//    Gets the smart expanded directory for the input path
//
//  Programmer: Cyrus Harrison
//  Creation:   Tuesday September 25, 2007
//
// ****************************************************************************

std::string
VisWinPathTracker::GetSmartDirectory(const std::string &path)
{
    // get the path's entry
    std::map<std::string, Entry>::iterator itr;
    itr = entires.find(path);
    if(itr != entires.end())
    {
        // return the smart path
        return entires[path].GetSmartDirectory();
    }
    // if the path does not have an entry, simply return the full path
    return path;
}

// ****************************************************************************
//  Method: VisWinPathTracker::UpdatePaths
//
//  Purpose:
//    Construct a smart path for all tracked paths.
//
//  Programmer: Cyrus Harrison
//  Creation:   Sunday June 17, 2007
//
//  Modifications:
//
//    Cyrus Harrison, Tue Sep 25 09:54:17 PDT 2007
//    Added construction of the smart directory
//
// ****************************************************************************

void
VisWinPathTracker::UpdatePaths()
{
    std::map<std::string, Entry>::iterator itr;
    
    std::string spath;
    std::string sdir;
    
    stringVector paths;
    stringVector dirs;

    // construct a list of paths
    for ( itr = entires.begin(); itr != entires.end(); ++itr)
        paths.push_back(itr->second.GetPath());

    // construct a list of paths
    for ( itr = entires.begin(); itr != entires.end(); ++itr)
        dirs.push_back(itr->second.GetDirectory());

    
    std::string path_common = GetCommonPath(paths);
    std::string dir_common = GetCommonPath(dirs);
    
    int path_common_size = path_common.size();
    int dir_common_size  = dir_common.size();
    
    // compute the smart paths
    for ( itr = entires.begin(); itr != entires.end(); ++itr)
    {
        if( path_common == itr->second.GetPath())
            spath = itr->second.GetFileName();
        else
            spath = itr->second.GetPath().substr(path_common_size+1);


        itr->second.SetSmartPath(spath);
    }
    
    // compute the smart paths
    for ( itr = entires.begin(); itr != entires.end(); ++itr)
    {
        if( dir_common == itr->second.GetDirectory())
            sdir = dir_common.substr(dir_common.rfind("/")+1);
        else
            sdir = itr->second.GetDirectory().substr(dir_common_size+1);
        
        itr->second.SetSmartDirectory(sdir);
    }
}

/****************************************************************************
//  Method: VisWinPathTracker::GetSubPath
//
//  Purpose:
//    Gets the a path string at a given "depth". Starts at the beginning
//    of the string and uses ith '/' to terminate the subpath.
//
//  Programmer: Cyrus Harrison
//  Creation:   June 14, 2007
//
//  Modifications:
//
//    Cyrus Harrison, Mon Oct  1 11:39:47 PDT 2007
//    Switched from right path to left path to support GetCommonPath.
//
// *************************************************************************/

std::string VisWinPathTracker::GetSubPath(const std::string &path,
                                          int depth)
{
    int i = 0;
    // start depth
    int cdepth = -1;
    // current string pos
    int pos   = 0;
    // string pause
    int size = path.size();

    for ( i = 0; i < size && cdepth != depth; i++)
    {
        // if we encounter a /, update depth
        if(path[i] == '/')
        {
            pos= i;
            cdepth++;
        }
    }
    // return result
    if(cdepth != depth)
        return path;
    else
        return path.substr(0,pos);
}

/****************************************************************************
//  Method: VisWinPathTracker::GetCommonPath
//
//  Purpose:
//    Gets the common path prefix from a list of paths
//
//  Programmer: Cyrus Harrison
//  Creation:   October 1, 2007
//
// *************************************************************************/

std::string VisWinPathTracker::GetCommonPath(stringVector &paths)
{

    // loop indices
    int i = 0;
    int j = 0;
    // get # of paths
    int npaths = paths.size();
    
    // handle simple cases
    if(npaths == 0)
        return "";
    if(npaths == 1)
        return paths[0];
        
    std::string res = paths[0];
    std::string r_sub,p_sub;
    
    for( i=1; i<npaths; i++)
    {
        // if paths are exactly the same skip
        if(paths[i] == res)
            continue;
        
        int depth = 1;
            
        bool done = false;
        while( !done )
        {
            // get subpaths
            r_sub = GetSubPath(res,depth);
            p_sub = GetSubPath(paths[i],depth);

            // if they are equal continue
            if(r_sub == p_sub)
            {
                depth++;
            }
            else
            {
                // the common path the last sucessful depth
                done = true;
                res = GetSubPath(res,depth-1);
            }
        }
    }
    return res;
}

// ****************************************************************************
//  Method: VisWinPathTracker::Entry constructor
//
//  Programmer: Cyrus Harrison
//  Creation:   Sunday June 17, 2007
//
// ****************************************************************************


VisWinPathTracker::Entry::Entry()
{
    refCount  = 0;
    fullPath  = "";
    smartPath = "";
}

// ****************************************************************************
//  Method: VisWinPathTracker::Entry constructor
//
//  Programmer: Cyrus Harrison
//  Creation:   Sunday June 17, 2007
//
//    Cyrus Harrison, Tue Sep 25 10:57:57 PDT 2007
//    Added init of fileName and directory
//
// ****************************************************************************


VisWinPathTracker::Entry::Entry(const std::string &path)
{
    refCount  = 1;
    fullPath  = path;
    smartPath = path;
    directory = path;
    fileName  = path;
    
    int idx = directory.rfind("/");
    if(idx != string::npos)
    {
        fileName  = directory.substr(idx+1);
        directory = directory.substr(0,idx);
    }   
    smartDir  = directory;
}

// ****************************************************************************
//  Method: VisWinPathTracker::Entry destructor
//
//  Programmer: Cyrus Harrison
//  Creation:   Sunday June 17, 2007
//
// ****************************************************************************


VisWinPathTracker::Entry::~Entry()
{
    ;
}

