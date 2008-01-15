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
//                             avtBinaryMathFilter.C                         //
// ************************************************************************* //

#include <avtBinaryMathFilter.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtBinaryMathFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtBinaryMathFilter::avtBinaryMathFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtBinaryMathFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtBinaryMathFilter::~avtBinaryMathFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtBinaryMathFilter::DeriveVariable
//
//  Purpose:
//      Derives a variable based on two variables in the input dataset.
//
//  Arguments:
//      in_ds     The first input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Sean Ahern
//  Creation:     Tue Jun 25 19:42:13 PDT 2002
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
//    Hank Childs, Thu Aug 14 13:40:20 PDT 2003
//    Allow the derived types to specify how many components there will be in
//    the output.
//
//    Hank Childs, Wed Dec 10 11:11:46 PST 2003
//    Do a better job of handling variables with different centerings.
//
//    Hank Childs, Thu Apr 22 11:11:33 PDT 2004
//    When the centerings are different, always use zonal.
//
//    Hank Childs, Thu Jan 20 17:32:13 PST 2005
//    Make cur_mesh accessible to derived types.
//
//    Hank Childs, Thu Jan 12 10:01:41 PST 2006
//    Make sure that all arrays are valid.
//
//    Hank Childs, Mon Jan 14 17:29:34 PST 2008
//    Add support for singletons.
//
// ****************************************************************************

vtkDataArray *
avtBinaryMathFilter::DeriveVariable(vtkDataSet *in_ds)
{
    // Our first operand is in the active variable.  We don't know if it's
    // point data or cell data, so check which one is non-NULL.
    vtkDataArray *cell_data1 = in_ds->GetCellData()->GetArray(varnames[0]);
    vtkDataArray *point_data1 = in_ds->GetPointData()->GetArray(varnames[0]);
    vtkDataArray *data1 = NULL, *data2 = NULL;

    if (cell_data1 != NULL)
    {
        data1 = cell_data1;
        centering = AVT_ZONECENT;
    }
    else
    {
        data1 = point_data1;
        centering = AVT_NODECENT;
    }

    if (data1 == NULL)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "An internal error occurred when calculating an expression."
                   "  Please contact a VisIt developer.");
    }

    // Get the second variable.
    bool ownData1 = false;
    bool ownData2 = false;
    if (centering == AVT_ZONECENT)
    {
        data2 = in_ds->GetCellData()->GetArray(varnames[1]);
        if (data2 == NULL)
        {
            data2 = in_ds->GetPointData()->GetArray(varnames[1]);
            if (data2 != NULL)
            {
                data2 = Recenter(in_ds, data2,AVT_NODECENT,outputVariableName);
                ownData2 = true;
            }
            else
            {
                EXCEPTION2(ExpressionException, outputVariableName, 
                           "Unable to locate variable");
            }
        }
    }
    else
    {
        data2 = in_ds->GetPointData()->GetArray(varnames[1]);
        if (data2 == NULL)
        {
            data2 = in_ds->GetCellData()->GetArray(varnames[1]);
            if (data2 != NULL)
            {
                // Recenter data1 so it will match data2.  We want both zonal.
                data1 = Recenter(in_ds, data1, AVT_NODECENT, outputVariableName);
                centering = AVT_ZONECENT;
                ownData1 = true;
            }
            else
            {
                EXCEPTION2(ExpressionException, outputVariableName, 
                           "Unable to locate variable");
            }
        }
    }

    //
    // Set up a VTK variable reflecting the calculated variable
    //
    int ncomps1 = data1->GetNumberOfComponents();
    int ncomps2 = data2->GetNumberOfComponents();
    int ncomps = GetNumberOfComponentsInOutput(ncomps1, ncomps2);
    int nvals  = data1->GetNumberOfTuples();
    if (nvals == 1)  // data1 is a singleton
        nvals = data2->GetNumberOfTuples();
         

    vtkDataArray *dv = CreateArray(data1);
    dv->SetNumberOfComponents(ncomps);
    dv->SetNumberOfTuples(nvals);

    cur_mesh = in_ds;
    DoOperation(data1, data2, dv, ncomps, nvals);
    cur_mesh = NULL;

    if (GetOutput()->GetInfo().GetAttributes().GetVariableDimension()
        != ncomps)
    {
        GetOutput()->GetInfo().GetAttributes().SetVariableDimension(ncomps);
    }

    if (ownData1)
    {
        data1->Delete();
    }
    if (ownData2)
    {
        data2->Delete();
    }

    return dv;
}


// ****************************************************************************
//  Method: avtBinaryMathFilter::CreateArray
//
//  Purpose:
//      Creates an array to write the output into.  Most derived types want
//      the array to be of the same form as the input.  Some (like logical
//      operators) always want them to be a specific type (like uchar).
//
//  Programmer: Hank Childs
//  Creation:   August 20, 2003
//
// ****************************************************************************

vtkDataArray *
avtBinaryMathFilter::CreateArray(vtkDataArray *in1)
{
    return in1->NewInstance();
}


// ****************************************************************************
//  Method: avtBinaryMathFilter::GetVariableDimension
//
//  Purpose:
//      Determines what the variable dimension of the output is.
//
//  Programmer: Hank Childs
//  Creation:   August 19, 2005
//
// ****************************************************************************

int
avtBinaryMathFilter::GetVariableDimension(void)
{
    if (*(GetInput()) == NULL)
        return avtMultipleInputExpressionFilter::GetVariableDimension();
    if (varnames.size() != 2)
        return avtMultipleInputExpressionFilter::GetVariableDimension();

    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    if (! atts.ValidVariable(varnames[0]))
        return avtMultipleInputExpressionFilter::GetVariableDimension();
    int ncomp1 = atts.GetVariableDimension(varnames[0]);

    if (! atts.ValidVariable(varnames[1]))
        return avtMultipleInputExpressionFilter::GetVariableDimension();
    int ncomp2 = atts.GetVariableDimension(varnames[1]);

    return GetNumberOfComponentsInOutput(ncomp1, ncomp2);
}


// ****************************************************************************
//  Method:  avtBinaryMathFilter::FilterUnderstandsTransformedRectMesh
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
avtBinaryMathFilter::FilterUnderstandsTransformedRectMesh()
{
    // simple binary math operates only on variables and
    // shouldn't be directly affected by coordinates
    return true;
}
