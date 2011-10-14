#ifndef VTKQT_RENDER_WINDOW_H
#define VTKQT_RENDER_WINDOW_H
#include <vtkqt_exports.h>
#include <QMainWindow>
// Veneer over QVTKWidget2 to make it look like VisIt's vtkQtRenderWindow.

class vtkRenderWindow;
class vtkRenderWindowInteractor;

class vtkQtRenderWindowPrivate;

class VTKQT_API vtkQtRenderWindow : public QMainWindow
{
public:
    vtkQtRenderWindow(QWidget *parent = 0, Qt::WindowFlags f = Qt::Widget);
    virtual ~vtkQtRenderWindow();

    static vtkQtRenderWindow *New();
    void Delete();

    // Description:
    // Get the vtk render window.
    vtkRenderWindow* GetRenderWindow();

    // Description:
    // Get the Qt/vtk interactor that was either created by default or set by the user
    vtkRenderWindowInteractor* GetInteractor();

    // Description:
    // Get the GL part of the window as a QWidget.
    QWidget *GetGLWidget();

    // Added by LLNL
    // Description:
    // Callback for a window resize event.
    void SetResizeCallback(void (*callback)(void *), void*);

    // Added by LLNL
    // Description:
    // Sets callback function pointer that is called when window closes.
    void SetCloseCallback(void (*callback)(void *), void*);

    // Added by LLNL
    // Description:
    // Sets callback function pointer that is called when window hides.
    void SetHideCallback(void (*callback)(void *), void*);

    // Added by LLNL
    // Description:
    // Sets callback function pointer that is called when window shows.
    void SetShowCallback(void (*callback)(void *), void*);

    // Added by LLNL
    // Description:
    // Allows the GL widget to tell the window that its size changed. This
    // is called mostly when toolbars are repositioned in such a way that the
    // size of the GL widget changes.
//    void TellGLSize(int w, int h);

    // Added by LLNL
    // Description:
    // Needed to set the format
//    virtual void SetStereoCapableWindow(int capable);

protected:
    virtual void resizeEvent(QResizeEvent *);
    virtual void closeEvent(QCloseEvent *);
    virtual void hideEvent(QHideEvent *);
    virtual void showEvent(QShowEvent *);
private:
    vtkQtRenderWindowPrivate *d;
};

#endif
