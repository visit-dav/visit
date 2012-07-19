/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                               EngineProxy.C                               //
// ************************************************************************* //

#include <EngineProxy.h>

#include <AbortException.h>
#include <LostConnectionException.h>

// MCM -- 22Feb05: Hack to fix problem on SGI where reconstituting an
// exception would die in the throw. If we ever figure out why SGI needed
// this, we should remove this include directive
#include <InvalidVariableException.h>

#include <DebugStream.h>
#include <ExpressionList.h>
#include <ParentProcess.h>
#include <RemoteProcess.h>
#include <SocketConnection.h>
#include <StatusAttributes.h>
#include <TimingsManager.h>
#include <snprintf.h>

#include <stdio.h>

#include <string>
#include <vector>

using std::string;
using std::vector;

// ****************************************************************************
//  Method: EngineProxy constructor
//
//  Programmer: Eric Brugger
//  Creation:   July 26, 2000
//
//  Modifications:
//     Brad Whitlock, Fri Oct 20 12:50:32 PDT 2000
//     Added intialization of remoteUserName.
//
//     Brad Whitlock, Mon Apr 30 17:43:51 PST 2001
//     Added code to create the status attributes.
//
//     Brad Whitlock, Thu Sep 26 17:28:54 PST 2002
//     Initialized the progress callbacks.
//
//     Brad Whitlock, Wed Nov 27 14:10:03 PST 2002
//     I added numProcs, numNodes, and loadBalancing.
//
//     Brad Whitlock, Fri May 2 15:32:27 PST 2003
//     I made it inherit from RemoteProxyBase.
//
//     Jeremy Meredith, Tue Aug 24 22:12:21 PDT 2004
//     Added metadata and sil atts for simulations.
//
//     Brad Whitlock, Thu Jan 25 13:53:15 PST 2007
//     Added commandFromSim.
//
//     Brad Whitlock, Thu Apr  9 15:04:14 PDT 2009
//     Add engineP.
//
// ****************************************************************************

EngineProxy::EngineProxy(bool sim) : RemoteProxyBase("-engine")
{
    engineP = NULL;

    // Indicate that we want 2 write sockets from the engine.
    nWrite = sim ? 3 : 2;

    // Initialize the engine information that we can query.
    numProcs = 1;
    numNodes = -1;
    loadBalancing = 0;

    state = new EngineState();
    methods = new EngineMethods(state);

    // Create the status attributes that we use to communicate status
    // information to the client.
    simxfer = sim ? (new Xfer) : NULL;
    metaData = new avtDatabaseMetaData;
    silAtts = new SILAttributes;
    commandFromSim = new SimulationCommand;
}

// ****************************************************************************
//  Method: EngineProxy destructor
//
//  Programmer: Eric Brugger
//  Creation:   July 26, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Sep 29 19:18:02 PST 2000
//    Added code to delete command line arguments.
//
//    Hank Childs, Mon Oct 16 11:18:58 PDT 2000
//    Fixed up memory leak.
//
//    Brad Whitlock, Fri Oct 6 11:32:23 PDT 2000
//    I removed the SocketConnections.
//
//    Brad Whitlock, Fri Oct 20 12:51:14 PDT 2000
//    Added code to delete remoteUserName.
//
//    Brad Whitlock, Tue Apr 24 12:48:21 PDT 2001
//    Added code to delete the engine.
//
//    Brad Whitlock, Mon Apr 30 17:43:23 PST 2001
//    Added code to delete the status attributes.
//
//    Brad Whitlock, Fri May 2 15:33:21 PST 2003
//    I removed some members since they are now deleted in the base class.
//
//    Kathleen Bonnell, Tue Jan 11 16:06:33 PST 2005 
//    Delete metaData and silAtts.
//
//    Brad Whitlock, Thu Jan 25 13:53:36 PST 2007
//    Delete commandFromSim.
//
//    Brad Whitlock, Thu Apr  9 15:03:57 PDT 2009
//    Delete engineP.
//
// ****************************************************************************

EngineProxy::~EngineProxy()
{
    delete state;
    delete methods;

    delete simxfer;
    delete metaData;
    delete silAtts;
    delete commandFromSim;

    delete engineP;
}

// ****************************************************************************
// Method: EngineProxy::Create
//
// Purpose: 
//   This method is used to connect to an existing compute engine.
//
// Arguments:
//   args : The arguments that we need to connect back to the engine.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr  9 15:07:34 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
EngineProxy::Connect(const stringVector &args)
{
    char **argv = new char*[args.size()+1];
    for(size_t i = 0; i <= args.size(); ++i)
    {
        if(i < args.size())
            argv[i] = (char*)args[i].c_str();
        else
            argv[i] = NULL;
    }
    int argc = args.size();

    engineP = new ParentProcess;
    methods->SetParentProcess(engineP);

    int nwrite = (simxfer != NULL) ? 3 : 2;
    engineP->Connect(1, nwrite, &argc, &argv, true);
    delete [] argv;

    // Use engineP's connections for xfer.
    xfer.SetInputConnection(engineP->GetWriteConnection());
    xfer.SetOutputConnection(engineP->GetReadConnection());

    //
    // Set up the RPCs
    //
    SetupAllRPCs();

    //
    // List the objects that were hooked up. 
    //
    xfer.ListObjects();
}

// ****************************************************************************
// Method: EngineProxy::SetupComponentRPCs
//
// Purpose: 
//   Hook up the engine's RPC's to the xfer object. This is called after the
//   engine is launched.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 15:34:54 PST 2003
//
// Modifications:
//
//    Sean Ahern, Thu Nov 21 20:14:01 PST 2002
//    Removed AddNamedFunction (no longer needed).  Added observation of
//    the ExpressionList.
//
//    Sean Ahern, Wed Feb  5 15:34:04 PST 2003
//    Changed the interface to expression lists.
//
//    Sean Ahern, Tue Jul 29 12:58:08 PDT 2003
//    Added a notification for the expression list.
//   
//    Jeremy Meredith, Mon Sep 15 17:16:20 PDT 2003
//    Removed SetFinalVariableNameRPC.
//
//    Kathleen Bonnell, Wed Mar 31 17:23:01 PST 2004 
//    Added CloneNetworkRPC.
//
//    Jeremy Meredith, Tue Aug 24 22:12:21 PDT 2004
//    Added metadata and sil atts for simulations.
//
//    Hank Childs, Mon Feb 28 17:26:21 PST 2005
//    Added StartQuery.
//
//    Mark C. Miller, Tue Mar  8 17:59:40 PST 2005
//    Added ProcInfoRPC
//
//    Jeremy Meredith, Mon Apr  4 16:01:21 PDT 2005
//    Added simulationCommandRPC.
//
//    Hank Childs, Thu May 26 11:42:40 PDT 2005
//    Added exportDatabaseRPC.
//
//    Hank Childs, Mon Feb 13 22:21:42 PST 2006
//    Add constructDDFRPC.
//
//    Brad Whitlock, Thu Jan 25 13:54:02 PST 2007
//    Added commandFromSim.
//
//    Jeremy Meredith, Wed Jan 23 16:11:41 EST 2008
//    Added setEFileOpenOptionsRPC.
//
//    Hank Childs, Thu Jan 29 10:16:47 PST 2009
//    Added namedSelectionRPC.
//
//    Kathleen Biagas, Fri Jul 15 11:06:13 PDT 2011
//    Added queryParametersRPC.
//
//    Brad Whitlock, Mon Oct 10 12:11:42 PDT 2011
//    Added enginePropertiesRPC.
//
//    Brad Whitlock, Tue Nov 29 22:31:30 PST 2011
//    Add launchRPC.
//
// ****************************************************************************

void
EngineProxy::SetupComponentRPCs()
{
    //
    // Add RPCs to the transfer object.
    //

    state->SetupComponentRPCs(&xfer);
    xfer.Add(&state->exprList); //TODO: modify, Engine does something different, that is why this is public..

    //
    // Add other state objects to the transfer object
    //
    Xfer *x = &xfer;
    if(simxfer != NULL && component != NULL)
    {
        x = simxfer;
        simxfer->SetInputConnection(component->GetWriteConnection(2));
    }
    x->Add(metaData);
    x->Add(silAtts);
    x->Add(commandFromSim);

    // Extract some information about the engine from the command line
    // arguments that were used to create it.
    ExtractEngineInformation();
}

// ****************************************************************************
// Method: EngineProxy::ExtractEngineInformation
//
// Purpose: 
//   Extracts information about the engine from its command line arguments.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 15:38:22 PST 2003
//
// Modifications:
//
// ****************************************************************************

void
EngineProxy::ExtractEngineInformation()
{
    //
    // Look for certain key arguments in the command line arguments
    // and parse them out so we can query their values in the
    // engine proxy.
    //
    for(int i = 0; i < argv.size(); ++i)
    {
        if(argv[i] == "-np" && (i+1) < argv.size())
        {
           int np = 1;
           if(sscanf(argv[i+1].c_str(), "%d", &np) == 1)
           {
              if(np >= 1)
                  numProcs = np;
           }
           ++i;
        }
        else if(argv[i] == "-nn" && (i+1) < argv.size())
        {
           int nn = 1;
           if(sscanf(argv[i+1].c_str(), "%d", &nn) == 1)
           {
              if(nn >= 1)
              {
                  numNodes = nn;
                  methods->SetNumNodes(numNodes);
              }
           }
           ++i;
        }
        else if(argv[i] == "-forcestatic")
        {
           loadBalancing = 0;
        }
        else if(argv[i] == "-forcedynamic")
        {
           loadBalancing = 1;
        }
    }
}

// ****************************************************************************
//  Method:  EngineProxy::GetWriteSocket
//
//  Purpose:
//    Get the socket to receive input from the engine
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2004
//
// ****************************************************************************

int
EngineProxy::GetWriteSocket()
{
    if (simxfer != NULL && simxfer->GetInputConnection() != NULL)
        return simxfer->GetInputConnection()->GetDescriptor();
    else
        return -1;
}

// ****************************************************************************
//  Method:  EngineProxy::ReadDataAndProcess
//
//  Purpose:
//    Get some data fom the engine and process it.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2004
//
// ****************************************************************************

void
EngineProxy::ReadDataAndProcess()
{
    int amountRead = simxfer->GetInputConnection()->Fill();
    if (amountRead > 0)
        simxfer->Process();
    else
        EXCEPTION0(LostConnectionException);
}


// ****************************************************************************
//  Method:  EngineProxy::GetSimulationMetaData
//
//  Purpose:
//    Return the engine proxy metadata attributes for simulations.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2004
//
// ****************************************************************************

avtDatabaseMetaData *
EngineProxy::GetSimulationMetaData()
{
    return metaData;
}

// ****************************************************************************
//  Method:  EngineProxy::GetSimulationSILAtts
//
//  Purpose:
//    Return the engine proxy SIL attributes for simulations.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2004
//
// ****************************************************************************

SILAttributes *
EngineProxy::GetSimulationSILAtts()
{
    return silAtts;
}

// ****************************************************************************
//  Method:  EngineProxy::GetCommandFromSimulation
//
//  Purpose:
//    Return a command object that simulations can use to send the viewer
//    commands.
//
//  Arguments:
//    none
//
//  Programmer:  Brad Whitlock
//  Creation:    Thu Jan 25 13:55:06 PST 2007
//
// ****************************************************************************

SimulationCommand *
EngineProxy::GetCommandFromSimulation()
{
    return commandFromSim;
}


// ****************************************************************************
//  Method: EngineProxy::Interrupt
//
//  Purpose:
//    Interrupt execution of the current pipeline in the engine.
//
//  Programmer: Jeremy Meredith
//  Creation:   July  2, 2001
//
//  Modifications:
//
// ****************************************************************************

void
EngineProxy::Interrupt()
{
    xfer.SendInterruption();
}

// ****************************************************************************
// Method: EngineProxy::GetComponentName
//
// Purpose: 
//   Returns the name of this component.
//
// Returns:    The name of the component.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 6 10:47:35 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

string
EngineProxy::GetComponentName() const
{
    return "compute engine";
}

// ****************************************************************************
// Method: EngineProxy::SendKeepAlive
//
// Purpose: 
//   Sends a KeepAlive RPC to the compute engine and waits for a response.
//
// Note:       This method can cause a LostConnectionException if the engine
//             cannot be contacted.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 11:04:57 PDT 2004
//
// Modifications:
//   Brad Whitlock, Thu Apr  9 16:15:50 PDT 2009
//   I added reverse launch support.
//
// ****************************************************************************

void
EngineProxy::SendKeepAlive()
{
    debug3 << "Sending KeepAlive RPC to compute engine." << endl;

    //
    // Call the base class's SendKeepAlive method so the command sockets
    // will be exercised.
    //
    RemoteProxyBase::SendKeepAlive();

    //
    // Now read some input back from the engine's data socket.
    //
#define KEEPALIVE_SIZE 10
    unsigned char buf[KEEPALIVE_SIZE];
    if(engineP != NULL)
    {
        if (engineP->GetReadConnection(1)->DirectRead(buf, KEEPALIVE_SIZE) < 0)
            debug1 << "Error reading keep alive data from engine!!!!\n";
    }
    else
    {
        if (component->GetWriteConnection(1)->DirectRead(buf, KEEPALIVE_SIZE) < 0)
            debug1 << "Error reading keep alive data from engine!!!!\n";
    }
}

void
EngineProxy::Create(const std::string &hostName,
            MachineProfile::ClientHostDetermination chd,
            const std::string &clientHostName,
            bool manualSSHPort,
            int sshPort,
            bool useTunneling,
            bool useGateway,
            const std::string &gatewayHost,
            ConnectCallback *connectCallback,
            void *data, bool createAsThoughLocal)
{
    RemoteProxyBase::Create(hostName,
                            chd,
                            clientHostName,
                            manualSSHPort,
                            sshPort,
                            useTunneling,
                            useGateway,
                            gatewayHost,
                            connectCallback,
                            data,
                            createAsThoughLocal);
    methods->SetRemoteProcess(component);
}
