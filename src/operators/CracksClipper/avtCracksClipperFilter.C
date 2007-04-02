// ************************************************************************* //
//  File: avtCracksClipperFilter.C
// ************************************************************************* //

#include <avtCracksClipperFilter.h>
#include <vtkCellData.h>
#include <vtkCracksClipper.h>
#include <vtkCrackWidthFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkAppendFilter.h>
#include <DebugStream.h>
#include <InvalidVariableException.h>


// ****************************************************************************
//  Method: avtCracksClipperFilter constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   Thu Oct 13 08:17:36 PDT 2005
//
// ****************************************************************************

avtCracksClipperFilter::avtCracksClipperFilter()
{
}


// ****************************************************************************
//  Method: avtCracksClipperFilter destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   Thu Oct 13 08:17:36 PDT 2005
//
//  Modifications:
//
// ****************************************************************************

avtCracksClipperFilter::~avtCracksClipperFilter()
{
}


// ****************************************************************************
//  Method:  avtCracksClipperFilter::Create
//
//  Programmer: Kathleen Bonnell
//  Creation:   Thu Oct 13 08:17:36 PDT 2005
//
// ****************************************************************************

avtFilter *
avtCracksClipperFilter::Create()
{
    return new avtCracksClipperFilter();
}


// ****************************************************************************
//  Method:      avtCracksClipperFilter::SetAtts
//
//  Purpose:
//    Sets the state of the filter based on the attribute object.
//
//  Arguments:
//    a         The attributes to use.
//
//  Programmer: Kathleen Bonnell
//  Creation:   Thu Oct 13 08:17:36 PDT 2005
//
// ****************************************************************************

void
avtCracksClipperFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const CracksClipperAttributes*)a;
}


// ****************************************************************************
//  Method: avtCracksClipperFilter::Equivalent
//
//  Purpose:
//    Returns true if creating a new avtCracksClipperFilter with the given
//    parameters would result in an equivalent avtCracksClipperFilter.
//
//  Programmer: Kathleen Bonnell
//  Creation:   Thu Oct 13 08:17:36 PDT 2005
//
// ****************************************************************************

bool
avtCracksClipperFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(CracksClipperAttributes*)a);
}


// ****************************************************************************
//  Method: avtCracksClipperFilter::ExecuteData
//
//  Purpose:
//    Sends the specified input and output through the Cracks filter.
//
//  Arguments:
//    in_ds     The input dataset.
//    dom       The domain number.
//    <unused>  The label.
//
//  Returns:    The output dataset.
//
//  Programmer: Kathleen Bonnell
//  Creation:   Thu Oct 13 08:17:36 PDT 2005
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet*
avtCracksClipperFilter::ExecuteData(vtkDataSet *in_ds, int dom, std::string)
{
    if (in_ds == NULL || in_ds->GetNumberOfCells() == 0)
        return NULL;

    //
    // We've requested secondary vars needed only by this filter,
    // if we won't be processing the input, we need an output without
    // the requested vars.
    //
    vtkDataSet *defaultReturn = in_ds->NewInstance();
    defaultReturn->ShallowCopy(in_ds);
    RemoveExtraArrays(defaultReturn);

    vtkDataSet *rv;
    //
    // Does this domain need to be processed?
    //
    bool needsProc[3];
    if (!NeedsProcessing(in_ds, needsProc))
    {
        rv = defaultReturn;
        ManageMemory(defaultReturn);
        defaultReturn->Delete();
        return rv;
    }

    //
    // Calculate the crack widths and cell centers needed by the 
    // cracks clip filter
    //
    vtkCrackWidthFilter *cwf = vtkCrackWidthFilter::New();
    cwf->SetInput(in_ds); 
    if (atts.GetUseCrack1())
      cwf->SetCrack1Var(atts.GetCrack1Var().c_str());
    if (atts.GetUseCrack2())
      cwf->SetCrack2Var(atts.GetCrack2Var().c_str());
    if (atts.GetUseCrack3())
      cwf->SetCrack3Var(atts.GetCrack3Var().c_str());
    cwf->SetStrainVar(atts.GetStrainVar().c_str());
    cwf->Update();

    float mw[3];
    for (int i = 0; i < 3; i++)
    {
        mw[i] = cwf->GetMaxCrackWidth(i);
    }

    //
    // If all the crack widths are zero, then no need to process
    //
    if (mw[0] == 0 && mw[1] == 0 && mw[2] == 0)
    {
        debug5 << "avtCracksClipperFilter not processing domain " << dom
               << "  because all crack widths are zero" << endl;
        cwf->Delete();
        rv = defaultReturn;
        ManageMemory(defaultReturn);
        defaultReturn->Delete();
        return rv;
    }

    vtkDataSet *output[3] = {NULL, NULL, NULL};
    if (needsProc[0]  && mw[0] > 0)
    {
        output[0] = RemoveCracks(cwf->GetOutput(), 0);
    }
    if (needsProc[1]  && mw[1] > 0)
    {
        if (needsProc[0]  && mw[0] > 0)
        {
            output[1] = RemoveCracks(output[0], 1);
            output[0]->Delete();
            output[0] = NULL;
        }
        else 
        {
            output[1] = RemoveCracks(cwf->GetOutput(), 1);
        }
    }
    if (needsProc[2]  && mw[2] > 0)
    {
        if (needsProc[1]  && mw[1] > 0)
        {
            output[2] = RemoveCracks(output[1], 2);
            output[1]->Delete();
            output[1] = NULL;
        }
        else if (needsProc[0]  && mw[0] > 0)
        {
            output[2] = RemoveCracks(output[0], 2);
            output[0]->Delete();
            output[0] = NULL;
        }
        else 
        {
            output[2] = RemoveCracks(cwf->GetOutput(), 2);
        }
    }

    cwf->Delete();
    if (output[2] != NULL)
    {
        if (output[2]->GetNumberOfCells() <= 0)
        {
            output[2]->Delete();
            output[2] = NULL;
            rv = NULL;
        }
        else
        {
            RemoveExtraArrays(output[2], true);
            rv = output[2];
            ManageMemory(output[2]);
            output[2]->Delete();
        }
        defaultReturn->Delete();
    }
    else if (output[1] != NULL)
    {
        if (output[1]->GetNumberOfCells() <= 0)
        {
            output[1]->Delete();
            output[1] = NULL;
            rv = NULL; 
        }
        else
        {
            RemoveExtraArrays(output[1], true);
            rv = output[1];
            ManageMemory(output[1]);
            output[1]->Delete();
        }
        defaultReturn->Delete();
    }
    else if (output[0] != NULL)
    {
        if (output[0]->GetNumberOfCells() <= 0)
        {
            output[0]->Delete();
            output[0] = NULL;
            rv = NULL;
        }
        else
        {
            RemoveExtraArrays(output[0], true);
            rv = output[0];
            ManageMemory(output[0]);
            output[0]->Delete();
        }
        defaultReturn->Delete();
    }
    else
    {
        rv = defaultReturn;
        ManageMemory(defaultReturn);
        defaultReturn->Delete();
    }
    return rv;
}


// ****************************************************************************
//  Method: avtCracksClipperFilter::PerformRestriction
//
//  Purpose:
//    Requests secondary variables needed by this filter, turns on Node and/or
//    Zone numbers when appropriate. 
//
//  Arguments:
//    pspec     The original pipeline specification.
//
//  Returns:    The modified pipeline specification. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   Thu Oct 13 08:17:36 PDT 2005
//
//  Modifications:
//
// ****************************************************************************

avtPipelineSpecification_p
avtCracksClipperFilter::PerformRestriction(avtPipelineSpecification_p pspec)
{
    avtDataSpecification_p ds = pspec->GetDataSpecification();

    // Create a new dspec so that we can add secondary vars
    avtDataSpecification_p nds = new avtDataSpecification(ds->GetVariable(),
                ds->GetTimestep(), ds->GetRestriction());

    if (atts.GetUseCrack1())
      nds->AddSecondaryVariable(atts.GetCrack1Var().c_str());
    if (atts.GetUseCrack2())
      nds->AddSecondaryVariable(atts.GetCrack2Var().c_str());
    if (atts.GetUseCrack3())
      nds->AddSecondaryVariable(atts.GetCrack3Var().c_str());

    nds->AddSecondaryVariable(atts.GetStrainVar().c_str());
    avtPipelineSpecification_p rv = new avtPipelineSpecification(pspec, nds);

    //
    // Since this filter 'clips' the dataset, the zone and possibly
    // node numbers will be invalid, request them when needed.
    //
    avtDataAttributes &data = GetInput()->GetInfo().GetAttributes();
    if (pspec->GetDataSpecification()->MayRequireZones() || 
        pspec->GetDataSpecification()->MayRequireNodes())
    {
        if (data.ValidActiveVariable())
        {
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
            // canot determine variable centering, so turn on both
            // node numbers and zone numbers.
            rv->GetDataSpecification()->TurnNodeNumbersOn();
            rv->GetDataSpecification()->TurnZoneNumbersOn();
        }
    }
    return rv; 
}


// ****************************************************************************
//  Method: avtCracksClipperFilter::RefashionDataObjectInfo
//
//  Purpose:
//    Informs the pipeline that this filter modifies zones.
//
//  Programmer: Kathleen Bonnell
//  Creation:   Thu Oct 13 08:17:36 PDT 2005
//
//  Modifications:
//    Kathleen Bonnell, Thu Mar  2 14:26:06 PST 2006 
//    Set ZonesSplit.
//
// ****************************************************************************

void
avtCracksClipperFilter::RefashionDataObjectInfo()
{
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    GetOutput()->GetInfo().GetValidity().ZonesSplit();
}


// ****************************************************************************
//  Method: avtCracksClipperFilter::PostExecute
//
//  Purpose:
//    Removes secondary variables from the pipeline that were requested 
//    during PerformRestriction.
//
//  Programmer: Kathleen Bonnell
//  Creation:   Thu Oct 13 08:17:36 PDT 2005
//
//  Modifications:
//
// ****************************************************************************

void
avtCracksClipperFilter::PostExecute()
{
    avtPluginStreamer::PostExecute();
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();

    // Remove the secondary variable we requested before.
    outAtts.RemoveVariable(atts.GetCrack1Var().c_str());
    outAtts.RemoveVariable(atts.GetCrack2Var().c_str());
    outAtts.RemoveVariable(atts.GetCrack3Var().c_str());
    outAtts.RemoveVariable(atts.GetStrainVar().c_str());
}


// ****************************************************************************
//  Method: avtCracksClipperFilter::NeedsProcessing
//
//  Purpose:
//    Determines whether or not the passed data set should be processed for
//    crack removal. 
//
//  Arguments:
//    ds        The input dataset.
//    np        A place to store flags indicating whether individiual crack 
//              directions should be processed.
//
//  Returns:    A Flag indicating whether any processing should occur for the
//              input dataset.   True if any crack direction should be
//              processed, false if no crack direction should be processed.
//
//  Programmer: Kathleen Bonnell
//  Creation:   Thu Oct 13 08:17:36 PDT 2005
// 
//  Modifications:
//
// ****************************************************************************

bool
avtCracksClipperFilter::NeedsProcessing(vtkDataSet *ds, bool *np)
{
    if (ds == NULL)
        return false;

    //
    // Are all the necessary variable arrays in the dataset?
    //
    vtkDataArray *strain = ds->GetCellData()->
        GetArray(atts.GetStrainVar().c_str());

    if (strain == NULL)
    {
        EXCEPTION1(InvalidVariableException, atts.GetStrainVar().c_str());
    }
 
    int i;
    for (i = 0; i < 3; i++)
        np[i] = false;
    vtkDataArray *cracks[3] = {NULL, NULL, NULL};
    if (atts.GetUseCrack1())
    {
        cracks[0] = ds->GetCellData()->GetArray(atts.GetCrack1Var().c_str());
        if (cracks[0] == NULL)
        {
            EXCEPTION1(InvalidVariableException, atts.GetCrack1Var().c_str());
        } 
        np[0] = true;
    }
    if (atts.GetUseCrack2())
    {
        cracks[1] = ds->GetCellData()->GetArray(atts.GetCrack2Var().c_str());
        if (cracks[1] == NULL)
        {
            EXCEPTION1(InvalidVariableException, atts.GetCrack2Var().c_str());
        } 
        np[1] = true;
    }
    if (atts.GetUseCrack3())
    {
        cracks[2] = ds->GetCellData()->GetArray(atts.GetCrack3Var().c_str());
        if (cracks[2] == NULL)
        {
            EXCEPTION1(InvalidVariableException, atts.GetCrack3Var().c_str());
        } 
        np[2] = true;
    }

    // If there aren't any crack directions, no point in continuing
    if (!np[0] && !np[1] && !np[2])
        return false;

    //
    // The crack width will be determined by the strain, do we have
    // non-zero strain values for a given crack direction that will 
    // necessitate further processing?
    //
    int nc = strain->GetNumberOfTuples();
    int idx[3] = {0, 4, 8};
    float *s = (float*)strain->GetVoidPointer(0);
    float *c;
    for (i = 0; i < 3; i++)
    {
        if (!np[i]) // this crack direction is not needed
            continue;

        np[i] = false;
        int comp = idx[i];

        for (int j = 0; j < nc && !np[i]; j++)
        {
            if (s[9*j+comp] != 0)
                np[i] = true;
        }
    }

    return np[0] || np[1] || np[2];
}


// ****************************************************************************
//  Method: avtCracksClipperFilter::RemoveCracks
//
//  Purpose:
//    Does the actual work of removing 'cracks' from the dataset.  Uses
//    vtkCracksClipper. 
//
//  Arguments:
//    inds         The input dataset.
//    whichCrack   Wich crach direction should be removed.
//
//  Returns:    The cracks-clipped dataset.
//
//  Programmer: Kathleen Bonnell
//  Creation:   Thu Oct 13 08:17:36 PDT 2005
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtCracksClipperFilter::RemoveCracks(vtkDataSet *inds, int whichCrack)
{
    char *crackvar;
    char *crackwidth;
    char *centers = "avtCellCenters";

    //
    // Retrieve the correct variable names to be sent to the cracks clipper
    //
    if (whichCrack == 0)
    {
        crackvar = const_cast<char*>(atts.GetCrack1Var().c_str());
        crackwidth = "avtCrack1Width";
    }
    else if (whichCrack == 1)
    {
        crackvar = const_cast<char*>(atts.GetCrack2Var().c_str());
        crackwidth = "avtCrack2Width";
    }
    else // if (whichCrack == 2)
    {
        crackvar = const_cast<char*>(atts.GetCrack3Var().c_str());
        crackwidth = "avtCrack3Width";
    }


    //
    //  Requires two clips in order to remove the 'crack'.
    //
    vtkDataSet *outds1 = vtkUnstructuredGrid::New();

    vtkCracksClipper *posClip = vtkCracksClipper::New();
    posClip->SetUseOppositePlane(false);
    posClip->SetInput(inds);
    posClip->SetCrackDir(crackvar);
    posClip->SetCrackWidth(crackwidth);
    posClip->SetCellCenters(centers);
    posClip->SetOutput((vtkUnstructuredGrid*)outds1);
    posClip->Update();
    posClip->Delete();

    if (outds1->GetNumberOfCells() <= 0)
    {
        outds1->Delete();
        outds1 = NULL;
    }

    vtkDataSet *outds2 = vtkUnstructuredGrid::New();

    vtkCracksClipper *negClip = vtkCracksClipper::New();
    negClip->SetUseOppositePlane(true);
    negClip->SetInput(inds);
    negClip->SetCrackDir(crackvar);
    negClip->SetCrackWidth(crackwidth);
    negClip->SetCellCenters(centers);
    negClip->SetOutput((vtkUnstructuredGrid*)outds2);
    negClip->Update();
    negClip->Delete();

    if (outds2->GetNumberOfCells() <= 0)
    {
       outds2->Delete();
       outds2 = NULL;
    }
 
    if (outds1 == NULL)
    {
        return outds2; 
    } 
    if (outds2 == NULL)
    {
        return outds1; 
    } 
    vtkAppendFilter *append = vtkAppendFilter::New();
    append->AddInput(outds1);
    append->AddInput(outds2);
    append->GetOutput()->Update();

    vtkDataSet *ds = outds1->NewInstance();
    ds->ShallowCopy(append->GetOutput());
    outds1->Delete();
    outds2->Delete();
    append->Delete();
    return ds;
}


// ****************************************************************************
//  Method: avtCracksClipperFilter::RemoveExtraArrays
//
//  Purpose:
//    Removes from the passed dataset any arrays added for the processing of 
//    this filter. 
//
//  Arguments:
//    ds        The dataset.
//    all       A flag indicating whether or not to remove arrays added
//              by the CrackWidth filter. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   Thu Oct 13 08:17:36 PDT 2005
//
//  Modifications:
//
// ****************************************************************************

void
avtCracksClipperFilter::RemoveExtraArrays(vtkDataSet *ds, bool all)
{
    // Secondary variables requested from the db
    ds->GetCellData()->RemoveArray((atts.GetCrack1Var().c_str()));
    ds->GetCellData()->RemoveArray((atts.GetCrack2Var().c_str()));
    ds->GetCellData()->RemoveArray((atts.GetCrack3Var().c_str()));
    ds->GetCellData()->RemoveArray((atts.GetStrainVar().c_str()));
    if (all)
    { 
        // Variables created by CrackWidth filter.
        ds->GetCellData()->RemoveArray("avtCellCenters");
        ds->GetCellData()->RemoveArray("avtCrack1Width");
        ds->GetCellData()->RemoveArray("avtCrack2Width");
        ds->GetCellData()->RemoveArray("avtCrack3Width");
    } 
}


