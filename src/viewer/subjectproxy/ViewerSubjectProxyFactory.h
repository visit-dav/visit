// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_SUBJECT_PROXY_FACTORY_UI_H
#define VIEWER_SUBJECT_PROXY_FACTORY_UI_H
#include <ViewerFactoryUI.h>

// ****************************************************************************
// Class: ViewerSubjectProxyFactory
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

class ViewerSubjectProxyFactory : public ViewerFactoryUI
{
public:
    ViewerSubjectProxyFactory();
    virtual ~ViewerSubjectProxyFactory();

    virtual ViewerWindow *CreateViewerWindow(int windowIndex);

    void SetCreateViewerWindowCallback(void (*cb)(ViewerWindow *, void *),
                                       void *cbdata);
private:
    void (*createWindowCB)(ViewerWindow *, void *);
    void *createWindowCBData;
};

#endif
