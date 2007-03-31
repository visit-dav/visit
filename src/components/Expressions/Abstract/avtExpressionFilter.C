// ************************************************************************* //
//                           avtExpressionFilter.C                           //
// ************************************************************************* //

#include <EngineExprNode.h>
#include <avtExpressionFilter.h>

#include <math.h>
#include <float.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <DebugStream.h>
#include <avtExtents.h>


// ****************************************************************************
//  Method: avtExpressionFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2002
//
// ****************************************************************************

avtExpressionFilter::avtExpressionFilter()
{
    outputVariableName = NULL;
}


// ****************************************************************************
//  Method: avtExpressionFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2002
//
// ****************************************************************************

avtExpressionFilter::~avtExpressionFilter()
{
    if (outputVariableName != NULL)
    {
        delete [] outputVariableName;
        outputVariableName = NULL;
    }
}

void
avtExpressionFilter::ProcessArguments(ArgsExpr *args, ExprPipelineState *state)
{
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    std::vector<ArgExpr*>::iterator i;
    for (i=arguments->begin(); i != arguments->end(); i++)
        dynamic_cast<EngineExprNode*>((*i)->GetExpr())->CreateFilters(state);
}


// ****************************************************************************
//  Method: avtExpressionFilter::SetOutputVariableName
//
//  Purpose:
//      Sets the name of the expression.
//
//  Arguments:
//      name     The name of the derived output variable.
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2002
//
// ****************************************************************************

void
avtExpressionFilter::SetOutputVariableName(const char *name)
{
    if (outputVariableName != NULL)
        delete [] outputVariableName;

    outputVariableName = new char[strlen(name)+1];
    strcpy(outputVariableName, name);
}


// ****************************************************************************
//  Method: avtExpressionFilter::PreExecute
//
//  Purpose:
//      Gins up some default extents so that the cumulative ones will work.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//
//  Returns:       The output dataset.
//
//  Programmer: Hank Childs
//  Creation:   June 7, 2002
//
// ****************************************************************************
 
void
avtExpressionFilter::PreExecute(void)
{
    avtStreamer::PreExecute();
    double exts[6] = {FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX};
    GetOutput()->GetInfo().GetAttributes().GetCumulativeTrueDataExtents()->Set(exts);
}


// ****************************************************************************
//  Method: avtExpressionFilter::PostExecute
//
//  Purpose:
//      Gins up some default extents so that the cumulative ones will work.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//
//  Returns:       The output dataset.
//
//  Programmer:  ??? <Sean Ahern>
//  Creation:    ~June 7, 2002
//
//  Modifications:
//
//    Hank Childs, Mon Jul 21 09:35:52 PDT 2003
//    Called avtStreamer's PostExecute, since that is the base class.
//    
// ****************************************************************************

void
avtExpressionFilter::PostExecute(void)
{
    // Make our derived variable be the active variable.
    avtStreamer::PostExecute();
    OutputSetActiveVariable(outputVariableName);
}


// ****************************************************************************
//  Method: avtExpressionFilter::ExecuteData
//
//  Purpose:
//      Does the actual VTK code to modify the dataset.
//
//  Arguments:
//      inDS      The input dataset.
//      <unused>  The domain number.
//
//  Returns:      The output dataset.
//
//  Programmer:   Hank Childs
//  Creation:     June 7, 2002
//
//  Modifications:
//
//    Sean Ahern, Tue Jun 11 16:35:48 PDT 2002
//    Modified to work with both point and cell data.  Note, this doesn't
//    yet handle filters that convert from point to cell, or vice versa.
//
//    Akira Haddox, Thu Aug 15 16:41:44 PDT 2002
//    Modified to work with vector expressions.
//
//    Akira Haddox, Mon Aug 19 16:39:07 PDT 2002 
//    Modified to use IsPointVariable to determine variable type.
//
//    Hank Childs, Mon Sep  2 18:53:40 PDT 2002
//    Do not use enhanced connectivity points when calculating ranges.
//
//    Hank Childs, Tue Nov 19 08:50:42 PST 2002
//    Use the dimension of the returned type instead of the virtual function
//    call to decide if the variable is a vector.  Did not remove virtual
//    function to determine this, since it may be used before execution (by the
//    GUI, for example to decide if something is a vector,scalar, etc).
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Use NewInstance instead of MakeObject, new vtk api. 
//
//    Hank Childs, Thu Jan 23 11:19:14 PST 2003
//    Set only the cumulative extents.
//
// ****************************************************************************

vtkDataSet *
avtExpressionFilter::ExecuteData(vtkDataSet *in_ds, int index,
                                 std::string label)
{
    int   i, j;

    //
    // Start off by having the derived type calculate the derived variable.
    //
    currentDomainsLabel = label;
    currentDomainsIndex = index;
    vtkDataArray *dat = DeriveVariable(in_ds);
    dat->SetName(outputVariableName);

    int vardim = dat->GetNumberOfComponents();

    //
    // Now make a copy of the input and add the derived variable as its output.
    //
    vtkDataSet *rv = (vtkDataSet *) in_ds->NewInstance();
    rv->ShallowCopy(in_ds);
    if (IsPointVariable())
    {
        rv->GetPointData()->AddArray(dat);
        if (vardim == 1)
        {
            rv->GetPointData()->SetActiveScalars(outputVariableName);
        }
        else
        {
            rv->GetPointData()->SetActiveVectors(outputVariableName);
        }
    }
    else
    {
        rv->GetCellData()->AddArray(dat);
        if (vardim == 1)
        {
            rv->GetCellData()->SetActiveScalars(outputVariableName);
        }
        else
        {
            rv->GetCellData()->SetActiveVectors(outputVariableName);
        }
    }

    //
    // Make our best attempt at maintaining our extents.
    //
    double exts[6];
    unsigned char *ghosts = NULL;
    if (!IsPointVariable())
    {
        vtkUnsignedCharArray *g = (vtkUnsignedCharArray *)
                                 rv->GetCellData()->GetArray("vtkGhostLevels");
        if (g != NULL)
        {
            ghosts = g->GetPointer(0);
        }
    }
    int ntuples = dat->GetNumberOfTuples();
    int nvars   = dat->GetNumberOfComponents();
    for (i = 0 ; i < nvars ; i++)
    {
        exts[2*i+0] = +FLT_MAX;
        exts[2*i+1] = -FLT_MAX;
    }
    for (i = 0 ; i < ntuples ; i++)
    {
        if (ghosts != NULL && ghosts[i] > 0) 
        {
            continue;
        }
        float *val = dat->GetTuple(i);
        for (j = 0 ; j < nvars ; j++)
        {
            if (val[j] < exts[2*j+0])
            {
                exts[2*j+0] = val[j];
            }
            if (val[j] > exts[2*j+1])
            {
                exts[2*j+1] = val[j];
            }
        }
    }
    GetOutput()->GetInfo().GetAttributes().
                                   GetCumulativeTrueDataExtents()->Merge(exts);

    //
    // Make sure that we don't have any memory leaks.
    //
    dat->Delete();
    ManageMemory(rv);
    rv->Delete();

    return rv;
}


// ****************************************************************************
//  Method: avtExpressionFilter::RefashionDataObjectInfo
//
//  Purpose:
//      Tells our output that we now have a variable.
//
//  Programmer: Hank Childs
//  Creation:   June 7, 2002
//
//  Modifications:
//      Akira Haddox, Mon Aug 19 16:41:12 PDT 2002
//      Modified to set the centering of the variable to cell or point
//      based on IsPointVariable().
// ****************************************************************************
 
void
avtExpressionFilter::RefashionDataObjectInfo(void)
{
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.SetVariableName(outputVariableName);
    outAtts.SetVariableDimension(GetVariableDimension());
    outAtts.SetCentering(IsPointVariable()?AVT_NODECENT:AVT_ZONECENT);
}


// ****************************************************************************
//  Method: avtExpressionFilter::PerformRestriction
//
//  Purpose:
//      Determines if there is a request for the derived type's derived 
//      variable as a secondary variable.  If so, snag the request, because it
//      will only confuse the database.
//
//  Programmer: Hank Childs
//  Creation:   June 7, 2002
//
// ****************************************************************************

avtPipelineSpecification_p
avtExpressionFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    avtPipelineSpecification_p rv = spec;

    avtDataSpecification_p ds = spec->GetDataSpecification();
    if (ds->HasSecondaryVariable(outputVariableName))
    {
        avtDataSpecification_p newds = new avtDataSpecification(ds);
        newds->RemoveSecondaryVariable(outputVariableName);
        rv = new avtPipelineSpecification(spec, newds);
    }

    return rv;
} 


// ****************************************************************************
//  Method: avtExpressionFilter::IsPointVariable
//
//  Purpose:
//      Used to determine if expression is for point or cell data. Defaults
//      to cell if input is cell data, or point data otherwise.
//
//  Returns:    True if the data should be point data, false if cell data.
//
//  Programmer: Akira Haddox 
//  Creation:   August 19, 2002
//
// ****************************************************************************

bool
avtExpressionFilter::IsPointVariable()
{
    return (GetInput()->GetInfo().GetAttributes().GetCentering()
            == AVT_NODECENT);
}


