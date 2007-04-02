// ************************************************************************* //
//                           avtExpressionFilter.C                           //
// ************************************************************************* //

#include <avtExpressionFilter.h>

#include <math.h>
#include <float.h>

#include <vtkCellData.h>
#include <vtkCellDataToPointData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkPointDataToCellData.h>
#include <vtkUnsignedCharArray.h>

#include <avtExtents.h>
#include <avtCommonDataFunctions.h>

#include <avtExprNode.h>

#include <DebugStream.h>
#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtExpressionFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2002
//
//  Modifications:
//    Kathleen Bonnell, Mon Jun 28 07:48:55 PDT 2004
//    Initialize currentTimeState.
//
// ****************************************************************************

avtExpressionFilter::avtExpressionFilter()
{
    outputVariableName = NULL;
    currentTimeState = 0;
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


// ****************************************************************************
//  Method: avtExpressionFilter::ProcessArguments
//
//  Purpose:
//      Walks through the arguments and creates filters.
//
//  Programmer: Sean Ahern
//  Creation:   December 27, 2004 (added header -- presumed to be 2002)
//
//  Modifications:
//
//    Hank Childs, Sat Jan  1 11:11:17 PST 2005
//    Do not assume that there are arguments (which leads to a crash).
//
// ****************************************************************************

void
avtExpressionFilter::ProcessArguments(ArgsExpr *args, ExprPipelineState *state)
{
    if (args == NULL)
    {
        debug5 << "Warning: null arguments given to expression." << endl;
        return;
    }

    std::vector<ArgExpr*> *arguments = args->GetArgs();
    std::vector<ArgExpr*>::iterator i;
    for (i=arguments->begin(); i != arguments->end(); i++)
        dynamic_cast<avtExprNode*>((*i)->GetExpr())->CreateFilters(state);
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
//  Modifications:
//
//    Hank Childs, Wed Dec 10 08:59:31 PST 2003
//    Fix wrap-around lines.
//
//    Kathleen Bonnell, Thu Mar 11 11:24:40 PST 2004
//    DataExents now always have only 2 components. 
//
//    Hank Childs, Mon Dec 27 10:28:51 PST 2004
//    Call avtDatasetToDatasetFilter's PostExecute, since that is now the base 
//    class.
//
// ****************************************************************************
 
void
avtExpressionFilter::PreExecute(void)
{
    avtDatasetToDatasetFilter::PreExecute();
    double exts[2] = {FLT_MAX, -FLT_MAX};
    GetOutput()->GetInfo().GetAttributes().GetCumulativeTrueDataExtents()
                                                                   ->Set(exts);
}


// ****************************************************************************
//  Method: avtExpressionFilter::PostExecute
//
//  Purpose:
//      Sets the active variable and maintains the extents.
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
//    Hank Childs, Mon Dec 27 10:28:51 PST 2004
//    Call avtDatasetToDatasetFilter's PostExecute, since that is now the base 
//    class.
//
//    Hank Childs, Tue Aug 30 13:30:28 PDT 2005
//    Update the extents in the data attributes.
//
// ****************************************************************************

void
avtExpressionFilter::PostExecute(void)
{
    // Make our derived variable be the active variable.
    avtDatasetToDatasetFilter::PostExecute();
    OutputSetActiveVariable(outputVariableName);

    UpdateExtents(GetDataTree());
}


// ****************************************************************************
//  Method: avtExpressionFilter::UpdateExtents
//
//  Purpose:
//      Establishes the cumulative extents.
//
//  Programmer: Hank Childs
//  Creation:   August 30, 2005
//
// ****************************************************************************

void
avtExpressionFilter::UpdateExtents(avtDataTree_p tree)
{
    if (*tree == NULL)
        return;

    int nc = tree->GetNChildren();
    if (nc <= 0 && tree->HasData())
    {
        vtkDataSet *ds = tree->GetDataRepresentation().GetDataVTK();
        bool isPoint = true;
        vtkDataArray *dat = ds->GetPointData()->GetArray(outputVariableName);
        if (dat == NULL)
        {
            dat = ds->GetCellData()->GetArray(outputVariableName);
            isPoint = false;
        }
        if (dat == NULL)
        {
            debug1 << "VERY STRANGE.  We have been asked to update the "
                   << "extents for variable \"" << outputVariableName
                   << "\", but the variable could not be located." << endl;
            return;
        }

        int nvars = dat->GetNumberOfComponents();
        if (nvars <= 3 || nvars == 9)
        {
            double exts[6];
            unsigned char *ghosts = NULL;
            if (!isPoint)
            {
                vtkUnsignedCharArray *g = (vtkUnsignedCharArray *)
                                  ds->GetCellData()->GetArray("avtGhostZones");
                if (g != NULL)
                {
                    ghosts = g->GetPointer(0);
                }
            }
            int ntuples = dat->GetNumberOfTuples();
            exts[0] = +FLT_MAX;
            exts[1] = -FLT_MAX;
            for (int i = 0 ; i < ntuples ; i++)
            {
                if (ghosts != NULL && ghosts[i] > 0)
                {
                    continue;
                }
                float *val = dat->GetTuple(i);
                float value = 0;
                if (nvars == 1)
                    value = *val;
                else if (nvars == 3)
                    value = val[0]*val[0] + val[1] * val[1] + val[2] *val[2];
                else if (nvars == 9)
                    // This function is found in avtCommonDataFunctions.
                    value = MajorEigenvalue(val);
                // else ... array variable
    
                if (value < exts[0])
                    exts[0] = value;
                if (value > exts[1])
                    exts[1] = value;
            }
            if (nvars == 3)
            {
                exts[0] = sqrt(exts[0]);
                exts[1] = sqrt(exts[1]);
            }
            GetOutput()->GetInfo().GetAttributes().
                 GetCumulativeTrueDataExtents(outputVariableName)->Merge(exts);
        }
    }
    else if (nc > 0)
        for (int i = 0 ; i < nc ; i++)
            if (tree->ChildIsPresent(i))
                UpdateExtents(tree->GetChild(i));
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
//    Akira Haddox, Mon Aug 19 16:41:12 PDT 2002
//    Modified to set the centering of the variable to cell or point
//    based on IsPointVariable().
//
//    Hank Childs, Fri Feb 20 15:08:58 PST 2004
//    Account for data attributes using multiple variables.
//
//    Brad Whitlock, Tue Jul 20 17:08:42 PST 2004
//    Added code to propagate units.
//
//    Hank Childs, Thu May 19 13:43:47 PDT 2005
//    Do not assume output variable name has been already set.
//
//    Hank Childs, Mon Aug 29 14:45:40 PDT 2005
//    Moved most of the method to SetExpressionAtts.
//
// ****************************************************************************
 
void
avtExpressionFilter::RefashionDataObjectInfo(void)
{
    avtDataAttributes &inputAtts = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();

    SetExpressionAttributes(inputAtts, outAtts);
}


// ****************************************************************************
//  Method: avtExpressionFilter::SetExpressionAttributes
//
//  Purpose:
//      Sets the attributes of the expression.  This is separated from 
//      RefashionDataObjectInfo, since the CMFE infrastructure also uses this
//      method for setting the info, and it uses "inputAtts" from the alternate
//      database.
//
//  Programmer: Hank Childs
//  Creation:   August 29, 2005
//
// ****************************************************************************

void
avtExpressionFilter::SetExpressionAttributes(const avtDataAttributes &inputAtts,
                                             avtDataAttributes &outAtts)
{
    if (outputVariableName == NULL)
        return;

    if (!outAtts.ValidVariable(outputVariableName))
    {
       if (inputAtts.ValidActiveVariable())
       {
           //
           // Expressions should really do some kind of transformation on
           // the units. For example if you multiply a variable that has
           // Newtons (N) times a variable that is in Meters (m), the resulting
           // units for the output variable should be Nm  (Newton meters).
           // Since we don't have that kind of knowhow in the expressions code
           // yet, preserve the units of the active variable even though that's
           // not really the correct thing to do.
           //
           outAtts.AddVariable(outputVariableName,
                               inputAtts.GetVariableUnits());
       }
       else
           outAtts.AddVariable(outputVariableName);
    }
    outAtts.SetActiveVariable(outputVariableName);
    int dim = GetVariableDimension();
    outAtts.SetVariableDimension(dim);
    if (GetVariableType() != AVT_UNKNOWN_TYPE)
    {
        outAtts.SetVariableType(GetVariableType());
    }
    else
    {
        if (dim == 1)
            outAtts.SetVariableType(AVT_SCALAR_VAR);
        else if (dim == 3)
            outAtts.SetVariableType(AVT_VECTOR_VAR);
        else if (dim == 9)
            outAtts.SetVariableType(AVT_TENSOR_VAR);
        else
            outAtts.SetVariableType(AVT_SCALAR_VAR);
    }
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
//  Modifications:
//
//    Hank Childs, Wed Feb 25 14:43:17 PST 2004
//    Modify logic slightly since, with multiple variable changes, there
//    may not be any variables to ask for their centering.
//
// ****************************************************************************

bool
avtExpressionFilter::IsPointVariable()
{
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    if (atts.ValidActiveVariable())
    {
        return (atts.GetCentering() != AVT_ZONECENT);
    }

    return true;
}


// ****************************************************************************
//  Method: avtExpressionFilter::Recenter
//
//  Purpose:
//      Recenters a variable from zonal to nodal or vice-versa.
//
//  Arguments:
//      ds      The mesh the variable lays on.
//      arr     The variable to recenter.
//      cent    The centering of the variable now -- NOT the desired centering.
//
//  Returns:    The array recentered.  Note: the calling routine will then
//              be responsible for deleting the returned object.
//      
//  Programmer: Hank Childs
//  Creation:   December 10, 2003
//
// ****************************************************************************

vtkDataArray *
avtExpressionFilter::Recenter(vtkDataSet *ds, vtkDataArray *arr, 
                              avtCentering cent)
{
    vtkDataSet *ds2 = ds->NewInstance();
    ds2->CopyStructure(ds);

    vtkDataArray *outv = NULL;
    if (cent == AVT_NODECENT)
    {
        if (ds2->GetNumberOfPoints() != arr->GetNumberOfTuples())
        {
            EXCEPTION1(ExpressionException, "Asked to re-center a nodal "
                       "variable that is not nodal.");
        }

        ds2->GetPointData()->SetScalars(arr);

        vtkPointDataToCellData *pd2cd = vtkPointDataToCellData::New();
        pd2cd->SetInput(ds2);
        vtkDataSet *ds3 = pd2cd->GetOutput();
        ds3->Update();
        outv = ds3->GetCellData()->GetScalars();
        outv->Register(NULL);
        pd2cd->Delete();
    }
    else
    {
        if (ds2->GetNumberOfCells() != arr->GetNumberOfTuples())
        {
            EXCEPTION1(ExpressionException, "Asked to re-center a zonal "
                       "variable that is not zonal.");
        }

        ds2->GetCellData()->SetScalars(arr);

        vtkCellDataToPointData *cd2pd = vtkCellDataToPointData::New();
        cd2pd->SetInput(ds2);
        vtkDataSet *ds3 = cd2pd->GetOutput();
        ds3->Update();
        outv = ds3->GetPointData()->GetScalars();
        outv->Register(NULL);
        cd2pd->Delete();
    }

    ds2->Delete();
    return outv;
}


// ****************************************************************************
//  Method: avtExpressionFilter::ExamineSpecification
//
//  Purpose:
//    Capture current timestep information for use by derived types.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 25, 2004 
//
// ****************************************************************************

void
avtExpressionFilter::ExamineSpecification(avtPipelineSpecification_p spec)
{
    avtDatasetToDatasetFilter::ExamineSpecification(spec);
    currentTimeState = spec->GetDataSpecification()->GetTimestep();
} 


