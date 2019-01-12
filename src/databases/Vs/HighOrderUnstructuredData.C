/*
 * HighOrderUnstructuredData.C
 *
 *   @file        HighOrderUnstructuredData.C
 *
 *   @brief       Implementation of the HighOrderUnstructuredData.h class
 *
 *   @version $Id: HighOrderUnstructuredData.C 934 2014-02-04 00:01:57Z loverich $
 *
 *  Created on: Jan 18, 2014
 *      Author: loverich
 */

#include "HighOrderUnstructuredData.h"
#include <VsUnstructuredMesh.h>
#include <VsFilter.h>
#include <VsReader.h>
#include <VsLog.h>
#include <algorithm>

#define __CLASS__ "HighOrderUnstructuredData::"

#define CLASSFUNCLINE __CLASS__ << "  " << __FUNCTION__ << "  " << __LINE__

HighOrderUnstructuredData::HighOrderUnstructuredData() {

    VsLog::initialize(DebugStream::Stream3(), DebugStream::Stream4(),
            DebugStream::Stream5());

    VsLog::debugLog() << CLASSFUNCLINE << "  " << "entering" << std::endl;

    delny2d = NULL;
    delny3d = NULL;

    registry = NULL;
    reader = NULL;
    cellArray = NULL;

}

HighOrderUnstructuredData::~HighOrderUnstructuredData() {
}

void HighOrderUnstructuredData::globalPos3d(double n, double m, double p,
        double *v1, double* v2, double* v3, double* v4, double* v5, double* v6,
        double* v7, double* v8, double* pos) {

    //convert back to -1 to 1 range
    n = 2 * n - 1.0;
    m = 2 * m - 1.0;
    p = 2 * p - 1.0;

    for (int i = 0; i < 3; i++) {

        double x1 = v1[i];
        double x2 = v2[i];
        double x3 = v3[i];
        double x4 = v4[i];
        double x5 = v5[i];
        double x6 = v6[i];
        double x7 = v7[i];
        double x8 = v8[i];

        pos[i] = 0.125 * (1 - p)
                * (x1 * (1 - n) * (1 - m) + x2 * (1 + n) * (1 - m)
                        + x3 * (n + 1) * (1 + m) + x4 * (1 - n) * (1 + m));
        pos[i] = pos[i]
                + 0.125 * (1 + p)
                        * (x5 * (1 - n) * (1 - m) + x6 * (1 + n) * (1 - m)
                                + x7 * (n + 1) * (1 + m)
                                + x8 * (1 - n) * (1 + m));
    }

    return;

}

void HighOrderUnstructuredData::globalPos2d(double n, double m, double* v1,
        double* v2, double* v3, double* v4, double* pos) {

    //convert back to -1 to 1 range
    n = 2 * n - 1.0;
    m = 2 * m - 1.0;

    double x1 = v1[0];
    double y1 = v1[1];

    double x2 = v2[0];
    double y2 = v2[1];

    double x3 = v3[0];
    double y3 = v3[1];

    double x4 = v4[0];
    double y4 = v4[1];

    pos[0] = 0.25
            * (x1 * (1 - n) * (1 - m) + x2 * (1 + n) * (1 - m)
                    + x3 * (n + 1) * (1 + m) + x4 * (1 - n) * (1 + m));
    pos[1] = 0.25
            * (y1 * (1 - n) * (1 - m) + y2 * (1 + n) * (1 - m)
                    + y3 * (n + 1) * (1 + m) + y4 * (1 - n) * (1 + m));

    return;
}

void HighOrderUnstructuredData::globalPos1d(double n, double* v1, double* v2,
        double* pos) {

    //convert back to -1 to 1 range
    n = 2 * n - 1.0;

    double x1 = v1[0];
    double x2 = v2[0];

    pos[0] = x1 * (1 - n) + x2 * (1 + n);

    return;
}

//Assume it's a nodal data set following the grid
vtkDoubleArray* HighOrderUnstructuredData::getData(VsVariable* meta,
        std::string name, int tComp) {

    VsLog::debugLog() << CLASSFUNCLINE << "  " << "entering" << " with name "
            << meta->getFullName() << " and component " << tComp
            << " and registry " << registry << std::endl;

    VsDataset* dataSet = registry->getDataset(meta->getFullName());

    VsLog::debugLog() << CLASSFUNCLINE << " dataSet " << dataSet << std::endl;

    std::vector<double> tvar(meta->getLength());

    VsLog::debugLog() << CLASSFUNCLINE << "  " << " dimsSize"
            << meta->getDims().size() << std::endl;

    int components = meta->getDims()[2];

    reader->getData(dataSet, &tvar.front());

    vtkSmartPointer < vtkDoubleArray > array = vtkDoubleArray::New();

    VsUnstructuredMesh* mesh =
            dynamic_cast<VsUnstructuredMesh*>(meta->getMesh());
    numCells = mesh->getCellInfo()->getDims()[0];
    nodesPerCell = mesh->getSubCellLocationsDataset()->getDims()[0];

    array->SetNumberOfComponents(1);
    int tSize = numCells * nodesPerCell;
    array->SetNumberOfTuples(tSize);

    VsLog::debugLog() << CLASSFUNCLINE << " some values: components "
            << components << " numCells " << numCells << " nodesPerCell "
            << nodesPerCell << std::endl;

    int count = 0;
    for (size_t i = 0; i < (size_t)numCells; i++) {
        for (size_t j = 0; j < (size_t)nodesPerCell; j++) {
            double tVal = tvar[i * nodesPerCell * components + j * components
                    + tComp];
            array->SetValue(count, tVal);
            count++;
        }
    }

    array->SetName(name.c_str());

    VsLog::debugLog() << CLASSFUNCLINE << " exiting " << std::endl;

    return array;
}

void HighOrderUnstructuredData::getData(VsUnstructuredMesh* mesh) {

    VsLog::debugLog() << CLASSFUNCLINE << "  " << "entering" << std::endl;

    //Store the subCellLocations in a flattened array
    std::vector<double> subCellLocations;

    //Store the cell (vertex index) information in a flattened array
    std::vector<int> cells;

    //Store the vertex information in a flattened array
    std::vector<double> vertices;

    //Information on the vertices
    VsDataset* verticesInfo;

    //Get information on the cells
    VsDataset* cellsInfo;

    //Get information on the subCellLocation
    VsDataset* subCellLocationsInfo;

    verticesInfo = mesh->getPointsDataset();
    cellsInfo = mesh->getCellInfo();
    subCellLocationsInfo = mesh->getSubCellLocationsDataset();

    vertices.resize(verticesInfo->getLength());
    cells.resize(cellsInfo->getLength());
    subCellLocations.resize(subCellLocationsInfo->getLength());

    reader->getData(verticesInfo, &vertices.front());
    reader->getData(cellsInfo, &cells.front());
    reader->getData(subCellLocationsInfo, &subCellLocations.front());

    verticesPerCell = cellsInfo->getDims()[1];
    numCells = cellsInfo->getDims()[0];
    nodesPerCell = subCellLocationsInfo->getDims()[0];

    VsLog::debugLog() << CLASSFUNCLINE << " verticesPerCell " << verticesPerCell
            << " numCells " << numCells << " nodesPerCell " << nodesPerCell
            << std::endl;

    setNumDimensions();

    //Just reshape the flattened arrays so the access is more obvious
    fillVertexData(vertices);
    fillConnectivity(cells);
    fillQuadPoints(subCellLocations);

    VsLog::debugLog() << CLASSFUNCLINE << "  " << "exiting" << std::endl;

    return;
}

//This assumes you are using quads or hexes.  Will need to be modified for
//the case of triangles or tets.
void HighOrderUnstructuredData::setNumDimensions() {
    VsLog::debugLog() << CLASSFUNCLINE << "  " << "entering" << std::endl;
    switch (verticesPerCell) {
    case 2:
        dimensions = 1;
        break;
    case 4:
        dimensions = 2;
        break;
    case 8:
        dimensions = 3;
        break;
    default:
        VsLog::debugLog() << CLASSFUNCLINE << "  "
                << "number of vertices per cell not invalid" << std::endl;
        break;
    }
    VsLog::debugLog() << CLASSFUNCLINE << " dimensions  " << dimensions
            << std::endl;
}

void HighOrderUnstructuredData::fillVertexData(std::vector<double>& vertices) {
    VsLog::debugLog() << CLASSFUNCLINE << "  " << "entering" << std::endl;
    //Fill in the vertex data
    vertexData.resize(vertices.size() / dimensions);
    for (size_t i = 0; i < vertexData.size(); i++) {
        vertexData[i].resize(3);
        for (int j = 0; j < dimensions; j++) {
            vertexData[i][j] = vertices[i * dimensions + j];
        }
    }
}

void HighOrderUnstructuredData::fillConnectivity(std::vector<int>& cells) {
    VsLog::debugLog() << CLASSFUNCLINE << "  " << "entering" << std::endl;
    //fill connectivity
    connectivity.resize(cells.size() / verticesPerCell);
    for (size_t i = 0; i < connectivity.size(); i++) {
        connectivity[i].resize(verticesPerCell);
        for (int j = 0; j < verticesPerCell; j++) {
            connectivity[i][j] = cells[i * verticesPerCell + j];
        }
    }
}

void HighOrderUnstructuredData::fillQuadPoints(
        std::vector<double>& subCellLocations) {
    VsLog::debugLog() << CLASSFUNCLINE << "  " << "entering" << std::endl;
    //fill quadrature
    quadPoints.resize(subCellLocations.size() / dimensions);
    for (size_t i = 0; i < quadPoints.size(); i++) {
        quadPoints[i].resize(3);
        for (size_t j = 0; j < (size_t)dimensions; j++) {
            quadPoints[i][j] = subCellLocations[i * dimensions + j];
        }
    }
}

void HighOrderUnstructuredData::computeNodes1d(int i, vtkPoints* totalPoints,
        vtkPoints* points) {
    VsLog::debugLog() << CLASSFUNCLINE << " Entering " << std::endl;
    double* v1 = &vertexData[connectivity[i][0]][0];
    double* v2 = &vertexData[connectivity[i][1]][0];

    //Now there are N points inside this data, so
    //compute the points

    size_t numPoints = quadPoints.size();

    for (size_t j = 0; j < numPoints; j++) {

        double m = quadPoints[j][0];

        double x[1] = { 0.0 };
        globalPos1d(m, v1, v2, x);

        points->InsertPoint(j, x[0], 0.0, 0.0);
        totalPoints->InsertPoint(i * numPoints + j, x[0], 0.0, 0.0);
    }

    return;
}

void HighOrderUnstructuredData::computeNodes2d(int i, vtkPoints* totalPoints,
        vtkPoints* points) {
    VsLog::debugLog() << CLASSFUNCLINE << " Entering " << std::endl;
    double* v1 = &vertexData[connectivity[i][0]][0];
    double* v2 = &vertexData[connectivity[i][1]][0];
    double* v3 = &vertexData[connectivity[i][2]][0];
    double* v4 = &vertexData[connectivity[i][3]][0];

    //Now there are N points inside this data, so
    //compute the points

    size_t numPoints = quadPoints.size();

    for (size_t j = 0; j < numPoints; j++) {

        double m = quadPoints[j][0];
        double n = quadPoints[j][1];

        double x[2] = { 0.0, 0.0 };
        globalPos2d(m, n, v1, v2, v3, v4, x);

        points->InsertPoint(j, x[0], x[1], 0.0);
        totalPoints->InsertPoint(i * numPoints + j, x[0], x[1], 0.0);
    }

    return;
}

void HighOrderUnstructuredData::computeNodes3d(int i, vtkPoints* totalPoints,
        vtkPoints* points) {
    VsLog::debugLog() << CLASSFUNCLINE << " Entering " << std::endl;
    double* v1 = &vertexData[connectivity[i][0]][0];
    double* v2 = &vertexData[connectivity[i][1]][0];
    double* v3 = &vertexData[connectivity[i][2]][0];
    double* v4 = &vertexData[connectivity[i][3]][0];
    double* v5 = &vertexData[connectivity[i][4]][0];
    double* v6 = &vertexData[connectivity[i][5]][0];
    double* v7 = &vertexData[connectivity[i][6]][0];
    double* v8 = &vertexData[connectivity[i][7]][0];

    //Now there are N points inside this data, so
    //compute the points

    size_t numPoints = quadPoints.size();

    for (size_t j = 0; j < numPoints; j++) {

        double m = quadPoints[j][0];
        double n = quadPoints[j][1];
        double p = quadPoints[j][2];

        double x[3] = { 0.0, 0.0, 0.0 };
        globalPos3d(m, n, p, v1, v2, v3, v4, v5, v6, v7, v8, x);

        points->InsertPoint(j, x[0], x[1], x[2]);
        totalPoints->InsertPoint(i * numPoints + j, x[0], x[1], x[2]);
    }

    return;
}

void HighOrderUnstructuredData::computeNodes(int i, vtkPoints* totalPoints,
        vtkPoints* points) {
    switch (dimensions) {
    case 1:
        computeNodes1d(i, totalPoints, points);
        break;
    case 2:
        computeNodes2d(i, totalPoints, points);
        break;
    case 3:
        computeNodes3d(i, totalPoints, points);
        break;
    default:
        break;
    }

    return;
}

void HighOrderUnstructuredData::computeTriangulation(vtkPoints* points) {

    VsLog::debugLog() << CLASSFUNCLINE << "  " << "entering" << std::endl;

    vtkSmartPointer < vtkPolyData > profile = vtkPolyData::New();
    profile->SetPoints(points);

    switch (dimensions) {
    case 1:
        //In 1D we don't need to do anything.
        break;
    case 2:
        VsLog::debugLog() << CLASSFUNCLINE << " Setting up 2d " << std::endl;
        delny2d = vtkDelaunay2D::New();
        delny2d->SetTolerance(0.0);
        delny2d->SetInputData(profile);
        delny2d->Update();
        break;
    case 3:
        VsLog::debugLog() << CLASSFUNCLINE << " Setting up 3d " << std::endl;
        delny3d = vtkDelaunay3D::New();
        delny3d->SetTolerance(0.0);
        delny3d->SetInputData(profile);
        delny3d->Update();
        break;
    default:
        break;
    }

    VsLog::debugLog() << CLASSFUNCLINE << " exiting " << std::endl;
    return;
}

void HighOrderUnstructuredData::allocateArray() {
    cellArray = vtkCellArray::New();
}

void HighOrderUnstructuredData::deleteArray() {
    cellArray->Delete();
}

void HighOrderUnstructuredData::constructUnstructuredGrid(vtkPoints* points,
        vtkUnstructuredGrid* &grid) {

    VsLog::debugLog() << CLASSFUNCLINE << "  " << "entering" << std::endl;

    grid = vtkUnstructuredGrid::New();

    grid->SetPoints(points);

    if (dimensions == 2) {
        grid->SetCells(VTK_TRIANGLE, cellArray);
    } else {
        grid->SetCells(VTK_TETRA, cellArray);
    }

    return;
}

void HighOrderUnstructuredData::construct1DGrid(vtkPoints* points,
        vtkStructuredGrid* &grid) {

    VsLog::debugLog() << CLASSFUNCLINE << "  " << "entering" << std::endl;

    grid = vtkStructuredGrid::New();

    grid->SetPoints(points);

    return;
}

void HighOrderUnstructuredData::addToCellArray2D(int count, int numPoints) {

    VsLog::debugLog() << CLASSFUNCLINE << " entering" << std::endl;

    vtkSmartPointer < vtkPolyData > grid = delny2d->GetOutput();

    int offset = count * numPoints;
    for (int i = 0; i < grid->GetNumberOfCells(); i++) {
        vtkSmartPointer < vtkIdList > ptIds = vtkIdList::New();
        vtkSmartPointer < vtkIdList > newIds = vtkIdList::New();
        grid->GetCellPoints(i, ptIds);
        for (int j = 0; j < ptIds->GetNumberOfIds(); j++) {
            int tId = ptIds->GetId(j);
            newIds->InsertNextId(tId + offset);
        }

        cellArray->InsertNextCell(newIds);
    }

    VsLog::debugLog() << CLASSFUNCLINE << " exiting" << std::endl;
}

void HighOrderUnstructuredData::addToCellArray3D(int count, int numPoints) {

    vtkSmartPointer < vtkUnstructuredGrid > grid = delny3d->GetOutput();

    int offset = count * numPoints;
    for (int i = 0; i < grid->GetNumberOfCells(); i++) {
        vtkSmartPointer < vtkIdList > ptIds = vtkIdList::New();
        vtkSmartPointer < vtkIdList > newIds = vtkIdList::New();
        grid->GetCellPoints(i, ptIds);
        for (int j = 0; j < ptIds->GetNumberOfIds(); j++) {
            int tId = ptIds->GetId(j);
            newIds->InsertNextId(tId + offset);
        }

        cellArray->InsertNextCell(newIds);
    }
}

void HighOrderUnstructuredData::addToCellArray(int count, int numPoints) {

    VsLog::debugLog() << CLASSFUNCLINE << " entering with dimension "
            << dimensions << std::endl;

    switch (dimensions) {
    case 1:
        //Yes, there is nothing here
        break;
    case 2:
        addToCellArray2D(count, numPoints);
        break;
    case 3:
        addToCellArray3D(count, numPoints);
        break;
    default:
        break;
    }

    VsLog::debugLog() << CLASSFUNCLINE << " exiting " << std::endl;

}

vtkDataSet* HighOrderUnstructuredData::getMesh(
        VsUnstructuredMesh* unstructuredMesh) {
    VsLog::debugLog() << CLASSFUNCLINE << " Entering " << std::endl;

    allocateArray();
    getData(unstructuredMesh);

    VsLog::debugLog() << CLASSFUNCLINE << " Number of connections "
            << getNumberOfConnections() << std::endl;

    vtkSmartPointer<vtkPoints> totalPoints = vtkPoints::New();

    for (int i = 0; i < (int)getNumberOfConnections(); i++) {
        VsLog::debugLog() << CLASSFUNCLINE << " Adding element " << i
                << std::endl;
        vtkSmartPointer<vtkPoints> points = vtkPoints::New();
        computeNodes(i, totalPoints, points);

        computeTriangulation(points);

        addToCellArray(i, (int)getNumberOfQuadPoints());
        points->Delete();
    }

    if (getNumberOfDimensions() > 1) {
        VsLog::debugLog() << CLASSFUNCLINE << " Constructing 2d or 3d mesh"
                << std::endl;
        vtkUnstructuredGrid* grid;
        constructUnstructuredGrid(totalPoints, grid);
        totalPoints->Delete();
        deleteArray();

        VsLog::debugLog() << CLASSFUNCLINE << "  " << "Exiting" << std::endl;
        return grid;
    } //Otherwise it's 1d

    //In the 1D case use a structured grid.
    vtkStructuredGrid * grid1d;
    construct1DGrid(totalPoints, grid1d);
    VsLog::debugLog() << CLASSFUNCLINE << "  " << "Exiting" << std::endl;
    return grid1d;
}
