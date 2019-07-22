// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_MESSAGING_UI_H
#define VIEWER_MESSAGING_UI_H
#include <viewer_exports.h>
#include <ViewerMessaging.h>

// ****************************************************************************
// Class: ViewerMessagingUI
//
// Purpose:
//   Handles messaging within the viewer.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 10:38:37 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWER_API ViewerMessagingUI : public ViewerMessaging
{
public:
    ViewerMessagingUI();
    virtual ~ViewerMessagingUI();

    // Message box
    virtual bool InformationBox(const ViewerText &title, const ViewerText &msg);
protected:
    // Helper functions that we can override.
    virtual void SetMessageAttributesText(const ViewerText &message);
    virtual void SetStatusAttributesMessage(const ViewerText &message);
};

#endif
