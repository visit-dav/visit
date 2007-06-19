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
//  Method: VisWinPathTracker::UpdatePaths
//
//  Purpose:
//    Construct a smart path for all tracked paths.
//
//  Programmer: Cyrus Harrison
//  Creation:   Sunday June 17, 2007
//
// ****************************************************************************

void
VisWinPathTracker::UpdatePaths()
{
    std::map<std::string, Entry>::iterator itr;
    std::string spath;
    stringVector paths;

    // construct a list of paths
    for ( itr = entires.begin(); itr != entires.end(); ++itr)
    {paths.push_back(itr->second.GetPath());}

    // compute the smart paths
    for ( itr = entires.begin(); itr != entires.end(); ++itr)
    {
        spath = GetUniquePath(itr->second.GetPath(),paths);
        itr->second.SetSmartPath(spath);
    }

}

/****************************************************************************
//  Method: VisWinPathTracker::GetSubPath
//
//  Purpose:
//    Gets the a path string has a given "depth". Starts from the right
//    end of the string and uses ith '/' to terminate the subpath.
//
//  Programmer: Cyrus Harrison
//  Creation:   June 14, 2007
//
// *************************************************************************/

std::string VisWinPathTracker::GetSubPath(const std::string &path,
                                          int depth)
{
    // loop index
    int i     = 0;
    // start depth
    int cdepth = -1;
    // current string pos
    int pos   = 0;
    // string pause
    int size = path.size();

    // loop through string in reverse
    for ( i = size; i > 0 && cdepth != depth; i--)
    {
        // if we encounter a /, update depth
        if(path[i] == '/')
        {
            pos= i;
            cdepth++;
        }
    }
    // return result
    return path.substr(pos+1,size - pos);
}

// ****************************************************************************
//  Method: VisWinPathTracker::GetUniquePath
//
//  Purpose:
//      Finds a unique path string give a full path name and a list of other
//      active paths.
//
//  Programmer: Cyrus Harrison
//  Creation:   June 14, 2007
//
// ****************************************************************************

std::string
VisWinPathTracker::GetUniquePath(const std::string &path,
                                 stringVector &test_paths)
{
    // loop index
    int i;
    // get number of test paths
    int ntst = test_paths.size();
    // used to keep track of path depth
    int cur_depth = 0;
    int tst_depth = 0;
    // strings to hold current subpaths
    std::string cur_path ="";
    std::string tst_path ="";

    for ( i = 0; i < ntst; i++)
    {
        // make sure the an not the same path
        if (path == test_paths[i])
        {continue;}

        tst_depth = cur_depth;
        // make sure path is unique with test path
        bool done = false;
        while( !done )
        {
            // get the subpaths
            cur_path = GetSubPath(path,cur_depth);
            tst_path = GetSubPath(test_paths[i],tst_depth);

            if(cur_path == tst_path)
            {
                // inc depth if not unique
                tst_depth++;
                cur_depth++;
            }
            else
            {done = true;}
        }
    }

    // construct result
    std::string res = GetSubPath(path,cur_depth);
    // if full path is necessary, prepend final '/'
    if(path == "/" + res)
    {return path;}
    else
    {return res;}
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
// ****************************************************************************


VisWinPathTracker::Entry::Entry(const std::string &path)
{
    refCount  = 1;
    fullPath  = path;
    smartPath = path;
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

