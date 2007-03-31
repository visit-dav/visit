#ifndef QVISTOPOLOGYPLOTWINDOW_H
#define QVISTOPOLOGYPLOTWINDOW_H

#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

class TopologyAttributes;
class QLabel;
class QCheckBox;
class QLineEdit;
class QSpinBox;
class QVBox;
class QButtonGroup;
class QvisColorTableButton;
class QvisOpacitySlider;
class QvisColorButton;
class QvisLineStyleWidget;
class QvisLineWidthWidget;

// ****************************************************************************
// Class: QvisTopologyPlotWindow
//
// Purpose: 
//   Defines QvisTopologyPlotWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Tue Jul 1 08:44:44 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class QvisTopologyPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisTopologyPlotWindow(const int type,
                         TopologyAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisTopologyPlotWindow();
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
    void toleranceProcessText();
    void ColorChanged(const QColor &color);
    void lineWidthChanged(int style);
    void lineStyleChanged(int style);
    void hitpercentProcessText();
    void minOpacityChanged(int opacity, const void*);
    void minPlateauOpacityChanged(int opacity, const void*);
    void maxPlateauOpacityChanged(int opacity, const void*);
    void maxOpacityChanged(int opacity, const void*);
  private:
    int plotType;
    QLineEdit *tolerance;
    QvisColorButton *minColorButton;
    QvisColorButton *minPlateauColorButton;
    QvisColorButton *maxPlateauColorButton;
    QvisColorButton *maxColorButton;
    QvisLineWidthWidget *lineWidth;
    QvisLineStyleWidget *lineStyle;
    QLineEdit *hitpercent;
    QvisOpacitySlider *minOpacity;
    QvisOpacitySlider *minPlateauOpacity;
    QvisOpacitySlider *maxPlateauOpacity;
    QvisOpacitySlider *maxOpacity;

    TopologyAttributes *atts;
};



#endif
