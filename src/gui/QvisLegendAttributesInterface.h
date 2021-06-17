// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_LEGEND_ATTRIBUTES_INTERFACE_H
#define QVIS_LEGEND_ATTRIBUTES_INTERFACE_H
#include <QvisAnnotationObjectInterface.h>
#include <vectortypes.h>

// Forward declarations
class QComboBox;
class QCheckBox;
class QLabel;
class QNarrowLineEdit;
class QPushButton;
class QSpinBox;
class QTabWidget;
class QTableWidget;
class QvisColorButton;
class QvisOpacitySlider;
class QvisScreenPositionEdit;

// ****************************************************************************
// Class: QvisLegendAttributesInterface
//
// Purpose:
//   This class lets you set attributes for a legend.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 09:18:40 PDT 2007
//
// Modifications:
//    Dave Bremer, Wed Oct  8 11:36:27 PDT 2008
//    Added orientationComboBox
//   
//    Hank Childs, Fri Jan 23 15:19:28 PST 2009
//    Add support for whether or not to draw the "min/max" portion of the
//    legend.
//
//    Kathleen Bonnell, Thu Oct  1 14:46:14 PDT 2009
//    Added controls that allow users to modifiy tick values and labels.
//
// ****************************************************************************

class GUI_API QvisLegendAttributesInterface : public QvisAnnotationObjectInterface
{
    Q_OBJECT
public:
    QvisLegendAttributesInterface(QWidget *parent);
    virtual ~QvisLegendAttributesInterface();

    virtual QString GetName() const { return "Legend"; }
    virtual QString GetMenuText(const AnnotationObject &) const;

    virtual void GetCurrentValues(int which);
    virtual bool AllowInstantiation() const { return false; }
protected:
    virtual void UpdateControls();

    bool GetBool(int bit) const;
    void SetBool(int bit, bool val);
    void ResizeSuppliedLabelsList(int size);

private slots:
    void layoutChanged(bool);
    void positionChanged(double, double);
    void widthChanged(int);
    void heightChanged(int);
    void orientationChanged(int);
    void textChanged();
    void fontHeightChanged();
    void textColorChanged(const QColor &);
    void textOpacityChanged(int);
    void fontFamilyChanged(int);
    void boldToggled(bool);
    void italicToggled(bool);
    void shadowToggled(bool);
    void useForegroundColorToggled(bool);
    void drawMinmaxToggled(bool);
    void drawBoundingBoxToggled(bool);
    void boundingBoxColorChanged(const QColor &);
    void boundingBoxOpacityChanged(int);
    void drawTitleToggled(bool);
    void customTitleToggled(bool);
    void customTitleChanged();

    void tickControlToggled(bool);
    void numTicksChanged(int);
    void minMaxToggled(bool);

    void drawLabelsChanged(int);

    void addSuppliedLabelsRow();
    void deleteSelectedRow();

private:

    QTabWidget             *tabs;
    QCheckBox              *manageLayout;
    QvisScreenPositionEdit *positionEdit;
    QLabel                 *positionLabel;
    QSpinBox               *widthSpinBox;
    QSpinBox               *heightSpinBox;
    QComboBox              *orientationComboBox;
    QNarrowLineEdit        *formatString;
    QNarrowLineEdit        *fontHeight;
    QLabel                 *textColorLabel;
    QvisColorButton        *textColorButton;
    QvisOpacitySlider      *textColorOpacity;
    QComboBox              *fontFamilyComboBox;
    QCheckBox              *boldCheckBox;
    QCheckBox              *italicCheckBox;
    QCheckBox              *shadowCheckBox;
    QCheckBox              *useForegroundColorCheckBox;

    QCheckBox              *drawMinmaxCheckBox;
    QCheckBox              *drawTitleCheckBox;
    QCheckBox              *customTitleCheckBox;
    QNarrowLineEdit        *customTitle;

    QCheckBox              *drawBoundingBoxCheckBox;
    QvisColorButton        *boundingBoxColorButton;
    QvisOpacitySlider      *boundingBoxOpacity;

    QCheckBox              *tickControl; 
    QSpinBox               *numTicksSpinBox; 
    QLabel                 *numTicksLabel;
    QCheckBox              *minMaxCheckBox; 
    QComboBox              *drawLabelsComboBox;

    QTableWidget           *suppliedLabels;
    QPushButton            *addRowButton;
    QPushButton            *deleteRowButton;
};

#endif
