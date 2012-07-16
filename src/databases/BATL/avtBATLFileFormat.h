/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            avtBATLFileFormat.h                            //
// ************************************************************************* //

#ifndef AVT_BATL_FILE_FORMAT_H
#define AVT_BATL_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <map>
#include <string>
#include <vector>

#include <hdf5.h>

#include <vtkPolyData.h>

class     DBOptionsAttributes;


// ****************************************************************************
//  Class: avtBATLFileFormat
//
//  Purpose:
//      Reads in BATL files as a plugin to VisIt.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 23, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Aug 25 15:07:36 PDT 2005
//    Added particle support.
//
//    Jeremy Meredith, Tue Sep 27 14:22:29 PDT 2005
//    Added "new" style particle support.
//
//    Hank Childs, Fri Apr 28 14:20:35 PDT 2006
//    Added activate timestep.
//
//    Kathleen Bonnell, Thu Jul 20 11:22:13 PDT 2006
//    Added methods and structs to support for BATL3 formats.
//
//    Mark C. Miller, Mon Mar  5 22:04:50 PST 2007
//    Added support for Cycles/Times. Added methods to explicitly initialize
//    and finalize the hdf5 library
//
//    Mark C. Miller, Tue Mar  6 23:41:33 PST 2007
//    Removed simParamsHaveBeenRead; Added bool to ReadSimulationParameters
//
//    Randy HUDSON, Tue, Apr 3, 2007
//    Added support for Morton curve
//
//    Randy HUDSON, Mon, June 18, 2007
//    Added support for creating subsets of Morton curve.
//
//    Randy HUDSON, July, 2007
//    Added support for subsets by processor number.
//
//    Randy Hudson, August, 2007
//    Added support for concurrent subsets by block-level and block-processornumber
//    pairs for both the domain and Morton curve.
//
//    Randy Hudson, February, 2008
//    Added struct for "sim info" HDF5 DATASET so "file format version" can be read
//
//    Hank Childs, Fri Dec 11 13:15:13 PST 2009
//    Add support for the new, more efficient way of handling AMR meshes in the
//    SIL.
//
//    Hank Childs, Thu Dec 17 14:07:52 PST 2009
//    Add support for database options.
//
//    Hank Childs, Sat Mar 20 20:21:47 PDT 2010
//    Add data members for more database options.
//
// ****************************************************************************

class avtBATLFileFormat : public avtSTMDFileFormat
{
public:
    avtBATLFileFormat(const char *, DBOptionsAttributes *&);
    virtual           ~avtBATLFileFormat();

    virtual bool           HasInvariantMetaData(void) const
    {
        return false;
    };
    virtual bool           HasInvariantSIL(void) const
    {
        return false;
    };

    virtual const char    *GetType(void)
    {
        return "BATL";
    };
    virtual void           FreeUpResources(void);
    virtual void           ActivateTimestep(void);

    int                    GetCycle();
    double                 GetTime();

    virtual void          *GetAuxiliaryData(const char *var, int,
                                            const char *type, void *args,
                                            DestructorFunction &);

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

protected:
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    void ReadAllMetaData();

    void BuildDomainNesting();

//    void ReadSimulationParameters(hid_t file_id, bool timeAndCycleOnly = false);
    void ReadSimulationParameters(hid_t file_id);

    void ReadBlockStructure();
    void ReadBlockExtents();
    void ReadRefinementLevels();
    void ReadUnknownNames();
    void ReadUnknownUnits();
    void ReadAxisLabels();


    void DetermineGlobalLogicalExtentsForAllBlocks();
//    void ReadParticleAttributes();


    // HDF5 lib initialization
    void InitializeHDF5();
    void FinalizeHDF5();

    vtkPolyData* GetMortonCurve();
    vtkPolyData* GetMortonCurveSubset(int domain);

    void ReadCoordinates();
    void ReadIntCoordinates();
    void ReadProcessorNumbers();
    void ReadBlocksPerProc();
    void ReadMortonOrdering();
    void ReadBATLToVisItID();
protected:
    struct SimParams
    {
        int total_blocks;
        double time;
        double timestep;
//        double redshift;
        int typeGeometry;
        int nsteps;
        int nxb;
        int nyb;
        int nzb;
    };

    struct Block
    {
        int ID;
        int level;
        int nodetype;
        double coords[3];
        int procnum;
        double minSpatialExtents[3];
        double maxSpatialExtents[3];
        int    minGlobalLogicalExtents[3];
        int    maxGlobalLogicalExtents[3];

        void Print(ostream&);
    };

protected:
    static int                objcnt;
    std::string               filename;
    int                       dimension;
    int                       logicalDimension;
    int                       numBlocks;
    int                       numLevels;
    int                       amrDimension;
    int                       isCutFile;
    int                       nPlotVars;
    int                       numBlockCells; //remove if unused    int
    int                       numProcessors;
    std::vector<int>          mortonOrder;
    std::vector<int>          blocksPerProc;
    static const int          LEAF_NODE = 1;
    int                       numLeafBlocks;
    int                       fileFormatVersion;
    hid_t                     fileId;
    hid_t                     errStack;
    SimParams                 simParams;
    std::vector<Block>        blocks;
    int                       block_ndims[3];
    int                       block_zdims[3];
    int                       periodicBoundary[3];
    double                    minSpatialExtents[3];
    double                    maxSpatialExtents[3];
    std::vector<double>       minVals;
    std::vector<double>       maxVals;
    std::vector<std::string>  varNames;
    std::vector<std::string>  varUnits;
    std::vector<std::string>  axisLabels;

    std::vector<int>          leafBlocks;

    std::vector<int>          patchesPerLevel;
    std::vector<int>          visitIdToBATLId;
    std::vector<int>          BATLIdToVisitId;
    bool                      showProcessors;
    bool                      newStyleCurves;
    bool                      addStructuredDomainBoundaries;
    bool                      showUnits;
};


#endif
