// ************************************************************************* //
//                            avtSummationQuery.C                            //
// ************************************************************************* //

#include <avtSummationQuery.h>

#include <vtkDataSet.h>
#include <vtkUnsignedCharArray.h>

#include <avtCallback.h>
#include <avtParallel.h>

#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <snprintf.h>

using     std::string;


// ****************************************************************************
//  Method: avtSummationQuery constructor
//
//  Notes:  Taken mostly from Hank Childs' avtSummationFilter.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 30, 2002
//
// ****************************************************************************

avtSummationQuery::avtSummationQuery()
{
    sumGhostValues = false;
    sumOnlyPositiveValues = false;
    sum = 0.;
    sumType = "";
    strcpy(descriptionBuffer, "Summing up variable");
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
// ****************************************************************************

void
avtSummationQuery::PreExecute(void)
{
    sum = 0.;
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
// ****************************************************************************

void
avtSummationQuery::PostExecute(void)
{
    double newSum;
    SumDoubleArrayAcrossAllProcessors(&sum, &newSum, 1);
    sum = newSum;

    char buf[1024];
    std::string str;
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

    vtkUnsignedCharArray *ghosts = NULL;
    if (!pointData && !sumGhostValues)
    {
        ghosts = (vtkUnsignedCharArray *)
                                 ds->GetCellData()->GetArray("vtkGhostLevels");
    }

    int nValues = arr->GetNumberOfTuples();
    for (int i = 0 ; i < nValues ; i++)
    {
        float val = arr->GetTuple1(i);
        if (ghosts != NULL)
        {
            unsigned char g = ghosts->GetValue(i);
            if (g != 0)
            {
                val = 0.;
            }
        }
        if (sumOnlyPositiveValues && val < 0.)
            continue;

        sum += val;
    }
}


