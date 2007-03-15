/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//  File: avtCracksClipperFilter.C
// ************************************************************************* //

#include <avtCracksClipperFilter.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkCracksClipper.h>
#include <vtkCrackWidthFilter.h>
#include <vtkExtractCells.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>
#include <vtkAppendFilter.h>
#include <vtkVisItUtility.h>

#include <Expression.h>
#include <ExpressionList.h>
#include <ParsingExprList.h>

#include <DebugStream.h>
#include <InvalidVariableException.h>


// ****************************************************************************
//  Method: GetCrackVar 
//
//  Purpose:  Convenience method to return a crack variable.
//
//  Programmer: Kathleen Bonnell
//  Creation:   Fri Oct 13 11:05:01 PDT 2006
//
// ****************************************************************************

char * 
GetCrackVar(int which, CracksClipperAttributes *a)
{
    if (0 == which) 
        return const_cast<char*>(a->GetCrack1Var().c_str());
    else if (1 == which) 
        return const_cast<char*>(a->GetCrack2Var().c_str());
    else 
        return const_cast<char*>(a->GetCrack3Var().c_str());
}


// ****************************************************************************
//  Method: GetShowCrack 
//
//  Purpose:  Convenience method to return the value of ShowCrack.
//
//  Programmer: Kathleen Bonnell
//  Creation:   Fri Oct 13 11:05:01 PDT 2006
//
// ****************************************************************************

bool  
GetShowCrack(int which, CracksClipperAttributes *a)
{
    if (0 == which) 
        return a->GetShowCrack1();
    else if (1 == which) 
        return a->GetShowCrack2();
    else 
        return a->GetShowCrack3();
}

// ****************************************************************************
//  Method: GetCrackWidth 
//
//  Purpose:  Convenience method to return the crack width.
//
//  Programmer: Kathleen Bonnell
//  Creation:   Fri Oct 13 11:05:01 PDT 2006
//
// ****************************************************************************

char * 
GetCrackWidth(int which)
{
    if (0 == which) 
        return "avtCrack1Width";
    else if (1 == which) 
        return "avtCrack2Width";
    else 
        return "avtCrack3Width";
}


// ****************************************************************************
//  Method: GetCrackWidth 
//
//  Purpose:  Convenience method to create max-to-min odering of delta. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   Fri Oct 13 11:05:01 PDT 2006
//
// ****************************************************************************

void 
OrderThem2(double delta[3], int co[3])
{
  int min, mid, max;
  if (delta[0] <= delta[1] && delta[0] <= delta[2])
    min = 0; 
  else if (delta[1] <= delta[0] && delta[1] <= delta[2])
    min = 1; 
  else 
    min = 2; 

  if (delta[0] >= delta[1] && delta[0] >= delta[2])
    max = 0; 
  else if (delta[1] >= delta[0] && delta[1] >= delta[2])
    max = 1; 
  else 
    max = 2; 

  if (min == 0)
    mid = (max == 1 ? 2 : 1);    
  else if (min == 1)
    mid = (max == 2 ? 0 : 2);    
  else 
    mid = (max == 0 ? 1 : 0);    

  co[0] = max;
  co[1] = mid;
  co[2] = min;
}


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
//    Kathleen Bonnell, Fri Oct 13 11:05:01 PDT 2006
//    RemoveCracks has been restructured, only called once now.
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
    cwf->SetCrack1Var(atts.GetCrack1Var().c_str());
    cwf->SetCrack2Var(atts.GetCrack2Var().c_str());
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

    vtkDataSet *output = RemoveCracks(cwf->GetOutput());
    if (output->GetNumberOfCells() <= 0)
    {
        output->Delete();
        output = NULL;
        rv = NULL;
    }
    else 
    {
        RemoveExtraArrays(output, true);
        rv = output;
        ManageMemory(output);
        output->Delete();
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
//    Kathleen Bonnell, Fri Oct 13 11:05:01 PDT 2006
//    Always request the secondary vars.  Added cracks_vol expression to
//    secondary var list.
//
//    Kathleen Bonnell, Wed Jan 10 13:30:40 PST 2007 
//    Ensure any existing secondary variables get added to the new DataSpec. 
//
// ****************************************************************************

avtPipelineSpecification_p
avtCracksClipperFilter::PerformRestriction(avtPipelineSpecification_p pspec)
{
    avtDataSpecification_p ds = pspec->GetDataSpecification();

    // Retrieve secondary variables, if any, to pass along to the 
    // newly created DataSpec
    std::vector<CharStrRef> csv = ds->GetSecondaryVariables();

    // Create a new dspec so that we can add secondary vars
    avtDataSpecification_p nds = new avtDataSpecification(ds->GetVariable(),
                ds->GetTimestep(), ds->GetRestriction());

    // Add any previously existing SecondaryVariables.
    for (int i = 0; i < csv.size(); i++)
        nds->AddSecondaryVariable(*(csv[i]));

    // Add secondary variables necessary for CracksClipper
    nds->AddSecondaryVariable(atts.GetCrack1Var().c_str());
    nds->AddSecondaryVariable(atts.GetCrack2Var().c_str());
    nds->AddSecondaryVariable(atts.GetCrack3Var().c_str());

    avtDataAttributes &data = GetInput()->GetInfo().GetAttributes();

    ExpressionList *elist = ParsingExprList::Instance()->GetList();
    Expression *e = new Expression();

    string edef = string("volume2(<") + data.GetMeshname() + string(">)");
    e->SetName("cracks_vol");
    e->SetDefinition(edef.c_str());
    e->SetType(Expression::ScalarMeshVar);
    elist->AddExpressions(*e);
    delete e;

    nds->AddSecondaryVariable("cracks_vol");

    nds->AddSecondaryVariable(atts.GetStrainVar().c_str());
    avtPipelineSpecification_p rv = new avtPipelineSpecification(pspec, nds);

    //
    // Since this filter 'clips' the dataset, the zone and possibly
    // node numbers will be invalid, request them when needed.
    //
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
//    Kathleen Bonnell, Fri Oct 13 11:05:01 PDT 2006
//    Reflect atts api change.
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
    if (atts.GetShowCrack1())
    {
        cracks[0] = ds->GetCellData()->GetArray(atts.GetCrack1Var().c_str());
        if (cracks[0] == NULL)
        {
            EXCEPTION1(InvalidVariableException, atts.GetCrack1Var().c_str());
        } 
        np[0] = true;
    }
    if (atts.GetShowCrack2())
    {
        cracks[1] = ds->GetCellData()->GetArray(atts.GetCrack2Var().c_str());
        if (cracks[1] == NULL)
        {
            EXCEPTION1(InvalidVariableException, atts.GetCrack2Var().c_str());
        } 
        np[1] = true;
    }
    if (atts.GetShowCrack3())
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
//    Kathleen Bonnell, Thu Oct 12 16:04:38 PDT 2006
//    Clip on a cell-by-cell basis, considering crack size and clipping
//    with largest crack direction first in all cases.
//
// ****************************************************************************

vtkDataSet *
avtCracksClipperFilter::RemoveCracks(vtkDataSet *inds)
{
    vtkFloatArray *strain = (vtkFloatArray*)inds->GetCellData()->
                             GetArray(atts.GetStrainVar().c_str());

    int nc = inds->GetNumberOfCells(); 
    char *centers = "avtCellCenters";
    char *crackvar;
    char *crackwidth;
    vtkAppendFilter *apd = vtkAppendFilter::New();

    vtkDataSet *dsToUse = inds->NewInstance();
    dsToUse->ShallowCopy(inds);
    double delta[3];
    int cellId;
    int processedCell[3] = {0, 0, 0};
    vtkIdList *cellsToKeepIntact = vtkIdList::New();
                       
    for (int i = 0; i < nc; i++)
    {
        cellId = i;
        delta[0] = strain->GetComponent(i, 0);
        delta[1] = strain->GetComponent(i, 4);
        delta[2] = strain->GetComponent(i, 8);
    
        if (delta[0] == 0. && delta[1] == 0. && delta[2] == 0.)
        {
            cellsToKeepIntact->InsertNextId(cellId);
            continue;
        }

        
        int crackOrder[3] = {0, 1, 2};
        OrderThem2(delta, crackOrder);
        bool first = true;
        for (int j = 0; j < 3; j++)
        {
            int whichCrack = crackOrder[j];
            processedCell[whichCrack] = 0;
            if (!GetShowCrack(whichCrack, &atts) || delta[whichCrack] == 0)
            {
                continue;
            }
            crackvar = GetCrackVar(whichCrack, &atts);
            crackwidth = GetCrackWidth(whichCrack);

            if (first)
                dsToUse->ShallowCopy(inds);

            //
            //  Requires two clips in order to remove the 'crack'.
            //
            vtkDataSet *outds1 = vtkUnstructuredGrid::New();

            vtkCracksClipper *posClip = vtkCracksClipper::New();
            if (first)
                posClip->SetCellList(&cellId, 1);
            posClip->SetUseOppositePlane(false);
            posClip->SetInput(dsToUse);
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
            if (first)
                negClip->SetCellList(&cellId, 1);
            negClip->SetUseOppositePlane(true);
            negClip->SetInput(dsToUse);
            negClip->SetCrackDir(crackvar);
            negClip->SetCrackWidth(crackwidth);
            negClip->SetCellCenters(centers);
            negClip->SetOutput((vtkUnstructuredGrid*)outds2);
            negClip->Update();
            negClip->Delete();

            first = false;
            if (outds2->GetNumberOfCells() <= 0)
            {
               outds2->Delete();
               outds2 = NULL;
            }
 
            if (outds1 == NULL && outds2 == NULL)
            {
                processedCell[whichCrack] = 1;
                continue;
            } 
            processedCell[whichCrack] = 2;
            if (outds1 == NULL)
            {
                dsToUse->ShallowCopy(outds2);
                outds2->Delete();
            } 
            else if (outds2 == NULL)
            {
                dsToUse->ShallowCopy(outds1);
                outds1->Delete();
            } 
            else
            {
                vtkAppendFilter *append = vtkAppendFilter::New();
                append->AddInput(outds1);
                append->AddInput(outds2);
                append->GetOutput()->Update();
                dsToUse->ShallowCopy(append->GetOutput());
                outds1->Delete();
                outds2->Delete();
                append->Delete();
            }
        }
        if (processedCell[0] == 0 && 
            processedCell[1] == 0 && processedCell[2] == 0)
        {
            cellsToKeepIntact->InsertNextId(cellId);
            continue;
        }
        else if (!(processedCell[0] == 1 &&
                   processedCell[1] == 1 &&
                   processedCell[2] == 1))
        {
            if (i % 5 == 0 && apd->GetTotalNumberOfInputConnections() > 1)
            {
                apd->GetOutput()->Update();
                dsToUse->ShallowCopy(apd->GetOutput());
                apd->Delete();
                apd = vtkAppendFilter::New();
            }
            apd->AddInput(dsToUse);
        }
    }
    if (nc == cellsToKeepIntact->GetNumberOfIds())
    {
        dsToUse->Delete();
        cellsToKeepIntact->Delete();
        apd->Delete();
        return inds;
    }
    vtkExtractCells *extract = vtkExtractCells::New(); 
    if (cellsToKeepIntact->GetNumberOfIds() > 0)
    {
        extract->SetInput(inds);
        extract->SetCellList(cellsToKeepIntact);
        apd->AddInput(extract->GetOutput());
    }
    
    apd->GetOutput()->Update();
    vtkDataSet *ds = apd->GetOutput()->NewInstance();
    ds->ShallowCopy(apd->GetOutput());
    apd->Delete();
    extract->Delete();
    cellsToKeepIntact->Delete();
    dsToUse->Delete();
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
//    Kathleen Bonnell, Fri Oct 13 11:05:01 PDT 2006
//    Remove cracks_vol var.
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
        ds->GetCellData()->RemoveArray("cracks_vol");
    } 
}
