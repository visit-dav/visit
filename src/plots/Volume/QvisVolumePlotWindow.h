#ifndef QVIS_VOLUME_PLOT_WINDOW_H
#define QVIS_VOLUME_PLOT_WINDOW_H
#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

// Forward declarations
class VolumeAttributes;
class QButtonGroup;
class QVBoxLayout;
class QPushButton;
class QCheckBox;
class QGroupBox;
class QvisSpectrumBar;
class QvisColorSelectionWidget;
class QvisGaussianOpacityBar;
class QvisScribbleOpacityBar;
class QvisOpacitySlider;
class QLineEdit;
class QSlider;

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
// ****************************************************************************

class QvisVolumePlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisVolumePlotWindow(const int type, VolumeAttributes *volumeAtts_,
                             const char *caption = 0,
                             const char *shortName = 0,
                             QvisNotepadArea *notepad = 0);
    virtual ~QvisVolumePlotWindow();
    virtual void CreateWindowContents();
public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
protected:
    void UpdateWindow(bool doAll);
    void UpdateColorControlPoints();
    void UpdateGaussianControlPoints();
    void UpdateFreeform();
    void Apply(bool ignore = false);
    void GetCurrentValues(int which_widget);
    void CopyGaussianOpacitiesToFreeForm();
    void SetResampleTargetSliderFromAtts();
private slots:
    void addControlPoint();
    void removeControlPoint();
    void alignControlPoints();
    void controlPointMoved(int index, float position);
    void popupColorSelect(int index);
    void selectedColor(const QColor &color);
    void interactionModeChanged(int index);
    void attenuationChanged(int opacity);
    void legendToggled(bool val);
    void lightingToggled(bool val);
    void softwareToggled(bool val);
    void colorMinToggled(bool val);
    void colorMinProcessText();
    void colorMaxToggled(bool val);
    void colorMaxProcessText();
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
    void opacityVariableProcessText();
    void rendererTypeChanged(int val);
    void gradientTypeChanged(int val);
    void num3DSlicesProcessText();
private:
    int                      plotType;
    VolumeAttributes         *volumeAtts;
    int                      colorCycle;

    // Widgets and layouts.
    QGroupBox                *colorWidgetGroup;
    QCheckBox                *smoothCheckBox;
    QCheckBox                *equalCheckBox;
    QvisSpectrumBar          *spectrumBar;
    QvisColorSelectionWidget *colorSelect;
    QCheckBox                *colorMinToggle;
    QLineEdit                *colorMin;
    QCheckBox                *colorMaxToggle;
    QLineEdit                *colorMax;
    QCheckBox                *opacityMinToggle;
    QLineEdit                *opacityMin;
    QCheckBox                *opacityMaxToggle;
    QLineEdit                *opacityMax;
    QGroupBox                *opacityWidgetGroup;
    QButtonGroup             *modeButtonGroup;
    QvisGaussianOpacityBar   *alphaWidget;
    QvisScribbleOpacityBar   *scribbleAlphaWidget;
    QPushButton              *addPointButton;
    QPushButton              *rmPointButton;
    QPushButton              *alignPointButton;
    QPushButton              *zeroButton;
    QPushButton              *rampButton;
    QPushButton              *oneButton;
    QPushButton              *smoothButton;
    QvisOpacitySlider        *attenuationSlider;
    QCheckBox                *legendToggle;
    QCheckBox                *lightingToggle;
    QCheckBox                *softwareToggle;
    QCheckBox                *smoothDataToggle;
    QButtonGroup             *rendererButtonGroup;
    QButtonGroup             *gradientButtonGroup;
    QLineEdit                *resampleTarget;
    QSlider                  *resampleTargetSlider;
    QLineEdit                *num3DSlices;
    QLineEdit                *samplesPerRay;
    QLineEdit                *opacityVariable;
};

#endif
