// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_MESSAGE_DIALOG_H
#define QVIS_MESSAGE_DIALOG_H
#include <QDialog>

class QLabel;
class QTextEdit;

class MessageAttributes;

// *******************************************************************
// Class: QvisMessageDialog
//
// Purpose:
//   This window displays a message to the user.
//
// Programmer: Eric Brugger
// Creation:   Thu Feb 12 15:53:33 PST 2009
//
// Modifications:
//
// *******************************************************************

class QvisMessageDialog : public QDialog
{
    Q_OBJECT
public:
    QvisMessageDialog();
    virtual ~QvisMessageDialog();

    void DisplayMessage(MessageAttributes *);

private:
    QLabel    *severityLabel;
    QTextEdit *messageText;
};

#endif
