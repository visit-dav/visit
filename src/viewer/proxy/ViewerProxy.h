// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    Hari Krishnan, Tue Oct 18 11:25:40 PDT 2011
//    Virtualized several functions and added static class to create
//    ViewerProxy.
// ****************************************************************************

class VIEWER_PROXY_API ViewerProxy : public SimpleObserver
{
  public:
    ViewerProxy();
    virtual ~ViewerProxy();

    virtual Connection *GetReadConnection() const;
    virtual Connection *GetWriteConnection() const;
    virtual const std::string &GetLocalHostName() const;
    virtual const std::string &GetLocalUserName() const;
    virtual void ProcessInput();

    virtual void AddArgument(const std::string &arg);
    virtual void Create(int *argc = 0, char ***argv = 0);
    virtual void Create(const char *, int *argc = 0, char ***argv = 0);
    virtual void Close();
    virtual void Detach();

    virtual void InitializePlugins(PluginManager::PluginCategory t, const char *pluginDir=0);
    virtual void LoadPlugins();
    virtual PlotPluginManager     *GetPlotPluginManager() const;
    virtual OperatorPluginManager *GetOperatorPluginManager() const;

    // Get the proxy's ViewerState object which contains the state objects
    // used in the viewer/client communication interface.
    virtual ViewerState   *GetViewerState() const;

    // Get the proxy's ViewerMethods object which contains the methods that
    // are converted into ViewerRPC calls in the viewer.
    virtual ViewerMethods *GetViewerMethods() const;

    // Methods for dealing with plot SIL restrictions.
    virtual avtSILRestriction_p GetPlotSILRestriction()
                     { return internalSILRestriction; };
    virtual avtSILRestriction_p GetPlotSILRestriction() const
                     { return new avtSILRestriction(internalSILRestriction); };
    virtual void SetPlotSILRestriction(avtSILRestriction_p newRestriction);
    virtual void SetPlotSILRestriction();

    // Convenience methods 
    void AnimationStop();
    int MethodRequestHasRequiredInformation() const;
    void InterruptComputeEngine(const std::string &hostName,
                                const std::string &simName);

    // Don't use this method unless absolutely necessary.
    void SetXferUpdate(bool val);
    
    //Static function to create ViewerProxy
    //this allows other libraries to override creation of ViewerProxy
    //namely PySide which is dynamically loaded from a separate module
    static ViewerProxy* CreateViewerProxy(ViewerProxy* = NULL);

  protected:
    virtual void Update(Subject *subj);
  private:
    void ConnectXfer();
    bool ConnectToExistingViewer(const std::string& host,
                                 const int& port,
                                 const std::string& password);

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
