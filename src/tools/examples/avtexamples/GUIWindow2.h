// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef GUI_WINDOW2_H
#define GUI_WINDOW2_H

#include <QMainWindow>

class vtkRenderWindow;

class GUIWindow2 : public QMainWindow
{
    Q_OBJECT
public:
    GUIWindow2();
    virtual ~GUIWindow2();

private slots:
    void spinBoxChanged(int val);

private:
    void DrawCylinder(vtkRenderWindow *renderWindow);
    void CreateMainWindow();
};

#endif
