#ifndef MULTI_WINDOW_VIS_APP_H
#define MULTI_WINDOW_VIS_APP_H
#include <QMainWindow>
class VisItViewer;
class QListWidget;
class vtkQtRenderWindow;
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
class QvisVariableButton;
class QGridLayout;

#define NWINDOWS           3
 
#define WINDOW_3D 1
#define WINDOW_2D 2
#define WINDOW_1D 3

class SyntheticDiagnosticApp : public QMainWindow
{
    Q_OBJECT
public:
    SyntheticDiagnosticApp(VisItViewer *v);
    virtual ~SyntheticDiagnosticApp();

    // Window creation callback. Returns the vis window pointers to VisIt.
    static vtkQtRenderWindow *ReturnVisWin(void *data);

public slots:
    virtual void show();

private slots:
    void selectFile();
    void openFile(const QString &);
    void onSelectDiagnostic(const QString &var);
    void resetWindow();
    void openGUI();
    void showTheWindow();
private:
    bool               windowsAdded;
    QString            activeFile;
    VisItViewer       *viewer;
    QListWidget       *diagnostics;
    int                currentWindow;
    vtkQtRenderWindow *viswindows[NWINDOWS+1];

    QWidget      *coordinateSystem;
    QButtonGroup *coordinateSystemButtonGroup;
    QWidget      *beamShape;
    QButtonGroup *beamShapeButtonGroup;
    QLineEdit *radius;
    QLineEdit *divergence;
    QWidget      *beamProjection;
    QButtonGroup *beamProjectionButtonGroup;
    QLineEdit *nBeams;
    QLineEdit *offset;
    QLineEdit *angle;
    QLineEdit *origin;
    QWidget      *beamAxis;
    QButtonGroup *beamAxisButtonGroup;
    QLineEdit *poloialAngle;
    QLineEdit *poloialRTilt;
    QLineEdit *poloialZTilt;
    QLineEdit *toroialAngle;
    QWidget      *viewDimension;
    QButtonGroup *viewDimensionButtonGroup;
    QWidget      *beamType;
    QButtonGroup *beamTypeButtonGroup;
    QLineEdit *standardDeviation;
    QLineEdit *sampleDistance;
    QLineEdit *sampleArc;
    QLabel *coordinateSystemLabel;
    QLabel *beamShapeLabel;
    QLabel *radiusLabel;
    QLabel *divergenceLabel;
    QLabel *beamProjectionLabel;
    QLabel *nBeamsLabel;
    QLabel *offsetLabel;
    QLabel *angleLabel;
    QLabel *originLabel;
    QLabel *beamAxisLabel;
    QLabel *poloialAngleLabel;
    QLabel *poloialRTiltLabel;
    QLabel *poloialZTiltLabel;
    QLabel *toroialAngleLabel;
    QLabel *viewDimensionLabel;
    QLabel *beamTypeLabel;
    QLabel *standardDeviationLabel;
    QLabel *sampleDistanceLabel;
    QLabel *sampleArcLabel;    

    void CreateSimulationGUI(QWidget *parent, QGridLayout *layout);
};

#endif
