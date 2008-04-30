/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
class QListBox;
class QSlider;
class QvisColorButton;
class QvisColorTableButton;
class QvisLineStyleWidget;
class QvisLineWidthWidget;
class QvisOpacitySlider;
class QvisPointControl;

// ****************************************************************************
// Class: QvisFilledBoundaryPlotWindow
//
// Purpose: 
//   This class is an observer window that watches boundary plot attributes
//   and always represents their current state.
//
// Notes:  Copied from QvisMaterialPlotWindow with minor changes.
//
// Programmer: Jeremy Meredith
// Creation:   May  9, 2003
//
// Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//    Jeremy Meredith, Fri Jun 13 16:56:43 PDT 2003
//    Added clean zones only.
//
//    Jeremy Meredith, Tue Apr 13 16:42:55 PDT 2004
//    Added mixed color.
//
//    Kathleen Bonnell, Fri Nov 12 10:42:08 PST 2004 
//    Added pointControl and associated slots, also added GetCurrentValues.
//
//    Brad Whitlock, Wed Jul 20 14:23:58 PST 2005
//    Added a new slot to handle a new signal from QvisPointControl.
//
// ****************************************************************************

class QvisFilledBoundaryPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisFilledBoundaryPlotWindow(const int type,
                                 FilledBoundaryAttributes *boundaryAtts_,
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
    void GetCurrentValues(int which_widget);
private slots:
    void lineStyleChanged(int newStyle);
    void lineWidthChanged(int newWidth);
    void legendToggled(bool val);
    void wireframeToggled(bool val);
    void drawInternalToggled(bool val);
    void cleanZonesOnlyToggled(bool val);
    void singleColorChanged(const QColor &color);
    void singleColorOpacityChanged(int opacity);
    void multipleColorChanged(const QColor &color);
    void multipleColorOpacityChanged(int opacity);
    void colorModeChanged(int index);
    void colorTableClicked(bool useDefault, const QString &ctName);
    void boundarySelectionChanged();
    void overallOpacityChanged(int opacity);
    void smoothingLevelChanged(int index);
    void mixedColorChanged(const QColor &color);

    void pointSizeChanged(double d);
    void pointSizePixelsChanged(int size);
    void pointTypeChanged(int index);
    void pointSizeVarToggled(bool on);
    void pointSizeVarChanged(const QString &);

private:
    int                       plotType;
    FilledBoundaryAttributes *boundaryAtts;
    QLabel                   *lineStyleLabel;
    QvisLineStyleWidget      *lineStyle;
    QLabel                   *lineWidthLabel;
    QvisLineWidthWidget      *lineWidth;
    QButtonGroup             *colorModeButtons;
    QCheckBox                *legendCheckBox;
    QCheckBox                *cleanZonesOnlyCheckBox;
    QGroupBox                *boundaryColorGroup;
    QvisColorButton          *singleColor;
    QvisOpacitySlider        *singleColorOpacity;
    QLabel                   *multipleColorLabel;
    QListBox                 *multipleColorList;
    QvisColorButton          *multipleColor;
    QvisOpacitySlider        *multipleColorOpacity;
    QvisColorTableButton     *colorTableButton;
    QvisOpacitySlider        *overallOpacity;
    QLabel                   *mixedColorLabel;
    QvisColorButton          *mixedColor;
    QCheckBox                *wireframeCheckBox;
    QCheckBox                *drawInternalCheckBox;
    QButtonGroup             *smoothingLevelButtons;
    QvisPointControl         *pointControl;
};

#endif
