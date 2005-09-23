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
        EXCEPTION1(ExpressionException, "An internal error occurred when "
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

    delete haveNode;
    new_ds->Delete();

    return rv;
}


