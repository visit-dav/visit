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

#ifndef QVIS_VOLUME_PLOT_WINDOW_H
#define QVIS_VOLUME_PLOT_WINDOW_H
#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

// SLIVR and Qt share 'emit' causing a conflict
#ifdef HAVE_LIBSLIVR
#   undef emit
#   include <QvisCMap2Display.h>
#endif

// Forward declarations
class VolumeAttributes;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QvisColorTableButton;
class QRadioButton;
class QSlider;
class QVBoxLayout;
class QvisColorSelectionWidget;
class QvisGaussianOpacityBar;
class QvisOpacitySlider;
class QvisScribbleOpacityBar;
class QvisSpectrumBar;
class QvisVariableButton;
class QvisCMap2Widget;
class TransferFunction2D;
typedef int WidgetID;

// ****************************************************************************
// Class: QvisVolumePlotWindow
//
// Purpose:
//   This class contains the widgets that manipulate the transfer function
//   used to do the volume rendering.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 27 11:55:49 PDT 2001
//
// Modifications:
//    Jeremy Meredith, Tue Nov 13 11:46:23 PST 2001
//    Added resample target LineEdit and Slider, and opacity variable LineEdit.
//   
//    Hank Childs, Fri Feb  8 18:53:41 PST 2002
//    Added support for smoothing the data and setting the number of samples
//    per ray.
//
//    Jeremy Meredith, Thu Oct  2 13:09:18 PDT 2003
//    Added settings for the renderer type, the gradient method, and
//    the number of 3D textured slices.
//
//    Jeremy Meredith, Fri Mar 19 15:04:39 PST 2004
//    I added a new callback for when the resample target slider
//    is released.
//
//    Hank Childs, Mon Nov 22 09:27:26 PST 2004
//    Make "Software" button become "Ray Trace" toggle.
//
//    Brad Whitlock, Thu Dec 9 17:32:14 PST 2004
//    I changed the opacity variable so it uses QvisVariableButton.
//
//    Brad Whitlock, Wed Dec 15 09:20:45 PDT 2004
//    I removed the raytrace toggle and made it a rendering mode. Changed to
//    a combobox widget.
//
//    Kathleen Bonnell, Thu Mar  3 11:01:22 PST 2005 
//    Added skewLineEdit and scalingButtons. 
//
//    Hank Childs, Sun Jan  8 08:14:11 PST 2006
//    Added support for kernel based sampling.
//
//    Hank Childs, Mon Sep 11 11:46:01 PDT 2006
//    Created data members for previously untracked radio buttons.
//
//    Gunther Weber, Fri Apr  6 16:31:06 PDT 2007
//    Added data members for showing color spectrum in the opacity
//    editor and also for an inverse linear ramp.
//
//    Brad Whitlock, Fri Jan 11 15:29:40 PST 2008
//    Added renderSamples for SLIVR.
//
//    Tom Fogal, Fri Sep 19 11:17:37 MDT 2008
//    Don't use the CMap2D widget without SLIVR.
//
//    Brad Whitlock, Tue Dec 9 14:41:37 PST 2008
//    Always include a pointer to the QvisCMap2Widget widget or else moc
//    gets the object size confused. We forward declare QvisCMap2Widget
//    and use a dummy class if we don't end up needing it. If we don't do 
//    this, we get weird memory errors when deleting the window when
//    SLIVR is enabled.
//
//    Josh Stratton, Wed Dec 17 12:30:01 MST 2008
//    Added handles for Tuvok's controls
//
//    Brad Whitlock, Thu Dec 18 15:23:06 PST 2008
//    I reorganized the window a little and added methods for dealing with
//    histogram data.
//
// ****************************************************************************

class QvisVolumePlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisVolumePlotWindow(const int type, VolumeAttributes *volumeAtts_,
                         const QString &caption = QString::null,
                         const QString &shortName = QString::null,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisVolumePlotWindow();
    virtual void CreateWindowContents();
public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
protected:
    void UpdateWindow(bool doAll);
    void UpdateHistogram();
    void UpdateColorControlPoints();
    void UpdateGaussianControlPoints();
    void UpdateFreeform();
    void Update2DTransferFunction();
    void Apply(bool ignore = false);
    void GetCurrentValues(int which_widget);
    void CopyGaussianOpacitiesToFreeForm();
    void SetResampleTargetSliderFromAtts();
    void SetRendererSamplesSliderFromAtts();
    QWidget *Create1DTransferFunctionGroup(int);
    QWidget *Create2DTransferFunctionGroup();
    void CreateColorGroup(QWidget *, QVBoxLayout *, int);
    void CreateOpacityGroup(QWidget *, QVBoxLayout *, int);
    void CreateOptions(int);
private slots:
    void addControlPoint();
    void removeControlPoint();
    void alignControlPoints();
    void controlPointMoved(int index, float position);
    void popupColorSelect(int index, const QPoint &);
    void selectedColor(const QColor &color);
    void interactionModeChanged(int index);
    void showColorsInAlphaWidgetToggled(bool);
    void attenuationChanged(int opacity);
    void legendToggled(bool val);
    void lightingToggled(bool val);
    void colorMinToggled(bool val);
    void colorMinProcessText();
    void colorMaxToggled(bool val);
    void colorMaxProcessText();
    void opacityVariableChanged(const QString &);
    void opacityMinToggled(bool val);
    void opacityMinProcessText();
    void opacityMaxToggled(bool val);
    void opacityMaxProcessText();
    void smoothToggled(bool val);
    void smoothDataToggled(bool val);
    void equalSpacingToggled(bool val);
    void alphaValuesChanged();
    void resampleTargetProcessText();
    void resampleTargetSliderChanged(int val);
    void resampleTargetSliderReleased();
    void samplesPerRayProcessText();
    void rendererTypeChanged(int val);
    void gradientTypeChanged(int val);
    void samplingTypeChanged(int val);
    void num3DSlicesProcessText();
    void processSkewText();
    void scaleClicked(int scale);
    void colorTableClicked(bool useDefault, const QString &ctName);
    void rendererSamplesProcessText();
    void rendererSamplesSliderChanged(int val);
    void rendererSamplesSliderReleased();
    void transferDimChanged(int);
    void updateTransferFunc2D();
    void updateTransferFunc2D(WidgetID id);
private:
    int                      plotType;
    VolumeAttributes         *volumeAtts;
    int                      colorCycle;
    bool                     showColorsInAlphaWidget;

    // 1D transfer function widgets
    QWidget                  *tfParent1D;
    QGroupBox                *colorWidgetGroup;
    QCheckBox                *smoothCheckBox;
    QCheckBox                *equalCheckBox;
    QvisSpectrumBar          *spectrumBar;
    QvisColorSelectionWidget *colorSelect;
    QCheckBox                *colorMinToggle;
    QLineEdit                *colorMin;
    QCheckBox                *colorMaxToggle;
    QLineEdit                *colorMax;
    QComboBox                *scaling;
    QLabel                   *skewLabel;
    QLineEdit                *skewLineEdit;
    QvisVariableButton       *opacityVariable;
    QCheckBox                *opacityMinToggle;
    QLineEdit                *opacityMin;
    QCheckBox                *opacityMaxToggle;
    QLineEdit                *opacityMax;
    QGroupBox                *opacityWidgetGroup;
    QCheckBox                *showColorsInAlphaWidgetToggle;
    QButtonGroup             *modeButtonGroup;
    QvisGaussianOpacityBar   *alphaWidget;
    QvisScribbleOpacityBar   *scribbleAlphaWidget;
    QPushButton              *addPointButton;
    QPushButton              *rmPointButton;
    QPushButton              *alignPointButton;
    QvisColorTableButton     *colorTableButton;
    QPushButton              *zeroButton;
    QPushButton              *rampButton;
    QPushButton              *inverseRampButton;
    QPushButton              *oneButton;
    QPushButton              *smoothButton;
    QvisOpacitySlider        *attenuationSlider;

    // 2D transfer function widgets
    QWidget                  *tfParent2D;
    QvisCMap2Widget          *transferFunc2D;

    // General widgets
    QCheckBox                *legendToggle;
    QCheckBox                *lightingToggle;
    QCheckBox                *softwareToggle;
    QCheckBox                *smoothDataToggle;
    QComboBox                *rendererTypesComboBox;
    QButtonGroup             *gradientButtonGroup;
    QButtonGroup             *samplingButtonGroup;
    QButtonGroup             *transferFunctionGroup;
    QRadioButton             *oneDimButton;
    QRadioButton             *twoDimButton;
    QRadioButton             *rasterizationButton;
    QRadioButton             *kernelButton;
    QRadioButton             *centeredDiffButton;
    QRadioButton             *sobelButton;
    QLabel                   *resampleTargetLabel;
    QLineEdit                *resampleTarget;
    QSlider                  *resampleTargetSlider;
    QLabel                   *num3DSlicesLabel;
    QLineEdit                *num3DSlices;
    QLabel                   *samplesPerRayLabel;
    QLineEdit                *samplesPerRay;
    QLabel                   *rendererSamplesLabel;
    QSlider                  *rendererSamplesSlider;
    QLineEdit                *rendererSamples;
};
#endif
