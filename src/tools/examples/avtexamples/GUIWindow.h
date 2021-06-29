// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef GUI_WINDOW_H
#define GUI_WINDOW_H

#include <QMainWindow>

class vtkRenderWindow;

class GUIWindow : public QMainWindow
{
    Q_OBJECT
public:
    GUIWindow();
    virtual ~GUIWindow();

private slots:
    void spinBoxChanged(int val);

private:
    void DrawCylinder(vtkRenderWindow *renderWindow);
    void CreateMainWindow();
};

#endif
