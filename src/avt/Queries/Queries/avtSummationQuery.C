/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#include <string>
#include <vector>

using     std::string;


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
//    Cyrus Harrison, Mon Aug 16 15:34:12 PDT 2010
//    Added support for the sum of each component of an array variable.
//
// ****************************************************************************

avtSummationQuery::avtSummationQuery()
{
    sumGhostValues = false;
    sumOnlyPositiveValues = false;
    sumFromOriginalElement = false;
    sums.clear();
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
//    Jeremy Meredith, Thu Feb 15 11:55:03 EST 2007
//    Call inherited PreExecute before everything else.
//
//    Cyrus Harrison, Mon Aug 16 15:34:12 PDT 2010
//    Added support for the sum of each component of an array variable.
//
// ****************************************************************************

void
avtSummationQuery::PreExecute(void)
{
    avtDatasetQuery::PreExecute();

    sums.clear();
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
//    Cyrus Harrison, Tue Sep 18 09:41:09 PDT 2007
//    Added support for user settable floating point format string
//
//    Cyrus Harrison, Mon Aug 16 15:34:12 PDT 2010
//    Added support for the sum of each component of an array variable.
//
//    Cyrus Harrison, Wed Aug 18 14:41:26 PDT 2010
//    Fix parallel problem when we have more procs than chunks to execute.
//
//    Hank Childs, Thu May 12 15:37:21 PDT 2011
//    Improve message a bit.
//
//    Brad Whitlock, Tue May 31 23:29:23 PST 2011
//    Fix the case where sums.size can return zero, causing a crash when we
//    index the first element in the empty vector.
//
//    Kathleen Biagas, Thu Feb 13 15:04:58 PST 2014
//    Add Xml results.
//
//    Kathleen Biagas, Mon Feb 24 15:37:33 PST 2014
//    Use sumType instead of variableName for Xml results.
//
// ****************************************************************************

void
avtSummationQuery::PostExecute(void)
{
    int ncomps = sums.size();
    // for procs with no data, broadcast the # of comps so they can
    // still particpate in the global sum.
    ncomps = UnifyMaximumValue(ncomps);

    // Allocate an input_sum array to contain the sum vector. We allocate it
    // using ncomps+1 elements because ncomps is the global size agreed upon
    // by all processors for this exchange. The +1 is done in case ncomps is
    // zero, which can happen. We fill the potentially extra slots with zeroes
    // so they will not contribute to the sum.
    double *input_sum = new double[ncomps + 1];
    memset(input_sum, 0, sizeof(double) * (ncomps + 1));
    for(size_t i = 0; i < sums.size(); ++i)
        input_sum[i] = sums[i];

    // Allocate a final_sums array to contain the final sum. We add 1 in case
    // ncomps==0
    double *final_sum = new double[ncomps + 1];
    memset(final_sum, 0, sizeof(double) * (ncomps + 1));

    // Element-wise sum across all processors.
    SumDoubleArrayAcrossAllProcessors(input_sum, final_sum, ncomps);

    // Store the final sum back into sums.
    sums.clear();
    for(int i = 0; i < ncomps; ++i)
        sums.push_back(final_sum[i]);

    delete [] input_sum;
    delete [] final_sum;

    if (CalculateAverage())
    {
        double newDenomSum;
        SumDoubleArrayAcrossAllProcessors(&denomSum, &newDenomSum, 1);
        denomSum = newDenomSum;
        if (denomSum != 0.)
        {
            for(int i=0; i < ncomps; i++)
                sums[i] /= denomSum;
        }
    }

    // get floating point format string 
    string floatFormat = queryAtts.GetFloatFormat();

    MapNode result_node;
    if (sums.size() == 1)
        result_node[sumType] = sums[0];
    else 
        result_node[sumType] = sums;
    char buf[1024];
    string str;
    if (CalculateAverage())
        str += "The average value of ";
    else
        str += "The total ";

    str += sumType + " is " ;

    for(int i=0; i < ncomps; i++)
    {
        if(i>0)
            str+=", ";
        SNPRINTF(buf, 1024,  floatFormat.c_str(), sums[i]);
        str += buf;
    }

    if (!units.empty())
    {
        SNPRINTF(buf, 1024, " %s%s", units.c_str(), unitsAppend.c_str());
        str += buf;
        result_node["units"] = units+unitsAppend;
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
    SetResultValues(sums);
    SetXmlResult(result_node.ToXML());
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
//    Kathleen Bonnell, Tue Jul 29 10:08:41 PDT 2008
//    If ghost-nodes unavailable, use ghost-zones to aid in determining if
//    point data should be included in the sum.
//
//    Cyrus Harrison, Mon Aug 16 15:34:12 PDT 2010
//    Added support for the sum of each component of an array variable.
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
    if (!sumGhostValues)
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
    std::set<int> summedElements;

    int nvalues = arr->GetNumberOfTuples();
    int ncomps  = arr->GetNumberOfComponents();

    if(sums.size() == 0)
    {
        sums = std::vector<double>(ncomps,0.0);
    }
    else if(sums.size() != (size_t)ncomps)
    {
         debug3 << "Summation Query ran into a multi-component variable with "
                << "an inconsistent number of components across domains!" << endl;
    }

    vtkIdList *list = vtkIdList::New();

    for (int i = 0 ; i < nvalues ; i++)
    {
        if (!pointData)
        {
            if (ghost_zones != NULL)
            {
                if (ghost_zones->GetValue(i) != 0)
                    continue;
            }
            else if (ghost_nodes != NULL)
            {
                bool allGhost = true;
                ds->GetCellPoints(i, list);
                for (int j = 0 ; j < list->GetNumberOfIds() ; j++)
                {
                    if (ghost_nodes->GetValue(list->GetId(j)) == 0)
                        allGhost = false;
                }
                if (allGhost)
                    continue;
            }
        }
        else 
        {
            if (ghost_nodes != NULL)
            {
                if (ghost_nodes->GetValue(i) != 0)
                    continue;
            }
            else if (ghost_zones != NULL)
            {
                ds->GetPointCells(i, list);
                int nghost = 0;
                for (int j = 0 ; j < list->GetNumberOfIds(); j++)
                {
                    nghost += ghost_zones->GetValue(list->GetId(j)) > 0 ? 1 : 0;
                }
                if (nghost == list->GetNumberOfIds())
                    continue;
            }
        }

        double *vals = arr->GetTuple(i);
        if (sumOnlyPositiveValues)
        {
            bool all_pos = true;
            for(int j = 0; j < ncomps && all_pos; j++)
                all_pos = all_pos && (vals[j] > 0.0);
            // skip if we only want positive vals
            if(!all_pos)
                continue;
        }

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

        for(int j = 0; j < ncomps; j++)
            sums[j] += vals[j];
        if (doAverage)
            denomSum += arr2->GetTuple1(i);
    }
    list->Delete();
}


