// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ----------------------------------------------------------------------------
// File:  ClipEditor.h
//
// Programmer: Jeremy Meredith
// Date:       August 11, 2003
// ----------------------------------------------------------------------------

#ifndef CLIP_EDITOR_H
#define CLIP_EDITOR_H

#include <QMainWindow>
#include <vector>
#include <string>
#include "DataSet.h"

class Viewer;

class ClipEditor : public QMainWindow
{
    Q_OBJECT
  public:
    ClipEditor(const QString &shape, QWidget *parent);
    
    virtual void keyPressEvent(QKeyEvent *kev);

    void LoadFromFile();
    void SaveToFile();

  protected:
    // Editing
    enum TextMode
    {
        TM_MAIN,
        TM_ADD
    };
    TextMode textMode;

    ShapeType shapetype;
    int       ncases;

    int defaultcolor;

    std::vector<DataSet*> datasets;
    int caseindex;

    std::string pts;
    int npts;
    ShapeType addingShape;
    int addedpoints;


    Viewer *viewer;
};

#endif
