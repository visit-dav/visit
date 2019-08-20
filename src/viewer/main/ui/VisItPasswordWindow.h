// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISITPASSWORDWINDOW_H
#define VISITPASSWORDWINDOW_H
#include <QDialog>

// Forward declarations
class QLineEdit;
class QLabel;

// ****************************************************************************
//  Class:  VisItPasswordWindow
//
//  Purpose:
//    Main window for the program.
//
//  Programmer:  Brad Whitlock
//  Creation:    June 12, 2012
//
//  Modifications:
//
// ****************************************************************************

class VisItPasswordWindow : public QDialog
{
    Q_OBJECT
public:
    VisItPasswordWindow(QWidget *parent=NULL);
    virtual ~VisItPasswordWindow();

    typedef enum {
        PW_Accepted,
        PW_Rejected,
        PW_ChangedUsername
    } ReturnCode;

    QString getPassword(const QString &host, const QString &username,
                        QString phrase, ReturnCode &ret);

private slots:
    void changeUserName();
private:
    QLineEdit *passedit;
    QLabel    *label;
    bool       changedUserName;                
};

#endif
