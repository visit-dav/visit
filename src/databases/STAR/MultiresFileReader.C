/*****************************************************************************
*
* Copyright (c) 2010, University of New Hampshire Computer Science Department
* All rights reserved.
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

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  File:        MultiresFileReader.cpp                                      //
//  Programmer:  Andrew Foulks <rafoulks@cs.unh.edu>                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "MultiresFileReader.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

/* ========================================================================= */

MultiresFileReader::MultiresFileReader() 
{
    TRACE();

    mDataBuffer = NULL;
    mHeaderSize = 0;
    mFilename = "";   
    mGridFilename = "";
    mNumResolutions = 0;
    mFileVersion = 0.0;
    mDataRank = "";
    mVariableName = "variable";
    mHasMinVal = false;
    mHasMaxVal = false;
}

/* ========================================================================= */

MultiresFileReader::MultiresFileReader(const char* filename) 
{
    TRACE();

    mDataBuffer = NULL;
    mHeaderSize = 0;
    mFilename = filename;
    mGridFilename = "";
    mDataFilename = "";
    mNumResolutions = 0;
    mFileVersion = 0.0;
    mDataRank = "";
    mVariableName = "variable";
    mHasMinVal = false;
    mHasMaxVal = false;

    parseFile(filename);

    DEBUG("filename=%s", filename);
    DEBUG("file version=%f", mFileVersion);
    DEBUG("numResolutions=%d", mNumResolutions);
    DEBUG("numErrorDataSets=%d", mNumErrorDataSets);
    DEBUG("gridFilename=%s", gridFilename().c_str());
    DEBUG("dataFilename=%s", mDataFilename.c_str());
    DEBUG("variablename=%s", mVariableName.c_str());
    DEBUG("headersize=%d", mHeaderSize);
    DEBUG("minmax=[%f,%f]", mMinVal, mMaxVal);
    DEBUG("isScalar=%s", isScalar()?"true":"false");
    DEBUG("isVectorComponent=%s", isVectorComponent()?"true":"false");
    for(int i=0; i<mNumResolutions; i++)
        DEBUG("res %d: chunks=[%d|%d|%d]", 
                i, mNumXchunks[i], mNumYchunks[i], mNumZchunks[i]);
}

/* ========================================================================= */

void MultiresFileReader::parseFile(const char* filename)
{
    TRACE(filename);
    DEBUG("MultiresFileReader is '%s'", filename);

    const char* line = NULL;
    mFilename = filename;
    FILE* infile = fopen(filename, "rb");

    if(infile == NULL) {
        ERROR("Unable to open file '%s' for reading", filename);
        return;
    }

    try {
        line = getNextLine(infile);
        mFileVersion        = parseVersionNumber(line);
        mHeaderSize         = parseHeaderSize(line);
        mNumResolutions     = parseNumResolutions(getNextLine(infile));
        mNumErrorDataSets   = parseNumErrorDataSets(getNextLine(infile));
        mDataFilename       = parseDataFilename(getNextLine(infile));
        mGridFilename       = parseGridFilename(getNextLine(infile));
        mDataType           = parseDataType(getNextLine(infile));
        mDataRank           = parseDataRank(getNextLine(infile));
        mVariableName       = parseVariableName(getNextLine(infile));

        line = getNextLine(infile);
        mNumXchunks         = parseNumXchunks(line);
        mNumYchunks         = parseNumYchunks(line);
        mNumZchunks         = parseNumZchunks(line);

        line = getNextLine(infile);
        mMinVal             = parseMinVal(line);
        mMaxVal             = parseMaxVal(line);

        mOffsets.resize(mNumResolutions, 0);
        mWidths.resize(mNumResolutions);
        mHeights.resize(mNumResolutions);
        mDepths.resize(mNumResolutions);
        mChunkSizes.resize(mNumResolutions);
        mRawDataPtrs.resize(mNumResolutions, NULL);

        for(int i=0; i<mNumResolutions; i++) {
            int resolution;
            int width;
            int height;
            int depth;
            long long offset;

            parseResolutionMetadata(getNextLine(infile), 
                                    resolution, 
                                    width, 
                                    height, 
                                    depth, 
                                    offset);

            BOUNDS_CHECK(resolution, 0, (int)mOffsets.size());
            BOUNDS_CHECK(resolution, 0, (int)mWidths.size());
            BOUNDS_CHECK(resolution, 0, (int)mHeights.size());
            BOUNDS_CHECK(resolution, 0, (int)mDepths.size());

            mOffsets[resolution] = offset;
            mWidths[resolution] = width;
            mHeights[resolution] = height;
            mDepths[resolution] = depth;

            int chunkwidth = width/mNumXchunks[resolution];
            int chunkheight = height/mNumYchunks[resolution];
            int chunkdepth = depth/mNumZchunks[resolution];
            mChunkSizes[resolution] = chunkwidth * chunkheight * chunkdepth;
        }

        for(int i=0; i<mNumErrorDataSets; i++) {
            ERROR("Error data not supported yet");
        }

        fclose(infile);
    }
    catch(IOerror& e) {
        ERROR("I/O Error.  Something bad will probably happen.\n");
    }
}

/* ========================================================================= */
/**
 *      Returns the full path to the grid filename.  If the filename
 *      in the metadata is a relative name, prepend the directory
 *      path of this .mrd file.  If the gridfilename is absolute,
 *      simply return it.
 **/

string MultiresFileReader::gridFilename() const 
{
    string gridfile = "error in MultiresFileReader::gridFilename()";

    if(mGridFilename[0] == '/') {           // absolute path name
        gridfile = mGridFilename;
    }
    else {
        vector<string> pathAndFile = splitPathName(mFilename);
        REQUIRE(pathAndFile.size()>=2, "internal error, fullpath '%s' "
                "doesn't split into separate path and filename, i'm confused",
                mFilename.c_str());

        string path = pathAndFile[0];
        string file = pathAndFile[1];

        gridfile = path + "/" + mGridFilename;
    }

    return gridfile;
}

/* ========================================================================= */
/**
 *      For a single file, there is only one timestep, and
 *      it is in the filename:
 *
 *          jcd0004.rr.000060.mrd
 *                     ^^^^^^
 *                        +-- '000060' is the timestep
 *
 *      And if it isn't in the filename, just call it time 0.
 **/

vector<int> MultiresFileReader::timeStepList() const
{
    vector<string> tokens = split(mFilename, ".\t\n");
    vector<int> timesteps;

    if(tokens.size() >= 3) {
        if(isInt(tokens[2]))
            timesteps.push_back(toInt(tokens[2]));
        else
            timesteps.push_back(0);
    }
    else {
        timesteps.push_back(0);
    }

    return timesteps;
}

/* ========================================================================= */
/**
 *      The attribute name is the second piece of the filename:
 *
 *      jcd0004.rr.000060.mrd
 *              ^^
 *               +-- 'rr' is density
 *
 *      The @param index is the index into the 'array'
 *      of scalars/vectors.  For a single file, there is at
 *      most one scalar or vector, so there is no array, 
 *      and the index should always be 0.
 **/

string MultiresFileReader::variableNameAt(int index) const
{
    TRACE(index);
    REQUIRE(index==0,"index=%d, but must be 0", index);

    // if there was a variable name in the file, use that
    if(mVariableName != "") {
        return mVariableName;
    }
    else {  // otherwise, try to get it from the filename
        vector<string> tokens = split(mFilename, ".\t\n");

        if(tokens.size() >= 2)
            return tokens[1];
    }

    // if we get here, something's probably gone wrong
    return "unknown variable";
}

/* ========================================================================= */
/**
 *      Returns true is the variable is a scalar or a component
 *      of a vector.
 *
 *      @param name -- the name if the variable is optional,
 *             since a single file only represents a single
 *             variable.  Default value is the empty string (""),
 *             but if the name is passed in, it must match
 *             the variable name in the file.
 **/

bool MultiresFileReader::isScalar(const string& name) const
{
    TRACE(name);
    REQUIRE(name=="" or name==variableNameAt(0),
            "invalid name '%s', this file is for '%s' only",
             name.c_str(), variableNameAt(0).c_str());

    return mDataRank == "scalar" or mDataRank == "vectorcomponent";
}

/* ========================================================================= */
/**
 *      Is the variable is a component of a vector, returns
 *      that component ('x', 'y', or 'z') as a single
 *      char.  If not, returns the character 0 ('\0').
 *
 *      @param name -- the name if the variable is optional,
 *             since a single file only represents a single
 *             variable.  Default value is the empty string (""),
 *             but if the name is passed in, it must match
 *             the variable name in the file.
 **/

char MultiresFileReader::isVectorComponent(const string& name) const
{
    TRACE(name);
    REQUIRE(name=="" or name==variableNameAt(0),
            "invalid name '%s', this file is for '%s' only",
             name.c_str(), variableNameAt(0).c_str());

    char component = '\0';

    if(mDataRank == "vectorcomponent") {
        string name = variableNameAt(0);

        component = name[name.length()-1];

        REQUIRE((component=='x'||component=='y'||component=='z'),
                "component is '%c', needs to be 'x', 'y', or 'z'",component);
    }

    return component;
}

/* ========================================================================= */
/**
 *      Returns true is the variable is vector.
 *
 *      @param name -- the name if the variable is optional,
 *             since a single file only represents a single
 *             variable.  Default value is the empty string (""),
 *             but if the name is passed in, it must match
 *             the variable name.
 **/

bool MultiresFileReader::isVector(const string& name) const
{
    TRACE(name);
    REQUIRE(name=="" or name==variableNameAt(0),
            "invalid name '%s', this file is for '%s' only",
             name.c_str(), variableNameAt(0).c_str());

    return mDataRank == "vector";
}

/* ========================================================================= */
/**
 *      Returns true is the variable is a tensor.
 *
 *      @param name -- the name if the variable is optional,
 *             since a single file only represents a single
 *             variable.  Default value is the empty string (""),
 *             but if the name is passed in, it must match
 *             the variable name.
 **/

bool MultiresFileReader::isTensor(const string& name) const
{
    TRACE(name);
    REQUIRE(name=="" or name==variableNameAt(0),
            "invalid name '%s', this file is for '%s' only",
             name.c_str(), variableNameAt(0).c_str());

    return mDataRank == "tensor";
}

/* ========================================================================= */

int MultiresFileReader::width(int resolution) const
{
    TRACE(resolution);
    BOUNDS_CHECK(resolution, 0, (int)mWidths.size());

    return mWidths[resolution];
}

/* ========================================================================= */

int MultiresFileReader::height(int resolution) const
{
    TRACE(resolution);
    BOUNDS_CHECK(resolution, 0, (int)mHeights.size());

    return mHeights[resolution];
}

/* ========================================================================= */

int MultiresFileReader::depth(int resolution) const
{
    TRACE(resolution);
    BOUNDS_CHECK(resolution, 0, (int)mDepths.size());

    return mDepths[resolution];
}

/* ========================================================================= */
/**
 *      Returns the number of chunks along the width axis
 *      for the specified resolution.  
 **/

int MultiresFileReader::numXchunks(int resolution) const
{
    TRACE(resolution);
    BOUNDS_CHECK(resolution, 0, (int)mNumXchunks.size());

    return mNumXchunks[resolution];
}

/* ========================================================================= */
/**
 *      Returns the number of chunks along the height axis
 *      for the specified resolution.  
 **/

int MultiresFileReader::numYchunks(int resolution) const
{
    TRACE(resolution);
    BOUNDS_CHECK(resolution, 0, (int)mNumYchunks.size());

    return mNumYchunks[resolution];
}

/* ========================================================================= */
/**
 *      Returns the number of chunks along the depth axis
 *      for the specified resolution.  
 **/

int MultiresFileReader::numZchunks(int resolution) const
{
    TRACE(resolution);
    BOUNDS_CHECK(resolution, 0, (int)mNumZchunks.size());

    return mNumZchunks[resolution];
}

/* ========================================================================= */
/**
 *      Returns a pointer to the data at the specified resolution.
 *      This class is still the 'owner' of the memory -- don't
 *      delete it, call @see freeRawDataMemory() when done.
 *
 *      @param resolution -- integer resolution, 0 is the highest
 *
 *      @param variableName -- the name of the scalar, vector
 *             or tensor to get the data for.  For this class
 *             there is only one attribute, so the parameter
 *             is optional (default to "", the empty string), 
 *             but if it is specified, must match this file's
 *             variable name.
 *
 *      @param fileIndex -- since this class only represents 
 *             a single file, this parameter is optional 
 *             (default to 0), but if specified should be 0.
 *
 *      Here, we must support the full interface for DataManagerAPI,
 *      so the parameters fileIndex and variableName are part
 *      of the signature.  For a single file, the file index 
 *      must be 0, and the variablename must match the variable 
 *      name for this file's data.
 **/

const float* MultiresFileReader::rawData(const string& variableName,
                                   int resolution, 
                                   int fileIndex,
                                   int chunkIndex)
{
    TRACE(variableName, resolution, fileIndex);
    REQUIRE(fileIndex==0,
            "fileIndex=%d, must be 0 for this function",fileIndex);
    REQUIRE(variableName==variableNameAt(0),
            "variableName='%s', but this file is for '%s'",
             variableName.c_str(), variableNameAt(0).c_str());
    BOUNDS_CHECK(resolution, 0, (int)mRawDataPtrs.size());
    BOUNDS_CHECK(chunkIndex, 0, (mNumXchunks[resolution]*
                mNumYchunks[resolution]*mNumZchunks[resolution]));

    if(mDataBuffer == NULL) {
        loadDataFromFile();
    }

    NOTNULL(mDataBuffer);
    NOTNULL(mRawDataPtrs[resolution]);

    float* data = mRawDataPtrs[resolution];

    // calculate the number of values to offset to get to the chunk
    long long chunkOffset = mChunkSizes[resolution] * chunkIndex; 
    data += chunkOffset;

    return data;
}

/* ========================================================================= */
/**
 *      Frees the memory used to hold ALL resolutions of data,
 *      but not the metadata information (number of resolutions,
 *      sizes of each resolution, etc).
 *
 *      @param variableName -- the name of the scalar, vector
 *             or tensor to get the data for.  For this class
 *             there is only one attribute, so the parameter
 *             is optional (default to "", the empty string), 
 *             but if it is specified, must match this file's
 *             variable name.
 *
 *      @param fileIndex -- since this class only represents 
 *             a single file, this parameter is optional 
 *             (default to 0), but if specified should be 0.
 **/

void MultiresFileReader::freeRawDataMemory(const string& variableName,
                                     int fileIndex)
{
    TRACE();
    REQUIRE(fileIndex==0,
            "fileIndex=%d, must be 0 for this function",fileIndex);
    REQUIRE((variableName==""||variableName==variableNameAt(0)),
            "variableName='%s', but this file is for '%s'",
             variableName.c_str(), variableNameAt(0).c_str());
    REQUIRE((int)mRawDataPtrs.size()==mNumResolutions,
           "Internal error, unexpected size mismatch: mRawDataPtrs.size()="
            "%d, mNumResolutions=%d",(int)mRawDataPtrs.size(),mNumResolutions);

    delete [] mDataBuffer;

    mDataBuffer = NULL;

    for(int i=0; i<mNumResolutions; i++)
        mRawDataPtrs[i] = NULL;
}

/* ========================================================================= */
/**
 *      I/O operation: Loads the data from the data file 
 *      into the data buffer.
 **/

void MultiresFileReader::loadDataFromFile()
{
    REQUIRE(mDataBuffer==NULL,
            "data already loaded from file %s",filename());

    FILE* infile = fopen(mDataFilename.c_str(), "rb");

    if(infile == NULL) {
        ERROR("Unable to open file '%s' for reading", mDataFilename.c_str());
        return;
    }

    int totalNumElements = 0;
    for(int i=0; i<mNumResolutions; i++) {
        totalNumElements += mWidths[i]*mHeights[i]*mDepths[i];
    }

    mDataBuffer = new float[totalNumElements];

    float* ptr = mDataBuffer;
    for(int i=0; i<mNumResolutions; i++) {
        int numElements = mWidths[i]*mHeights[i]*mDepths[i];

        fseek(infile, mOffsets[i], SEEK_SET);
        size_t res = fread(ptr, 4, numElements, infile); (void) res;

        mRawDataPtrs[i] = ptr;
        ptr += numElements;
    }

    fclose(infile);
}

/* ========================================================================= */
/**
 *      The version is on the first line.
 *      Syntax: 
 *
 *      # MRD DATA v=1.0; headersize=  1024
 *
 **/

float MultiresFileReader::parseVersionNumber(const char* line)
{
    TRACE(line);
    
    float version = 1.0;

    vector<string> tokens = split(line, " #;=\t\n");

    if(tokens.size() >= 6) {
        REQUIRE(tokens[2]=="v","invalid syntax, line '%s'", line);
        version = toFloat(tokens[3]);
    }
    else {
        ERROR("Unable to get version from line '%s'\n", line);
    }

    return version;
}

/* ========================================================================= */
/**
 *      The header size is on the first line:
 *  
 *      # MRD DATA v=1.0; headersize=  1024
 *
 **/

int MultiresFileReader::parseHeaderSize(const char* line)
{
    TRACE(line);

    // default
    int headersize = 0;

    vector<string> tokens = split(line, " ;#=\t\n");

    if(tokens.size() >= 6) {
        REQUIRE(tokens[4]=="headersize","invalid syntax, line '%s'", line);
        headersize = toInt(tokens[5]);
    }
    else {
        ERROR("Unable to get headersize from line '%s'\n", line);
    }

    return headersize;
}

/* ========================================================================= */
/**
 *  Syntax: 
 *
 *  # numresolutions=4
 **/

int MultiresFileReader::parseNumResolutions(const char* line)
{
    TRACE(line);

    // default
    int numResolutions = 1;

    vector<string> tokens = split(line, " #=\t\n");

    if(tokens.size() >= 2) {
        REQUIRE(tokens[0]=="numresolutions","invalid LHS, line '%s'", line);
        numResolutions = toInt(tokens[1]);
    }
    else {
        ERROR("Unable to get numResolutions from line '%s'\n", line);
    }

    return numResolutions;
}

/* ========================================================================= */
/**
 *  Syntax: 
 *
 *  # numerrordatasets=0
 **/

int MultiresFileReader::parseNumErrorDataSets(const char* line)
{
    TRACE(line);

    // default
    int numErrorDataSets = 0;

    vector<string> tokens = split(line, " #=\t\n");

    if(tokens.size() >= 2) {
        REQUIRE(tokens[0]=="numerrordatasets","invalid LHS, line '%s'", line);
        numErrorDataSets = toInt(tokens[1]);
    }
    else {
        ERROR("Unable to get numResolutions from line '%s'\n", line);
    }

    return numErrorDataSets;
}

/* ========================================================================= */
/**
 *      Gets the data file name, if not the file currently being 
 *      processed.  
 *
 *      Syntax: 
 *
 *          # datafile=
 *      or
 *          # datafile=./mydataset.bin
 **/

string MultiresFileReader::parseDataFilename(const char* line)
{
    TRACE(line);

    // default
    string filename = "";

    vector<string> tokens = split(line, " #=\t\n");

    if(tokens.size() == 1) {
        REQUIRE(tokens[0]=="datafile","invalid LHS, line '%s'", line);
        // if there is nothing on the RHS, the data is with the metadata
        filename = mFilename;
    }
    else if(tokens.size() >= 2) {
        REQUIRE(tokens[0]=="datafile","invalid LHS, line '%s'", line);
        filename = tokens[1];
    }
    else {
        ERROR("Unable to get data filename from line '%s'\n", line);
    }

    return filename;
}

/* ========================================================================= */
/**
 *      Syntax: 
 *
 *      # gridfile=jcd0004.392x112x112.grid
 **/

string MultiresFileReader::parseGridFilename(const char* line)
{
    TRACE(line);

    // default
    string filename = "";

    vector<string> tokens = split(line, " #=\t\n");

    if(tokens.size() == 1) {
        REQUIRE(tokens[0]=="gridfile","invalid LHS, line '%s'", line);
        // it's okay to not have a grid file
        filename = "";
    }
    else if(tokens.size() >= 2) {
        REQUIRE(tokens[0]=="gridfile","invalid LHS, line '%s'", line);
        filename = tokens[1];
    }
    else {
        ERROR("Unable to get grid filename from line '%s'\n", line);
    }

    return filename;
}

/* ========================================================================= */
/**
 *  Syntax: 
 *
 *  # datatype=float
 *
 *  Anything other than float, we are hosed.
 **/

string MultiresFileReader::parseDataType(const char* line)
{
    TRACE(line);

    // default
    string datatype = "float";

    vector<string> tokens = split(line, " #=\t\n");

    if(tokens.size() >= 2) {
        REQUIRE(tokens[0]=="datatype","invalid LHS, line '%s'", line);
        datatype = tokens[1];

        if(datatype != "float") {
            ERROR("Invalid datatype on line '%s'\n", line);

            datatype = "float";
        }
    }
    else {
        ERROR("Unable to get datatype from line'%s'\n", line);
    }

    return datatype;
}

/* ========================================================================= */ /**
 *      Syntax: 
 *
 *      # datarank=scalar
 *
 *      may also be 'vector', or 'vector_component'
 **/

string MultiresFileReader::parseDataRank(const char* line)
{
    TRACE(line);

    // default
    string dataRank = "scalar";

    vector<string> tokens = split(line, " #=\t\n");

    if(tokens.size() >= 2) {
        REQUIRE(tokens[0]=="datarank","invalid LHS, line '%s'", line);
        dataRank = tokens[1];
    }
    else {
        ERROR("Unable to get datarank from line'%s'\n", line);
    }

    return dataRank;
}

/* ========================================================================= */ /**
 *      Syntax: 
 *
 *      # variablename=rr
 **/

string MultiresFileReader::parseVariableName(const char* line)
{
    TRACE(line);

    // default
    string varName = "variable";

    vector<string> tokens = split(line, " #=\t\n");

    if(tokens.size() >= 2) {
        REQUIRE(tokens[0]=="variablename","invalid LHS for line '%s'", line);
        varName = tokens[1];
    }
    else {
        ERROR("Unable to get datarank from line'%s'\n", line);
    }

    return varName;
}

/* ========================================================================= */
/**
 *      Syntax: 
 *
 *      # minmax=[min, max]
 **/

float MultiresFileReader::parseMinVal(const char* line)
{
    TRACE(line);

    // defaults
    float min = 0.0;

    vector<string> tokens = split(line, " #=,[]\t\n");

    if(tokens.size() == 1) {  // empty line is okay -- will get it from data
        min = 0.0;
    }
    else if(tokens.size() >= 3) {
        REQUIRE(tokens[0]=="minmax","invalid LHS, line '%s'", line);
        min = toFloat(tokens[1]);
        mHasMinVal = true;
    }
    else {
        ERROR("Unable to get min value from line '%s'\n", line);
    }

    return min;
}

/* ========================================================================= */
/**
 *      Syntax: 
 *
 *      # minmax=[min, max]
 **/

float MultiresFileReader::parseMaxVal(const char* line)
{
    TRACE(line);

    // defaults
    float max = 0.0;

    vector<string> tokens = split(line, " #=,[]\t\n");

    if(tokens.size() == 1) {  // empty line is okay -- will get it from data
        max = 0.0;
    }
    else if(tokens.size() >= 3) {
        REQUIRE(tokens[0]=="minmax","invalid LHS, line '%s'", line);
        max = toFloat(tokens[2]);
        mHasMaxVal = true;
    }
    else {
        ERROR("Unable to get max value from line '%s'\n", line);
    }

    return max;
}

/* ========================================================================= */
/**
 *      Syntax: 
 *
 *      # numchunks=[1,1,1]
 *          (if each resolution uses the same chunking scheme)
 *
 *         or
 *
 *      # numchunks={[1|1|1]; [1|1|1]; [2|2|2]; ...}
 *          (if each resolution has a different chunking scheme)
 *
 **/

vector<int> MultiresFileReader::parseNumXchunks(const char* line)
{
    TRACE(line);

    vector<int> numXchunks(numResolutions(), 1);  // set defatult to 1
    vector<string> chunkSpecs = split(line, "#={}; \t\n");

    // if there is a single chunking specification, apply it to all resolutions
    if(chunkSpecs.size() == 2) {
        REQUIRE(chunkSpecs[0]=="numchunks","invalid LHS, line '%s'", line);
        vector<string> tokens = split(chunkSpecs[1], " ,[]\t\n");

        if(tokens.size() >= 3) 
            for(int i=0; i<numResolutions(); i++) 
                numXchunks[i] = toInt(tokens[0]);
        else 
            ERROR("Unable to get chunk info from line '%s'\n", line);
    }

    // if each resolution gets its own chunking scheme
    else if((int)chunkSpecs.size() == numResolutions()+1) {
        REQUIRE(chunkSpecs[0]=="numchunks","invalid LHS, line '%s'", line);

        for(int i=0; i<numResolutions(); i++) {
            vector<string> tokens = split(chunkSpecs[i+1], " #=,[]\t\n");

            if(tokens.size() >= 3)
                numXchunks[i] = toInt(tokens[0]);
        }
    }
    else {
        ERROR("In line '%s', number of chunking specifications, '%d', "
              "must match number of resolutions, '%d'\n", line,
              (int)chunkSpecs.size()-1, numResolutions());
    }

    return numXchunks;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/**
 *      @See parseNumXchunks().
 **/

vector<int> MultiresFileReader::parseNumYchunks(const char* line)
{
    TRACE(line);

    vector<int> numYchunks(numResolutions(), 1);  // set defatult to 1
    vector<string> chunkSpecs = split(line, "#={}; \t\n");

    // if there is a single chunking specification, apply it to all resolutions
    if(chunkSpecs.size() == 2) {
        REQUIRE(chunkSpecs[0]=="numchunks","invalid LHS, line '%s'", line);
        vector<string> tokens = split(chunkSpecs[1], " ,[]\t\n");

        if(tokens.size() >= 3)
            for(int i=0; i<numResolutions(); i++) 
                numYchunks[i] = toInt(tokens[1]);
        else 
            ERROR("Unable to get chunk info from line '%s'\n", line);
    }

    // if each resolution gets its own chunking scheme
    else if((int)chunkSpecs.size() == numResolutions()+1) {
        REQUIRE(chunkSpecs[0]=="numchunks","invalid LHS, line '%s'", line);

        for(int i=0; i<numResolutions(); i++) {
            vector<string> tokens = split(chunkSpecs[i+1], " #=,[]\t\n");

            if(tokens.size() >= 3)
                numYchunks[i] = toInt(tokens[1]);
        }
    }
    else {
        ERROR("In line '%s', number of chunking specifications, '%d', "
              "must match number of resolutions, '%d'\n", line,
              (int)chunkSpecs.size()-1, numResolutions());
    }

    return numYchunks;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/**
 *      @See parseNumXchunks().
 **/

vector<int> MultiresFileReader::parseNumZchunks(const char* line)
{
    TRACE(line);

    vector<int> numZchunks(numResolutions(), 1);  // set defatult to 1
    vector<string> chunkSpecs = split(line, "#={}; \t\n");

    // if there is a single chunking specification, apply it to all resolutions
    if(chunkSpecs.size() == 2) {
        REQUIRE(chunkSpecs[0]=="numchunks","invalid LHS, line '%s'", line);
        vector<string> tokens = split(chunkSpecs[1], " ,[]\t\n");

        if(tokens.size() >= 3)
            for(int i=0; i<numResolutions(); i++) 
                numZchunks[i] = toInt(tokens[2]);
        else 
            ERROR("Unable to get chunk info from line '%s'\n", line);
    }

    // if each resolution gets its own chunking scheme
    else if((int)chunkSpecs.size() == numResolutions()+1) {
        REQUIRE(chunkSpecs[0]=="numchunks","invalid LHS, line '%s'", line);

        for(int i=0; i<numResolutions(); i++) {
            vector<string> tokens = split(chunkSpecs[i+1], " #=,[]\t\n");

            if(tokens.size() >= 3)
                numZchunks[i] = toInt(tokens[2]);
        }
    }
    else {
        ERROR("In line '%s', number of chunking specifications, '%d', "
              "must match number of resolutions, '%d'\n", line,
              (int)chunkSpecs.size()-1, numResolutions());
    }

    return numZchunks;
}

/* ========================================================================= */
/**
 *  Syntax: 
 *
 *  # res0=[392,112,112]@1024
 *     ^         ^        ^-- fseek offset to start of data
 *     |         +-- size
 *     +-- resolution
 *
 *  Values are returned by reference in arguments @param resolution,
 *  @param width, @param height, @param depth, @param offset.
 **/

void MultiresFileReader::parseResolutionMetadata(const char* line,
                                 int& resolution,
                                 int& width,
                                 int& height,
                                 int& depth,
                                 long long& offset)
{
    TRACE(line);

    // defaults
    resolution  = 0;
    width       = 0;
    height      = 0;
    depth       = 0;
    offset      = 0;

    vector<string> tokens = split(line, " #=@,[]\t\n");

    if(tokens.size() >= 5) {
        resolution  = toInt(tokens[0][3]);
        width       = toInt(tokens[1]);
        height      = toInt(tokens[2]);
        depth       = toInt(tokens[3]);
        offset      = toLong(tokens[4]);
    }
    else {
       ERROR("Unable to get resolution from line '%s'\n", line);
    }
}

/* ========================================================================= */

void MultiresFileReader::checkFilePtr(FILE* ptr)
{
    TRACE(ptr);

    if(feof(ptr)) {
        ERROR("Unexpected EOF\n");
        throw IOerror();
    }

    if(ferror(ptr)) {
        ERROR("Unexpected I/O error");
        throw IOerror();
    }
}

/* ========================================================================= */

const char* MultiresFileReader::getNextLine(FILE* ptr)
{
    static char line[1024];

    checkFilePtr(ptr);

    char* res = fgets(line, sizeof line, ptr); (void) res;

    return line;
}

/* ========================================================================= */

MultiresFileReader::~MultiresFileReader()
{
    TRACE();
    DEBUG("Deleting memory for '%s'",mFilename.c_str());

    freeRawDataMemory();
}

/* ========================================================================= */

//  g++ -g -Wall -DTEST=main MultiresFileReader.C

int TEST(int argc, char** argv)
{
    //const char* file = "/local/data/mhd/jcd0004/test/multires/"
                       //"rr392x112x112/jcd0004.rr.000060.mrd";

    //const char* file = "/local/data/single-helium/metadata.varchunking.mrd";
    const char* file = "/local/data/mhd/thm-20070323-j13c/orig/foo/rr630x200x300/thm-20070323-j13c.rr.000060.mrd";

    MultiresFileReader mrf;
    mrf.parseFile(file);

    fprintf(stderr,"numResolutions=%d\n", mrf.numResolutions());
    fprintf(stderr,"timestep=%d\n", mrf.timeStepList()[0]);
    fprintf(stderr,"gridFilename=%s\n", mrf.gridFilename().c_str());
    fprintf(stderr,"variableName=%s\n", mrf.variableNameAt(0).c_str());
    fprintf(stderr,"fileVersion=%f\n", mrf.fileVersion());
    fprintf(stderr,"isScalar=%s\n", mrf.isScalar()?"true":"false");
    fprintf(stderr,"isVectorComponent=%s\n", 
                    mrf.isVectorComponent()?"true":"false");

    for(int i=0; i<mrf.numResolutions(); i++) {
        fprintf(stderr,"res %d: dataSize=[%d,%d,%d], chunking=[%d,%d,%d]\n", 
                    i, 
                    mrf.width(i),
                    mrf.height(i),
                    mrf.depth(i),
                    mrf.numXchunks(i),
                    mrf.numYchunks(i),
                    mrf.numZchunks(i));

        const float* data = mrf.rawData(mrf.variableNameAt(0), i, 0, 0);
        fprintf(stderr,"res %d: first 5 values:\n", i);
        for(int j=0; j<5; j++) 
            fprintf(stderr,"[%0.8f] ", data[j]);

        fprintf(stderr,"\n");
    }

    return 0;
}

/* ========================================================================= */
