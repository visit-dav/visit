// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                    avtOriginalDataSpatialExtentsQuery.C                   //
// ************************************************************************* //

#include <avtOriginalDataSpatialExtentsQuery.h>

#include <avtDatasetExaminer.h>
#include <avtParallel.h>
#include <avtOriginatingSource.h>
#include <ParsingExprList.h>
#include <snprintf.h>

#include <string>

// ****************************************************************************
//  Method: avtOriginalDataSpatialExtentsQuery constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 31, 2006 
//
//  Modifications:
//
// ****************************************************************************

avtOriginalDataSpatialExtentsQuery::avtOriginalDataSpatialExtentsQuery() : avtDatasetQuery() 
{
}


// ****************************************************************************
//  Method: avtOriginalDataSpatialExtentsQuery destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 31, 2006 
//
//  Modifications:
//
// ****************************************************************************
avtOriginalDataSpatialExtentsQuery::~avtOriginalDataSpatialExtentsQuery() 
{
}


// ****************************************************************************
//  Method: avtOriginalDataSpatialExtentsQuery::ApplyFilters
//
//  Purpose:
//    Makes sure the entire SIL is used. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   January 31, 2006 
//
//  Modifications:
//
// ****************************************************************************

avtDataObject_p
avtOriginalDataSpatialExtentsQuery::ApplyFilters(avtDataObject_p inData)
{
    avtDataRequest_p dataRequest = inData->GetOriginatingSource()->
        GetGeneralContract()->GetDataRequest();

    std::string dbVar = ParsingExprList::GetRealVariable(
                       queryAtts.GetVariables()[0]);
    avtDataRequest_p new_dataRequest = new avtDataRequest(dataRequest,
                                                                dbVar.c_str());

    avtContract_p contract = 
        new avtContract(new_dataRequest, queryAtts.GetPipeIndex()); 

    avtDataObject_p retObj;
    CopyTo(retObj, inData);
    retObj->Update(contract);
    return retObj;
}


// ****************************************************************************
//  Method: avtOriginalDataSpatialExtentsQuery::PerformQuery
//
//  Purpose:
//    Perform the requested query. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 31, 2006 
//
//  Modifications:
//
//    Hank Childs, Fri Jun  9 14:43:27 PDT 2006
//    Remove unused variable.
//
//    Cyrus Harrison, Tue Sep 18 13:45:35 PDT 2007
//    Added support for user settable floating point format string 
//
//    Kathleen Biagas, Thu Feb 13 15:04:58 PST 2014
//    Add Xml results.
//
// ****************************************************************************

void
avtOriginalDataSpatialExtentsQuery::PerformQuery(QueryAttributes *qA)
{
    queryAtts = *qA;
    Init(); 

    std::string floatFormat = queryAtts.GetFloatFormat();
    std::string format ="";
    UpdateProgress(0, 0);

    avtDataObject_p dob = ApplyFilters(GetInput());

    SetTypedInput(dob);

    avtDataset_p input = GetTypedInput();
    double extents[6] = {0., 0., 0., 0., 0., 0.};
    char msg[1024];

    avtDatasetExaminer::GetSpatialExtents(input, extents);
    UnifyMinMax(extents, 6);
    int dim = input->GetInfo().GetAttributes().GetSpatialDimension();
    if (dim == 1)
    {
        format = "The original extents are (" + floatFormat + ", " 
                                              + floatFormat + ")";
        SNPRINTF(msg, 1024,format.c_str(), 
                extents[0], extents[1]);
    }
    else if (dim == 2)
    {
        format = "The original extents are (" + floatFormat + ", " 
                                              + floatFormat + ", " 
                                              + floatFormat + ", " 
                                              + floatFormat + ")";
        SNPRINTF(msg, 1024, format.c_str(), 
            extents[0], extents[1], extents[2], extents[3]);
    }
    else if (dim == 3)
    {
        format = "The original extents are (" + floatFormat + ", " 
                                              + floatFormat + ", " 
                                              + floatFormat + ", " 
                                              + floatFormat + ", " 
                                              + floatFormat + ", " 
                                              + floatFormat + ")";
       SNPRINTF(msg, 1024, format.c_str(),
        extents[0], extents[1], extents[2], extents[3], extents[4], extents[5]);
    }
    doubleVector d;
    for (int i = 0 ; i < 2*dim ; i++)
        d.push_back(extents[i]);
    qA->SetResultsMessage(msg);
    qA->SetResultsValue(d);

    MapNode result_node;
    result_node["extents"] = d;
    qA->SetXmlResult(result_node.ToXML());

    UpdateProgress(1, 0);
}

