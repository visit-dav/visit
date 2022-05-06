// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_VOLUME_PLOT_WINDOW_H
#define QVIS_VOLUME_PLOT_WINDOW_H
#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

// Forward declarations
class VolumeAttributes;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QDoubleSpinBox;
class QVBoxLayout;
class QGridLayout;
class QvisColorTableButton;
class QvisColorSelectionWidget;
class QvisGaussianOpacityBar;
class QvisOpacitySlider;
class QvisScribbleOpacityBar;
class QvisSpectrumBar;
class QvisVariableButton;
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
//    Jeremy Meredith, Mon Jan  4 17:12:16 EST 2010
//    Added ability to reduce amount of lighting for low-gradient-mag areas.
//
//    Jeremy Meredith, Tue Jan  5 14:25:17 EST 2010
//    Added more settings for low-gradient-mag area lighting reduction: more
//    curve shape power, and an optional max-grad-mag-value clamp useful both
//    as an extra tweak and for making animations not have erratic lighting.
//
//    Hank Childs, Fri May 21 12:05:03 PDT 2010
//    Add argument to UpdateHistogram.
//
//    Allen Harvey, Thurs Nov 3 7:21:13 EST 2011
//    Make resampling optional.
//
//    Alister Maguire, Fri May 12 10:15:45 PDT 2017
//    Removed Splatting and Texture3D, and added the Default renderer. 
//
//    Kathleen Biagas, Fri Mar  2 14:53:14 MST 2018
//    Removed Tuvok.
//
//    Kathleen Biagas, Tue Apr 26 2022
//    Removed ProcessOldVersions.
//
// ****************************************************************************

class QvisVolumePlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisVolumePlotWindow(const int type, VolumeAttributes *volumeAtts_,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
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
    void Apply(bool ignore = false);
    void GetCurrentValues(int which_widget);
    void CopyGaussianOpacitiesToFreeForm();
    QWidget *Create1DTransferFunctionGroup(int);
    QWidget *CreateRendererOptionsGroup(int);
    void CreateMatLightGroup(QWidget *parent, QLayout *pLayout, int maxWidth);
    void CreateColorGroup(QWidget *, QVBoxLayout *, int);
    void CreateOpacityGroup(QWidget *, QVBoxLayout *, int);
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
    void resampleToggled(bool val);
    void lowGradientLightingReductionChanged(int val);
    void lowGradientClampToggled(bool val);
    void lowGradientClampProcessText();
    void limitsSelectChanged(int);
    void colorMinToggled(bool val);
    void colorMinProcessText();
    void colorMaxToggled(bool val);
    void colorMaxProcessText();
    void compactVariableChanged(const QString &);
    void opacityVariableChanged(const QString &);
    void opacityMinToggled(bool val);
    void opacityMinProcessText();
    void opacityMaxToggled(bool val);
    void opacityMaxProcessText();
    void smoothingMethodChanged(int val);
    void smoothDataToggled(bool val);
    void equalSpacingToggled(bool val);
    void alphaValuesChanged();
    void resampleTargetChanged(int val);
    void samplesPerRayChanged(int val);
    void rendererTypeChanged(int val);
    void gradientTypeChanged(int val);
    void samplingTypeChanged(int val);
    void processSkewText();
    void scaleClicked(int scale);
    void colorTableClicked(bool useDefault, const QString &ctName);
    void rendererSamplesChanged(double val);
    void setMaterialKa(double val);
    void setMaterialKd(double val);
    void setMaterialKs(double val);
    void setMaterialN(double val);
    void shiftGuassiansLeft();
    void shiftGuassiansRight();
    void raiseGuassians();
    void lowerGuassians();
    void thinGuassians();
    void thickenGuassians();
    void raiseLeftGuassians();
    void raiseRightGuassians();
    void clearAllGuassians();
    void setGuassians();
    void setManyGuassians();
    // ospray options
    void osprayShadowToggled(bool val);
    void osprayUseGridAcceleratorToggled(bool val);
    void osprayPreIntegrationToggled(bool val);
    void ospraySingleShadeToggled(bool val);
    void osprayOneSidedLightingToggled(bool val);
    void osprayAoTransparencyToggled(bool val);
    void ospraySppChanged(int val);
    void osprayAoSamplesChanged(int val);
    void osprayAoDistanceChanged(double val);
    void osprayMinContributionChanged(double val);

private:
    int                      plotType;
    VolumeAttributes         *volumeAtts;
    int                      colorCycle;
    bool                     showColorsInAlphaWidget;

    QVBoxLayout              *rendererOptionsLayout;

    QTabWidget               *tfTabs;

    // 1D transfer function widgets
    QWidget                  *tfParent1D;
    QGroupBox                *colorWidgetGroup;
    QComboBox                *smoothingMethod;
    QCheckBox                *equalCheckBox;
    QvisSpectrumBar          *spectrumBar;
    QvisColorSelectionWidget *colorSelect;
    QComboBox                *limitsSelect;
    QCheckBox                *colorMinToggle;
    QLineEdit                *colorMin;
    QCheckBox                *colorMaxToggle;
    QLineEdit                *colorMax;
    QButtonGroup             *scalingButtons;
    QLineEdit                *skewLineEdit;
    QvisVariableButton       *compactVariable;
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
    QPushButton              *tentButton;
    QPushButton              *oneButton;
    QPushButton              *smoothButton;
    QvisOpacitySlider        *attenuationSlider;

    QWidget                  *tfRendererOptions;

    // General widgets
    QCheckBox                *legendToggle;
    QCheckBox                *lightingToggle;
    QCheckBox                *resampleToggle;
    QGroupBox                *methodsGroup;
    QGroupBox                *lowGradientGroup;
    QLabel                   *lowGradientLightingReductionLabel;
    QComboBox                *lowGradientLightingReductionCombo;
    QCheckBox                *lowGradientClampToggle;
    QLineEdit                *lowGradientClamp;
    QCheckBox                *softwareToggle;
    QCheckBox                *smoothDataToggle;
    QComboBox                *rendererTypesComboBox;
    QButtonGroup             *gradientButtonGroup;
    QButtonGroup             *samplingButtonGroup;
    QWidget                  *samplingMethodWidget;
    QButtonGroup             *transferFunctionGroup;
    QWidget                  *tfWidget;
    QRadioButton             *oneDimButton;
    QRadioButton             *twoDimButton;
    QLabel                   *samplingMethodLabel;
    QRadioButton             *rasterizationButton;
    QRadioButton             *kernelButton;
    QRadioButton             *trilinearButton;
    QRadioButton             *centeredDiffButton;
    QRadioButton             *sobelButton;
    QWidget                  *resampleTargetWidget;
    QLabel                   *resampleTargetLabel;
    QSpinBox                 *resampleTarget;
    QWidget                  *samplesPerRayWidget;
    QLabel                   *samplesPerRayLabel;
    QSpinBox                 *samplesPerRay;
    QWidget                  *rendererSamplesWidget;
    QLabel                   *rendererSamplesLabel;
    QDoubleSpinBox           *rendererSamples;
    QWidget                  *materialProperties;
    QGroupBox                *lightMaterialPropGroup;
    QDoubleSpinBox           *matKa; // ambient
    QDoubleSpinBox           *matKd; // diffuse
    QDoubleSpinBox           *matKs; // specular
    QDoubleSpinBox           *matN;  // specularPower
    QLabel                   *Ka;
    QLabel                   *Kd;
    QLabel                   *Ks;
    QLabel                   *specPow;

    QPushButton              *shiftLeftButton;
    QPushButton              *shiftRightButton;
    QPushButton              *lowerPeaksButton;
    QPushButton              *raisePeaksButton;
    QPushButton              *thinningButton;
    QPushButton              *thickenButton;
    QPushButton              *rightToLeftButton;
    QPushButton              *leftToRightButton;
    QPushButton              *clearGaussButton;
    QPushButton              *setGaussButton;
    QPushButton              *setManyGaussButton;

    //OSPRay group
    QGroupBox               *osprayGroup;
    QGridLayout             *osprayGroupLayout;
    QCheckBox               *osprayShadowToggle;
    QCheckBox               *osprayUseGridAcceleratorToggle;
    QCheckBox               *osprayPreIntegrationToggle;
    QCheckBox               *ospraySingleShadeToggle;
    QCheckBox               *osprayOneSidedLightingToggle;
    QCheckBox               *osprayAoTransparencyToggle;
    QWidget                 *ospraySppWidget;
    QLabel                  *ospraySppLabel;
    QSpinBox                *ospraySpp;
    QWidget                 *osprayAoSamplesWidget;
    QLabel                  *osprayAoSamplesLabel;
    QSpinBox                *osprayAoSamples;
    QWidget                 *osprayAoDistanceWidget;
    QLabel                  *osprayAoDistanceLabel;
    QDoubleSpinBox          *osprayAoDistance;
    QWidget                 *osprayMinContributionWidget;
    QLabel                  *osprayMinContributionLabel;
    QDoubleSpinBox          *osprayMinContribution;
    
    //Sampling group
    QGroupBox               *resampleGroup;
    QWidget                 *defaultOptions;
    QVBoxLayout             *defaultGroupLayout;
    QGroupBox               *defaultGroup;
    QGroupBox               *raycastingGroup;
    void                    CreateSamplingGroups(QWidget *parent, QLayout *pLayout);
    void                    CreateOSPRayGroups(QWidget *parent, QLayout *pLayout);
    void                    UpdateSamplingGroup();
    void                    EnableSamplingMethods(bool enable);
    void                    EnableDefaultGroup();
    void                    UpdateLowGradientGroup(bool enable);

};
#endif
