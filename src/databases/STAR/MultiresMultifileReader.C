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
//  File:        MultiresMultifileReader.cpp                                 //
//  Programmer:  Andrew Foulks <rafoulks@cs.unh.edu>                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "MultiresMultifileReader.h"
#include "ConfigFileReader.h"
#include "MultiresFileReader.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

/* ========================================================================= */
/**
 *      Constructs an empty MultiresMultifileReader object.  To read in 
 *      a metadata file, @see parseFile().
 **/

MultiresMultifileReader::MultiresMultifileReader() 
{
    TRACE();

    mFilename = "";      
    mFileVersion = 0.0;   
    mFileReader = NULL;
}

/* ========================================================================= */
/**
 *      Constructs an empty MultiresMultifileReader object and reads
 *      the filename passed in, parsing  the metadata.
 **/
 
MultiresMultifileReader::MultiresMultifileReader(const char* filename) 
{
    TRACE(filename);

    mFilename = filename;
    mFileReader = NULL;

    mFileVersion = parseFileVersion(filename);
    parseFile(filename);

    mCache.resize(numVariables());
    for(unsigned i=0; i<mCache.size(); i++) 
        mCache[i].resize(numFiles(), NULL);

    DEBUG("filename='%s'",filename);
    DEBUG("file version=%f", mFileVersion);
}

/*=========================================================================*/
/**
 *      Returns the number of files for which we have data.
 *      This is distinguished from the number of simulation
 *      time steps.  For example, if the density directory has
 *      2 files
 *
 *          jcd0004.rr.000060.mrd  
 *          jcd0004.rr.000120.mrd
 *
 *      The number of time steps is 120, for which
 *      only 2 were saved, timestep 60 and timestep 120.
 *
 *      For this example the return would be 2.  To get
 *      the timesteps, @see timeStepList().
 **/
 
int MultiresMultifileReader::numFiles() const
{
    TRACE();
    NOTNULL(mFileReader);

    return mFileReader->findIntValue("numfiles");
}

/*=========================================================================*/
/**
 *      Returns the number of resolutions as specified
 *      in the metadata file.
 **/

int MultiresMultifileReader::numResolutions() const
{
    TRACE();
    NOTNULL(mFileReader);

    return mFileReader->findIntValue("numresolutions");
}

/*=========================================================================*/
/**
 *      Returns the canonical grid filename as specified
 *      in the metadata file.  The path to this metadata file
 *      is prepended to the front of the grid filename
 *      so that the full path filename is returned.
 *      Normally, the grid filename is specified as a 
 *      relative name, in which we assume that it is located
 *      in the same directory as this metadata file.  However
 *      the grid filename can also be specified as an absolute 
 *      path, in which case no prepending is done and the name
 *      is returned directly.
 **/

string MultiresMultifileReader::gridFilename() const
{
    TRACE();
    NOTNULL(mFileReader);

    string fullpath = "error in MultiresMultifileReader::gridFilename()";
    string gridfilename = mFileReader->findValue("gridfile");
    
    if(gridfilename[0] == '/') {           // absolute path name
        fullpath = gridfilename;
    }
    else {
        vector<string> pathAndFile = splitPathName(mFilename);
        REQUIRE(pathAndFile.size()>=2, "internal error, fullpath '%s' "
                "doesn't split into separate path and filename, i'm confused",
                mFilename.c_str());
    
        string path = pathAndFile[0];
        string file = pathAndFile[1];
            
        fullpath = path + "/" + gridfilename;
    }       
        
    return fullpath;
}

/*=========================================================================*/
/**
 *      Returns all of the simulation timesteps as an array.
 *      Time steps are part of the filename, and represent 
 *      the steps saved when the simulation was run.
 *      For example, if we have 2 files:
 *
 *          jcd0004.rr.000060.mrd  
 *          jcd0004.rr.000120.mrd
 *
 *      At file index 0, the simulation timestep is 60, and
 *      at file index 1, the simulation timestep is 120.
 *
 *      This example would return the array '[60, 120]'
 *      as a C++ vector.
 **/

vector<int> MultiresMultifileReader::timeStepList() const
{
    TRACE();
    NOTNULL(mFileReader);

    vector<int> timesteps;

    int first = mFileReader->findIntValue("firsttimestep");
    int increment = mFileReader->findIntValue("timestepincrement");
    int numfiles = numFiles();

    int nexttime = first;
    for(int i=0; i<numfiles; i++) {
        timesteps.push_back(nexttime);

        nexttime += increment;
    }

    return timesteps;
}

/*=========================================================================*/
/**
 *      Returns the number of variables in the simulation
 *      as specified in the metadata file.  Typically about
 *      11, such as pressure ('pp'), or magnetic field ('{bx,by,bz}').
 **/

int MultiresMultifileReader::numVariables() const
{
    TRACE();

    return mVariables.size();
}

/*=========================================================================*/
/**
 *      Returns the name of the variable.  The name in the metadata
 *      file is abbreviated, for example 'rr', or 'pp'.  @See
 *      longScalarNameFor() to get the expanded name.
 *
 *      @param index -- the index into the internal array 
 *      of available scalar and vector names.  For example,
 *      if the names in the metadata file are spec'd as 
 *      ['rr', 'pp'], then the name at index 0 is 
 *      'rr' ('density').
 **/

string MultiresMultifileReader::variableNameAt(int index) const
{
    TRACE(index);
    NOTNULL(mFileReader);
    BOUNDS_CHECK(index, 0, mVariables.size());

    return mVariables[index].name;
}

/*=========================================================================*/
/**
 *      Returns true if the names variable is a scalar.
 **/

bool MultiresMultifileReader::isScalar(const string& name) const
{
    TRACE(name);

    int index = indexOfVariableName(name);

    if(index >=0 )
        return mVariables[index].rank == 0;

    return false;
}

/*=========================================================================*/
/**
 *      If the variable name is a scalar that is part 
 *      of a vector, returns the component name, 
 *      otherwise, returns character 0 ('\0').
 *
 *      Component names are either 'x', 'y', or 'z'.
 **/

char MultiresMultifileReader::isVectorComponent(const string& name) const
{
    TRACE(name);

    int index = indexOfVariableName(name);

    if(index >=0 ) {
        if(mVariables[index].component != '\0') {
            REQUIRE(mVariables[index].rank==0,
                    "rank of '%s' is '%d', but component = '%c'",
                     mVariables[index].name.c_str(),
                     mVariables[index].rank,
                     mVariables[index].component);

            return mVariables[index].component;
        }
    }

    return '\0';
}

/*=========================================================================*/
/**
 *      Returns true if the variable name is that of a vector.
 **/

bool MultiresMultifileReader::isVector(const string& name) const
{
    TRACE(name);

    int index = indexOfVariableName(name);

    if(index >=0 )
        return mVariables[index].rank == 1;

    return false;
}

/*=========================================================================*/
/**
 *      Returns true if the variable name is that of a tensor.
 **/

bool MultiresMultifileReader::isTensor(const string& name) const
{ 
    TRACE(name);

    int index = indexOfVariableName(name);

    if(index >=0 )
        return mVariables[index].rank == 2;

    return false;
}

/*=========================================================================*/
/**
 *      Returns the number of vectors that are composed of 3
 *      separate scalars.  For example, the magnetic field
 *      components may be stored in 3 separate files, one
 *      for 'bx', one for 'by', and one for 'bz'.  If this
 *      is the case, 'bx,'by', and 'bz' will be flagged
 *      as scalar variables (@see isScalar() will return true),
 *      but as part of an expression define a vector.
 *
 *      The vector expression is defined by curly brace
 *      syntax:
 *          {bx, by, bz}
 *
 *      which is compatible with VisIt expressions.
 **/

int MultiresMultifileReader::numVectorExpressions() const
{
    TRACE();

    return mVectorExpressions.size();
}

/*=========================================================================*/
/**
 *      Returns the vector expression at the index, using
 *      curly brace format compatible with VisIt (@see
 *      numVectorExpressions()).  For example given 2
 *      vector expressions, one for magnetic field and
 *      one for bulk flow velocity, 
 *
 *          vectorExpressionAt(0) => "{bx, by, bz}"
 *          vectorExpressionAt(1) => "{vx, vy, vz}"
 **/

string MultiresMultifileReader::vectorExpressionAt(int index) const
{
    TRACE();
    BOUNDS_CHECK(index, 0, mVectorExpressions.size());

    return mVectorExpressions[index];
}

/*=========================================================================*/
/**
 *      Returns the width of the data at the specified
 *      resolution.  
 *
 *      The metadata file contains the resolution at the 
 *      highest level.  Each resolution lower is a factor 
 *      of 2 smaller, truncated for odd values.  Example:
 *
 *          if high resolution 0 = [392,112,112]
 *          then
 *              resolution 1 = [196,56,56]
 *              resolution 2 = [98,28,28]
 *              resolution 3 = [49,14,14]
 *              resolution 4 = [24,7,7]    => width was truncated
 *              resolution 5 = [12,3,3]    => height, depth were truncated
 **/

int MultiresMultifileReader::width(int resolution) const
{
    TRACE();
    NOTNULL(mFileReader);

    string sizeinfo = mFileReader->findValue("hires_size");
    vector<string> sizes = split(sizeinfo, " \t\n[],'");

    int width = toInt(sizes[0]);

    for(int i=0; i<resolution; i++) 
        width /= 2;

    return width;
}

/*=========================================================================*/
/**
 *      Returns the height of the data at the specified
 *      resolution.  
 *
 *      The metadata file contains the resolution at the 
 *      highest level.  Each resolution lower is a factor 
 *      of 2 smaller, truncated for odd values.  Example:
 *
 *          if high resolution 0 = [392,112,112]
 *          then
 *              resolution 1 = [196,56,56]
 *              resolution 2 = [98,28,28]
 *              resolution 3 = [49,14,14]
 *              resolution 4 = [24,7,7]    => width was truncated
 *              resolution 5 = [12,3,3]    => height, depth were truncated
 **/

int MultiresMultifileReader::height(int resolution) const
{
    TRACE();
    NOTNULL(mFileReader);

    string sizeinfo = mFileReader->findValue("hires_size");
    vector<string> sizes = split(sizeinfo, " \t\n[],'");

    int height = toInt(sizes[1]);

    for(int i=0; i<resolution; i++) 
        height /= 2;

    return height;
}

/*=========================================================================*/
/**
 *      Returns the depth of the data at the specified
 *      resolution.  
 *
 *      The metadata file contains the resolution at the 
 *      highest level.  Each resolution lower is a factor 
 *      of 2 smaller, truncated for odd values.  Example:
 *
 *          if high resolution 0 = [392,112,112]
 *          then
 *              resolution 1 = [196,56,56]
 *              resolution 2 = [98,28,28]
 *              resolution 3 = [49,14,14]
 *              resolution 4 = [24,7,7]    => width was truncated
 *              resolution 5 = [12,3,3]    => height, depth were truncated
 **/

int MultiresMultifileReader::depth(int resolution) const
{
    TRACE();
    NOTNULL(mFileReader);

    string sizeinfo = mFileReader->findValue("hires_size");
    vector<string> sizes = split(sizeinfo, " \t\n[],'");

    int depth = toInt(sizes[2]);

    for(int i=0; i<resolution; i++) 
        depth /= 2;

    return depth;
}

/*=========================================================================*/
/**
 *      Returns the number of chunks along the X (width) axis.
 *
 *      For a MultiresMultifileReader, the chunking scheme needs to 
 *      be the same across all resolutions, so the parameter
 *      @param resolution is ignored.  For other subclasses the
 *      chunking scheme may vary depending on resolution.
 **/

int MultiresMultifileReader::numXchunks(int resolution) const
{
    TRACE();
    NOTNULL(mFileReader);

    string chunkinfo = mFileReader->findValue("numchunks");

    vector<string> numChunks = split(chunkinfo, " \t\n[],'");

    return toInt(numChunks[0]);
}

/*=========================================================================*/
/**
 *      Returns the number of chunks along the Y (height) axis.
 *
 *      For a MultiresMultifileReader, the chunking scheme needs to 
 *      be the same across all resolutions, so the parameter
 *      @param resolution is ignored.  For other subclasses the
 *      chunking scheme may vary depending on resolution.
 **/

int MultiresMultifileReader::numYchunks(int resolution) const
{
    TRACE();
    NOTNULL(mFileReader);

    string chunkinfo = mFileReader->findValue("numchunks");

    vector<string> numChunks = split(chunkinfo, " \t\n[],'");

    return toInt(numChunks[1]);
}

/*=========================================================================*/
/**
 *      Returns the number of chunks along the Z (depth) axis.
 *
 *      For a MultiresMultifileReader, the chunking scheme needs to 
 *      be the same across all resolutions, so the parameter
 *      @param resolution is ignored.  For other subclasses the
 *      chunking scheme may vary depending on resolution.
 **/

int MultiresMultifileReader::numZchunks(int resolution) const
{
    TRACE();
    NOTNULL(mFileReader);

    string chunkinfo = mFileReader->findValue("numchunks");

    vector<string> numChunks = split(chunkinfo, " \t\n[],'");

    return toInt(numChunks[2]);
}

/*=========================================================================*/
/**
 *      Returns a pointer to the data at the specified resolution.
 *      This class is still the 'owner' of the memory -- don't
 *      delete it, call @see freeRawDataMemory() when done.
 *      
 *      @param resolution -- integer resolution, 0 is the highest
 *
 *      @param variableName -- the name of the scalar, vector
 *             or tensor to get the data for.  
 *
 *      @param fileIndex -- which file to grab the data for
 **/

const float* MultiresMultifileReader::rawData(const string& variableName,
                                        int resolution, 
                                        int fileIndex,
                                        int chunkIndex)
{
    TRACE(variableName, resolution, fileIndex, chunkIndex);
    BOUNDS_CHECK(fileIndex, 0, numFiles());
    BOUNDS_CHECK(resolution, 0, numResolutions());
    DEBUG("resolution=%d, varname=%s, fileIndex=%d, chunkIndex=%d",
            resolution, variableName.c_str(), fileIndex, chunkIndex);

    MultiresFileReader* mf = findInCache(variableName, fileIndex);

    if(mf == NULL) {
        string filename = getFilename(variableName, fileIndex);
        mf = new MultiresFileReader(filename.c_str());
        addToCache(mf);
    }

    NOTNULL(mf);

    return mf->rawData(resolution, chunkIndex);
}

/*=========================================================================*/
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

void MultiresMultifileReader::freeRawDataMemory(const string& variableName,
                                          int fileIndex)
{
    TRACE(variableName, fileIndex);

    ERROR("DON'T CALL THIS FUNCTION RIGHT NOW");

    MultiresFileReader* mf = findInCache(variableName, fileIndex);

    if(mf != NULL) {
        mf->freeRawDataMemory();
    }
}


/* ========================================================================= */
/**
 *      Opens and parses the metadata file, setting class
 *      member variables and data structures appropriately.
 **/

void MultiresMultifileReader::parseFile(const char* filename)
{
    TRACE(filename);
    DEBUG("MultiresMultifileReader file is '%s'", filename);

    mFilename = filename;
    mFileVersion = parseFileVersion(filename);

    try {
        mFileReader = new ConfigFileReader();
        bool success = mFileReader->parseFile(filename);

        if(!success)
            ERROR("Unable to read metadata file!\n");
    }
    catch(ConfigFileReader::ParseError& e) {
        ERROR("Parse Error.  Something bad will probably happen.\n");
    }
    catch(IOerror& e) {
        ERROR("I/O Error.  Something bad will probably happen.\n");
    }

    string scalars = mFileReader->findValue("scalars");
    vector<string> scalarNames = split(scalars, " \t\n,[]'");

    string vectors = mFileReader->findValue("vectors");
    vector<string> vectorNames = split(vectors, " \t\n,[]'");

    string tensors = mFileReader->findValue("tensors");
    vector<string> tensorNames = split(tensors, " \t\n,[]'");

    string vectorComponents = mFileReader->findValue("vectorcomponents");
    vector<string> vectorComponentNames = split(vectorComponents," \t\n,{};[]");
    mVectorExpressions = split(vectorComponents, " \t\n[];");

    for(unsigned i=0; i<scalarNames.size(); i++) {
        mVariables.push_back(Variable(scalarNames[i], 0, '\0'));
    }

    for(unsigned i=0; i<vectorComponentNames.size(); i++) {
        int lastIndex = vectorComponentNames[i].length()-1;
        char component = vectorComponentNames[i][lastIndex];

        REQUIRE((component=='x'||component=='y'||component=='z'),
                "component is '%c', needs to be 'x', 'y', or 'z'",component);

        mVariables.push_back(Variable(vectorComponentNames[i], 0, component));
    }

    for(unsigned i=0; i<vectorNames.size(); i++) {
        mVariables.push_back(Variable(vectorNames[i], 1, '\0'));
    }

    for(unsigned i=0; i<tensorNames.size(); i++) {
        mVariables.push_back(Variable(tensorNames[i], 2, '\0'));
    }
}

/*=========================================================================*/
/* PRIVATE, helper functions */
/*=========================================================================*/
/**
 *      Returns the index of the variable name specified.
 *      Used to look up variables in the table by name.
 *
 *      Returns -1 if the variable name was not found.
 **/

int MultiresMultifileReader::indexOfVariableName(const string& name) const
{
    TRACE(name);

    for(unsigned i=0; i<mVariables.size(); i++) {
        if(name == mVariables[i].name)
            return i;
    }

    return -1;
}

/*=========================================================================*/
/**
 *      Returns the filename for the given variable 
 *      at the given timestep.  The filename returned will
 *      look something liks this:
 *
 *          ./rr392x112x112/jcd0004.rr.000060.mrd
 *            ^ ^           ^          ^      ^
 *            | |           |          |      +--extension
 *            | |           |          +-- timestep
 *            | |           +--prefix
 *            | +--partialdirname
 *            +--scalarname
 *
 *      The filename is inferred, then built, from the variable 
 *      name, the size of the data at the highest resolution,
 *      and the prefix and extension, which are both spec'd
 *      in the metadata file.  
 *
 *      @param varName   -- name of the variable (scalar or vector)
 *      @param fileIndex -- index of the file, an integer between
 *                          0 and numFiles()-1.
 **/

string MultiresMultifileReader::
getFilename(const string& varName, int fileIndex)
{
    NOTNULL(mFileReader);
    
    char filename[1024];  // return value
    string validScalarNames = mFileReader->findValue("scalars");
    string validVectorComponents = mFileReader->findValue("vectorcomponents");
    string validVectorNames = mFileReader->findValue("vectors");
    string validTensorNames = mFileReader->findValue("tensors");

    string validNames = validScalarNames + " " +
                        validVectorComponents + " " +
                        validVectorNames + " " +
                        validTensorNames;

    if(validNames.find(varName) == string::npos) {
        ERROR("Variable name '%s' is not valid, possible choices in '%s'",
               varName.c_str(), validNames.c_str());
    }

    vector<int> timeSteps = timeStepList();
    BOUNDS_CHECK(fileIndex, 0, timeSteps.size());

    int timestep = timeSteps[fileIndex];

    // mFilename is the full path name of this metadata file
    vector<string> pathPlusFilename = splitPathName(mFilename);
    string mrmPathname = pathPlusFilename[0];
    string mrmFilename = pathPlusFilename[1];

    const char* prefix = mFileReader->findValue("fileprefix");
    const char* extension = mFileReader->findValue("extension");

    sprintf(filename, "%s/%s%dx%dx%d/%s.%s.%06d.%s",
                        mrmPathname.c_str(), varName.c_str(), 
                        width(0),height(0),depth(0), prefix, varName.c_str(), 
                        timestep, extension);

    DEBUG("return filename '%s'", filename);

    return string(filename);
}

/*=========================================================================*/
/**
 *      The cache of instances of MultiresFileReader objects is a
 *      2D array (C++ vector of vector).  Indexing is
 *      first dereference variableIndex, second dereference
 *      the fileIndex:
 *
 *          mCache[variableIndex][fileIndex]
 *
 *      If the entry is null, then no file has been loaded 
 *      yet for that timestep and variable.  Returns null
 *      if not found.
 **/

MultiresFileReader* MultiresMultifileReader::findInCache(const string& variableName,
                                             int fileIndex)
{
    TRACE(resolution, variableName, fileIndex);
    BOUNDS_CHECK(fileIndex, 0, numFiles());
    BOUNDS_CHECK(indexOfVariableName(variableName), 0, numVariables());

    int varIndex = indexOfVariableName(variableName);
    MultiresFileReader* mf = mCache[varIndex][fileIndex];

    if(mf != NULL) {
        REQUIRE((mf->filename()==getFilename(variableName,fileIndex)),
                "Cache corruption.  filename in cache='%s'. getFilename='%s'",
                 mf->filename(),getFilename(variableName,fileIndex).c_str());
    }

    return mf;
}

/*=========================================================================*/
/**
 *      Adds the instance of MultiresFileReader to the back of the
 *      queue.  The the individual files are kept in a list so
 *      that the data is not loaded into memory, nor the file
 *      parsed, more than once.
 **/

void MultiresMultifileReader::addToCache(MultiresFileReader* mf)
{
    TRACE(mf);
    NOTNULL(mf);

    string name = mf->variableNameAt(0);
    int varIndex = indexOfVariableName(name);

    int time = mf->timeStepList()[0];
    vector<int> timesteps = timeStepList();

    int fileIndex = -1;
    for(unsigned int i=0; i<timesteps.size(); i++) {
        if(timesteps[i] == time) {
            fileIndex = (int)i;
            break;
        }
    }
        
    REQUIRE(fileIndex>=0 && varIndex>=0,
            "fileIndex is -1, name='%s', varIndex='%d', time='%d'",
             name.c_str(), varIndex, time);

    mCache[varIndex][fileIndex] = mf;
}

/*=========================================================================*/
/**
 *  Parses the file version number which is in the comment on
 *  the first line of the file.
 *
 *  Syntax: 
 *
 *  # MRM METADATA file version 1.0
 *
 **/

float MultiresMultifileReader::parseFileVersion(const char* filename)
{
    TRACE(line);
    
    FILE* infile = NULL;
    char line[1024] = {0};
    float version = 1.0;

    infile = openFile(filename);

    if(infile == NULL) {
        ERROR("Unable to open file '%s'\n", filename);
        throw IOerror();
    }
    if(feof(infile)) {
        ERROR("Unexpected EOF\n");
        throw IOerror();
    }
    if(ferror(infile)) {
        ERROR("Unexpected I/O error");
        throw IOerror();
    }

    fgets(line, sizeof line, infile);

    vector<string> tokens = split(line, " #\t\n");

    if(tokens.size() >= 5) {
        version = toFloat(tokens[4]);
    }
    else {
        ERROR("Unable to get version from line '%s'\n", line);
    }

    fclose(infile);

    return version;
}

/*=========================================================================*/

MultiresMultifileReader::~MultiresMultifileReader()
{
    TRACE();
    DEBUG("Deleting memory for '%s'",mFilename.c_str());

    delete mFileReader;
    for(unsigned i=0; i<mCache.size(); i++) {
        for(unsigned j=0; j<mCache[i].size(); j++) {
            delete mCache[i][j];
            mCache[i][j] = NULL;
        }
    }
}

/* ========================================================================= */

//  g++ -g -Wall -DTEST=main MultiresMultifileReader.C

#ifdef TEST
#include "ConfigFileReader.C"

#undef TEST
#include "MultiresFileReader.C"

int main(int argc, char** argv)
{
    const char* file = "/local/data/mhd/jcd0004/test/multires/jcd0004.mrm";

    MultiresMultifileReader mrm;
    mrm.parseFile(file);

    fprintf(stderr,"filename=%s\n", mrm.filename());
    fprintf(stderr,"fileVersion=%f\n", mrm.fileVersion());
    fprintf(stderr,"numFiles=%d\n", mrm.numFiles());
    fprintf(stderr,"numResolutions=%d\n", mrm.numResolutions());
    fprintf(stderr,"gridFilename=%s\n", mrm.gridFilename().c_str());
    fprintf(stderr,"numVariables=%d\n", mrm.numVariables());
    fprintf(stderr,"numChunks=[%d,%d,%d]\n", 
                        mrm.numXchunks(),
                        mrm.numYchunks(),
                        mrm.numZchunks());

    for(int i=0; i<mrm.numVariables(); i++) {
        string name = mrm.variableNameAt(i);
        fprintf(stderr,"Variable [%d]='%s'\n",i,name.c_str());
        fprintf(stderr,"  isScalar='%s'\n",mrm.isScalar(name)?"true":"false");
        fprintf(stderr,"  isVector='%s'\n",mrm.isVector(name)?"true":"false");
        fprintf(stderr,"  isTensor='%s'\n",mrm.isTensor(name)?"true":"false");
        fprintf(stderr,"  comp='%c'\n",mrm.isVectorComponent(name));
    }

    for(int i=0; i<mrm.numVectorExpressions(); i++) {
        fprintf(stderr,"Expression [%d]='%s'\n",i,
                mrm.vectorExpressionAt(i).c_str());
    }

    for(int i=0; i<mrm.numResolutions(); i++) {
        fprintf(stderr, "resolution[%d]: size=[%d,%d,%d]\n", i,
                mrm.width(i), mrm.height(i), mrm.depth(i));
   }

    return 0;
}

#endif // TEST

/* ========================================================================= */
