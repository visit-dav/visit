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

// ************************************************************************* //
//                         avtMeshLogFilter.C                                //
// ************************************************************************* // 

#include <math.h>

#include <avtMeshLogFilter.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkPoints.h>
#include <vtkPointSet.h>
#include <vtkRectilinearGrid.h>
#include <avtExtents.h>


// ****************************************************************************
//  Method: avtMeshLogFilter::Constructor
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 6, 2007 
//
// ****************************************************************************

avtMeshLogFilter::avtMeshLogFilter()
{
    xScaleMode = LINEAR;
    yScaleMode = LINEAR;
    useInvLogX = false;
    useInvLogY = false;
}

// ****************************************************************************
//  Templated scaling functions.
//
//  Notes:
//     Methods modify the data in place.
//
//  Programmer: Kathleen Biagas
//  Creation:   March 22, 2012 
//
// ****************************************************************************

#define SMALL 1e-100

template <typename T>
void
avtMeshLogFilter_ScaleVal_log(T &v)
{
    v = (T) log10(fabs(v) + SMALL);
}

template <typename T>
void
avtMeshLogFilter_ScaleVal_invlog(T &v)
{
    v = (T) pow(10., (double) v);
}

template <typename T>
void
avtMeshLogFilter_ScaleVal(T &v, bool invLog)
{
    if (invLog)
        avtMeshLogFilter_ScaleVal_invlog<T>(v);
    else
        avtMeshLogFilter_ScaleVal_log<T>(v);
}

template <typename T> 
void 
avtMeshLogFilter_ScaleValues(T *a, vtkIdType n, bool inv, 
                              vtkIdType start, vtkIdType stride)
{
    if (inv)
    {
        for (vtkIdType i = start; i < n; i+=stride)
            avtMeshLogFilter_ScaleVal_invlog<T>(a[i]);
    }
    else 
    {
        for (vtkIdType i = start; i < n; i+=stride)
            avtMeshLogFilter_ScaleVal_log<T>(a[i]);
    }
}


void
avtMeshLogFilter_ScaleValuesHelper(vtkDataArray *a, bool inv, 
                                   int start = 0, int stride = 1)
{
    if (a->GetDataType() == VTK_FLOAT)
    {
        vtkIdType size = a->GetDataSize();
        avtMeshLogFilter_ScaleValues<float>(
            (float*)((vtkFloatArray*)a)->GetVoidPointer(0),
            size, inv, start, stride);
    }
    else if (a->GetDataType() == VTK_DOUBLE)
    {
        vtkIdType size = a->GetDataSize();
        avtMeshLogFilter_ScaleValues<double>(
            (double*)((vtkDoubleArray*)a)->GetVoidPointer(0),
            size, inv, start, stride);
    }
    else // Generic, GetComponent method
    {
        if (inv)
        {
            for (vtkIdType i = 0; i < a->GetNumberOfTuples(); ++i)
            {
                double v = a->GetComponent(i, start);
                avtMeshLogFilter_ScaleVal_invlog(v);
                a->SetComponent(i, start, v);
            }
        }
        else 
        {
            for (vtkIdType i = 0; i < a->GetNumberOfTuples(); ++i)
            {
                double v = a->GetComponent(i, start);
                avtMeshLogFilter_ScaleVal_log(v);
                a->SetComponent(i, start, v);
            }
        }
    }
}


// ****************************************************************************
//  Method: avtMeshLogFilter::Execute
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 6, 2007 
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 23 18:34:01 MST 2012
//    Use new templated methods for scaling. 
//
//    Eric Brugger, Mon Jul 21 14:26:26 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

avtDataRepresentation *
avtMeshLogFilter::ExecuteData(avtDataRepresentation *in_dr)
{
    //
    // Get the VTK data set.
    //
    vtkDataSet *in_ds = in_dr->GetDataVTK();

    if (xScaleMode == LINEAR && yScaleMode == LINEAR)
        return in_dr;

    vtkDataSet *out_ds = in_ds->NewInstance();
    out_ds->ShallowCopy(in_ds);
        
    if (out_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        if (xScaleMode == LOG)
        {
            avtMeshLogFilter_ScaleValuesHelper(
                ((vtkRectilinearGrid*)out_ds)->GetXCoordinates(), useInvLogX);
        }
        if (yScaleMode == LOG)
        {
            avtMeshLogFilter_ScaleValuesHelper(
                ((vtkRectilinearGrid*)out_ds)->GetYCoordinates(), useInvLogY);
        }
    }
    else 
    {
        vtkDataArray *points = ((vtkPointSet*)out_ds)->GetPoints()->GetData();
        if (xScaleMode == LOG)
        {
            avtMeshLogFilter_ScaleValuesHelper(points, useInvLogX, 0, 3); 
        }
        if (yScaleMode == LOG)
        {
            avtMeshLogFilter_ScaleValuesHelper(points, useInvLogY, 1, 3); 
        }
    }

    avtDataRepresentation *out_dr = new avtDataRepresentation(out_ds,
        in_dr->GetDomain(), in_dr->GetLabel());

    out_ds->Delete();

    return out_dr;
}


// ****************************************************************************
//  Method: avtMeshLogFilter::PostExecute
//
//  Purpose:  Ensures correct extents get passed along
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 6, 2007 
//
//  Modifications:
//
//    Hank Childs, Thu Aug 26 13:47:30 PDT 2010
//    Change extents names.
//
//    Kathleen Biagas, Fri Mar 23 18:34:01 MST 2012
//    Use new templated methods for scaling. 
//
// ****************************************************************************

void
avtMeshLogFilter::PostExecute()
{
    avtDataTreeIterator::PostExecute();

    avtDataAttributes& inAtts  = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes& outAtts = GetOutput()->GetInfo().GetAttributes();

    // over-write spatial extents
    outAtts.GetOriginalSpatialExtents()->Clear();
    outAtts.GetThisProcsOriginalSpatialExtents()->Clear();

    // get the outputs's spatial extents
    double se[6];
    if (inAtts.GetOriginalSpatialExtents()->HasExtents())
    {
        inAtts.GetOriginalSpatialExtents()->CopyTo(se);
        if (xScaleMode == LOG)
        {
            avtMeshLogFilter_ScaleVal(se[0], useInvLogX);
            avtMeshLogFilter_ScaleVal(se[1], useInvLogX);
        }
        if (yScaleMode == LOG)
        {
            avtMeshLogFilter_ScaleVal(se[2], useInvLogY);
            avtMeshLogFilter_ScaleVal(se[3], useInvLogY);
        }
        outAtts.GetOriginalSpatialExtents()->Set(se);
    }
    else if (inAtts.GetThisProcsOriginalSpatialExtents()->HasExtents())
    {
        inAtts.GetThisProcsOriginalSpatialExtents()->CopyTo(se);
        if (xScaleMode == LOG)
        {
            avtMeshLogFilter_ScaleVal(se[0], useInvLogX);
            avtMeshLogFilter_ScaleVal(se[1], useInvLogX);
        }
        if (yScaleMode == LOG)
        {
            avtMeshLogFilter_ScaleVal(se[2], useInvLogY);
            avtMeshLogFilter_ScaleVal(se[3], useInvLogY);
        }
        outAtts.GetThisProcsOriginalSpatialExtents()->Set(se);
    }
}


// ****************************************************************************
//  Method: avtMeshLogFilter::UpdateDataObjectInfo
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 6, 2007 
//
// ****************************************************************************

void
avtMeshLogFilter::UpdateDataObjectInfo(void)
{
    avtDataValidity &va = GetOutput()->GetInfo().GetValidity();
    va.InvalidateSpatialMetaData();
    va.SetPointsWereTransformed(true);
}




