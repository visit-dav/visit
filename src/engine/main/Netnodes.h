// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef NETNODES_H
#define NETNODES_H

#include <avtDataObject.h>
#include <avtDatabase.h>
#include <avtFilter.h>
#include <string>
#include <engine_main_exports.h>

// ****************************************************************************
//  Class: Netnode
//
//  Purpose:
//      Abstract class that identifies one node in a Network.
//
//  Programmer: Sean Ahern
//  Creation:   Mon May 13 14:39:37 PDT 2002
//
//  Modifications:
//
//    Hank Childs, Mon Apr  6 13:05:35 PDT 2009
//    Add support for getting direct access to filters.
//
// ****************************************************************************
class ENGINE_MAIN_API Netnode
{
public:
    virtual ~Netnode(void) {};
    virtual avtDataObject_p GetOutput(void) = 0;
    virtual void ReleaseData() = 0;
    virtual avtFilter *GetFilter(void) { return NULL; };
};

// ****************************************************************************
//  Class: Netnode
//
//  Purpose:
//      Class that identifies a database node in a Network.
//
//  Programmer: Sean Ahern
//  Creation:   Mon May 13 14:39:37 PDT 2002
//
//  Modifications:
//    Sean Ahern, Tue May 21 16:10:35 PDT 2002
//    Made the destructor tell the avtDatabase to free its resources.
//
//    Hank Childs, Mon Jul 28 13:33:54 PDT 2003
//    Do more initialization in the constructor.  Added necessary information
//    for caching data objects.
//
//    Hank Childs, Tue Nov 18 14:11:33 PST 2003
//    Put SetDBInfo into the .C file, since it now does more work.
//
//    Jeremy Meredith, Wed Aug 25 11:54:18 PDT 2004
//    Added method to return the raw database.
//
//    Kathleen Biagas, Wed May 24 17:16:49 PDT 2017
//    Removed duplicate method for returning the database.
//
//    Alister Maguire, Tue Oct 29 13:38:59 MST 2019
//    Added GetVarName.
//
// ****************************************************************************
class ENGINE_MAIN_API NetnodeDB: public Netnode
{
public:
    NetnodeDB(avtDatabase *);
    NetnodeDB(ref_ptr<avtDatabase>);
    virtual ~NetnodeDB(void) {FreeUpResources();};
    void SetDBInfo(std::string _filename, std::string _var, int _time);
    virtual avtDataObject_p GetOutput(void);
    std::string GetFilename(void) {return filename;};
    std::string GetVarName(void) {return var;};
    int GetTime(void) {return time;};
    ref_ptr<avtDatabase> GetDB(void) {return db;};
    void ReleaseData();
    void FreeUpResources() { db->FreeUpResources(); };
    avtDatabaseMetaData *GetMetaData(int ts) { return db->GetMetaData(ts); }

protected:
    ref_ptr<avtDatabase>    db;
    avtDataObject_p         output;
    std::string             filename;
    std::string             var;
    int                     time;
    std::string             current_var;
    int                     current_time;
};

// ****************************************************************************
//  Class: NetnodeFilter
//
//  Purpose:
//      Class that identifies a filter in a Network.
//
//  Programmer: Sean Ahern
//  Creation:   Mon May 13 14:39:37 PDT 2002
//
//  Modifications:
//
// ****************************************************************************
class ENGINE_MAIN_API NetnodeFilter: public Netnode
{
public:
    NetnodeFilter(avtFilter *_filter, const std::string &type)
        { filter = _filter; filtertype = type; };
    virtual ~NetnodeFilter(void) {};
    std::vector<Netnode*>& GetInputNodes(void) { return inputNodes; };
    virtual avtDataObject_p GetOutput(void);
    void ReleaseData();
    virtual avtFilter *GetFilter(void) { return *filter; };

protected:
    ref_ptr<avtFilter>          filter;
    std::string                 filtertype;
    std::vector<Netnode*>       inputNodes;
};


// ****************************************************************************
//  Class: NetnodeTransition
//
//  Purpose:
//      Class that identifies a transition in a Network.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 18, 2004
//
//  Modifications:
//
// ****************************************************************************

class ENGINE_MAIN_API NetnodeTransition: public NetnodeFilter
{
public:
             NetnodeTransition(avtDataObject_p input);
    virtual ~NetnodeTransition(void) {};
    virtual avtDataObject_p GetOutput(void) { return output; };
    void     ReleaseData() {;} ;

protected:
    avtDataObject_p             output;
};

#endif
