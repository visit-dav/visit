#ifndef QVISTRUECOLORPLOTWINDOW_H
#define QVISTRUECOLORPLOTWINDOW_H

#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

class TruecolorAttributes;
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
// Class: QvisTruecolorPlotWindow
//
// Purpose: 
//   Defines QvisTruecolorPlotWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Tue Jun 15 11:10:32 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

class QvisTruecolorPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisTruecolorPlotWindow(const int type,
                         TruecolorAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisTruecolorPlotWindow();
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
    void opacityChanged(int opacity, const void*);
  private:
    int plotType;
    QvisOpacitySlider *opacity;
    QLabel *opacityLabel;

    TruecolorAttributes *atts;
};



#endif
