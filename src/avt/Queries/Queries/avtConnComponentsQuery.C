// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtConnComponentsQuery.C                       //
// ************************************************************************* //

#include <avtConnComponentsQuery.h>

#include <avtCallback.h>
#include <avtConnComponentsExpression.h>
#include <avtParallel.h>
#include <avtSourceFromAVTDataset.h>


#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkIntArray.h>
#include <vtkPoints.h>
#include <vtkPointData.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtConnComponentsQuery constructor
//
//  Programmer: Cyrus Harrison
//  Creation:   February 5, 2007
//
// ****************************************************************************

avtConnComponentsQuery::avtConnComponentsQuery()
{
    // create the connected components filter
    cclFilter = new avtConnComponentsExpression();
    cclFilter->SetOutputVariableName("avt_ccl");
}


// ****************************************************************************
//  Method: avtConnComponentsQuery destructor
//
//  Programmer: Cyrus Harrison
//  Creation:   February 5, 2007
//
//  Modifications:
// 
//    Cyrus Harrison, Tue Aug  7 16:18:18 PDT 2007
//     Fixed memory leak - added delete of cclFilter
//
// ****************************************************************************

avtConnComponentsQuery::~avtConnComponentsQuery()
{
    delete cclFilter;
}


// ****************************************************************************
//  Method: avtConnComponentsQuery::PreExecute
//
//  Purpose:
//      This is called before all of the domains are executed to get the total
//      number of connected components.
//
//  Programmer: Cyrus Harrison
//  Creation:   February 5, 2007
//
// ****************************************************************************

void
avtConnComponentsQuery::PreExecute(void)
{
    // get number of comps from the ccl filter
    nComps = cclFilter->GetNumberOfComponents();
}


// ****************************************************************************
//  Method: avtConnComponentsQuery::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed to prepare the
//      query result.
//
//  Programmer: Cyrus Harrison
//  Creation:   February 5, 2007
//
//  Modifications:
//    Cyrus Harrison, Tue Mar 31 08:26:51 PDT 2009
//    Only set results on the root processor.
//
//    Kathleen Biagas, Wed Feb 26 12:23:31 PST 2014
//    Add Xml results.
//
// ****************************************************************************

void
avtConnComponentsQuery::PostExecute(void)
{
    if(PAR_Rank() == 0)
    {
        // simply return the number of connected components
        std::string msg = "";
        char buff[2048];

        if(nComps == 1)
        {snprintf(buff,2048,"Found %d connected component\n",nComps);}
        else
        {snprintf(buff,2048,"Found %d connected components\n",nComps);}

        msg += buff;

        // set output message
        SetResultMessage(msg);
        // set output value
        SetResultValue(nComps);
        // set Xml result
        MapNode result_node;
        result_node["connected_component_count"] = nComps;
        SetXmlResult(result_node.ToXML());
    }
}


// ****************************************************************************
//  Method: avtConnComponentsQuery::Execute
//
//  Purpose:
//      Processes a single domain. (No processing required for this base class)
//
//  Programmer: Cyrus Harrison
//  Creation:   February 5, 2007
//
//
// ****************************************************************************

void
avtConnComponentsQuery::Execute(vtkDataSet *ds, const int dom)
{
    ;
}

// ****************************************************************************
//  Method: avtConnComponentsQuery::ApplyFilters
//
//  Purpose:
//      Creates an artificial pipeline with the connected components filter.
//
//  Programmer: Cyrus Harrison
//  Creation:   February 2, 2007
//
// ****************************************************************************
avtDataObject_p
avtConnComponentsQuery::ApplyFilters(avtDataObject_p inData)
{
    // Create an artificial pipeline.
    avtDataset_p ds;
    CopyTo(ds, inData);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p dob = termsrc.GetOutput();

    // add the connected components filter

    cclFilter->SetInput(dob);
    dob = cclFilter->GetOutput();

    // Cause our artificial pipeline to execute.

    avtContract_p contract = 
        inData->GetOriginatingSource()->GetGeneralContract();

    cclFilter->GetOutput()->Update(contract);
    return cclFilter->GetOutput();
}


