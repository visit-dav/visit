#ifndef QVISKERBELPLOTWINDOW_H
#define QVISKERBELPLOTWINDOW_H

#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

class KerbelAttributes;
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
// Class: QvisKerbelPlotWindow
//
// Purpose: 
//   Defines QvisKerbelPlotWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Fri Nov 15 10:46:12 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

class QvisKerbelPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisKerbelPlotWindow(const int type,
                         KerbelAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisKerbelPlotWindow();
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
    void CenterProcessText();
    void RadiusProcessText();
    void opacityChanged(int opacity, const void*);
    void colorTableNameChanged(bool useDefault, const QString &ctName);
    void legendFlagChanged(bool val);
    void filenameProcessText();
  private:
    int plotType;
    QLineEdit *Center;
    QLineEdit *Radius;
    QvisOpacitySlider *opacity;
    QvisColorTableButton *colorTableName;
    QCheckBox *legendFlag;
    QLineEdit *filename;

    KerbelAttributes *atts;
};



#endif
