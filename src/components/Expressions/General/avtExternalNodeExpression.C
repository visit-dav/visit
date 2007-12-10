/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

// ************************************************************************* //
//                        avtExternalNodeExpression.C                        //
// ************************************************************************* //

#include <avtExternalNodeExpression.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>

#include <avtCallback.h>
#include <avtCondenseDatasetFilter.h>
#include <avtDatasetExaminer.h>
#include <avtGhostZoneAndFacelistFilter.h>
#include <avtSourceFromAVTDataset.h>
#include <avtTypes.h>

#include <ExpressionException.h>

// ****************************************************************************
//  Method: avtExternalNodeExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   September 21, 2005
//
// ****************************************************************************

avtExternalNodeExpression::avtExternalNodeExpression()
{
}


// ****************************************************************************
//  Method: avtExternalNodeExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   September 21, 2005
//
// ****************************************************************************

avtExternalNodeExpression::~avtExternalNodeExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtExternalNodeExpression::DeriveVariable
//
//  Purpose:
//      Assigns the zone ID to each variable.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     September 21, 2005
//
//  Modifications:
//
//    Hank Childs, Wed Sep 28 11:07:50 PDT 2005
//    Use correct form of delete operator.
//
// ****************************************************************************

vtkDataArray *
avtExternalNodeExpression::DeriveVariable(vtkDataSet *in_ds)
{
    int  i;

    vtkDataSet *new_ds = in_ds->NewInstance();
    new_ds->ShallowCopy(in_ds);
    int npts = new_ds->GetNumberOfPoints();
    vtkIntArray *arr = vtkIntArray::New();
    arr->SetNumberOfTuples(npts);
    for (i = 0 ; i < npts ; i++)
        arr->SetValue(i, i);
    const char *varname = "_avt_node_id";
    arr->SetName(varname);
    new_ds->GetPointData()->AddArray(arr);
    arr->Delete();

    //
    // A whole bunch of rigamorale to get the AVT pipeline to update using
    // only this VTK dataset.
    //
    avtDataset_p ds = new avtDataset(new_ds, *(GetTypedInput()));
    avtSourceFromAVTDataset termsrc(ds);
    avtGhostZoneAndFacelistFilter gzff;
    gzff.SetInput(termsrc.GetOutput());
    gzff.SetUseFaceFilter(true);
    gzff.SetCreateEdgeListFor2DDatasets(true);
    avtCondenseDatasetFilter cdf;
    cdf.KeepAVTandVTK(true);
    cdf.BypassHeuristic(true);
    cdf.SetInput(gzff.GetOutput());
    cdf.Update(GetGeneralPipelineSpecification());
    avtDataset_p ds2;
    avtDataObject_p output = cdf.GetOutput();
    CopyTo(ds2, output);
    avtCentering node_cent = AVT_NODECENT;
    vtkDataArray *arr2 = 
                   avtDatasetExaminer::GetArray(ds2, varname, 0, node_cent);
    if (arr2 == NULL || arr2->GetDataType() != VTK_INT)
        EXCEPTION2(ExpressionException, outputVariableName, "An internal error occurred when "
                   "calculating the external nodes.");
    vtkIntArray *arr3 = (vtkIntArray *) arr2;

    bool *haveNode = new bool[npts];
    for (i = 0 ; i < npts ; i++)
        haveNode[i] = false;

    int nArr = arr3->GetNumberOfTuples();
    for (i = 0 ; i < nArr ; i++)
        haveNode[arr3->GetValue(i)] = true;

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfTuples(npts);
    for (i = 0 ; i < npts ; i++)
        rv->SetTuple1(i, (haveNode[i] ? 1. : 0.));

    delete [] haveNode;
    new_ds->Delete();

    return rv;
}


