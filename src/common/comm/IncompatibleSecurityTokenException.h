#ifndef INCOMPATIBLE_SECURITY_TOKEN_EXCEPTION_H
#define INCOMPATIBLE_SECURITY_TOKEN_EXCEPTION_H
#include <comm_exports.h>
#include <VisItException.h>

// ****************************************************************************
// Class: IncompatibleSecurityTokenException
//
// Purpose:
//   This exception is used for cases where the two components exchange
//   security tokens that do not match.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 16 15:43:28 PST 2002
//
// Modifications:
//   
// ****************************************************************************

class COMM_API IncompatibleSecurityTokenException : public VisItException
{
public:
    IncompatibleSecurityTokenException() {;};
    virtual ~IncompatibleSecurityTokenException() VISIT_THROW_NOTHING {;};
};

#endif
