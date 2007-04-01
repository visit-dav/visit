#ifndef QVISLABELPLOTWINDOW_H
#define QVISLABELPLOTWINDOW_H

#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

class LabelAttributes;
class QLabel;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLineEdit;
class QSpinBox;
class QvisColorButton;

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
//   
// ****************************************************************************

class QvisLabelPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisLabelPlotWindow(const int type,
                         LabelAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
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

    void labelDisplayFormatChanged(int val);
    void useForegroundTextColorToggled(bool val);
    void textColorChanged(const QColor &color);
    void textHeightChanged(int val);
    void horizontalJustificationChanged(int val);
    void verticalJustificationChanged(int val);

    void showSingleNodeToggled(bool val);
    void singleNodeProcessText();
    void showSingleCellToggled(bool val);
    void singleCellProcessText();
    void markerProcessText();

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

    QGroupBox       *formattingGroupBox;
    QComboBox       *labelDisplayFormatComboBox;
    QvisColorButton *textColorButton;
    QLabel          *textColorLabel;
    QCheckBox       *useForegroundTextColorToggle;
    QSpinBox        *textHeightSpinBox;
    QComboBox       *horizontalJustificationComboBox;
    QComboBox       *verticalJustificationComboBox;

    QGroupBox       *singleValueGroupBox;
    QCheckBox       *showSingleNodeToggle;
    QLineEdit       *singleNodeLineEdit;
    QCheckBox       *showSingleCellToggle;
    QLineEdit       *singleCellLineEdit;
    QLineEdit       *markerTextLineEdit;

    QCheckBox       *legendToggle;
};

#endif
