// ************************************************************************* //
//                       avtMedianFilterExpression.C                         //
// ************************************************************************* //

#include <avtMedianFilterExpression.h>

#include <algorithm>

#include <vtkDataArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>

#include <avtCallback.h>

#include <ExpressionException.h>


using std::sort;


// ****************************************************************************
//  Method: avtMedianFilterExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2005
//
// ****************************************************************************

avtMedianFilterExpression::avtMedianFilterExpression()
{
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtMedianFilterExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2005
//
// ****************************************************************************

avtMedianFilterExpression::~avtMedianFilterExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtMedianFilterExpression::PreExecute
//
//  Purpose:
//      Initialize the haveIssuedWarning flag.
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2005
//
// ****************************************************************************

void
avtMedianFilterExpression::PreExecute(void)
{
    avtUnaryMathFilter::PreExecute();
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtMedianFilterExpression::DoOperation
//
//  Purpose:
//      Calculates the median.  I'm sure we could be much smarter about how
//      the median is calculated.  (Right now, it just sorts them and chooses
//      the middle.)
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
// ****************************************************************************

void
avtMedianFilterExpression::DoOperation(vtkDataArray *in1, vtkDataArray *out, 
                           int ncomponents,int ntuples)
{
    int  i;

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
                vector<float> all_vals(9);
                int numSamps = 0;
                for (int ii = i-1 ; ii <= i+1 ; ii++)
                {
                    if (ii < 0 || ii >= dims[0])
                        continue;
                    for (int jj = j-1 ; jj <= j+1 ; jj++)
                    {
                        if (jj < 0 || jj >= dims[1])
                            continue;
                        int idx2 = jj*dims[0] + ii;
                        all_vals[numSamps] = in1->GetTuple1(idx2);
                        numSamps++;
                    }
                }
                sort(all_vals.begin(), all_vals.end());
                out->SetTuple1(idx, all_vals[numSamps/2+1]);
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
                    vector<float> all_vals(27);
                    int numSamps = 0;
                    for (int ii = i-1 ; ii <= i+1 ; ii++)
                    {
                        if (ii < 0 || ii >= dims[0])
                            continue;
                        for (int jj = j-1 ; jj <= j+1 ; jj++)
                        {
                            if (jj < 0 || jj >= dims[1])
                                continue;
                            for (int kk = k-1 ; kk <= k+1 ; kk++)
                            {
                                if (kk < 0 || kk >= dims[1])
                                    continue;
                                int idx2 = kk*dims[1]*dims[0] + jj*dims[0] +ii;
                                all_vals[numSamps] = in1->GetTuple1(idx2);
                                numSamps++;
                            }
                        }
                    }
                    sort(all_vals.begin(), all_vals.end());
                    out->SetTuple1(idx, all_vals[numSamps/2+1]);
                }
            }
        }
    }
}


