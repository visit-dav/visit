// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtPlotMetaData.h>

avtPlotMetaData::avtPlotMetaData(const avtDatabaseMetaData  *_md,
    const std::string          &_variableName,
    avtVarType                 _variableType,
    avtSILRestriction_p        _silr,
    const avtExtents           &_actualSpatialExt,
    const avtExtents           &_originalSpatialExt) : 
    md(_md),
    variableName(_variableName),
    variableType(_variableType),
    silr(_silr),
    actualSpatialExtents(_actualSpatialExt),
    originalSpatialExtents(_originalSpatialExt)
{
}

avtPlotMetaData::avtPlotMetaData(const avtPlotMetaData &obj) :
    md(obj.md),
    variableName(obj.variableName),
    variableType(obj.variableType),
    silr(obj.silr),
    actualSpatialExtents(obj.actualSpatialExtents),
    originalSpatialExtents(obj.originalSpatialExtents)
{
}

avtPlotMetaData::~avtPlotMetaData()
{
}

void
avtPlotMetaData::operator = (const avtPlotMetaData &obj)
{
    md = obj.md;
    variableName = obj.variableName;
    variableType = obj.variableType;
    silr = obj.silr;
    actualSpatialExtents = obj.actualSpatialExtents;
    originalSpatialExtents = obj.originalSpatialExtents;
}

const avtDatabaseMetaData *
avtPlotMetaData::GetMetaData() const
{
    return md;
}

const std::string &
avtPlotMetaData::GetVariableName() const
{
    return variableName;
}

std::string
avtPlotMetaData::GetMeshName() const
{
    return md->MeshForVar(variableName);
}

avtVarType
avtPlotMetaData::GetVariableType() const
{
    return variableType;
}

avtSILRestriction_p
avtPlotMetaData::GetSILRestriction() const
{
    return silr;
}

const avtExtents &
avtPlotMetaData::GetActualSpatialExtents() const
{
    return actualSpatialExtents;
}

const avtExtents &
avtPlotMetaData::GetOriginalSpatialExtents() const
{
    return originalSpatialExtents;
}
