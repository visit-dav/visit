// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtAbelInversionExpression.C                     //
// ************************************************************************* //

#include <avtAbelInversionExpression.h>

#include <math.h>

#include <vtkDataArray.h>
#include <vtkRectilinearGrid.h>

#include <avtCallback.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtAbelInversionExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   October 6, 2006
//
//  Modifications:
//
//  Alister Maguire, Thu Jun 18 10:02:58 PDT 2020
//  Set canApplyToDirectDatabaseQOT to false.
//
// ****************************************************************************

avtAbelInversionExpression::avtAbelInversionExpression()
{
    haveIssuedWarning = false;
    canApplyToDirectDatabaseQOT = false;
}


// ****************************************************************************
//  Method: avtAbelInversionExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   October 6, 2006
//
// ****************************************************************************

avtAbelInversionExpression::~avtAbelInversionExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtAbelInversionExpression::PreExecute
//
//  Purpose:
//      Initialize the haveIssuedWarning flag.
//
//  Programmer: Hank Childs
//  Creation:   October 6, 2006
//
// ****************************************************************************

void
avtAbelInversionExpression::PreExecute(void)
{
    avtUnaryMathExpression::PreExecute();
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtAbelInversionExpression::DoOperation
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
//  Creation:   October 6, 2006
//
// ****************************************************************************

void
avtAbelInversionExpression::DoOperation(vtkDataArray *in1, vtkDataArray *out, 
                           int ncomponents,int ntuples)
{
    if (cur_mesh->GetDataObjectType() != VTK_RECTILINEAR_GRID)
    {
        if (!haveIssuedWarning)
        {
            avtCallback::IssueWarning("The inverse abel transform expression "
                                      "only operates on rectilinear grids.");
            haveIssuedWarning = true;
        }
        return;
    }

    vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) cur_mesh;
    int dims[3];
    rgrid->GetDimensions(dims);

    if (dims[2] > 1)
    {
        if (!haveIssuedWarning)
        {
            avtCallback::IssueWarning("The inverse abel transform expression "
                                      "only operates on 2D grids.");
            haveIssuedWarning = true;
        }
    }

    bool nodeCentered = true;
    if (dims[0]*dims[1] != ntuples)
        nodeCentered = false;

    if (!nodeCentered)
    {
        dims[0] -= 1;
        dims[1] -= 1;
    }

    vtkDataArray *yCoord = rgrid->GetYCoordinates();
    for (int i = 0 ; i < dims[0] ; i++)
    {
        for (int j = 0 ; j < dims[1] ; j++)
        {
            double integral = 0;
            for (int k = j ; k < dims[1]-1 ; k++)
            {
                double grad;
                
                int idx = k*dims[0]+i;
                int idx_p1 = (k+1)*dims[0]+i;
                int idx_m1 = (k-1)*dims[0]+i;
                if (k == dims[1]-1)
                {
                    grad = in1->GetTuple1(idx) - in1->GetTuple1(idx_m1);
                    grad /= yCoord->GetTuple1(k) - yCoord->GetTuple1(k-1);
                }
                else if (k == 0)
                {
                    grad = in1->GetTuple1(idx_p1) - in1->GetTuple1(idx);
                    grad /= yCoord->GetTuple1(k+1) - yCoord->GetTuple1(k);
                }
                else 
                {
                    grad = in1->GetTuple1(idx_p1) - in1->GetTuple1(idx_m1);
                    grad /= yCoord->GetTuple1(k+1) - yCoord->GetTuple1(k-1);
                }

                double y1 = yCoord->GetTuple1(k);
                double y2 = yCoord->GetTuple1(k+1);
                double r = yCoord->GetTuple1(j);
                double yMid = (y1+y2)/2.;
                integral += grad / sqrt(yMid*yMid - r*r) * (y2-y1);
            }
            integral *= -1./M_PI;
            int idx = j*dims[0]+i;
            out->SetTuple1(idx, integral);
        }
    }
}


