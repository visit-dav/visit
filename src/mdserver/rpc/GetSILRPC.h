#ifndef _GETSIL_RPC_H_
#define _GETSIL_RPC_H_
#include <mdsrpc_exports.h>

#include <VisItRPC.h>
#include <vector>
#include <string>
#include <SILAttributes.h>

// ****************************************************************************
// Class: GetSILRPC
//
// Purpose:
//   This class encapsulates a call to get the SIL for a database
//   from a remote file system.
//
// Notes:      
//
// Programmer: Hank Childs
// Creation:   March 29, 2001
//
// Modifications:
//   Brad Whitlock, Tue May 13 15:31:25 PST 2003
//   I added timeState.
//
// ****************************************************************************

class MDSERVER_RPC_API GetSILRPC : public BlockingRPC
{
public:
    GetSILRPC();
    virtual ~GetSILRPC();

    virtual const std::string TypeName() const;

    // Invokation method
    const SILAttributes *operator()(const std::string&, int ts = 0);

    // Property setting methods
    void SetFile(const std::string&);
    void SetTimeState(int ts);

    // Property getting methods
    std::string GetFile() const;
    int         GetTimeState() const;

    // Property selection methods
    virtual void SelectAll();
private:
    SILAttributes    sil;
    std::string      file;
    int              timeState;
};


#endif
