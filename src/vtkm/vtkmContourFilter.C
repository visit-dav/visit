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

#include <vtkmContourFilter.h>

#include <vtkm/worklet/MarchingCubes.h>
#include <vtkm/worklet/DispatcherMapField.h>

typedef VTKM_DEFAULT_DEVICE_ADAPTER_TAG DeviceAdapter;

int
vtkmContourFilter(vtkm::cont::DataSet &input, vtkm::cont::DataSet &output,
    const std::string &contourVar, float isoValue)
{
    //
    // If we don't have any fields return.
    //
    if (input.GetNumberOfFields() <= 0)
    {
        return 0;
    }

    //
    // Run the marching cubes worklet on the input data set.
    //
    typedef vtkm::cont::internal::DeviceAdapterTraits<DeviceAdapter>
                                                        DeviceAdapterTraits;

    vtkm::cont::ArrayHandle<vtkm::Float32> fieldArray;
    input.GetField(contourVar).GetData().CastToArrayHandle(fieldArray);

    vtkm::worklet::MarchingCubes<vtkm::Float32, DeviceAdapter> *isosurfaceFilter;
    vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float32,3> > verticesArray, normalsArray;

    isosurfaceFilter = new vtkm::worklet::MarchingCubes<vtkm::Float32, DeviceAdapter>();

    if (input.GetCellSet().IsType(vtkm::cont::CellSetStructured<3>()))
    {
        typedef vtkm::cont::CellSetStructured<3> CellSet;
        isosurfaceFilter->Run(isoValue,
                              input.GetCellSet().CastTo(CellSet()),
                              input.GetCoordinateSystem(),
                              fieldArray,
                              verticesArray,
                              normalsArray);
    }
    else
    {
        typedef vtkm::cont::CellSetExplicit<> CellSet;
        isosurfaceFilter->Run(isoValue,
                              input.GetCellSet().CastTo(CellSet()),
                              input.GetCoordinateSystem(),
                              fieldArray,
                              verticesArray,
                              normalsArray);
    }

    int nScalars = input.GetNumberOfFields();
    vtkm::cont::ArrayHandle<vtkm::Float32> *scalarArrays = 
        scalarArrays = new vtkm::cont::ArrayHandle<vtkm::Float32>[nScalars];

    for (int i = 0; i < nScalars; ++i)
    {
        vtkm::cont::ArrayHandle<vtkm::Float32> field2Array;
        input.GetField(i).GetData().CastToArrayHandle(field2Array);

        isosurfaceFilter->MapFieldOntoIsosurface(field2Array,
            scalarArrays[i]);
    }

    //
    // Create the output data set.
    //
    output.AddCoordinateSystem(
        vtkm::cont::CoordinateSystem("coordinates", 1, verticesArray));

    vtkm::Id numCells = verticesArray.GetNumberOfValues() / 3;

    vtkm::cont::ArrayHandle<vtkm::Id> connectivity;
    connectivity.Allocate(numCells*3);

    vtkm::cont::ArrayHandle<vtkm::Id>::PortalControl connectivityPortal =
      connectivity.GetPortalControl();
    for (vtkm::Id i = 0; i < numCells*3; ++i)
    {
        connectivityPortal.Set(i, i);
    }

    vtkm::cont::CellSetSingleType<> cs;

    typedef vtkm::CellShapeIdToTag<vtkm::CELL_SHAPE_TRIANGLE>::Tag CellShapeTag;
    cs = vtkm::cont::CellSetSingleType<>(CellShapeTag(), "cells");

    cs.Fill(connectivity);
    output.AddCellSet(cs);

    for (int i = 0; i < nScalars; ++i)
    {
        output.AddField(
            vtkm::cont::Field(input.GetField(i).GetName(), 1,
            vtkm::cont::Field::ASSOC_POINTS, scalarArrays[i]));
    }

    return 0;
}
