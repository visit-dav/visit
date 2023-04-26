// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_PICK_QUERY_WIDGET_H
#define QVIS_PICK_QUERY_WIDGET_H
#include <gui_exports.h>
#include <QWidget>

// Forward declarations.
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class MapNode;

// ****************************************************************************
// Class: QvisPickQueryWidget
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
//   Matt Larsen, December 12, 2016 
//   added GetElementRange to detect if a range of picks has been entered in 
//   the pick window.
//
//   Matt Larsen, June 17, 2017 
//   added GetElementLabel to support label picks
//
// ****************************************************************************

class GUI_API QvisPickQueryWidget : public QWidget
{
    Q_OBJECT
public:
    QvisPickQueryWidget(QWidget *parent = 0, Qt::WindowFlags f = Qt::Widget);
    virtual ~QvisPickQueryWidget();

    bool                 GetQueryParameters(MapNode &params);

public slots:
    void                 timeOptionsToggled(bool);

private slots:
    void                 pickTypeChanged(int);

private:
    void                 UpdateControls();
    int                  GetPickType(void);
    bool                 GetPoint(double *pt);
    bool                 GetDomain(int *);
    bool                 GetElement(int *);
    bool                 GetElementRange(std::string &);
    bool                 GetElementLabel(std::string &);
    int                  GetPlotType(void);
    int                  GetElementType(void);
    bool                 GetTimePreservesCoord(void);

    QComboBox           *pickType;
    QLabel              *coordinateLabel;
    QLineEdit           *coordinate;
    QLabel              *domainLabel;
    QLineEdit           *domain;
    QLineEdit           *element;
    QButtonGroup        *elementType;
    QButtonGroup        *plotType;
    QButtonGroup        *timePreserveType;
    QGroupBox           *tcbox;
    QGroupBox           *mcbox;

};

#endif
