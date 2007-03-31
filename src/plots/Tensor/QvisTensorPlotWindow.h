#ifndef QVIS_VECTOR_WINDOW_H
#define QVIS_VECTOR_WINDOW_H
#include <QvisPostableWindowObserver.h>

// Forward declarations
class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QvisColorButton;
class QvisLineStyleWidget;
class QvisLineWidthWidget;
class TensorAttributes;
class QvisOpacitySlider;
class QvisColorTableButton;

// ****************************************************************************
// Class: QvisTensorPlotWindow
//
// Purpose:
//   This class is a postable window that watches tensor plot attributes and
//   always represents their current state.
//
// Notes:      
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// ****************************************************************************

class QvisTensorPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisTensorPlotWindow(const int type, TensorAttributes *_vecAtts,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisTensorPlotWindow();
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
    void tensorColorChanged(const QColor &color);
    void processScaleText();
    void reduceMethodChanged(int index);
    void processNTensorsText();
    void processStrideText();
    void legendToggled();
    void colorModeChanged(int);
    void colorTableClicked(bool useDefault, const QString &ctName);
private:
    int                  plotType;
    TensorAttributes     *tensorAtts;

    QGroupBox            *colorGroupBox;
    QvisColorButton      *tensorColor;
    QButtonGroup         *colorButtonGroup; 
    QvisColorTableButton *colorTableButton;
    QGroupBox            *scaleGroupBox;    
    QLineEdit            *scaleLineEdit;
    QGroupBox            *reduceGroupBox;
    QButtonGroup         *reduceButtonGroup;
    QLineEdit            *nTensorsLineEdit;
    QLineEdit            *strideLineEdit;
    QCheckBox            *legendToggle;
};

#endif
