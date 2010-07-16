/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#ifndef QVISCURVEPLOTWINDOW_H
#define QVISCURVEPLOTWINDOW_H

#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

class CurveAttributes;
class QLabel;
class QNarrowLineEdit;
class QCheckBox;
class QComboBox;
class QButtonGroup;
class QSpinBox;
class QvisColorButton;
class QvisLineStyleWidget;
class QvisLineWidthWidget;

// ****************************************************************************
// Class: QvisCurvePlotWindow
//
// Purpose: 
//   Defines QvisCurvePlotWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Tue Jul 23 13:34:33 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Tue Dec 23 13:27:22 PST 2003
//   Added showPoints, pointSize and pointSizeLabel widgets. 
//   Added showPointChanged and processPointSizeText slots.
//
//   Kathleen Bonnell, Thu Oct 27 15:35:11 PDT 2005 
//   Added showLegend.
//   
//   Kathleen Bonnell, Mon Oct 31 17:05:35 PST 2005
//   Added cycleColors, colorLabel.
//
//   Brad Whitlock, Mon Nov 20 13:25:30 PST 2006
//   Added controls to control symbol-based rendering.
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
//   Hank Childs, Thu Jul 15 18:20:26 PDT 2010
//   Add cue for the current location.
//
// ****************************************************************************

class QvisCurvePlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisCurvePlotWindow(const int type,
                         CurveAttributes *subj,
                         const QString &caption = QString::null,
                         const QString &shortName = QString::null,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisCurvePlotWindow();
    virtual void CreateWindowContents();
  public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
  protected:
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
  private slots:
    void lineStyleChanged(int style);
    void lineWidthChanged(int style);
    void labelsToggled(bool val);
    void legendToggled(bool val);
    void showPointsChanged(bool val);
    void processPointSizeText();

    void curveColorClicked(int val);
    void curveColorChanged(const QColor &color);

    void renderModeChanged(int);
    void symbolTypeChanged(int);
    void symbolDensityChanged(int);

    void doBallTimeCueChanged(bool val);
    void ballTimeCueColorChanged(const QColor &color);
    void timeCueBallSizeProcessText();
    void doLineTimeCueChanged(bool val);
    void lineTimeCueColorChanged(const QColor &color);
    void lineTimeCueWidthChanged(int);
    void doCropTimeCueChanged(bool val);
    void timeForTimeCueProcessText();

  private:
    int plotType;
    QvisLineStyleWidget *lineStyle;
    QLabel              *lineStyleLabel;
    QvisLineWidthWidget *lineWidth;
    QLabel              *lineWidthLabel;
    QCheckBox           *cycleColors;

    QButtonGroup        *curveColorButtons;
    QvisColorButton     *curveColor;

    QCheckBox           *labelsToggle;
    QCheckBox           *legendToggle;
    QCheckBox           *showPoints;
    QNarrowLineEdit     *pointSize;
    QLabel              *pointSizeLabel;
    QButtonGroup        *renderMode;
    QComboBox           *symbolType;
    QLabel              *symbolTypeLabel;
    QSpinBox            *symbolDensity;
    QLabel              *symbolDensityLabel;

    QCheckBox           *doBallTimeCue;
    QvisColorButton     *ballTimeCueColor;
    QLineEdit           *timeCueBallSize;
    QCheckBox           *doLineTimeCue;
    QvisColorButton     *lineTimeCueColor;
    QvisLineWidthWidget *lineTimeCueWidth;
    QCheckBox           *doCropTimeCue;
    QLineEdit           *timeForTimeCue;
    QLabel              *timeCueBallSizeLabel;
    QLabel              *lineTimeCueWidthLabel;
    QLabel              *timeForTimeCueLabel;

    CurveAttributes *atts;
};



#endif
