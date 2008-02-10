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
    avtDataTreeIterator::PreExecute();
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
    avtDataTreeIterator::PostExecute();

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


