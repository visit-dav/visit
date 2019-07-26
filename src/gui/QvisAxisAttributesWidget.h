// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.
#ifndef QVIS_AXIS_ATTRIBUTES_WIDGET_H
#define QVIS_AXIS_ATTRIBUTES_WIDGET_H
#include <QWidget>
#include <GUIBase.h>
#include <AxisAttributes.h>

class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QNarrowLineEdit;
class QSpinBox;
class QvisFontAttributesWidget;

// ****************************************************************************
// Class: QvisAxisAttributesWidget
//
// Purpose:
//   Widget that makes it easier to show/update AxisAttributes objects.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 17:55:10 PST 2008
//
// Modifications:
//   Brad Whitlock, Wed Jun 25 09:59:31 PDT 2008
//   Qt 4.
//
//   Jeremy Meredith, Fri Jan 16 11:12:48 EST 2009
//   Allow clients to not expose the "ShowGrid" and custom title/units settings
//
// ****************************************************************************

class QvisAxisAttributesWidget : public QWidget, public GUIBase
{
    Q_OBJECT
public:
    QvisAxisAttributesWidget(QWidget *parent, bool ticksCheckEnabled=true,
                             bool titleCheckEnabled=true,
                             bool showGridEnabled=true,
                             bool customTitleAndUnitsEnabled=true);
    virtual ~QvisAxisAttributesWidget();

    void setAxisAttributes(const AxisAttributes &);
    const AxisAttributes &getAxisAttributes();

    void setAutoScaling(bool val);
    void setAutoTickMarks(bool val);
signals:
    void axisChanged(const AxisAttributes &);
private slots:
    void Apply();

    void titleToggled(bool);
    void customTitleToggled(bool);
    void customUnitsToggled(bool);
    void titleFontChanged(const FontAttributes &);

    void labelToggled(bool);
    void labelScalingChanged(int);
    void labelFontChanged(const FontAttributes &);

    void tickToggled(bool);

    void gridToggled(bool);
private:
    void Update(int,int);
    void GetCurrentValues(AxisAttributes &, int = -1, int = -1);
    bool GetDouble(double &val, QLineEdit *le, const QString &name);

    AxisAttributes           atts;
    bool                     autoScaling;
    bool                     autoTickMarks;

    // Title widgets
    QGroupBox                *titleGroup;
    QCheckBox                *customTitleToggle;
    QLineEdit                *customTitle;
    QCheckBox                *customUnitsToggle;
    QLineEdit                *customUnits;
    QvisFontAttributesWidget *titleFont;

    // Label widgets
    QGroupBox                *labelGroup;
    QLabel                   *labelScalingLabel;
    QSpinBox                 *labelScaling;
    QvisFontAttributesWidget *labelFont;

    // Tick widgets
    QGroupBox                *tickGroup;
    QLabel                   *majorMinimumLabel;
    QNarrowLineEdit          *majorMinimum;
    QLabel                   *majorMaximumLabel;
    QNarrowLineEdit          *majorMaximum;
    QLabel                   *minorSpacingLabel;
    QNarrowLineEdit          *minorSpacing;
    QLabel                   *majorSpacingLabel;
    QNarrowLineEdit          *majorSpacing;

    // Grid widgets
    QCheckBox                *grid;
};

#endif
