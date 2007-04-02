#include <avtDistanceToBestFitLineFilter.h>
#include <vtkDataArray.h>
#include <avtDataTree.h>
#include <avtParallel.h>
#include <math.h>

#define N_SUM   0
#define X_SUM   1
#define Y_SUM   2
#define XY_SUM  3
#define X2_SUM  4

#define N_CALC_VALUES 5

// ****************************************************************************
// Method: avtDistanceToBestFitLineFilter::avtDistanceToBestFitLineFilter
//
// Purpose: 
//   Constructor for the avtDistanceToBestFitLineFilter class.
//
// Arguments:
//   v : If true then difference vertically - otherwise calculate perpendicular
//       distance.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 18 16:15:57 PST 2005
//
// Modifications:
//   
// ****************************************************************************

avtDistanceToBestFitLineFilter::avtDistanceToBestFitLineFilter(bool v) :
    avtBinaryMathFilter()
{
    verticalDifference = v;
    pass = 1;
    sums[N_SUM] = 0.;
    sums[X_SUM] = 0.;
    sums[Y_SUM] = 0.;
    sums[XY_SUM] = 0.;
    sums[X2_SUM] = 0.;
}

// ****************************************************************************
// Method: avtDistanceToBestFitLineFilter::~avtDistanceToBestFitLineFilter
//
// Purpose: 
//   Destructor for the avtDistanceToBestFitLineFilter class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 18 16:16:49 PST 2005
//
// Modifications:
//   
// ****************************************************************************

avtDistanceToBestFitLineFilter::~avtDistanceToBestFitLineFilter()
{
}

// ****************************************************************************
// Method: avtDistanceToBestFitLineFilter::PreExecute
//
// Purpose: 
//   Sets all of the sums to zero.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 18 16:17:07 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtDistanceToBestFitLineFilter::PreExecute(void)
{
    sums[N_SUM] = 0.;
    sums[X_SUM] = 0.;
    sums[Y_SUM] = 0.;
    sums[XY_SUM] = 0.;
    sums[X2_SUM] = 0.;
}

// ****************************************************************************
// Method: avtDistanceToBestFitLineFilter::Execute
//
// Purpose: 
//   Performs a multi-pass execute. In pass 1 we calculate sums for the best
//   fit line. In pass 2, we calculate the difference between the variables
//   and the best fit line.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 18 16:17:24 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtDistanceToBestFitLineFilter::Execute(void)
{
    //
    // Sum the values required to do the best fit line.
    //
    pass = 1;
    avtDataTree_p tree    = GetInputDataTree();
    totalNodes = 2 * tree->GetNumberOfLeaves();
    avtDataTree_p newTree = avtDataTreeStreamer::Execute(tree);
    newTree = 0;

    // Sum the array values over all processors, making sure each processor
    // gets the results
    double d[N_CALC_VALUES];
    SumDoubleArrayAcrossAllProcessors(sums, d, N_CALC_VALUES);
    for(int i = 0; i < N_CALC_VALUES; ++i)
        sums[i] = d[i];

    //
    // Make it perform the expression.
    //
    pass = 2;
    avtDataTree_p newTree2 = avtDataTreeStreamer::Execute(tree);

    SetOutputDataTree(newTree2);
}

// ****************************************************************************
// Method: avtDistanceToBestFitLineFilter::DoOperation
//
// Purpose: 
//   Does the work of calculating the expression.
//
// Arguments:
//   in1 : The data array containing the X coordinate.
//   in2 : The data array containing the Y coordinate.
//   out : The resulting data.
//   ncomps : The number of components.
//   ntuples : The number of tuples.
//
// Note:       Note that we only populate the out data array in pass 2. The 
//             results of pass 1 get thrown away.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 18 16:18:09 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtDistanceToBestFitLineFilter::DoOperation(vtkDataArray *in1, 
    vtkDataArray *in2, vtkDataArray *out, int ncomps, int ntuples)
{
    if(pass == 1)
    {
        // Sum up the values required to calculate the best fit line.
        sums[N_SUM] += double(ntuples);
        for(vtkIdType i = 0; i < ntuples; ++i)
        {
            float x = in1->GetTuple1(i);
            float y = in2->GetTuple1(i);

            sums[X_SUM] += double(x);
            sums[Y_SUM] += double(y);
            sums[XY_SUM] += double(x * y);
            sums[X2_SUM] += double(x * x);
        }
    }
    else if(pass == 2)
    {
        double dY = (sums[N_SUM] * sums[XY_SUM] - sums[X_SUM] * sums[Y_SUM]);
        double dX = (sums[N_SUM] * sums[X2_SUM] - sums[X_SUM] * sums[X_SUM]);
        double m, b;

        if(dX == 0.)
        {
            double xLine = sums[X_SUM] / sums[N_SUM];
            for(vtkIdType i = 0; i < ntuples; ++i)
            {
                float x = in1->GetTuple1(i);
                out->SetTuple1(i, x - xLine);
            }
        }
        else
        {
            m =  dY / dX;
            b = (sums[Y_SUM] - m * sums[X_SUM]) / sums[N_SUM];

            if(verticalDifference)
            {
                for(vtkIdType i = 0; i < ntuples; ++i)
                {
                    float x = in1->GetTuple1(i);
                    float y = in2->GetTuple1(i);
                    float yLine = m * x + b;
                    out->SetTuple1(i, y - yLine);
                }
            }
            else // perpendicular distance.
            {
                for(vtkIdType i = 0; i < ntuples; ++i)
                {
                    float x0 = in1->GetTuple1(i);
                    float y0 = in2->GetTuple1(i);
                    
                    // We know line eq. Y = MX + B
                    // Reorganized: ax + by + c = 0 where a=M, b=-1, c=B
                    //
                    // Dist from point to line:
                    //   |a*x0 + b*y0 + c| / sqrt(a^2 + b^2)
                    //
                    // Subst a,b,c
                    //   |M*x0 -1*y0 + B| / sqrt(M^2 + 1)

                    float d = fabs(m * x0 - y0 + b) / sqrt(m*m + 1);
                    out->SetTuple1(i, d);
                }
            }
        }
    }
}
