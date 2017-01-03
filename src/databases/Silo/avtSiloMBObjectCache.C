/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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
//                           avtSiloMBObjectCache.C                          //
// ************************************************************************* //

#include <avtSiloMBObjectCache.h>
#include <iostream>
#include <cstring>
#include <string>
#include <DebugStream.h>
using namespace std;

// ****************************************************************************
//  Method: avtSiloMBNameGenerator Constructor
//
//  Purpose:
//    Initializes a name generator object from a silo multi block object.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//   Cyrus Harrison, Mon Aug  6 11:37:08 PDT 2012
//    Add more flexibility w/ by parsing namescheme block array name.
//
//    Mark C. Miller, Wed Oct 31 15:52:34 PDT 2012
//    Updated to use improved Silo DBnamescheme constructor that can handle
//    external array references internally itself.
//
//    Mark C. Miller, Wed Mar 13 23:03:57 PDT 2013
//    If objpath is non-null but "", still pass null to DBMakeNamescheme.
//
//    Mark C. Miller, Tue Feb  2 15:19:09 PST 2016
//    Handle empty list too.
// ****************************************************************************

avtSiloMBNameGenerator::avtSiloMBNameGenerator
(DBfile *dbfile, char const *objpath, int nblocks,
 char const * const *names_lst, // standard use case
 const char *file_ns, const char *block_ns,
 int empty_cnt, int const *empty_lst) // nameschem use case
 : nblocks(nblocks), namesLst(names_lst),
   fileNS(0), fileVals(0),
   blockNS(0), blockVals(0),
   emptyCnt(empty_cnt), emptyLst(empty_lst)
   
{
    // if we have an empty object, we
    if(nblocks == 0)
        return;

    // if we simple have a list of names we are done
    if(namesLst != NULL)
        return;

    // we need to create nameschemes.
    if (file_ns)
        fileNS = DBMakeNamescheme(file_ns, 0, dbfile,
            objpath?(strlen(objpath)?objpath:0):0);
    if (block_ns)
        blockNS = DBMakeNamescheme(block_ns, 0, dbfile,
            objpath?(strlen(objpath)?objpath:0):0);
}

// ****************************************************************************
//  Method: avtSiloMBNameGenerator Destructor
//
//  Purpose:
//    Cleans up name generator resources.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
// ****************************************************************************

avtSiloMBNameGenerator::~avtSiloMBNameGenerator()
{
    // clean up namescheme data if we generated it.
    if(fileNS    != 0) DBFreeNamescheme(fileNS);
    if(fileVals  != 0) delete [] fileVals;
    if(blockNS   != 0) DBFreeNamescheme(blockNS);
    if(blockVals != 0) delete [] blockVals;
}

// ****************************************************************************
//  Method: avtSiloMBNameGenerator::Name
//
//  Purpose:
//    Returns proper name for given block index.
//    Uses string list, or namescheme as appropriate.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
//    Mark C. Miller, Tue Feb  2 15:19:09 PST 2016
//    Handle empty list too.
// ****************************************************************************

string
avtSiloMBNameGenerator::Name(int idx) const
{
    string res = "";

    // bounds check
    if(idx < 0 || idx >=nblocks)
        return res;

    // check for simple case
    if(namesLst != 0)
        return string(namesLst[idx]);

    if (emptyLst)
    {
        register int bot = 0;
        register int top = emptyCnt - 1;
        register int mid;
        while (bot <= top)
        {
            mid = (bot + top) >> 1;

            if (idx > emptyLst[mid])
                bot = mid + 1;
            else if (idx < emptyLst[mid])
                top = mid - 1;
            else
                return "EMPTY";
        }
    }

    // namescheme case
    if(fileNS !=0)
    {
        const char *file_res = DBGetName(fileNS,idx);
        if(file_res != 0)
            res += (string(file_res) + ":");
    }

    if(blockNS !=0)
    {
        const char *block_res = DBGetName(blockNS,idx);
        if(block_res != 0)
            res += string(block_res);
    }

    return res;
}

// ****************************************************************************
//  Method: avtSiloMBObjectCacheEntry Constructor
//
//  Purpose:
//    Creates a name generator object.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
//    Mark C. Miller, Tue Feb  2 15:19:09 PST 2016
//    Handle empty list too.
// ****************************************************************************

avtSiloMBObjectCacheEntry::avtSiloMBObjectCacheEntry(DBfile *dbfile,
                                                     char const *objpath,
                                                     int     nblocks,
                                                     char  **mesh_names,
                                                     const char   *file_ns,
                                                     const char   *block_ns,
                                                     int empty_cnt,
                                                     int *empty_list)
: nameGen(NULL)
{
    nameGen = new avtSiloMBNameGenerator(dbfile,objpath,nblocks,
                                         mesh_names,
                                         file_ns,block_ns,
                                         empty_cnt, empty_list);
}


// ****************************************************************************
//  Method: avtSiloMBObjectCacheEntry Destructor
//
//  Purpose:
//    Frees created name generator.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
// ****************************************************************************

avtSiloMBObjectCacheEntry::~avtSiloMBObjectCacheEntry()
{
    if(nameGen != NULL) delete nameGen;
}


// ****************************************************************************
//  Method: avtSiloMBObjectCache Constructor
//
//  Purpose:
//    Initializes a MB Object Cache object.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
// ****************************************************************************

avtSiloMBObjectCache::avtSiloMBObjectCache()
{}

// ****************************************************************************
//  Method: avtSiloMBObjectCache Destructor
//
//  Purpose:
//    Clears (and frees) all cache entries.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
// ****************************************************************************

avtSiloMBObjectCache::~avtSiloMBObjectCache()
{Clear();}


// ****************************************************************************
//  Method: avtSiloMBObjectCache::AddEntry
//
//  Purpose:
//    Adds a cache entry for the given full path.
//    If a matching entry exists, it is deleted.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
// ****************************************************************************

void
avtSiloMBObjectCache::AddEntry(const string &full_path,
                               avtSiloMBObjectCacheEntry *cache_entry)
{
    if(HasEntry(full_path))
        delete cache[full_path];
    cache[full_path] = cache_entry;
}

// ****************************************************************************
//  Method: avtSiloMBObjectCache::AddEntry
//
//  Purpose:
//    Adds a cache entry for the given name and path.
//    If a matching entry exists, it is deleted.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
// ****************************************************************************

void
avtSiloMBObjectCache::AddEntry(const char *path, const char *name,
                               avtSiloMBObjectCacheEntry *cache_entry)
{
    AddEntry(CombinePath(path,name),cache_entry);
}

// ****************************************************************************
//  Method: avtSiloMBObjectCache::FetchEntry
//
//  Purpose:
//    Fetches a the cache entry with the given full path.
//    Returns null if the entry does not exist.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
// ****************************************************************************

avtSiloMBObjectCacheEntry *
avtSiloMBObjectCache::FetchEntry(const string &full_path) const
{
    avtSiloMBObjectCacheEntry *res = NULL;
    map<string, avtSiloMBObjectCacheEntry * >::const_iterator itr;
    itr = cache.find(full_path);
    if(itr != cache.end())
        res = (*itr).second;
    return res;
}

// ****************************************************************************
//  Method: avtSiloMBObjectCache::FetchEntry
//
//  Purpose:
//    Fetches a the cache entry with the given path and name.
//    Returns null if the entry does not exist.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
// ****************************************************************************

avtSiloMBObjectCacheEntry *
avtSiloMBObjectCache::FetchEntry(const char *path, const char *name) const
{
    return FetchEntry(CombinePath(path,name));
}

// ****************************************************************************
//  Method: avtSiloMBObjectCache::HasEntry
//
//  Purpose:
//    Checks if a cache entry with the given full path exists.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
// ****************************************************************************

bool
avtSiloMBObjectCache::HasEntry(const string &full_path) const
{
    bool res = false;
    map<string, avtSiloMBObjectCacheEntry * >::const_iterator itr;
    itr = cache.find(full_path);
    if(itr != cache.end())
        res = true;
    return res;
}

// ****************************************************************************
//  Method: avtSiloMBObjectCache::HasEntry
//
//  Purpose:
//    Checks if a cache entry with the given path and name exists.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
// ****************************************************************************

bool
avtSiloMBObjectCache::HasEntry(const char *path, const char *name) const
{
    return HasEntry(CombinePath(path,name));
}

// ****************************************************************************
//  Method: avtSiloMBObjectCache::RemoveEntry
//
//  Purpose:
//    Removes (and frees) a cache entry by full path.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
// ****************************************************************************

void
avtSiloMBObjectCache::RemoveEntry(const string &full_path)
{
    //avtSiloMBObjectCacheEntry *res = NULL; (void) res;
    map<string, avtSiloMBObjectCacheEntry * >::iterator itr;
    itr = cache.find(full_path);
    if(itr != cache.end())
    {
        delete (*itr).second;
        cache.erase(itr);
    }
}


// ****************************************************************************
//  Method: avtSiloMBObjectCache::RemoveEntry
//
//  Purpose:
//    Removes (and frees) a cache entry by path and name.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
// ****************************************************************************

void
avtSiloMBObjectCache::RemoveEntry(const char *path, const char *name)
{
    RemoveEntry(CombinePath(path,name));
}

// ****************************************************************************
//  Method: avtSiloMBObjectCache::Clear
//
//  Purpose:
//    Clears (and frees) all cache entires.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
// ****************************************************************************

void
avtSiloMBObjectCache::Clear()
{
    map<string, avtSiloMBObjectCacheEntry * >::iterator itr;
    for(itr  = cache.begin(); itr != cache.end(); ++itr)
    {
        delete (*itr).second;
    }
    cache.clear();
}

// ****************************************************************************
//  Method: avtSiloMBObjectCache::CombinePath
//
//  Purpose:
//    Helper which creates a valid silo object path for various usecases.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
// ****************************************************************************

string
avtSiloMBObjectCache::CombinePath(const char *path, const char *name)
{
    string res = "";
    if ((path == NULL) || (strcmp(path, "") == 0) || (strcmp(path, "/") == 0))
        res = string(name);
    else
        res = string(path) + string("/") + string(name);
    return res;
}


// ****************************************************************************
//  Method: avtSiloMultiMeshCacheEntry Constructor
//
//  Purpose:
//   Creates a avtSiloMultiMeshCacheEntry object that wraps a Silo
//   multimesh.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
//    Mark C. Miller, Tue Feb  2 15:19:09 PST 2016
//    Handle empty list too.
// ****************************************************************************
avtSiloMultiMeshCacheEntry::avtSiloMultiMeshCacheEntry(DBfile *dbfile,
                                                       char const *objpath,
                                                       DBmultimesh *mm)
: avtSiloMBObjectCacheEntry(dbfile,
                            objpath,
                            mm->nblocks,
                            mm->meshnames,
                            mm->file_ns,
                            mm->block_ns,
                            mm->empty_cnt,
                            mm->empty_list),
  obj(mm)
{
}

// ****************************************************************************
//  Method: avtSiloMultiMeshCacheEntry Destructor
//
//  Purpose:
//    Frees wrapped mulitmesh object.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
// ****************************************************************************
avtSiloMultiMeshCacheEntry::~avtSiloMultiMeshCacheEntry()
{
    if(obj != NULL) DBFreeMultimesh(obj);
}

// ****************************************************************************
//  Method: avtSiloMultiMeshCacheEntry::MeshType
//
//  Purpose:
//    Returns mesh type of given block index.
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Jan  3 09:34:46 PST 2012
//
//  Modifications:
//
// ****************************************************************************
int
avtSiloMultiMeshCacheEntry::MeshType(int idx) const
{
    int res = DB_NONE;
    if(obj != NULL)
    {
        if(obj->meshtypes == NULL)
            res = obj->block_type;
        else
            res = obj->meshtypes[idx];
    }
    return res;
}

// ****************************************************************************
//  Method: avtSiloMultiVarCacheEntry Constructor
//
//  Purpose:
//    Creates a avtSiloMultiVarCacheEntry object that wraps A Silo multivar
//    object.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
//    Mark C. Miller, Tue Feb  2 15:19:09 PST 2016
//    Handle empty list too.
// ****************************************************************************
avtSiloMultiVarCacheEntry::avtSiloMultiVarCacheEntry(DBfile *dbfile,
                                                     char const *objpath,
                                                     DBmultivar *mv)
: avtSiloMBObjectCacheEntry(dbfile,
                            objpath,
                            mv->nvars,
                            mv->varnames,
                            mv->file_ns,
                            mv->block_ns,
                            mv->empty_cnt,
                            mv->empty_list),
  obj(mv)
{
}

// ****************************************************************************
//  Method: avtSiloMultiVarCacheEntry Destructor
//
//  Purpose:
//    Frees wrapped mulitvar object.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
// ****************************************************************************
avtSiloMultiVarCacheEntry::~avtSiloMultiVarCacheEntry()
{
    if(obj != NULL) DBFreeMultivar(obj);
}

// ****************************************************************************
//  Method: avtSiloMultiVarCacheEntry::VarType
//
//  Purpose:
//    Returns variable type of the given block index.
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Jan  3 09:34:46 PST 2012
//
//  Modifications:
//
// ****************************************************************************
int
avtSiloMultiVarCacheEntry::VarType(int idx) const
{
    int res = DB_NONE;
    if(obj != NULL)
    {
        if(obj->vartypes == NULL)
            res = obj->block_type;
        else 
            res = obj->vartypes[idx];
    }
    return res;
}



// ****************************************************************************
//  Method: avtSiloMultiMatCacheEntry Constructor
//
//  Purpose:
//    Creates a avtSiloMultiMatCacheEntry object that wraps A Silo multimat
//    object.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
//    Mark C. Miller, Tue Feb  2 15:19:09 PST 2016
//    Handle empty list too.
// ****************************************************************************
avtSiloMultiMatCacheEntry::avtSiloMultiMatCacheEntry(DBfile *dbfile,
                                                     char const *objpath,
                                                     DBmultimat *mm)
: avtSiloMBObjectCacheEntry(dbfile,
                            objpath,
                            mm->nmats,
                            mm->matnames,
                            mm->file_ns,
                            mm->block_ns,
                            mm->empty_cnt,
                            mm->empty_list),
  obj(mm)
{
}

// ****************************************************************************
//  Method: avtSiloMultiMatCacheEntry Destructor
//
//  Purpose:
//    Frees wrapped multimat object.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
// ****************************************************************************
avtSiloMultiMatCacheEntry::~avtSiloMultiMatCacheEntry()
{
    if(obj != NULL) DBFreeMultimat(obj);
}


// ****************************************************************************
//  Method: avtSiloMultiSpecCacheEntry Constructor
//
//  Purpose:
//    Creates a avtSiloMultiSpecCacheEntry object that wraps A Silo
//    multimatspecies object.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
//    Mark C. Miller, Tue Feb  2 15:19:09 PST 2016
//    Handle empty list too.
// ****************************************************************************
avtSiloMultiSpecCacheEntry::avtSiloMultiSpecCacheEntry(DBfile *dbfile,
                                                       char const *objpath,
                                                       DBmultimatspecies *ms)
: avtSiloMBObjectCacheEntry(dbfile,
                            objpath,
                            ms->nspec,
                            ms->specnames,
                            ms->file_ns,
                            ms->block_ns,
                            ms->empty_cnt,
                            ms->empty_list),
  obj(ms)
{
}

// ****************************************************************************
//  Method: avtSiloMultiSpecCacheEntry Destructor
//
//  Purpose:
//    Frees the wrapped multimatspecies object.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
// ****************************************************************************
avtSiloMultiSpecCacheEntry::~avtSiloMultiSpecCacheEntry()
{
    if(obj != NULL) DBFreeMultimatspecies(obj);
}

