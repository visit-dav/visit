#ifndef _GETMETADATA_RPC_H_
#define _GETMETADATA_RPC_H_
#include <mdsrpc_exports.h>

#include <VisItRPC.h>
#include <vector>
#include <string>
#include <avtDatabaseMetaData.h>

// ****************************************************************************
// Class: GetMetaDataRPC
//
// Purpose:
//   This class encapsulates a call to get the metadata for a database
//   from a remote file system.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   Brad Whitlock, Tue May 13 15:18:10 PST 2003
//   I added an optional timestate argument.
//
// ****************************************************************************

class MDSERVER_RPC_API GetMetaDataRPC : public BlockingRPC
{
public:
    GetMetaDataRPC();
    virtual ~GetMetaDataRPC();

    virtual const std::string TypeName() const;

    // Invokation method
    const avtDatabaseMetaData *operator()(const std::string&, int timeState=0);

    // Property setting methods
    void SetFile(const std::string&);
    void SetTimeState(int ts);

    // Property getting methods
    std::string GetFile() const;
    int GetTimeState() const;

    // Property selection methods
    virtual void SelectAll();
private:
    avtDatabaseMetaData  metaData;
    std::string          file;
    int                  timeState;
};


#endif
