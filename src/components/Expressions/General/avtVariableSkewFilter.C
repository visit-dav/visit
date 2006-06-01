/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
        double *r = in1->GetRange();
        for (int i = 0 ; i < ntuples ; i++)
        {
            double val1 = in1->GetComponent(i, 0);
            double val2 = in2->GetComponent(i, 0);
            double f = SkewTheValue(val1, r[0], r[1], val2);
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

double
avtVariableSkewFilter::SkewTheValue(double val, double min, double max, 
                                    double factor)
{
    if (factor <= 0 || factor == 1. || min == max) 
        return val;

    double range = max - min; 
    double rangeInverse = 1. / range;
    double logSkew = log(factor);
    double k = range / (factor -1.);
    double v2 = (val - min) * rangeInverse;
    double temp =   k * (exp(v2 * logSkew) -1.) + min;
    return temp;
}
