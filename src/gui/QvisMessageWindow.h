// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_MESSAGE_WINDOW_H
#define QVIS_MESSAGE_WINDOW_H
#include <gui_exports.h>
#include <QvisWindowBase.h>
#include <Observer.h>

class QLabel;
class QTextEdit;

// *******************************************************************
// Class: QvisMessageWindow
//
// Purpose:
//   This window observes a MessageAttributes state object and prints
//   the message in the state object into the window when it is
//   updated.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 18:09:22 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Jan 18 15:32:51 PST 2008
//   Added preserveInformation flag.
//
//   Brad Whitlock, Wed Apr  9 10:54:00 PDT 2008
//   QString for captionString.
//
//   Brad Whitlock, Fri May 30 14:27:35 PDT 2008
//   Qt 4.
//
//   Eric Brugger, Tue Aug 24 13:27:39 PDT 2010
//   Added a control to enable/disable the popping up of warning
//   messages.
//
// *******************************************************************

class GUI_API QvisMessageWindow : public QvisWindowBase, public Observer
{
    Q_OBJECT
public:
    QvisMessageWindow(MessageAttributes *msgAttr,
                      const QString &captionString = QString());
    virtual ~QvisMessageWindow();
    virtual void Update(Subject *);

    void SetEnableWarningPopups(bool val);
public slots:
    void EnableWarningPopups(bool);
private slots:
    void doHide();
private:
    QLabel    *severityLabel;
    QTextEdit *messageText;
    bool       preserveInformation;
    bool       enableWarningPopups;
};

#endif
