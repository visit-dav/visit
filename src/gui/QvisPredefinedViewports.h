// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_PREDEFINED_VIEWPORTS_H
#define QVIS_PREDEFINED_VIEWPORTS_H
#include <QPushButton>
#include <QMenu>
#include <gui_exports.h>

// ****************************************************************************
// Class: QvisPredefinedViewports
//
// Purpose:
//   This widget is a button with predefined viewport pictures.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Oct 2006
//
// Modifications:
//   Brad Whitlock, Tue Oct  7 10:27:30 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class GUI_API QvisPredefinedViewports : public QPushButton
{
    Q_OBJECT
public:
    QvisPredefinedViewports(QWidget *parent);
    virtual ~QvisPredefinedViewports();
signals:
    void activated(int);
private slots:
    void emitActivated(QAction*);
private:
    QMenu *menu;
};

#endif
