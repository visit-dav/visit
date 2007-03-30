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
