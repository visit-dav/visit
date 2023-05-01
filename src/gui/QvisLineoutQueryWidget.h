// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_LINEOUT_QUERY_WIDGET_H
#define QVIS_LINEOUT_QUERY_WIDGET_H
#include <gui_exports.h>
#include <QWidget>

// Forward declarations.
class QCheckBox;
class QLabel;
class QLineEdit;
class MapNode;

// ****************************************************************************
// Class: QvisLineoutQueryWidget
//
// Purpose:
//   This widget provides options for performing a Pick query.
//
// Notes:      
//
// Programmer: Kathleen Biagas
// Creation:   June 9, 2011 
//
// Modifications:
//    Kathleen Biagas, Fri Nov 15 17:14:54 PST 2013
//    Add sampling options.
//
// ****************************************************************************

class GUI_API QvisLineoutQueryWidget : public QWidget
{
    Q_OBJECT
public:
    QvisLineoutQueryWidget(QWidget *parent = 0, Qt::WindowFlags f = Qt::Widget);
    virtual ~QvisLineoutQueryWidget();

    bool                GetQueryParameters(MapNode &);

private:

    bool                 GetPoint(int, double *pt);
    QLineEdit           *startPoint;
    QLineEdit           *endPoint;
    QCheckBox           *samplingOn;
    QLineEdit           *numSamples;
    QLabel              *numSamplesLabel;

};

#endif
