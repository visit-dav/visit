#include <MDServerState.h>

#include <GetDirectoryRPC.h>
#include <ChangeDirectoryRPC.h>
#include <GetFileListRPC.h>
#include <GetMetaDataRPC.h>
#include <GetSILRPC.h>
#include <ConnectRPC.h>
#include <CreateGroupListRPC.h>
#include <ExpandPathRPC.h>
#include <CloseDatabaseRPC.h>
#include <LoadPluginsRPC.h>
#include <GetPluginErrorsRPC.h>
#include <GetDBPluginInfoRPC.h>
#include <SetMFileOpenOptionsRPC.h>
#include <Xfer.h>

#include <visit-config.h>

MDServerState::MDServerState()
{
    separator = VISIT_SLASH_CHAR;

    objVector.push_back(&getDirectoryRPC);
    objVector.push_back(&changeDirectoryRPC);
    objVector.push_back(&getFileListRPC);
    objVector.push_back(&getMetaDataRPC);
    objVector.push_back(&getSILRPC);
    objVector.push_back(&connectRPC);
    objVector.push_back(&createGroupListRPC);
    objVector.push_back(&expandPathRPC);
    objVector.push_back(&closeDatabaseRPC);
    objVector.push_back(&loadPluginsRPC);
    objVector.push_back(&getPluginErrorsRPC);
    objVector.push_back(&getDBPluginInfoRPC);
    objVector.push_back(&setMFileOpenOptionsRPC);
}

MDServerState::~MDServerState()
{
}

void
MDServerState::DetermineSeparator()
{
    // Get the startup directory.
    std::string d = getDirectoryRPC();

    // Determine the separator to use in filenames.
    for(size_t i = 0; i < d.length(); ++i)
    {
        if(d[i] == '/')
        {
            separator = '/';
            break;
        }
        else if(d[i] == '\\')
        {
            separator = '\\';
            break;
        }
    }
}

void
MDServerState::SetupComponentRPCs(Xfer* xfer)
{
    //
    // Connect the RPCs to the xfer object.
    //

    for(size_t i = 0; i < GetNumStateObjects(); ++i)
        xfer->Add(GetStateObject(i));

}

size_t
MDServerState::GetNumStateObjects()
{
    return objVector.size();
}

VisItRPC*
MDServerState::GetStateObject(int i)
{
    return (i >= 0 && (size_t)i < objVector.size()) ?
           objVector[i] : 0;
}
