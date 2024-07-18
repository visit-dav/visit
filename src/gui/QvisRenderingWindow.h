// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_RENDERING_WINDOW_H
#define QVIS_RENDERING_WINDOW_H
#include <QvisPostableWindowSimpleObserver.h>
#include <gui_exports.h>
#include <GUIBase.h>

// Forward declarations
class QButtonGroup;
class QCheckBox;
class QLabel;
class QGroupBox;
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
//   Jeremy Meredith, Fri Apr 30 14:25:11 EDT 2010
//   Added depth cueing automatic mode.
//
//   Dave Pugmire, Tue Aug 24 11:32:12 EDT 2010
//   Add compact domain options.
//
//   Eric Brugger, Tue Oct 25 12:29:09 PDT 2011
//   Add a multi resolution display capability for AMR data.
//
//   Burlen Loring, Thu Aug 13 08:38:52 PDT 2015
//   Added options for depth peeling
//
//   Burlen Loring, Sun Sep  6 08:44:26 PDT 2015
//   Added option for ordered composting
//
//   Burlen Loring, Tue Sep 29 11:49:41 PDT 2015
//   Added option to set the compositer thread pool size
//
//   Kathleen Biagas, Wed Aug 17, 2022
//   Incorporate ARSanderson's OSPRAY 2.8.0 work for VTK 9:
//   add osprayGroup.
//
// ****************************************************************************

class GUI_API QvisRenderingWindow : public QvisPostableWindowSimpleObserver
{
    Q_OBJECT
public:
    QvisRenderingWindow(const QString &caption = QString(),
                        const QString &shortName = QString(),
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
    void updateOrderedComposite();
    void updateDepthCompositeThreads();
    void updateDepthCompositeBlocking();
    void updateAlphaCompositeThreads();
    void updateAlphaCompositeBlocking();
    void updateDepthPeeling();
    void multiresolutionModeToggled(bool);
    void processMultiresolutionSmallestCellText();
    void processMultiresolutionSmallestCellText(const QString &);
    void objectRepresentationChanged(int);
    void stereoToggled(bool);
    void stereoTypeChanged(int);
    void renderNotifyToggled(bool);
    void scalrenActivationModeChanged(int);
    void scalrenAutoThresholdChanged(int val);
    void compactDomainsActivationModeChanged(int);
    void compactDomainsAutoThresholdChanged(int val);
    void scalrenCompressModeChanged(int);
    void specularToggled(bool);
    void specularStrengthChanged(int, const void*);
    void specularPowerChanged(int, const void*);
    void shadowToggled(bool);
    void shadowStrengthChanged(int, const void*);
    void depthCueingToggled(bool);
    void depthCueingAutoToggled(bool);
    void depthCueingStartChanged();
    void depthCueingEndChanged();
    void colorTexturingToggled(bool);
    void osprayRenderingToggled(bool);
    void ospraySPPChanged(int);
    void osprayAOChanged(int);
    void osprayShadowsToggled(bool);

private:
    QWidget *CreateBasicPage();
    QWidget *CreateAdvancedPage();
    QWidget *CreateInformationPage();

    RenderingAttributes *renderAtts;
    WindowInformation   *windowInfo;

    // Basic controls
    QCheckBox         *antialiasingToggle;
    QLabel            *restartLabel;
    QCheckBox         *orderedComposite;
    QLabel            *compositeLabel;
    QLabel            *depthCompositeThreadsLabel;
    QLineEdit         *depthCompositeThreads;
    QLabel            *alphaCompositeThreadsLabel;
    QLineEdit         *alphaCompositeThreads;
    QLabel            *depthCompositeBlockingLabel;
    QLineEdit         *depthCompositeBlocking;
    QLabel            *alphaCompositeBlockingLabel;
    QLineEdit         *alphaCompositeBlocking;
    QCheckBox         *depthPeeling;
    QLabel            *occlusionRatioLabel;
    QLineEdit         *occlusionRatio;
    QLabel            *numberOfPeelsLabel;
    QLineEdit         *numberOfPeels;
    QCheckBox         *multiresolutionModeToggle;
    QLabel            *multiresolutionSmallestCellLabel;
    QLineEdit         *multiresolutionSmallestCellLineEdit;
    QButtonGroup      *objectRepresentation;
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
    QButtonGroup      *compactDomainsActivationMode;
    QRadioButton      *compactDomainsAuto;
    QRadioButton      *compactDomainsAlways;
    QRadioButton      *compactDomainsNever;
    QLabel            *compactDomainsGeometryLabel;
    QSpinBox          *compactDomainsAutoThreshold;
    QCheckBox         *shadowToggle;
    QLabel            *shadowStrengthLabel;
    QvisOpacitySlider *shadowStrengthSlider;
    QCheckBox         *depthCueingToggle;
    QCheckBox         *depthCueingAutoToggle;
    QLabel            *depthCueingStartLabel;
    QLineEdit         *depthCueingStartEdit;
    QLabel            *depthCueingEndLabel;
    QLineEdit         *depthCueingEndEdit;
    QCheckBox         *colorTexturingToggle;
#if defined(HAVE_OSPRAY)
    QGroupBox         *osprayGroup;
    QLabel            *ospraySPPLabel;
    QSpinBox          *ospraySPP;
    QLabel            *osprayAOLabel;
    QSpinBox          *osprayAO;
    QCheckBox         *osprayShadowsToggle;
#endif
    // Labels to display renderer information.
    QLabel            *scalrenUsingLabel;
    QLabel            *compactDomainsUsingLabel;
    QLabel            *fpsLabel;
    QLabel            *fpsMinLabel;
    QLabel            *fpsAvgLabel;
    QLabel            *fpsMaxLabel;
    QLabel            *approxNumPrimitives;
    QLabel            *extents[6];
};

#endif
