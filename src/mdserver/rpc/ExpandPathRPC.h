// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef _EXPANDPATH_RPC_H_
#define _EXPANDPATH_RPC_H_
#include <mdsrpc_exports.h>

#include <VisItRPC.h>
#include <string>

// ****************************************************************************
// Class: ExpandPathRPC
//
// Purpose:
//   This RPC expands pathnames to their fullest.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 13:38:19 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Dec  7 11:34:01 PST 2007
//   Added TypeName override.
//
// ****************************************************************************

class MDSERVER_RPC_API ExpandPathRPC : public BlockingRPC
{
public:
    struct MDSERVER_RPC_API PathName : public AttributeSubject
    {
        std::string name;
    public:
        PathName();
        ~PathName();
        virtual void SelectAll();
        virtual const std::string TypeName() const;
    };
public:
    ExpandPathRPC();
    virtual ~ExpandPathRPC();

    // Invocation method
    std::string operator()(const std::string &);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetPath(const std::string &p);

    // Property getting methods
    std::string GetPath() const { return path.name; };

    virtual const std::string TypeName() const;
private:
    PathName path;
};


#endif
