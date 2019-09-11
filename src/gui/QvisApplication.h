// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_APPLICATION_H
#define QVIS_APPLICATION_H
#include <QApplication>

#if defined(Q_OS_MAC)
class QEventLoop;
#endif

// ****************************************************************************
// Class: QvisApplication
//
// Purpose:
//   This class inherits from QApplication so it can redefine some event
//   handling methods on MacOS X. Otherwise it is the same as QApplication.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 4 10:17:21 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Oct 9 15:16:34 PST 2007
//   Changed signature for macEventFilter to match newer Qt method.
//
//   Brad Whitlock, Fri May 30 11:50:32 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class QvisApplication : public QApplication
{
    Q_OBJECT
public:
    QvisApplication(int &argc, char **argv);
    QvisApplication(int &argc, char **argv, bool GUIenabled);
    virtual ~QvisApplication();
    
signals:
    void showApplication();
    void hideApplication();
private slots:
    void exitTheLoop();

#if defined(Q_OS_MAC)
public:
    bool         needToMakeActive;
    QEventLoop  *eventLoop;
#endif
};
#endif
