/*****************************************************************************
 *
 * Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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
//                             Spy.h                                         //
// ************************************************************************* //

#ifndef SPY_H
#define SPY_H

#define MAGIC_STR_BYTES 8
#define FILE_TITLE_BYTES 128
#define INT_BYTES 4
#define DOUBLE_BYTES 8

#define REAL double

#define MAX_DUMPS   100
#define EV2K 11604.5

/* Cell fields */

#define STM_VOID 0
#define STM_P 1
#define STM_T 2
#define STM_CS 3
#define STM_VX 4
#define STM_VY 5
#define STM_VZ 6
#define STM_VMAG 7
#define STM_XXDEV 8
#define STM_XYDEV 9
#define STM_YYDEV 10
#define STM_XZDEV 11
#define STM_YZDEV 12
#define STM_DENS 13
#define STM_HE1 14
#define STM_HE2 15
#define STM_RMFP 16

/* Material fields */

#define STM_VOLM 100000
#define STM_M 200000
#define STM_PM 300000
#define STM_TM 400000
#define STM_EM 500000
#define STM_CSM 600000

/* Derived fields */

#define STM_ZZDEV -19
#define STM_XXSTRESS -20
#define STM_YYSTRESS -21
#define STM_ZZSTRESS -22
#define STM_KE -23
#define STM_TK -24
#define STM_J2P -25
#define STM_CVMAG -26
#define STM_DIVV -27
#define STM_IE -28
#define STM_J2PP -29
#define STM_MVOL -30
#define STM_X -31
#define STM_Y -32
#define STM_Z -33
#define STM_XC -34
#define STM_YC -35
#define STM_ZC -36
#define STM_CVX -37
#define STM_CVY -38
#define STM_CVZ -39
#define STM_LX -40
#define STM_LY -41
#define STM_LZ -42
#define STM_DX -43
#define STM_DY -44
#define STM_DZ -45
#define STM_VM -46

#define STM_DENSM_1 -2200
#define STM_DENSM   -2300
#define STM_TKM_1 -1400
#define STM_TKM   -1500

/* Note: field id's <= -1000000 are reserved for user defined variables */
#define STM_UVAR   -1000000
#define STM_UVAR2  -2000000

// ****************************************************************************
//  Struct: Histogram
//
//  Purpose:
//     No Info
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
// ****************************************************************************

struct Histogram
{
    Histogram() : numIndicators(0), maxBin(0), histogram(NULL), histMin(NULL),
    histMax(NULL), histType(NULL), refAbove(NULL), refBelow(NULL),
    unrAbove(NULL), unrBelow(NULL), nBins(NULL)
    {}
    
    int numIndicators;
    int maxBin;
    
    double  **histogram;
    double  *histMin;
    double  *histMax;
    int     *histType;
    double  *refAbove;
    double  *refBelow;
    double  *unrAbove;
    double  *unrBelow;
    int     *nBins;
};

// ****************************************************************************
//  Struct: Tracer
//
//  Purpose:
//     No Info
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
// ****************************************************************************

struct Tracer {
    Tracer() : numTracers(0), xTracer(NULL), yTracer(NULL), zTracer(NULL),
    lTracer(NULL), iTracer(NULL), jTracer(NULL), kTracer(NULL)
    { }
    
    // Number of tracers
    int numTracers;
    
    // x,y,z tracers
    double *xTracer;
    double *yTracer;
    double *zTracer;
    
    // Tracer location logicals
    // Note: these indices use FORTRAN array ordering (first location is 1)
    // Tracer block id if (lTracer[n] <= 0) then tracer is not active or not on this processor
    int *lTracer;
    // Tracer cell location indices (within a block)
    // iTracer[n] = X index
    int *iTracer;
    // jTracer[n] = Y index
    int *jTracer;
    // kTracer[n] = Z index
    int *kTracer;
};

// ****************************************************************************
//  Struct: DataBlock
//
//  Purpose:
//      Data blocks hold the meta data about each block.
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
// ****************************************************************************

struct DataBlock {
    DataBlock(): nBlocks(0), nx(NULL), ny(NULL), nz(NULL), allocated(NULL),
    active(NULL), level(NULL), bxBot(NULL), bxTop(NULL), byBot(NULL), byTop(NULL),
    bzBot(NULL), bzTop(NULL), xBytes(NULL), x(NULL), yBytes(NULL), y(NULL),
    zBytes(NULL), z(NULL), cField(NULL), mField(NULL)
    {}
    
    int nBlocks;
    int *nx;
    int *ny;
    int *nz;
    int *allocated;
    int *active;
    int *level;
    
    // Spy Version >= 103
    // Boundary Conditions:
    //     -1 = internal boundary, ghost cells contain valid data
    //      0 = reflecting boundary, ghost cells should be copied from adjancent internal cells
    //     >0 = other boundary condition
    int *bxBot;
    int *bxTop;
    int *byBot;
    int *byTop;
    int *bzBot;
    int *bzTop;
    
    // Geometry Arrays
    int     *xBytes;
    double  **x;
    int     *yBytes;
    double  **y;
    int     *zBytes;
    double  **z;
    
    // Cell Field
    // CField[0]    - Void volume fraction - %
    // CField[1]    - P                    - dyn/cm^2
    // CField[2]    - T                    - eV
    // CField[3]    - Sound Speed          - cm/s
    // CField[4]    - X velocity           - cm/s
    // CField[5]    - Y velocity           - cm/s
    // CField[6]    - Z velocity           - cm/s
    // CField[7]    - Vel. Mag.            - cm/s
    // CField[8]    - XX stress deviator   - dyn/cm^2
    // CField[9]    - XY stress deviator   - dyn/cm^2
    // CField[10]   - YY stress deviator   - dyn/cm^2
    // CField[11]   - XZ stress deviator   - dyn/cm^2
    // CField[12]   - YZ stress deviator   - dyn/cm^2
    // CField[13]   - Density              - g/cm^3
    // CField[14]   - Burn Time            - s
    // CField[15]   - Burn Energy          - erg
    double *****cField;
    
    // Material Field
    // MField[0][m] - Volume Fraction      - %
    // MField[1][m] - Mass                 - g
    // MField[2][m] - Pressure             - dyn/cm^2
    // MField[3][m] - Temperature          - eV
    // MField[4][m] - Specific Energy      - erg/g
    double ******mField;
};

// ****************************************************************************
//  Struct: MarkerBlock
//
//  Purpose:
//      Marker blocks hold the meta data about each marker material. Only
//      read in if SpyVersions >= 105.
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
// ****************************************************************************

struct MarkerBlock {
    MarkerBlock() : iloc(NULL), jloc(NULL), kloc(NULL),
                    xloc(NULL), yloc(NULL), zloc(NULL),
                    bmark(NULL), vars(NULL)
    { }
    
    // i,j,k index locations of markers
    int **iloc;
    int **jloc;
    int **kloc;
    
    // x,y,z locations of markers
    double **xloc;
    double **yloc;
    double **zloc;
    
    int **bmark;
    
    // Variable values
    REAL ***vars;
};

// ****************************************************************************
//  Struct: MarkerHeader
//
//  Purpose:
//      The marker header contains information specific to the material point
//      method.
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
// ****************************************************************************

struct MarkerHeader {
    MarkerHeader() : numMarkers(0), numRealMarkers(0), junk(NULL), numVars(0), varnames(NULL), varlabels(NULL)
    {}
    
    // Number of markers for the material
    int numMarkers;
    // Number of markers not counting ghost markers
    int numRealMarkers;
    // Dummy flags for future capabilities
    int *junk;
    // Number of marker variables
    int numVars;
    
    // Variable names
    char **varnames;
    // Variable labels
    char **varlabels;
};

// ****************************************************************************
//  Struct: Field
//
//  Purpose:
//      Stores information for a cell or material field.
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
// ****************************************************************************

struct Field
{
    Field() : fieldID(NULL), fieldComment(NULL), fieldInt(0)
    {}
    
    // Field ID
    char *fieldID;
    // Field comment
    char *fieldComment;
    // Field Int (only for m_fileversion > 101)
    int fieldInt;
};

// ****************************************************************************
//  Struct: Dump
//
//  Purpose:
//      Stores information about a particular dump (time step).
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
// ****************************************************************************

struct Dump
{
    Dump(int c, double t, double dt, double off) : loaded(false), cycle(c), time(t), dTime(dt), offset(off)
    {}
    
    bool loaded;
    // Computational cycles (indirectly a measure of time)
    const int cycle;
    const double time;
    const double dTime;
    const double offset;
};

// ****************************************************************************
//  Struct: DumpHeader
//
//  Purpose:
//      The dump header contains information specific to each dump.
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
// ****************************************************************************

struct DumpHeader
{
    DumpHeader() : numVars(0), vars(NULL), varsOffset(NULL)
    {}
    
    int numVars;    // NumSavedVariables
    int *vars;      // SavedVariables
    double *varsOffset; // SavedVariablesOffsets
};


// ****************************************************************************
//  Struct: GroupHeader
//
//  Purpose:
//      The group header contains some meta-data about the all of the ‘dumps’
//      including time, cycles, and file pointer into the data block for each
//      time step.
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
// ****************************************************************************

struct GroupHeader
{
    GroupHeader() : dumpCycles(NULL), dumpTimes(NULL), dumpDT(NULL),
                    dumpOffsets(NULL), lastOffset(0)
    {}
    
    // Dump cycles
    int *dumpCycles;
    // Dump Times
    double *dumpTimes;
    //
    double *dumpDT;
    // Dump offesets
    double *dumpOffsets;
    // Last offset
    double lastOffset;
};

// ****************************************************************************
//  Struct: FileHeaderInfo
//
//  Purpose:
//      Store the SPCTH (Spy) file header information.
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
// ****************************************************************************

struct FileHeaderInfo
{
    FileHeaderInfo() : magicStr(NULL), title(NULL), version(-1), fileptrSize(0), compression(0),
                       processorId(0), nprocs(0), igm(0), ndim(0), nmat(0), maxMat(0), gMin(NULL),
                       gMax(NULL), blocks(0), markIsOn(0), markerHeaders(NULL), maxLevel(0), mfIsOn(0),
                       headerJunk(NULL), nCellFields(0), cellFields(NULL), nMatFields(0), matFields(NULL),
                       firstGroup(0), ndumps(0), dumps(NULL)
    { }
    
    char    *magicStr;
    char    *title;
    int     version;
    int     fileptrSize;
    int     compression;
    int     processorId;
    int     nprocs;
    /**
     * IGM is the geometry flag.
     * IGM = 10, 1D planar geometry
     * IGM = 11, 1D cylindrical geometry
     * IGM = 12, 1D spherical geometry
     * IGM = 20, 2D rectangular geometry
     * IGM = 21, 2D cylindrical geometry
     * IGM = 30, 3D rectangular geometry
     **/
    int     igm;
    int     ndim;
    int     nmat;
    int     maxMat;
    
    double  *gMin;
    double  *gMax;
    
    int     blocks;
    int     markIsOn;
    
    MarkerHeader    *markerHeaders; // per material
    
    int     maxLevel;
    int     mfIsOn;
    
    int     *headerJunk;
    
    int     nCellFields;
    Field   *cellFields;
    
    int     nMatFields;
    Field   *matFields;
    
    double  firstGroup;
    int     ndumps;
    std::vector<Dump *>     *dumps;
    
};

// ****************************************************************************
//  Struct: TimestepData
//
//  Purpose:
//      Store data for the current timestep
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
// ****************************************************************************

struct TimestepData
{
    TimestepData() : dumpHeader(NULL), tracer(NULL), histogram(NULL), dataBlock(NULL), markerBlock(NULL)
    {}
    
    DumpHeader      *dumpHeader;
    Tracer          *tracer;
    Histogram       *histogram;
    DataBlock       *dataBlock;
    MarkerBlock     *markerBlock;
};

#endif
