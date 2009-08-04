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
//                             avtTimeExpression.C                           //
// ************************************************************************* //

#include <avtTimeExpression.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkUnsignedCharArray.h>


// ****************************************************************************
//  Method: avtTimeExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2006
//
//  Modifications:
//    Jeremy Meredith, Wed Mar 11 12:35:29 EDT 2009
//    Added support for cycle and timestep values.
//
// ****************************************************************************

avtTimeExpression::avtTimeExpression(TimeMode m)
{
    mode = m;
}


// ****************************************************************************
//  Method: avtTimeExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2006
//
// ****************************************************************************

avtTimeExpression::~avtTimeExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtTimeExpression::DoOperation
//
//  Purpose:
//      Generates new constants (the time).
//
//  Arguments:
//      in            The input data array (ignored).
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2006
//
//  Modifications:
//    Jeremy Meredith, Wed Mar 11 12:35:29 EDT 2009
//    Added support for cycle and timestep values.
//
// ****************************************************************************
 
void
avtTimeExpression::DoOperation(vtkDataArray *, vtkDataArray *out,
                               int ncomponents, int ntuples)
{
    float val;
    switch (mode)
    {
      case MODE_TIME:
        val = (float) GetInput()->GetInfo().GetAttributes().GetTime();
        break;
      case MODE_CYCLE:
        val = (float) GetInput()->GetInfo().GetAttributes().GetCycle();
        break;
      case MODE_INDEX:
        val = (float)(GetInput()->GetContractFromPreviousExecution()->GetDataRequest()->GetTimestep());
        break;
    }
    for (int i = 0 ; i < ntuples ; i++)
        out->SetTuple1(i, val);
}


// ****************************************************************************
//  Method: avtTimeExpression::CreateArray
//
//  Purpose:
//      Creates an array of constant value.  Note that this routine will not
//      return uchar arrays.  The motivation for this is that most uchar arrays
//      come from the conditional expression (if), which uses a uchar as the
//      conditional, but really wants floats as the then- and else- variables.
//      (ie if(gt(X, 2.), 1., 0.) -- the constant 1. should be a float
//      (or whatever X is, not a uchar).
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2006
//
// ****************************************************************************

vtkDataArray *
avtTimeExpression::CreateArray(vtkDataArray *in)
{
    if (in->GetDataType() == VTK_UNSIGNED_CHAR)
    {
        return vtkFloatArray::New();
    }

    return in->NewInstance();
}


