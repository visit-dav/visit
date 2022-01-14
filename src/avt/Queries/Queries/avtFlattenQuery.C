// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtFlattenQuery.C                             //
// ************************************************************************* //

#include <avtFlattenQuery.h>

#include <avtDataAttributes.h>
#include <avtParallel.h>
#include <DebugStream.h>
#include <QueryArgumentException.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <array>
#include <cstring>

const int avtFlattenQuery::NODE_DATA = 0;
const int avtFlattenQuery::ZONE_DATA = 1;

// ****************************************************************************
//  Method: avtFlattenQuery::avtFlattenQuery
//
//  Purpose:
//      Construct an avtFlattenQuery object.
//
//  Programmer:   Chris Laganella
//  Creation:     Tue Jan 11 17:26:14 EST 2022
//
//  Modifications:
//
// ****************************************************************************
avtFlattenQuery::avtFlattenQuery()
    : variables(), nodeData(), zoneData(), varTypes(),
        varNComps(), fillValue(0.), maxDataSize(5.)
{
    // Do nothing
}

// ****************************************************************************
//  Method: avtFlattenQuery::~avtFlattenQuery
//
//  Purpose:
//      Destroy an avtFlattenQuery object.
//
//  Programmer:   Chris Laganella
//  Creation:     Tue Jan 11 17:26:14 EST 2022
//
//  Modifications:
//
// ****************************************************************************
avtFlattenQuery::~avtFlattenQuery()
{
    // Do nothing
}

// ****************************************************************************
//  Method: avtFlattenQuery::SetInputParams
//
//  Purpose:
//      Set the input parameters.
//
//  Programmer:   Chris Laganella
//  Creation:     Wed Jan 12 13:11:52 EST 2022
//
//  Modifications:
//
// ****************************************************************************
void
avtFlattenQuery::SetInputParams(const MapNode &params)
{
    debug5 << "avtFlattenQuery::SetInputParams: "
        << params.ToJSON() << std::endl;

    if(params.HasEntry("vars"))
    {
        const MapNode *vars = params.GetEntry("vars");
        if(vars->Type() == MapNode::STRING_VECTOR_TYPE)
        {
            variables = vars->AsStringVector();
        }
    }

    if(params.HasEntry("fillValue"))
    {
        const MapNode *fv = params.GetEntry("fillValue");
        if(fv->Type() == MapNode::DOUBLE_TYPE)
        {
            fillValue = fv->AsDouble();
        }
    }

    if(variables.empty())
    {
        EXCEPTION1(QueryArgumentException, "vars");
    }
}

// ****************************************************************************
//  Method: avtFlattenQuery::GetDefaultInputParams
//
//  Purpose:
//      Get default input parameters.
//
//  Programmer:   Chris Laganella
//  Creation:     Wed Jan 12 13:11:52 EST 2022
//
//  Modifications:
//
// ****************************************************************************
void
avtFlattenQuery::GetDefaultInputParams(MapNode &params)
{
    stringVector vars;
    vars.push_back("default");
    params["vars"] = vars;

    params["fillValue"] = 0.;
}

// ****************************************************************************
//  Method: avtFlattenQuery::GetSecondaryVars
//
//  Purpose:
//
//
//  Programmer:   Chris Laganella
//  Creation:     Wed Jan 12 16:07:06 EST 2022
//
//  Modifications:
//
// ****************************************************************************
void
avtFlattenQuery::GetSecondaryVars(stringVector &outVars)
{
    outVars.clear();
    for(const auto &var : variables)
    {
        outVars.push_back(var);
    }
}

// ****************************************************************************
//  Method: avtFlattenQuery::PerformQuery
//
//  Purpose:
//
//
//  Programmer:   Chris Laganella
//  Creation:     Tue Jan 11 17:26:14 EST 2022
//
//  Modifications:
//
// ****************************************************************************
void
avtFlattenQuery::PerformQuery(QueryAttributes *qA)
{
    Init();

    UpdateProgress(0, 0);
    //
    // Allow derived types to apply any necessary filters.
    //
    avtDataObject_p dob = ApplyFilters(GetInput());

    //
    // Reset the input so that we have access to the data tree.
    //
    SetTypedInput(dob);

    avtDataTree_p tree = GetInputDataTree();
    int validInputTree = 0;
    totalNodes = 0;

    if (*tree != NULL && !tree->IsEmpty())
    {
        validInputTree = 1;
        totalNodes = tree->GetNumberOfLeaves();
    }
    else
    {
        debug2 << "Query encountered EMPTY InputDataTree after ApplyFilters.  "
               << "This may be a valid state if running parallel and there "
               << "are more processors than domains." << endl;
    }

    bool hadError = false;
    PreExecute();
    TRY
    {
        Execute(tree);
    }
    CATCH2(VisItException, e)
    {
        debug1 << "Exception occurred in " << GetType() << endl
             << "Going to keep going to prevent a parallel hang." << endl;
        queryAtts.SetResultsMessage(e.Message());
        hadError = true;
    }
    ENDTRY
    PostExecute();

    if (! ParallelizingOverTime())
        validInputTree = UnifyMaximumValue(validInputTree);

    UpdateProgress(1, 0);
    *qA = queryAtts;
}

// ****************************************************************************
//  Method: avtFlattenQuery::VerifyInput
//
//  Purpose:
//
//
//  Programmer:   Chris Laganella
//  Creation:     Tue Jan 11 17:26:14 EST 2022
//
//  Modifications:
//
// ****************************************************************************
void
avtFlattenQuery::VerifyInput(void)
{
    // Nothing yet
}

// ****************************************************************************
//  Method: avtFlattenQuery::PreExecute
//
//  Purpose:
//
//
//  Programmer:   Chris Laganella
//  Creation:     Tue Jan 11 17:26:14 EST 2022
//
//  Modifications:
//
// ****************************************************************************
void
avtFlattenQuery::PreExecute(void)
{
    nodeData.clear();
    zoneData.clear();
    varTypes.clear();
    varNComps.clear();
}

// ****************************************************************************
//  Method: avtFlattenQuery::Execute
//
//  Purpose:
//
//
//  Programmer:   Chris Laganella
//  Creation:     Tue Jan 11 17:26:14 EST 2022
//
//  Modifications:
//
// ****************************************************************************
void
avtFlattenQuery::Execute(vtkDataSet *ds, const int chunk)
{
    // Unused
    std::cout << "avtFlattenQuery::Execute(vtkDataSet*, const int)" << std::endl;
}

// ****************************************************************************
//  Method: avtFlattenQuery::Execute
//
//  Purpose:
//
//
//  Programmer:   Chris Laganella
//  Creation:     Thu Jan 13 10:19:57 EST 2022
//
//  Modifications:
//
// ****************************************************************************
void
avtFlattenQuery::Execute(avtDataTree_p dataTree)
{
    std::cout << "avtFlattenQuery::Execute(avtDataTree_p)" << std::endl;

    int nblocks = 0;
    vtkDataSet **datasets = (dataTree)
        ? dataTree->GetAllLeaves(nblocks)
        : nullptr;
    if(nblocks == 0 || !datasets || !dataTree)
    {
        debug2 << "avtFlattenQuery::Execute: Data tree on rank "
            << PAR_Rank() << " has no leaves." << std::endl;
        nblocks = 0;
    }

    std::cout << "nblocks " << nblocks << std::endl;

    // Determine how many columns / how big each domain is
    std::array<std::vector<vtkIdType>, 2> blockSizes;
    for(int i = 0; i < nblocks; i++)
    {
        vtkDataSet *block = datasets[i];
        blockSizes[NODE_DATA].push_back(block->GetNumberOfPoints());
        blockSizes[ZONE_DATA].push_back(block->GetNumberOfCells());

        // Inspect block 0 for information about each variable of interest
        if(i == 0)
        {
            vtkCellData  *cellData = block->GetCellData();
            vtkPointData *pointData = block->GetPointData();
            vtkIdType localNCol = 0;
            for(const std::string &name : variables)
            {
                // First, figure out if this variable is node or zone based.
                vtkDataArray *da = cellData->GetArray(name.c_str());
                if(!da)
                {
                    da = pointData->GetArray(name.c_str());
                    varTypes.push_back(NODE_DATA);
                    if(!da)
                    {
                        debug1 << "Variable " << name
                            << " does not exist on block 0 of rank "
                            << PAR_Rank() << std::endl;
                        varNComps.push_back(0);
                        continue;
                    }
                }
                else
                {
                    varTypes.push_back(ZONE_DATA);
                }

                // Next, check ncomps
                const auto ncomp = da->GetNumberOfComponents();
                varNComps.push_back(ncomp);
            }
        }
    }

    if(variables.size() != varTypes.size()
        || variables.size() != varNComps.size())
    {
        std::cout << "NOT OKAY" << std::endl;
        return;
    }

    // TODO: Communicate info to all ranks
    int haveInfo = 1;
    for(const auto nc : varNComps)
    {
        if(nc == 0)
        {
            haveInfo = 0;
            break;
        }
    }

    // Build the output tables
    std::array<int, 2> tableNCol{0, 0};
    std::vector<int> varOffsets;
    for(std::size_t i = 0; i < variables.size(); i++)
    {
        const int type = varTypes[i];
        varOffsets.push_back(tableNCol[type]);
        tableNCol[type] += varNComps[i];
    }

    // Sum up block sizes for both NODE_DATA and ZONE_DATA
    std::array<vtkIdType, 2> tableSizes{0, 0};
    for(int dataType = 0; dataType < 2; dataType++)
    {
        for(int blockIdx = 0; blockIdx < nblocks; blockIdx++)
        {
            tableSizes[dataType] += blockSizes[dataType][blockIdx];
        }
    }

    // Multiply the sum by the number of columns
    tableSizes[NODE_DATA] *= tableNCol[NODE_DATA];
    tableSizes[ZONE_DATA] *= tableNCol[ZONE_DATA];

    std::cout << "blockNodeSizes: [";
    for(const auto size : blockSizes[NODE_DATA])
    {
        std::cout << " " << size;
    }
    std::cout << " ]" << std::endl;

    std::cout << "blockZoneSizes: [";
    for(const auto size : blockSizes[ZONE_DATA])
    {
        std::cout << " " << size;
    }
    std::cout << " ]" << std::endl;

    std::cout << "tableSizes: [";
    for(const auto size : tableSizes)
    {
        std::cout << " " << size;
    }
    std::cout << " ]" << std::endl;

    std::array<vtkIdType, 2> blockOffsets{0, 0};
    nodeData.resize(tableSizes[NODE_DATA], fillValue);
    zoneData.resize(tableSizes[ZONE_DATA], fillValue);
    for(int blockIdx = 0; blockIdx < nblocks; blockIdx++)
    {
        vtkDataSet *ds = datasets[blockIdx];
        const std::array<vtkDataSetAttributes*, 2> dataAtts{
            (vtkDataSetAttributes*)ds->GetPointData(),
            (vtkDataSetAttributes*)ds->GetCellData()
        };

        std::cout << "Current block offsets: " << blockOffsets[0]
            << " " << blockOffsets[1] << std::endl;

        // Copy data into table in row major form
        for(std::size_t varIdx = 0; varIdx < variables.size(); varIdx++)
        {
            const std::string varName = variables[varIdx];
            const int type = varTypes[varIdx];
            const int nc = varNComps[varIdx];
            const int stride = tableNCol[type];
            // std::cout << "type " << type << " nc " << nc
            //     << " stride " << stride << std::endl;
            const vtkIdType N = blockSizes[type][blockIdx];
            vtkDataArray *da = dataAtts[varTypes[varIdx]]->GetArray(varName.c_str());
            doubleVector &out = (type == NODE_DATA) ? nodeData : zoneData;
            if(da)
            {
                doubleVector tup;
                tup.resize(nc);
                vtkIdType outIdx = blockOffsets[type] + varOffsets[varIdx];
                // std::cout << "starting index " << outIdx << std::endl;
                for(vtkIdType i = 0; i < N; i++)
                {
                    da->GetTuple(i, tup.data());
                    for(vtkIdType c = 0; c < nc; c++)
                    {
                        out[outIdx + c] = tup[c];
                    }
                    outIdx += stride;
                }
                // std::cout << "outIdx after iterating " << outIdx << std::endl;
            }
        }

        // Update offsets
        blockOffsets[NODE_DATA] += blockSizes[NODE_DATA][blockIdx] * tableNCol[NODE_DATA];
        blockOffsets[ZONE_DATA] += blockSizes[ZONE_DATA][blockIdx] * tableNCol[ZONE_DATA];
    }
}

// ****************************************************************************
//  Method: avtFlattenQuery::PostExecute
//
//  Purpose:
//
//
//  Programmer:   Chris Laganella
//  Creation:     Tue Jan 11 17:26:14 EST 2022
//
//  Modifications:
//
// ****************************************************************************
void
avtFlattenQuery::PostExecute(void)
{
    // TODO: Gather all vectors to rank 0
    std::cout << "avtFlattenQuery::PostExecute()" << std::endl;

    if(PAR_Rank() == 0)
    {
        // Now combine results into one big buffer
        doubleVector &out = queryAtts.GetResultsValue();
        out.resize(nodeData.size() + zoneData.size());
        void *zoneOut = out.data() + nodeData.size();
        if(!nodeData.empty())
        {
            memcpy(out.data(), nodeData.data(), nodeData.size() * sizeof(double));
        }
        if(!zoneData.empty())
        {
            memcpy(zoneOut, zoneData.data(), zoneData.size() * sizeof(double));
        }
        std::cout << "Successfully copied into queryAtts!" << std::endl;
        queryAtts.SelectResultsValue();

        MapNode outInfo;
        std::array<stringVector, 2> columnNames;
        for(std::size_t i = 0; i < variables.size(); i++)
        {
            const std::string &name = variables[i];
            const int nc = varNComps[i];
            const int type = varTypes[i];
            if(nc > 1)
            {
                for(int c = 0; c < nc; c++)
                {
                    columnNames[type].push_back(name + "/c"
                        + std::to_string(c));
                }
            }
            else
            {
                columnNames[type].push_back(name);
            }
        }
        std::cout << "Built up the columnNames!" << std::endl;

        if(!nodeData.empty())
        {
            outInfo["nodeColumnNames"] = columnNames[NODE_DATA];
        }
        if(!zoneData.empty())
        {
            outInfo["zoneColumnNames"] = columnNames[ZONE_DATA];
        }

        longVector shape(2, 0);
        if(!nodeData.empty())
        {
            shape[1] = columnNames[NODE_DATA].size();
            shape[0] = nodeData.size() / shape[1];
            outInfo["nodeTableShape"] = shape;
        }

        if(!zoneData.empty())
        {
            shape[1] = columnNames[ZONE_DATA].size();
            shape[0] = zoneData.size() / shape[1];
            outInfo["zoneTableShape"] = shape;
            outInfo["zoneTableOffset"] = (long)nodeData.size();
        }

        std::cout << "Built outInfo!" << std::endl;
        queryAtts.SetXmlResult(outInfo.ToXML());
        queryAtts.SetResultsMessage("Success!");
    }
    nodeData.clear();
    zoneData.clear();
    varNComps.clear();
    varTypes.clear();
}
