/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef QVIS_LIGHTING_WINDOW_H
#define QVIS_LIGHTING_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>

// Forward declarations.
class LightList;
class QComboBox;
class QButtonGroup;
class QCheckBox;
class QLabel;
class QLineEdit;
class QPixmap;
class QvisColorButton;
class QvisLightingWidget;
class QvisOpacitySlider;
class QGroupBox;
class QSpinBox;

// ****************************************************************************
// Class: QvisLightingWindow
//
// Purpose:
//   This window displays and manipulates the viewer's lighting attributes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 17 09:14:36 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Aug 20 10:15:17 PDT 2002
//   I added a couple private pixmaps.
//
//   Brad Whitlock, Wed Mar 26 08:08:12 PDT 2003
//   I added a brightness text field.
//
//   Brad Whitlock, Wed Feb 23 17:57:53 PST 2005
//   Added another slot to handle setting the brightness.
//
// ****************************************************************************

class GUI_API QvisLightingWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisLightingWindow(LightList *subj,
                       const char *caption = 0,
                       const char *shortName = 0,
                       QvisNotepadArea *notepad = 0);
    virtual ~QvisLightingWindow();
    virtual void CreateWindowContents();
protected:
    virtual void UpdateWindow(bool doAll);
    void Apply(bool ignore = false);
    void GetCurrentValues(int which_widget);
    void UpdateLightWidget();
private slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();

    void activeLightComboBoxChanged(int);
    void brightnessChanged(int);
    void brightnessChanged2(int);
    void enableToggled(bool);
    void lightMoved(double x, double y, double z);
    void lightTypeComboBoxChanged(int);
    void modeClicked(int);
    void processLineDirectionText();
    void selectedLightColor(const QColor &);
private:
    int                 activeLight;
    LightList          *lights;
    int                 mode;

    QPixmap            *onLightIcon;
    QPixmap            *offLightIcon;
    QButtonGroup       *modeButtons;
    QvisLightingWidget *lightWidget;
    QLabel             *activeLightLabel;
    QComboBox          *activeLightComboBox;
    QGroupBox          *lightGroupBox;
    QComboBox          *lightTypeComboBox;
    QCheckBox          *lightEnabledCheckBox;
    QLabel             *lightDirectionLabel;
    QLineEdit          *lightDirectionLineEdit;
    QvisColorButton    *lightColorButton;
    QvisOpacitySlider  *lightBrightness;
    QSpinBox           *lightBrightnessSpinBox;
};

#endif
