/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                                ViewerProxy.h                              //
// ************************************************************************* //

#ifndef VIEWER_PROXY_H
#define VIEWER_PROXY_H
#include <viewerproxy_exports.h>
#include <ViewerMethods.h>
#include <ViewerState.h>
#include <SimpleObserver.h>
#include <avtSILRestriction.h>
#include <PlotPluginManager.h>
#include <OperatorPluginManager.h>
#include <vectortypes.h>

class Connection;
class ParentProcess;
class RemoteProcess;
class Xfer;

// ****************************************************************************
//  Class: ViewerProxy
//
//  Purpose:
//    ViewerProxy is a proxy class for creating and controlling a viewer.
//
//  Note:
//
//  Programmer: Eric Brugger
//  Creation:   August 4, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Feb 9 18:35:35 PST 2007
//    I moved a lot of code to the ViewerState and ViewerInterface classes and
//    I removed old modification comments.
//
//    Brad Whitlock, Tue Jun 24 11:21:40 PDT 2008
//    Added plugin managers since they are no longer singletons.
//
// ****************************************************************************

class VIEWER_PROXY_API ViewerProxy : public SimpleObserver
{
  public:
    ViewerProxy();
    virtual ~ViewerProxy();

    Connection *GetReadConnection() const;
    Connection *GetWriteConnection() const;
    const std::string &GetLocalHostName() const;
    const std::string &GetLocalUserName() const;
    void ProcessInput();

    void AddArgument(const std::string &arg);
    void Create(int *argc = 0, char ***argv = 0);
    void Create(const char *, int *argc = 0, char ***argv = 0);
    void Close();
    void Detach();

    void InitializePlugins(PluginManager::PluginCategory t, const char *pluginDir=0);
    void LoadPlugins();
    PlotPluginManager     *GetPlotPluginManager() const;
    OperatorPluginManager *GetOperatorPluginManager() const;

    // Get the proxy's ViewerState object which contains the state objects
    // used in the viewer/client communication interface.
    ViewerState   *GetViewerState() const;

    // Get the proxy's ViewerMethods object which contains the methods that
    // are converted into ViewerRPC calls in the viewer.
    ViewerMethods *GetViewerMethods() const;

    // Methods for dealing with plot SIL restrictions.
    avtSILRestriction_p GetPlotSILRestriction() 
                     { return internalSILRestriction; };
    avtSILRestriction_p GetPlotSILRestriction() const
                     { return new avtSILRestriction(internalSILRestriction); };
    void SetPlotSILRestriction(avtSILRestriction_p newRestriction);
    void SetPlotSILRestriction();

    // Convenience methods 
    void AnimationStop();
    int MethodRequestHasRequiredInformation() const;
    void InterruptComputeEngine(const std::string &hostName,
                                const std::string &simName);

    // Don't use this method unless absolutely necessary.
    void SetXferUpdate(bool val);

  protected:
    virtual void Update(Subject *subj);
  private:
    void ConnectXfer();

    RemoteProcess              *viewer;
    ParentProcess              *viewerP;
    Xfer                       *xfer;
    ViewerMethods              *methods;
    ViewerState                *state;
    PlotPluginManager          *plotPlugins;
    OperatorPluginManager      *operatorPlugins;

    int                        animationStopOpcode;
    int                        iconifyOpcode;

    // Extra command line arguments to pass to the viewer.
    stringVector               argv;

    // Used to store the sil restriction in avt format.
    avtSILRestriction_p        internalSILRestriction;
};

#endif
