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

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkCleanPolyData.h>
#include <vtkExecutive.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
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
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 11:55:03 EST 2007
//    Call inherited PreExecute before everything else.
//
// ****************************************************************************

void
avtLineScanQuery::PreExecute(void)
{
    avtDatasetQuery::PreExecute();

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
//  Function: MergeSegmentPoints
//
//  Purpose:
//      Finds points that lie are the same line segment that are some
//      epsilon apart and merges them to be one point. 
//
//  Arguments:
//      input       The input poly data to merge.
//      varname     The name of the variable that says which line scan a
//                  segment comes from.
//      tolerance   The tolerance to use when merging.
// 
//  Returns:  A new vtkPolyData.  The calling function must free this.
//
//  Note:    This could not be accomplished by using vtkCleanPolyData,
//           since it has no functionality for differing line segment.
//           By way of example, assume that two distinct scan lines have
//           a line segment that ends (and begins) near point P.  Explicitly,
//           line scan 1 has line segment A, which has an endpoint near 
//           point P, and line segment B which also has an endpoint near point
//           P.  Similarly, line scan 2 has line segments C and D which also
//           have endpoints near point P.  So: the points from C and D should
//           be merged.  And the points from A and B should be merged.  But
//           the merged point A and B should *not* be merged with the merged
//           point from C and D.  If they are, then the results change in
//           parallel, because, with a different number of processors, line
//           scan 1 and 2 may be on different processors.  And the merged 
//           point may pull each segment off the path of the original line
//           scan slightly, changing the answer.
//
//  Programmer: Hank Childs
//  Creation:   January 19, 2006
//
// ****************************************************************************

typedef struct
{
    int lineId;
    int ptId;
    double radSquared;
}  IdPoint;

static int
IdPointSorter(const void *arg1, const void *arg2)
{
    const IdPoint *r1 = (const IdPoint *) arg1;
    const IdPoint *r2 = (const IdPoint *) arg2;

    if (r1->lineId > r2->lineId)
        return 1;
    else if (r1->lineId < r2->lineId)
        return -1;

    if (r1->radSquared > r2->radSquared)
        return 1;
    else if (r1->radSquared < r2->radSquared)
        return -1;

    return 0;
}


vtkPolyData *
MergeSegmentPoints(vtkPolyData *input, const char *varname, double tolerance)
{
    int   i, j;

    vtkIntArray *line_id = (vtkIntArray *) 
                                        input->GetCellData()->GetArray(varname);
    if (line_id == NULL)
    {
        if (input->GetNumberOfCells() == 0)
        {
            input->Register(NULL);
            return NULL;
        }

        EXCEPTION0(ImproperUseException);
    }

    int ncells = input->GetNumberOfCells();
    int npts = input->GetNumberOfPoints();
    int *newPtId = new int[npts];
    int numNewPts = 0;

    //
    // Determine which points can be merged.  Here's the game plan.  We can
    // only merge points if they have the same line id and they are within
    // "tolerance" of each other.  We will construct a big array to qsort.
    // An entry in the array will be of the form (lineID, X, Y, Z).  The 
    // sorting returned by qsort will sort first by lineID.  Then it will
    // sort the rest of the entries by distance to the origin.  
    //
    // After this sort is made, we can pick off the repeats pretty easily.
    // We check to see if the lineID is the same.  If so, we check all of
    // the point nearby in the array.  The only points that can be nearby
    // are those that have similar distance to the origin.  Of course, some
    // points will be in dramatically different locations, but we can eliminate
    // those quickly.  When we find a match, we update newPtId.  The code at
    // the bottom of this method will construct the new vtkPolyData.
    //
    // Note: this code assumes that the number of points is exactly twice
    // the number of cells and that each cell has two unique endpoints 
    // (i.e., the endpoints are not shared).
    //
    if (npts != 2*ncells) 
    {
        EXCEPTION0(ImproperUseException);
    }
    IdPoint *idPoints = new IdPoint[npts];
    vtkPoints *inPts = input->GetPoints();
    for (i = 0 ; i < npts ; i++)
    {
        idPoints[i].lineId = line_id->GetValue(i/2); // See assumption above
        idPoints[i].ptId = i;
        double pt[3];
        inPts->GetPoint(i, pt);
        idPoints[i].radSquared = (pt[0]*pt[0] + pt[1]*pt[1] + pt[2]*pt[2]);
    }
    qsort(idPoints, npts, sizeof(IdPoint), IdPointSorter);

    double tolSqrd = tolerance*tolerance;
    for (i = 0 ; i < npts ; i++)
    {
        bool foundMatch = false;
        int  match = -1;
        double ptI[3];
        inPts->GetPoint(idPoints[i].ptId, ptI);
        for (j = i-1 ; j >= 0 ; j--)
        {
            if (idPoints[i].lineId != idPoints[j].lineId)
                break;

            // We can bound their distance apart by using the triangle
            // inequality.
            double distApart = idPoints[i].radSquared - idPoints[j].radSquared;
            if (distApart > tolSqrd)
                // This point and point J are too close to the origin
                // to be a match.  True for all points less than J as well.
                break;

            double ptJ[3];
            inPts->GetPoint(idPoints[j].ptId, ptJ);
            double diff[3];
            diff[0] = ptI[0]-ptJ[0];
            diff[1] = ptI[1]-ptJ[1];
            diff[2] = ptI[2]-ptJ[2];
            if (fabs(diff[0]) > tolerance)
                continue;
            if (fabs(diff[1]) > tolerance)
                continue;
            if (fabs(diff[2]) > tolerance)
                continue;

            double dist = diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2];
            if (dist < tolSqrd)
            {
                foundMatch = true;
                match = idPoints[j].ptId;
                break;
            }
        }

        if (foundMatch)
            newPtId[idPoints[i].ptId] = newPtId[idPoints[j].ptId];
        else
            newPtId[idPoints[i].ptId] = numNewPts++;
    }
  
    //
    // Construct a new vtkPolyData using the newPtId list.
    //
    vtkPolyData *rv = vtkPolyData::New();
    rv->GetFieldData()->ShallowCopy(input->GetFieldData());
    vtkPointData *outPD = rv->GetPointData();
    vtkPointData *inPD  = input->GetPointData();
    outPD->CopyAllocate(inPD, numNewPts);
    vtkPoints *outPts = vtkPoints::New();
    outPts->SetDataType(inPts->GetDataType());
    outPts->SetNumberOfPoints(numNewPts);
    for (i = 0 ; i < npts ; i++)
    {
        outPD->CopyData(inPD, i, newPtId[i]);
        outPts->SetPoint(newPtId[i], inPts->GetPoint(i));
    }
    rv->SetPoints(outPts);
    outPts->Delete();
    
    rv->GetCellData()->ShallowCopy(input->GetCellData());
    rv->Allocate(ncells*(2+1));
    for (i = 0 ; i < ncells ; i++)
    {
        vtkCell *cell = input->GetCell(i);
        int id1 = cell->GetPointId(0);
        id1 = newPtId[id1];
        int id2 = cell->GetPointId(1);
        id2 = newPtId[id2];
        vtkIdType line[2] = { id1, id2 };
        rv->InsertNextCell(VTK_LINE, 2, line);
    }

    delete [] newPtId;
    return rv;
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
    vtkPolyData *cleaned = MergeSegmentPoints(pd, "avtLineID", 1e-7);

    UpdateProgress(extraMsg*currentNode+extraMsg/3, totalProg);

    ExecuteLineScan(cleaned);

    cleaned->Delete();
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
//    Dave Bremer, Thu Sep  7 16:43:27 PDT 2006
//    Set the lines member.
//
//    Dave Bremer, Thu Dec  7 17:12:17 PST 2006
//    Made the construction of the line scan filter virtual, so I could
//    build it differently in the derived class avtHohlraumFluxQuery.
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
    
        avtLineScanFilter *filt = CreateLineScanFilter();
        filt->SetNumberOfLines(linesForThisPass);
        filt->SetRandomSeed(i);
        filt->SetInput(dob);
        //
        // Cause our artificial pipeline to execute.
        //
        filt->GetOutput()->Update(pspec);
        lines = filt->GetLines();

        avtDataset_p ds2 = filt->GetTypedOutput();
        avtDataTree_p tree = ds2->GetDataTree();
        ExecuteTree(tree);
        lines = NULL;
        delete filt;

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


// ****************************************************************************
//  Method: avtLineScanQuery::CreateLineScanFilter
//
//  Purpose:
//      Create the line scan filter using a virtual function, so that
//      derived classes can build the filter in different ways.
//
//  Programmer: David Bremer
//  Creation:   Dec 6, 2006
//
// ****************************************************************************

avtLineScanFilter *
avtLineScanQuery::CreateLineScanFilter()
{
    avtLineScanFilter *r = new avtLineScanFilter;
    return r;
}


