// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_DELAYED_WINDOW_H
#define QVIS_DELAYED_WINDOW_H
#include <gui_exports.h>
#include <QvisWindowBase.h>

class QWidget;
class QVBoxLayout;

// ****************************************************************************
// Class: QvisDelayedWindow
//
// Purpose:
//   This class defines the interface for a top level window that
//   is not created until it is needed.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 11:27:10 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Aug 30 13:41:37 PST 2000
//   Made it inherit from QvisWindowBase.
//
//   Brad Whitlock, Fri Feb 15 11:31:07 PDT 2002
//   Changed protection on some methods.
//
//   Brad Whitlock, Mon Sep 30 07:50:55 PDT 2002
//   I added window flags.
//
//   Brad Whitlock, Wed Apr  9 10:31:28 PDT 2008
//   Changed ctor args.
//
//   Brad Whitlock, Thu Jun 19 14:13:42 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class GUI_API QvisDelayedWindow : public QvisWindowBase
{
    Q_OBJECT
public:
    QvisDelayedWindow(const QString &captionString, Qt::WindowFlags f = Qt::Widget);
    virtual ~QvisDelayedWindow();
    QWidget *GetCentralWidget();
    virtual void CreateEntireWindow();
public slots:
    virtual void raise();
    virtual void show();
    virtual void hide();
protected:
    virtual void CreateWindowContents() = 0;
    virtual void UpdateWindow(bool doAll);
protected:
    bool        isCreated;
    QWidget     *central;
    QVBoxLayout *topLayout;
};

#endif
