// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_TURNDOWN_BUTTON_H
#define QVIS_TURNDOWN_BUTTON_H
#include <gui_exports.h>
#include <QPushButton>

// ****************************************************************************
// Class: QvisTurnDownButton
//
// Purpose:
//   This class creates a turn down button.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 13:59:57 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Jun  3 16:12:47 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class GUI_API QvisTurnDownButton : public QPushButton
{
    Q_OBJECT
public:
    QvisTurnDownButton(QWidget *parent = 0);
    virtual ~QvisTurnDownButton();
    virtual QSize sizeHint() const { return QSize(20,15); }
protected:
    virtual void paintEvent(QPaintEvent *);
};

#endif
