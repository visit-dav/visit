// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_PLOT_META_DATA_H
#define AVT_PLOT_META_DATA_H
#include <plotter_exports.h>
#include <avtExtents.h>
#include <avtSILRestriction.h>
#include <avtTypes.h>
#include <avtDatabaseMetaData.h>
#include <string>

// ****************************************************************************
// Class: avtPlotMetaData
//
// Purpose:
//   This class contains the metadata that we use to initialize plot and 
//   operator plugins. We use this class so we can initialize both viewer and
//   engine plugins the same way without resorting to ViewerPlot.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 10:34:05 PDT 2014
//
// Modifications:
//    Mark C. Miller, Mon Nov  2 16:58:54 PST 2020
//    Add bg/fg color members and methods
// ****************************************************************************

class PLOTTER_API avtPlotMetaData
{
public:
    avtPlotMetaData(const avtDatabaseMetaData  *_md,
                    const std::string          &_variableName,
                    avtVarType                  _variableType,
                    avtSILRestriction_p         _silr,
                    const avtExtents           &_actualSpatial,
                    const avtExtents           &_originalSpatial,
                    const double                _bgColor[4],
                    const double                _fgColor[4]);
    avtPlotMetaData(const avtPlotMetaData &obj);
    ~avtPlotMetaData();

    void operator = (const avtPlotMetaData &obj);

    const avtDatabaseMetaData *GetMetaData() const;
    const std::string         &GetVariableName() const;
    std::string                GetMeshName() const;
    avtVarType                 GetVariableType() const;
    avtSILRestriction_p        GetSILRestriction() const;
    const avtExtents          &GetActualSpatialExtents() const;
    const avtExtents          &GetOriginalSpatialExtents() const;
    const double              *GetBackgroundColor() const;
    const double              *GetForegroundColor() const;

private:
    const avtDatabaseMetaData *md;
    std::string                variableName;
    avtVarType                 variableType;
    avtSILRestriction_p        silr;
    avtExtents                 actualSpatialExtents;
    avtExtents                 originalSpatialExtents;
    double                     bgColor[4];
    double                     fgColor[4];
};

#endif
