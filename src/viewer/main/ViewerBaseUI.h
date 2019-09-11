// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_BASE_UI_H
#define VIEWER_BASE_UI_H
#include <viewer_exports.h>
#include <QObject>
#include <ViewerBase.h>

// ****************************************************************************
// Class: ViewerBaseUI
//
// Purpose:
//   Base class for Qt-enabled viewer objects.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 10 16:23:23 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class ViewerBaseUI : public QObject, public ViewerBase
{
   Q_OBJECT
public:
    ViewerBaseUI(QObject *parent = 0);
    virtual ~ViewerBaseUI();
};

#endif
