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
#include <Expression.h>
#include <ExpressionList.h>
#include <ParsingExprList.h>
#include <QueryArgumentException.h>
#include <UnexpectedValueException.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <array>
#include <cstdint>
#include <cstring>

#ifndef WIN32
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#endif

using FloatType = float;

const int avtFlattenQuery::NODE_DATA = 0;
const int avtFlattenQuery::ZONE_DATA = 1;

struct avtFlattenQuery::impl
{
    MapNode outInfo;
    stringVector variables;
    std::vector<FloatType> outData;

    // Options
    std::string sharedMemoryName = "avtFlattenQuery";
    double fillValue = 0.;
    double maxDataSize = 1.024;
    int useSharedMemory = false;
    int nodeIds = true;
    int zoneIds = true;
    int nodeIJK = true;
    int zoneIJK = true;
    int zoneCenters = false;
};

// ****************************************************************************
//  Function: CombineTables
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
template<typename T>
static void
CombineTables(const std::vector<T> &nodeTable, const std::vector<T> &zoneTable,
    std::vector<T> &out, double fillValue)
{
    const auto nodeSize = nodeTable.size();
    const auto zoneSize = zoneTable.size();
    out.resize(nodeSize + zoneSize, fillValue);
    void *const zoneOut = out.data() + nodeSize;
    if(nodeSize)
    {
        memcpy(out.data(), nodeTable.data(), nodeSize * sizeof(T));
    }
    if(zoneSize)
    {
        memcpy(zoneOut, zoneTable.data(), zoneSize * sizeof(T));
    }
}

// ****************************************************************************
//  Function: CollectVectors
//
//  Purpose:
//      Helper function to invoke the proper CollectVectors MPI call
//
//  Programmer:   Chris Laganella
//  Creation:     Thu Jan 27 11:24:30 EST 2022
//
//  Modifications:
//
// ****************************************************************************
static void
CollectVectors(std::vector<float> &globalTable, std::vector<int> &recvCounts,
    const std::vector<float> &localTable)
{
    CollectFloatVectorsOnRootProc(globalTable, recvCounts, localTable);
}

static void
CollectVectors(std::vector<double> &globalTable, std::vector<int> &recvCounts,
    const std::vector<double> &localTable)
{
    CollectDoubleVectorsOnRootProc(globalTable, recvCounts, localTable);
}

// ****************************************************************************
//  Function: SwapIntoQueryAttributes
//
//  Purpose:
//      Helper function used to swap with the float or double vector in
//      query attributes.
//
//  Programmer:   Chris Laganella
//  Creation:     Fri Feb  4 10:49:14 EST 2022
//
//  Modifications:
//
// ****************************************************************************
static void
SwapIntoQueryAttributes(QueryAttributes &qA, std::vector<float> &outData)
{
    std::vector<float> &out = qA.GetFloatResultsValue();
    out = std::move(outData);
    qA.SelectFloatResultsValue();
}

static void
SwapIntoQueryAttributes(QueryAttributes &qA, std::vector<double> &outData)
{
    std::vector<double> &out = qA.GetResultsValue();
    out = std::move(outData);
    qA.SelectResultsValue();
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
    : pimpl()
{
    pimpl.reset(new avtFlattenQuery::impl);
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
//  Chris Laganella, Mon Jan 31 17:58:13 EST 2022
//  Updated the function to define node/zone id/IJK expressions & add them
//  to the variable list if these values are desired in the output table.
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
            pimpl->variables = vars->AsStringVector();
        }
    }

    if(params.HasEntry("fillValue"))
    {
        const MapNode *fv = params.GetEntry("fillValue");
        if(fv->Type() == MapNode::DOUBLE_TYPE)
        {
            pimpl->fillValue = fv->AsDouble();
        }
    }

#ifndef WIN32
    if(params.HasEntry("useSharedMemory"))
    {
        const MapNode *useShm = params.GetEntry("useSharedMemory");
        if(useShm->Type() == MapNode::INT_TYPE)
        {
            pimpl->useSharedMemory = useShm->AsInt();
        }

        if(pimpl->useSharedMemory)
        {
            const MapNode *shmName = params.GetEntry("sharedMemoryName");
            if(shmName && shmName->Type() == MapNode::STRING_TYPE)
            {
                pimpl->sharedMemoryName = shmName->AsString();
            }
        }
    }
#endif

    if(params.HasEntry("nodeIds"))
    {
        const MapNode *doNodeIds = params.GetEntry("nodeIds");
        if(doNodeIds->Type() == MapNode::INT_TYPE)
        {
            pimpl->nodeIds = doNodeIds->AsInt();
        }
    }

    if(params.HasEntry("zoneIds"))
    {
        const MapNode *doZoneIds = params.GetEntry("zoneIds");
        if(doZoneIds->Type() == MapNode::INT_TYPE)
        {
            pimpl->zoneIds = doZoneIds->AsInt();
        }
    }

    if(params.HasEntry("nodeIJK"))
    {
        const MapNode *doNodeIJK = params.GetEntry("nodeIJK");
        if(doNodeIJK->Type() == MapNode::INT_TYPE)
        {
            pimpl->nodeIJK = doNodeIJK->AsInt();
        }
    }

    if(params.HasEntry("zoneIJK"))
    {
        const MapNode *doZoneIJK = params.GetEntry("zoneIJK");
        if(doZoneIJK->Type() == MapNode::INT_TYPE)
        {
            pimpl->zoneIJK = doZoneIJK->AsInt();
        }
    }

    if(params.HasEntry("zoneCenters"))
    {
        const MapNode *doZoneCenters = params.GetEntry("zoneCenters");
        if(doZoneCenters->Type() == MapNode::INT_TYPE)
        {
            pimpl->zoneCenters = doZoneCenters->AsInt();
        }
    }

    if(pimpl->variables.empty())
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
    params["use_actual_data"] = 1;
    // Start with 40 MB as this takes ~10 sec on my machine
    //  the Flatten CLI function will update this to 1GB by default.
    // This small value should only be used if avtFlattenQuery
    //  is called by the gui, which has no use.
    params["maxDataSize"] = 0.04;

    params["fillValue"] = 0.;
    params["useSharedMemory"] = 0;
    params["sharedMemoryName"] = "avtFlattenQuery";

    params["nodeIds"] = 1;
    params["zoneIds"] = 1;
    params["nodeIJK"] = 1;
    params["zoneIJK"] = 1;
    params["zoneCenters"] = 0;
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
//  Chris Laganella, Mon Feb  7 20:27:01 EST 2022
//  I updated the function to add expressions for nodeid/zoneid/IJK/centers
//  when requested by the user.
// ****************************************************************************
void
avtFlattenQuery::GetSecondaryVars(stringVector &outVars)
{
    outVars.clear();

    if(pimpl->nodeIds || pimpl->nodeIJK || pimpl->zoneIds || pimpl->zoneIJK)
    {
        ExpressionList *exprList = ParsingExprList::Instance()->GetList();
        const auto &dataAtts = GetTypedInput()->GetInfo().GetAttributes();
        const std::string meshName = dataAtts.GetMeshname();

        if(pimpl->zoneCenters)
        {
            Expression e;
            e.SetType(Expression::VectorMeshVar);
            e.SetName("zoneCenters");
            e.SetDefinition("zone_centers(" + meshName + ")");
            exprList->AddExpressions(e);
            pimpl->variables.push_back("zoneCenters");
        }

        if(pimpl->nodeIds)
        {
            Expression e;
            e.SetType(Expression::ScalarMeshVar);
            e.SetName("nodeIds");
            e.SetDefinition("nodeid(" + meshName + ")");
            exprList->AddExpressions(e);
            pimpl->variables.push_back("nodeIds");
        }

        if(pimpl->nodeIJK)
        {
            Expression e;
            e.SetType(Expression::VectorMeshVar);
            e.SetName("nodeIJK");
            e.SetDefinition("logical_nodeid(" + meshName + ")");
            exprList->AddExpressions(e);
            pimpl->variables.push_back("nodeIJK");
        }

        if(pimpl->zoneIds)
        {
            Expression e;
            e.SetType(Expression::ScalarMeshVar);
            e.SetName("zoneIds");
            e.SetDefinition("zoneid(" + meshName + ")");
            exprList->AddExpressions(e);
            pimpl->variables.push_back("zoneIds");
        }

        if(pimpl->zoneIJK)
        {
            Expression e;
            e.SetType(Expression::VectorMeshVar);
            e.SetName("zoneIJK");
            e.SetDefinition("logical_zoneid(" + meshName + ")");
            exprList->AddExpressions(e);
            pimpl->variables.push_back("zoneIJK");
        }

        // Add domain id if ids or IJKs were requested
        if(pimpl->nodeIds || pimpl->nodeIJK)
        {
            Expression e;
            e.SetType(Expression::ScalarMeshVar);
            e.SetName("nodeDomains");
            e.SetDefinition("node_domain(" + meshName + ")");
            exprList->AddExpressions(e);
            pimpl->variables.push_back("nodeDomains");
        }

        if(pimpl->zoneIds || pimpl->zoneIJK)
        {
            Expression e;
            e.SetType(Expression::ScalarMeshVar);
            e.SetName("zoneDomains");
            e.SetDefinition("zone_domain(" + meshName + ")");
            exprList->AddExpressions(e);
            pimpl->variables.push_back("zoneDomains");
        }
    }

    for(const auto &var : pimpl->variables)
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
    pimpl->outData.clear();
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
    debug1 << "avtFlattenQuery::Execute(vtkDataSet*, const int), "
        << "this function should never have been invoked!" << std::endl;
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
    debug5 << "avtFlattenQuery::Execute(avtDataTree_p)" << std::endl;
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
    for(const std::string &name : pimpl->variables)
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

    // NOTE: This can't happen as varTypes & varNComps are built by
    //   iterating over each item in variables once. More of a debug assert.
    // if(pimpl->variables.size() != varTypes.size()
    //     || variables.size() != varNComps.size())
    // {
    //     debug1 << "variables.size(), varTypes.size() and varNComps.size() are "
    //         << "not equal! Cannot continue executing avtFlattenQuery." << std::endl;
    //     return;
    // }

    // Build the output tables
    std::array<int, 2> tableNCol{0, 0};
    std::vector<int> varOffsets;
    for(std::size_t i = 0; i < pimpl->variables.size(); i++)
    {
        const int type = varTypes[i];
        varOffsets.push_back(tableNCol[type]);
        tableNCol[type] += varNComps[i];
        debug5 << pimpl->variables[i] << ": ncomps " << varNComps[i]
            << ", centering " << varTypes[i] << std::endl;
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
    std::vector<FloatType> nodeData(tableSizes[NODE_DATA], pimpl->fillValue);
    std::vector<FloatType> zoneData(tableSizes[ZONE_DATA], pimpl->fillValue);
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
        for(std::size_t varIdx = 0; varIdx < pimpl->variables.size(); varIdx++)
        {
            const std::string varName = pimpl->variables[varIdx];
            const int type = varTypes[varIdx];
            const int nc = varNComps[varIdx];
            const int stride = tableNCol[type];
            const vtkIdType N = blockSizes[type][blockIdx];
            vtkDataArray *da = dataAtts[varTypes[varIdx]]->GetArray(varName.c_str());
            std::vector<FloatType> &out = (type == NODE_DATA) ? nodeData : zoneData;
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
                        out[outIdx + c] = (FloatType)tup[c];
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
        CombineTables(nodeData, zoneData, pimpl->outData, pimpl->fillValue);
        BuildOutputInfo(varNComps, varTypes, nodeData.size(), zoneData.size(), pimpl->outInfo);
    }
    else
    {
        std::vector<FloatType> globalNodeTable;
        std::vector<FloatType> globalZoneTable;

        debug5 << "Rank " << PAR_Rank() << " contributing (node) " << nodeData.size() << std::endl;
        debug5 << "Rank " << PAR_Rank() << " contributing (zone) " << zoneData.size() << std::endl;

        std::vector<int> recvCounts;
        CollectVectors(globalNodeTable, recvCounts, nodeData);
        long nodeSize = 0;
        for(const auto count : recvCounts) nodeSize += count;
        recvCounts.clear();

        CollectVectors(globalZoneTable, recvCounts, zoneData);
        long zoneSize = 0;
        for(const auto count : recvCounts) zoneSize += count;
        recvCounts.clear();

        if(PAR_Rank() == 0)
        {
            debug5 << "globalNode size " << nodeSize << std::endl;
            debug5 << "globalZone size " << zoneSize << std::endl;
            CombineTables(globalNodeTable, globalZoneTable, pimpl->outData, pimpl->fillValue);
            BuildOutputInfo(varNComps, varTypes, nodeSize, zoneSize, pimpl->outInfo);
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
    if(hadError)
    {
        avtDatasetQuery::SetOutputQueryAtts(qA, hadError);
        return;
    }

    if(PAR_Rank() == 0)
    {
        const double sizeInGigaBytes = (pimpl->outData.size() * sizeof(FloatType)) / (double)1e9;
        debug5 << "avtFlattenQuery XML output:\n" << pimpl->outInfo.ToXML() << std::endl;
        qA->SetXmlResult(pimpl->outInfo.ToXML());
        qA->SetResultsMessage("Success!\nNOTE: This query should only "
                        "be used in the CLI via the Flatten() function.");
        if(pimpl->useSharedMemory)
        {
            WriteSharedMemory();
        }
        else if(sizeInGigaBytes > pimpl->maxDataSize)
        {
            std::stringstream s;
            s << "ERROR: Data too large to transport via query attributes ("
                << sizeInGigaBytes << "GB). You can override the limit by"
                << " overriding the parameter 'maxDataSize' with a (double)"
                << " size repesented in Gigabytes. (Current value = "
                << pimpl->maxDataSize << ").\nNOTE: This query should only "
                << "be used in the CLI via the Flatten() function.";
            qA->SetResultsMessage(s.str());
            debug1 << s.str() << std::endl;
        }
        else
        {
            SwapIntoQueryAttributes(*qA, pimpl->outData);
        }
    }
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
    for(std::size_t i = 0; i < pimpl->variables.size(); i++)
    {
        const std::string &name = pimpl->variables[i];
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

    outInfo["totalSize"] = static_cast<long>(pimpl->outData.size());
    if(std::is_same<float, FloatType>::value)
    {
        outInfo["dataType"] = "float";
    }
    else
    {
        outInfo["dataType"] = "double";
    }

    outInfo["useSharedMemory"] = pimpl->useSharedMemory;
    if(pimpl->useSharedMemory)
    {
        outInfo["sharedMemoryName"] = pimpl->sharedMemoryName;
    }
}

// ****************************************************************************
//  Method: avtFlattenQuery::WriteSharedMemory
//
//  Purpose:
//      Write the results of the flatten query to shared memory.
//
//
//  Programmer:   Chris Laganella
//  Creation:     Wed Jan 19 11:57:26 EST 2022
//
//  Modifications:
//
// ****************************************************************************
void
avtFlattenQuery::WriteSharedMemory() const
{
#ifndef WIN32
    int fd = shm_open(pimpl->sharedMemoryName.c_str(),
                        O_CREAT | O_RDWR | O_EXCL,
                        S_IREAD | S_IWRITE);
    if(fd == -1)
    {
        debug1 << "Error opening shared memory block with the name "
            << pimpl->sharedMemoryName << std::endl;
        return;
    }

    const unsigned long shmSize = pimpl->outData.size() * sizeof(FloatType);
    int err = ftruncate(fd, shmSize);
    if(err == -1)
    {
        debug1 << "Could not truncate shared memory block to size "
            << shmSize << "." << std::endl;
        close(fd);
        return;
    }

    void *const buff = mmap(NULL, shmSize, PROT_WRITE, MAP_SHARED, fd, 0);
    if(buff == NULL || buff == MAP_FAILED)
    {
        debug1 << "Failed to mmap the shared memory block." << std::endl;
        close(fd);
        return;
    }

    memcpy(buff, pimpl->outData.data(), shmSize);
    munmap(buff, shmSize);
    close(fd);
#endif
}
