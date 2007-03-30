#ifndef LOST_CONNECTION_EXCEPTION_H
#define LOST_CONNECTION_EXCEPTION_H
#include <comm_exports.h>
#include <VisItException.h>

// ****************************************************************************
// Class: LostConnectionException
//
// Purpose:
//   This exception is used for cases where a client program is lost.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 15 12:18:25 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

class COMM_API LostConnectionException : public VisItException
{
public:
    LostConnectionException() {;};
    virtual ~LostConnectionException() VISIT_THROW_NOTHING {;};
};

#endif
