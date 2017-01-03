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
//                           avtSiloMBObjectCache.h                          //
// ************************************************************************* //

#include <string>
#include <map>
#include <silo.h>

// ****************************************************************************
//  Class: avtSiloMBNameGenerator
//
//  Purpose:
//    A name geneartor for mulit block objects that handles the to silo
//    mb object naming conventions:
//       string lists
//       nameschemes
//
//  Namescheme support will not be completely general until some future Silo
//  API changes.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
//    Mark C. Miller, Tue Feb  2 15:19:09 PST 2016
//    Handle empty list too.
// ****************************************************************************


class avtSiloMBNameGenerator
{
public:
                 avtSiloMBNameGenerator(DBfile      *dbfile,
                                        char const *objpath,
                                        int          nblocks,
                                        char const * const *names_lst,
                                        const char  *file_ns,
                                        const char  *block_ns,
                                        int          empty_cnt,
                                        int const   *empty_lst);

    virtual     ~avtSiloMBNameGenerator();
    std::string  Name(int idx) const;
    int          NumberOfBlocks() const { return nblocks;}

private:
      int             nblocks;
      char const * const *namesLst;
      DBnamescheme   *fileNS;
      int            *fileVals;
      DBnamescheme   *blockNS;
      int            *blockVals;
      int             emptyCnt;
      int const      *emptyLst;
};


// ****************************************************************************
//  Class: avtSiloMBObjectCacheEntry
//
//  Purpose:
//    Base class for a cachable silo mulit block object.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
//    Mark C. Miller, Tue Feb  2 15:19:09 PST 2016
//    Handle empty list too.
// ****************************************************************************

class avtSiloMBObjectCacheEntry
{
public:
                             avtSiloMBObjectCacheEntry(DBfile *dbfile,
                                                       char const *objpath,
                                                       int     nblocks,
                                                       char  **names_lst,
                                                       const char *file_ns,
                                                       const char *block_ns,
                                                       int empty_cnt,
                                                       int *empty_list);
    virtual                ~avtSiloMBObjectCacheEntry();

    int                     NumberOfBlocks() const
                                { return nameGen->NumberOfBlocks();}
    std::string             GenerateName(int idx) const
                                { return nameGen->Name(idx);}
private:
    avtSiloMBNameGenerator *nameGen;
};

// ****************************************************************************
//  Class: avtSiloMBObjectCache
//
//  Purpose:
//    Cache for silo multi block objects.
//    Provides a single data structure for accecssing various multi block
//    objects.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
// ****************************************************************************

class avtSiloMBObjectCache
{
public:
             avtSiloMBObjectCache();
    virtual ~avtSiloMBObjectCache();

    void                       AddEntry(const std::string &full_path,
                                        avtSiloMBObjectCacheEntry *cache_entry);

    void                       AddEntry(const char *path, const char *name,
                                        avtSiloMBObjectCacheEntry *cache_entry);

    bool                       HasEntry(const std::string &full_path)       const;
    bool                       HasEntry(const char *path, const char *name) const;

    avtSiloMBObjectCacheEntry *FetchEntry(const std::string &full_path) const;
    avtSiloMBObjectCacheEntry *FetchEntry(const char *path, const char *name) const;

    void                       RemoveEntry(const std::string &full_path);
    void                       RemoveEntry(const char *path, const char *name);

    void                       Clear();

    static std::string         CombinePath(const char *path, const char*name);

private:
    std::map<std::string, avtSiloMBObjectCacheEntry * > cache;
};


// ****************************************************************************
//  Class: avtSiloMultiMeshCacheEntry
//
//  Purpose:
//    Cache entry that wraps a silo mulitmesh object.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
// ****************************************************************************

class avtSiloMultiMeshCacheEntry: public avtSiloMBObjectCacheEntry
{
public:
                 avtSiloMultiMeshCacheEntry(DBfile      *dbfile,
                                            char const *objpath,
                                            DBmultimesh *mm);
    virtual     ~avtSiloMultiMeshCacheEntry();
    DBmultimesh *DataObject() {return obj;}
    int          MeshType(int idx) const;

private:
    DBmultimesh *obj;
};


// ****************************************************************************
//  Class: avtSiloMultiVarCacheEntry
//
//  Purpose:
//    Cache entry that wraps a silo mulitvar object.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
// ****************************************************************************

class avtSiloMultiVarCacheEntry: public avtSiloMBObjectCacheEntry
{
public:
                avtSiloMultiVarCacheEntry(DBfile     *dbfile,
                                          char const *objpath,
                                          DBmultivar *mv);
    virtual    ~avtSiloMultiVarCacheEntry();
    DBmultivar *DataObject() {return obj;}
    int         VarType(int idx) const;

private:
    DBmultivar *obj;
};

// ****************************************************************************
//  Class: avtSiloMultiMatCacheEntry
//
//  Purpose:
//    Cache entry that wraps a silo mulitmat object.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
// ****************************************************************************

class avtSiloMultiMatCacheEntry: public avtSiloMBObjectCacheEntry
{
public:
                 avtSiloMultiMatCacheEntry(DBfile     *dbfile,
                                           char const *objpath,
                                           DBmultimat *mmat);
    virtual     ~avtSiloMultiMatCacheEntry();
    DBmultimat  *DataObject() {return obj;}

private:
    DBmultimat *obj;
};

// ****************************************************************************
//  Class: avtSiloMultiSpecCacheEntry
//
//  Purpose:
//    Cache entry that wraps a silo mulitmatspecies object.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Dec 21 14:30:09 PST 2011
//
//  Modifications:
//
// ****************************************************************************

class avtSiloMultiSpecCacheEntry: public avtSiloMBObjectCacheEntry
{
public:
                       avtSiloMultiSpecCacheEntry(DBfile            *dbfile,
                                                  char const *objpath,
                                                  DBmultimatspecies *mv);
    virtual           ~avtSiloMultiSpecCacheEntry();
    DBmultimatspecies *DataObject() {return obj;}

private:
    DBmultimatspecies *obj;
};


