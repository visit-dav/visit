// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_FACTORY_UI_H
#define VIEWER_FACTORY_UI_H
#include <viewer_exports.h>
#include <ViewerFactory.h>

// ****************************************************************************
// Class: ViewerFactoryUI
//
// Purpose:
//   Creates various viewer objects.
//
// Notes:    The UI versions get created.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 21 23:56:40 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWER_API ViewerFactoryUI : public ViewerFactory
{
public:
    ViewerFactoryUI();
    virtual ~ViewerFactoryUI();

    virtual ViewerWindow        *CreateViewerWindow(int windowIndex);

    virtual VisWindow           *CreateVisWindow();

    virtual ViewerActionManager *CreateActionManager(ViewerWindow *win);

    virtual ViewerMessaging     *CreateViewerMessaging();

    virtual ViewerConnectionProgress *CreateConnectionProgress();

    virtual ViewerConnectionPrinter  *CreateConnectionPrinter();

    virtual ViewerHostProfileSelector *CreateHostProfileSelector();

    virtual ViewerChangeUsername *CreateChangeUsername();

    virtual ViewerFileServerInterface *CreateFileServerInterface();

    virtual ViewerEngineManagerInterface *CreateEngineManagerInterface();

protected:
    void SetActionManagerUI(ViewerActionManager *, ViewerWindow *);
    void SetActionManagerLogicUI(ViewerActionManager *, ViewerWindow *);
};

#endif
