/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
// ****************************************************************************

//
//  Function prototypes
//

PIPELINE_API void CGetSpatialExtents(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetDataExtents(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CUpdateData(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CAddInputToAppendFilter(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CBreakVTKPipelineConnections(avtDataRepresentation &, 
                                               void *, bool &);
PIPELINE_API void CGetAllDatasets(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetNumberOfZones(avtDataRepresentation &, void *, bool &);
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
PIPELINE_API void CGetVariableCentering(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetNumberOfNodes(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CGetNumberOfRealZones(avtDataRepresentation &, void *, bool &);
PIPELINE_API void CConvertUnstructuredGridToPolyData(avtDataRepresentation &,
                                                     void *, bool &);

PIPELINE_API void GetDataRange(vtkDataSet *, double *, const char *);
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
} GetVariableRangeArgs;

typedef struct
{
    std::vector<vtkDataSet *> datasets;
    std::vector<int>          domains;
    std::vector<std::string>  labels;
} GetAllDatasetsArgs;


#endif


