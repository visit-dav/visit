// ************************************************************************* //
//                             avtVectorFilter.C                             //
// ************************************************************************* //

#include <avtVectorFilter.h>

#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkVectorReduceFilter.h>
#include <vtkVertexFilter.h>

#include <Expression.h>
#include <ExpressionList.h>
#include <ParsingExprList.h>

#include <string.h>
using std::string;


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
//    Kathleen Bonnell, Tue Oct 12 16:18:37 PDT 2004 
//    Added keepNodeZone. 
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
    keepNodeZone = false;
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
avtVectorFilter::ExecuteData(vtkDataSet *inDS, int, string)
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
//    Kathleen Bonnell, Tue Oct 12 16:02:33 PDT 2004 
//    Set whether or not we want to specify that node and zone arrays be
//    kept around through to the renderer.  (Needed by pick).
//
// ****************************************************************************

void
avtVectorFilter::RefashionDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    GetOutput()->GetInfo().GetAttributes().SetTopologicalDimension(0);
    GetOutput()->GetInfo().GetValidity().SetNormalsAreInappropriate(true);
    GetOutput()->GetInfo().GetAttributes().SetKeepNodeZoneArrays(keepNodeZone);
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
//  Modifications:
//
//    Hank Childs, Fri Mar  4 08:12:25 PST 2005
//    Do not set outputs of filters to NULL, since this will prevent them
//    from re-executing correctly in DLB-mode.
//
// ****************************************************************************

void
avtVectorFilter::ReleaseData(void)
{
    avtStreamer::ReleaseData();

    reduce->SetInput(NULL);
    reduce->SetOutput(vtkPolyData::New());
    vertex->SetInput(NULL);
    vertex->SetOutput(vtkPolyData::New());
}


// ****************************************************************************
//  Method: avtVectorFilter::SetMagVarName
//
//  Purpose:
//    Sets the name that should be used when creating the magnitude data
//    array. 
//
//  Arguments:
//    mname     The name of the magnitude array.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 9, 2004 
//
// ****************************************************************************

void
avtVectorFilter::SetMagVarName(const string &mname)
{
    magVarName = mname;
}


// ****************************************************************************
//  Method: avtVectorFilter::PerformRestriction
//
//  Purpose:  Create an expression for the magnitude of the requested
//            vector variable, so that the vectors are colored correctly.
// 
//  Programmer: Kathleen Bonnell 
//  Creation:   August 9, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Tue Oct 12 16:02:33 PDT 2004 
//    Store whether or not we want to specify that node and zone arrays be
//    kept around through to the renderer.  (Needed by pick).
//
//    Kathleen Bonnell, Wed Dec  1 09:19:31 PST 2004 
//    Turn on Node/Zone numbers when 'MayRequireNodes' is true. 
//
// ****************************************************************************

avtPipelineSpecification_p
avtVectorFilter::PerformRestriction(avtPipelineSpecification_p pspec)
{
    avtPipelineSpecification_p rv = pspec;
    avtDataSpecification_p ds = pspec->GetDataSpecification();
    //
    // Create the expression definition
    //
    string edef = string("magnitude(") + ds->GetVariable() + string(")");

    ExpressionList *elist = ParsingExprList::Instance()->GetList();
    Expression *e = new Expression();
        
    e->SetName(magVarName.c_str());
    e->SetDefinition(edef.c_str());
    e->SetType(Expression::ScalarMeshVar);
    elist->AddExpression(*e);
    delete e;

    // Create a new dpspec so that we can add the secondary var.
    avtDataSpecification_p nds = new avtDataSpecification(ds->GetVariable(),
                ds->GetTimestep(), ds->GetRestriction());
    nds->AddSecondaryVariable(magVarName.c_str());
    rv = new avtPipelineSpecification(pspec, nds);
    

    avtDataAttributes &data = GetInput()->GetInfo().GetAttributes();
    if (pspec->GetDataSpecification()->MayRequireZones() || 
        pspec->GetDataSpecification()->MayRequireNodes())
    {
        keepNodeZone = true;
        if (data.GetCentering() == AVT_NODECENT)
        {
            rv->GetDataSpecification()->TurnNodeNumbersOn();
        }
        else if (data.GetCentering() == AVT_ZONECENT)
        {
            rv->GetDataSpecification()->TurnZoneNumbersOn();
        }
    }
    else
    {
        keepNodeZone = false;
    }

    return rv;
}
