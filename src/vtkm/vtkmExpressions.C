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

// If we have not defined the device adaptor, use the serial device.
#ifndef VTKM_DEVICE_ADAPTER
#define VTKM_DEVICE_ADAPTER VTKM_DEVICE_ADAPTER_SERIAL
#endif

#include <iostream>
using std::cout;
using std::endl;

#include <vtkmExpressions.h>
#include <vtkmDataSet.h>
#include <vtkm/worklet/Magnitude.h>

void
vtkmMagnitudeExpression(vtkmDataSet *ds,
    int domain,
    const std::string &activeVar,
    const std::string &outputVar)
{
    if(ds->ds.HasField(activeVar))
    {
        try
        {
            // Get the active field.
            const vtkm::cont::Field &vec = ds->ds.GetField(activeVar);

            // Try and turn the field into a vec3 of floats. (could throw)
            vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float32,3> > vectorData;
            vec.GetData().CopyTo(vectorData);

            // Create the output array handle for the worklet.
            vtkm::cont::ArrayHandle<vtkm::Float32> magnitudes;

            // Invoke the Magnitude worklet to compute the vector magnitude.
            vtkm::worklet::Magnitude magWorklet;
            vtkm::worklet::DispatcherMapField<vtkm::worklet::Magnitude> dispatch(magWorklet);
            dispatch.Invoke(vectorData, magnitudes);

            // Add the new field to the output dataset.
            ds->ds.AddField(
                vtkm::cont::Field(
                    outputVar, vtkm::cont::Field::ASSOC_POINTS, magnitudes));

            ds->ds.PrintSummary(std::cout);
        }
        catch(...)
        {
        }
    }
}
