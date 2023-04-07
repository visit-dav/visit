// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_HOHLRAUMFLUX_QUERY_WIDGET_H
#define QVIS_HOHLRAUMFLUX_QUERY_WIDGET_H
#include <gui_exports.h>
#include <QWidget>

// Forward declarations.
class QCheckBox;
class QLineEdit;
class MapNode;

// ****************************************************************************
// Class: QvisHohlraumFluxQueryWidget
//
// Purpose:
//   This widget provides options for performing a HohlraumFlux query.
//
// Notes:      
//
// Programmer: Kathleen Biagas
// Creation:   June 20, 2011 
//
// Modifications:
//
// ****************************************************************************

class GUI_API QvisHohlraumFluxQueryWidget : public QWidget
{
    Q_OBJECT
public:
    QvisHohlraumFluxQueryWidget(QWidget *parent = 0, Qt::WindowFlags f = Qt::Widget);
    virtual ~QvisHohlraumFluxQueryWidget();

    bool GetQueryParameters(MapNode &params);


private:
    bool             GetDoubleValues(int whichWidget, int n, double *pt);
    bool             GetIntValues(int whichWidget, int *pt);

    QCheckBox       *divideFlag;
    QLineEdit       *numLines;
    QLineEdit       *rayCenter;
    QLineEdit       *radius;
    QLineEdit       *theta;
    QLineEdit       *phi;
};

#endif
