#ifndef GET_METADATA_EXCEPTION_H
#define GET_METADATA_EXCEPTION_H
#include <mdsrpc_exports.h>
#include <string>
#include <VisItException.h>

// *******************************************************************
// Class: GetMetaDataException
//
// Purpose:
//   The exception that should be thrown when the file list cannot
//   be gotten.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   Brad Whitlock, Thu Feb 7 11:14:27 PDT 2002
//   Added a new constructor.
//
// *******************************************************************

class MDSERVER_RPC_API GetMetaDataException : public VisItException
{
public:
    GetMetaDataException();
    GetMetaDataException(const std::string &str);
    virtual ~GetMetaDataException() VISIT_THROW_NOTHING {;};
};

#endif
