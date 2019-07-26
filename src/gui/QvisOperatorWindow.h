// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_OPERATOR_WINDOW_H
#define QVIS_OPERATOR_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>

// ****************************************************************************
// Class: QvisOperatorWindow
//
// Purpose:
//   This is the base class for all operator windows.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 12 12:32:00 PDT 2002
//
// Modifications:
//   Brad Whitlock, Mon Apr 15 13:47:21 PST 2002
//   Added GUI exports.
//
//   Brad Whitlock, Thu Mar 6 11:28:59 PDT 2003
//   I added stretch.
//
//   Brad Whitlock, Wed Apr  9 12:47:17 PDT 2008
//   QString for caption, shortName.
//
// ****************************************************************************

class GUI_API QvisOperatorWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisOperatorWindow(const int type, Subject *subj,
                       const QString &caption,
                       const QString &shortName, 
                       QvisNotepadArea *notepad, bool stretch=true);
    virtual ~QvisOperatorWindow();
public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
protected:
    void         Apply(bool ignore = false);
    virtual void GetCurrentValues(int which_widget);
    void         SetOperatorOptions();
private:
    int operatorType;
};

#endif
