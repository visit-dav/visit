// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
#include <utility>
#include <set>

#include <visit-config.h>

#include <cmath>


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
//    Hank Childs, Thu Jan 22 15:42:54 PST 2004
//    Removed unused CSetMapperInput.
//
//    Kathleen Bonnell, Wed Feb 18 08:31:26 PST 2004 
//    Added CGetNumberOfRealZones, CGetNumberOfNodes. 
//
//    Hank Childs, Mon Mar  1 07:49:35 PST 2004
//    Modified data extent methods to handle multiple variables.
//
//    Kathleen Bonnell, Thu Mar 11 12:47:55 PST 2004 
//    Removed CGetDataMagnitudeExtents (handled by CGetDataExtents).
//    Added MajorEigenvalue. 
//
//    Hank Childs, Tue Jul 27 08:49:51 PDT 2004
//    Add CConvertUnstructuredGridToPolyData.
//
//    Hank Childs, Tue Oct 11 13:47:59 PDT 2005
//    Add CGetAllDatasets.
//
//    Mark C. Miller, Wed Nov 16 14:17:01 PST 2005
//    Added compression support functions 
//
//    Hank Childs, Tue Jan 16 15:58:02 PST 2007
//    Added CBreakVTKPipelineConnections.
//
//    Jeremy Meredith, Thu Feb 15 13:07:12 EST 2007
//    Added CApplyTransformToRectGrid and
//    CInsertRectilinearTransformInfoIntoDataset.
//
//    Hank Childs, Thu May 31 13:49:59 PDT 2007
//    Added varsizes to VarList structure.
//
//    Hank Childs, Wed Oct 10 15:56:16 PDT 2007
//    Added ignoreGhost as an argument to GetDataRange.
//
//    Hank Childs, Sun Jan 13 14:36:38 PST 2008
//    Added CExpandSingletonConstants.
//
//    Jeremy Meredith, Thu Feb  7 17:54:34 EST 2008
//    Added GetDataAllComponentsRange.
//
//    Hank Childs, Mon May 12 09:12:36 PDT 2008
//    Added CPruneByDomainList
//
//    Kathleen Bonnell, Tue Jul 29 09:50:17 PDT 2008
//    Added CGetNumberOfRealNodes.
//
//    Hank Childs, Fri May 21 11:22:21 CDT 2010
//    Added CCalculateHistogram.
//
//    Hank Childs, Sat Jan  1 17:35:23 PST 2011
//    Moved visitIsFinite to header, so other classes can use it.
//
//    Kathleen Biagas, Mon Jan 28 10:06:01 PST 2013
//    Remove no longer used method CUpdateData.
//
//    Kathleen Biagas, Wed May 28 17:25:57 MST 2014
//    Added connectedNodesOnly to GetVariableRangeArgs.
//
//    Kathleen Biagas, Thu Sep 11 09:29:57 PDT 2014
//    Added 'Original' options for GetNumberOfZones/Nodes.
//
//    Kathleen Biagas, Wed Nov 18 2020
//    Replace VISIT_LONG_LONG with long long.
//
//    Kathleen Biagas, Tue Apr 5 2022
//    Added CGetTopologicalDimension.
//
// ****************************************************************************

//
//  Function prototypes
//

PIPELINE_API void CGetSpatialExtents(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetDataExtents(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CAddInputToAppendFilter(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CBreakVTKPipelineConnections(avtDataRepresentation &, 
                                               void *, bool &);
PIPELINE_API void CCalculateHistogram(avtDataRepresentation &, 
                                               void *, bool &);
PIPELINE_API void CGetAllDatasets(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetNumberOfZones(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetNumberOfOriginalZones(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetChunkByDomain(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CPruneByDomainList(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetChunkByLabel(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CSetActiveVariable(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CRemoveVariable(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetVariableList(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CFindMaximum(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CFindMinimum(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CLocateZone(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CLocateNode(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetArray(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetVariableCentering(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetNumberOfNodes(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetNumberOfOriginalNodes(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetNumberOfRealZones(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetNumberOfRealOriginalZones(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetNumberOfRealNodes(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetNumberOfRealOriginalNodes(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CExpandSingletonConstants(avtDataRepresentation &,
                                                     void *, bool &);
PIPELINE_API void CConvertUnstructuredGridToPolyData(avtDataRepresentation &,
                                                     void *, bool &);
PIPELINE_API void CApplyTransformToRectGrid(avtDataRepresentation &,
                                            void *, bool &);
PIPELINE_API void CInsertRectilinearTransformInfoIntoDataset(
                                                  avtDataRepresentation &,
                                                  void *, bool &);

PIPELINE_API void GetDataRange(vtkDataSet *, double *, const char *, 
                               bool ignoreGhost = false);
PIPELINE_API void GetDataAllComponentsRange(vtkDataSet *, double *,
                                            const char *,
                                            bool ignoreGhost = false);
PIPELINE_API double MajorEigenvalue(float *);
PIPELINE_API double MajorEigenvalue(double *);

PIPELINE_API bool CCompressDataString(const unsigned char *dstr, int len, 
                      unsigned char **newdstr, int *newlen,
                      float *timeToCompress, float *compressionRatio);
PIPELINE_API bool CDecompressDataString(const unsigned char *dstr, int len, 
                      unsigned char **newdstr, int *newlen,
                      float *timeToCompress, float *timeToDecompress,
                      float *compressionRatio);
PIPELINE_API bool CMaybeCompressedDataString(const unsigned char *dstr);
PIPELINE_API void CGetCompressionInfoFromDataString(const unsigned char *dstr,
                      int len, float *timeToCompress, float *compressionRatio);

PIPELINE_API void CGetTopologicalDim(avtDataRepresentation &data, void *info, bool &success);


typedef struct
{
    const char *varname;
    bool        hasPointVars;
    bool        hasCellVars;
    bool        activeIsPoint;
    int         activeVarDim;
} SetActiveVariableArgs;

typedef struct
{
    int  nvars;
    std::vector<std::string> varnames;
    std::vector<int>         varsizes;
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

typedef struct
{
    double       *extents;
    const char   *varname;
    bool          connectedNodesOnly;
} GetVariableRangeArgs;

typedef struct
{
    std::vector<vtkDataSet *> datasets;
    std::vector<int>          domains;
    std::vector<std::string>  labels;
} GetAllDatasetsArgs;

typedef struct
{
    double                         min;
    double                         max;
    std::string                    variable;
    std::vector<long long>   numVals;
} CalculateHistogramArgs;

typedef struct
{
    std::set< std::pair<unsigned int, unsigned int> > elementCount;
    std::set< std::pair<unsigned int, unsigned int> > ghostElementCount;
} OrigElementCountArgs;


// ****************************************************************************
//  Function: visitIsFinite
//
//  Purpose:
//      Determines if a given number is finite.
//
//  Programmer: Hank Childs
//  Creation:   September 19, 2010
//
//  Modifications:
//    Kathleen Bonnell, Mon Sep 20 10:40:15 MST 2010
//    Use _finite on Windows.
//
//    Mark C. Miller, Mon Jan 10 07:20:17 PST 2011
//    Added 'std::' to non-win32 variant. Otherwise, failed to compile on 
//    ancient RH3-gcc-3.2.3 system.
// ****************************************************************************

template <class T>
inline bool visitIsFinite(T t)
{
#ifndef _WIN32
#ifdef HAVE_ISFINITE
    return std::isfinite(t);
#endif
#else
    return _finite(t);
#endif
    return true;
}


// ****************************************************************************
//  Function: visitIsNan
//
//  Purpose:
//      Determines if a given value is nan.
//
//  Programmer: Alister Maguire
//  Creation:   May 28, 2019
//
//  Modifications:
//
// ****************************************************************************

template <class T>
inline bool visitIsNan(T t)
{
#ifndef _WIN32
#ifdef HAVE_ISFINITE
    return std::isnan(t);
#endif
#else
    return _isnan(t);
#endif
    return true;
}


#endif


