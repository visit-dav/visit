/*
 * HighOrderUnstructuredData.C
 *
 *   @file        HighOrderUnstructuredData.h
 *
 *   @brief       This class is used for getting data formatted in
 *                the nodal discontinuous Galerkin format into visit.
 *                It triangulates the dg elements individually.  When
 *                data is plotted with contours there may be small
 *                gaps, but this is the most efficient way of solving
 *                this problem (we've found).
 *
 *                As of this writing this dg data format assumes the
 *                DG element uses gauss lobatto type points in the sense
 *                that the edge is included in the quadrature.
 *
 *                It also assumes all elements are either quads or hexes,
 *                we don't currently have support for triangles or tets.
 *
 *                Finally, all elements are assumed to have the same template
 *                so it does not support multiple quadratures for the same grid or
 *                elements with curved edges.
 *
 *
 *   @version $Id: HighOrderUnstructuredData.h 933 2014-02-03 17:28:18Z loverich $
 *
 *  Created on: Jan 18, 2014
 *      Author: loverich
 */

#ifndef HIGH_ORDER_UNSTRUCTURED_DATA_H_
#define HIGH_ORDER_UNSTRUCTURED_DATA_H_

#include <iostream>

#include <vector>

#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtkDelaunay2D.h>
#include <vtkDelaunay3D.h>
#include <vtkUnstructuredGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkDoubleArray.h>

#include <VsReader.h>
#include <VsRegistry.h>
#include <VsDataset.h>
#include <VsVariable.h>
#include <VsUnstructuredMesh.h>
#include <VsAttribute.h>

#include <DebugStream.h>

class HighOrderUnstructuredData
{

public:

    //The constructor!!
    HighOrderUnstructuredData();

    ~HighOrderUnstructuredData();

    /**
     * Fill up the arrays with all the data.
     * @param mesh is the mesh of interest
     */
    void getData(VsUnstructuredMesh* mesh);

    /**
     * Set the reader that will be used
     * @param tReader is the reader used
     */
    void setReader(VsReader* tReader)
    {
        reader = tReader;
    }

    /**
     * Set the registry to use
     * @param tRegistry is the registry to use
     */
    void setRegistry(VsRegistry* tRegistry)
    {
        registry = tRegistry;
    }

    /**
     * Return the unstructured mesh data
     * @param unstructuredmesh is the VsMesh* which is a group
     */
    vtkDataSet* getMesh(VsUnstructuredMesh* unstructuredMesh);

    /**
     * Return a particular variable and component
     * @param meta is the meta information for the variable
     * @param name is the name of the variable (main name q say not q_0)
     * @param component of interest
     */
    vtkDoubleArray* getData(VsVariable* meta, std::string name, int component);

protected:

    /**
     * Compute the nodes for a 2d or 3d element
     * @param i is the index of the element in consideration
     * @param totalPoints is an array where we add the points (nodes) of this
     * element.  By the end it should contain every point in the entire grid
     * @param points contains the points (nodes) just in the local grid
     */
    void computeNodes(int i, vtkPoints* totalPoints, vtkPoints* points);

    /**
     * Given a set of points, determine the delaunay triangulation.  The triangulation
     * is stored in delny2d or deln3d
     */
    void computeTriangulation(vtkPoints* points);

    /**
     * Given points and the cell array construct the unstructured grid
     * @param points are the points in the grid
     * @param grid is the actual grid that is returned.
     */
    void constructUnstructuredGrid(vtkPoints* points,
            vtkUnstructuredGrid* &grid);

    /**
     * Given points and the cell array construct the unstructured grid
     * @param points are the points in the grid
     * @param grid is the actual grid that is returned.
     */
    void construct1DGrid(vtkPoints* points, vtkStructuredGrid* &grid);

    /**
     * Add cells to the total grid.  Count is the index of
     * the cell of interest and num points is the number of
     * points per cell
     */
    void addToCellArray(int count, int numPoints);

    /**
     * Allocate the cell array.  Needs to be called before the
     * grid is constructed.
     */
    void allocateArray();

    /**
     * Delete the cell array.  Needs to be called after all the
     * cells have been added and the grid constructed.  Because of
     * the way the grid is constructed, if it is constructed twice
     * without a delete the grid will contain twice as many elements.
     */
    void deleteArray();

    /**
     * Get the number of cells in the grid
     */
    size_t getNumberOfConnections()
    {
        return connectivity.size();
    }

    /**
     * Get the number of nodes per cell
     */
    size_t getNumberOfQuadPoints()
    {
        return quadPoints.size();
    }

    /**
     * Get the number of dimensions
     */
    size_t getNumberOfDimensions()
    {
        return dimensions;
    }

    /**
     * Get the cell dataset
     * @param mesh is the mesh of interest
     * @return the name where the cell data is stored.
     */
    VsDataset* getCellInfo(VsUnstructuredMesh* unstructuredMesh);

    /**
     * Compute the nodes for a 1d element
     * @param i is the index of the element in consideration
     * @param totalPoints is an array where we add the points (nodes) of this
     * element.  By the end it should contain every point in the entire grid
     * @param points contains the points (nodes) just in the local grid
     */
    void computeNodes1d(int i, vtkPoints* totalPoints, vtkPoints* points);

    /**
     * Compute the nodes for a 2d element
     * @param i is the index of the element in consideration
     * @param totalPoints is an array where we add the points (nodes) of this
     * element.  By the end it should contain every point in the entire grid
     * @param points contains the points (nodes) just in the local grid
     */
    void computeNodes2d(int i, vtkPoints* totalPoints, vtkPoints* points);

    /**
     * Compute the nodes for a 3d element
     * @param i is the index of the element in consideration
     * @param totalPoints is an array where we add the points (nodes) of this
     * element.  By the end it should contain every point in the entire grid
     * @param points contains the points (nodes) just in the local grid
     */
    void computeNodes3d(int i, vtkPoints* totalPoints, vtkPoints* points);

    /*
     * Compute the global position of a node given its template coordinates (n,m,p) in 3d
     * and the 8 vertices of the hex element, v1, v2 ... v8.
     * The vertices are assumed to be in the order (on a unit cube)
     * v1 = (0,0,0), v2=(1,0,0), v3=(1,1,0), v4=(0,1,0)
     * v5 = (0,0,1), v6=(1,0,1), v7=(1,1,1), v8=(0,1,1)
     * @param n the first barycentric coordinate (0 to 1)
     * @param m the second barycentric coordinate (0 to 1)
     * @param p the third barycentric coordinate (0 to 1)
     * @param v1 is a pointer to the first vertex coordinate (x,y,z)
     * @param v2 is a pointer to the second vertex coordinate
     * @param v3 is a pointer to the third vertex coordinate
     * @param v4 is a pointer to the fourth vertex coordinate
     * @param v5 is a pointer to the fifth vertex coordinate
     * @param v6 is a pointer to the sixth vertex coordinate
     * @param v7 is a pointer to the seventh vertex coordinate
     * @param v8 is a pointer to the eighth vertex coordinate
     * @param pos is the resulting position (x,y,z) given the barycentric coordinates (n,m,p)
     */
    void globalPos3d(double n, double m, double p, double *v1, double* v2,
            double* v3, double* v4, double* v5, double* v6, double* v7,
            double* v8, double* pos);

    /*
     * Compute the global position of a node given its template coordinates (n,m) in 2d
     * and the 4 vertices of the quad element, v1, v2 ... v4.
     * The vertices are assumed to be in the order (on a unit square)
     * v1 = (0,0,0), v2=(1,0,0), v3=(1,1,0), v4=(0,1,0)
     * @param n the first barycentric coordinate (0 to 1)
     * @param m the second barycentric coordinate (0 to 1)
     * @param v1 is a pointer to the first vertex coordinate (x,y,z)
     * @param v2 is a pointer to the second vertex coordinate
     * @param v3 is a pointer to the third vertex coordinate
     * @param v4 is a pointer to the fourth vertex coordinate
     * @param pos is the resulting position (x,y) given the barycentric coordinates (n,m)
     */
    void globalPos2d(double n, double m, double* v1, double* v2, double* v3,
            double* v4, double* pos);

    /*
     * Compute the global position of a node given its template coordinates (n) in 1d
     * and the 2 vertices of the line segment, v1, v2.
     * The vertices are assumed to be in the order
     * v1 = (0), v2=(1)
     * @param n the first barycentric coordinate (0 to 1)
     * @param v1 is a pointer to the first vertex coordinate (x,y,z)
     * @param v2 is a pointer to the second vertex coordinate
     * @param pos is the resulting position (x) given the barycentric coordinates (n)
     */
    void globalPos1d(double n, double* v1, double* v2, double* pos);

    /**
     * Add cells to the total grid.  Count is the index of
     * the cell of interest and num points is the number of
     * points per cell
     */
    void addToCellArray2D(int count, int numPoints);

    /**
     * Add cells to the total grid.  Count is the index of
     * the cell of interest and num points is the number of
     * points per cell
     */
    void addToCellArray3D(int count, int numPoints);

private:

    //Compute the number of dimensions in this system
    void setNumDimensions();

    //Convert flattened vertex data to N-D array
    void fillVertexData(std::vector<double>& vertices);

    //Convert flattened connectivity to N-D array
    void fillConnectivity(std::vector<int>& cells);

    //Convert flattened quad points to N-D array
    void fillQuadPoints(std::vector<double>& subCellLocations);

    //Viz Schema reader used
    VsReader* reader;

    //Viz Schema registry used
    VsRegistry* registry;

    //Triangulation for the 2d case
    vtkSmartPointer<vtkDelaunay2D> delny2d;

    //Triangulation for the 3d case
    vtkSmartPointer<vtkDelaunay3D> delny3d;

    //Number of vertices for this element
    int verticesPerCell;

    //Number of nodes in this element.  An element with 5 quadrature
    //points would have 5 nodesPerCell
    int nodesPerCell;

    //The number of cells in the original (un-triangulated grid)
    int numCells;

    //Number of dimensions of the grid
    int dimensions;

    //Array for storing each of the vertices of the original grid
    std::vector<std::vector<double> > vertexData;

    //Array for storing how each cell is composed of vertices
    std::vector<std::vector<int> > connectivity;

    //Array for storing the quadature points inside each element
    std::vector<std::vector<double> > quadPoints;

    //Array for collecting individual cells that are put into the final grid
    vtkSmartPointer<vtkCellArray> cellArray;

    //The number of variables.
    size_t numberOfVariables;

    //The number of meshes
    size_t numberOfMeshes;

};

#endif
