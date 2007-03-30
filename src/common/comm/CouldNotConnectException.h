#ifndef COULD_NOT_CONNECT_EXCEPTION_H
#define COULD_NOT_CONNECT_EXCEPTION_H
#include <comm_exports.h>
#include <VisItException.h>

// ****************************************************************************
// Class: CouldNotConnectException
//
// Purpose:
//   This exception is used for cases where a client program could not connect.
//
// Programmer: Jeremy Meredith
// Creation:   April 27, 2001
//
// Modifications:
//   
// ****************************************************************************

class COMM_API CouldNotConnectException : public VisItException
{
public:
    CouldNotConnectException() {;};
    virtual ~CouldNotConnectException() VISIT_THROW_NOTHING {;};
};

#endif
