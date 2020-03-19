// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_TIME_SLIDER_CONTROL_WIDGET_H
#define QVIS_TIME_SLIDER_CONTROL_WIDGET_H
#include <gui_exports.h>
#include <QWidget>
#include <QGroupBox>
#include <SimpleObserver.h>
#include <GUIBase.h>
#include <QualifiedFilename.h>
#include <TimeFormat.h>
#include <map>

// Forward declarations.
class QComboBox;
class QContextMenuEvent;
class QLabel;
class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;
class QLineEdit;
class QPixmap;
class QMenu;
class QvisAnimationSlider;
class QvisFilePanelItem;
class QvisVCRControl;

class avtDatabaseMetaData;
class FileServerList;
class PlotList;
class WindowInformation;
class ViewerProxy;

// ****************************************************************************
// Class: QvisTimeSliderControlWidget
//
// Purpose:
//      Holds widgets that provide time slider selection and control
//      the active time slider.
//
// Note:
//   These widgets were moved out of QvisFilePanel into this class.
//
// Programmer: Cyrus Harrison
// Creation:   Fri Mar 12 10:55:05 PST 2010
//
//
// Modifications:
//  Kathleen Biagas, Mon Sep 30 09:25:45 PDT 2019
//  Observe PlotList so VCR controls play and reverse play buttons enabled
//  state can be dependent upon the active plots being drawn.
//
// ****************************************************************************

class GUI_API QvisTimeSliderControlWidget : public QGroupBox,
     public SimpleObserver, public GUIBase
{
    Q_OBJECT

public:
    QvisTimeSliderControlWidget(QWidget *parent = 0);
    virtual ~QvisTimeSliderControlWidget();
    virtual void Update(Subject *);
    virtual void SubjectRemoved(Subject *);

    void ConnectFileServer(FileServerList *);
    void ConnectWindowInformation(WindowInformation *);
    void ConnectPlotList(PlotList *);

    void SetTimeStateFormat(const TimeFormat &fmt);
    const TimeFormat &GetTimeStateFormat() const;

signals:
    void reopenOnNextFrame();
private:
    void UpdateAnimationControls(bool doAll);
    void UpdateTimeFieldText(int timeState);
    void SetTimeFieldText(const QString &text);
    void UpdateAnimationControlsEnabledState();
    QString FormattedCycleString(const int cycle) const;
    QString FormattedTimeString(const double d, bool accurate) const;
    void SetTimeSliderState(int);

private slots:
    void changeActiveTimeSlider(int);
    void backwardStep();
    void reversePlay();
    void stop();
    void play();
    void forwardStep();
    void sliderStart();
    void sliderMove(int val);
    void sliderEnd();
    void sliderChange(int val);
    void processTimeText();

private:
    QComboBox                *activeTimeSlider;
    QLabel                   *activeTimeSliderLabel;
    QLineEdit                *timeField;
    QvisAnimationSlider      *animationPosition;
    QvisVCRControl           *vcrControls;

    WindowInformation        *windowInfo;
    int                       sliderVal;
    TimeFormat                timeStateFormat;
    PlotList                 *plotList;
};

#endif
