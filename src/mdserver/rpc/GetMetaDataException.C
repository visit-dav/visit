#include <GetMetaDataException.h>

// *******************************************************************
// Method: GetMetaDataException::GetMetaDataException
//
// Purpose: 
//   Constructor for the GetMetaDataException class.
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   
// *******************************************************************

GetMetaDataException::GetMetaDataException()
{
    msg = "Can't get the metadata.";
}

// *******************************************************************
// Method: GetMetaDataException::GetMetaDataException
//
// Purpose: 
//   Constructor for the GetMetaDataException class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 7 11:15:06 PDT 2002
//
// Modifications:
//   
// *******************************************************************

GetMetaDataException::GetMetaDataException(const std::string &str)
{
    msg = str;
}
