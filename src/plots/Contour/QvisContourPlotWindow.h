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

#ifndef QVIS_CONTOUR_PLOT_WINDOW_H
#define QVIS_CONTOUR_PLOT_WINDOW_H
#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

class ContourAttributes;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QvisColorButton;
class QvisColorManagerWidget;
class QvisColorTableButton;
class QvisLineStyleWidget;
class QvisLineWidthWidget;
class QvisOpacitySlider;

// ****************************************************************************
// Class: QvisContourPlotWindow
//
// Purpose: 
//   This class is an observer window that watches material plot attributes
//   and always represents their current state.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 16 13:48:18 PST 2001
//
// Modifications:
//   Eric Brugger, Wed Mar 14 11:58:10 PST 2001
//   I added a plot type to the constructor for use with the viewer proxy.
//   
//   Jeremy Meredith, Wed Mar 13 10:20:32 PST 2002
//   Added a toggle for wireframe mode.
//
//   Brad Whitlock, Thu Aug 22 10:25:17 PDT 2002
//   I added a slot function for single color opacity.
//
//   Brad Whitlock, Tue Dec 3 08:52:43 PDT 2002
//   I added a color table coloring option.
//
//   Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//   Moved StringToDoubleList to QvisWindowBase 
//
// ****************************************************************************

class QvisContourPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisContourPlotWindow(const int type, ContourAttributes *atts,
                          const QString &caption = QString::null,
                          const QString &shortName = QString::null,
                          QvisNotepadArea *notepad = 0);
    virtual ~QvisContourPlotWindow();
    virtual void CreateWindowContents();
public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
protected:
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);

    bool UpdateMultipleAreaColors();
    bool UpdateMultipleAreaNames();
    void UpdateSelectByText();

    QString LevelString(int i);
    void ProcessSelectByText();
private slots:
    void lineStyleChanged(int newStyle);
    void lineWidthChanged(int newWidth);
    void legendToggled(bool val);
    void wireframeToggled(bool val);
    void singleColorChanged(const QColor &color);
    void singleColorOpacityChanged(int opacity);
    void multipleColorChanged(const QColor &color, int index);
    void colorTableClicked(bool useDefault, const QString &ctName);
    void opacityChanged(int opacity, int index);
    void colorModeChanged(int index);
    void scaleClicked(int scale);
    void selectByChanged(int);
    void processSelectByText();
    void minToggled(bool);
    void processMinLimitText();
    void maxToggled(bool);
    void processMaxLimitText();
private:
    int                     plotType;
    ContourAttributes      *contourAtts;
    QLabel                 *lineStyleLabel;
    QvisLineStyleWidget    *lineStyle;
    QLabel                 *lineWidthLabel;
    QvisLineWidthWidget    *lineWidth;
    QButtonGroup           *colorModeButtons;
    QCheckBox              *legendCheckBox;
    QGroupBox              *contourColorGroup;
    QvisColorButton        *singleColor;
    QvisOpacitySlider      *singleColorOpacity;
    QvisColorManagerWidget *multipleColors;
    QvisColorTableButton   *colorTableButton;
    QComboBox              *selectByComboBox;
    QLineEdit              *selectByLineEdit;
    QButtonGroup           *scalingButtons;
    QCheckBox              *minToggle;
    QLineEdit              *minLineEdit;
    QCheckBox              *maxToggle;
    QLineEdit              *maxLineEdit;
    QCheckBox              *wireframeCheckBox;
};

#endif
