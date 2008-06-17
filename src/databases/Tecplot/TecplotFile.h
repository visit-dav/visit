/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#ifndef TECPLOT_FILE_H
#define TECPLOT_FILE_H

#include <string>
#include <vector>
#include <visitstream.h>

//
// This file contains classes that are used to read/write Tecplot binary files.
//

// ****************************************************************************
// Class: TecplotBase
//
// Purpose:
//   Base class for various Tecplot reader objects.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:12:35 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class TecplotBase
{
public:
    TecplotBase();
    virtual ~TecplotBase();

    virtual bool Read(FILE *f) = 0;
    virtual bool Write(FILE *f) = 0;

    int         ReadInt(FILE *);
    void        WriteInt(FILE *f, int i);
    float       ReadFloat(FILE *);
    void        WriteFloat(FILE *f, float f);
    double      ReadDouble(FILE *);
    void        WriteDouble(FILE *f, double d);
    std::string ReadString(FILE *);
    void        WriteString(FILE *, const std::string &);

    static std::string FilterName(const std::string &);

    static bool reverseEndian;
    static int  version;
    static std::string (*filterNameCB)(const std::string &);
};

//
// Classes that are used to read the header section of the file.
//

// ****************************************************************************
// Class: TecplotTitleAndVariables
//
// Purpose:
//   Contains the title of the tecplot file as well as the list of variable
//   names.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:16:32 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class TecplotTitleAndVariables : public TecplotBase
{
public:
    TecplotTitleAndVariables();
    virtual ~TecplotTitleAndVariables();

    virtual bool Read(FILE *f);
    virtual bool Write(FILE *f);

    int                      fileType;
    std::string              title;
    std::vector<std::string> varNames;
    std::vector<std::string> varUnits;
};

// ****************************************************************************
// Class: TecplotDataSetAux
//
// Purpose:
//   Record for the dataset auxiliary data.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:19:00 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class TecplotDataSetAux : public TecplotBase
{
public:
    TecplotDataSetAux();
    virtual ~TecplotDataSetAux();

    virtual bool Read(FILE *f);
    virtual bool Write(FILE *f);

    std::string name;
    int         valueFormat;
    std::string value; 
};

// ****************************************************************************
// Class: TecplotZoneData
//
// Purpose:
//   Abstract base class for zone data, which is data that varies depending
//   on the zone type.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:20:26 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class TecplotZone;

class TecplotZoneData : public TecplotBase
{
public:
    TecplotZoneData();
    virtual ~TecplotZoneData();

    virtual bool Read(FILE *, TecplotZone *) = 0;

    virtual int GetNumNodes() const = 0;
    virtual int GetNumElements() const = 0;
    virtual int GetNumSpatialDimensions() const = 0;
    virtual int GetNumTopologicalDimensions() const = 0;
private:
    virtual bool Read(FILE *);
};

// Zone types.
typedef enum
{
    ORDERED,
    FELINESEG,
    FETRIANGLE,
    FEQUADRILATERAL,
    FETETRAHEDRON,
    FEBRICK,
    FEPOLYGON,
    FEPOLYHEDRON
} ZoneType;

int TecplotNumNodesForZoneType(ZoneType z);
int TecplotNumFacesForZoneType(ZoneType z);
std::string TecplotZoneType2String(ZoneType z);

// ****************************************************************************
// Class: TecplotFEZone
//
// Purpose:
//   Contains FE zone data.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:21:07 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class TecplotFEZone : public TecplotZoneData
{
public:
    TecplotFEZone(ZoneType);
    TecplotFEZone(const TecplotFEZone &);
    virtual ~TecplotFEZone();

    virtual bool Read(FILE *, TecplotZone *);
    virtual bool Write(FILE *f);

    void operator = (const TecplotFEZone &);

    virtual int GetNumNodes() const;
    virtual int GetNumElements() const;
    virtual int GetNumSpatialDimensions() const;
    virtual int GetNumTopologicalDimensions() const;

    ZoneType zoneType;

    int numPts;
    // if(zoneType == FEPOLYGON or PEPOLYHEDRON)
         int numFaces;
         int numFaceNodes;
         int numBoundaryFaces;
         int numBoundaryConnections;
    int numElements;
    int iCellDim, jCellDim;
    int kCellDim; // reserved
};

// ****************************************************************************
// Class: TecplotOrderedZone
//
// Purpose:
//   Contains ordered zone data.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:21:29 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class TecplotOrderedZone : public TecplotZoneData
{
public:
    TecplotOrderedZone();
    TecplotOrderedZone(const TecplotOrderedZone &);
    virtual ~TecplotOrderedZone();
    void operator = (const TecplotOrderedZone &);

    virtual bool Read(FILE *f, TecplotZone *);
    virtual bool Write(FILE *f);

    virtual int GetNumNodes() const;
    virtual int GetNumElements() const;
    virtual int GetNumSpatialDimensions() const;
    virtual int GetNumTopologicalDimensions() const;

    int iMax, jMax, kMax;
};

// ****************************************************************************
// Class: TecplotZone
//
// Purpose:
//   Describes a zone.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:22:13 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

#define TECPLOT_MAX_ZONE_TYPES 10

class TecplotZone : public TecplotBase
{
public:
    enum { NodeCentered = 0, CellCentered = 1};
    enum { Block, Point };

    TecplotZone(int nV);
    TecplotZone(const TecplotZone &);
    virtual ~TecplotZone();
    void operator = (const TecplotZone &);

    virtual bool Read(FILE *f);
    virtual bool Write(FILE *f);

    int GetNumNodes() const;
    int GetNumElements() const;
    bool IsBoundaryZone();
    int GetNumSpatialDimensions() const;
    int GetNumTopologicalDimensions() const;

    std::string          zoneName;
    int                  parentZone;
    int                  strandID;
    double               solutionTime;
    int                  zoneColor;
    ZoneType             zoneType;
    int                  dataPacking;
    int                  varLocation;
    // if(varLocation == 1)
        std::vector<int> centering;
    int                  rawLocalFaceNeighbors;
    int                  numUserDefinedNeighborConnections;
    // if(numUserDefinedNeighborConnections != 0)
        int              userDefinedFaceNeighborMode;
        // if(zoneType == FE Zone)
            int          FEFaceNeighborsCompletelySpecfied;
    TecplotZoneData     *zoneData;
    TecplotDataSetAux   *auxData[TECPLOT_MAX_ZONE_TYPES];
};

typedef enum
{
    TecplotGrid,
    TecplotFrame,
    TecplotFrameOffset,
    TecplotOldWindow,
    TecplotGrid3D
} TecplotPositionCoordSys;

typedef enum
{
    TecplotLine,
    TecplotRectangle,
    TecplotSquare,
    TecplotCircle,
    TecplotEllipse
} TecplotGeomType;

typedef enum
{ 
    TecplotSolid,
    TecplotDashed,
    TecplotDashDot,
    TecplotDashDotDot,
    TecplotDotted,
    TecplotLongDash
} TecplotLinePattern;

typedef enum
{
    TecplotPlain,
    TecplotFilled,
    TecplotHollow
} TecplotArrowheadStyle;

typedef enum
{
    TecplotNone,
    TecplotBeg,
    TecplotEnd,
    TecplotBoth
} TecplotArrowheadAttachment;

typedef enum
{
    TecplotClipToAxes,
    TecplotClipToViewport,
    TecplotClipToFrame
} TecplotClipping;

// ****************************************************************************
// Class: TecplotGeometry
//
// Purpose:
//   Contains the information for a geometry record.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 14:35:34 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class TecplotGeometryInfo : public TecplotBase
{
public:
    TecplotGeometryInfo();
    virtual ~TecplotGeometryInfo();

    virtual bool Read(FILE *f);
    virtual bool Write(FILE *f);

    TecplotPositionCoordSys    positionCoordSys;
    int                        scope;
    int                        drawOrder;
    double                     startLocation[3];
    int                        zone;
    int                        color;
    int                        fillColor;
    int                        isFilled;
    TecplotGeomType            geomType;
    TecplotLinePattern         linePattern;
    double                     patternLength;
    double                     lineThickness;
    int                        numEllipsePts;
    TecplotArrowheadStyle      arrowHeadStyle;
    TecplotArrowheadAttachment arrowHeadAttachment;
    double                     arrowHeadSize;
    double                     arrowHeadAngle;
    std::string                macroFunctionCommand;
    int                        polyLineFieldType;
    TecplotClipping            clipping;
};

// ****************************************************************************
// Class: TecplotGeometry
//
// Purpose:
//   Base class for various geometry entities.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 15:26:44 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class TecplotGeometry : public TecplotBase
{
public:
    TecplotGeometry(const TecplotGeometryInfo &);
    virtual ~TecplotGeometry();

    virtual bool Write(FILE *f);

    TecplotGeometryInfo info;
};

class TecplotPolyline : public TecplotBase
{
public:
    TecplotPolyline();
    TecplotPolyline(const TecplotPolyline &);
    virtual ~TecplotPolyline();
    void operator = (const TecplotPolyline &);

    bool Read(FILE *f, bool readFloat, bool readZ);
    bool Write(FILE *f, bool writeFloat, bool writeZ);

    int nPts;
    double *xPts;
    double *yPts;
    double *zPts;
private:
    virtual bool Read(FILE *f);
    virtual bool Write(FILE *f);
};

class TecplotLineData : public TecplotGeometry
{
public:
    TecplotLineData(const TecplotGeometryInfo &);
    virtual ~TecplotLineData();

    virtual bool Read(FILE *f);
    virtual bool Write(FILE *f);

    std::vector<TecplotPolyline> polyLines;
};

class TecplotRectangleData : public TecplotGeometry
{
public:
    TecplotRectangleData(const TecplotGeometryInfo &);
    virtual ~TecplotRectangleData();

    virtual bool Read(FILE *f);
    virtual bool Write(FILE *f);

    double xOffset;
    double yOffset;
};

class TecplotCircleData : public TecplotGeometry
{
public:
    TecplotCircleData(const TecplotGeometryInfo &);
    virtual ~TecplotCircleData();

    virtual bool Read(FILE *f);
    virtual bool Write(FILE *f);

    double radius;
};

class TecplotSquareData : public TecplotGeometry
{
public:
    TecplotSquareData(const TecplotGeometryInfo &);
    virtual ~TecplotSquareData();

    virtual bool Read(FILE *f);
    virtual bool Write(FILE *f);

    double width;
};

class TecplotEllipseData : public TecplotGeometry
{
public:
    TecplotEllipseData(const TecplotGeometryInfo &);
    virtual ~TecplotEllipseData();

    virtual bool Read(FILE *f);
    virtual bool Write(FILE *f);

    double xRadius;
    double yRadius;
};

typedef enum
{
    TecplotLeft,
    TecplotCenter,
    TecplotRight,
    TecplotMidLeft,
    TecplotMidCenter,
    TecplotMidRight,
    TecplotHeadLeft,
    TecplotHeadCenter,
    TecplotHeadRight
} TecplotTextAnchor;

// ****************************************************************************
// Class: TecplotText
//
// Purpose:
//   Record type that contains text.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 16:41:46 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class TecplotText : public TecplotBase
{
public:
    TecplotText();
    virtual ~TecplotText();

    virtual bool Read(FILE *f);
    virtual bool Write(FILE *f);

    TecplotPositionCoordSys    positionCoordSys;
    int                        scope;
    double                     startLocation[3];
    int                        fontType;
    int                        characterHeightUnits;
    double                     characterHeight;
    int                        boxType;
    double                     boxMargin;
    double                     boxMarginLineWidth;
    int                        boxOutlineColor;
    int                        boxFillColor;
    double                     angle;
    double                     lineSpacing;
    TecplotTextAnchor          anchor;
    int                        zone;
    int                        color;
    std::string                macroFunctionCommand;
    TecplotClipping            clipping;
    std::string                text;
};

// ****************************************************************************
// Class: TecplotCustomLabel
//
// Purpose:
//   Record type that contains custom labels.
//
// Notes:      section vii of header
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 15:48:51 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class TecplotCustomLabel : public TecplotBase
{
public:
    TecplotCustomLabel();
    virtual ~TecplotCustomLabel();

    virtual bool Read(FILE *);
    virtual bool Write(FILE *);

    std::vector<std::string> labels;
};

// ****************************************************************************
// Class: TecplotVariableAux
//
// Purpose:
//   Record type that contains variable auxiliary data.
//
// Notes:      section x of header.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 16:10:31 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class TecplotVariableAux : public TecplotBase
{
public:
    TecplotVariableAux();
    virtual ~TecplotVariableAux();

    virtual bool Read(FILE *f);
    virtual bool Write(FILE *f);

    std::string variableMarker;
    std::string name;
    int         format;
    std::string value;
};

//
// Classes that are used to read the data section of the file.
//

typedef enum
{
    TecplotFloat = 1,
    TecplotDouble = 2,
    TecplotLongInt = 3,
    TecplotShortInt = 4,
    TecplotByte = 5,
    TecplotBit = 6
} TecplotDataType;

std::string TecplotDataType2String(TecplotDataType dt);
int TecplotNumBytesForType(TecplotDataType dt);

// ****************************************************************************
// Class: TecplotVariable
//
// Purpose: 
//   Contains information about a variable, including its size and location
//   within the file so it can be read later on demand.
//
// Notes:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:22:42 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class TecplotVariable
{
public:
    TecplotVariable();
    virtual ~TecplotVariable();

    long            dataOffset;
    long            dataSize;

    TecplotDataType dataType;
    int             isPassive;
    int             zoneShareNumber;
    double          minValue;
    double          maxValue;
};

class TecplotDataRecord;

// ****************************************************************************
// Class: TecplotConnectivity
//
// Purpose:
//   Abstract base class for records that contain connectivity data.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:23:50 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class TecplotConnectivity : public TecplotBase
{
public:
    TecplotConnectivity();
    virtual ~TecplotConnectivity();

    virtual bool Read(FILE *f, const TecplotZone &, const TecplotDataRecord *) = 0;

    virtual TecplotConnectivity *NewInstance() const = 0;
private:
    virtual bool Read(FILE *f) { return false; }
};

// ****************************************************************************
// Class: TecplotOrderedConnectivity
//
// Purpose:
//   Contains information for ordered zone connectivity, including its size
//   and location within the file so it can be read later on demand.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:24:14 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class TecplotOrderedConnectivity : public TecplotConnectivity
{
public:
    TecplotOrderedConnectivity();
    virtual ~TecplotOrderedConnectivity();

    virtual TecplotConnectivity *NewInstance() const;

    virtual bool Read(FILE *f, const TecplotZone &, const TecplotDataRecord *);
    virtual bool Write(FILE *f);    

    long connectivityOffset;
    long connectivitySize;
};

// ****************************************************************************
// Class: TecplotFEConnectivity
//
// Purpose:
//   Contains information for FE zone connectivity, including its size
//   and location within the file so it can be read later on demand.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:24:14 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class TecplotFEConnectivity : public TecplotConnectivity
{
public:
    TecplotFEConnectivity();
    virtual ~TecplotFEConnectivity();

    virtual TecplotConnectivity *NewInstance() const;

    virtual bool Read(FILE *f, const TecplotZone &, const TecplotDataRecord *);
    virtual bool Write(FILE *f);   

    long zoneConnectivitySize;
    long raw1to1FaceNeighborSize;
    long faceNeighborConnectionSize;

    long zoneConnectivityOffset;
    long raw1to1FaceNeighborOffset;
    long faceNeighborConnectionOffset;
};

// ****************************************************************************
// Class: TecplotPolyConnectivity
//
// Purpose:
//   Contains information for FEPOLYGON,FEPOLYHEDRAL zone connectivity, 
//   including its size and location within the file so it can be read 
//   later on demand.
//
// Notes:      NOT DONE!
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:24:14 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class TecplotPolyConnectivity : public TecplotConnectivity
{
public:
    TecplotPolyConnectivity(){};
    virtual ~TecplotPolyConnectivity(){};

    virtual TecplotConnectivity *NewInstance() const;

    virtual bool Read(FILE *f, const TecplotZone &, const TecplotDataRecord *){ return false;}
    virtual bool Write(FILE *f){ return false; }
};

// ****************************************************************************
// Class: TecplotDataRecord
//
// Purpose:
//   Data record for a zone. The data record includes data for all variables,
//   zone coordinates and connectivity, etc.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:25:57 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class TecplotDataRecord : public TecplotBase
{
public:
    TecplotDataRecord(int nv);
    TecplotDataRecord(const TecplotDataRecord &);
    virtual ~TecplotDataRecord();
    void operator = (const TecplotDataRecord &);

    bool Read(FILE *f, const TecplotZone &);
    virtual bool Write(FILE *f);

    // In-memory-only data
    long                         dataOffset;
    long                         connectivityOffset;

    // Data from file
    std::vector<TecplotVariable> variables;
    int                          hasPassiveVariables;
    int                          hasVariableSharing;
    int                          zoneNumberForConnectivity;
    TecplotConnectivity         *connectivity;
private:
    void CalculateOffsets(const TecplotZone &);
    virtual bool Read(FILE *f){return false;}
};

// ****************************************************************************
// Class: TecplotFile
//
// Purpose:
//   This class reads/writes binary Tecplot files and serves as an in-memory
//   representation of the data from the file.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:13:44 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class TecplotFile : public TecplotBase
{
public:
    TecplotFile(const std::string &fn);
    virtual ~TecplotFile();

    // Tecplot header section
    char                            magic[9];              // section i
    // TecplotBase::reverseEndian                          // section ii
    TecplotTitleAndVariables        titleAndVars;          // section iii
    std::vector<TecplotZone>        zones;                 // section iv
    std::vector<TecplotGeometry *>  geometries;            // section v
    std::vector<TecplotText>        texts;                 // section vi
    std::vector<TecplotCustomLabel> customLabels;          // section vii
    std::vector<std::string>        userRecs;              // section viii
    std::vector<TecplotDataSetAux>  datasetAuxiliaryData;  // section ix
    std::vector<TecplotVariableAux> variableAuxiliaryData; // section x

    // Tecplot data section
    std::vector<TecplotDataRecord>  zoneData;              // data section

    bool Read();
    bool Write();

    bool DisableReadingDataInformation();
    std::string CoordinateVariable(int axis) const;
    int ZoneNameToIndex(const std::string &) const;
    int VarNameToIndex(const std::string &) const;
    int GetNumSpatialDimensions(int zoneId) const;
    int GetNumTopologicalDimensions(int zoneId) const;

    // Read data into allocated arrays
    bool ReadVariable(int zoneId, const std::string &varName, void *);
    bool ReadVariableAsFloat(int zoneId, const std::string &varName, float *);
    bool ReadFEConnectivity(int zoneId, int **conn);

protected:
    bool ReadData(long dataOffset, long dataSize, TecplotDataType dataType,
                  int dataPacking, int nnodes, void *ptr);
    void EnsureUniqueZoneNames();

    FILE        *tec;
    std::string  fileName;
    bool         readDataInformation;

    virtual bool Read(FILE *f);
    virtual bool Write(FILE *f);
};

ostream &operator << (ostream &os, const TecplotFile &obj);

#endif
