#ifndef INCOMPATIBLE_VERSION_EXCEPTION_H
#define INCOMPATIBLE_VERSION_EXCEPTION_H
#include <comm_exports.h>
#include <VisItException.h>

// ****************************************************************************
// Class: IncompatibleVersionException
//
// Purpose:
//   This exception is used for cases where the two components exchange
//   version numbers that do not match.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Apr 25 11:10:29 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

class COMM_API IncompatibleVersionException : public VisItException
{
public:
    IncompatibleVersionException() {;};
    virtual ~IncompatibleVersionException() VISIT_THROW_NOTHING {;};
};

#endif
