/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//  File: avtRemoveCracksFilter.C
// ************************************************************************* //

#include <avtRemoveCracksFilter.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkCracksClipper.h>
#include <vtkCrackWidthFilter.h>
#include <vtkExtractCells.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>
#include <vtkAppendFilter.h>
#include <vtkVisItUtility.h>

#include <avtCallback.h>
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
//  Modifications:
//    Jeremy Meredith, Thu Aug  7 15:33:06 EDT 2008
//    Made the function return a const char* so we don't need the
//    const cast.
//
// ****************************************************************************

const char * 
GetCrackVar(int which, CracksClipperAttributes *a)
{
    if (0 == which) 
        return a->GetCrack1Var().c_str();
    else if (1 == which) 
        return a->GetCrack2Var().c_str();
    else 
        return a->GetCrack3Var().c_str();
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
//  Modifications:
//    Jeremy Meredith, Thu Aug  7 15:30:07 EDT 2008
//    Since we're returning string literals, make them be const char*.
//
// ****************************************************************************

const char * 
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
//  Method: avtRemoveCracksFilter constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   Thu Oct 13 08:17:36 PDT 2005
//
// ****************************************************************************

avtRemoveCracksFilter::avtRemoveCracksFilter()
{
}


// ****************************************************************************
//  Method: avtRemoveCracksFilter destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   Thu Oct 13 08:17:36 PDT 2005
//
//  Modifications:
//
// ****************************************************************************

avtRemoveCracksFilter::~avtRemoveCracksFilter()
{
}


// ****************************************************************************
//  Method:  avtRemoveCracksFilter::Create
//
//  Programmer: Kathleen Bonnell
//  Creation:   Thu Oct 13 08:17:36 PDT 2005
//
// ****************************************************************************

avtFilter *
avtRemoveCracksFilter::Create()
{
    return new avtRemoveCracksFilter();
}


// ****************************************************************************
//  Method:      avtRemoveCracksFilter::SetAtts
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
avtRemoveCracksFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const CracksClipperAttributes*)a;
}


// ****************************************************************************
//  Method: avtRemoveCracksFilter::Equivalent
//
//  Purpose:
//    Returns true if creating a new avtRemoveCracksFilter with the given
//    parameters would result in an equivalent avtRemoveCracksFilter.
//
//  Programmer: Kathleen Bonnell
//  Creation:   Thu Oct 13 08:17:36 PDT 2005
//
// ****************************************************************************

bool
avtRemoveCracksFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(CracksClipperAttributes*)a);
}


// ****************************************************************************
//  Method: avtRemoveCracksFilter::ExecuteData
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
avtRemoveCracksFilter::ExecuteData(vtkDataSet *in_ds, int dom, std::string)
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
    int nc = in_ds->GetNumberOfCells();

    vtkIntArray *numOCells = vtkIntArray::New();
    numOCells->SetNumberOfComponents(1);
    numOCells->SetNumberOfTuples(1);
    numOCells->SetValue(0, nc); 
    numOCells->SetName("originalNumCells");

    defaultReturn->GetFieldData()->AddArray(numOCells);

    vtkIntArray *oz = vtkIntArray::New();
    oz->SetNumberOfComponents(1);
    oz->SetNumberOfTuples(nc);
    oz->SetName("cracksOriginalZones");

    for (int i = 0; i < nc; i++)
        oz->SetValue(i, i);

    defaultReturn->GetCellData()->AddArray(oz);

    vtkDataSet *useThis = in_ds->NewInstance();
    useThis->ShallowCopy(in_ds);
    useThis->GetCellData()->AddArray(oz);

    //
    // Does this domain need to be processed?
    //
    bool needsProc[3];
    if (!NeedsProcessing(useThis, needsProc))
    {
        rv = defaultReturn;
        ManageMemory(defaultReturn);
        defaultReturn->Delete();
        numOCells->Delete();
        oz->Delete();
        useThis->Delete();
        return rv;
    }

    //
    // Calculate the crack widths and cell centers needed by the 
    // cracks clip filter
    //
    vtkCrackWidthFilter *cwf = vtkCrackWidthFilter::New();
    cwf->SetInput(useThis); 
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
        debug5 << "avtRemoveCracksFilter not processing domain " << dom
               << "  because all crack widths are zero" << endl;
        cwf->Delete();
        rv = defaultReturn;
        ManageMemory(defaultReturn);
        defaultReturn->Delete();
        numOCells->Delete();
        oz->Delete();
        useThis->Delete();
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
    rv->GetFieldData()->AddArray(numOCells);

    cwf->Delete();
    useThis->Delete();
    oz->Delete();
    numOCells->Delete();
    defaultReturn->Delete();

    return rv;
}



// ****************************************************************************
//  Method: avtRemoveCracksFilter::NeedsProcessing
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
//    Kathleen Bonnell, Tue Jul 1 15:09:54 PDT 2008 
//    Removed unreferenced variable.
//
// ****************************************************************************

bool
avtRemoveCracksFilter::NeedsProcessing(vtkDataSet *ds, bool *np)
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
//  Method: avtRemoveCracksFilter::RemoveCracks
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
//    Jeremy Meredith, Thu Aug  7 15:30:34 EDT 2008
//    Use const char* for string literals.
//
// ****************************************************************************

vtkDataSet *
avtRemoveCracksFilter::RemoveCracks(vtkDataSet *inds)
{
    vtkFloatArray *strain = (vtkFloatArray*)inds->GetCellData()->
                             GetArray(atts.GetStrainVar().c_str());

    int nc = inds->GetNumberOfCells(); 
    const char *centers = "avtCellCenters";
    const char *crackvar;
    const char *crackwidth;
    vtkAppendFilter *apd = vtkAppendFilter::New();

    vtkDataSet *dsToUse = inds->NewInstance();
    dsToUse->ShallowCopy(inds);
    double delta[3];
    int i, cellId;
    int processedCell[3] = {0, 0, 0};
    vtkIdList *cellsToKeepIntact = vtkIdList::New();

    vtkDataSet *apdInput[5];
    for (i = 0; i < 5; i++)
        apdInput[i] = dsToUse->NewInstance();
    int apdInputNum = 0;
                           
    for (i = 0; i < nc; i++)
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
                apdInput[apdInputNum]->ShallowCopy(outds2);
                outds2->Delete();
            } 
            else if (outds2 == NULL)
            {
                dsToUse->ShallowCopy(outds1);
                apdInput[apdInputNum]->ShallowCopy(outds1);
                outds1->Delete();
            } 
            else
            {
                vtkAppendFilter *append = vtkAppendFilter::New();
                append->AddInput(outds1);
                append->AddInput(outds2);
                append->GetOutput()->Update();
                dsToUse->ShallowCopy(append->GetOutput());
                apdInput[apdInputNum]->ShallowCopy(append->GetOutput());
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
        else 
        {
            if (apdInputNum == 4)
            {
                apd->AddInput(apdInput[apdInputNum]);
                apd->GetOutput()->Update();
                apdInputNum = 0;
                apdInput[apdInputNum]->ShallowCopy(apd->GetOutput());
                apd->Delete();
                apd = vtkAppendFilter::New();
                avtCallback::ResetTimeout(60*5);
            }
            apd->AddInput(apdInput[apdInputNum]);
            apdInputNum++;
        }
    }
    if (nc == cellsToKeepIntact->GetNumberOfIds())
    {
        cellsToKeepIntact->Delete();
        apd->Delete();
        dsToUse->ShallowCopy(inds);
        return dsToUse;
    }
    vtkExtractCells *extract = vtkExtractCells::New(); 
    if (cellsToKeepIntact->GetNumberOfIds() > 0)
    {
        vtkDataSet *temp = inds->NewInstance();
        temp->ShallowCopy(inds);
        extract->SetInput(temp);
        extract->SetCellList(cellsToKeepIntact);
        apd->AddInput(extract->GetOutput());
        temp->Delete();
    }
    
    apd->GetOutput()->Update();
    vtkDataSet *ds = apd->GetOutput()->NewInstance();
    ds->ShallowCopy(apd->GetOutput());

    apd->Delete();
    cellsToKeepIntact->Delete();
    extract->Delete();
    dsToUse->Delete();
    return ds;
}


// ****************************************************************************
//  Method: avtRemoveCracksFilter::RemoveExtraArrays
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
avtRemoveCracksFilter::RemoveExtraArrays(vtkDataSet *ds, bool all)
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
