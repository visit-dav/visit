/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

//
// We first check if VTKM_DEVICE_ADAPTER is defined, so that when TBB and CUDA
// includes this file we use the device adapter that they have set.
//
#ifndef VTKM_DEVICE_ADAPTER
#define VTKM_DEVICE_ADAPTER VTKM_DEVICE_ADAPTER_SERIAL
#endif

#include <vtkmDataSet.h>

#include <vtkCellArray.h>
#include <vtkCellType.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>

typedef VTKM_DEFAULT_DEVICE_ADAPTER_TAG DeviceAdapter;

vtkmDataSet *
ConvertVTKToVTKm(vtkDataSet *data)
{
    vtkmDataSet *ret = NULL;

    if (data->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        //
        // Get VTK data.
        //
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) data;
        vtkDataArray *xCoords = rgrid->GetXCoordinates();
        vtkDataArray *yCoords = rgrid->GetYCoordinates();
        vtkDataArray *zCoords = rgrid->GetZCoordinates();
        float *xPts = static_cast<float*>(xCoords->GetVoidPointer(0));
        float *yPts = static_cast<float*>(yCoords->GetVoidPointer(0));
        float *zPts = static_cast<float*>(zCoords->GetVoidPointer(0));

        int dims[3];
        rgrid->GetDimensions(dims);

        //
        // Create the VTKm data set.
        //
        ret = new vtkmDataSet;

        // Add the structured cell set.
        const vtkm::Id3 vdims(dims[0], dims[1], dims[2]);

        vtkm::cont::CellSetStructured<3> cs("cells");
        cs.SetPointDimensions(vdims);
        ret->ds.AddCellSet(cs);

        // Add the coordinate system.
        vtkm::Vec<vtkm::Float32,3> origin(xPts[0], yPts[0], zPts[0]);
        vtkm::Vec<vtkm::Float32,3> spacing(
            static_cast<vtkm::Float32>(xPts[1] - xPts[0]),
            static_cast<vtkm::Float32>(yPts[1] - yPts[0]),
            static_cast<vtkm::Float32>(zPts[1] - zPts[0]));

        vtkm::cont::ArrayHandleUniformPointCoordinates
            coordinates(vdims, origin, spacing);
        ret->ds.AddCoordinateSystem(
            vtkm::cont::CoordinateSystem("coordinates", 1, coordinates));
    }
    else if (data->GetDataObjectType() == VTK_STRUCTURED_GRID)
    {
        //
        // Get VTK data.
        //
        vtkStructuredGrid *sgrid = (vtkStructuredGrid *) data;
        vtkIdType nPoints = sgrid->GetNumberOfPoints();
        vtkPoints *points = sgrid->GetPoints();
        float *pts = static_cast<float*>(points->GetVoidPointer(0));

        int dims[3];
        sgrid->GetDimensions(dims);

        //
        // Create the VTKm data set.
        //
        ret = new vtkmDataSet;

        // Add the structured cell set.
        const vtkm::Id3 vdims(dims[0], dims[1], dims[2]);

        vtkm::cont::CellSetStructured<3> cs("cells");
        cs.SetPointDimensions(vdims);
        ret->ds.AddCellSet(cs);

        // Add the coordinate system.
        vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float32,3> > coordinates;
        coordinates.Allocate(nPoints);

        for (vtkm::Id i = 0; i < nPoints; ++i)
        {
            vtkm::Vec<vtkm::Float32,3> point(pts[i*3], pts[i*3+1], pts[i*3+2]);
            coordinates.GetPortalControl().Set(i, point);
        }

        ret->ds.AddCoordinateSystem(
            vtkm::cont::CoordinateSystem("coordinates", 1, coordinates));
    }
    else if (data->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {
        //
        // Get VTK data.
        //
        vtkUnstructuredGrid *ugrid = (vtkUnstructuredGrid *) data;
        vtkIdType nPoints = ugrid->GetNumberOfPoints();
        vtkPoints *points = ugrid->GetPoints();

        vtkUnsignedCharArray *cellTypes = ugrid->GetCellTypesArray();
        vtkIdTypeArray *cellLocations = ugrid->GetCellLocationsArray();
        vtkCellArray *cells = ugrid->GetCells();

        vtkIdType nCells = cells->GetNumberOfCells();
        vtkIdType nConnectivity = cells->GetNumberOfConnectivityEntries();
        vtkIdType *nl = cells->GetPointer();
        unsigned char *ct = cellTypes->GetPointer(0);

        //
        // Create the VTKm data set.
        //
        ret = new vtkmDataSet;

        // Add the unstructured cell set.
        vtkm::cont::ArrayHandle<vtkm::Id> connectivity;
        vtkm::cont::ArrayHandle<vtkm::IdComponent> nIndices;
        vtkm::cont::ArrayHandle<vtkm::UInt8> shapes; 

        connectivity.Allocate(nConnectivity - nCells);
        nIndices.Allocate(nCells);
        shapes.Allocate(nCells);

        vtkm::cont::ArrayHandle<vtkm::Id>::PortalControl
            connectivityPortal = connectivity.GetPortalControl();
        vtkm::cont::ArrayHandle<vtkm::IdComponent>::PortalControl
            nIndicesPortal = nIndices.GetPortalControl();
        vtkm::cont::ArrayHandle<vtkm::UInt8>::PortalControl shapesPortal =
            shapes.GetPortalControl();
        vtkm::Id nCellsActual = 0, connInd = 0;
        for (vtkm::Id i = 0; i < nCells; ++i)
        {
            vtkm::IdComponent nInds = static_cast<vtkm::IdComponent>(*nl++);
            switch (*ct)
            {
#if 0
              case vtkm::CELL_SHAPE_VERTEX:
              case vtkm::CELL_SHAPE_LINE:
              case vtkm::CELL_SHAPE_TRIANGLE:
              case vtkm::CELL_SHAPE_QUAD:
              case vtkm::CELL_SHAPE_TETRA:
#endif
              case vtkm::CELL_SHAPE_HEXAHEDRON:
#if 0
              case vtkm::CELL_SHAPE_WEDGE:
              case vtkm::CELL_SHAPE_PYRAMID:
#endif
                nIndicesPortal.Set(nCellsActual, nInds);
                for (vtkm::IdComponent j = 0; j < nInds; ++j, ++connInd)
                {
                    connectivityPortal.Set(connInd, static_cast<vtkm::Id>(*nl++));
                }
                shapesPortal.Set(nCellsActual, static_cast<vtkm::UInt8>(*ct++));
                nCellsActual++;
                break;
              default:
                // Unsupported type, skipping.
                ct++;
                nl += static_cast<vtkIdType>(nInds);
                break;
            }
        }

        // If we skipped any cells adjust the cell set array lengths.
        if (nCellsActual < nCells)
        {
            connectivity.Shrink(connInd);
            nIndices.Shrink(nCellsActual);
            shapes.Shrink(nCellsActual);
        }

        vtkm::cont::CellSetExplicit<> cs(0, "cells");
        cs.Fill(shapes, nIndices, connectivity);
        ret->ds.AddCellSet(cs);

        // Add the coordinate system.
        if (points->GetDataType() == VTK_FLOAT)
        {
            float *pts = static_cast<float*>(points->GetVoidPointer(0));
            vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float32,3> > coordinates;
            coordinates.Allocate(nPoints);

            for (vtkm::Id i = 0; i < nPoints; ++i)
            {
                vtkm::Vec<vtkm::Float32,3> point(pts[i*3], pts[i*3+1], pts[i*3+2]);
                coordinates.GetPortalControl().Set(i, point);
            }

            ret->ds.AddCoordinateSystem(
                vtkm::cont::CoordinateSystem("coordinates", 1, coordinates));
        }
        else
        {
            double *pts = static_cast<double*>(points->GetVoidPointer(0));
            vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float64,3> > coordinates;
            coordinates.Allocate(nPoints);

            for (vtkm::Id i = 0; i < nPoints; ++i)
            {
                vtkm::Vec<vtkm::Float64,3> point(pts[i*3], pts[i*3+1], pts[i*3+2]);
                coordinates.GetPortalControl().Set(i, point);
            }

            ret->ds.AddCoordinateSystem(
                vtkm::cont::CoordinateSystem("coordinates", 1, coordinates));
        }
    }

    //
    // Add the fields.
    //
    int nArrays = data->GetPointData()->GetNumberOfArrays();
    for (int i = 0; i < nArrays; ++i)
    {
        vtkDataArray *array = data->GetPointData()->GetArray(i);

        if (array->GetNumberOfComponents() == 1 &&
            array->GetDataType() == VTK_FLOAT)
        {
            vtkIdType nVals = array->GetNumberOfTuples();
            float *vals =
                vtkFloatArray::SafeDownCast(array)->GetPointer(0);

            vtkm::cont::ArrayHandle<vtkm::Float32> fieldArray;
            fieldArray.Allocate(nVals);

            for (vtkm::Id j = 0; j < nVals; ++j)
                fieldArray.GetPortalControl().Set(j, vals[j]);

            ret->ds.AddField(
                vtkm::cont::Field(array->GetName(), 1,
                vtkm::cont::Field::ASSOC_POINTS, fieldArray));
        }
        else if (array->GetNumberOfComponents() == 1 &&
            array->GetDataType() == VTK_DOUBLE)
        {
            vtkIdType nVals = array->GetNumberOfTuples();
            double *vals =
                vtkDoubleArray::SafeDownCast(array)->GetPointer(0);

            vtkm::cont::ArrayHandle<vtkm::Float64> fieldArray;
            fieldArray.Allocate(nVals);

            for (vtkm::Id j = 0; j < nVals; ++j)
                fieldArray.GetPortalControl().Set(j, vals[j]);

            ret->ds.AddField(
                vtkm::cont::Field(array->GetName(), 1,
                vtkm::cont::Field::ASSOC_POINTS, fieldArray));
        }
    }

    return ret;
}

vtkDataSet *
ConvertVTKmToVTK(vtkmDataSet *data)
{
    vtkDataSet *ret = NULL;

    //
    // Get the coordinates.
    //
    vtkm::cont::CoordinateSystem coords = data->ds.GetCoordinateSystem(0);
    vtkm::cont::DynamicArrayHandleCoordinateSystem cdata = coords.GetData();

    vtkm::Id nPoints = cdata.GetNumberOfValues();

    vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float32,3> > cdata2;
    cdata.CastToArrayHandle(cdata2);

    //
    // Create the vtk data set.
    //
    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(nPoints);
    for (vtkm::Id i = 0; i < nPoints; ++i)
    {
        vtkm::Vec<vtkm::Float32, 3> point = cdata2.GetPortalConstControl().Get(i);
        pts->SetPoint((vtkIdType)i, point[0], point[1], point[2]);
    }

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(pts);
    pts->Delete();

    vtkIdType nCells = nPoints / 3;

    ugrid->Allocate(nCells);
    for (vtkIdType i = 0; i < nCells; ++i)
    {
         vtkIdType ids[3];
         ids[0] = i * 3; ids[1] = i * 3 + 1; ids[2] = i * 3 + 2;
         ugrid->InsertNextCell(VTK_TRIANGLE, 3, ids);
    }

    //
    // Add the fields.
    //
    int nFields = data->ds.GetNumberOfFields();
    for (int i = 0; i < nFields; ++i)
    {
        vtkm::cont::ArrayHandle<vtkm::Float32> fieldArray;
        data->ds.GetField(i).GetData().CastToArrayHandle(fieldArray);
        vtkm::Id nValues = fieldArray.GetNumberOfValues();
        const char *fieldName = data->ds.GetField(i).GetName().c_str();

        vtkFloatArray *outArray = vtkFloatArray::New();
        outArray->SetName(fieldName);
        outArray->SetNumberOfTuples(nValues);

        for (vtkm::Id j = 0; j < nValues; ++j)
        {
            float val = fieldArray.GetPortalConstControl().Get(j);
            outArray->SetTuple1(j, val);
        }

        ugrid->GetPointData()->AddArray(outArray);
        ugrid->GetPointData()->SetActiveScalars(fieldName);
        ugrid->GetPointData()->CopyFieldOn(fieldName);
        outArray->Delete();
    }

    ret = ugrid;

    return ret;
}
