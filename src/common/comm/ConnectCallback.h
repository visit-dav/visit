#ifndef CONNECT_CALLBACK_H
#define CONNECT_CALLBACK_H
#include <vectortypes.h>

//
// This is a type of callback function that is used by the 
// ExistingRemoteProcess class when trying to connect to a remote process.
//
// Modifications:
//   Brad Whitlock, Thu Dec 19 11:35:47 PDT 2002
//   I added the key argument.
//
//   Brad Whitlock, Mon May 5 14:00:28 PST 2003
//   I replaced the individual arguments with stringVector.
//

typedef void (ConnectCallback)(const std::string &remoteHost,
    const stringVector &args, void *data);

#endif
