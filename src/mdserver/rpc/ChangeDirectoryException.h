#ifndef CHANGE_DIRECTORY_EXCEPTION_H
#define CHANGE_DIRECTORY_EXCEPTION_H
#include <mdsrpc_exports.h>
#include <string>
#include <VisItException.h>

// *******************************************************************
// Class: ChangeDirectoryException
//
// Purpose:
//   The exception that should be thrown when a ChangeDirectoryRPC fails.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 25 12:40:50 PDT 2000
//
// Modifications:
//   
// *******************************************************************

class MDSERVER_RPC_API ChangeDirectoryException : public VisItException
{
public:
    ChangeDirectoryException(const std::string &dir);
    virtual ~ChangeDirectoryException() VISIT_THROW_NOTHING {;};

    const std::string &GetDirectory() const;
private:
    std::string directoryName;
};

#endif
