#ifndef QVIS_STREAMLINE_PLOT_WINDOW_H
#define QVIS_STREAMLINE_PLOT_WINDOW_H
#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

// Forward declarations
class QLabel;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLineEdit;
class QSpinBox;
class QvisColorTableButton;
class QvisColorButton;
class QvisLineWidthWidget;
class StreamlineAttributes;

// ****************************************************************************
// Class: QvisStreamlinePlotWindow
//
// Purpose: 
//   Defines QvisStreamlinePlotWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:12:49 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Dec 22 13:03:49 PST 2004
//   Added support for coloring by vorticity and ribbons.
//
//   Brad Whitlock, Mon Jan 3 15:32:24 PST 2005
//   Added override of ProcessOldVersions.
//
// ****************************************************************************

class QvisStreamlinePlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisStreamlinePlotWindow(const int type,
                         StreamlineAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisStreamlinePlotWindow();
    virtual void CreateWindowContents();

    virtual void ProcessOldVersions(DataNode *node, const char *ver);
  public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
  protected:
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
    void UpdateSourceAttributes();
  private slots:
    void sourceTypeChanged(int val);
    void stepLengthProcessText();
    void maxTimeProcessText();
    void pointSourceProcessText();
    void lineStartProcessText();
    void lineEndProcessText();
    void planeOriginProcessText();
    void planeNormalProcessText();
    void planeUpAxisProcessText();
    void planeRadiusProcessText();
    void sphereOriginProcessText();
    void sphereRadiusProcessText();
    void pointDensityChanged(int val);
    void displayMethodChanged(int val);
    void showStartChanged(bool val);
    void radiusProcessText();
    void lineWidthChanged(int style);
    void coloringMethodChanged(int val);
    void colorTableNameChanged(bool useDefault, const QString &ctName);
    void singleColorChanged(const QColor &color);
    void legendFlagChanged(bool val);
    void lightingFlagChanged(bool val);
    void boxExtentsProcessText();
  private:
    int plotType;
    QComboBox *sourceType;
    QGroupBox *sourceAtts;
    QLineEdit *stepLength;
    QLineEdit *maxTime;
    QLineEdit *pointSource;
    QLabel    *pointSourceLabel;
    QLineEdit *lineStart;
    QLabel    *lineStartLabel;
    QLineEdit *lineEnd;
    QLabel    *lineEndLabel;
    QLineEdit *planeOrigin;
    QLabel    *planeOriginLabel;
    QLineEdit *planeNormal;
    QLabel    *planeNormalLabel;
    QLineEdit *planeUpAxis;
    QLabel    *planeUpAxisLabel;
    QLineEdit *planeRadius;
    QLabel    *planeRadiusLabel;
    QLineEdit *sphereOrigin;
    QLabel    *sphereOriginLabel;
    QLineEdit *sphereRadius;
    QLabel    *sphereRadiusLabel;
    QLineEdit *boxExtents[3];
    QLabel    *boxExtentsLabel[3];
    QSpinBox  *pointDensity;
    QComboBox *displayMethod;
    QCheckBox *showStart;
    QLineEdit *radius;
    QLabel    *radiusLabel;
    QvisLineWidthWidget *lineWidth;
    QLabel    *lineWidthLabel;
    QComboBox *coloringMethod;
    QvisColorTableButton *colorTableName;
    QLabel    *colorTableNameLabel;
    QvisColorButton *singleColor;
    QLabel    *singleColorLabel;
    QCheckBox *legendFlag;
    QCheckBox *lightingFlag;

    StreamlineAttributes *streamAtts;
};



#endif
