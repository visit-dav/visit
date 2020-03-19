// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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

class COMM_API2 CouldNotConnectException : public VisItException
{
public:
    CouldNotConnectException() {;};
    CouldNotConnectException(const std::string &s) : VisItException(s) {;}
    virtual ~CouldNotConnectException() VISIT_THROW_NOTHING {;};
};

#endif
