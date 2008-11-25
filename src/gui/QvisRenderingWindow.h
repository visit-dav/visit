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

#ifndef QVIS_RENDERING_WINDOW_H
#define QVIS_RENDERING_WINDOW_H
#include <QvisPostableWindowSimpleObserver.h>
#include <gui_exports.h>

// Forward declarations
class QButtonGroup;
class QCheckBox;
class QLabel;
class QRadioButton;
class QSlider;
class QSpinBox;
class RenderingAttributes;
class WindowInformation;
class QvisOpacitySlider;
class QLineEdit;

// ****************************************************************************
// Class: QvisRenderingWindow
//
// Purpose:
//   This class implements a window that displays rendering settings from the
//   viewer and also allows some settings to be changed.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 18 14:33:24 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Oct 24 13:33:31 PST 2002
//   I made the stereo radio buttons class members.
//
//   Kathleen Bonnell, Wed Dec  4 18:42:48 PST 2002  
//   Removed quality slider, no longer needed. 
//
//   Jeremy Meredith, Fri Nov 14 16:03:31 PST 2003
//   Added specular lighting.
//
//   Mark C. Miller, Tue Apr 27 14:41:35 PDT 2004
//   Added stuff to deal with adjusting scalable threshold with a spinbox 
//
//   Hank Childs, Sun May  9 15:54:29 PDT 2004
//   Add support for multiple display list modes.
//
//   Hank Childs, Sun Oct 24 07:34:09 PDT 2004
//   Add shadows.
//
//   Mark C. Miller, Fri Mar  4 13:05:02 PST 2005
//   Changed approxNumTriangles to approxNumPrimitives
//
//   Kathleen Bonnell, Thu Jun 30 15:29:55 PDT 2005 
//   Added redgreen radio button. 
//
//   Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//   Added compression controls
//
//   Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//   Added fpsLabel.
//
//   Brad Whitlock, Mon Sep 18 10:44:21 PDT 2006
//   Added color texturing check box.
//
//   Jeremy Meredith, Wed Aug 29 15:23:19 EDT 2007
//   Added depth cueing support.  Improved some other thing in the
//   window (like sensitivities and layout).
//
//   Brad Whitlock, Wed Apr  9 11:51:19 PDT 2008
//   QString for caption, shortName.
//
//   Brad Whitlock, Thu Jun 19 12:14:23 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class GUI_API QvisRenderingWindow : public QvisPostableWindowSimpleObserver
{
    Q_OBJECT
public:
    QvisRenderingWindow(const QString &caption = QString::null,
                        const QString &shortName = QString::null,
                        QvisNotepadArea *n = 0);
    virtual ~QvisRenderingWindow();
    virtual void CreateWindowContents();
    virtual void SubjectRemoved(Subject *TheRemovedSubject);

    void ConnectRenderingAttributes(RenderingAttributes *);
    void ConnectWindowInformation(WindowInformation *);
protected slots:
    virtual void apply();
protected:
    virtual void UpdateWindow(bool doAll);
    void UpdateOptions(bool doAll);
    void UpdateInformation(bool doAll);
    void Apply(bool ignore = false);
    void InterpretScalableAutoThreshold(int,int*,QString*,int*) const;
    void GetCurrentValues();
    void UpdateWindowSensitivity();
private slots:
    void antialiasingToggled(bool);
    void objectRepresentationChanged(int);
    void displayListModeChanged(int);
    void stereoToggled(bool);
    void stereoTypeChanged(int);
    void renderNotifyToggled(bool);
    void scalrenActivationModeChanged(int);
    void scalrenAutoThresholdChanged(int val);
    void scalrenCompressModeChanged(int);
    void specularToggled(bool);
    void specularStrengthChanged(int, const void*);
    void specularPowerChanged(int, const void*);
    void shadowToggled(bool);
    void shadowStrengthChanged(int, const void*);
    void depthCueingToggled(bool);
    void depthCueingStartChanged();
    void depthCueingEndChanged();
    void colorTexturingToggled(bool);
private:
    QWidget *CreateBasicPage();
    QWidget *CreateAdvancedPage();
    QWidget *CreateInformationPage();

    RenderingAttributes *renderAtts;
    WindowInformation   *windowInfo;

    // Basic controls
    QCheckBox         *antialiasingToggle;
    QButtonGroup      *objectRepresentation;
    QButtonGroup      *dlMode;
    QCheckBox         *stereoToggle;
    QButtonGroup      *stereoType;
    QRadioButton      *redblue;
    QRadioButton      *interlace;
    QRadioButton      *crystalEyes;
    QRadioButton      *redgreen;
    QCheckBox         *renderNotifyToggle;
    QCheckBox         *specularToggle;
    QLabel            *specularStrengthLabel;
    QvisOpacitySlider *specularStrengthSlider;
    QLabel            *specularPowerLabel;
    QvisOpacitySlider *specularPowerSlider;

    // Advanced controls
    QButtonGroup      *scalrenActivationMode;
    QRadioButton      *scalrenAuto;
    QRadioButton      *scalrenAlways;
    QRadioButton      *scalrenNever;
    QLabel            *scalrenGeometryLabel;
    QSpinBox          *scalrenAutoThreshold;
    QLabel            *scalrenCompressLabel;
    QButtonGroup      *scalrenCompressMode;
    QCheckBox         *shadowToggle;
    QLabel            *shadowStrengthLabel;
    QvisOpacitySlider *shadowStrengthSlider;
    QCheckBox         *depthCueingToggle;
    QLabel            *depthCueingStartLabel;
    QLineEdit         *depthCueingStartEdit;
    QLabel            *depthCueingEndLabel;
    QLineEdit         *depthCueingEndEdit;
    QCheckBox         *colorTexturingToggle;

    // Labels to display renderer information.
    QLabel            *scalrenUsingLabel;
    QLabel            *fpsLabel;
    QLabel            *fpsMinLabel;
    QLabel            *fpsAvgLabel;
    QLabel            *fpsMaxLabel;
    QLabel            *approxNumPrimitives;
    QLabel            *extents[6];
};

#endif
