/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                           avtUnaryMathExpression.C                        //
// ************************************************************************* //

#include <avtUnaryMathExpression.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>

#include <ImproperUseException.h>
#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtUnaryMathExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtUnaryMathExpression::avtUnaryMathExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtUnaryMathExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtUnaryMathExpression::~avtUnaryMathExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtUnaryMathExpression::DeriveVariable
//
//  Purpose:
//      Derives a variable based on the input dataset.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Sean Ahern
//  Creation:     Wed Jun 12 16:44:28 PDT 2002
//
//  Notes:
//      Sean Ahern, Fri Jun 14 11:52:33 PDT 2002
//      Since the centering that's stored in
//      GetInput()->GetInfo().GetAttributes().GetCentering() is not on a
//      per-variable basis, we can't rely on it for the centering
//      information.  Instead, get the scalars from the point and cell
//      data.  Whichever one is non-NULL is the one we want.
//
//  Modifications:
//
//    Hank Childs, Fri Nov 15 14:30:21 PST 2002
//    Allow for arbitrary data array types.
//
//    Kathleen Bonnell, Thu Jan  2 15:16:50 PST 2003 
//    Replace MakeObject() with NewInstance() to match new vtk api. 
//
//    Sean Ahern, Tue Mar  4 18:43:51 America/Los_Angeles 2003
//    If we don't have an active variable name, just use array 0.  There
//    *has* to be one in the dataset.  Unfortunately, we don't know that
//    it's array 0, but let's go with it for now.
//
//    Hank Childs, Thu Aug 21 23:49:59 PDT 2003
//    Avoid choosing an array that is 'vtkGhostLevels', etc.
//
//    Hank Childs, Fri Sep 19 16:47:36 PDT 2003
//    Allow derived types to specify how many components there are in the 
//    output.
//
//    Hank Childs, Mon Nov  3 16:02:21 PST 2003
//    Make use of virtual function CreateArray to create VTK arrays.
//
//    Hank Childs, Tue Feb 10 08:33:05 PST 2004
//    Account for case where there is no variable to work from ['4435].
//
//    Hank Childs, Thu Jan  6 13:32:50 PST 2005
//    Create the correct sized array.
//
//    Hank Childs, Tue Aug 16 09:05:03 PDT 2005
//    Make cur_mesh accessible to derived types.
//
//    Hank Childs, Tue Mar 13 09:59:08 PDT 2007
//    Refine method for determining centering of the output variable.
//
//    Hank Childs, Sun Jan 13 20:07:56 PST 2008
//    Add support for creating singletons.
//
//    Kathleen Bonnell, Tue Apr  7 07:58:33 PDT 2009
//    When data == NULL, don't create a new vtkFloatArray twice.
//
// ****************************************************************************

vtkDataArray *
avtUnaryMathExpression::DeriveVariable(vtkDataSet *in_ds)
{
    int  i;

    vtkDataArray *cell_data = NULL;
    vtkDataArray *point_data = NULL;
    vtkDataArray *data = NULL;

    if (activeVariable == NULL)
    {
        //
        // This hack is getting more and more refined.  This situation comes up
        // when we don't know what the active variable is (mostly for the
        // constant creation filter).  We probably need more infrastructure
        // to handle this.
        // Iteration 1 of this hack said take any array.
        // Iteration 2 said take any array that isn't vtkGhostLevels, etc.
        // Iteration 3 says take the first scalar array if one is available,
        //             provided that array is not vtkGhostLevels, etc.
        //             This is because most constants we create are scalar.
        //
        // Note: this hack used to be quite important because we would use
        // the resulting array to determine the centering of the variable.
        // Now we use the IsPointVariable() method.  So this data array is
        // only used to get the type.
        //
        int ncellArray = in_ds->GetCellData()->GetNumberOfArrays();
        for (i = 0 ; i < ncellArray ; i++)
        {
            vtkDataArray *candidate = in_ds->GetCellData()->GetArray(i);
            if (strstr(candidate->GetName(), "vtk") != NULL)
                continue;
            if (strstr(candidate->GetName(), "avt") != NULL)
                continue;
            if (candidate->GetNumberOfComponents() == 1)
            {
                // Definite winner
                cell_data = candidate;
                break;
            }
            else
                // Potential winner -- keep looking
                cell_data = candidate;
        }
        int npointArray = in_ds->GetPointData()->GetNumberOfArrays();
        for (i = 0 ; i < npointArray ; i++)
        {
            vtkDataArray *candidate = in_ds->GetPointData()->GetArray(i);
            if (strstr(candidate->GetName(), "vtk") != NULL)
                continue;
            if (strstr(candidate->GetName(), "avt") != NULL)
                continue;
            if (candidate->GetNumberOfComponents() == 1)
            {
                // Definite winner
                point_data = candidate;
                break;
            }
            else
                // Potential winner -- keep looking
                point_data = candidate;
        }

        if (cell_data != NULL && cell_data->GetNumberOfComponents() == 1)
        {
            data = cell_data;
            centering = AVT_ZONECENT;
        }
        else if (point_data != NULL && point_data->GetNumberOfComponents()== 1)
        {
            data = point_data;
            centering = AVT_NODECENT;
        }
        else if (cell_data != NULL)
        {
            data = cell_data;
            centering = AVT_ZONECENT;
        }
        else
        {
            data = point_data;
            centering = AVT_NODECENT;
        }
    } 
    else
    {
        cell_data = in_ds->GetCellData()->GetArray(activeVariable);
        point_data = in_ds->GetPointData()->GetArray(activeVariable);

        if (cell_data != NULL)
        {
            data = cell_data;
            centering = AVT_ZONECENT;
        }
        else
        {
            data = point_data;
            centering = AVT_NODECENT;
        }
    }

    //
    // Set up a VTK variable reflecting the calculated variable
    //
    int ncomps = 0;
    int nvals = 0;
    if (FilterCreatesSingleton())
        nvals = 1;
    else if (activeVariable == NULL || data == NULL)
        nvals = (IsPointVariable() ? in_ds->GetNumberOfPoints() 
                                   : in_ds->GetNumberOfCells());
    else
        nvals = data->GetNumberOfTuples();

    vtkDataArray *dv = NULL;
    if (data == NULL)
    {
        //
        // We could not find a single array.  We must be doing something with
        // the mesh.
        //
        ncomps = 1;
        dv = vtkFloatArray::New();
    }
    else
    {
        ncomps = data->GetNumberOfComponents();
        dv = CreateArray(data);
    }

    if (data == NULL)
    {
        if (! NullInputIsExpected())
        {
            // One way to get here is to have vtkPolyData Curve plots.
            EXCEPTION2(ExpressionException, outputVariableName,
                 "An internal error occurred when "
                 "trying to calculate your expression.  Please contact a "
                 "VisIt developer.");
        }
    }

    int noutcomps = GetNumberOfComponentsInOutput(ncomps);
    dv->SetNumberOfComponents(noutcomps);
    dv->SetNumberOfTuples(nvals);

    //
    // Should we send in ncomps or noutcomps?  They are the same number 
    // unless the derived type re-defined GetNumberOfComponentsInOutput.
    // If it did, it probably doesn't matter.  If not, then it is the same
    // number.  So send in the input.  Really doesn't matter.
    //
    cur_mesh = in_ds;
    DoOperation(data, dv, ncomps, nvals);
    cur_mesh = NULL;

    return dv;
}


// ****************************************************************************
//  Method: avtUnaryMathExpression::CreateArray
//
//  Purpose:
//      Creates an array to write the output into.  Most derived types want
//      the array to be of the same form as the input.  Some (like logical
//      operators) always want them to be a specific type (like uchar).
//
//  Programmer: Hank Childs
//  Creation:   August 21, 2003
//
// ****************************************************************************

vtkDataArray *
avtUnaryMathExpression::CreateArray(vtkDataArray *in1)
{
    return in1->NewInstance();
}


// ****************************************************************************
//  Method: avtUnaryMathExpression::GetVariableDimension
//
//  Purpose:
//      Determines what the variable dimension of the output is.
//
//  Programmer: Hank Childs
//  Creation:   August 19, 2005
//
// ****************************************************************************

int
avtUnaryMathExpression::GetVariableDimension(void)
{
    if (*(GetInput()) == NULL)
        return avtSingleInputExpressionFilter::GetVariableDimension();

    //
    // The base class will set the variable of interest to be the 
    // 'activeVariable'.  This is a by-product of how the base class sets its
    // input.  If that method should change (SetActiveVariable), this
    // technique for inferring the variable name may stop working.
    //
    const char *varname = activeVariable;
    if (varname == NULL)
        return avtSingleInputExpressionFilter::GetVariableDimension();

    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    if (! atts.ValidVariable(varname))
        return avtSingleInputExpressionFilter::GetVariableDimension();
    int ncomp = atts.GetVariableDimension(varname);

    return GetNumberOfComponentsInOutput(ncomp);
}


// ****************************************************************************
//  Method:  avtUnaryMathExpression::FilterUnderstandsTransformedRectMesh
//
//  Purpose:
//    If this filter returns true, this means that it correctly deals
//    with rectilinear grids having an implied transform set in the
//    data attributes.  It can do this conditionally if desired.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 15, 2007
//
// ****************************************************************************
bool
avtUnaryMathExpression::FilterUnderstandsTransformedRectMesh()
{
    // simple unary math operates only on variables and
    // shouldn't be directly affected by coordinates
    return true;
}


