// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_STATE_BUFFERED_H
#define VIEWER_STATE_BUFFERED_H
#include <ViewerBaseUI.h>
#include <SimpleObserver.h>

class AttributeSubject;
class ViewerClientConnection;
class ViewerState;

// ****************************************************************************
// Class: ViewerStateBuffered
//
// Purpose:
//   Contains a ViewerState and any time its state objects change, we emit
//   the InputFromClient signal.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 20 15:49:48 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class ViewerStateBuffered : public ViewerBaseUI, public SimpleObserver
{
    Q_OBJECT
public:
    ViewerStateBuffered(ViewerState *);
    virtual ~ViewerStateBuffered();

    virtual void Update(Subject *);

    ViewerState *GetState() { return viewerState; }
signals:
    void InputFromClient(ViewerClientConnection *, AttributeSubject *);
private:
    ViewerState *viewerState;
};

#endif
