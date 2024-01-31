// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    Kathleen Biagas, Tues Oct 13, 2020
//    Set the Modified flag on Points and Coordinates, to ensure downstream
//    filters know they have been updated.
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
            ((vtkRectilinearGrid*)out_ds)->GetXCoordinates()->Modified();
        }
        if (yScaleMode == LOG)
        {
            avtMeshLogFilter_ScaleValuesHelper(
                ((vtkRectilinearGrid*)out_ds)->GetYCoordinates(), useInvLogY);
            ((vtkRectilinearGrid*)out_ds)->GetYCoordinates()->Modified();
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
        points->Modified();
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




