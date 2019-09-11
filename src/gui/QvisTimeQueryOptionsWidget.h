// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_TIMEQUERY_OPTIONSWIDGET_H
#define QVIS_TIMEQUERY_OPTIONSWIDGET_H

#include <gui_exports.h>

#include <QGroupBox>

#include <GUIBase.h>
#include <MapNode.h>

class QLabel;
class QSpinBox;
class PlotList;

// ****************************************************************************
// Class: QvisTimeQueryOptionsWidget
//
// Purpose: 
//   Defines QvisTimeQueryOptionsWidget class.
//
// Programmer: Kathleen Biagas 
// Creation:   August 1, 2011
//
// Modifications:
//    Kathleen Biagas, Wed Apr 11 19:10:38 PDT 2012
//    Removed GetDatabaseNStates, added UpdateState.
//
// ****************************************************************************

class GUI_API QvisTimeQueryOptionsWidget : public QGroupBox, public GUIBase
{
    Q_OBJECT
  public:
    QvisTimeQueryOptionsWidget(QWidget *parent = 0);
    QvisTimeQueryOptionsWidget(const QString &title, QWidget *parent = 0);
    virtual ~QvisTimeQueryOptionsWidget();

    bool GetTimeQueryOptions(MapNode &options);

    void UpdateState(PlotList *plotList);

    virtual void setCheckable(bool val);
 
  public slots:
    virtual void show();
    virtual void setEnabled(bool);

  private:
    void         CreateWindowContents();
    void         SetMax(const int val);

    QLabel      *msgLabel;
    QSpinBox    *startTime;
    QSpinBox    *endTime;
    QSpinBox    *stride;
    QLabel      *startLabel;
    QLabel      *endLabel;
    QLabel      *strideLabel;
    int maxTime;
};

#endif
