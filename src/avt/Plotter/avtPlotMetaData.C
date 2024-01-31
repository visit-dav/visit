// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtPlotMetaData.h>

avtPlotMetaData::avtPlotMetaData(const avtDatabaseMetaData  *_md,
    const std::string          &_variableName,
    avtVarType                 _variableType,
    avtSILRestriction_p        _silr,
    const avtExtents           &_actualSpatialExt,
    const avtExtents           &_originalSpatialExt,
    const double               _bgColor[4],
    const double               _fgColor[4]) : 
    md(_md),
    variableName(_variableName),
    variableType(_variableType),
    silr(_silr),
    actualSpatialExtents(_actualSpatialExt),
    originalSpatialExtents(_originalSpatialExt),
    bgColor{_bgColor[0],_bgColor[1],_bgColor[2],_bgColor[3]},
    fgColor{_fgColor[0],_fgColor[1],_fgColor[2],_fgColor[3]}
{
}

avtPlotMetaData::avtPlotMetaData(const avtPlotMetaData &obj) :
    md(obj.md),
    variableName(obj.variableName),
    variableType(obj.variableType),
    silr(obj.silr),
    actualSpatialExtents(obj.actualSpatialExtents),
    originalSpatialExtents(obj.originalSpatialExtents),
    bgColor{obj.bgColor[0],obj.bgColor[1],obj.bgColor[2],obj.bgColor[3]},
    fgColor{obj.fgColor[0],obj.fgColor[1],obj.fgColor[2],obj.fgColor[3]}
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
    for (int i = 0; i < 4; i++)
    {
        bgColor[i] = obj.bgColor[i];
        fgColor[i] = obj.fgColor[i];
    }
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

const double *
avtPlotMetaData::GetBackgroundColor() const
{
    return bgColor;
}

const double *
avtPlotMetaData::GetForegroundColor() const
{
    return fgColor;
}
