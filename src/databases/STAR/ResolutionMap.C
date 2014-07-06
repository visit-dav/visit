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
//  File:        ResolutionMap.cpp                                           //
//  Programmer:  Andrew Foulks <rafoulks@cs.unh.edu>                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "ResolutionMap.h"

/* ========================================================================= */

ResolutionMap::ResolutionMap(int numchunks, int numresolutions)
{
    TRACE();

    mNumChunks = numchunks;
    mNumResolutions = numresolutions;

    mResolutionMap.resize(numchunks, 0);  // initialize each entry with a 0

    srand(time(NULL));
}

/* ========================================================================= */
/**
 *      For debugging, I'd like to be able to read in 
 *      a configuration from a simple file.  
 *
 *      The file format is a single integer on each line.  The 
 *      first line contains the resolution of the first 
 *      chunk (chunk 0), the second line is the resolution
 *      of the second chunk, and so forth.
 **/

void ResolutionMap::loadMapFromFile(const char* filename)
{
    TRACE(filename);
    NOTNULL(filename);
    DEBUG("filename = '%s'",filename);

    FILE* infile = openFile(filename, "r");
    int highestResolution = 0;

    if(infile==NULL) {
        ERROR("Unable to open file '%s' for read", filename);
    }
    else {
        while(true) {
            int resolution;
            int res = fscanf(infile, "%d", &resolution); (void) res;
            if(feof(infile))
                break;

            if(resolution > highestResolution)
                highestResolution = resolution;

            mResolutionMap.push_back(resolution);
        }
    }

    mNumChunks = mResolutionMap.size();
    mNumResolutions = highestResolution + 1;  // assuming we start at 0

    DEBUG("Got %d chunks, chunk[0]=>res %d",mNumChunks,mResolutionMap[0]);
}

/* ========================================================================= */
/**
 *      loads a random map for the given number of 
 *      chunks and resolutions, used for testing.
 **/

void ResolutionMap::loadRandomMap()
{
    TRACE();
    REQUIRE(mNumChunks>0,"mNumChunks=%d",mNumChunks);
    REQUIRE(mNumResolutions>0,"mNumResolutions=%d",mNumResolutions);

    mResolutionMap.resize(mNumChunks);

    for(int i=0; i<mNumChunks; i++) 
        mResolutionMap[i] = rand() % mNumResolutions;
}

/* ========================================================================= */
/**
 *      sets the resolution map to the values in the array passed in.
 **/

void ResolutionMap::setMap(int resolutions[])
{
    TRACE(resolution);
    REQUIRE(mNumChunks>0,"mNumChunks=%d",mNumChunks);
    REQUIRE(mNumResolutions>0,"mNumResolutions=%d",mNumResolutions);

    for(int i=0; i<mNumChunks; i++) {
        BOUNDS_CHECK(resolutions[i], 0, mNumResolutions);
        mResolutionMap[i] = resolutions[i];
    }
}

/* ========================================================================= */
/**
 *      loads a uniform resolution map by assigning each chunk
 *      the specified resolution.  
 *
 *      Used for testing.
 **/

void ResolutionMap::loadUniformResMap(int resolution)
{
    TRACE(resolution);
    REQUIRE(mNumChunks>0,"mNumChunks=%d",mNumChunks);
    REQUIRE(mNumResolutions>0,"mNumResolutions=%d",mNumResolutions);
    BOUNDS_CHECK(resolution, 0, mNumResolutions);

    for(int i=0; i<mNumChunks; i++) 
        mResolutionMap[i] = resolution;
}

/* ========================================================================= */


int ResolutionMap::resolutionForChunk(int chunkIndex)
{
    TRACE(chunkIndex);
    BOUNDS_CHECK(chunkIndex,0,(int)mResolutionMap.size());

    return mResolutionMap[chunkIndex];
}

/* ========================================================================= */

int ResolutionMap::numChunks()
{
    TRACE();

    return mNumChunks;
}

/* ========================================================================= */

int ResolutionMap::numResolutions()
{
    TRACE();

    return mNumResolutions;
}

/* ========================================================================= */

void ResolutionMap::outputToFile(FILE* outfile)
{
    NOTNULL(outfile);

    fprintf(outfile, "ADAPTIVE RES MAP:\n");
    fprintf(outfile, "numchunks=%d, numresolutions=%d\n",
            mNumChunks,mNumResolutions);
    
    for(int i=0; i<mNumChunks; i++) {
        fprintf(outfile, "chunk [%d] ==> resolution %d | ",i,mResolutionMap[i]);
        if(i%2==1)
            fprintf(outfile, "\n");
    }
}

/* ========================================================================= */

ResolutionMap::~ResolutionMap()
{
    TRACE();
}

/* ========================================================================= */
