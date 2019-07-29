// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_FACTORY_H
#define VIEWER_FACTORY_H
#include <viewercore_exports.h>
#include <ViewerBase.h>

class OperatorPluginManager;
class PlotPluginManager;
class ViewerActionManager;
class ViewerChangeUsername;
class ViewerConnectionPrinter;
class ViewerConnectionProgress;
class ViewerEngineManagerInterface;
class ViewerFileServerInterface;
class ViewerHostProfileSelector;
class ViewerMessaging;
class ViewerWindow;
class VisWindow;

// ****************************************************************************
// Class: ViewerFactory
//
// Purpose:
//   Factory class for various object types.
//
// Notes:    The classes we produce here typically have non-UI and UI variants.
//           This particular factory produces the non-UI versions.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep  4 00:09:44 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API ViewerFactory : public ViewerBase
{
public:
    ViewerFactory();
    virtual ~ViewerFactory();

    virtual ViewerWindow        *CreateViewerWindow(int windowIndex);

    virtual VisWindow           *CreateVisWindow();

    virtual ViewerActionManager *CreateActionManager(ViewerWindow *win);

    virtual ViewerMessaging     *CreateViewerMessaging();

    virtual ViewerConnectionProgress *CreateConnectionProgress();

    virtual ViewerConnectionPrinter  *CreateConnectionPrinter();

    virtual ViewerFileServerInterface *CreateFileServerInterface();

    virtual ViewerEngineManagerInterface *CreateEngineManagerInterface();

    virtual ViewerHostProfileSelector *CreateHostProfileSelector();

    virtual ViewerChangeUsername *CreateChangeUsername();

    virtual PlotPluginManager *CreatePlotPluginManager();

    virtual OperatorPluginManager *CreateOperatorPluginManager();

    // Set some method override callbacks.
    void OverrideCreateConnectionPrinter(ViewerConnectionPrinter *(*cb)(void));

protected:
    void SetActionManagerLogic(ViewerActionManager *, ViewerWindow *);

    ViewerConnectionPrinter      *(*override_CreateConnectionPrinter)(void);
};

#endif

