// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//   Brad Whitlock, Wed Apr  9 11:08:03 PDT 2008
//   QString for caption, shortName.
//
//   Hank Childs, Fri Aug  6 07:13:17 PDT 2010
//   Add data member for keeping tracking if the enabled toggle has changed.
//
// ****************************************************************************

class GUI_API QvisLightingWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisLightingWindow(LightList *subj,
                       const QString &caption = QString(),
                       const QString &shortName = QString(),
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
    bool                enableToggledSinceApply;

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
