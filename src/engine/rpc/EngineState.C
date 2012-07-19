#include <EngineState.h>

#include <Xfer.h>

EngineState::EngineState()
{
    objVector.push_back(&readRPC);
    objVector.push_back(&applyOperatorRPC);
    objVector.push_back(&makePlotRPC);
    objVector.push_back(&useNetworkRPC);
    objVector.push_back(&updatePlotAttsRPC);
    objVector.push_back(&pickRPC);
    objVector.push_back(&startPickRPC);
    objVector.push_back(&startQueryRPC);
    objVector.push_back(&executeRPC);
    objVector.push_back(&clearCacheRPC);
    objVector.push_back(&queryRPC);
    objVector.push_back(&queryParametersRPC);
    objVector.push_back(&releaseDataRPC);
    objVector.push_back(&openDatabaseRPC);
    objVector.push_back(&defineVirtualDatabaseRPC);
    objVector.push_back(&renderRPC);
    objVector.push_back(&setWinAnnotAttsRPC);
    objVector.push_back(&cloneNetworkRPC);
    objVector.push_back(&procInfoRPC);
    objVector.push_back(&simulationCommandRPC);
    objVector.push_back(&exportDatabaseRPC);
    objVector.push_back(&constructDataBinningRPC);
    objVector.push_back(&namedSelectionRPC);
    objVector.push_back(&setEFileOpenOptionsRPC);
    objVector.push_back(&enginePropertiesRPC);
    objVector.push_back(&launchRPC);
    statusAtts  = new StatusAttributes;
}

EngineState::~EngineState()
{
    delete statusAtts;
}

void
EngineState::SetupComponentRPCs(Xfer* xfer)
{
    //
    // Add RPCs to the transfer object.
    //
    for(int i = 0; i < GetNumStateObjects(); ++i)
        xfer->Add(GetStateObject(i));
    //xfer->Add(&exprList); TODO: PUT THIS BACK when Engine.C xfer->Add(l->GetList()); is fixed..
}

size_t
EngineState::GetNumStateObjects()
{
    return objVector.size();
}

VisItRPC*
EngineState::GetStateObject(int i)
{
    return (i >= 0 && i < objVector.size()) ?
           objVector[i] : 0;
}

