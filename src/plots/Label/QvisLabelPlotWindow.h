// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISLABELPLOTWINDOW_H
#define QVISLABELPLOTWINDOW_H

#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

class LabelAttributes;
class FontAttributes;
class QFrame;
class QLabel;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QDoubleSpinBox;
class QLineEdit;
class QSpinBox;
class QvisFontAttributesWidget;

// ****************************************************************************
// Class: QvisLabelPlotWindow
//
// Purpose: 
//   Defines QvisLabelPlotWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 21 18:18:16 PST 2004
//
// Modifications:
//   Brad Whitlock, Tue Aug 2 14:55:40 PST 2005
//   I removed some controls then I added other controls for setting the
//   colors and heights for node labels and cell labels independently, also
//   for z-buffering.
//
//   Cyrus Harrison, Fri Jul 18 14:38:14 PDT 2008
//   Initial Qt4 Port.
//
//   Hank Childs, Wed Oct 20 10:45:46 PDT 2010
//   Change "Label height" to be a QDoubleSpinBox.
//
// ****************************************************************************

class QvisLabelPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisLabelPlotWindow(const int type,
                         LabelAttributes *subj,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisLabelPlotWindow();
    virtual void CreateWindowContents();
public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
protected:
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
private slots:
    void showNodesToggled(bool val);
    void showCellsToggled(bool val);
    void restrictNumberOfLabelsToggled(bool val);
    void numberOfLabelsChanged(int val);
    void drawLabelsFacingChanged(int val);
    void depthTestButtonGroupChanged(int);
    void labelDisplayFormatChanged(int val);

    void textFont1Changed(const FontAttributes &);
    void textFont2Changed(const FontAttributes &);

    void horizontalJustificationChanged(int val);
    void verticalJustificationChanged(int val);
    void formatTemplateChanged();

    void legendToggled(bool val);
private:
    int              plotType;
    LabelAttributes *labelAtts;

    QGroupBox       *selectionGroupBox;
    QCheckBox       *showNodesToggle;
    QCheckBox       *showCellsToggle;
    QCheckBox       *restrictNumberOfLabelsToggle;
    QSpinBox        *numberOfLabelsSpinBox;
    QComboBox       *drawLabelsFacingComboBox;
    QButtonGroup    *depthTestButtonGroup;

    QGroupBox       *formattingGroupBox;
    QComboBox       *labelDisplayFormatComboBox;

    QvisFontAttributesWidget  *textFont1;
    QvisFontAttributesWidget  *textFont2;
    QFrame          *cellFrame;
    QLabel          *cellLabel;
    QFrame          *nodeFrame;
    QLabel          *nodeLabel;
    QFrame          *bottomFrame;

    QComboBox       *horizontalJustificationComboBox;
    QComboBox       *verticalJustificationComboBox;
    QLineEdit       *formatTemplate;

    QCheckBox       *legendToggle;
};

#endif
