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

#ifndef QVIS_MESH_PLOT_WINDOW_H
#define QVIS_MESH_PLOT_WINDOW_H
#include <QvisPostableWindowObserver.h>

class QLineEdit;
class QCheckBox;
class QButtonGroup;
class QLabel;
class QvisLineStyleWidget;
class QvisLineWidthWidget;
class QvisColorButton;
class QvisPointControl;
class Subject;
class MeshAttributes;
class QvisOpacitySlider;

// ****************************************************************************
// Class: QvisMeshPlotWindow
//
// Purpose:
//   This class is a postable window that watches mesh plot attributes and
//   always represents their current state.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 9 16:31:31 PST 2001
//
// Modifications:
//   Kathleen Bonnell, Wed Sep  5 16:20:52 PDT 2001
//   Added color choice for opaque mode.
//   
//   Kathleen Bonnell, Wed Sep 26 10:06:05 PDT 2001 
//   Added checkboxes for foreground and background. 
//   
//   Jeremy Meredith, Tue Dec 10 10:22:40 PST 2002
//   Added smoothing level.
//
//   Jeremy Meredith, Fri Dec 20 11:36:03 PST 2002
//   Added scaling of point variables by a scalar field.
//
//   Hank Childs, Thu Aug 21 23:14:39 PDT 2003
//   Added support for different types of point glyphs.
//
//   Kathleen Bonnell, Thu Sep  4 11:15:30 PDT 2003 
//   Changed opaqueToggle check box to opaqueMode button group.
//   Renamed signal opaqueToggled to opaqueModeChanged.
//
//   Kathleen Bonnell, Thu Feb  5 11:51:39 PST 2004 
//   Added showInternalToggle, signal showInternalToggled.
//
//   Kathleen Bonnell, Fri Nov 12 10:51:59 PST 2004 
//   Replaced point-related control widgets (and associated slots) 
//   with QvisPointControl. 
//
//   Brad Whitlock, Wed Jul 20 14:23:58 PST 2005
//   Added a new slot to handle a new signal from QvisPointControl.
//
//   Cyrus Harrison, Fri Jul 18 14:44:51 PDT 2008
//   Initial Qt4 Port. 
//
//   Jeremy Meredith, Fri Feb 20 17:28:17 EST 2009
//   Added per-plot alpha (opacity) support.
//
// ****************************************************************************

class QvisMeshPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisMeshPlotWindow(const int type, MeshAttributes *_meshAtts,
                       const QString &caption = QString::null,
                       const QString &shortName = QString::null,
                       QvisNotepadArea *notepad = 0);
    virtual ~QvisMeshPlotWindow();
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
    void lineStyleChanged(int newStyle);
    void lineWidthChanged(int newWidth);
    void legendToggled(bool val);
    void showInternalToggled(bool val);
    void meshColorChanged(const QColor &color);
    void outlineOnlyToggled(bool on);
    void opaqueModeChanged(int val);
    void processErrorToleranceText();
    void opaqueColorChanged(const QColor &color);
    void backgroundToggled(bool on);
    void foregroundToggled(bool on);
    void smoothingLevelChanged(int index);

    void pointSizeChanged(double d);
    void pointSizePixelsChanged(int);
    void pointTypeChanged(int index);
    void pointSizeVarToggled(bool);
    void pointSizeVarChanged(const QString &);

    void changedOpacity(int opacity, const void *);

private:
    int                     plotType;
    MeshAttributes         *meshAtts;
    QLabel                 *lineStyleLabel;
    QvisLineStyleWidget    *lineStyle;
    QLabel                 *lineWidthLabel;
    QvisLineWidthWidget    *lineWidth;
    QLabel                 *meshColorLabel;
    QvisColorButton        *meshColor;
    QCheckBox              *outlineOnlyToggle;
    QLabel                 *errorToleranceLabel;
    QLineEdit              *errorToleranceLineEdit;
    QButtonGroup           *opaqueModeGroup;
    QCheckBox              *legendToggle;
    QCheckBox              *showInternalToggle;
    QLabel                 *opaqueColorLabel;
    QvisColorButton        *opaqueColor;
    QCheckBox              *backgroundToggle;
    QCheckBox              *foregroundToggle;
    QButtonGroup           *smoothingLevelGroup;
    QvisPointControl       *pointControl;
    QLabel                *opacityLabel;
    QvisOpacitySlider     *opacitySlider;
};

#endif
