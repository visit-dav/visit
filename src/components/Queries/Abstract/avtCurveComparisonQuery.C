// ************************************************************************* //
//                          avtCurveComparisonQuery.C                        //
// ************************************************************************* //

#include <avtCurveComparisonQuery.h>

// For qsort
#include <stdlib.h>

#include <vtkDataSet.h>

#include <avtCurveConstructorFilter.h>
#include <avtDatasetSink.h>
#include <avtSourceFromAVTDataset.h>
#include <avtTerminatingSource.h>

#include <DebugStream.h>
#include <ImproperUseException.h>


static int PointSorter(const void *, const void *);


// ****************************************************************************
//  Method: avtCurveComparisonQuery::avtCurveComparisonQuery
//
//  Purpose:
//      Construct an avtCurveComparisonQuery object.
//
//  Programmer:   Hank Childs
//  Creation:     October 2, 2003
//
// ****************************************************************************

avtCurveComparisonQuery::avtCurveComparisonQuery()
{
    //
    // Tell one of our base types, avtMultipleInputSink, that there are
    // exactly two inputs.
    //
    SetNumSinks(2);

    curve1 = new avtDatasetSink;
    SetSink(curve1, 0);
    curve2 = new avtDatasetSink;
    SetSink(curve2, 1);
}


// ****************************************************************************
//  Method: avtCurveComparisonQuery::~avtCurveComparisonQuery
//
//  Purpose:
//      Destruct an avtCurveComparisonQuery object.
//
//  Programmer:   Hank Childs
//  Creation:     October 2, 2003
//
//  Modifications:
//    Kathleen Bonnell, Thu May 12 17:21:34 PDT 2005
//    Fix memory leak.
//
// ****************************************************************************

avtCurveComparisonQuery::~avtCurveComparisonQuery()
{
    if (curve1 != NULL)
    {
        delete curve1;
        curve1 = NULL;
    }
    if (curve2 != NULL)
    {
        delete curve2;
        curve2 = NULL;
    }
}


// ****************************************************************************
//  Method: avtCurveComparisonQuery::Execute
//
//  Purpose:
//      Computes the CurveComparison number of the input curves.
//
//  Arguments:
//      inDS      The input dataset.
//      dom       The domain number.
//
//  Programmer:   Hank Childs
//  Creation:     October 2, 2003
//
//  Modifications:
//    Kathleen Bonnell, Thu May 12 17:21:34 PDT 2005
//    Fix memory leak.
//
// ****************************************************************************

void 
avtCurveComparisonQuery::Execute(void)
{
    //
    // Some extreme ugliness here to do what we want to do.
    // (1) First, we will have apply a filter that will properly construct
    //     the curve and bring it to processor 0.  That's okay.
    // (2) Second, we will have to feed it into our input sinks, so
    // (3) We can exploit our base class' friend status to get the data tree.
    // Once we have done that, we can
    // (4) Assume that each output "data tree" has exactly one VTK dataset,
    //     and it is a curve (this is because it went through the curve
    //     construction filter.
    //

    //
    // Go through the rigamorale above to construct the curve for input 1.
    //
    avtDataObject_p input = curve1->GetInput();
    avtPipelineSpecification_p pspec1 =
        input->GetTerminatingSource()->GetGeneralPipelineSpecification();

    avtDataset_p ds;
    CopyTo(ds, input);
    avtSourceFromAVTDataset termsrc1(ds);
    avtDataObject_p dob1 = termsrc1.GetOutput();

    avtCurveConstructorFilter ccf1;
    ccf1.SetInput(dob1);
    avtDataObject_p objOut1 = ccf1.GetOutput();

    objOut1->Update(pspec1);
    curve1->SetInput(objOut1);

    //
    // Go through the rigamorale above to construct the curve for input 2.
    //
    input = curve2->GetInput();
    avtPipelineSpecification_p pspec2 =
        input->GetTerminatingSource()->GetGeneralPipelineSpecification();

    CopyTo(ds, input);
    avtSourceFromAVTDataset termsrc2(ds);
    avtDataObject_p dob2 = termsrc2.GetOutput();

    avtCurveConstructorFilter ccf2;
    ccf2.SetInput(dob2);
    avtDataObject_p objOut2 = ccf2.GetOutput();

    //
    // Get the datasets.
    //
    avtDataTree_p tree1 = GetTreeFromSink(curve1);
    avtDataTree_p tree2 = GetTreeFromSink(curve2);

    //
    // Make sure that the datasets are what we expect.  If we are running
    // in parallel and not on processor 0, then there will be no data.
    //
    if (tree1->GetNumberOfLeaves() == 0)
        return;
    else if (tree1->GetNumberOfLeaves() != 1)
        EXCEPTION0(ImproperUseException);

    if (tree2->GetNumberOfLeaves() == 0)
        return;
    else if (tree2->GetNumberOfLeaves() != 1)
        EXCEPTION0(ImproperUseException);

    //
    // We know that there is only one leaf node.  It is the curve.
    //
    vtkDataSet *curve1 = tree1->GetSingleLeaf();
    vtkDataSet *curve2 = tree2->GetSingleLeaf();

    int  i;
    float pt[3];

    //
    // Construct the first curve.
    //
    int n1 = curve1->GetNumberOfPoints();
    float *n1x = new float[n1];
    float *n1y = new float[n1];
    for (i = 0 ; i < n1 ; i++)
    {
         curve1->GetPoint(i, pt);
         n1x[i] = pt[0];
         n1y[i] = pt[1];
    }

    //
    // Construct the second curve.
    //
    int n2 = curve2->GetNumberOfPoints();
    float *n2x = new float[n2];
    float *n2y = new float[n2];
    for (i = 0 ; i < n2 ; i++)
    {
         curve2->GetPoint(i, pt);
         n2x[i] = pt[0];
         n2y[i] = pt[1];
    }

    //
    // Now let the derived types worry about doing the "real" work.
    //
    double result = CompareCurves(n1, n1x, n1y, n2, n2x, n2y);

    //
    // Set the result with our output.
    //
    SetResultValue(result);
    std::string msg = CreateMessage(result);
    SetResultMessage(msg);

    delete [] n1x;
    delete [] n1y;
    delete [] n2x;
    delete [] n2y;
}


// ****************************************************************************
//  Method: avtCurveComparisonQuery::PutOnSameXIntervals
//
//  Purpose:
//      Curves are defined by a series of line segments.  The endpoints of
//      these line segments may not correspond on the two input curves.
//      The output of this function will be a new series of x-intervals that
//      will allow easier comparison between two curves.
//
//  Programmer:   Hank Childs
//  Creation:     October 4, 2003
//
//  Modifications:
//    Kathleen Bonnell, Thu Oct 14 17:19:01 PDT 2004
//    This method assumes that there are no duplicate x-values in the
//    passed array -- so call AverageYValsForDuplicateX to ensure that is
//    the case.
//
// ****************************************************************************

void 
avtCurveComparisonQuery::PutOnSameXIntervals(int on1, const float *ox1, 
        const float *oy1, int on2, const float *ox2, const float *oy2,
        std::vector<float> &usedX, std::vector<float> &newCurve1Vals,
        std::vector<float> &newCurve2Vals)
{
    int  i;
    vector<float> x1, y1, x2, y2;

    AverageYValsForDuplicateX(on1, ox1, oy1, x1, y1);
    AverageYValsForDuplicateX(on2, ox2, oy2, x2, y2);
   
    int n1 = x1.size();
    int n2 = x2.size();
    int  total_n_pts = n1 + n2;

    //
    // We want to put the line segments along the same x-intervals.  So we
    // want to determine what those intervals are.  Start by identifying all
    // of the unique x-points.
    //
    float *all_xs = new float[total_n_pts];
    int index = 0;
    for (i = 0 ; i < n1 ; i++)
        all_xs[index++] = x1[i];
    for (i = 0 ; i < n2 ; i++)
        all_xs[index++] = x2[i];
    qsort(all_xs, total_n_pts, sizeof(float), PointSorter);

    //
    // Repeats will through the algorithm off, so sort those out now.
    //
    vector<float> unique_x;
    for (i = 0 ; i < total_n_pts ; i++)
    {
         bool uniquePoint = true;
         if ((i > 0) && (all_xs[i] == all_xs[i-1]))
             uniquePoint = false;
         if (uniquePoint)
             unique_x.push_back(all_xs[i]);
    }
    delete [] all_xs;
    total_n_pts = unique_x.size();

    int nextIndForCurve1 = 0;
    int nextIndForCurve2 = 0;
    for (i = 0 ; i < total_n_pts ; i++)
    {
        // We don't want to consider points that are not valid for both curves.
        if ((unique_x[i] < x1[0]) || (unique_x[i] > x1[n1-1]) ||
            (unique_x[i] < x2[0]) || (unique_x[i] > x2[n2-1]))
        {
            if (unique_x[i] == x1[nextIndForCurve1])
                nextIndForCurve1++;
            if (unique_x[i] == x2[nextIndForCurve2])
                nextIndForCurve2++;
            continue;
        }

        if (unique_x[i] == x1[nextIndForCurve1])
        {
            // The point to consider is from curve 1.  Simply push back the
            // Y-value and indicate that we are now focused on the next point.
            newCurve1Vals.push_back(y1[nextIndForCurve1]);
            nextIndForCurve1++;
        }
        else
        {
            // We haven't seen x1[nextIndForCurve] yet, so we know
            // that unique_x[i] must be less than it.  In addition, we know
            // that x1[nextIndForCurve-1] must be valid, since otherwise
            // we would have skipped unique_x[i] as "out of range".
            float x_begin = x1[nextIndForCurve1-1];
            float x_end  = x1[nextIndForCurve1];
            float percent = (unique_x[i] - x_begin) / (x_end - x_begin);
            float slope = y1[nextIndForCurve1] - y1[nextIndForCurve1-1];
            float y = percent * slope + y1[nextIndForCurve1-1];
            newCurve1Vals.push_back(y);
        }

        if (unique_x[i] == x2[nextIndForCurve2])
        {
            // The point to consider is from curve 2.  Simply push back the
            // Y-value and indicate that we are now focused on the next point.
            newCurve2Vals.push_back(y2[nextIndForCurve2]);
            nextIndForCurve2++;
        }
        else
        {
            // We haven't seen x2[nextIndForCurve] yet, so we know
            // that unique_x[i] must be less than it.  In addition, we know
            // that x2[nextIndForCurve-1] must be valid, since otherwise
            // we would have skipped unique_x[i] as "out of range".
            float x_begin = x2[nextIndForCurve2-1];
            float x_end  = x2[nextIndForCurve2];
            float percent = (unique_x[i] - x_begin) / (x_end - x_begin);
            float slope = y2[nextIndForCurve2] - y2[nextIndForCurve2-1];
            float y = percent * slope + y2[nextIndForCurve2-1];
            newCurve2Vals.push_back(y);
        }

        usedX.push_back(unique_x[i]);
    }
}


// ****************************************************************************
//  Function: PointSorter
//
//  Purpose:
//      Used to sort points using the qsort routine.
//
//  Programmer: Hank Childs
//  Creation:   October 3, 2003
//
// ****************************************************************************

static int
PointSorter(const void *p1, const void *p2)
{
    const float *f1 = (const float *) p1;
    const float *f2 = (const float *) p2;

    if (*f1 > *f2)
        return 1;
    if (*f1 < *f2)
        return -1;

    return 0;
}


// ****************************************************************************
//  Method: avtCurveComparisonQuery::AverageYValsForDuplicateX
//
//  Purpose:
//    If there are duplicate x-values, then average the y-values for all
//    duplicates to create a unique x-values list with appropriate y-values.  
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 14, 2004
//
//  Modifications:
//
// ****************************************************************************

void
avtCurveComparisonQuery::AverageYValsForDuplicateX(int n, const float *x, 
        const float *y, vector<float> &X, vector<float> &Y)
{
    int i, j, nDups = 1;
    float sum;
    for (i = 0; i < n ; i+= nDups) 
    {
        if (i < n-1)
        {
            if (x[i] != x[i+1])
            {
                X.push_back(x[i]);
                Y.push_back(y[i]);
                nDups = 1;
            }
            else 
            {
                sum = y[i];
                nDups = 1;
                for (j = i+1; j < n; j++)
                {
                    if (x[j] != x[i])
                        break;
                    sum += y[j];
                    nDups++;
                }
                X.push_back(x[i]);
                Y.push_back(sum/nDups);
            }
        }
        else if (i == (n-1) && (x[i] != x[i-1]))
        {
            X.push_back(x[i]);
            Y.push_back(y[i]);
            nDups = 1;
        }
    }
}
