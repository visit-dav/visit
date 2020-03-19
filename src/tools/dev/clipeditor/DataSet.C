// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ----------------------------------------------------------------------------
// File:  DataSet.C
//
// Programmer: Jeremy Meredith
// Date:       August 11, 2003
// ----------------------------------------------------------------------------

#include "DataSet.h"
#include "Vector.h"
#include "Shape.h"

#include <iostream>

DataSet::DataSet(ShapeType st, int sc)
{
    shapes.push_back(Shape(st, sc, this));

    xmin=-1;
    ymin=-1;
    zmin=-1;
    xmax= 1;
    ymax= 1;
    zmax= 1;
    xsize=2;
    ysize=2;
    zsize=2;

    selectedShape = 0;
    copyOfDataset = NULL;
}

void
DataSet::DrawPolyData(Vector &up, Vector &right)
{
    if (selectedShape > (int)shapes.size())
        selectedShape = 0;

    if (selectedShape > 0)
    {
        shapes[0].DrawPolyData(up, right);
        shapes[selectedShape].DrawPolyData(up, right);
    }
    else
    {
        for (size_t i=0; i<shapes.size(); i++)
        {
            shapes[i].DrawPolyData(up, right);
        }
    }
}

void
DataSet::ReInit()
{
    if (copyOfDataset)
    {
        shapes.resize(copyOfDataset->shapes.size());
        for (size_t i=1; i<copyOfDataset->shapes.size(); i++)
        {
            shapes[i] = Shape(&copyOfDataset->shapes[i],
                              transformNumber,
                              &shapes[0],
                              this);
        }
    }

    for (size_t i=1; i<shapes.size(); i++)
    {
        shapes[i].parentShape = &shapes[0];
        shapes[i].Init();
    }

    selectedShape = 0;
}
