// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VTKQT_RENDER_WINDOW_H
#define VTKQT_RENDER_WINDOW_H
#include <vtkqt_exports.h>
#include <QMainWindow>

class vtkRenderWindow;
class vtkRenderWindowInteractor;

class vtkQtRenderWindowPrivate;

class VTKQT_API vtkQtRenderWindow : public QMainWindow
{
public:
    vtkQtRenderWindow(QWidget *parent = 0, Qt::WindowFlags f = Qt::Widget);
    vtkQtRenderWindow(bool stereo, QWidget *parent = 0, Qt::WindowFlags f = Qt::Widget);
    virtual ~vtkQtRenderWindow();

    static vtkQtRenderWindow *New();
    static vtkQtRenderWindow *New(bool stereo);
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

protected:
    void resizeEvent(QResizeEvent *) override;
    void closeEvent(QCloseEvent *) override;
    void hideEvent(QHideEvent *) override;
    void showEvent(QShowEvent *) override;
private:
    vtkQtRenderWindowPrivate *d;
};

#endif
