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

#include <execinfo.h>

#define PRINT_BACKTRACE(stream) \
do { \
    void *array[20]; \
    char **strings; \
    int size; \
    size = backtrace(array, 20); \
    strings = backtrace_symbols(array, size); \
    if(strings) \
    { \
        stream << __PRETTY_FUNCTION__ << " called, backtrace:\n"; \
        for(int i = 0; i < size; i++) \
        { \
            stream << "[" << i << "]: " << strings[i] << "\n"; \
        } \
        stream.flush(); \
    } \
    free(strings); \
} while(0)

const int avtFlattenQuery::NODE_DATA = 0;
const int avtFlattenQuery::ZONE_DATA = 1;

// ****************************************************************************
//  Function: combine_tables
//
//  Purpose:
//      Helper function to combine node & zone tables.
//
//  Programmer:   Chris Laganella
//  Creation:     Wed Jan 19 11:43:18 EST 2022
//
//  Modifications:
//
// ****************************************************************************
static void
CombineTables(const doubleVector &nodeTable, const doubleVector &zoneTable,
    doubleVector &out, double fillValue)
{
    const auto nodeSize = nodeTable.size();
    const auto zoneSize = zoneTable.size();
    out.resize(nodeSize + zoneSize, fillValue);
    void *const zoneOut = out.data() + nodeSize;
    if(nodeSize)
    {
        memcpy(out.data(), nodeTable.data(), nodeSize * sizeof(double));
    }
    if(zoneSize)
    {
        memcpy(zoneOut, zoneTable.data(), zoneSize * sizeof(double));
    }
}

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
    : variables(), outData(), fillValue(0.), maxDataSize(5.)
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
    outData.clear();
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

    debug5 << "nblocks " << nblocks << std::endl;

    // Determine how many columns / how big each domain is
    std::array<std::vector<vtkIdType>, 2> blockSizes;
    for(int i = 0; i < nblocks; i++)
    {
        vtkDataSet *block = datasets[i];
        blockSizes[NODE_DATA].push_back(block->GetNumberOfPoints());
        blockSizes[ZONE_DATA].push_back(block->GetNumberOfCells());
    }


    intVector varNComps;
    intVector varTypes;
    const auto &dataAtts = GetTypedInput()->GetInfo().GetAttributes();
    vtkIdType localNCol = 0;
    for(const std::string &name : variables)
    {
        avtCentering centering = AVT_NO_VARIABLE;
        int nc = 0;
        TRY
            centering = dataAtts.GetCentering(name.c_str());
            nc = dataAtts.GetVariableDimension(name.c_str());
        CATCHALL
            nc = 0;
            centering = AVT_NO_VARIABLE;
            debug1 << "The variable " << name
                << " does not exist in the input data." << std::endl;
        ENDTRY

        varNComps.push_back(nc);
        varTypes.push_back(centering == AVT_ZONECENT ? ZONE_DATA : NODE_DATA);
    }

    if(variables.size() != varTypes.size()
        || variables.size() != varNComps.size())
    {
        std::cout << "NOT OKAY" << std::endl;
        return;
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

    debug5 << "blockNodeSizes: [";
    for(const auto size : blockSizes[NODE_DATA])
    {
        debug5 << " " << size;
    }
    debug5 << " ]" << std::endl;

    debug5 << "blockZoneSizes: [";
    for(const auto size : blockSizes[ZONE_DATA])
    {
        debug5 << " " << size;
    }
    debug5 << " ]" << std::endl;

    debug5 << "tableSizes: [";
    for(const auto size : tableSizes)
    {
        debug5 << " " << size;
    }
    debug5 << " ]" << std::endl;

    std::array<vtkIdType, 2> blockOffsets{0, 0};
    doubleVector nodeData(tableSizes[NODE_DATA], fillValue);
    doubleVector zoneData(tableSizes[ZONE_DATA], fillValue);
    for(int blockIdx = 0; blockIdx < nblocks; blockIdx++)
    {
        vtkDataSet *ds = datasets[blockIdx];
        const std::array<vtkDataSetAttributes*, 2> dataAtts{
            (vtkDataSetAttributes*)ds->GetPointData(),
            (vtkDataSetAttributes*)ds->GetCellData()
        };

        debug5 << "Current block offsets: " << blockOffsets[0]
            << " " << blockOffsets[1] << std::endl;

        // Copy data into table in row major form
        for(std::size_t varIdx = 0; varIdx < variables.size(); varIdx++)
        {
            const std::string varName = variables[varIdx];
            const int type = varTypes[varIdx];
            const int nc = varNComps[varIdx];
            const int stride = tableNCol[type];
            const vtkIdType N = blockSizes[type][blockIdx];
            vtkDataArray *da = dataAtts[varTypes[varIdx]]->GetArray(varName.c_str());
            doubleVector &out = (type == NODE_DATA) ? nodeData : zoneData;
            if(da)
            {
                doubleVector tup;
                tup.resize(nc);
                vtkIdType outIdx = blockOffsets[type] + varOffsets[varIdx];
                for(vtkIdType i = 0; i < N; i++)
                {
                    da->GetTuple(i, tup.data());
                    for(vtkIdType c = 0; c < nc; c++)
                    {
                        out[outIdx + c] = tup[c];
                    }
                    outIdx += stride;
                }
            }
        }

        // Update offsets
        blockOffsets[NODE_DATA] += blockSizes[NODE_DATA][blockIdx] * tableNCol[NODE_DATA];
        blockOffsets[ZONE_DATA] += blockSizes[ZONE_DATA][blockIdx] * tableNCol[ZONE_DATA];
    }

    if(PAR_Size() == 1)
    {
        CombineTables(nodeData, zoneData, outData, fillValue);
        BuildOutputInfo(varNComps, varTypes, nodeData.size(), zoneData.size(), outInfo);
    }
    else
    {
        doubleVector globalNodeTable;
        doubleVector globalZoneTable;

        std::cout << "Rank " << PAR_Rank() << " contributing (node) " << nodeData.size() << std::endl;
        std::cout << "Rank " << PAR_Rank() << " contributing (zone) " << zoneData.size() << std::endl;

        std::vector<int> recvCounts;
        CollectDoubleVectorsOnRank(globalNodeTable, recvCounts, nodeData, 0);
        long nodeSize = 0;
        for(const auto count : recvCounts) nodeSize += count;
        recvCounts.clear();

        CollectDoubleVectorsOnRank(globalZoneTable, recvCounts, zoneData, 0);
        long zoneSize = 0;
        for(const auto count : recvCounts) zoneSize += count;
        recvCounts.clear();

        if(PAR_Rank() == 0)
        {
            std::cout << "globalNode size " << nodeSize << std::endl;
            std::cout << "globalZone size " << zoneSize << std::endl;
            CombineTables(globalNodeTable, globalZoneTable, outData, fillValue);
            BuildOutputInfo(varNComps, varTypes, nodeSize, zoneSize, outInfo);
        }
    }
}

// ****************************************************************************
//  Method: avtFlattenQuery::SetOutputQueryAtts
//
//  Purpose:
//
//
//  Programmer:   Chris Laganella
//  Creation:     Tue Jan 18 11:39:52 EST 2022
//
//  Modifications:
//
// ****************************************************************************
void
avtFlattenQuery::SetOutputQueryAtts(QueryAttributes *qA, bool hadError)
{
    std::cout << "avtFlattenQuery::SetOutputQueryAtts" << std::endl;
    if(hadError)
    {
        avtDatasetQuery::SetOutputQueryAtts(qA, hadError);
        return;
    }

    if(PAR_Rank() == 0)
    {
        qA->SetXmlResult(outInfo.ToXML());
        qA->SetResultsMessage("Success!");
        qA->GetResultsValue().swap(outData);
        qA->Compress();
        qA->SelectResultsValue();
    }
    std::cout << "avtFlattenQuery - Done." << std::endl;
    // PRINT_BACKTRACE(std::cout);
}

// ****************************************************************************
//  Method: avtFlattenQuery::BuildOutputInfo
//
//  Purpose:
//
//
//  Programmer:   Chris Laganella
//  Creation:     Wed Jan 19 11:57:26 EST 2022
//
//  Modifications:
//
// ****************************************************************************
void
avtFlattenQuery::BuildOutputInfo(const intVector &varNComps, const intVector &varTypes,
                                 const long nodeSize, const long zoneSize,
                                 MapNode &outInfo)
{
    outInfo.Reset();
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

    longVector shape(2, 0);
    if(nodeSize)
    {
        outInfo["nodeColumnNames"] = columnNames[NODE_DATA];
        shape[1] = columnNames[NODE_DATA].size();
        shape[0] = nodeSize / shape[1];
        outInfo["nodeTableShape"] = shape;
    }

    if(zoneSize)
    {
        outInfo["zoneColumnNames"] = columnNames[ZONE_DATA];
        shape[1] = columnNames[ZONE_DATA].size();
        shape[0] = zoneSize / shape[1];
        outInfo["zoneTableShape"] = shape;
        outInfo["zoneTableOffset"] = nodeSize;
    }
}
