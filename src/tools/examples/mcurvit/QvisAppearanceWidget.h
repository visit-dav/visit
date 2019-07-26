// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_APPEARANCE_WIDGET_H
#define QVIS_APPEARANCE_WIDGET_H
#include <QWidget>
#include <SimpleObserver.h>

class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;

class QvisColorButton;
class QvisColorManagerWidget;
class QvisLineWidthWidget;
class QvisOpacitySlider;

class VisItViewer;

class AttributeSubject;

// ****************************************************************************
// Class: QvisAppearanceWidget
//
// Purpose:
//   Widget that handles the appearance widgets.
//
// Programmer: Eric Brugger
// Creation:   Mon Feb  2 17:44:56 PST 2009
//
// Modifications:
//
// ****************************************************************************

class QvisAppearanceWidget : public QWidget, public SimpleObserver
{
    Q_OBJECT
public:
    QvisAppearanceWidget(QWidget *parent, VisItViewer *v);
    virtual ~QvisAppearanceWidget();

    virtual void Update(Subject *subject);
    virtual void SubjectRemoved(Subject *subject);

    void ConnectMultiCurveAttributes(AttributeSubject *m);

    void CreateWindow();
    void UpdateMultiCurveWidgets();

    bool LineEditGetDouble(QLineEdit *lineEdit, double &val);

signals:
    void multiCurveChanged(const AttributeSubject *);
private slots:
    void colorModeChanged(int index);
    void singleColorChanged(const QColor &color);
    void singleColorOpacityChanged(int opacity);
    void multipleColorChanged(const QColor &color, int index);
    void opacityChanged(int opacity, int index);
    void lineWidthChanged(int style);
    void yAxisTitleFormatProcessText();
    void useYAxisTickSpacingChanged(bool val);
    void yAxisTickSpacingProcessText();
    void displayMarkersChanged(bool val);
    void displayIdsChanged(bool val);
    void legendChanged(bool val);
private:
    VisItViewer             *viewer;

    AttributeSubject        *multiCurveAtts;

    QGroupBox               *curveColorGroup;
    QButtonGroup            *colorModeButtons;
    QvisColorButton         *singleColor;
    QvisOpacitySlider       *singleColorOpacity;
    QvisColorManagerWidget  *multipleColors;
    QLabel                  *lineWidthLabel;
    QvisLineWidthWidget     *lineWidth;
    QLabel                  *yAxisTitleFormatLabel;
    QLineEdit               *yAxisTitleFormat;
    QCheckBox               *useYAxisTickSpacing;
    QLineEdit               *yAxisTickSpacing;
    QCheckBox               *displayMarkers;
    QCheckBox               *displayIds;
    QCheckBox               *legend;
};

#endif
