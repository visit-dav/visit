/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
//                             avtLineScanQuery.C                            //
// ************************************************************************* //

#include <avtLineScanQuery.h>

#include <stdio.h>
#include <math.h>

#include <vtkCellData.h>
#include <vtkCleanPolyData.h>
#include <vtkExecutive.h>
#include <vtkIdList.h>
#include <vtkPolyData.h>

#include <avtCallback.h>
#include <avtLineScanFilter.h>
#include <avtParallel.h>
#include <avtSourceFromAVTDataset.h>
#include <avtTerminatingSource.h>
#include <avtWeightedVariableSummationQuery.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtLineScanQuery constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   July 20, 2006
//
// ****************************************************************************

avtLineScanQuery::avtLineScanQuery()
{
    numBins   = 100;
    numLines  = 1000;
    minLength = 0.;
    maxLength = 1.0;
    numLinesPerIteration = 10000;
}


// ****************************************************************************
//  Method: avtLineScanQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   July 20, 2006
//
// ****************************************************************************

avtLineScanQuery::~avtLineScanQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtLineScanQuery::PreExecute
//
//  Purpose:
//      Does some initialization work before the query executes.
//
//  Programmer: Hank Childs
//  Creation:   July 20, 2006
//
// ****************************************************************************

void
avtLineScanQuery::PreExecute(void)
{
    if (numBins <= 0)
    {
        EXCEPTION1(VisItException, "There must be at least one bin.");
    }
    if (numLines <= 0)
    {
        EXCEPTION1(VisItException, "There must be at least one line.");
    }
    if (minLength < 0 || maxLength <= minLength)
    {
        EXCEPTION1(VisItException, "The min length must be less than the max "
                        "length and they both must be positive.");
    }
}


// ****************************************************************************
//  Method: avtLineScanQuery::GetPointsForCell
//
//  Purpose:
//      Gets the points for a cell.  This uses the avtLineID array to figure
//      out which cells are incident to a point (there may be more than 
//      two cells for a given point, but those cells are probably not all from
//      the line ID.
//
//  Programmer: Hank Childs
//  Creation:   July 20, 2006
//
// ****************************************************************************

int
avtLineScanQuery::GetCellsForPoint(int ptId, vtkPolyData *pd, 
                                   vtkIntArray *lineids, int lineid,
                                   int &seg1, int &seg2)
{
    static vtkIdList *list = vtkIdList::New();
    pd->GetPointCells(ptId, list);
    int numMatches = 0;
    int workingLineid = lineid;
    for (int i = 0 ; i < list->GetNumberOfIds() ; i++)
    {
        int curId = list->GetId(i);
        if (workingLineid >= 0 && lineids->GetValue(curId) != workingLineid)
            continue;  // This is a separate line going through the same point
        if (pd->GetCellType(curId) == VTK_VERTEX)
            continue;  // This will only screw up our logic.
        if (numMatches == 0)
        {
            seg1 = curId;
            if (workingLineid < 0)
                workingLineid = lineids->GetValue(seg1);
            numMatches++;
        }
        else if (numMatches == 1)
        {
            seg2 = curId;
            numMatches++;
        }
        else 
        {
            // This is an error condition.  It is believed to occur when
            // a line coincides with an edge.  Empirically, it is believed
            // to happen about one time when you cast 100K lines over 90M
            // zones.  So: it doesn't happen often, but it happens enough.
            // In this case, just ignoring the line won't affect statistics.
            return 3;
        }
    }

    return numMatches;
}


// ****************************************************************************
//  Method: avtLineScanQuery::WalkChain
//
//  Purpose:
//      A helper function that is called recursively to determine where the
//      start and end of a poly-line lies.
//
//  Programmer: Hank Childs
//  Creation:   July 20, 2006
//
// ****************************************************************************

int 
avtLineScanQuery::WalkChain(vtkPolyData *pd, int ptId, int cellId, 
                            std::vector<bool> &usedPoint,
                            vtkIntArray *lineids, int lineid)
{
    static vtkIdList *list = vtkIdList::New();

    bool haventFoundEnd = true;
    int  curCell = cellId;
    int  curPt   = ptId;

    int  endOfChain = -1;
    int  counter = 0;
    while (haventFoundEnd)
    {
        list->Reset();
        pd->GetCellPoints(curCell, list);
        if (list->GetNumberOfIds() != 2)
        {
            EXCEPTION0(ImproperUseException);
        }

        int id1 = list->GetId(0);
        int id2 = list->GetId(1);
        int newId = (id1 == curPt ? id2 : id1);
        usedPoint[newId] = true;

        int seg1, seg2;
        int numMatches = 
                      GetCellsForPoint(newId, pd, lineids, lineid, seg1, seg2);
        if (numMatches <= 1)
        {
            haventFoundEnd = false;
            endOfChain = newId;
        }
        else if (numMatches > 2)
        {
            // This is an error condition.  It is believed to occur when
            // a line coincides with an edge.  Empirically, it is believed
            // to happen about one time when you cast 100K lines over 90M
            // zones.  So: it doesn't happen often, but it happens enough.
            // In this case, just ignoring the line won't affect statistics.
            haventFoundEnd = false;
            endOfChain = -1;
        }
        else
        {
            curPt = newId;
            curCell = (seg1 == curCell ? seg2 : seg1);
        }
        if (counter++ > 1000000)
        {
            haventFoundEnd = false;
            endOfChain = -1;
        }
    }

    return endOfChain;
}


// ****************************************************************************
//  Method: avtLineScanQuery::WalkChain1
//
//  Purpose:
//      A helper function that is called recursively to determine where the
//      start and end of a poly-line lies.  This variant is similar to 
//      WalkChain above, but it returns every point on the line.
//
//  Programmer: Hank Childs
//  Creation:   August 2, 2006
//
// ****************************************************************************

void
avtLineScanQuery::WalkChain1(vtkPolyData *pd, int ptId, int cellId,
                             vtkIntArray *lineids, int lineid, 
                             int &newPtId, int &newCellId)
{
    static vtkIdList *list = vtkIdList::New();
    list->Reset();
    pd->GetCellPoints(cellId, list);
    if (list->GetNumberOfIds() != 2)
    {
        EXCEPTION0(ImproperUseException);
    }

    int id1 = list->GetId(0);
    int id2 = list->GetId(1);
    newPtId = (id1 == ptId ? id2 : id1);
    int seg1, seg2;
    int numMatches = GetCellsForPoint(newPtId, pd, lineids, lineid, seg1, seg2);
    if (numMatches <= 1)
        newCellId = -1;
    else if (numMatches > 2)
    {
        EXCEPTION0(ImproperUseException);
    }
    else
    {
        newCellId = (seg1 == cellId ? seg2 : seg1);
    }
}


// ****************************************************************************
//  Method: avtLineScanQuery::Execute
//
//  Purpose:
//      Examines the input data.  Note that the line scan filter will organize
//      the data so that all of the contributions from a given line will be in
//      the same vtkPolyData input.
//
//  Programmer: Hank Childs
//  Creation:   July 20, 2006
//
// ****************************************************************************

void
avtLineScanQuery::Execute(vtkDataSet *ds, const int chunk)
{
    if (ds->GetDataObjectType() != VTK_POLY_DATA)
    {
        EXCEPTION0(ImproperUseException);
    }

    int extraMsg = 100;
    int totalProg = totalNodes * extraMsg;
    UpdateProgress(extraMsg*currentNode, totalProg);

    vtkPolyData *pd = (vtkPolyData *) ds;
    vtkCleanPolyData *cpd = vtkCleanPolyData::New();
    cpd->SetToleranceIsAbsolute(0);
    cpd->SetTolerance(1e-7);
    cpd->SetInput(pd);
    vtkPolyData *output = cpd->GetOutput();
    output->Update();
    UpdateProgress(extraMsg*currentNode+extraMsg/3, totalProg);

    ExecuteLineScan(output);

    cpd->Delete();
}


// ****************************************************************************
//  Method: avtLineScanQuery::Execute
//
//  Purpose:
//      Applies the line scan filter to the data set and then has the
//      derived type's method interrogate the resulting lines.  Because
//      casting too many lines can overflow memory, it will execute the
//      line scan filter multiple times with smaller numbers of lines each
//      time.
//
//  Programmer: Hank Childs
//  Creation:   July 20, 2006
//
//  Modifications:
//
//    Hank Childs, Tue Sep  5 11:40:33 PDT 2006
//    Reset the time out so that queries can take longer than ten minutes.
//
// ****************************************************************************

void
avtLineScanQuery::Execute(avtDataTree_p tree)
{
    avtDataset_p input = GetTypedInput();

    int numPasses = numLines / numLinesPerIteration;
    if (numLines % numLinesPerIteration != 0)
        numPasses++;

    avtPipelineSpecification_p pspec =
        input->GetTerminatingSource()->GetGeneralPipelineSpecification();
    varname = pspec->GetDataSpecification()->GetVariable();
    for (int i = 0 ; i < numPasses ; i++)
    {
        int numForLast = (numLines % numLinesPerIteration);
        numForLast = (numForLast == 0 ? numLinesPerIteration : numForLast);
        int linesForThisPass = (i == numPasses-1  ? numForLast 
                                                  : numLinesPerIteration);

        //
        // Create an artificial pipeline.
        //
        avtDataset_p ds;
        CopyTo(ds, input);
        avtSourceFromAVTDataset termsrc(ds);
        avtDataObject_p dob = termsrc.GetOutput();
    
        avtLineScanFilter filt;
        filt.SetNumberOfLines(linesForThisPass);
        filt.SetRandomSeed(i);
        filt.SetInput(dob);

        //
        // Cause our artificial pipeline to execute.
        //
        filt.GetOutput()->Update(pspec);

        avtDataset_p ds2 = filt.GetTypedOutput();
        avtDataTree_p tree = ds2->GetDataTree();
        ExecuteTree(tree);

        //
        // Reset the timeout for the next iteration.
        //
        avtCallback::ResetTimeout(60*5);
    }
}


// ****************************************************************************
//  Method: avtLineScanQuery::GetNFilters
//
//  Purpose:
//      Declares how many filters we have in the pipeline.  This is important
//      for progress.
//
//  Programmer: Hank Childs
//  Creation:   August 2, 2006
//
// ****************************************************************************

int
avtLineScanQuery::GetNFilters(void)
{
    int numPasses = numLines / numLinesPerIteration;
    if (numLines % numLinesPerIteration != 0)
        numPasses++;
    return 2*numPasses;
}


// ****************************************************************************
//  Method: avtLineScanQuery::ExecuteTree
//
//  Purpose:
//      The mechanism for executing over a tree.
//
//  Programmer: Hank Childs
//  Creation:   August 2, 2006
//
// ****************************************************************************

void
avtLineScanQuery::ExecuteTree(avtDataTree_p inDT)
{
    if (*inDT == NULL)
    {
        return;
    }

    int nc = inDT->GetNChildren();

    if (nc <= 0 && !inDT->HasData())
    {
        return;
    }

    if ( nc == 0 )
    {
        //
        // There is only one dataset to process.
        //
        vtkDataSet *in_ds = inDT->GetDataRepresentation().GetDataVTK();
        int dom = inDT->GetDataRepresentation().GetDomain();

        //
        // Setting the source to NULL for the input will break the
        // pipeline.
        //
        // NO LONGER A GOOD IDEA
        //in_ds->SetSource(NULL);
        Execute(in_ds, dom);
        if (!timeVarying)
        {
            currentNode++;
            UpdateProgress(currentNode, totalNodes);
        }
    }
    else
    {
        //
        // There is more than one input dataset to process.
        // Recurse through the children.
        //
        for (int j = 0; j < nc; j++)
        {
            if (inDT->ChildIsPresent(j))
            {
                Execute(inDT->GetChild(j));
            }
        }
    }
}


