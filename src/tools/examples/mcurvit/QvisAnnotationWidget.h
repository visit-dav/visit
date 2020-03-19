// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_ANNOTATION_WIDGET_H
#define QVIS_ANNOTATION_WIDGET_H
#include <QWidget>
#include <SimpleObserver.h>

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;

class QNarrowLineEdit;

class VisItViewer;

class AnnotationAttributes;
class AnnotationObjectList;
class ViewAxisArrayAttributes;

// ****************************************************************************
// Class: QvisAnnotaionWidget
//
// Purpose:
//   Widget that makes it easier to show/update AxisAttributes objects.
//
// Programmer: Eric Brugger
// Creation:   Mon Feb  2 09:20:22 PST 2009
//
// Modifications:
//
// ****************************************************************************

class QvisAnnotationWidget : public QWidget, public SimpleObserver
{
    Q_OBJECT
public:
    QvisAnnotationWidget(QWidget *parent, VisItViewer *v);
    virtual ~QvisAnnotationWidget();

    virtual void Update(Subject *subject);
    virtual void SubjectRemoved(Subject *subject);

    void ConnectAnnotationAttributes(AnnotationAttributes *a);
    void ConnectViewAxisArrayAttributes(ViewAxisArrayAttributes *v);
    void ConnectAnnotationObjectList(AnnotationObjectList *a);

    void CreateWindow();
    void UpdateAnnotationWidgets();
    void UpdateViewAxisArrayWidgets();
    void UpdateAnnotationObjectListWidgets();

    bool QStringToDoubles(const QString &, double *, int);
    QString DoublesToQString(const double *, int);
    bool LineEditGetDouble(QLineEdit *lineEdit, double &val);

signals:
    void annotationChanged(const AnnotationAttributes *);
    void viewAxisArrayChanged(const ViewAxisArrayAttributes *);
    void annotationObjectListChanged(const AnnotationObjectList *);
private slots:
    void titleToggled(bool val);
    void titleFontFamilyChanged(int value);
    void titleBoldToggled(bool val);
    void titleItalicToggled(bool val);
    void labelToggled(bool val);
    void labelFontFamilyChanged(int value);
    void labelBoldToggled(bool val);
    void labelItalicToggled(bool val);
    void legendFontFamilyChanged(int value);
    void legendBoldToggled(bool val);
    void legendItalicToggled(bool val);
    void tickToggled(bool val);
    void processTitleFontScaleText();
    void processLabelFontScaleText();
    void processLegendFontHeightText();
    void processMajorMinimumText();
    void processMajorMaximumText();
    void processMinorSpacingText();
    void processMajorSpacingText();
    void processViewportText();
    void processDomainText();
    void processRangeText();
private:
    VisItViewer             *viewer;

    AnnotationAttributes    *annotationAtts;
    ViewAxisArrayAttributes *axisArrayAtts;
    AnnotationObjectList    *annotationObjectList;

    QGroupBox               *titleGroup;
    QComboBox               *titleFontFamilyComboBox;
    QNarrowLineEdit         *titleFontScale;
    QCheckBox               *titleBoldCheckBox;
    QCheckBox               *titleItalicCheckBox;
    QGroupBox               *labelGroup;
    QComboBox               *labelFontFamilyComboBox;
    QNarrowLineEdit         *labelFontScale;
    QCheckBox               *labelBoldCheckBox;
    QCheckBox               *labelItalicCheckBox;
    QGroupBox               *legendGroup;
    QComboBox               *legendFontFamilyComboBox;
    QNarrowLineEdit         *legendFontHeight;
    QCheckBox               *legendBoldCheckBox;
    QCheckBox               *legendItalicCheckBox;
    QGroupBox               *tickGroup;
    QLabel                  *majorMinimumLabel;
    QNarrowLineEdit         *majorMinimum;
    QLabel                  *majorMaximumLabel;
    QNarrowLineEdit         *majorMaximum;
    QLabel                  *minorSpacingLabel;
    QNarrowLineEdit         *minorSpacing;
    QLabel                  *majorSpacingLabel;
    QNarrowLineEdit         *majorSpacing;
    QGroupBox               *viewGroup;
    QLineEdit               *viewportLineEdit;
    QLabel                  *viewportLabel;
    QLineEdit               *domainLineEdit;
    QLabel                  *domainLabel;
    QLineEdit               *rangeLineEdit;
    QLabel                  *rangeLabel;
};

#endif
