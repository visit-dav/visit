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
#include <TecplotFile.h>
#include <map>

#include <DebugStream.h> // for now...

#define TECPLOT_71  71
#define TECPLOT_75  75
#define TECPLOT_100 100
#define TECPLOT_101 101
#define TECPLOT_102 102
#define TECPLOT_103 103
#define TECPLOT_104 104
#define TECPLOT_105 105
#define TECPLOT_106 106
#define TECPLOT_107 107
#define TECPLOT_108 108
#define TECPLOT_109 109
#define TECPLOT_110 110
#define TECPLOT_111 111

#define TECPLOT_SWAP(A, B, tmp) \
        tmp = A;\
        A = B;\
        B = tmp;

#define TECPLOT_VERSION_7X(v) (v >= TECPLOT_71 && v <= 79)

// ****************************************************************************
// Method: tecplot_reverse_endian32
//
// Purpose: 
//   Reverses 32 bit endianness.
//
// Arguments:
//   ptr      : The source data to reverse.
//   dataSize : The total number of bytes to reverse.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:31:56 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

static void
tecplot_reverse_endian32(void *ptr, long dataSize)
{
    char *cptr = (char*)ptr;
    char *end = cptr + dataSize;
    char tmp;

    while(cptr != end)
    {
#if 1
        TECPLOT_SWAP(cptr[0], cptr[3], tmp);
        TECPLOT_SWAP(cptr[1], cptr[2], tmp);
#else
        // 1 ASM instruction for this?
#endif
        cptr += 4;
    }
}

// ****************************************************************************
// Method: tecplot_reverse_endian64
//
// Purpose: 
//   Reverses 64 bit endianness.
//
// Arguments:
//   ptr      : The source data to reverse.
//   dataSize : The total number of bytes to reverse.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:31:56 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

static void
tecplot_reverse_endian64(void *ptr, long dataSize)
{
    char *cptr = (char*)ptr;
    char *end = cptr + dataSize;
    char tmp;

    while(cptr != end)
    {
#if 1
        TECPLOT_SWAP(cptr[0], cptr[7], tmp);
        TECPLOT_SWAP(cptr[1], cptr[6], tmp);
        TECPLOT_SWAP(cptr[2], cptr[5], tmp);
        TECPLOT_SWAP(cptr[3], cptr[4], tmp);
#else
        // 1 ASM instruction for this?
#endif
        cptr += 8;
    }
}

// ****************************************************************************
// Method: tecplot_reverse_endian16
//
// Purpose: 
//   Reverses 16 bit endianness.
//
// Arguments:
//   ptr      : The source data to reverse.
//   dataSize : The total number of bytes to reverse.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:31:56 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

static void
tecplot_reverse_endian16(void *ptr, long dataSize)
{
    char *cptr = (char*)ptr;
    char *end = cptr + dataSize;
    char tmp;

    while(cptr != end)
    {
#if 1
        TECPLOT_SWAP(cptr[0], cptr[1], tmp);
#else
        // 1 ASM instruction for this?
#endif
        cptr += 2;
    }
}

int
TecplotNumNodesForZoneType(ZoneType z)
{
    int nnodes = 0; 
    switch(z)
    {
    case ORDERED: nnodes = 0; break;
    case FELINESEG: nnodes = 2; break;
    case FETRIANGLE: nnodes = 3; break;
    case FEQUADRILATERAL: nnodes = 4; break;
    case FETETRAHEDRON: nnodes = 4; break;
    case FEBRICK: nnodes = 8; break;
    case FEPOLYGON: nnodes = 0; break;
    case FEPOLYHEDRON: nnodes = 0; break;
    }
    return nnodes;
}

int
TecplotNumFacesForZoneType(ZoneType z)
{
    int nfaces = 0; 
    switch(z)
    {
    case ORDERED: nfaces = 1; break;
    case FELINESEG: nfaces = 1; break;
    case FETRIANGLE: nfaces = 1; break;
    case FEQUADRILATERAL: nfaces = 1; break;
    case FETETRAHEDRON: nfaces = 4; break;
    case FEBRICK: nfaces = 6; break;
    case FEPOLYGON: nfaces = 1; break;
    case FEPOLYHEDRON: nfaces = 0; break;
    }
    return nfaces;
}

std::string
TecplotZoneType2String(ZoneType z)
{
    std::string s; 
    switch(z)
    {
    case ORDERED: s = "ORDERED"; break;
    case FELINESEG: s = "FELINESEG"; break;
    case FETRIANGLE: s = "FETRIANGLE"; break;
    case FEQUADRILATERAL: s = "FEQUADRILATERAL"; break;
    case FETETRAHEDRON: s = "FETETRAHEDRON"; break;
    case FEBRICK: s = "FEBRICK"; break;
    case FEPOLYGON: s = "FEPOLYGON"; break;
    case FEPOLYHEDRON: s = "FEPOLYHEDRON"; break;
    }
    return s;
}

std::string
TecplotDataType2String(TecplotDataType dt)
{
    std::string s;
    switch(dt)
    {
    case TecplotFloat: s = "Float"; break;
    case TecplotDouble: s = "Double"; break;
    case TecplotLongInt: s = "LongInt"; break;
    case TecplotShortInt: s = "ShortInt"; break;
    case TecplotByte: s = "Byte"; break;
    case TecplotBit: s = "Bit"; break;
    }
    return s;
}

int
TecplotNumBytesForType(TecplotDataType dt)
{
    int s;
    switch(dt)
    {
    case TecplotFloat: s = 4; break;
    case TecplotDouble: s = 8; break;
    case TecplotLongInt: s = 4; break;
    case TecplotShortInt: s = 2; break;
    case TecplotByte: s = 1; break;
    case TecplotBit: s = 1; break;
    }
    return s;
}

///////////////////////////////////////////////////////////////////////////////

bool TecplotBase::reverseEndian = false;
int TecplotBase::version = TECPLOT_108;
std::string (*TecplotBase::filterNameCB)(const std::string &) = 0;

std::string
TecplotBase::FilterName(const std::string &input)
{
    std::string output(input);
    if(filterNameCB != 0)
        output = filterNameCB(input);
    return output;
}

TecplotBase::TecplotBase()
{
}

TecplotBase::~TecplotBase()
{
}

int
TecplotBase::ReadInt(FILE *f)
{
    int val;
    fread(&val, 4, 1, f);
    if(reverseEndian)
    {
        char *cptr = (char *)&val;
        char tmp;
        TECPLOT_SWAP(cptr[0], cptr[3], tmp);
        TECPLOT_SWAP(cptr[1], cptr[2], tmp);
    }

    return val;
}

void
TecplotBase::WriteInt(FILE *f, int i)
{
    fwrite(&i, 1, 4, f);
}

float
TecplotBase::ReadFloat(FILE *f)
{
    float val;
    fread(&val, 4, 1, f);
    if(reverseEndian)
    {
        char *cptr = (char *)&val;
        char tmp;
        TECPLOT_SWAP(cptr[0], cptr[3], tmp);
        TECPLOT_SWAP(cptr[1], cptr[2], tmp);
    }

    return val;
}

void
TecplotBase::WriteFloat(FILE *f, float fv)
{
    fwrite(&fv, 1, 4, f);
}

double
TecplotBase::ReadDouble(FILE *f)
{
    double val;
    fread(&val, 8, 1, f);
    if(reverseEndian)
    {
        char *cptr = (char *)&val;
        char tmp;
        TECPLOT_SWAP(cptr[0], cptr[7], tmp);
        TECPLOT_SWAP(cptr[1], cptr[6], tmp);
        TECPLOT_SWAP(cptr[2], cptr[5], tmp);
        TECPLOT_SWAP(cptr[3], cptr[4], tmp);
    }
    return val;
}

void
TecplotBase::WriteDouble(FILE *f, double dv)
{
    fwrite(&dv, 1, 8, f);
}

// ****************************************************************************
// Method: TecplotBase::ReadString
//
// Purpose: 
//   Reads a string from the Tecplot file.
//
// Arguments:
//   f : The file pointer to use.
//
// Returns:    A string.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:34:42 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

std::string
TecplotBase::ReadString(FILE *f)
{
    bool reading = true;
    std::string retval;
    char tmp[100];
    int  nc = 0;
    do
    {
        int c = ReadInt(f);
        if(nc == 100-1)
        {
            tmp[100-1] = '\0';
            retval += tmp;
            nc = 0;
        }
        else
            tmp[nc++] = (char)(c & 0xff);

        reading = c != 0;
    } while(reading);

    if(nc > 0)
        retval += tmp;

    return retval;
}

// ****************************************************************************
// Method: TecplotBase::WriteString
//
// Purpose: 
//   Writes a string to the Tecplot file.
//
// Arguments:
//   f : The file pointer to use.
//   s : The string to write.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:34:42 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
TecplotBase::WriteString(FILE *f, const std::string &s)
{
    int val;
    for(size_t i = 0; i < s.size(); ++i)
    {
        val = (int)s[i];
        fwrite(&val, 1, 4, f);
    }
    val = 0;
    fwrite(&val, 1, 4, f);
}

///////////////////////////////////////////////////////////////////////////////

TecplotTitleAndVariables::TecplotTitleAndVariables() : TecplotBase(), fileType(0), title(),
    varNames(), varUnits()
{
}

TecplotTitleAndVariables::~TecplotTitleAndVariables()
{
}

// ****************************************************************************
// Method: TecplotTitleAndVariables::Read
//
// Purpose: 
//   Reads the title and variable names (section iii) from the Tecplot file
//   and stores the values internally.
//
// Arguments:
//   f : The file pointer to use.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:36:22 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
TecplotTitleAndVariables::Read(FILE *f)
{
    if(version >= TECPLOT_111)
        fileType = ReadInt(f);
    title = ReadString(f);
    int numVar = ReadInt(f);
    for(int i = 0; i < numVar; ++i)
    {
        std::string s = ReadString(f);

        int p1 = s.find("(");
        int p2 = s.find(")");
        if(p1 != -1 && p2 != -1)
        {
            varNames.push_back(FilterName(s.substr(0, p1)));
            varUnits.push_back(s.substr(p1+1, p2 - p1 - 1));
        }
        else
        {
            varNames.push_back(FilterName(s));
            varUnits.push_back(std::string());
        }
    }
    return true;
}

bool
TecplotTitleAndVariables::Write(FILE *f)
{
    WriteInt(f, fileType);
    WriteString(f, title);
    WriteInt(f, varNames.size());
    for(size_t i = 0; i < varNames.size(); ++i)
    {
        if(varUnits[i].size() > 0)
        {
            std::string s(varNames[i]);
            s += "(";
            s += varUnits[i];
            s += ")";
            WriteString(f, s);
        }
        else
            WriteString(f, varNames[i]);
    }
    return true;
}

ostream &
operator << (ostream &os, const TecplotTitleAndVariables &obj)
{
    os << "TitlesAndVariables" << endl;
    os << "{" << endl;
    os << "fileType = " << obj.fileType << endl;
    os << "title = \"" << obj.title << "\"" << endl;
    os << "varNames = ";
    for(size_t i = 0; i < obj.varNames.size(); ++i)
    {
        os << obj.varNames[i];
        if(obj.varUnits[i].size() > 0)
            os << "(" << obj.varUnits[i] << ")";
        os << ", ";
    }
    os << endl;
    os << "}" << endl;
    return os;
}

///////////////////////////////////////////////////////////////////////////////

TecplotDataSetAux::TecplotDataSetAux() : TecplotBase(), name(), valueFormat(0), value()
{
}

TecplotDataSetAux::~TecplotDataSetAux()
{
}

bool
TecplotDataSetAux::Read(FILE *f)
{
    name = ReadString(f);
    valueFormat = ReadInt(f);
    value = ReadString(f);
    return true;
}

bool
TecplotDataSetAux::Write(FILE *f)
{
    WriteString(f, name);
    WriteInt(f, valueFormat);
    WriteString(f, value);
    return true;
}

ostream &
operator << (ostream &os, const TecplotDataSetAux &obj)
{
    os << "name=\"" << obj.name << "\""
       << ", valueFormat=" << obj.valueFormat
       << ", value=\"" << obj.value << "\"";
    return os;
}

///////////////////////////////////////////////////////////////////////////////
TecplotZoneData::TecplotZoneData() : TecplotBase()
{
}

TecplotZoneData::~TecplotZoneData()
{
}

bool
TecplotZoneData::Read(FILE *)
{
    return false;
}

///////////////////////////////////////////////////////////////////////////////

TecplotFEZone::TecplotFEZone(ZoneType zt) : TecplotZoneData(), 
    zoneType(zt),
    numPts(0),
    numFaces(0),
    numFaceNodes(0),
    numBoundaryFaces(0),
    numBoundaryConnections(0),
    numElements(0),
    iCellDim(0),
    jCellDim(0),
    kCellDim(0)
{
}

TecplotFEZone::TecplotFEZone(const TecplotFEZone &obj) : TecplotZoneData(), 
    zoneType(obj.zoneType),
    numPts(obj.numPts),
    numFaces(obj.numFaces),
    numFaceNodes(obj.numFaceNodes),
    numBoundaryFaces(obj.numBoundaryFaces),
    numBoundaryConnections(obj.numBoundaryConnections),
    numElements(obj.numElements),
    iCellDim(obj.iCellDim),
    jCellDim(obj.jCellDim),
    kCellDim(obj.kCellDim)
{
}

TecplotFEZone::~TecplotFEZone()
{
}

void
TecplotFEZone::operator = (const TecplotFEZone &obj)
{
    zoneType = obj.zoneType;
    numPts = obj.numPts;
    numFaces = obj.numFaces;
    numFaceNodes = obj.numFaceNodes;
    numBoundaryFaces = obj.numBoundaryFaces;
    numBoundaryConnections = obj.numBoundaryConnections;
    numElements = obj.numElements;
    iCellDim = obj.iCellDim;
    jCellDim = obj.jCellDim;
    kCellDim = obj.kCellDim;
}

bool
TecplotFEZone::Read(FILE *f, TecplotZone *zone)
{
    if(TECPLOT_VERSION_7X(version))
    {
        numPts = ReadInt(f);
        numElements = ReadInt(f);
        int zt = ReadInt(f);
        if(zt == 3)
            zone->zoneType = FEBRICK;
    }
    else
    {
        numPts = ReadInt(f);
        if(zoneType == FEPOLYGON || zoneType == FEPOLYHEDRON)
        {
            numFaces = ReadInt(f);
            numFaceNodes = ReadInt(f);
            numBoundaryFaces = ReadInt(f);
            numBoundaryConnections = ReadInt(f);
        }
        numElements = ReadInt(f);
        iCellDim = ReadInt(f);
        jCellDim = ReadInt(f);
        kCellDim = ReadInt(f);
    }
    return true;
}

bool
TecplotFEZone::Write(FILE *f)
{
    WriteInt(f, numPts);
    if(zoneType == FEPOLYGON || zoneType == FEPOLYHEDRON)
    {
        WriteInt(f, numFaces);
        WriteInt(f, numFaceNodes);
        WriteInt(f, numBoundaryFaces);
        WriteInt(f, numBoundaryConnections);
    }
    WriteInt(f, numElements);
    WriteInt(f, iCellDim);
    WriteInt(f, jCellDim);
    WriteInt(f, kCellDim);
    return true;
}

int
TecplotFEZone::GetNumNodes() const
{
    return numPts;
}

int
TecplotFEZone::GetNumElements() const
{
    return numElements;
}

int
TecplotFEZone::GetNumSpatialDimensions() const
{
    return 3;
}

int
TecplotFEZone::GetNumTopologicalDimensions() const
{
    int tdim = 0;
    switch(zoneType)
    {
    case FELINESEG:
        tdim = 1;
        break;
    case FETRIANGLE:
        tdim = 2;
        break;
    case FEQUADRILATERAL:
        tdim = 2;
        break;
    case FETETRAHEDRON:
        tdim = 3;
        break;
    case FEBRICK:
        tdim = 3;
        break;
    case FEPOLYGON:
        tdim = 2;
        break;
    case FEPOLYHEDRON:
        tdim = 3;
        break;
    }

    return tdim;
}

ostream &
operator << (ostream &os, const TecplotFEZone &obj)
{
    os << "numPts=" << obj.numPts << endl;
    os << "numFaces=" << obj.numFaces << endl;
    os << "numFaceNodes=" << obj.numFaceNodes << endl;
    os << "numBoundaryFaces=" << obj.numBoundaryFaces << endl;
    os << "numBoundaryConnections=" << obj.numBoundaryConnections << endl;
    os << "numElements=" << obj.numElements << endl;
    os << "iCellDim=" << obj.iCellDim << endl;
    os << "jCellDim=" << obj.jCellDim << endl;
    os << "kCellDim=" << obj.kCellDim << endl;
    return os;
}

///////////////////////////////////////////////////////////////////////////////

TecplotOrderedZone::TecplotOrderedZone() : TecplotZoneData(), iMax(0), jMax(0), kMax(0)
{
}

TecplotOrderedZone::TecplotOrderedZone(const TecplotOrderedZone &obj) : TecplotZoneData(),
    iMax(obj.iMax), jMax(obj.jMax), kMax(obj.kMax)
{
}

TecplotOrderedZone::~TecplotOrderedZone()
{
}

void
TecplotOrderedZone::operator = (const TecplotOrderedZone &obj)
{
    iMax = obj.iMax;
    jMax = obj.jMax;
    kMax = obj.kMax;
}

bool
TecplotOrderedZone::Read(FILE *f, TecplotZone *)
{
    iMax = ReadInt(f);
    jMax = ReadInt(f);
    kMax = ReadInt(f);
    return true;
}

bool
TecplotOrderedZone::Write(FILE *f)
{
    WriteInt(f, iMax);
    WriteInt(f, jMax);
    WriteInt(f, kMax);
    return true;
}

int
TecplotOrderedZone::GetNumNodes() const
{
    return iMax * jMax * kMax;
}

int
TecplotOrderedZone::GetNumElements() const
{
    return (iMax - 1) * (jMax - 1) * (kMax - 1);
}

int
TecplotOrderedZone::GetNumSpatialDimensions() const
{
    int sdim;
    if(jMax == 1 && kMax == 1)
        sdim = 1;
    else if(kMax == 1)
        sdim = 2;
    else
        sdim = 3;
    return sdim;
}

int
TecplotOrderedZone::GetNumTopologicalDimensions() const
{
    int tdim = 0;
    if(iMax > 1)
       ++tdim;
    if(jMax > 1)
       ++tdim;
    if(kMax > 1)
       ++tdim;
    return tdim;
}

ostream &
operator << (ostream &os, const TecplotOrderedZone &obj)
{
    os << "iMax = " << obj.iMax << endl;
    os << "jMax = " << obj.jMax << endl;
    os << "kMax = " << obj.kMax << endl;
    return os;
}

///////////////////////////////////////////////////////////////////////////////

TecplotZone::TecplotZone(int nV) : TecplotBase(), zoneName(), parentZone(-1),
    strandID(-1), solutionTime(0.), zoneColor(-1), zoneType(ORDERED),
    dataPacking(TecplotZone::Block), varLocation(0), centering(), 
    rawLocalFaceNeighbors(0), numUserDefinedNeighborConnections(0),
    userDefinedFaceNeighborMode(0),FEFaceNeighborsCompletelySpecfied(0),
    zoneData(0)
{
    for(int i = 0; i < nV; ++i)
       centering.push_back(TecplotZone::NodeCentered);

    for(int i = 0; i < TECPLOT_MAX_ZONE_TYPES; ++i)
        auxData[i] = 0;
}

TecplotZone::TecplotZone(const TecplotZone &obj) : TecplotBase()
{
    zoneName = obj.zoneName;
    parentZone = obj.parentZone;
    strandID = obj.strandID;
    solutionTime = obj.solutionTime;
    zoneColor = obj.zoneColor;
    zoneType = obj.zoneType;
    dataPacking = obj.dataPacking;
    varLocation = obj.varLocation;
    centering = obj.centering;
    rawLocalFaceNeighbors = obj.rawLocalFaceNeighbors;
    numUserDefinedNeighborConnections = obj.numUserDefinedNeighborConnections;
    userDefinedFaceNeighborMode = obj.userDefinedFaceNeighborMode;
    FEFaceNeighborsCompletelySpecfied = obj.FEFaceNeighborsCompletelySpecfied;

    if(zoneType == ORDERED)
    {
        TecplotOrderedZone *d = (TecplotOrderedZone *)obj.zoneData;
        zoneData = new TecplotOrderedZone(*d);
    }
    else
    {
        TecplotFEZone *d = (TecplotFEZone *)obj.zoneData;
        zoneData = new TecplotFEZone(*d);
    }

    for(int i = 0; i < TECPLOT_MAX_ZONE_TYPES; ++i)
    {
        if(obj.auxData[i] != 0)
            auxData[i] = new TecplotDataSetAux(*obj.auxData[i]);
        else
            auxData[i] = 0;
    }
}

TecplotZone::~TecplotZone()
{
    delete zoneData;
    for(int i = 0; i < TECPLOT_MAX_ZONE_TYPES; ++i)
    {
        if(auxData[i] != 0)
            delete auxData[i];
    }
}

void
TecplotZone::operator = (const TecplotZone &obj)
{
    zoneName = obj.zoneName;
    parentZone = obj.parentZone;
    strandID = obj.strandID;
    solutionTime = obj.solutionTime;
    zoneColor = obj.zoneColor;
    zoneType = obj.zoneType;
    dataPacking = obj.dataPacking;
    varLocation = obj.varLocation;
    centering = obj.centering;
    rawLocalFaceNeighbors = obj.rawLocalFaceNeighbors;
    numUserDefinedNeighborConnections = obj.numUserDefinedNeighborConnections;
    userDefinedFaceNeighborMode = obj.userDefinedFaceNeighborMode;
    FEFaceNeighborsCompletelySpecfied = obj.FEFaceNeighborsCompletelySpecfied;

    if(zoneType == ORDERED)
    {
        TecplotOrderedZone *d = (TecplotOrderedZone *)obj.zoneData;
        zoneData = new TecplotOrderedZone(*d);
    }
    else
    {
        TecplotFEZone *d = (TecplotFEZone *)obj.zoneData;
        zoneData = new TecplotFEZone(*d);
    }

    for(int i = 0; i < TECPLOT_MAX_ZONE_TYPES; ++i)
    {
        if(obj.auxData[i] != 0)
            auxData[i] = new TecplotDataSetAux(*obj.auxData[i]);
        else
            auxData[i] = 0;
    }
}

bool
TecplotZone::Read(FILE *f)
{
    const char *mName = "TecplotZone::Read: ";

    zoneName = FilterName(ReadString(f));

    if(version > TECPLOT_106)
        parentZone = ReadInt(f); // speculation

    if(TECPLOT_VERSION_7X(version))
    {
        // Zone types are different in Tecplot 7x
        int zt = ReadInt(f);
        if(zt == 2)
            zoneType = FETETRAHEDRON; // This zt might just mean FE
        else if(zt == 3)
            zoneType = FETRIANGLE;
        else if(zt == 1)
            zoneType = ORDERED;
        else
            zoneType = (ZoneType)zt;
        strandID = ReadInt(f);
    }
    else
    {
        strandID = ReadInt(f);
        solutionTime = ReadDouble(f);
        zoneColor = ReadInt(f);
        zoneType = (ZoneType)ReadInt(f);
        dataPacking = ReadInt(f);
        varLocation = ReadInt(f);
        if(varLocation == 1)
        {
            for(size_t i = 0; i < centering.size(); ++i)
                centering[i] = ReadInt(f);
        }
        rawLocalFaceNeighbors = ReadInt(f);
        if(version > TECPLOT_107)
        {
            numUserDefinedNeighborConnections = ReadInt(f);
            if(numUserDefinedNeighborConnections != 0)
            {
                userDefinedFaceNeighborMode = ReadInt(f);
                if(zoneType != ORDERED)
                    FEFaceNeighborsCompletelySpecfied = ReadInt(f);
            }
        }
    }

    // Read Zonetype specific data
    if(zoneType == ORDERED)
        zoneData = new TecplotOrderedZone;
    else
        zoneData = new TecplotFEZone(zoneType);
    zoneData->Read(f, this);

    // Read any dataset aux data for the zone.
    if(!TECPLOT_VERSION_7X(version))
    {
        bool keepReading = true;
        for(int i = 0; i < TECPLOT_MAX_ZONE_TYPES && keepReading; ++i)
        {
            int hasAuxData = ReadInt(f);
            if(hasAuxData != 0)
            {
                auxData[i] = new TecplotDataSetAux;
                auxData[i]->Read(f);
            }
            else
            {
                keepReading = false;
            }
        }
    }

    return true;
}

bool
TecplotZone::Write(FILE *f)
{
    WriteString(f, zoneName);
    WriteInt(f, parentZone);
    WriteInt(f, strandID);
    WriteDouble(f, solutionTime);
    WriteInt(f, zoneColor);
    WriteInt(f, (int)zoneType );
    WriteInt(f, dataPacking);
    WriteInt(f, varLocation);
    if(varLocation == 1)
    {
        for(size_t i = 0; i < centering.size(); ++i)
            WriteInt(f, centering[i]);
    }
    WriteInt(f, rawLocalFaceNeighbors);
    WriteInt(f, numUserDefinedNeighborConnections);
    if(numUserDefinedNeighborConnections != 0)
    {
        WriteInt(f, userDefinedFaceNeighborMode);
        if(zoneType != ORDERED)
            WriteInt(f, FEFaceNeighborsCompletelySpecfied);
    }
    zoneData->Write(f);
    bool keepWriting = true;
    for(int i = 0; i < TECPLOT_MAX_ZONE_TYPES && keepWriting; ++i)
    {
        if(auxData[i] != 0)
        {
            WriteInt(f, 1);
            auxData[i]->Write(f);
        }
        else
        { 
            WriteInt(f, 0);
            keepWriting = false;
        }
    }
    return true;
}

int
TecplotZone::GetNumNodes() const
{
    return zoneData->GetNumNodes();
}

int
TecplotZone::GetNumElements() const
{
    return zoneData->GetNumElements();
}

bool
TecplotZone::IsBoundaryZone()
{
    bool isBoundary = false;
    for(int i = 0; auxData[i] != 0; ++i)
    {
        if(auxData[i]->name == "Common.IsBoundaryZone" &&
           auxData[i]->value == "Y")
        {
            isBoundary = true;
            break;
        }
    }
    return isBoundary;
}

int
TecplotZone::GetNumSpatialDimensions() const
{
    return zoneData->GetNumSpatialDimensions();
}

int
TecplotZone::GetNumTopologicalDimensions() const
{
    return zoneData->GetNumTopologicalDimensions();
}

ostream &
operator << (ostream &os, const TecplotZone &obj)
{
    os << "zoneName = " << obj.zoneName << endl;
    os << "parentZone = " << obj.parentZone << endl;
    os << "strandID = " << obj.strandID << endl;
    os << "solutionTime = " << obj.solutionTime << endl;
    os << "zoneColor = " << obj.zoneColor << endl;
    os << "zoneType = " << TecplotZoneType2String(obj.zoneType) << endl;
    os << "dataPacking = " << obj.dataPacking << endl;
    os << "varLocation = " << obj.varLocation << endl;
    os << "centering = "; 
    for(size_t i = 0; i < obj.centering.size(); ++i)
        os << obj.centering[i] << ", ";
    os << endl;
    os << "rawLocalFaceNeighbors = " << obj.rawLocalFaceNeighbors << endl;

    os << "numUserDefinedNeighborConnections = " << obj.numUserDefinedNeighborConnections << endl;
    if(obj.numUserDefinedNeighborConnections != 0)
    {
        os << "userDefinedFaceNeighborMode = " << obj.userDefinedFaceNeighborMode << endl;
        if(obj.zoneType != ORDERED)
            os << "FEFaceNeighborsCompletelySpecfied = " << obj.FEFaceNeighborsCompletelySpecfied << endl;
    }
    if(obj.zoneType == ORDERED)
    {
        TecplotOrderedZone *z = (TecplotOrderedZone *)obj.zoneData;
        os << *z << endl;
    }
    else
    {
        TecplotFEZone *z = (TecplotFEZone *)obj.zoneData;
        os << *z << endl;
    }

    for(int i = 0; i < TECPLOT_MAX_ZONE_TYPES; ++i)
    {
        if(obj.auxData[i] != 0)
            os << *obj.auxData[i] << endl;
        else
            break;
    }
    return os;
}

///////////////////////////////////////////////////////////////////////////////

TecplotGeometryInfo::TecplotGeometryInfo() : TecplotBase(),
    positionCoordSys(TecplotGrid),
    scope(0),
    drawOrder(0),
    zone(0),
    color(-1),
    fillColor(-1),
    isFilled(0),
    geomType(TecplotLine),
    linePattern(TecplotSolid),
    patternLength(1.),
    lineThickness(1.),
    numEllipsePts(2),
    arrowHeadStyle(TecplotFilled),
    arrowHeadAttachment(TecplotBeg),
    arrowHeadSize(1.),
    arrowHeadAngle(30.),
    macroFunctionCommand(),
    polyLineFieldType(2),
    clipping(TecplotClipToAxes)
{
    startLocation[0] = 0.;
    startLocation[1] = 0.;
    startLocation[2] = 0.;
}

TecplotGeometryInfo::~TecplotGeometryInfo()
{
}

bool
TecplotGeometryInfo::Read(FILE *f)
{
    positionCoordSys = (TecplotPositionCoordSys)ReadInt(f);
    scope = ReadInt(f);
    drawOrder = ReadInt(f);
    startLocation[0] = ReadDouble(f);
    startLocation[1] = ReadDouble(f);
    startLocation[2] = ReadDouble(f);
    zone = ReadInt(f);
    color = ReadInt(f);
    fillColor = ReadInt(f);
    isFilled = ReadInt(f);
    geomType = (TecplotGeomType)ReadInt(f);
    linePattern = (TecplotLinePattern)ReadInt(f);
    patternLength = ReadDouble(f);
    lineThickness = ReadDouble(f);
    numEllipsePts = ReadInt(f);
    arrowHeadStyle = (TecplotArrowheadStyle)ReadInt(f);
    arrowHeadAttachment = (TecplotArrowheadAttachment)ReadInt(f);
    arrowHeadSize = ReadDouble(f);
    arrowHeadAngle = ReadDouble(f);
    macroFunctionCommand = ReadString(f);
    polyLineFieldType = ReadInt(f);
    clipping = (TecplotClipping)ReadInt(f);
    return true;
}

bool
TecplotGeometryInfo::Write(FILE *f)
{
    WriteInt(f, (int)positionCoordSys);
    WriteInt(f, scope);
    WriteInt(f, drawOrder);
    WriteDouble(f, startLocation[0]);
    WriteDouble(f, startLocation[1]);
    WriteDouble(f, startLocation[2]);
    WriteInt(f, zone);
    WriteInt(f, color);
    WriteInt(f, fillColor);
    WriteInt(f, isFilled);
    WriteInt(f, (int)geomType);
    WriteInt(f, (int)linePattern);
    WriteDouble(f, patternLength);
    WriteDouble(f, lineThickness);
    WriteInt(f, numEllipsePts);
    WriteInt(f, (int)arrowHeadStyle);
    WriteInt(f, (int)arrowHeadAttachment);
    WriteDouble(f, arrowHeadSize);
    WriteDouble(f, arrowHeadAngle);
    WriteString(f, macroFunctionCommand);
    WriteInt(f, polyLineFieldType);
    WriteInt(f, (int)clipping);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

TecplotGeometry::TecplotGeometry(const TecplotGeometryInfo &info_) : TecplotBase(), info(info_)
{
}

TecplotGeometry::~TecplotGeometry()
{
}

bool
TecplotGeometry::Write(FILE *f)
{
    return info.Write(f);
}

///////////////////////////////////////////////////////////////////////////////

TecplotPolyline::TecplotPolyline() : TecplotBase(), nPts(0), 
    xPts(0), yPts(0), zPts(0)
{
}

TecplotPolyline::TecplotPolyline(const TecplotPolyline &obj) : TecplotBase()
{
    nPts = obj.nPts;
    if(nPts > 0)
    {
        xPts = new double[nPts];
        yPts = new double[nPts];
        if(obj.zPts != 0)
            zPts = new double[nPts];
        else
            zPts = 0;
        for(int i = 0; i < nPts; ++i)
        {
            xPts[i] = obj.xPts[i];
            yPts[i] = obj.yPts[i];
            if(zPts != 0)
                zPts[i] = obj.zPts[i];
        }
    }
    else
    {
        xPts = 0;
        yPts = 0;
        zPts = 0;
    }
}

TecplotPolyline::~TecplotPolyline()
{
    delete [] xPts;
    delete [] yPts;
    delete [] zPts;
}

void
TecplotPolyline::operator = (const TecplotPolyline &obj)
{
    nPts = obj.nPts;
    if(nPts > 0)
    {
        xPts = new double[nPts];
        yPts = new double[nPts];
        if(obj.zPts != 0)
            zPts = new double[nPts];
        else
            zPts = 0;
        for(int i = 0; i < nPts; ++i)
        {
            xPts[i] = obj.xPts[i];
            yPts[i] = obj.yPts[i];
            if(zPts != 0)
                zPts[i] = obj.zPts[i];
        }
    }
    else
    {
        xPts = 0;
        yPts = 0;
        zPts = 0;
    }
}

bool
TecplotPolyline::Read(FILE *f)
{
    return false;
}

bool
TecplotPolyline::Write(FILE *f)
{
    return false;
}

bool
TecplotPolyline::Read(FILE *f, bool readFloat, bool readZ)
{
    nPts = ReadInt(f);
    xPts = new double[nPts];
    yPts = new double[nPts];
    if(readZ)
        zPts = new double[nPts];

    if(readFloat)
    {
        for(int i = 0; i < nPts; ++i)
        {
            float tmp = ReadFloat(f);
            xPts[i] = (double)tmp;
        }
        for(int i = 0; i < nPts; ++i)
        {
            float tmp = ReadFloat(f);
            yPts[i] = (double)tmp;
        }
        if(readZ)
        {
            for(int i = 0; i < nPts; ++i)
            {
                float tmp = ReadFloat(f);
                zPts[i] = (double)tmp;
            }
        }
    }
    else
    {
        for(int i = 0; i < nPts; ++i)
            xPts[i] = ReadDouble(f);
        for(int i = 0; i < nPts; ++i)
            yPts[i] = ReadDouble(f);
        if(readZ)
        {
            for(int i = 0; i < nPts; ++i)
                zPts[i] = ReadDouble(f);
        }
    }

    return true;
}

bool
TecplotPolyline::Write(FILE *f, bool writeFloat, bool writeZ)
{
    WriteInt(f, nPts);
    if(writeFloat)
    {
        for(int i = 0; i < nPts; ++i)
        {
            float tmp = (float)xPts[i];
            WriteDouble(f, tmp);
        }
        for(int i = 0; i < nPts; ++i)
        {
            float tmp = (float)yPts[i];
            WriteDouble(f, tmp);
        }
        if(writeZ)
        {
            for(int i = 0; i < nPts; ++i)
            {
                float tmp = (float)zPts[i];
                WriteDouble(f, tmp);
            }
        }
    }
    else
    {
        for(int i = 0; i < nPts; ++i)
            WriteDouble(f, xPts[i]);
        for(int i = 0; i < nPts; ++i)
            WriteDouble(f, yPts[i]);
        if(writeZ)
        {
            for(int i = 0; i < nPts; ++i)
                WriteDouble(f, zPts[i]);
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

TecplotLineData::TecplotLineData(const TecplotGeometryInfo &info_) : TecplotGeometry(info_),
     polyLines()
{
}

TecplotLineData::~TecplotLineData()
{
}

bool
TecplotLineData::Read(FILE *f)
{
    bool readFloat = info.polyLineFieldType==1;
    bool readZ = info.positionCoordSys == TecplotGrid3D;

    int n = ReadInt(f);
    for(int i = 0; i < n; ++i)
    {
        polyLines.push_back(TecplotPolyline());
        polyLines[i].Read(f, readFloat, readZ);
    }
    return true;
}

bool
TecplotLineData::Write(FILE *f)
{
    TecplotGeometry::Write(f);

    bool writeFloat = info.polyLineFieldType==1;
    bool writeZ = info.positionCoordSys == TecplotGrid3D;

    WriteInt(f, polyLines.size());
    for(size_t i = 0; i < polyLines.size(); ++i)
        polyLines[i].Write(f, writeFloat, writeZ);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

TecplotRectangleData::TecplotRectangleData(const TecplotGeometryInfo &info_) : 
    TecplotGeometry(info_), xOffset(0.), yOffset(0.)
{
}

TecplotRectangleData::~TecplotRectangleData()
{
}

bool
TecplotRectangleData::Read(FILE *f)
{
    bool readFloat = info.polyLineFieldType==1;
    if(readFloat)
    {
        xOffset = (double)ReadFloat(f);
        yOffset = (double)ReadFloat(f);
    }
    else
    {
        xOffset = ReadDouble(f);
        yOffset = ReadDouble(f);
    }
    return true;
}

bool
TecplotRectangleData::Write(FILE *f)
{
    TecplotGeometry::Write(f);

    bool useFloat = info.polyLineFieldType==1;
    if(useFloat)
    {
        WriteFloat(f, (float)xOffset);
        WriteFloat(f, (float)yOffset);
    }
    else
    {
        WriteDouble(f, xOffset);
        WriteDouble(f, yOffset);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

TecplotCircleData::TecplotCircleData(const TecplotGeometryInfo &info_) : 
    TecplotGeometry(info_), radius(1.)
{
}

TecplotCircleData::~TecplotCircleData()
{
}

bool
TecplotCircleData::Read(FILE *f)
{
    bool useFloat = info.polyLineFieldType==1;
    if(useFloat)
        radius = (double)ReadFloat(f);
    else
        radius = ReadDouble(f);
    return true;
}

bool
TecplotCircleData::Write(FILE *f)
{
    TecplotGeometry::Write(f);

    bool useFloat = info.polyLineFieldType==1;
    if(useFloat)
        WriteFloat(f, (float)radius);
    else
        WriteDouble(f, radius);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

TecplotSquareData::TecplotSquareData(const TecplotGeometryInfo &info_) : 
    TecplotGeometry(info_), width(0.)
{
}

TecplotSquareData::~TecplotSquareData()
{
}

bool
TecplotSquareData::Read(FILE *f)
{
    bool useFloat = info.polyLineFieldType==1;
    if(useFloat)
        width = (double)ReadFloat(f);
    else
        width = ReadDouble(f);
    return true;
}

bool
TecplotSquareData::Write(FILE *f)
{
    TecplotGeometry::Write(f);

    bool useFloat = info.polyLineFieldType==1;
    if(useFloat)
        WriteFloat(f, (float)width);
    else
        WriteDouble(f, width);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

TecplotEllipseData::TecplotEllipseData(const TecplotGeometryInfo &info_) : 
    TecplotGeometry(info_), xRadius(0.), yRadius(0.)
{
}

TecplotEllipseData::~TecplotEllipseData()
{
}

bool
TecplotEllipseData::Read(FILE *f)
{
    bool readFloat = info.polyLineFieldType==1;
    if(readFloat)
    {
        xRadius = (double)ReadFloat(f);
        yRadius = (double)ReadFloat(f);
    }
    else
    {
        xRadius = ReadDouble(f);
        yRadius = ReadDouble(f);
    }
    return true;
}

bool
TecplotEllipseData::Write(FILE *f)
{
    TecplotGeometry::Write(f);

    bool useFloat = info.polyLineFieldType==1;
    if(useFloat)
    {
        WriteFloat(f, (float)xRadius);
        WriteFloat(f, (float)yRadius);
    }
    else
    {
        WriteDouble(f, xRadius);
        WriteDouble(f, yRadius);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

TecplotText::TecplotText() : TecplotBase(),
    positionCoordSys(TecplotGrid),
    scope(0),
    fontType(-1),
    characterHeightUnits(0),
    characterHeight(1.),
    boxType(0),
    boxMargin(1.),
    boxMarginLineWidth(1.),
    boxOutlineColor(-1),
    boxFillColor(-1),
    angle(0.),
    lineSpacing(1.),
    anchor(TecplotLeft),
    zone(0),
    color(-1),
    macroFunctionCommand(),
    clipping(TecplotClipToAxes),
    text()
{
    startLocation[0] = 0.;
    startLocation[1] = 0.;
    startLocation[2] = 0.;
}

TecplotText::~TecplotText()
{
}

bool
TecplotText::Read(FILE *f)
{
    positionCoordSys = (TecplotPositionCoordSys)ReadInt(f);
    scope = ReadInt(f);
    startLocation[0] = ReadDouble(f);
    startLocation[1] = ReadDouble(f);
    startLocation[2] = ReadDouble(f);
    fontType = ReadInt(f);
    characterHeightUnits = ReadInt(f);
    characterHeight = ReadDouble(f);
    boxType = ReadInt(f);
    boxMargin = ReadDouble(f);
    boxMarginLineWidth = ReadDouble(f);
    boxOutlineColor = ReadInt(f);
    boxFillColor = ReadInt(f);
    angle = ReadDouble(f);
    lineSpacing = ReadDouble(f);
    anchor = (TecplotTextAnchor)ReadInt(f);
    zone = ReadInt(f);
    color = ReadInt(f);
    macroFunctionCommand = ReadString(f);
    clipping = (TecplotClipping)ReadInt(f);
    text = ReadString(f);

    return true;
}

bool
TecplotText::Write(FILE *f)
{
    WriteInt(f, (int)positionCoordSys);
    WriteInt(f, scope);
    WriteDouble(f, startLocation[0]);
    WriteDouble(f, startLocation[1]);
    WriteDouble(f, startLocation[2]);
    WriteInt(f, fontType);
    WriteInt(f, characterHeightUnits);
    WriteDouble(f, characterHeight);
    WriteInt(f, boxType);
    WriteDouble(f, boxMargin);
    WriteDouble(f, boxMarginLineWidth);
    WriteInt(f, boxOutlineColor);
    WriteInt(f, boxFillColor);
    WriteDouble(f, angle);
    WriteDouble(f, lineSpacing);
    WriteInt(f, (int)anchor);
    WriteInt(f, zone);
    WriteInt(f, color);
    WriteString(f, macroFunctionCommand);
    WriteInt(f, (int)clipping);
    WriteString(f, text);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

TecplotCustomLabel::TecplotCustomLabel() : TecplotBase(), labels()
{
}

TecplotCustomLabel::~TecplotCustomLabel()
{
}

bool
TecplotCustomLabel::Read(FILE *f)
{
    int n = ReadInt(f);
    for(int i = 0; i < n; ++i)
        labels.push_back(ReadString(f));
    return true;
}

bool
TecplotCustomLabel::Write(FILE *f)
{
    WriteInt(f, labels.size());
    for(size_t i = 0; i < labels.size(); ++i)
        WriteString(f, labels[i]);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

TecplotVariableAux::TecplotVariableAux() : TecplotBase(),
    variableMarker(), name(), format(0), value()
{
}

TecplotVariableAux::~TecplotVariableAux()
{
}

bool
TecplotVariableAux::Read(FILE *f)
{
    variableMarker = ReadString(f);
    name = ReadString(f);
    format = ReadInt(f);
    value = ReadString(f);
    return true;
}

bool
TecplotVariableAux::Write(FILE *f)
{
    WriteString(f, variableMarker);
    WriteString(f, name);
    WriteInt(f, format);
    WriteString(f, value);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///                        D A T A    S E C T I O N
///////////////////////////////////////////////////////////////////////////////

TecplotConnectivity::TecplotConnectivity() : TecplotBase()
{
}

TecplotConnectivity::~TecplotConnectivity()
{
}

///////////////////////////////////////////////////////////////////////////////

TecplotOrderedConnectivity::TecplotOrderedConnectivity() : TecplotConnectivity(),
    connectivityOffset(0), connectivitySize(0)
{
}

TecplotOrderedConnectivity::~TecplotOrderedConnectivity()
{
}

TecplotConnectivity *
TecplotOrderedConnectivity::NewInstance() const
{
    return new TecplotOrderedConnectivity(*this);
}

bool
TecplotOrderedConnectivity::Read(FILE *f, const TecplotZone &,
    const TecplotDataRecord *)
{
    // write me
    return true;
}

bool
TecplotOrderedConnectivity::Write(FILE *f)
{
   // write me
   return true;
}    

///////////////////////////////////////////////////////////////////////////////

TecplotFEConnectivity::TecplotFEConnectivity() : TecplotConnectivity(),
    zoneConnectivitySize(0),
    raw1to1FaceNeighborSize(0),
    faceNeighborConnectionSize(0),
    zoneConnectivityOffset(0),
    raw1to1FaceNeighborOffset(0),
    faceNeighborConnectionOffset(0)
{
}

TecplotFEConnectivity::~TecplotFEConnectivity()
{
}

TecplotConnectivity *
TecplotFEConnectivity::NewInstance() const
{
    return new TecplotFEConnectivity(*this);
}

bool
TecplotFEConnectivity::Read(FILE *f, const TecplotZone &zone, 
    const TecplotDataRecord *data)
{
    // Calculate the sizes and offsets
    if(data->zoneNumberForConnectivity == -1)
    {
        zoneConnectivityOffset = ftell(f);
        zoneConnectivitySize = zone.GetNumElements() * 
                               TecplotNumNodesForZoneType(zone.zoneType) * 4;
        fseek(f, zoneConnectivitySize, SEEK_CUR);
    }

    if(data->zoneNumberForConnectivity == -1 &&
       zone.rawLocalFaceNeighbors > 0)
    {
        raw1to1FaceNeighborOffset = ftell(f);
        raw1to1FaceNeighborSize = zone.GetNumElements() *
                                  TecplotNumFacesForZoneType(zone.zoneType) * 4;
        fseek(f, raw1to1FaceNeighborSize, SEEK_CUR);
    }

    if(data->zoneNumberForConnectivity == -1 &&
       zone.numUserDefinedNeighborConnections != 0)
    {
        faceNeighborConnectionOffset = ftell(f);
        faceNeighborConnectionSize = zone.numUserDefinedNeighborConnections *
                                     TecplotNumNodesForZoneType(zone.zoneType) * 4;
        fseek(f, faceNeighborConnectionSize, SEEK_CUR);
    }

    return true;
}

bool
TecplotFEConnectivity::Write(FILE *f)
{
    // write me
    return true;
}

///////////////////////////////////////////////////////////////////////////////

TecplotConnectivity *
TecplotPolyConnectivity::NewInstance() const
{
    return new TecplotPolyConnectivity(*this);
}

///////////////////////////////////////////////////////////////////////////////

TecplotVariable::TecplotVariable() : dataOffset(0), dataSize(0),
    dataType(TecplotFloat), isPassive(0), zoneShareNumber(-1), 
    minValue(0.), maxValue(0.)
{
}

TecplotVariable::~TecplotVariable()
{
}

ostream &
operator << (ostream &os, const TecplotVariable &obj)
{
    os << "dataOffset = 0x" << std::hex << obj.dataOffset << std::dec << endl;
    os << "dataSize = " << obj.dataSize << endl;
    os << "dataType = " << TecplotDataType2String(obj.dataType) << endl;
    os << "isPassive = " << obj.isPassive << endl;
    os << "zoneShareNumber = " << obj.zoneShareNumber << endl;
    os << "minValue = " << obj.minValue << endl;
    os << "maxValue = " << obj.maxValue << endl;
    return os;
}

///////////////////////////////////////////////////////////////////////////////

TecplotDataRecord::TecplotDataRecord(int nv) : TecplotBase(), 
    dataOffset(0), connectivityOffset(0),
    variables(), hasPassiveVariables(0), hasVariableSharing(0), 
    zoneNumberForConnectivity(-1)
{
    for(int i = 0; i < nv; ++i)
        variables.push_back(TecplotVariable());
    connectivity = 0;
}

TecplotDataRecord::TecplotDataRecord(const TecplotDataRecord &obj) : TecplotBase(),
    dataOffset(obj.dataOffset), connectivityOffset(obj.connectivityOffset),
    variables(obj.variables),
    hasPassiveVariables(obj.hasPassiveVariables),
    hasVariableSharing(obj.hasVariableSharing),
    zoneNumberForConnectivity(obj.zoneNumberForConnectivity)
{
    if(obj.connectivity != 0)
        connectivity = obj.connectivity->NewInstance();
    else
        connectivity = 0;
}

TecplotDataRecord::~TecplotDataRecord()
{
    if(connectivity != 0)
        delete connectivity;
}

void
TecplotDataRecord::operator = (const TecplotDataRecord &obj)
{
    dataOffset = obj.dataOffset;
    connectivityOffset = obj.connectivityOffset;
    
    variables = obj.variables;
    hasPassiveVariables = obj.hasPassiveVariables;
    hasVariableSharing = obj.hasVariableSharing;
    zoneNumberForConnectivity = obj.zoneNumberForConnectivity;

    if(connectivity != 0)
        delete connectivity;
    connectivity = obj.connectivity->NewInstance();
}

bool
TecplotDataRecord::Read(FILE *f, const TecplotZone &zone)
{
    // skip past an int in version 7x
    if(TECPLOT_VERSION_7X(version))
        ReadInt(f);

    // Read the data types.
    for(size_t i = 0; i < variables.size(); ++i)
         variables[i].dataType = (TecplotDataType)ReadInt(f);

    if(!TECPLOT_VERSION_7X(version))
    {
        hasPassiveVariables = ReadInt(f);
        if(hasPassiveVariables != 0)
        {
            for(size_t i = 0; i < variables.size(); ++i)
                variables[i].isPassive = ReadInt(f);
        }

        hasVariableSharing = ReadInt(f);
        if(hasVariableSharing != 0)
        {
            for(size_t i = 0; i < variables.size(); ++i)
                variables[i].zoneShareNumber = ReadInt(f);
        }

        zoneNumberForConnectivity = ReadInt(f);
        for(size_t i = 0; i < variables.size(); ++i)
        {
            if(variables[i].zoneShareNumber == -1 && !variables[i].isPassive)
            {
                variables[i].minValue = ReadDouble(f);
                variables[i].maxValue = ReadDouble(f);
            }
        }
    }

    dataOffset = ftell(f);
    debug4 << "Data offset after reading min/max pairs: "  << dataOffset << endl;

    // Now that we've read the main part of the variables, calculate offsets to
    // the variables and skip past them to the connectivity.
    debug4 << "Zone = " << zone << endl;
    debug4 << "num nodes = " << zone.GetNumNodes() << endl;
    CalculateOffsets(zone);
    fseek(f, connectivityOffset, SEEK_SET);
    debug4 << "start of connectivity: " << std::hex << ftell(f) << endl;

    // Read the connectivity
    if(zone.zoneType == ORDERED)
        connectivity = new TecplotOrderedConnectivity;
    else if(zone.zoneType != FEPOLYGON &&
            zone.zoneType != FEPOLYHEDRON)
        connectivity = new TecplotFEConnectivity;
    else
        connectivity = new TecplotPolyConnectivity;
    connectivity->Read(f, zone, this);
    debug4 << "after reading connectivity, offset=" << ftell(f) << std::dec << endl;

    return true;
}

bool
TecplotDataRecord::Write(FILE *f)
{
    bool hasPassive = false;
    bool hasSharing = false;
    for(size_t i = 0; i < variables.size(); ++i)
    {
        WriteInt(f, (int)variables[i].dataType);
        hasPassive |= (variables[i].isPassive > 0);
        hasSharing |= (variables[i].zoneShareNumber != -1);
    }

    WriteInt(f, hasPassive ? 1 : 0);
    if(hasPassive)
    {
        for(size_t i = 0; i < variables.size(); ++i)
             WriteInt(f, variables[i].isPassive);
    }
    
    WriteInt(f, hasSharing ? 1 : 0);
    if(hasSharing)
    {
        for(size_t i = 0; i < variables.size(); ++i)
             WriteInt(f, variables[i].zoneShareNumber);
    }

    WriteInt(f, zoneNumberForConnectivity);

    for(size_t i = 0; i < variables.size(); ++i)
    {
        if(variables[i].zoneShareNumber == -1 && !variables[i].isPassive)
        {
            WriteDouble(f, variables[i].minValue);
            WriteDouble(f, variables[i].maxValue);
        }
    }

    connectivity->Write(f);

    return true;
}

ostream &
operator << (ostream &os, const TecplotDataRecord &obj)
{
    os << "TecplotDataRecord =" << endl;
    os << "{" << endl;
    for(size_t i = 0; i < obj.variables.size(); ++i)
        os << "variable[" << i << "] = " << endl << "{" << endl
           << obj.variables[i] << "}" << endl;
    os << "hasPassiveVariables = " << obj.hasPassiveVariables << endl;
    os << "hasVariableSharing = " << obj.hasVariableSharing << endl;
    os << "zoneNumberForConnectivity = " << obj.zoneNumberForConnectivity << endl;
    os << "***dataOffset = 0x" << std::hex << obj.dataOffset << endl;
    os << "***connectivityOffset = 0x" << obj.connectivityOffset << std::dec << endl;
    os << "}" << endl;
    return os;
}

// ****************************************************************************
// Method: TecplotDataRecord::CalculateOffsets
//
// Purpose: 
//   Calculates offsets to different variables.
//
// Arguments:
//   zone : The zone that contains the variables.
//
// Returns:    
//
// Note:       The variable's dataOffset will always point to the start of the
//             first relevant variable value. The dataSize for dataPacking==1 
//             is the size of the record for a point. That is, the size of all
//             of the variables summed. For block packing, the data size is the
//             number of nodes times the storage size for the variable.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 16 16:32:28 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
TecplotDataRecord::CalculateOffsets(const TecplotZone &zone)
{
    const char *mName =  "TecplotDataRecord::CalculateOffsets: ";

    if(zone.dataPacking == 1)
    {
        // Point data packing. Each variable value is stored one after the next
        // in a record format.
        long recordSize = 0;
        long offset = 0;
        for(size_t i = 0; i < variables.size(); ++i)
        {
            variables[i].dataOffset = offset;

            if(variables[i].dataType == TecplotFloat)
                recordSize += sizeof(float);
            else if(variables[i].dataType == TecplotDouble)
                recordSize += sizeof(double);
            else if(variables[i].dataType == TecplotLongInt)
                recordSize += sizeof(int);
            else if(variables[i].dataType == TecplotShortInt)
                recordSize += sizeof(short);
            else if(variables[i].dataType == TecplotByte)
                recordSize++;
            else
                recordSize++;

            offset = recordSize;
        }

        for(size_t i = 0; i < variables.size(); ++i)
        {
            variables[i].dataOffset += dataOffset;
            variables[i].dataSize = recordSize;
            debug4 << mName << "Offset to variable " << i
                   << " data = " << std::hex << variables[i].dataOffset
                   << ". size = " << std::dec << variables[i].dataSize << endl;
        }

        connectivityOffset = dataOffset + recordSize * zone.GetNumNodes();
    }
    else
    {
        int numNodes = zone.GetNumNodes();
        long offset = 0;
        for(size_t i = 0; i < variables.size(); ++i)
        {
            variables[i].dataOffset = dataOffset + offset;

            if(variables[i].dataType == TecplotFloat)
                variables[i].dataSize = numNodes * sizeof(float);
            else if(variables[i].dataType == TecplotDouble)
                variables[i].dataSize = numNodes * sizeof(double);
            else if(variables[i].dataType == TecplotLongInt)
                variables[i].dataSize = numNodes * sizeof(int);
            else if(variables[i].dataType == TecplotShortInt)
                variables[i].dataSize = numNodes * sizeof(short);
            else if(variables[i].dataType == TecplotByte)
                variables[i].dataSize = numNodes;
            else
                variables[i].dataSize = (numNodes / 8) + 1; // ???
    
            offset += variables[i].dataSize;

            debug4 << mName << "Offset to variable " << i
                   << " data = " << std::hex << variables[i].dataOffset
                   << ". size = " << std::dec << variables[i].dataSize << endl;
        }

        if(TECPLOT_VERSION_7X(version) && zone.zoneType != ORDERED)
            offset += 4;

        connectivityOffset = dataOffset + offset;
    }

    debug4 << mName << "Connectivity offset = " << std::hex
           << connectivityOffset << std::dec << endl;
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: TecplotFile::TecplotFile
//
// Purpose: 
//   Constructor for the TecplotFile class.
//
// Arguments:
//   fn : The filename that will be used.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:38:37 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

TecplotFile::TecplotFile(const std::string &fn) : TecplotBase(), titleAndVars(),
    zones(), geometries(), texts(), customLabels(), userRecs(), datasetAuxiliaryData(), 
    variableAuxiliaryData(),
    zoneData(), fileName(fn)
{
    strcpy(magic, "#!TDV108");
    tec = 0;
    readDataInformation = true; // for now.
}

// ****************************************************************************
// Method: TecplotFile::~TecplotFile
//
// Purpose: 
//   Destructor for the TecplotFile class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:39:05 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

TecplotFile::~TecplotFile()
{
    for(size_t i = 0; i < geometries.size(); ++i)
        delete geometries[i];
}

// ****************************************************************************
// Method: TecplotFile::Read
//
// Purpose: 
//   Read the Tecplot file, storing the values internally.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:39:19 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
TecplotFile::Read()
{
    bool retval = false;
    tec = fopen(fileName.c_str(), "rb");
    if(tec != 0)
    {
        retval = Read(tec);
        fclose(tec);
        tec = 0;
    }
    return retval;
}

// ****************************************************************************
// Method: TecplotFile::Write
//
// Purpose: 
//   Write the internal values to a Tecplot file.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:39:19 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
TecplotFile::Write()
{
    bool retval = false;
    tec = fopen(fileName.c_str(), "wb");
    if(tec != 0)
    {
        Write(tec);
        fclose(tec);
        tec = 0;
        retval = true;
    }
    return retval;
}

// ****************************************************************************
// Method: TecplotFile::Read
//
// Purpose: 
//   Do the actual work of reading the Tecplot file.
//
// Arguments:
//   f : The file pointer to use.
//
// Returns:    True on success, false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:40:18 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
TecplotFile::Read(FILE *f)
{
    const char *mName = "TecplotFile::Read: ";
    bool validFile = false;

    // Read the magic number.
    memset(magic, 0, sizeof(char) * 9);
    if(fread(magic, 1, 8, f) == 8)
    {
        if(strncmp(magic, "#!TDV", 5) != 0)
            return false;

        // Figure out the version so we can read the file differently, if needed
        if(strcmp(magic+5,"71 ") == 0)
            version = TECPLOT_71;
        else if(strcmp(magic+5,"75 ") == 0)
            version = TECPLOT_75;
        else if(strcmp(magic+5,"100") == 0)
            version = TECPLOT_100;
        else if(strcmp(magic+5,"101") == 0)
            version = TECPLOT_101;
        else if(strcmp(magic+5,"102") == 0)
            version = TECPLOT_102;
        else if(strcmp(magic+5,"103") == 0)
            version = TECPLOT_103;
        else if(strcmp(magic+5,"104") == 0)
            version = TECPLOT_104;
        else if(strcmp(magic+5,"105") == 0)
            version = TECPLOT_105;
        else if(strcmp(magic+5,"106") == 0)
            version = TECPLOT_106;
        else if(strcmp(magic+5,"107") == 0)
            version = TECPLOT_107;
        else if(strcmp(magic+5,"108") == 0)
            version = TECPLOT_108;
        else if(strcmp(magic+5,"109") == 0)
            version = TECPLOT_109;
        else if(strcmp(magic+5,"110") == 0)
            version = TECPLOT_110;
        else if(strcmp(magic+5,"111") == 0)
            version = TECPLOT_111;
        else
        {
            debug4 << mName << "Unsupported Tecplot version: " << (magic+5) << endl;
            return false;
        }
    }

    // Look at the file's endian flag.
    int one = 1;
    int val;
    fread((void*)&val, 1, 4, f);
    reverseEndian = one != val;

    // Read the title and variables.
    titleAndVars.Read(f);
    debug4 << titleAndVars << endl;

    bool keepReading = true;
    float recordType;
    do
    {
        recordType = ReadFloat(f);
        debug4 << mName << "Process record: " << recordType << endl;

        if(recordType == 299.0f)
        {
            TecplotZone rec(titleAndVars.varNames.size());
            rec.Read(f);
            zones.push_back(rec);
            debug4 << mName << "zone = " << endl << rec << endl;
        }
        else if(recordType == 399.0f)
        {
            TecplotGeometry *obj = 0;
            TecplotGeometryInfo info;
            info.Read(f);
            if(info.geomType == TecplotLine)
                obj = new TecplotLineData(info);
            else if(info.geomType == TecplotRectangle)
                obj = new TecplotRectangleData(info);
            else if(info.geomType == TecplotSquare)
                obj = new TecplotSquareData(info);
            else if(info.geomType == TecplotCircle)
                obj = new TecplotCircleData(info);
            else if(info.geomType == TecplotEllipse)
                obj = new TecplotEllipseData(info);
            if(obj != 0)
            {
                obj->Read(f);
                geometries.push_back(obj);
            }
            else
            {
                debug4 << mName << "Could not create geometry data." << endl;
                keepReading = false;
            }
        }
        else if(recordType == 499.0f)
        {
            TecplotText rec;
            rec.Read(f);
            texts.push_back(rec);
        }
        else if(recordType == 599.0f)
        {
            TecplotCustomLabel rec;
            rec.Read(f);
            customLabels.push_back(rec);
        }
        else if(recordType == 699.0f)
        {
            userRecs.push_back(ReadString(f));
        }
        else if(recordType == 799.0f)
        {
            TecplotDataSetAux rec;
            rec.Read(f);
            datasetAuxiliaryData.push_back(rec);
        }
        else if(recordType == 899.0f)
        {
            TecplotVariableAux rec;
            rec.Read(f);
            variableAuxiliaryData.push_back(rec);
        }
        else if(recordType == 357.0f)
        {
            debug4 << mName << "Reached end of header" << endl;
            keepReading = false;
            validFile = true;
        }
        else
        {
            debug4 << mName << "recordType=" << recordType << endl;
            keepReading = false;
        }
    } while(keepReading);

    // Now that we're done reading the header, read the variable information
    if(readDataInformation)
    {
        int nz = zones.size();
        for(int i = 0; i < nz; ++i)
        {
            if(!feof(f))
            {
                recordType = ReadFloat(f);
#if 1 // for now
                while(recordType != 299.f && !feof(f))
                {
                    debug4 << "z=" << i << "/" << zones.size() << ", Error reading recordType: " << recordType << " offset=" << ftell(f) << endl;
                    recordType = ReadFloat(f);
                }
                if(feof(f))
                    break;
#endif
                int nV = titleAndVars.varNames.size();
                zoneData.push_back(TecplotDataRecord(nV));
                zoneData[i].Read(f, zones[i]);
                debug4 << "///////////////////////////////////////////////////////" << endl;
                debug4 << zoneData[i] << endl;
            }
        }
    }

    EnsureUniqueZoneNames();

    return validFile;
}

// ****************************************************************************
// Method: TecplotFile::Write
//
// Purpose: 
//   Do the actual work of writing the Tecplot file.
//
// Arguments:
//   f : The file pointer to use.
//
// Returns:    True on success, false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:40:47 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
TecplotFile::Write(FILE *f)
{
    version = TECPLOT_111;
    strcpy(magic, "#!TDV111");
    fwrite(magic, 1, 8, f);
    int one = 1;
    fwrite((void*)&one, 1, 4, f);

    titleAndVars.Write(f);

    // Write the zone records.
    for(size_t i = 0; i < zones.size(); ++i)
    {
        WriteFloat(f, 299.0f);
        zones[i].Write(f);
    }

    // Write the geometry records
    for(size_t i = 0; i < geometries.size(); ++i)
    {
        WriteFloat(f, 399.0f);
        geometries[i]->Write(f);
    }

    // Write the text records
    for(size_t i = 0; i < texts.size(); ++i)
    {
        WriteFloat(f, 499.0f);
        texts[i].Write(f);
    }

    // Write the custom label records
    for(size_t i = 0; i < customLabels.size(); ++i)
    {
        WriteFloat(f, 599.0f);
        customLabels[i].Write(f);
    }

    // Write the user records.
    for(size_t i = 0; i < userRecs.size(); ++i)
    {
        WriteFloat(f, 699.0f);
        WriteString(f, userRecs[i]);
    }

    // Write the dataset auxiliary records.
    for(size_t i = 0; i < datasetAuxiliaryData.size(); ++i)
    {
        WriteFloat(f, 799.0f);
        datasetAuxiliaryData[i].Write(f);
    }

    // Write the variable auxiliary records.
    for(size_t i = 0; i < variableAuxiliaryData.size(); ++i)
    {
        WriteFloat(f, 899.0f);
        variableAuxiliaryData[i].Write(f);
    }

    // Write the end of header record
    WriteFloat(f, 357.f);

    // Write the zone data
    for(size_t i = 0; i < zoneData.size(); ++i)
    {
        WriteFloat(f, 299.0f);
        zoneData[i].Write(f);
    }

    return true;
}

// ****************************************************************************
// Method: TecplotFile::EnsureUniqueZoneNames
//
// Purpose: 
//   Iterates over the zone names and ensures they are unique.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 16 15:23:39 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
TecplotFile::EnsureUniqueZoneNames()
{
    std::map<std::string, std::vector<int> > zoneNames;
    for(size_t i = 0; i < zones.size(); ++i)
        zoneNames[zones[i].zoneName].push_back(i);

    std::map<std::string, std::vector<int> >::iterator pos;
    for(pos = zoneNames.begin(); pos != zoneNames.end(); ++pos)
    {
        if(pos->second.size() > 1)
        {
            for(size_t i = 0; i < pos->second.size(); ++i)
            {
                char suffix[20];
                sprintf(suffix, "_%02d", i+1);
                zones[i].zoneName += suffix;
            }
        }
    }
}

//**************************************************************************
// Method: TecplotFile::ReadData
//
// Purpose: 
//   Internal method for reading a chunk of data from the file and converting
//   the endianness, if needed.
//
// Arguments:
//   dataOffset : The offset from the start of the file where the chunk begins.
//   dataSize   : The number of bytes used to store the chunk.
//   dataType   : The type of data represented in the chunk.
//   ptr        : The destination address for the chunk in memory.
//
// Returns:    True on success; false othewise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:42:17 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
TecplotFile::ReadData(long dataOffset, long dataSize, TecplotDataType dataType, 
    int dataPacking, int nnodes, void *ptr)
{
    bool retval = false;
    if((tec = fopen(fileName.c_str(), "rb")) != 0)
    {
        fseek(tec, dataOffset, SEEK_SET);

        if(dataPacking == 1)
        {
            // Read the part of the record that we care about.
            int nBytes = TecplotNumBytesForType(dataType);
            char *cptr = (char *)ptr;
            long skip = dataSize - nBytes;
            for(int i = 0; i < nnodes; ++i)
            {
                fread(cptr, 1, nBytes, tec);
                fseek(tec, skip, SEEK_CUR);
                cptr += nBytes;
            }
        }
        else
            fread(ptr, 1, dataSize, tec);
        fclose(tec);
        tec = 0;

        // Now that we've read the data, reverse endians if needed.
        if(reverseEndian)
        {
            if(dataType == TecplotFloat ||
               dataType == TecplotLongInt)
            {
                tecplot_reverse_endian32(ptr, dataSize);
            }
            else if(dataType == TecplotDouble)
                tecplot_reverse_endian64(ptr, dataSize);
            else if(dataType == TecplotShortInt)
                tecplot_reverse_endian16(ptr, dataSize);
        }

        retval = true;
    }
    return retval;
}

// ****************************************************************************
// Method: TecplotFile::ReadVariable
//
// Purpose: 
//   Reads the named variable from the specified zone into an array in its 
//   native precision.
//
// Arguments:
//   zoneId  : The zone number that contains the variable.
//   varName : The name of the variable.
//   ptr     : The destination array for the data.
//
// Returns:    True on success; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:44:35 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
TecplotFile::ReadVariable(int zoneId, const std::string &varName, void *ptr)
{
    bool retval = false;
    int varId = -1;
    if(zoneId >= 0 && 
       zoneId < (int)zones.size() &&
       (varId = VarNameToIndex(varName)) != -1
      )
    {
        const TecplotVariable &var = zoneData[zoneId].variables[varId];
        if(var.isPassive)
        {
            retval = true;
            memset(ptr, 0, var.dataSize);
        }
        else
        {
            retval = ReadData(var.dataOffset, var.dataSize, var.dataType,
                zones[zoneId].dataPacking, zones[zoneId].GetNumNodes(),
                ptr);
        }
    }
    return retval;
}

// ****************************************************************************
// Method: TecplotFile::ReadVariableAsFloat
//
// Purpose: 
//   Reads the named variable from the specified zone into a float array,
//   converting the data to float if necessary.
//
// Arguments:
//   zoneId  : The zone number that contains the variable.
//   varName : The name of the variable.
//   ptr     : The destination array for the data.
//
// Returns:    True on success; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:45:57 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
TecplotFile::ReadVariableAsFloat(int zoneId, const std::string &varName, float *ptr)
{
    bool retval = false;
    int varId = -1;
    if(zoneId >= 0 && 
       zoneId < (int)zones.size() &&
       (varId = VarNameToIndex(varName)) != -1
      )
    {
        const TecplotVariable &var = zoneData[zoneId].variables[varId];
        if(var.dataType == TecplotFloat)
            retval = ReadVariable(zoneId, varName, ptr);
        else
        {
            long dataSize;
            unsigned int N = (unsigned int)zones[zoneId].GetNumNodes();
            if(zones[zoneId].dataPacking == 1)
                dataSize = N * TecplotNumBytesForType(var.dataType);
            else
                dataSize = var.dataSize;
            void *storage = malloc(dataSize);
            if(storage != 0)
            {
                retval = ReadVariable(zoneId, varName, storage);
                if(var.dataType == TecplotDouble)
                {
                    double *src = (double *)storage;
                    float  *dest = ptr;
                    for(unsigned int i = 0; i < N; ++i)
                        *dest++ = (float)*src++;
                }
                else if(var.dataType == TecplotLongInt)
                {
                    int *src = (int *)storage;
                    float  *dest = ptr;
                    for(unsigned int i = 0; i < N; ++i)
                        *dest++ = (float)*src++;
                }
                else if(var.dataType == TecplotShortInt)
                {
                    short *src = (short *)storage;
                    float  *dest = ptr;
                    for(unsigned int i = 0; i < N; ++i)
                        *dest++ = (float)*src++;
                }

                free(storage);
            }    
        }        
    }

    return retval;
}

// ****************************************************************************
// Method: TecplotFile::ReadFEConnectivity
//
// Purpose: 
//   Reads some of the FE connectivity data and returns a new array containing
//   that data.
//
// Arguments:
//
// Returns:    
//
// Note:       This does not account for reading all types of FE connectivity
//             arrays that may be present; just one type.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:46:55 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
TecplotFile::ReadFEConnectivity(int zoneId, int **ptr)
{
    bool retval = false;
    if(zoneId >= 0 &&
       zoneId < (int)zones.size() &&
       zones[zoneId].zoneType != ORDERED)
    {
        TecplotFEConnectivity *conn = (TecplotFEConnectivity *)
            zoneData[zoneId].connectivity;
        
        *ptr = new int[conn->zoneConnectivitySize/4];

        retval = ReadData(
            conn->zoneConnectivityOffset,
            conn->zoneConnectivitySize,
            TecplotLongInt,
            0,
            -1,
            (void *)*ptr);
    }
    return retval;
}

// ****************************************************************************
// Method: TecplotFile::ZoneNameToIndex
//
// Purpose: 
//   Converts a zone name into a numeric index.
//
// Arguments:
//   zName : The zone name.
//
// Returns:    A number in [0,zones.size()-1] or -1 if not found.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:47:56 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

int
TecplotFile::ZoneNameToIndex(const std::string &zName) const
{
    int retval = -1;
    for(size_t i = 0; i < zones.size(); ++i)
    {
        if(zones[i].zoneName == zName)
        {
            retval = i;
            break;
        }
    }
    return retval;
}

// ****************************************************************************
// Method: TecplotFile::VarNameToIndex
//
// Purpose: 
//   Converts a variable name into a numeric index.
//
// Arguments:
//   varName : The variable name.
//
// Returns:    A number in [0,varNames.size()-1] or -1 if not found.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 12 11:47:56 PDT 2008
//
// Modifications:
//   
// ****************************************************************************


int
TecplotFile::VarNameToIndex(const std::string &varName) const
{
    int retval = -1;
    for(size_t i = 0; i < titleAndVars.varNames.size(); ++i)
    {
        if(titleAndVars.varNames[i] == varName)
        {
            retval = i;
            break;
        }
    }
    return retval;
}

// ****************************************************************************
// Method: TecplotFile::CoordinateVariable
//
// Purpose: 
//   Return the variable nmae for a coordinate axis.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 13 14:29:44 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

std::string
TecplotFile::CoordinateVariable(int axis) const
{
    std::string possibilities[12];
    possibilities[0] = std::string("X");
    possibilities[1] = std::string("x");
    possibilities[2] = std::string("CoordinateX");
    possibilities[3] = std::string("I");

    possibilities[4] = std::string("Y");
    possibilities[5] = std::string("y");
    possibilities[6] = std::string("CoordinateY");
    possibilities[7] = std::string("J");

    possibilities[8] = std::string("Z");
    possibilities[9] = std::string("z");
    possibilities[10] = std::string("CoordinateZ");
    possibilities[11] = std::string("K");

    const std::vector<std::string> &vars = titleAndVars.varNames;
    for(size_t i = 0; i < vars.size(); ++i)
    {
        for(int p = 0; p < 4; ++p)
           if(vars[i] == possibilities[4 * axis + p])
               return vars[i];
    }

    return (axis < 2) ? vars[axis] : std::string();
}

// ****************************************************************************
// Method: TecplotFile::DisableReadingDataInformation
//
// Purpose: 
//   Turn off reading data information section of the file to speed up
//   reading.
//
// Arguments:
//
// Returns:    
//
// Note:       Must be called before Read().
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 13 14:29:59 PDT 2008
//
// Modifications:
//   Kathleen Bonnell, Thu Jun 26 10:27:12 PDT 2008
//   Change return type to void.
//   
// ****************************************************************************

void
TecplotFile::DisableReadingDataInformation()
{
    readDataInformation = false;
}

// ****************************************************************************
// Method: TecplotFile::GetNumSpatialDimensions
//
// Purpose: 
//   Returns the number of spatial dimensions for the given zone.
//
// Arguments:
//   zoneId : The zone whose spatial dimensions we want.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 13 14:53:59 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

int
TecplotFile::GetNumSpatialDimensions(int zoneId) const
{
    return (CoordinateVariable(2).size() > 0) ? 3 : 2;
}

// ****************************************************************************
// Method: TecplotFile::GetNumTopologicalDimensions
//
// Purpose: 
//   Returns the number of topological dimensions for the given zone.
//
// Arguments:
//   zoneId : The zone whose spatial dimensions we want.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 16 15:24:13 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

int
TecplotFile::GetNumTopologicalDimensions(int zoneId) const
{
    return zones[zoneId].GetNumTopologicalDimensions();
}

ostream &
operator << (ostream &os, const TecplotFile &obj)
{
   os << "magic=" << obj.magic << endl;
   os << "reverseEndian=" << (obj.reverseEndian?"true":"false") << endl;
   os << obj.titleAndVars;
   os << "Dataset Auxiliary Data" << endl;
   for(size_t i = 0; i < obj.datasetAuxiliaryData.size(); ++i)
       os << "    " << obj.datasetAuxiliaryData[i] << endl;
   os << "Zones" << endl;
   for(size_t i = 0; i < obj.zones.size(); ++i)
       os << "    " << obj.zones[i] << endl;
   for(size_t i = 0; i < obj.zoneData.size(); ++i)
       os << obj.zoneData[i] << endl;
   return os;
}
