// ************************************************************************* //
//                             avtVectorFilter.C                             //
// ************************************************************************* //

#include <avtVectorFilter.h>

#include <vtkPolyData.h>
#include <vtkVectorReduceFilter.h>
#include <vtkVertexFilter.h>


// ****************************************************************************
//  Method: avtVectorFilter constructor
//
//  Arguments:
//      us      A boolean indicating if you should use the stride (true) or
//              stick with a fixed number of vectors (false).
//      red     The reduction factor, the stride of number of vectors depending
//              on us.
//
//  Programmer: Hank Childs
//  Creation:   March 21, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Aug 30 17:30:48 PDT 2001
//    Added vertex filter.
//
// ****************************************************************************

avtVectorFilter::avtVectorFilter(bool us, int red)
{
    reduce = vtkVectorReduceFilter::New();
    vertex = vtkVertexFilter::New();

    if (us)
    {
        SetStride(red);
    }
    else
    {
        SetNVectors(red);
    }

    if (useStride)
    {
        reduce->SetStride(stride);
    }
    else
    {
        reduce->SetNumberOfElements(nVectors);
    }
}


// ****************************************************************************
//  Method: avtVectorFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   March 21, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Aug 30 17:30:48 PDT 2001
//    Added vertex filter.
//
// ****************************************************************************

avtVectorFilter::~avtVectorFilter()
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
//  Method: avtVectorFilter::SetStride
//
//  Purpose:
//      Sets the stride of reduction for the vector.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2001
//
// ****************************************************************************

void
avtVectorFilter::SetStride(int s)
{
    useStride = true;
    stride    = s;
    nVectors  = -1;
    reduce->SetStride(stride);
}


// ****************************************************************************
//  Method: avtVectorFilter::SetNVectors
//
//  Purpose:
//      Sets the number of vectors the filter should try to output.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2001
//
// ****************************************************************************

void
avtVectorFilter::SetNVectors(int n)
{
    useStride = false;
    stride    = -1;
    nVectors  = n;
    reduce->SetNumberOfElements(nVectors);
}


// ****************************************************************************
//  Method: avtVectorFilter::Equivalent
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
//  Creation:    March 21, 2001
//
// ****************************************************************************

bool
avtVectorFilter::Equivalent(bool us, int red)
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
        if (red != nVectors)
        {
            return false;
        }
    }

    return true;
}


// ****************************************************************************
//  Method: avtVectorFilter::ExecuteData
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
//  Creation:     March 21, 2001
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
avtVectorFilter::ExecuteData(vtkDataSet *inDS, int, std::string)
{
    vtkPolyData *outPD = vtkPolyData::New();

    if (inDS->GetPointData()->GetVectors() != NULL)
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
//  Method: avtVectorFilter::RefashionDataObjectInfo
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
avtVectorFilter::RefashionDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    GetOutput()->GetInfo().GetAttributes().SetTopologicalDimension(0);
    GetOutput()->GetInfo().GetValidity().SetNormalsAreInappropriate(true);
}


// ****************************************************************************
//  Method: avtVectorFilter::ReleaseData
//
//  Purpose:
//      Releases all problem size data associated with this filter.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2002
//
// ****************************************************************************

void
avtVectorFilter::ReleaseData(void)
{
    avtStreamer::ReleaseData();

    reduce->SetInput(NULL);
    reduce->SetOutput(NULL);
    vertex->SetInput(NULL);
    vertex->SetOutput(NULL);
}


