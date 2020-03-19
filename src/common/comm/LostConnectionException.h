// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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

class COMM_API2 LostConnectionException : public VisItException
{
public:
    LostConnectionException() {;};
    virtual ~LostConnectionException() VISIT_THROW_NOTHING {;};
};

#endif
