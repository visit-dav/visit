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
//                          avtCurveComparisonQuery.C                        //
// ************************************************************************* //

#include <avtCurveComparisonQuery.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>

#include <avtCurveConstructorFilter.h>
#include <avtDatasetSink.h>
#include <avtSourceFromAVTDataset.h>
#include <avtOriginatingSource.h>

#include <DebugStream.h>
#include <ImproperUseException.h>



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
//    Kathleen Bonnell, Thu Jul 27 17:43:38 PDT 2006 
//    Curves now represented as 1D RectilinearGrids.
// 
//    Hank Childs, Fri Feb 15 15:51:24 PST 2008
//    Add extra error checking.
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
    avtContract_p contract1 =
        input->GetOriginatingSource()->GetGeneralContract();

    avtDataset_p ds;
    CopyTo(ds, input);
    avtSourceFromAVTDataset termsrc1(ds);
    avtDataObject_p dob1 = termsrc1.GetOutput();

    avtCurveConstructorFilter ccf1;
    ccf1.SetInput(dob1);
    avtDataObject_p objOut1 = ccf1.GetOutput();

    objOut1->Update(contract1);
    curve1->SetInput(objOut1);

    //
    // Go through the rigamorale above to construct the curve for input 2.
    //
    input = curve2->GetInput();
    avtContract_p contract2 =
        input->GetOriginatingSource()->GetGeneralContract();

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
    if (curve1 == NULL)
        EXCEPTION0(ImproperUseException);
    vtkDataSet *curve2 = tree2->GetSingleLeaf();
    if (curve2 == NULL)
        EXCEPTION0(ImproperUseException);

    int  i;

    //
    // Construct the first curve.
    //
    vtkDataArray *xc = ((vtkRectilinearGrid*)curve1)->GetXCoordinates();
    vtkDataArray *sc = curve1->GetPointData()->GetScalars();
    int n1 = xc->GetNumberOfTuples();
    float *n1x = new float[n1];
    float *n1y = new float[n1];
    for (i = 0 ; i < n1 ; i++)
    {
         n1x[i] = xc->GetTuple1(i);
         n1y[i] = sc->GetTuple1(i);
    }

    //
    // Construct the second curve.
    //
    xc = ((vtkRectilinearGrid*)curve2)->GetXCoordinates();
    sc = curve2->GetPointData()->GetScalars();
    int n2 = xc->GetNumberOfTuples();
    float *n2x = new float[n2];
    float *n2y = new float[n2];
    for (i = 0 ; i < n2 ; i++)
    {
         n2x[i] = xc->GetTuple1(i);
         n2y[i] = sc->GetTuple1(i);
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


