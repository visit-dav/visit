// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef WIDGET_DATA_NODE_H
#define WIDGET_DATA_NODE_H
#include <QWidget>
#include <DataNode.h>

// ****************************************************************************
// Purpose: 
//   These functions allow DataNode trees to be populated based on the
//   contents of Qt user interface widgets (and vice-versa).
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 16 13:32:38 PST 2006
//
// Modifications:
//   
// ****************************************************************************

//
// Functions that can set/get DataNode values into a QWidget.
//
void InitializeWidgetFromDataNode(QWidget *ui, DataNode *node);
void InitializeDataNodeFromWidget(QWidget *ui, DataNode *node);

//
// Helper functions.
//
QString DataNodeToQString(const DataNode *node);

bool DataNodeToQColor(DataNode *node, QColor &color);
void QColorToDataNode(DataNode *node, const char *key, const QColor &color);

#endif
