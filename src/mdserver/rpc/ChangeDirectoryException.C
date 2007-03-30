#include <ChangeDirectoryException.h>

// *******************************************************************
// Method: ChangeDirectoryException::ChangeDirectoryException
//
// Purpose: 
//   Constructor for the ChangeDirectoryException class.
//
// Arguments:
//   dir : The bad directory name.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 25 12:47:43 PDT 2000
//
// Modifications:
//   
// *******************************************************************

ChangeDirectoryException::ChangeDirectoryException(const std::string &dir) :
    directoryName(dir)
{
    msg = dir + " is an invalid path.";
}

// *******************************************************************
// Method: ChangeDirectoryException::GetDirectory
//
// Purpose: 
//   Returns the bad directory name stored in the exception.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 25 12:48:39 PDT 2000
//
// Modifications:
//   
// *******************************************************************

const std::string &
ChangeDirectoryException::GetDirectory() const
{
    return directoryName;
}
