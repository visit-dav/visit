#ifndef _GETDIRECTORY_RPC_H_
#define _GETDIRECTORY_RPC_H_
#include <mdsrpc_exports.h>

#include <VisItRPC.h>
#include <string>

// *******************************************************************
// Class: GetDirectoryRPC
//
// Purpose:
//   This RPC gets the current directory from a remote file system.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 10:56:10 PDT 2000
//
// Notes:
//   The cwd attribute contains that data returned from executing
//   the RPC.
//
// Modifications:
//   Brad Whitlock, Tue Aug 29 10:57:00 PDT 2000
//   I moved DirectoryName's implementation to the .C file.
//
// *******************************************************************

class MDSERVER_RPC_API GetDirectoryRPC : public BlockingRPC
{
public:
    struct MDSERVER_RPC_API DirectoryName : public AttributeSubject
    {
        std::string name;
    public:
        DirectoryName();
        ~DirectoryName();
        virtual void SelectAll();
        virtual const std::string TypeName() const
            { return "GetDirectoryRPC::DirectoryName"; };
    };
public:
    GetDirectoryRPC();
    virtual ~GetDirectoryRPC();

    // Invokation method
    std::string operator()();

    // Property selection methods
    virtual void SelectAll();
private:
    DirectoryName cwd;
};


#endif
