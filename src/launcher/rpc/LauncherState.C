#include <LauncherState.h>
#include <LaunchRPC.h>
#include <ConnectSimRPC.h>
#include <Xfer.h>

LauncherState::LauncherState()
{
    objVector.push_back(&launchRPC);
    objVector.push_back(&connectSimRPC);
}

LauncherState::~LauncherState()
{
}

void
LauncherState::SetupComponentRPCs(Xfer* xfer)
{
    //
    // Connect the RPCs to the xfer object.
    //
    for(size_t i = 0; i < GetNumStateObjects(); ++i)
        xfer->Add(GetStateObject(i));
}

size_t
LauncherState::GetNumStateObjects()
{
    return objVector.size();
}

VisItRPC*
LauncherState::GetStateObject(int i)
{
    return (i >= 0 && (size_t)i < objVector.size()) ?
           objVector[i] : 0;
}


