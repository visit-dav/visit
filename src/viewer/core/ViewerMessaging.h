// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_MESSAGING_H
#define VIEWER_MESSAGING_H
#include <viewercore_exports.h>
#include <ViewerBase.h>
#include <ViewerText.h>

#include <deque>

class ViewerInternalCommand;

// ****************************************************************************
// Class: ViewerMessaging
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

class VIEWERCORE_API ViewerMessaging : public ViewerBase
{
public:
    ViewerMessaging();
    virtual ~ViewerMessaging();

    void BlockClientInput(bool b);
    bool ClientInputBlocked() const;

    // Methods for internal commands.
    ViewerInternalCommand *GetCommand();
    void                   QueueCommand(ViewerInternalCommand *cmd, int timeout = 1);
    void                   ProcessCommands();
    void                   SetCommandsNotifyCallback(void (*cb)(void *, int),
                                                     void *cbdata);

    // Methods to send messages
    void Error(const ViewerText &message);
    void Warning(const ViewerText &message);
    void Message(const ViewerText &message);
    void ErrorClear();

    // Methods to send status
    void Status(const ViewerText &message);
    void Status(const ViewerText &message, int milliseconds);
    void Status(const ViewerText &sender, const ViewerText &message);
    void Status(const ViewerText &sender, int percent, int curStage,
                        const ViewerText &curStageName, int maxStage);
    void ClearStatus(const ViewerText &sender = ViewerText());
    
    // Message Suppression Control
    void EnableMessageSuppression();
    void DisableMessageSuppression();
    bool SuppressMessages();

    // Message box
    virtual bool InformationBox(const ViewerText &title, const ViewerText &msg);
protected:
    // Helper functions that we can override.
    virtual void SetMessageAttributesText(const ViewerText &message);
    virtual void SetStatusAttributesMessage(const ViewerText &message);
private:
    std::deque<ViewerInternalCommand *> commands;
    bool blockSocketSignals;
    void (*commandsNotifyCB)(void *, int);
    void *commandsNotifyCBData;
};

#endif
