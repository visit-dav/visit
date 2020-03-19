// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_UI_LOADER_H
#define QVIS_UI_LOADER_H

#include <QUiLoader>
#include <gui_exports.h>

// ****************************************************************************
// Class: QvisUiLoader
//
// Purpose:
//   This class loads custom UI's from a .ui file.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul  7 10:41:56 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisUiLoader : public QUiLoader
{
public:
    QvisUiLoader();
    virtual ~QvisUiLoader();

    virtual QWidget *createWidget(const QString &className, QWidget *parent, const QString &name = QString());
};

#endif
