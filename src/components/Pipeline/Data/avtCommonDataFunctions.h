// ************************************************************************* //
//                      avtCommonDataFunctions.h                             //
// ************************************************************************* //

#ifndef AVT_COMMON_DATA_FUNCTIONS_H
#define AVT_COMMON_DATA_FUNCTIONS_H

#include <pipeline_exports.h>

#include <avtDataRepresentation.h>
#include <avtTypes.h>

#include <vector>
#include <string>


class     vtkDataArray;


// ****************************************************************************
//  Purpose:
//    Provides single place to store common data functions.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     April 17, 2001 
//
//  Modifications:
//
//    Hank Childs, Fri Nov  2 16:28:33 PST 2001
//    Added CRemoveVariable.
//
//    Hank Childs, Fri Mar 15 18:53:44 PST 2002 
//    Added CFindMinimum/CFindMaximum.
//
//    Brad Whitlock, Fri Nov 22 10:36:39 PDT 2002
//    I added PIPELINE_API so the functions are exported from the pipeline
//    DLL. I had to do this because the new expressions library (formerly
//    part of pipeline) uses these functions.
//
//    Brad Whitlock, Wed Dec 4 11:48:33 PDT 2002
//    I added CGetDataMagnitudeExtents and GetDataMagnitudeRange.
//
// ****************************************************************************

//
//  Function prototypes
//

PIPELINE_API void CGetSpatialExtents(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetDataExtents(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetDataMagnitudeExtents(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetNodeCenteredDataExtents(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CUpdateData(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CAddInputToAppendFilter(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetNumberOfZones(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CSetMapperInput(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetChunkByDomain(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetChunkByLabel(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CSetActiveVariable(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CRemoveVariable(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetVariableList(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CFindMaximum(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CFindMinimum(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CLocateZone(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CLocateNode(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetArray(avtDataRepresentation &, void *, bool &);

PIPELINE_API void GetDataRange(vtkDataSet *, double *, int);
PIPELINE_API void GetDataMagnitudeRange(vtkDataSet *, double *, int);
PIPELINE_API void GetPointDataRange(vtkDataSet *, double *);


typedef struct
{
    const char *varname;
    bool        hasPointVars;
    bool        hasCellVars;
    bool        activeIsPoint;
} SetActiveVariableArgs;

typedef struct
{
    int  nvars;
    std::vector<std::string> varnames;
} VarList;

typedef struct
{
    double   value;
    double   point[3];
}  FindExtremeArgs;

typedef struct
{
    int     domain;
    int     index;
    double  point[3];
}  LocateObjectArgs;

typedef struct
{
    vtkDataArray   *arr;
    int             domain;
    const char     *varname;
    avtCentering    centering;
} GetArrayArgs;

#endif


