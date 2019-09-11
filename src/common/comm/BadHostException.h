// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef BAD_HOST_EXCEPTION_H
#define BAD_HOST_EXCEPTION_H
#include <comm_exports.h>
#include <string>
#include <VisItException.h>

// *******************************************************************
// Class: BadHostException
//
// Purpose:
//   The exception that should be thrown when a bad host is specified.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 25 12:40:50 PDT 2000
//
// Modifications:
//   
// *******************************************************************

class COMM_API2 BadHostException : public VisItException
{
public:
    BadHostException(const std::string &host);
    virtual ~BadHostException() VISIT_THROW_NOTHING {;};

    const std::string &GetHostName() const;
private:
    std::string hostName;
};

#endif
