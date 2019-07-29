// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_CONNECTION_PROGRESS_DIALOG_H
#define VIEWER_CONNECTION_PROGRESS_DIALOG_H
#include <QWidget>
#include <ViewerConnectionProgress.h>

// Forward declarations.
class QLabel;
class QPushButton;
class QRadioButton;
class QTimer;

// ****************************************************************************
// Class: ViewerConnectionProgressDialog
//
// Purpose:
//   This class contains a progress dialog that is displayed when we launch
//   processes and it takes a long time.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 26 13:20:25 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Oct 24 11:46:47 PDT 2002
//   I increased the default timeout to 6 seconds.
//
//   Brad Whitlock, Wed May 7 10:14:04 PDT 2003
//   I added methods to set/get the component name. I also added methods
//   to set/get a flag that determines if the window should be hidden.
//
//   Brad Whitlock, Tue Nov 29 16:01:20 PST 2011
//   I added various methods for getting/setting values and I simplified the
//   constructor.
//
//   Brad Whitlock, Sat Sep  6 00:38:12 PDT 2014
//   Inherit from ViewerConnectionProgress.
//
// ****************************************************************************

class ViewerConnectionProgressDialog : public QWidget, 
    public ViewerConnectionProgress
{
    Q_OBJECT
public:
    ViewerConnectionProgressDialog();
    virtual ~ViewerConnectionProgressDialog();

    virtual void Hide();
    virtual void Show();

    virtual void SetComponentName(const std::string &cn);
    virtual void SetHostName(const std::string &host);
    virtual void SetParallel(bool);
    
public slots:
    virtual void show();
    virtual void hide();
private slots:
    void updateAnimation();
    void timedShow();
    void cancel();
private:
    void updateMessage();

    QTimer       *timer;
    QPushButton  *cancelButton;
    QRadioButton *rb[6];
    QLabel       *msgLabel;
    QLabel       *rightComputer;

    int           iconFrame;
    bool          cancelledShow;
};

#endif
