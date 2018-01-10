/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>

#include <TimingsManager.h>

#include <set>

typedef VTKM_DEFAULT_DEVICE_ADAPTER_TAG DeviceAdapter;

vtkmDataSet::vtkmDataSet()
{
}

vtkmDataSet::~vtkmDataSet()
{
}

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

        ret->ds.AddCoordinateSystem(
            vtkm::cont::CoordinateSystem("coordinates", vdims, origin, spacing));
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
            vtkm::cont::CoordinateSystem("coordinates", coordinates));
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

        vtkm::cont::CellSetExplicit<> cs("cells");
        cs.Fill(nCellsActual, shapes, nIndices, connectivity);
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
                vtkm::cont::CoordinateSystem("coordinates", coordinates));
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
                vtkm::cont::CoordinateSystem("coordinates", coordinates));
        }
    }

    //
    // Add the fields.
    //
    
    int nArrays = data->GetPointData()->GetNumberOfArrays();
    for (int i = 0; i < nArrays; ++i)
    {
        vtkDataArray *array = data->GetPointData()->GetArray(i);

        if (array->GetNumberOfComponents() == 1)
        {
            if(array->GetDataType() == VTK_FLOAT)
            {
                //cout << "ConvertVTKToVTKm: adding float point data field " << array->GetName() << endl;
                vtkIdType nVals = array->GetNumberOfTuples();
                float *vals =
                    vtkFloatArray::SafeDownCast(array)->GetPointer(0);

                vtkm::cont::ArrayHandle<vtkm::Float32> fieldArray;
                fieldArray.Allocate(nVals);

                for (vtkm::Id j = 0; j < nVals; ++j)
                    fieldArray.GetPortalControl().Set(j, vals[j]);

                ret->ds.AddField(
                    vtkm::cont::Field(array->GetName(),
                    vtkm::cont::Field::ASSOC_POINTS, fieldArray));
            }
            if(array->GetDataType() == VTK_DOUBLE)
            {
                //cout << "ConvertVTKToVTKm: adding double point data field " << array->GetName() << endl;
                vtkIdType nVals = array->GetNumberOfTuples();
                double *vals =
                    vtkDoubleArray::SafeDownCast(array)->GetPointer(0);

                vtkm::cont::ArrayHandle<vtkm::Float64> fieldArray;
                fieldArray.Allocate(nVals);
  
                for (vtkm::Id j = 0; j < nVals; ++j)
                    fieldArray.GetPortalControl().Set(j, vals[j]);

                ret->ds.AddField(
                    vtkm::cont::Field(array->GetName(),
                    vtkm::cont::Field::ASSOC_POINTS, fieldArray));
            }
        }
        else if (array->GetNumberOfComponents() == 3)
        {
            if(array->GetDataType() == VTK_FLOAT)
            {
                //cout << "ConvertVTKToVTKm: adding float point data field " << array->GetName() << endl;
                vtkIdType nVals = array->GetNumberOfTuples();
                vtkm::Vec<vtkm::Float32,3> *vals = 
                    reinterpret_cast<vtkm::Vec<vtkm::Float32,3> *>(array->GetVoidPointer(0));

                // Wrap the vector data as an array handle.
                // This is good as long as the VTK object is around. Is it safe?
                vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float32,3> > fieldArray = 
                    vtkm::cont::make_ArrayHandle(vals, nVals);

                ret->ds.AddField(
                    vtkm::cont::Field(array->GetName(), 
                    vtkm::cont::Field::ASSOC_POINTS, fieldArray));
            }
        }
    }

    return ret;
}

// ****************************************************************************
// Method: vtkPointsFromVTKM
//
// Purpose:
//   Makes a vtkPoints object from the coordinates in the VTKm dataset.
//
// Arguments:
//   data : The VTKm dataset. 
//
// Returns:    A vtkPoints object if one can be created.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar  9 13:20:01 PST 2017
//
// Modifications:
//
// ****************************************************************************

static vtkPoints *
vtkPointsFromVTKM(vtkmDataSet *data)
{
    vtkPoints *pts = NULL;
    try
    {
        //
        // Get the coordinates.
        //
        vtkm::cont::CoordinateSystem coords = data->ds.GetCoordinateSystem(0);
        vtkm::cont::DynamicArrayHandleCoordinateSystem cdata = coords.GetData();

        vtkm::Id nPoints = cdata.GetNumberOfValues();

        // This might throw
        vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float32, 3> > cdata2;
        coords.GetData().CopyTo(cdata2);

        //
        // Create the vtk points
        //
        pts = vtkPoints::New();
        pts->SetNumberOfPoints(nPoints);
        for (vtkm::Id i = 0; i < nPoints; ++i)
        {
            vtkm::Vec<vtkm::Float32, 3> point = cdata2.GetPortalConstControl().Get(i);
            pts->SetPoint((vtkIdType)i, point[0], point[1], point[2]);
        }
    }
    catch(...)
    {
    }

    return pts;
}

// ****************************************************************************
// Method: StructuredFromVTKM
//
// Purpose:
//   Create a rectilinear or structured grid from the VTKm dataset.
//
// Arguments:
//   data : The VTKm dataset.
//
// Returns:    A VTK version of the VTKm dataset.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar  9 13:19:08 PST 2017
//
// Modifications:
//
// ****************************************************************************

static vtkDataSet *
StructuredFromVTKM(vtkmDataSet *data)
{
    vtkDataSet *ds = NULL;

    try
    {
        // Try and make a structured cell set from the VTKm dataset.
        vtkm::cont::CellSetStructured<3> cs;
        data->ds.GetCellSet().CopyTo(cs);

        // Determine the size of the cell set.
        const vtkm::Id3 vdims = cs.GetPointDimensions();
        int dims[3];
        dims[0] = vdims[0];
        dims[1] = vdims[1];
        dims[2] = vdims[2];

        typedef vtkm::cont::ArrayHandle <vtkm::Float32 > AxisArrayType;
        typedef vtkm::cont::ArrayHandleCartesianProduct <AxisArrayType ,AxisArrayType ,AxisArrayType > CartesianProduct;

        // See if we have a uniform coordinate system.
        if(data->ds.GetCoordinateSystem().GetData().IsSameType(
           vtkm::cont::ArrayHandleUniformPointCoordinates()
           ))
        {
            vtkm::cont::ArrayHandleUniformPointCoordinates pc;
            data->ds.GetCoordinateSystem().GetData().CopyTo(pc);

#if 1
            // I can't figure out how to get the origin+spacing from the pc object.
            // Iterating over all of its elements gives all of the rectlinear coordinates
            // in the usual order. With that assumption, we can read along I, J, and K
            // to get the coordinate values we want. This is far faster than iterating
            // through all coordinates.
            std::set<float> X,Y,Z;
            for(int i = 0; i < dims[0]; ++i)
            {
                auto pt = pc.GetStorage().GetPortalConst().Get(i);
                X.insert(pt[0]);
            }
            for(int j = 0; j < dims[1]; ++j)
            {
                auto pt = pc.GetStorage().GetPortalConst().Get(j*dims[0]);
                Y.insert(pt[1]);
            }
            for(int k = 0; k < dims[2]; ++k)
            {
                auto pt = pc.GetStorage().GetPortalConst().Get(k*dims[0]*dims[1]);
                Z.insert(pt[2]);
            }

            if(dims[0] != X.size())
                cout << "Error in X dimension size!" << endl;
            if(dims[1] != Y.size())
                cout << "Error in Y dimension size!" << endl;
            if(dims[2] != Z.size())
                cout << "Error in Z dimension size!" << endl;

            int i;
            vtkFloatArray *axes[3] = {NULL,NULL,NULL};
            axes[0] = vtkFloatArray::New();
            axes[0]->SetNumberOfTuples(dims[0]);
            i = 0;
            for(std::set<float>::const_iterator it = X.begin(); it != X.end(); ++it, ++i)
                axes[0]->SetTuple1(i, *it);

            axes[1] = vtkFloatArray::New();
            axes[1]->SetNumberOfTuples(dims[1]);
            i = 0;
            for(std::set<float>::const_iterator it = Y.begin(); it != Y.end(); ++it, ++i)
                axes[1]->SetTuple1(i, *it);

            axes[2] = vtkFloatArray::New();
            axes[2]->SetNumberOfTuples(dims[2]);
            i = 0;
            for(std::set<float>::const_iterator it = Z.begin(); it != Z.end(); ++it, ++i)
                axes[2]->SetTuple1(i, *it);

#else
// How to get to the origin and spacing in the ArrayHandleUniformPointCoordinates?
            // Create axes.
            vtkFloatArray *axes[3] = {NULL,NULL,NULL};
            for(int i = 0; i < 3; ++i)
            {
                axes[i] = vtkFloatArray::New();
                axes[i]->SetNumberOfTuples(dims[i]);
                for(int j = 0; j < dims[i]; ++j)
                {
                    axes[i]->SetTuple1(j, pc.GetStorage().GetOrigin()[i] + // not working!
                                          double(j) * pc.GetStorage().GetSpacing()[i]);
                }
            }
#endif

            // Make rectilinear dataset.
            vtkRectilinearGrid *rgrid  = vtkRectilinearGrid::New();
            rgrid->SetDimensions(dims);
            rgrid->SetXCoordinates(axes[0]);
            axes[0]->Delete();
            rgrid->SetYCoordinates(axes[1]);
            axes[1]->Delete();
            rgrid->SetZCoordinates(axes[2]);
            axes[2]->Delete();
            ds = rgrid;
        }
#if 0
        // See if we have a cartesian product coordinate system.
        else if(data->ds.GetCoordinateSystem().GetData().IsSameType(
                CartesianProduct
                ))
        {
            // TODO: We have independent axes we can turn back into rgrid.
        }
#endif
        else
        {
            // See if we need to make a structured grid.
            vtkPoints *pts = vtkPointsFromVTKM(data);
            if(pts != NULL)
            {
                vtkStructuredGrid *sgrid = vtkStructuredGrid::New();
                sgrid->SetDimensions(dims);
                sgrid->SetPoints(pts);
                pts->Delete();
                ds = sgrid;
            }
        }
    }
    catch(...)
    {
    }

    return ds;
}

vtkDataSet *
ConvertVTKmToVTK(vtkmDataSet *data)
{
    vtkDataSet *ret = NULL;
 
    // Use the dataset's cell set to turn the connectivity back into VTK.
    int t0 = visitTimer->StartTimer();
    vtkDataSet *ds = NULL;
    if(data->ds.GetNumberOfCellSets() > 0)
    {
        if(data->ds.GetCellSet().IsSameType(vtkm::cont::CellSetStructured<3>()))
        {
            ds = StructuredFromVTKM(data);
        }
        else if(data->ds.GetCellSet().IsSameType(vtkm::cont::CellSetExplicit<>()))
        {
            vtkPoints *pts = vtkPointsFromVTKM(data);
            vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
            ugrid->SetPoints(pts);
            pts->Delete();
            ds = ugrid;

            cout << "ConvertVTKmToVTK: Casting cell set to CellSetExplicit." << endl;
            vtkm::cont::DynamicCellSet cs = data->ds.GetCellSet();
            vtkm::cont::CellSetExplicit<> cse;
            try
            {
                cs.CopyTo(cse);
                cout << "ConvertVTKmToVTK: Adding cells to ugrid." << endl;
                for(vtkm::Id cellid = 0; cellid < cse.GetNumberOfCells(); ++cellid)
                {
                    vtkm::Id npts = cse.GetNumberOfPointsInCell(cellid);
                    vtkm::UInt8 cellShape = cse.GetCellShape(cellid);
                    if(cellShape == vtkm::CELL_SHAPE_TRIANGLE)
                    {
                        vtkm::Vec<vtkm::Id,3> ids;
                        cse.GetIndices(cellid, ids);
                        vtkIdType vids[3];
                        vids[0] = static_cast<vtkIdType>(ids[0]);
                        vids[1] = static_cast<vtkIdType>(ids[1]);
                        vids[2] = static_cast<vtkIdType>(ids[2]);
                        ugrid->InsertNextCell(VTK_TRIANGLE, 3, vids);
                        //cout << "added tri: " << vids[0] << "," << vids[1] << "," << vids[2] << endl;
                    }
                    else if(cellShape == vtkm::CELL_SHAPE_QUAD)
                    {
                        vtkm::Vec<vtkm::Id,4> ids;
                        cse.GetIndices(cellid, ids);
                        vtkIdType vids[4];
                        vids[0] = static_cast<vtkIdType>(ids[0]);
                        vids[1] = static_cast<vtkIdType>(ids[1]);
                        vids[2] = static_cast<vtkIdType>(ids[2]);
                        vids[3] = static_cast<vtkIdType>(ids[3]);
                        ugrid->InsertNextCell(VTK_QUAD, 4, vids);
                        //cout << "added quad: " << vids[0] << "," << vids[1] << "," << vids[2] << "," << vids[3] << endl;
                    }
                    else
                    {
                        cout << "cell type: " << (int)cellShape << endl;
                    }
                }
            }
            catch(vtkm::cont::ErrorBadType)
            {
                cout << "Caught bad type from cse cast." << endl;
            }
        }
        else if(data->ds.GetCellSet().IsSameType(vtkm::cont::CellSetSingleType<>()))
        {
            //cout << "ConvertVTKmToVTK: Casting cell set to CellSetSingleType." << endl;
            vtkm::cont::DynamicCellSet cs = data->ds.GetCellSet();
            vtkm::cont::CellSetSingleType<> csst;
            try
            {
                cs.CopyTo(csst);
                //cout << "ConvertVTKmToVTK: Adding cells to ugrid." << endl;
                if(csst.GetCellShape(0) == vtkm::CELL_SHAPE_TRIANGLE)
                {
                    vtkPoints *pts = vtkPointsFromVTKM(data);
                    vtkPolyData *pd = vtkPolyData::New();
                    pd->SetPoints(pts);
                    pts->Delete();
                    pd->Allocate(csst.GetNumberOfCells());
                    ds = pd;

                    vtkm::Vec<vtkm::Id,3> ids;
                    vtkIdType vids[3];
                    for(vtkm::Id cellid = 0; cellid < csst.GetNumberOfCells(); ++cellid)
                    {
                        csst.GetIndices(cellid, ids);

                        vids[0] = static_cast<vtkIdType>(ids[0]);
                        vids[1] = static_cast<vtkIdType>(ids[1]);
                        vids[2] = static_cast<vtkIdType>(ids[2]);
                        pd->InsertNextCell(VTK_TRIANGLE, 3, vids);
                        //cout << "added tri: " << vids[0] << "," << vids[1] << "," << vids[2] << endl;
                    }
                }
                else
                {
                    cout << "Cell set contains " << (int)csst.GetCellShape(0) << endl;
                }
            }
            catch(vtkm::cont::ErrorBadType)
            {
                cout << "Caught bad type from csst cast." << endl;
            }
        }
    }
    visitTimer->StopTimer(t0, "Creating VTK mesh from VTKm");

    //
    // Add the fields.
    //
    if(ds != NULL)
    {
        int t1 = visitTimer->StartTimer();
        int nFields = data->ds.GetNumberOfFields();
        for (int i = 0; i < nFields; ++i)
        {
            const char *fieldName = data->ds.GetField(i).GetName().c_str();

            // Use the field's association to try and attach the data to the right
            // place in the VTK dataset.
            vtkDataSetAttributes *attr = ds->GetPointData();
            if(data->ds.GetField(i).GetAssociation() == vtkm::cont::Field::ASSOC_CELL_SET)
                attr = ds->GetCellData();

            // Try and access the field as a float32 array handle. (might throw)
            try //if(data->ds.GetField(i).GetData().IsSameType(vtkm::cont::ArrayHandle<vtkm::Float32>()))
            {
                vtkm::cont::ArrayHandle<vtkm::Float32> fieldArray;
                data->ds.GetField(i).GetData().CopyTo(fieldArray);

                cout << "Converting VTKm scalar field " << fieldName << " to VTK" << endl;
                vtkm::Id nValues = fieldArray.GetNumberOfValues();

                vtkFloatArray *outArray = vtkFloatArray::New();
                outArray->SetName(fieldName);
                outArray->SetNumberOfTuples(nValues);

#if 1
                // Hm. This assumes that the array handle contains a bunch of floats.
                // Array handles can be tricky and not be backed by real memory.
                // How can we detect that?
                memcpy(outArray->GetVoidPointer(0), fieldArray.GetStorage().GetArray(), sizeof(float)*nValues);
#else
                for (vtkm::Id j = 0; j < nValues; ++j)
                {
                    float val = fieldArray.GetPortalConstControl().Get(j);
                    outArray->SetTuple1(j, val);
                }
#endif
                attr->AddArray(outArray);
                attr->SetActiveScalars(fieldName);
                attr->CopyFieldOn(fieldName);
                outArray->Delete();
                continue;
            }
            catch(...)
            {
                cout << fieldName << " is not a float scalar." << endl;
            }

            try //else if(data->ds.GetField(i).GetData().IsSameType(vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float32,3> >()))
            {
                vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float32, 3> > fieldArray;
                data->ds.GetField(i).GetData().CopyTo(fieldArray);

                cout << "Converting VTKm vector field " << fieldName << " to VTK" << endl;
                vtkm::Id nValues = fieldArray.GetNumberOfValues();

                vtkFloatArray *outArray = vtkFloatArray::New();
                outArray->SetName(fieldName);
                outArray->SetNumberOfComponents(3);
                outArray->SetNumberOfTuples(nValues);

#if 1
                // Hm. This assumes that the array handle contains a bunch of floats.
                // Array handles can be tricky and not be backed by real memory.
                // How can we detect that?
                memcpy(outArray->GetVoidPointer(0), fieldArray.GetStorage().GetArray(), 3*sizeof(float)*nValues);
#else
                for (vtkm::Id j = 0; j < nValues; ++j)
                {
                    vtkm::Vec<vtkm::Float32,3> val = fieldArray.GetPortalConstControl().Get(j);
                    outArray->SetTuple3(j, val[0], val[1], val[2]);
                }
#endif
                attr->AddArray(outArray);
                attr->SetActiveScalars(fieldName);
                attr->CopyFieldOn(fieldName);
                outArray->Delete();
                continue;
            }
            catch(...)
            {
                cout << fieldName << " is not a float vector." << endl;
            }
        }
        visitTimer->StopTimer(t1, "Converting fields from VTKm to VTK.");
    }

    ret = ds;

    return ret;
}
