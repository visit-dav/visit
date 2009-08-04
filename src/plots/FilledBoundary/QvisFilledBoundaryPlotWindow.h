/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
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

#ifndef QVIS_FILLED_BOUNDARY_PLOT_WINDOW_H
#define QVIS_FILLED_BOUNDARY_PLOT_WINDOW_H
#include <QvisPostableWindowObserver.h>

class FilledBoundaryAttributes;
class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QSlider;
class QvisColorButton;
class QvisColorSwatchListWidget;
class QvisColorTableButton;
class QvisLineStyleWidget;
class QvisLineWidthWidget;
class QvisOpacitySlider;
class QvisPointControl;

// ****************************************************************************
// Class: QvisFilledBoundaryPlotWindow
//
// Purpose: 
//   This class is an observer window that watches FilledBoundary plot attributes
//   and always represents their current state.
//
// Notes:  Copied from QvisBoundaryPlotWindow with minor changes.
//
// Programmer: Jeremy Meredith
// Creation:   June 12, 2003
//
// Modifications:
//
// ****************************************************************************

class QvisFilledBoundaryPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisFilledBoundaryPlotWindow(const int type, FilledBoundaryAttributes *FilledBoundaryAtts_,
                           const QString &caption = QString::null,
                           const QString &shortName = QString::null,
                           QvisNotepadArea *notepad = 0);
    virtual ~QvisFilledBoundaryPlotWindow();
    virtual void CreateWindowContents();
public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
protected:
    void UpdateWindow(bool doAll);
    void Apply(bool ignore = false);
    void SetMultipleColorWidgets(int index);
    void UpdateMultipleArea();
    void UpdateItem(int i);
    bool CompareItem(int i, const QString &name, const QColor &c) const;
    void GetCurrentValues(int which_widget);
private slots:
    void lineStyleChanged(int newStyle);
    void lineWidthChanged(int newWidth);
    void legendToggled(bool val);
    void wireframeToggled(bool val);
    void singleColorChanged(const QColor &color);
    void singleColorOpacityChanged(int opacity);
    void multipleColorChanged(const QColor &color);
    void multipleColorOpacityChanged(int opacity);
    void colorModeChanged(int index);
    void colorTableClicked(bool useDefault, const QString &ctName);
    void boundarySelectionChanged();
    void overallOpacityChanged(int opacity);
    void smoothingLevelChanged(int index);

    void pointSizeChanged(double d);
    void pointSizePixelsChanged(int size);
    void pointTypeChanged(int index);
    void pointSizeVarToggled(bool on);
    void pointSizeVarChanged(const QString &);

    void mixedColorChanged(const QColor &color);
    void cleanZonesOnlyToggled(bool val);
    void drawInternalToggled(bool);
private:
    int                       plotType;
    FilledBoundaryAttributes *boundaryAtts;

    QLabel                 *lineStyleLabel;
    QvisLineStyleWidget    *lineStyle;
    QLabel                 *lineWidthLabel;
    QvisLineWidthWidget    *lineWidth;
    QButtonGroup           *colorModeButtons;
    QCheckBox              *legendCheckBox;
    QGroupBox              *boundaryColorGroup;
    QvisColorButton        *singleColor;
    QvisOpacitySlider      *singleColorOpacity;
    QLabel                 *multipleColorLabel;
    QvisColorSwatchListWidget *multipleColorList;
    QvisColorButton        *multipleColor;
    QvisOpacitySlider      *multipleColorOpacity;
    QvisColorTableButton   *colorTableButton;
    QvisOpacitySlider      *overallOpacity;
    QCheckBox              *wireframeCheckBox;
    QButtonGroup           *smoothingLevelButtons;
    QvisPointControl       *pointControl;
    QCheckBox              *cleanZonesOnlyCheckBox;
    QLabel                 *mixedColorLabel;
    QvisColorButton        *mixedColor;
    QCheckBox              *drawInternalCheckBox;
};

#endif
