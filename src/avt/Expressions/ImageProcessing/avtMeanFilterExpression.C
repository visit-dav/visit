/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
//                          avtMeanFilterExpression.C                        //
// ************************************************************************* //

#include <avtMeanFilterExpression.h>

#include <vtkDataArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>

#include <avtCallback.h>
#include <avtExprNode.h>

#include <ExpressionException.h>

#include <string>

// ****************************************************************************
//  Method: avtMeanFilterExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2005
//
//  Modifications:
//    Gunther H. Weber, Thu Jul 22 16:14:00 PDT 2010
//    Added initializaition for width.
//
// ****************************************************************************

avtMeanFilterExpression::avtMeanFilterExpression()
{
    haveIssuedWarning = false;
    width[0] = 1;
    width[1] = 1;
    width[2] = 1;
}


// ****************************************************************************
//  Method: avtMeanFilterExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2005
//
// ****************************************************************************

avtMeanFilterExpression::~avtMeanFilterExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtMeanFilterExpression::PreExecute
//
//  Purpose:
//      Initialize the haveIssuedWarning flag.
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2005
//
// ****************************************************************************

    void
avtMeanFilterExpression::PreExecute(void)
{
    avtUnaryMathExpression::PreExecute();
    haveIssuedWarning = false;
}

// ****************************************************************************
//  Method: avtMeanFilterExpression::ProcessArguments
//
//  Purpose:
//      Parses optional algorithm argument.
//
//  Arguments:
//      args      Expression arguments
//      state     Expression pipeline state
//
//  Programmer:   Gunther H. Weber
//  Creation:     Wed Mar  4 17:35:08 PST 2009
//
// ****************************************************************************
    void
avtMeanFilterExpression::ProcessArguments(ArgsExpr *args,
        ExprPipelineState *state)
{
    // Get the argument list and # of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    int nargs = arguments->size();

    // Check number of args
    if (nargs < 1 || nargs > 4)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                "mean_filter() Incorrect syntax.\n"
                "Usage: mean_filter(varname[, width[, width[, width]]).");
    }

    // First argument is the var name, let it do its own magic
    ArgExpr *first_arg = (*arguments)[0];
    avtExprNode *first_tree = dynamic_cast<avtExprNode*>(first_arg->GetExpr());
    first_tree->CreateFilters(state);

    for (int i=1; i<nargs; ++i)
    {
        // Parse the color table argument
        ArgExpr *add_arg = (*arguments)[i];
        ExprParseTreeNode *add_tree= add_arg->GetExpr();
        std::string add_type = add_tree->GetTypeName();
        if ((add_type == "IntegerConst"))
        {
            width[i-1] =  dynamic_cast<IntegerConstExpr*>(add_tree)->GetValue();
        }
        else
        {    EXCEPTION2(ExpressionException, outputVariableName,
                "mean_filter: Invalid width argument. Expected integer.\n");
        }
    }

    if (nargs == 1)
    {
        // If no argument was specified, use 1 as default (previous behavior)
        width[0] = 1;
        width[1] = 1;
        width[2] = 1;
    }
    else if (nargs == 2)
    {
        // If only one width is specified, use it along all directions
        width[1] = width[0];
        width[2] = width[0];
    }
    else if (nargs == 3)
    {
        // If no third width is specified, set the argument to 1. This is
        // somewhat arbitrary, but most likely users will use this only when
        // filtering two dimensional data where this value does not matter.
        // In the three dimensional case this choice is a bit strange but
        // saves additional consitency checks.
        width[2] = 1;
    }
}

// ****************************************************************************
//  Method: avtMeanFilterExpression::DoOperation
//
//  Purpose:
//      Calculates the mean.
//
//  Arguments:
//      in1           The first input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2005
//
//  Modifications:
//
//    Hank Childs, Tue Aug 23 09:37:51 PDT 2005
//    Fix indexing bug.
//
//    Hank Childs, Fri Jun  9 14:34:50 PDT 2006
//    Remove unused variable.
//
//    Gunther H. Weber, Thu Mar  5 13:18:47 PST 2009
//    Use user specified width instead of fixed width of one.    
//
// ****************************************************************************

    void
avtMeanFilterExpression::DoOperation(vtkDataArray *in1, vtkDataArray *out, 
        int ncomponents,int ntuples)
{
    if (cur_mesh->GetDataObjectType() != VTK_RECTILINEAR_GRID &&
            cur_mesh->GetDataObjectType() != VTK_STRUCTURED_GRID)
    {
        if (!haveIssuedWarning)
        {
            avtCallback::IssueWarning("The mean filter expression only "
                    "operates on structured grids.");
            haveIssuedWarning = true;
        }
        return;
    }

    int dims[3];
    if (cur_mesh->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) cur_mesh;
        rgrid->GetDimensions(dims);
    }
    else
    {
        vtkStructuredGrid *sgrid = (vtkStructuredGrid *) cur_mesh;
        sgrid->GetDimensions(dims);
    }

    bool nodeCentered = true;
    if (dims[0]*dims[1]*dims[2] != ntuples)
        nodeCentered = false;

    if (!nodeCentered)
    {
        dims[0] -= 1;
        dims[1] -= 1;
        dims[2] -= 1;
    }

    if (dims[2] <= 1)
    {
        for (int i = 0 ; i < dims[0] ; i++)
        {
            for (int j = 0 ; j < dims[1] ; j++)
            {
                int idx = j*dims[0]+i;
                double sum = 0.;
                int numSamps = 0;
                for (int ii = i-width[0] ; ii <= i+width[0] ; ii++)
                {
                    if (ii < 0 || ii >= dims[0])
                        continue;
                    for (int jj = j-width[1] ; jj <= j+width[1] ; jj++)
                    {
                        if (jj < 0 || jj >= dims[1])
                            continue;
                        int idx2 = jj*dims[0] + ii;
                        sum += in1->GetTuple1(idx2);
                        numSamps++;
                    }
                }
                out->SetTuple1(idx, sum/numSamps);
            }
        }
    }
    else
    {
        for (int i = 0 ; i < dims[0] ; i++)
        {
            for (int j = 0 ; j < dims[1] ; j++)
            {
                for (int k = 0 ; k < dims[2] ; k++)
                {
                    int idx = k*dims[0]*dims[1] + j*dims[0]+i;
                    double sum = 0.;
                    int numSamps = 0;
                    for (int ii = i-width[0] ; ii <= i+width[0] ; ii++)
                    {
                        if (ii < 0 || ii >= dims[0])
                            continue;
                        for (int jj = j-width[1] ; jj <= j+width[1] ; jj++)
                        {
                            if (jj < 0 || jj >= dims[1])
                                continue;
                            for (int kk = k-width[2] ; kk <= k+width[2] ; kk++)
                            {
                                if (kk < 0 || kk >= dims[2])
                                    continue;
                                int idx2 = kk*dims[1]*dims[0] + jj*dims[0] +ii;
                                sum += in1->GetTuple1(idx2);
                                numSamps++;
                            }
                        }
                    }
                    out->SetTuple1(idx, sum/numSamps);
                }
            }
        }
    }
}
