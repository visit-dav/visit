// ************************************************************************* //
//                           avtSummationFilter.C                            //
// ************************************************************************* //

#include <avtSummationFilter.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>

#include <avtCallback.h>
#include <avtParallel.h>

#include <DebugStream.h>
#include <InvalidVariableException.h>


using     std::string;


// ****************************************************************************
//  Method: avtSummationFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   August 30, 2002
//
// ****************************************************************************

avtSummationFilter::avtSummationFilter()
{
    passData = false;
    sumGhostValues = false;
    issueWarning = true;
    sum = 0.;
    sumType = "";
    strcpy(descriptionBuffer, "Summing up variable");
}


// ****************************************************************************
//  Method: avtSummationFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtSummationFilter::~avtSummationFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtSummationFilter::SetVariableName
//
//  Purpose:
//      Sets the variable name to sum up.
//
//  Programmer: Hank Childs
//  Creation:   August 30, 2002
//
// ****************************************************************************

void
avtSummationFilter::SetVariableName(string &vn)
{
    variableName = vn;
}


// ****************************************************************************
//  Method: avtSummationFilter::SetSumType
//
//  Purpose:
//      Sets the category of variable we are summing (used for print
//      statements).
//
//  Programmer: Hank Childs
//  Creation:   August 30, 2002
//
// ****************************************************************************

void
avtSummationFilter::SetSumType(string &vn)
{
    sumType = vn;
    sprintf(descriptionBuffer, "Summing up %s", sumType.c_str());
}


// ****************************************************************************
//  Method: avtSummationFilter::PassData
//
//  Purpose:
//      Specifies whether or not the input dataset should be passed down.
//
//  Arguments:
//      val     True if we should pass down the input dataset, false if we
//              should pass down an empty dataset instead.
//
//  Programmer: Hank Childs
//  Creation:   August 30, 2002
//
// ****************************************************************************

void
avtSummationFilter::PassData(bool val)
{
    passData = val;
}


// ****************************************************************************
//  Method: avtSummationFilter::SumGhostValues
//
//  Purpose:
//      Specifies whether or not ghost values should be used in the
//      computation.
//
//  Arguments:
//      val     True if we should include ghost values in the computation.
//
//  Programmer: Hank Childs
//  Creation:   August 30, 2002
//
// ****************************************************************************

void
avtSummationFilter::SumGhostValues(bool val)
{
    sumGhostValues = val;
}


// ****************************************************************************
//  Method: avtSummationFilter::IssueWarning
//
//  Purpose:
//      Specifies whether or not we should issue a warning that says what
//      the value is.
//
//  Arguments:
//      val     True if we should issue the warning.
//
//  Programmer: Hank Childs
//  Creation:   August 30, 2002
//
// ****************************************************************************

void
avtSummationFilter::IssueWarning(bool val)
{
    issueWarning = val;
}


// ****************************************************************************
//  Method: avtSummationFilter::PreExecute
//
//  Purpose:
//      This is called before all of the domains are executed.
//
//  Programmer: Hank Childs
//  Creation:   August 30, 2002
//
// ****************************************************************************

void
avtSummationFilter::PreExecute(void)
{
    avtStreamer::PreExecute();
    sum = 0.;
}


// ****************************************************************************
//  Method: avtSummationFilter::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed.
//
//  Programmer: Hank Childs
//  Creation:   August 30, 2002
//
// ****************************************************************************

void
avtSummationFilter::PostExecute(void)
{
    avtStreamer::PostExecute();

    double newSum;
    SumDoubleArrayAcrossAllProcessors(&sum, &newSum, 1);
    sum = newSum;

    char str[1024];
    sprintf(str, "The total %s is %f", sumType.c_str(), sum);
    if (issueWarning)
    {
        avtCallback::IssueWarning(str);
    }
    else
    {
        debug1 << str << endl;
    }
}


// ****************************************************************************
//  Method: avtSummationFilter::ExecuteData
//
//  Purpose:
//      Processes a single domain.
//
//  Programmer: Hank Childs
//  Creation:   August 30, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Aug 27 16:16:52 PDT 2004
//    Rename ghost data array.
//
// ****************************************************************************

vtkDataSet *
avtSummationFilter::ExecuteData(vtkDataSet *ds, int dom, std::string)
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
                                  ds->GetCellData()->GetArray("avtGhostZones");
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
        sum += val;
    }

    return (passData ? ds : NULL);
}


