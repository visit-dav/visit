// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CHANGE_USERNAME_EXCEPTION_H
#define CHANGE_USERNAME_EXCEPTION_H

#include <comm_exports.h>

#include <VisItException.h>


// ****************************************************************************
// Class: ChangeUsernameException
//
// Purpose:
//   This exception is used for cases where we want to change the username
//   when trying to connect to a remote program.
//
// Programmer: Hank Childs
// Creation:   November 11, 2007
//
// Modifications:
//   
// ****************************************************************************

class COMM_API2 ChangeUsernameException : public VisItException
{
public:
    ChangeUsernameException() {;};
    virtual ~ChangeUsernameException() VISIT_THROW_NOTHING {;};
};

#endif


