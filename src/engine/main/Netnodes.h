#ifndef NETNODES_H
#define NETNODES_H

#include <avtDataObject.h>
#include <avtDatabase.h>
#include <avtFilter.h>
#include <string>

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
// ****************************************************************************
class Netnode
{
public:
    virtual ~Netnode(void) {};
    virtual avtDataObject_p GetOutput(void) = 0;
    virtual void ReleaseData() = 0;
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
// ****************************************************************************
class NetnodeDB: public Netnode
{
public:
    NetnodeDB(avtDatabase *);
    NetnodeDB(ref_ptr<avtDatabase>);
    ref_ptr<avtDatabase>    GetDatabase(void) { return db; };
    virtual ~NetnodeDB(void) {FreeUpResources();};
    void SetDBInfo(std::string _filename, std::string _var, int _time);
    virtual avtDataObject_p GetOutput(void);
    std::string GetFilename(void) {return filename;};
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
class NetnodeFilter: public Netnode
{
public:
    NetnodeFilter(avtFilter *_filter, const std::string &type)
        { filter = _filter; filtertype = type; };
    virtual ~NetnodeFilter(void) {};
    std::vector<Netnode*>& GetInputNodes(void) { return inputNodes; };
    virtual avtDataObject_p GetOutput(void);
    void ReleaseData();

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

class NetnodeTransition: public NetnodeFilter
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
