#ifndef SIMPLE_VIS_APP_H
#define SIMPLE_VIS_APP_H
#include <QMainWindow>

class VisItViewer;
class CommandParser;

class QLabel;
class QListWidget;
class QButtonGroup;
class QSpinBox;
class QWidget;
class vtkQtRenderWindow;

// ****************************************************************************
// Class: SyntheticDiagnosticApp
//
// Purpose:
//
// Notes:      
//
//   Dave Pugmire, Wed Nov 24 08:06:26 EST 2010
//
// Modifications:
//   
// ****************************************************************************

class SyntheticDiagnosticApp : public QMainWindow
{
    Q_OBJECT
public:
    SyntheticDiagnosticApp(VisItViewer *);
    virtual ~SyntheticDiagnosticApp();

public slots:
    virtual void show();
private slots:
    void selectFile();
    void openFile(const QString &);
    void changeVariable(const QString &);
    void changeVariableAndUpdate(const QString &);
    void changePlotType(int);
    void setNContours(int);
    void saveWindow();
    void openGUI();
    void execFile();
private:
    static vtkQtRenderWindow *ReturnVisWin(void *);
    void resetWindow();

    QLabel            *scalarLabel;
    QListWidget       *variables;
    QWidget           *plotTypeWidget;
    QButtonGroup      *plotType;
    QWidget           *contourWidget;
    QSpinBox          *nContours;
    vtkQtRenderWindow *viswin;
    VisItViewer       *viewer;

    CommandParser     *cmd;
};

#endif
