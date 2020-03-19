// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SEQUENCE_BUTTON_H
#define QVIS_SEQUENCE_BUTTON_H
#include <QPushButton>
#include <QMenu>
#include <gui_exports.h>

// ****************************************************************************
// Class: QvisSequenceButton
//
// Purpose:
//   This is a special widget that creates a set of menus for the available
//   movie sequence types provided by the MovieSequenceFactory class.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Oct 2006
//
// Modifications:
//   Brad Whitlock, Tue Oct  7 09:18:15 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class GUI_API QvisSequenceButton : public QPushButton
{
    Q_OBJECT
public:
    QvisSequenceButton(QWidget *parent);
    virtual ~QvisSequenceButton();
signals:
    void activated(int);
private slots:
    void emitActivated(QAction*);
private:
    QMenu *menu;
};

#endif
