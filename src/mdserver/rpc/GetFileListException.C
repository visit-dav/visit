// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <GetFileListException.h>

// *******************************************************************
// Method: GetFileListException::GetFileListException
//
// Purpose: 
//   Constructor for the GetFileListException class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 25 12:47:43 PDT 2000
//
// Modifications:
//   
// *******************************************************************

GetFileListException::GetFileListException()
{
    msg = "Can't get the file list.";
}
