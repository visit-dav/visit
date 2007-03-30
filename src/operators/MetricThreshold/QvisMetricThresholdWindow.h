#ifndef QVISMETRICTHRESHOLDWINDOW_H
#define QVISMETRICTHRESHOLDWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class MetricThresholdAttributes;
class QLabel;
class QCheckBox;
class QLineEdit;
class QSpinBox;
class QVBox;
class QComboBox;
class QvisColorTableButton;
class QvisOpacitySlider;
class QvisColorButton;
class QvisLineStyleWidget;
class QvisLineWidthWidget;

// ****************************************************************************
// Class: QvisMetricThresholdWindow
//
// Purpose: 
//   Defines QvisMetricThresholdWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Mon Jul 29 14:33:19 PST 2002
//
// Modifications:
//   
// ****************************************************************************

class QvisMetricThresholdWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisMetricThresholdWindow(const int type,
                         MetricThresholdAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisMetricThresholdWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void presetChanged(int val);
    void HexahedronChanged(bool val);
    void hex_lowerProcessText();
    void hex_upperProcessText();
    void TetrahedronChanged(bool val);
    void tet_lowerProcessText();
    void tet_upperProcessText();
    void WedgeChanged(bool val);
    void wed_lowerProcessText();
    void wed_upperProcessText();
    void PyramidChanged(bool val);
    void pyr_lowerProcessText();
    void pyr_upperProcessText();
    void TriangleChanged(bool val);
    void tri_lowerProcessText();
    void tri_upperProcessText();
    void QuadChanged(bool val);
    void quad_lowerProcessText();
    void quad_upperProcessText();
  private:
    QComboBox *preset;
    QCheckBox *Hexahedron;
    QLineEdit *hex_lower;
    QLineEdit *hex_upper;
    QCheckBox *Tetrahedron;
    QLineEdit *tet_lower;
    QLineEdit *tet_upper;
    QCheckBox *Wedge;
    QLineEdit *wed_lower;
    QLineEdit *wed_upper;
    QCheckBox *Pyramid;
    QLineEdit *pyr_lower;
    QLineEdit *pyr_upper;
    QCheckBox *Triangle;
    QLineEdit *tri_lower;
    QLineEdit *tri_upper;
    QCheckBox *Quad;
    QLineEdit *quad_lower;
    QLineEdit *quad_upper;

    MetricThresholdAttributes *atts;
};



#endif
