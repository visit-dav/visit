#include <BadHostException.h>

// *******************************************************************
// Method: BadHostException::BadHostException
//
// Purpose: 
//   Constructor for the BadHostException class.
//
// Arguments:
//   host : The bad host name.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 25 12:47:43 PDT 2000
//
// Modifications:
//   
// *******************************************************************

BadHostException::BadHostException(const std::string &badHost) :
    hostName(badHost)
{
    msg = badHost + " is an invalid host name.";
}

// *******************************************************************
// Method: BadHostException::GetHostName
//
// Purpose: 
//   Returns the bad hostname stored in the exception.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 25 12:48:39 PDT 2000
//
// Modifications:
//   
// *******************************************************************

const std::string &
BadHostException::GetHostName() const
{
    return hostName;
}
