// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CANCELLED_CONNECT_EXCEPTION_H
#define CANCELLED_CONNECT_EXCEPTION_H
#include <comm_exports.h>
#include <VisItException.h>

// ****************************************************************************
// Class: CancelledConnectException
//
// Purpose:
//   This exception is used for cases where we abort the connection to a
//   remote program while it is being launched.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 27 11:32:20 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

class COMM_API2 CancelledConnectException : public VisItException
{
public:
    CancelledConnectException() {;};
    virtual ~CancelledConnectException() VISIT_THROW_NOTHING {;};
};

#endif
