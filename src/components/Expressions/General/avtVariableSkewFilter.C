// ********************************************************************** //
//                          avtVariableSkewFilter.C                       //
// ********************************************************************** //

#include <avtVariableSkewFilter.h>

#include <vtkDataArray.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtVariableSkewFilter constructor
//
//  Purpose:
//    Defines the constructor.  Note: this should not be inlined in the
//    header because it causes problems for certain compilers.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 2, 2050 
//
// ****************************************************************************

avtVariableSkewFilter::avtVariableSkewFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtVariableSkewFilter destructor
//
//  Purpose:
//    Defines the destructor.  Note: this should not be inlined in the header
//    because it causes problems for certain compilers.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 2, 2005 
//
// ****************************************************************************

avtVariableSkewFilter::~avtVariableSkewFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtVariableSkewFilter::DoOperation
//
//  Purpose:
//    Performs the skew operation on each component,tuple of a data array.
//
//  Arguments:
//    in1           The first input data array.
//    in2           The second data array.
//    out           The output data array.
//    ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                  vectors, etc.)
//    ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 5, 2005 
//
//  Modifications:
//
// ****************************************************************************

void
avtVariableSkewFilter::DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                   vtkDataArray *out, int ncomponents, 
                                   int ntuples)
{
    int in1ncomps = in1->GetNumberOfComponents();
    int in2ncomps = in2->GetNumberOfComponents();
    if (in1ncomps == 1 && in2ncomps == 1)
    {
        float *r = in1->GetRange();
        for (int i = 0 ; i < ntuples ; i++)
        {
            float val1 = in1->GetComponent(i, 0);
            float val2 = in2->GetComponent(i, 0);
            float f = SkewTheValue(val1, r[0], r[1], val2);
            out->SetComponent(i, 0, f);
        }
    }
    else
    {
        EXCEPTION1(ExpressionException, "Skew can only be used on scalar " 
                   "variables.");
    }
}


// ****************************************************************************
//  Method: avtVariableSkewFilter::SkewTheValue
//
//  Purpose:
//    Performs the skew operation on each component,tuple of a data array.
//
//  Returns:    The skewed value.
//
//  Arguments:
//    val       The value to be skewed.
//    min       The minimum of all data values. 
//    max       The maximum of all data values. 
//    factor    The skew factor. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 5, 2005 
//
//  Modifications:
//
// ****************************************************************************

float
avtVariableSkewFilter::SkewTheValue(float val, float min, float max, 
                                    float factor)
{
    if (factor <= 0 || factor == 1. || min == max) 
        return val;

    float range = max - min; 
    float rangeInverse = 1. / range;
    float logSkew = log(factor);
    float k = range / (factor -1.);
    float v2 = (val - min) * rangeInverse;
    float temp =   k * (exp(v2 * logSkew) -1.) + min;
    return temp;
}
