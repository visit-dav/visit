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

#include <EngineExprNode.h>

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
//  Modifications:
//
//    Hank Childs, Wed Dec 10 08:59:31 PST 2003
//    Fix wrap-around lines.
//
//    Kathleen Bonnell, Thu Mar 11 11:24:40 PST 2004
//    DataExents now always have only 2 components. 
//
// ****************************************************************************
 
void
avtExpressionFilter::PreExecute(void)
{
    avtStreamer::PreExecute();
    double exts[2] = {FLT_MAX, -FLT_MAX};
    GetOutput()->GetInfo().GetAttributes().GetCumulativeTrueDataExtents()
                                                                   ->Set(exts);
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
//    Hank Childs, Mon Sep 22 08:34:14 PDT 2003
//    Add support for tensors.
//
//    Hank Childs, Thu Jan 29 16:22:06 PST 2004
//    Fix some odd centering cases that can come up when a variable can get
//    misidentified.
//
//    Hank Childs, Wed Feb 25 14:48:31 PST 2004
//    Make sure that the extents get associated with the correct variable.
//
//    Kathleen Bonnell, Thu Mar 11 11:24:40 PST 2004 
//    DataExtents now always only 2 components.  Allow for Tensor var's range
//    to be computed. 
//
// ****************************************************************************

vtkDataSet *
avtExpressionFilter::ExecuteData(vtkDataSet *in_ds, int index,
                                 std::string label)
{
    int   i;

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
    int npts   = rv->GetNumberOfPoints();
    int ncells = rv->GetNumberOfCells();
    int ntups  = dat->GetNumberOfTuples();
    bool isPoint = false;
    if ((ntups == ncells) && (ntups != npts))
    {
        isPoint = false;
    }
    else if ((ntups == npts) && (ntups != ncells))
    {
        isPoint = true;
    }
    else if ((ntups == npts) && (ntups == ncells))
    {
        isPoint = IsPointVariable();
    }
    else
    {
        debug1 << "Number of tuples cannot be point or cell variable."
               << endl;
        debug1 << "Var = " << dat->GetName() << endl;
        debug1 << "Ntuples = " << ntups << endl;
        debug1 << "Ncells = " << ncells << endl;
        debug1 << "Npts = " << npts << endl;
        return rv;
    }

    if (isPoint)
    {
        rv->GetPointData()->AddArray(dat);
        if (vardim == 1)
            rv->GetPointData()->SetActiveScalars(outputVariableName);
        else if (vardim == 3)
            rv->GetPointData()->SetActiveVectors(outputVariableName);
        else if (vardim == 9)
            rv->GetPointData()->SetActiveTensors(outputVariableName);
    }
    else
    {
        rv->GetCellData()->AddArray(dat);
        if (vardim == 1)
            rv->GetCellData()->SetActiveScalars(outputVariableName);
        else if (vardim == 3)
            rv->GetCellData()->SetActiveVectors(outputVariableName);
        else if (vardim == 9)
            rv->GetCellData()->SetActiveTensors(outputVariableName);
    }

    //
    // Make our best attempt at maintaining our extents.
    //
    int nvars   = dat->GetNumberOfComponents();
    if (nvars <= 3 || nvars == 9)
    {
        double exts[6];
        unsigned char *ghosts = NULL;
        if (!isPoint)
        {
            vtkUnsignedCharArray *g = (vtkUnsignedCharArray *)
                                 rv->GetCellData()->GetArray("vtkGhostLevels");
            if (g != NULL)
            {
                ghosts = g->GetPointer(0);
            }
        }
        int ntuples = dat->GetNumberOfTuples();
        exts[0] = +FLT_MAX;
        exts[1] = -FLT_MAX;
        for (i = 0 ; i < ntuples ; i++)
        {
            if (ghosts != NULL && ghosts[i] > 0) 
            {
                continue;
            }
            float *val = dat->GetTuple(i);
            float value; 
            if (nvars == 1)
            {
                value = *val;
            }
            else if (nvars == 3)
            {
                value = val[0]*val[0] + val[1] * val[1] + val[2] *val[2];
            }
            else if (nvars == 9)
            {
                // This function is found in avtCommonDataFunctions.
                value = MajorEigenvalue(val);    
            }

            if (value < exts[0])
            {
                exts[0] = value;
            }
            if (value > exts[1])
            {
                exts[1] = value;
            }
        }
        if (nvars == 3)
        {
            exts[0] = sqrt(exts[0]);
            exts[1] = sqrt(exts[1]);
        }
        GetOutput()->GetInfo().GetAttributes().
                 GetCumulativeTrueDataExtents(outputVariableName)->Merge(exts);
    }

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
// ****************************************************************************
 
void
avtExpressionFilter::RefashionDataObjectInfo(void)
{
    avtDataAttributes &inputAtts = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();

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
        outAtts.AddVariable(outputVariableName, inputAtts.GetVariableUnits());
    }
    else
        outAtts.AddVariable(outputVariableName);
    outAtts.SetActiveVariable(outputVariableName);
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
