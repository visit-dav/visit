// ----------------------------------------------------------------------------
// File:  DataSet.h
//
// Programmer: Jeremy Meredith
// Date:       August 11, 2003
// ----------------------------------------------------------------------------

#ifndef DATASET_H
#define DATASET_H

#include <string>
#include <vector>
#include "Vector.h"
#include "Shape.h"

struct DataSet
{
    float xmin,xmax;
    float ymin,ymax;
    float zmin,zmax;
    float xsize, ysize, zsize;

    std::vector<Shape> shapes;

    DataSet(ShapeType, int);
    void DrawPolyData(Vector &up, Vector &right);
    void ReInit();

    int selectedShape;

    DataSet *copyOfDataset;
    int      copyOfIndex;
    int      transformNumber;
};

#endif
