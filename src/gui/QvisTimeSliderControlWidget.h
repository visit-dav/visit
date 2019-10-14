/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
