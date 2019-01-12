#ifndef MULTI_WINDOW_VIS_APP_H
#define MULTI_WINDOW_VIS_APP_H
#include <QMainWindow>
class VisItViewer;
class QListWidget;
class vtkQtRenderWindow;

#define NWINDOWS           2
 
#define PSEUDOCOLOR_WINDOW 1
#define VOLUME_WINDOW     2

class MultiWindowVisApp : public QMainWindow
{
    Q_OBJECT
public:
    MultiWindowVisApp(VisItViewer *v);
    virtual ~MultiWindowVisApp();

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
    vtkQtRenderWindow *viswindows[NWINDOWS + 1];
};

#endif
