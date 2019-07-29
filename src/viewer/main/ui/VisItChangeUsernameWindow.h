// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISITCHANGEUSERNAMEWINDOW_H
#define VISITCHANGEUSERNAMEWINDOW_H
#include <QDialog>

// Forward declarations
class QLineEdit;
class QLabel;

// ****************************************************************************
//  Class:  VisItChangeUsernameWindow
//
//  Purpose:
//     A window where the user can set the username for a host.
//
//  Programmer:  Brad Whitlock
//  Creation:    Tue Jun 12 14:31:34 PST 2012
//
//  Modifications:
//
// ****************************************************************************

class VisItChangeUsernameWindow : public QDialog
{
    Q_OBJECT
public:
    VisItChangeUsernameWindow(QWidget *parent = 0);
    virtual ~VisItChangeUsernameWindow();

    typedef enum {
        UW_Accepted,
        UW_Rejected
    } ReturnCode;

    QString getUsername(const QString &hostname, ReturnCode &ret);
private:
    QLineEdit *usernameedit;
    QLabel    *label;
};

#endif
