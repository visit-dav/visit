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
//                            avtSummationQuery.C                            //
// ************************************************************************* //

#include <avtSummationQuery.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>

#include <avtCallback.h>
#include <avtParallel.h>

#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <snprintf.h>
#include <set>

using     std::string;
using     std::set;


// ****************************************************************************
//  Method: avtSummationQuery constructor
//
//  Notes:  Taken mostly from Hank Childs' avtSummationFilter.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 30, 2002
//
//  Modifications:
//    Kathleen Bonnell, Thu Mar  2 15:05:17 PST 2006
//    Added sumFromOriginalElement.
//
//    Hank Childs, Tue May 16 09:18:41 PDT 2006
//    Initial variables for averaging.
//
// ****************************************************************************

avtSummationQuery::avtSummationQuery()
{
    sumGhostValues = false;
    sumOnlyPositiveValues = false;
    sumFromOriginalElement = false;
    sum = 0.;
    denomSum = 1.;
    sumType = "";
    strcpy(descriptionBuffer, "Summing up variable");
}


// ****************************************************************************
//  Method: avtSummationQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtSummationQuery::~avtSummationQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtSummationQuery::SetVariableName
//
//  Notes:  Taken mostly from Hank Childs' avtSummationFilter.
//
//  Purpose:
//      Sets the variable name to sum up.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 30, 2002
//
// ****************************************************************************

void
avtSummationQuery::SetVariableName(string &vn)
{
    variableName = vn;
}


// ****************************************************************************
//  Method: avtSummationQuery::SetSumType
//
//  Notes:  Taken mostly from Hank Childs' avtSummationFilter.
//
//  Purpose:
//      Sets the category of variable we are summing (used for print
//      statements).
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 30, 2002
//
// ****************************************************************************

void
avtSummationQuery::SetSumType(string &vn)
{
    sumType = vn;
    SNPRINTF(descriptionBuffer, 1024, "Summing up %s", sumType.c_str());
}


// ****************************************************************************
//  Method: avtSummationQuery::SetUnitsAppend
//
//  Purpose:
//      Sets the value that should be appended to units.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 11, 2003 
//
// ****************************************************************************

void
avtSummationQuery::SetUnitsAppend(string &append)
{
    unitsAppend = append;
}



// ****************************************************************************
//  Method: avtSummationQuery::SumGhostValues
//
//  Purpose:
//      Specifies whether or not ghost values should be used in the
//      computation.
//
//  Notes:  Taken mostly from Hank Childs' avtSummationFilter.
//
//  Arguments:
//      val     True if we should include ghost values in the computation.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 30, 2002
//
// ****************************************************************************

void
avtSummationQuery::SumGhostValues(bool val)
{
    sumGhostValues = val;
}


// ****************************************************************************
//  Method: avtSummationQuery::SumOnlyPositiveValues
//
//  Purpose:
//      Specifies whether or not negative values should be used in the
//      computation.
//
//  Arguments:
//      val     True if we should include only postive values in the 
//              computation.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 30, 2002
//
// ****************************************************************************

void
avtSummationQuery::SumOnlyPositiveValues(bool val)
{
    sumOnlyPositiveValues = val;
}


// ****************************************************************************
//  Method: avtSummationQuery::SumFromOriginalElement
//
//  Purpose:
//    Specifies whether or not original cell information should be considered,
//    all values from the same original cell contribute only once to the sum. 
//
//  Arguments:
//      val     True if we should sum from original cells.
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 28, 2006
//
// ****************************************************************************

void
avtSummationQuery::SumFromOriginalElement(bool val)
{
    sumFromOriginalElement = val;
}


// ****************************************************************************
//  Method: avtSummationQuery::PreExecute
//
//  Notes:  Taken mostly from Hank Childs' avtSummationFilter.
//
//  Purpose:
//      This is called before all of the domains are executed.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 30, 2002
//
//  Modifications:
//
//    Hank Childs, Tue May 16 09:18:41 PDT 2006
//    Initialize denomSum.
//
// ****************************************************************************

void
avtSummationQuery::PreExecute(void)
{
    sum = 0.;
    denomSum = 0.;
}


// ****************************************************************************
//  Method: avtSummationQuery::PostExecute
//
//  Notes:  Taken mostly from Hank Childs' avtSummationFilter.
//
//  Purpose:
//      This is called after all of the domains are executed.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 30, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 11 16:11:10 PDT 2003
//    Add units if they exist. Set the results value. Renamed 'SetMessage' to
//    'SetResultMessage'.
//
//    Hank Childs, Thu Jan 12 14:58:07 PST 2006
//    Add qualifier message if it exists.
//
//    Hank Childs, Tue May 16 09:18:41 PDT 2006
//    Added support for averaging.
//
// ****************************************************************************

void
avtSummationQuery::PostExecute(void)
{
    double newSum;
    SumDoubleArrayAcrossAllProcessors(&sum, &newSum, 1);
    sum = newSum;

    if (CalculateAverage())
    {
        double newDenomSum;
        SumDoubleArrayAcrossAllProcessors(&denomSum, &newDenomSum, 1);
        denomSum = newDenomSum;
        if (denomSum != 0.)
            sum /= denomSum;
    }

    char buf[1024];
    std::string str;
    if (CalculateAverage())
        str += "The average ";
    else
        str += "The total ";
    SNPRINTF(buf, 1024, "%s", sumType.c_str());
    str += buf;
    str += " is ";
    SNPRINTF(buf, 1024,  "%f", sum);
    str += buf; 
    if (!units.empty())
    {
        SNPRINTF(buf, 1024, " %s%s", units.c_str(), unitsAppend.c_str());
        str += buf; 
    }
    if (!qualifier.empty())
    {
        str += "\n";
        str += qualifier;
    }
  
    //
    //  Parent class uses this message to set the Results message
    //  in the Query Attributes that is sent back to the viewer.
    //  That is all that is required of this query.
    //
    SetResultMessage(str);
    SetResultValue(sum);
}


// ****************************************************************************
//  Method: avtSummationQuery::Execute
//
//  Purpose:
//      Processes a single domain.
//
//  Notes:  Taken mostly from Hank Childs' avtSummationFilter.
//
//  Programmer: Kathleen Bonnell  
//  Creation:   September 30, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002 
//    Added dom argument.
//
//    Hank Childs, Tue Aug 24 08:22:24 PDT 2004
//    Add support for ghost nodes.
//
//    Hank Childs, Fri Aug 27 16:02:58 PDT 2004
//    Rename ghost data array.
//
//    Kathleen Bonnell, Wed Dec 22 13:03:37 PST 2004 
//    Correct how ghostNodes are used with PointData.
//
//    Kathleen Bonnell, Thu Mar  2 15:05:17 PST 2006 
//    Added logic so that if sumFromOriginalElement flag is set, will check
//    OriginalCells/Nodes arrays, and use the value from the 'original' element
//    only once in the sum, regardless of how many new sub-elements the original
//    cell was split into -- because when that original cell was split, the
//    variable's value was passed intact to the new cells.
//
//    Hank Childs, Tue May 16 09:18:41 PDT 2006
//    Add support for averaging.
//
// ****************************************************************************

void
avtSummationQuery::Execute(vtkDataSet *ds, const int dom)
{
    bool pointData = true;
    vtkDataArray *arr = ds->GetPointData()->GetArray(variableName.c_str());
    if (arr == NULL)
    {
        arr = ds->GetCellData()->GetArray(variableName.c_str());
        pointData = false;
    }
    if (arr == NULL)
    {
        EXCEPTION1(InvalidVariableException, variableName);
    }

    vtkDataArray *arr2 = NULL;
    bool doAverage = CalculateAverage();
    if (doAverage)
    {
        if (pointData)
            arr2 = ds->GetPointData()->GetArray(denomVariableName.c_str());
        else 
            arr2 = ds->GetCellData()->GetArray(denomVariableName.c_str());

        if (arr2 == NULL)
        {
            // Note that we will get here if the centering is different
            // between variableName and denomVariableName.
            EXCEPTION1(InvalidVariableException, denomVariableName);
        }
    }


    vtkUnsignedCharArray *ghost_zones = NULL;
    if (!pointData && !sumGhostValues)
    {
        ghost_zones = (vtkUnsignedCharArray *)
                                  ds->GetCellData()->GetArray("avtGhostZones");
    }
    vtkUnsignedCharArray *ghost_nodes = NULL;
    if (!sumGhostValues)
    {
        ghost_nodes = (vtkUnsignedCharArray *)
                                 ds->GetPointData()->GetArray("avtGhostNodes");
    }
    int comp =0;
    vtkIntArray *originalCells = NULL;
    vtkIntArray *originalNodes = NULL;
    if (sumFromOriginalElement)
    {
        if (pointData)
        {
            originalNodes = (vtkIntArray *)
                         ds->GetPointData()->GetArray("avtOriginalNodeNumbers");
            if (originalNodes)
            {
                comp = originalNodes->GetNumberOfComponents() - 1;
            }
            else 
            {
                debug3 << "Summation Query told to sum from original nodes but "
                   << "could not find avtOriginalNodeNumbers array." << endl;
            }
        }
        else
        {
            originalCells = (vtkIntArray *)
                         ds->GetCellData()->GetArray("avtOriginalCellNumbers");
            if (originalCells)
            {
                comp = originalCells->GetNumberOfComponents() - 1;
            }
            else 
            {
                debug3 << "Summation Query told to sum from original cells but "
                   << "could not find avtOriginalCellNumbers array." << endl;
            }
        }
    }
    set<int> summedElements;

    int nValues = arr->GetNumberOfTuples();
    vtkIdList *list = vtkIdList::New();

    for (int i = 0 ; i < nValues ; i++)
    {
        float val = arr->GetTuple1(i);
        if (!pointData)
        {
            if (ghost_zones != NULL)
            {
                unsigned char g = ghost_zones->GetValue(i);
                if (g != 0)
                    continue;
            }
            else if (ghost_nodes != NULL)
            {
                bool allGhost = true;
                ds->GetCellPoints(i, list);
                vtkIdType npts = list->GetNumberOfIds();
                for (int j = 0 ; j < npts ; j++)
                {
                    vtkIdType id = list->GetId(j);
                    unsigned char g = ghost_nodes->GetValue(id);
                    if (g == 0)
                        allGhost = false;
                }
                if (allGhost)
                    continue;
            }
        }
        else if (ghost_nodes != NULL)
        {
            unsigned char g = ghost_nodes->GetValue(i);
            if (g != 0)
                continue;
        }
        if (sumOnlyPositiveValues && val < 0.)
            continue;
        
        if (originalCells)
        {
            int origCell = (int)originalCells->GetComponent(i, comp); 
            if (!(summedElements.insert(origCell)).second) 
                continue;
        } 
        else if (originalNodes)
        {
            int origNode = (int)originalNodes->GetComponent(i, comp); 
            if (origNode == -1 || !(summedElements.insert(origNode)).second) 
                continue;
        } 

        sum += val;
        if (doAverage)
            denomSum += arr2->GetTuple1(i);
    }
    list->Delete();
}


