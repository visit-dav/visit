#ifndef QVIS_VECTOR_WINDOW_H
#define QVIS_VECTOR_WINDOW_H
#include <QvisPostableWindowObserver.h>

// Forward declarations
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QvisColorButton;
class QvisColorTableButton;
class QvisLineStyleWidget;
class QvisLineWidthWidget;
class QvisOpacitySlider;
class VectorAttributes;

// ****************************************************************************
// Class: QvisVectorPlotWindow
//
// Purpose:
//   This class is a postable window that watches vector plot attributes and
//   always represents their current state.
//
// Notes:      
//
// Programmer: Hank Childs & Brad Whitlock
// Creation:   Thu Mar 22 23:40:52 PST 2001
//
// Modifications:
//   Brad Whitlock, Sat Jun 16 18:21:34 PST 2001
//   I added color table stuff.
//
//   Brad Whitlock, Fri Aug 29 11:34:52 PDT 2003
//   I grouped related fields into group boxes.
//
//   Jeremy Meredith, Fri Nov 21 12:06:46 PST 2003
//   Added radio buttons for vector origin.
//
//   Kathleen Bonnell, Wed Dec 22 16:42:35 PST 2004 
//   Added widgets for min/max and limitsSelection.
//
// ****************************************************************************

class QvisVectorPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisVectorPlotWindow(const int type, VectorAttributes *_vecAtts,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisVectorPlotWindow();
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
    void lineStyleChanged(int newStyle);
    void lineWidthChanged(int newWidth);
    void vectorColorChanged(const QColor &color);
    void processScaleText();
    void processHeadSizeText();
    void reduceMethodChanged(int index);
    void processNVectorsText();
    void processStrideText();
    void legendToggled();
    void drawHeadToggled();
    void colorModeChanged(int);
    void colorTableClicked(bool useDefault, const QString &ctName);
    void originTypeChanged(int);

    void minToggled(bool on);
    void maxToggled(bool on);
    void processMaxLimitText();
    void processMinLimitText();
    void limitsSelectChanged(int);


private:
    int                  plotType;
    VectorAttributes     *vectorAtts;

    QvisLineStyleWidget  *lineStyle;
    QvisLineWidthWidget  *lineWidth;
    QGroupBox            *colorGroupBox;
    QvisColorButton      *vectorColor;
    QButtonGroup         *colorButtonGroup; 
    QvisColorTableButton *colorTableButton;
    QGroupBox            *scaleGroupBox;    
    QLineEdit            *scaleLineEdit;
    QLineEdit            *headSizeLineEdit;
    QGroupBox            *reduceGroupBox;
    QButtonGroup         *reduceButtonGroup;
    QLineEdit            *nVectorsLineEdit;
    QLineEdit            *strideLineEdit;
    QCheckBox            *legendToggle;
    QCheckBox            *drawHeadToggle;
    QButtonGroup         *originButtonGroup;

    QGroupBox             *limitsGroupBox;
    QCheckBox             *minToggle;
    QCheckBox             *maxToggle;
    QComboBox             *limitsSelect;
    QLineEdit             *maxLineEdit;
    QLineEdit             *minLineEdit;
};

#endif
