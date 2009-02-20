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

#ifndef QVIS_PSEUDOCOLOR_WINDOW_H
#define QVIS_PSEUDOCOLOR_WINDOW_H
#include <QvisPostableWindowObserver.h>

class QComboBox;
class QLineEdit;
class QCheckBox;
class QButtonGroup;
class QLabel;
class QvisOpacitySlider;
class QvisColorTableButton;
class QvisPointControl;
class QvisLineStyleWidget;
class QvisLineWidthWidget;

class Subject;
class PseudocolorAttributes;

// ****************************************************************************
// Class: QvisPseudocolorPlotWindow
//
// Purpose:
//   This class is a postable window that watches pseudocolot plot
//   attributes and always represents their current state.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 28 17:16:38 PST 2000
//
// Modifications:
//   Kathleen Bonnell, Thu Dec 14 17:04:25 PST 2000
//   Added support for setting opacity.
//
//   Eric Brugger, Wed Mar 14 06:59:25 PST 2001
//   I added a plot type to the constructor for use with the viewer
//   proxy.
//
//   Brad Whitlock, Sat Jun 16 15:17:12 PST 2001
//   I added a color table button.
//
//   Kathleen Bonnell, Thu Oct  4 16:28:16 PDT 2001 
//   I added a limits combo box.
//
//   Brad Whitlock, Fri Feb 15 10:27:55 PDT 2002
//   Removed a method.
//
//   Jeremy Meredith, Tue Dec 10 10:22:40 PST 2002
//   Added smoothing level.
//
//   Jeremy Meredith, Fri Dec 20 11:36:03 PST 2002
//   Added scaling of point variables by a scalar field.
//
//   Kathleen Bonnell, Fri Nov 12 11:25:23 PST 2004 
//   Replace individual point-size related widgets and associated slots
//   with QvisPointControl 
//
//   Brad Whitlock, Wed Jul 20 14:23:58 PST 2005
//   Added a new slot to handle a new signal from QvisPointControl.
//
//   Jeremy Meredith, Wed Nov 26 11:28:24 EST 2008
//   Added line style/width controls.
//
//   Jeremy Meredith, Fri Feb 20 15:14:29 EST 2009
//   Added support for using per-color alpha values from a color table
//   (instead of just a single global opacity for the whole plot).
//   There's a new toggle for this, and it overrides the whole-plot opacity.
//
// ****************************************************************************

class QvisPseudocolorPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisPseudocolorPlotWindow(const int type, PseudocolorAttributes *_pcAtts,
                              const QString &caption = QString::null,
                              const QString &shortName = QString::null,
                              QvisNotepadArea *notepad = 0);
    virtual ~QvisPseudocolorPlotWindow();
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
    void centeringClicked(int button);
    void legendToggled(bool on);
    void lightingToggled(bool on);
    void minToggled(bool on);
    void maxToggled(bool on);
    void useColorTableOpacityToggled(bool on);
    void processMaxLimitText();
    void processMinLimitText();
    void processSkewText();
    void scaleClicked(int scale);
    void changedOpacity(int opacity, const void *);
    void colorTableClicked(bool useDefault, const QString &ctName);
    void limitsSelectChanged(int);
    void smoothingLevelChanged(int index);

    void pointSizeChanged(double d);
    void pointSizePixelsChanged(int size);
    void pointTypeChanged(int index);
    void pointSizeVarToggled(bool on);
    void pointSizeVarChanged(const QString &);

    void lineStyleChanged(int newStyle);
    void lineWidthChanged(int newWidth);

private:
    int                   plotType;
    PseudocolorAttributes *pcAtts;
    QButtonGroup          *centeringButtons;
    QCheckBox             *useColorTableOpacity;
    QCheckBox             *legendToggle;
    QCheckBox             *lightingToggle;
    QCheckBox             *minToggle;
    QCheckBox             *maxToggle;
    QComboBox             *limitsSelect;
    QLineEdit             *maxLineEdit;
    QLineEdit             *minLineEdit;
    QButtonGroup          *scalingButtons;
    QLabel                *skewLabel;
    QLineEdit             *skewLineEdit;
    QLabel                *opacityLabel;
    QvisOpacitySlider     *opacitySlider;
    QvisColorTableButton  *colorTableButton;
    QButtonGroup          *smoothingLevelButtons;
    QvisPointControl      *pointControl;
    QLabel                *lineStyleLabel;
    QvisLineStyleWidget   *lineStyle;
    QLabel                *lineWidthLabel;
    QvisLineWidthWidget   *lineWidth;
};

#endif
