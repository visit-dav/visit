// ************************************************************************* //
//                            avtWatertightQuery.C                           //
// ************************************************************************* //

#include <avtWatertightQuery.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <avtExternalNodeExpression.h>
#include <avtSourceFromAVTDataset.h>
#include <InvalidDimensionsException.h>
#include <InvalidVariableException.h>

#include <avtParallel.h>

using     std::string;

// ****************************************************************************
//  Method: avtWatertightQuery constructor
//
//  Programmer: Hank Childs 
//  Creation:   September 23, 2005 
//
// ****************************************************************************

avtWatertightQuery::avtWatertightQuery() 
{
    external_nodes = new avtExternalNodeExpression;
    external_nodes->SetOutputVariableName("_avt_external_nodes");
}


// ****************************************************************************
//  Method: avtWatertightQuery destructor
//
//  Programmer: Hank Childs 
//  Creation:   September 23, 2005 
//
//  Modifications:
//
// ****************************************************************************

avtWatertightQuery::~avtWatertightQuery()
{
    if (external_nodes != NULL)
    {
        delete external_nodes;
        external_nodes = NULL;
    }
}


// ****************************************************************************
//  Method: avtWatertightQuery::VerifyInput 
//
//  Purpose:  Throw exception for invalid input. 
//
//  Programmer: Hank Childs 
//  Creation:   September 23, 2005
//
// ****************************************************************************

void 
avtWatertightQuery::VerifyInput()
{
    //
    //  Since base class performs general input verification for all queries,
    //  make sure to call it's verify method first.
    //
    avtDataObjectQuery::VerifyInput();

    int tDim = GetInput()->GetInfo().GetAttributes().GetTopologicalDimension();
    if  (tDim != 2)
    {
        EXCEPTION2(InvalidDimensionsException, "Watertight query", 
                   "3D surface");
    }
}


// ****************************************************************************
//  Method: avtWatertightQuery::ApplyFilters 
//
//  Purpose:  Apply any avt filters necessary for completion of this query.
//
//  Programmer: Hank Childs 
//  Creation:   September 23, 2005 
//
// ****************************************************************************

avtDataObject_p 
avtWatertightQuery::ApplyFilters(avtDataObject_p inData)
{
    avtPipelineSpecification_p pspec =
        inData->GetTerminatingSource()->GetGeneralPipelineSpecification();

    //
    // Create an artificial pipeline.
    //
    avtDataset_p ds;
    CopyTo(ds, inData);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p dob = termsrc.GetOutput();
    external_nodes->SetInput(dob);
    avtDataObject_p objOut = external_nodes->GetOutput();
    objOut->Update(pspec);
    return objOut;
}



// ****************************************************************************
//  Method: avtWatertightQuery::PreExecute
//
//  Purpose:
//      This is called before all of the domains are executed.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2005
//
// ****************************************************************************

void
avtWatertightQuery::PreExecute(void)
{
    foundExternalNode = false;
}



// **************************************************************************** //  Method: avtWatertightQuery::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2005
//
// ****************************************************************************


void
avtWatertightQuery::PostExecute(void)
{
    int boolAsInt = (int) foundExternalNode;
    boolAsInt = UnifyMaximumValue(boolAsInt);
    foundExternalNode = (boolAsInt > 0);

    char buf[1024];
    if (foundExternalNode)
    {
        strcpy(buf, "The surface is not watertight.  To identify the regions "
                  "that are not watertight, perform the following steps:\n"
                  "\t1) Define the expression: \"external_node(meshname)\"\n"
                  "\t2) If some set of operations have been performed on the "
                  "mesh, then you will need to turn on the DeferExpression \n"
                  "\toperator under Options->Plugin Manager and then apply "
                  "this operator with the external node expression to your "
                  "plot.\n"
                  "\t3) Change the variable you are plotting by to the "
                  "external nodes expression.  This will show where the \n"
                  "\tnon-watertight nodes are located.\n");
    }
    else
    {
        strcpy(buf, "The surface is watertight.");
    }

    //
    //  Parent class uses this message to set the Results message
    //  in the Query Attributes that is sent back to the viewer.
    //  That is all that is required of this query.
    //
    SetResultMessage(buf);
    SetResultValue(foundExternalNode);
}


// ****************************************************************************
//  Method: avtWatertightQuery::Execute
//
//  Purpose:
//      Processes a single domain.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2005
//
// ****************************************************************************

void
avtWatertightQuery::Execute(vtkDataSet *ds, const int dom)
{
    vtkDataArray *arr = ds->GetPointData()->GetArray("_avt_external_nodes");
    if (arr == NULL)
        arr = ds->GetCellData()->GetArray("_avt_external_nodes");

    if (arr == NULL)
        EXCEPTION1(InvalidVariableException, "_avt_external_nodes");

    int nValues = arr->GetNumberOfTuples();
    for (int i = 0 ; i < nValues ; i++)
    {
        float val = arr->GetTuple1(i);
        if (val != 0.)
            foundExternalNode = true;
    }
}


