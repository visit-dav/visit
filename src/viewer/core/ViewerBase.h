// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_BASE_H
#define VIEWER_BASE_H
#include <viewercore_exports.h>

class OperatorPluginManager;
class PlotPluginManager;
class ViewerEngineManagerInterface;
class ViewerFactory;
class ViewerFileServerInterface;
class ViewerMessaging;
class ViewerMethods;
class ViewerOperatorFactory;
class ViewerPlotFactory;
class ViewerProperties;
class ViewerState;
class ViewerStateManager;

// ****************************************************************************
// Class: ViewerBase
//
// Purpose:
//   Base class for viewer classes
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 12 09:25:39 PDT 2007
//
// Modifications:   
//   Cyrus Harrison, Thu Feb 21 15:04:14 PST 2008
//   Added control to suppress messages. 
//
//   Brad Whitlock, Tue Apr 29 11:11:04 PDT 2008
//   Converted most messaging to QString.
//
//   Brad Whitlock, Fri May  9 14:34:56 PDT 2008
//   Qt 4.
//
//   Brad Whitlock, Tue Jun 24 14:33:17 PDT 2008
//   Added methods to return the plugin managers.
//
//   Brad Whitlock, Tue Apr 14 11:12:07 PDT 2009
//   I added GetViewerProperties. I made the message suppression stuff use the
//   new ViewerProperties.
//
//   Brad Whitlock, Fri Aug 22 11:51:12 PDT 2014
//   I added plot and operator factories and state manager.
//
// ****************************************************************************

class VIEWERCORE_API ViewerBase
{
public:
    ViewerBase();
    virtual ~ViewerBase();

    static void SetViewerFactory(ViewerFactory *);

    static ViewerFactory                *GetViewerFactory();
    static ViewerState                  *GetViewerState();
    static ViewerMethods                *GetViewerMethods();
    static ViewerMessaging              *GetViewerMessaging();
    static PlotPluginManager            *GetPlotPluginManager();
    static ViewerPlotFactory            *GetPlotFactory();
    static OperatorPluginManager        *GetOperatorPluginManager();
    static ViewerOperatorFactory        *GetOperatorFactory();
    static ViewerProperties             *GetViewerProperties();
    static ViewerStateManager           *GetViewerStateManager();
    static ViewerFileServerInterface    *GetViewerFileServer();
    static ViewerEngineManagerInterface *GetViewerEngineManager();

protected:
    static void SetProcessEventsCallback(void (*cb)(void*), void *cbdata);
    static void (*ProcessEventsCB)(void *);
    static void  *ProcessEventsCBData;

private:
    static ViewerFactory                *base_viewerFactory;
    static ViewerMethods                *base_viewerMethods;
    static ViewerState                  *base_viewerState;
    static PlotPluginManager            *base_plotPlugins;
    static ViewerPlotFactory            *base_plotFactory;
    static OperatorPluginManager        *base_operatorPlugins;
    static ViewerOperatorFactory        *base_operatorFactory;
    static ViewerProperties             *base_viewerProperties;
    static ViewerStateManager           *base_viewerStateManager;
    static ViewerMessaging              *base_viewerMessaging;
    static ViewerFileServerInterface    *base_viewerFileServer;
    static ViewerEngineManagerInterface *base_viewerEngineManager;
};

#endif
