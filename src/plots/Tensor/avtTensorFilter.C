// ************************************************************************* //
//                             avtTensorFilter.C                             //
// ************************************************************************* //

#include <avtTensorFilter.h>

#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkTensorReduceFilter.h>
#include <vtkVertexFilter.h>

// ****************************************************************************
//  Method: avtTensorFilter constructor
//
//  Arguments:
//      us      A boolean indicating if you should use the stride (true) or
//              stick with a fixed number of tensors (false).
//      red     The reduction factor, the stride of number of tensors depending
//              on us.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
// ****************************************************************************

avtTensorFilter::avtTensorFilter(bool us, int red)
{
    reduce = vtkTensorReduceFilter::New();
    vertex = vtkVertexFilter::New();

    if (us)
    {
        SetStride(red);
    }
    else
    {
        SetNTensors(red);
    }

    if (useStride)
    {
        reduce->SetStride(stride);
    }
    else
    {
        reduce->SetNumberOfElements(nTensors);
    }
}


// ****************************************************************************
//  Method: avtTensorFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Aug 30 17:30:48 PDT 2001
//    Added vertex filter.
//
// ****************************************************************************

avtTensorFilter::~avtTensorFilter()
{
    if (reduce != NULL)
    {
        reduce->Delete();
    }
    if (vertex != NULL)
    {
        vertex->Delete();
    }
}


// ****************************************************************************
//  Method: avtTensorFilter::SetStride
//
//  Purpose:
//      Sets the stride of reduction for the vector.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2001
//
// ****************************************************************************

void
avtTensorFilter::SetStride(int s)
{
    useStride = true;
    stride    = s;
    nTensors  = -1;
    reduce->SetStride(stride);
}


// ****************************************************************************
//  Method: avtTensorFilter::SetNTensors
//
//  Purpose:
//      Sets the number of vectors the filter should try to output.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2001
//
// ****************************************************************************

void
avtTensorFilter::SetNTensors(int n)
{
    useStride = false;
    stride    = -1;
    nTensors  = n;
    reduce->SetNumberOfElements(nTensors);
}


// ****************************************************************************
//  Method: avtTensorFilter::Equivalent
//
//  Purpose:
//      Determines if this vector filter is equivalent to the attributes.
//
//  Arguments:
//      us      A boolean indicating if you should use the stride (true) or
//              stick with a fixed number of vectors (false).
//      red     The reduction factor, the stride of number of vectors depending
//              on us.
//
//  Returns:     true if it is equal, false otherwise.
//
//  Programmer:  Hank Childs
//  Creation:    September 23, 2003
//
// ****************************************************************************

bool
avtTensorFilter::Equivalent(bool us, int red)
{
    if (us != useStride)
    {
        return false;
    }
    if (useStride)
    {
        if (red != stride)
        {
            return false;
        }
    }
    else
    {
        if (red != nTensors)
        {
            return false;
        }
    }

    return true;
}


// ****************************************************************************
//  Method: avtTensorFilter::ExecuteData
//
//  Purpose:
//      Takes in an input dataset and creates the vector poly data.
//
//  Arguments:
//      inDS      The input dataset.
//      <unused>  The domain number.
//      <unused>  The label.
//
//  Returns:      The output dataset.
//
//  Programmer:   Hank Childs
//  Creation:     September 23, 2003
//
//  Modifications:
//    
//    Kathleen Bonnell, Tue Apr 10 11:51:18 PDT 2001
//    Renamed method as ExecuteData from ExecuteDomain.
//
//    Hank Childs, Thu Aug 30 17:30:48 PDT 2001
//    Added vertex filter.
//
//    Hank Childs, Wed Sep 11 08:53:50 PDT 2002
//    Fixed memory leak.
//
// ****************************************************************************

vtkDataSet *
avtTensorFilter::ExecuteData(vtkDataSet *inDS, int, std::string)
{
    vtkPolyData *outPD = vtkPolyData::New();

    if (inDS->GetPointData()->GetTensors() != NULL)
    {
        vertex->VertexAtPointsOn();
    }
    else
    {
        vertex->VertexAtPointsOff();
    }

    vertex->SetInput(inDS);
    reduce->SetInput(vertex->GetOutput());
    reduce->SetOutput(outPD);
    outPD->Delete();
    outPD->Update();

    return outPD;
}


// ****************************************************************************
//  Method: avtTensorFilter::RefashionDataObjectInfo
//
//  Purpose:
//      Indicate that the vector are of dimension 0.
//
//  Programmer: Hank Childs
//  Creation:   June 12, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep  4 16:14:49 PDT 2001
//    Reflect new interface for avtDataAttributes.
//
//    Hank Childs, Wed Apr  9 13:53:09 PDT 2003
//    Do not calculate normals of these points.
//
// ****************************************************************************

void
avtTensorFilter::RefashionDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    GetOutput()->GetInfo().GetAttributes().SetTopologicalDimension(0);
    GetOutput()->GetInfo().GetValidity().SetNormalsAreInappropriate(true);
}


// ****************************************************************************
//  Method: avtTensorFilter::ReleaseData
//
//  Purpose:
//      Releases all problem size data associated with this filter.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Mar  4 08:12:25 PST 2005
//    Do not set outputs of filters to NULL, since this will prevent them
//    from re-executing correctly in DLB-mode.
//
// ****************************************************************************

void
avtTensorFilter::ReleaseData(void)
{
    avtStreamer::ReleaseData();

    reduce->SetInput(NULL);
    reduce->SetOutput(vtkPolyData::New());
    vertex->SetInput(NULL);
    vertex->SetOutput(vtkPolyData::New());
}


