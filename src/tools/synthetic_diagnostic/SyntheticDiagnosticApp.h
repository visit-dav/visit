#ifndef MULTI_WINDOW_VIS_APP_H
#define MULTI_WINDOW_VIS_APP_H
#include <QMainWindow>
class VisItViewer;
class QListWidget;
class vtkQtRenderWindow;

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
    void onSelectVariable(const QString &var);
    void resetWindow();
    void openGUI();
    void showTheWindow();
private:
    bool               windowsAdded;
    QString            activeFile;
    VisItViewer       *viewer;
    QListWidget       *variables;
    int                currentWindow;
    vtkQtRenderWindow *viswindows[NWINDOWS+1];
};

#endif
