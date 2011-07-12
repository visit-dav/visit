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
//  File:        MultiresGrid.cpp                                            //
//  Programmer:  Andrew Foulks <rafoulks@cs.unh.edu>                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#define DEBUG_TO_STDERR

#include <cmath>

#include <ImproperUseException.h>
#include "MultiresGrid.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

// class static global list of all grids, one for each resolution
vector<MultiresGrid*> MultiresGrid::mGrids;

/* ========================================================================= */
/**
 *      Constructs an empty grid with no data.
 **/

MultiresGrid::MultiresGrid()
{
    TRACE();

    mFilename = "";
    mResolution = 0;
    mWidth = mHeight = mDepth = 0;
    mXcoords = mYcoords = mZcoords = NULL;
}

/* ========================================================================= */
/**
 *      Constructs a grid object by reading the grid 
 *      information from the specified file.
 **/

MultiresGrid::MultiresGrid(const char* filename)
{
    TRACE(filename);
    NOTNULL(filename);

    mResolution = 0;

    mWidth = mHeight = mDepth = 0;
    mXcoords = mYcoords = mZcoords = NULL;
    mFilename = filename;

    parseGridFile(filename);
}

/* ========================================================================= */
/**
 *      Constructs a default grid based on the size given whose 
 *      geometrical locations are the topological locations 
 *      (indexes) of the data.
 **/

MultiresGrid::MultiresGrid(int width, int height, int depth)
{
    mFilename = "";
    mResolution = 0;

    mWidth = width;
    mHeight = height;
    mDepth = depth;

    float* xcoords = new float[width];
    float* ycoords = new float[height];
    float* zcoords = new float[depth];

    for(int i=0; i<width; i++)
        xcoords[i] = (float)i;

    for(int i=0; i<height; i++)
        ycoords[i] = (float)i;

    for(int i=0; i<depth; i++)
        zcoords[i] = (float)i;
    
    mXcoords = xcoords;
    mYcoords = ycoords;
    mZcoords = zcoords;
}

/* ========================================================================= */
/**
 *      Constructs a grid with the size given and data points 
 *      geometrical locations specified.
 *
 *      This method does not copy the data in arguments x/y/zcoords,
 *      only copies the pointers.  This means that this object 
 *      takes control of the memory behind x/y/zcoords, and will
 *      free it when the class dies.
 **/

MultiresGrid::MultiresGrid(int width, 
                                 int height, 
                                 int depth,
                                 const float* xcoords, 
                                 const float* ycoords, 
                                 const float* zcoords,
                                 int resolution)
{
    NOTNULL(xcoords);
    NOTNULL(ycoords);
    NOTNULL(zcoords);

    mFilename = "";
    mResolution = resolution;

    mWidth = width;
    mHeight = height;
    mDepth = depth;

    mXcoords = xcoords;
    mYcoords = ycoords;
    mZcoords = zcoords;
}

/* ========================================================================= */
/**
 *      Loads the coordinates locations for structured grid data sets
 *      @See x/y/zLocation() to get the coordinates of the data values.
 *
 *      The file format is expected to be ascii.  To be consistent with
 *      VTK, the first 3 lines should look like
 *
 *      DATASET MULTIRES_GRID
 *      DIMENSIONS <width> <height> <depth>
 *      X_COORDINATES <width> float
 *      ...
 *      x values listed, space separated
 *      ...
 *      Y_COORDINATES <height> float
 *      ...
 *      y values listed, space separated
 *      ...
 *      Z_COORDINATES <depth> float
 *      ...
 *      z values listed, space separated
 *      ...
 **/

bool MultiresGrid::parseGridFile(const char* filename)
{
    TRACE(filename);
    DEBUG("reading from file '%s'",filename);
    FILE* infile = NULL;

    if(filename != NULL)
        infile = openFile(filename, "r");
    
    char junk[1024];
    int numXvals, numYvals, numZvals;
    float* xc = NULL, *yc = NULL, *zc = NULL; // x|y|z coordinates

    if(infile != NULL) {
        // HACK -- need to make more robust, 
        //         skip to DIMENSIONS, then X_COORDINATES, etc.
        // Format:
        //     DATASET MULTIRES_GRID
        //     DIMENSIONS 392 112 112
        //     X_COORDINATES 392 float
        //     ...

        fscanf(infile, "%s %s %s", junk, junk, junk);  // skip 3 tokens
        fscanf(infile, "%d %d %d", &mWidth, &mHeight, &mDepth);
        fscanf(infile, "%s %d %s", junk, &numXvals, junk); 

        xc = new float[numXvals];

        // read in the x coords
        for(int i=0; i<numXvals; i++)
            fscanf(infile, "%f", &xc[i]);

        fscanf(infile, "%s %d %s", junk, &numYvals, junk); 

        yc = new float[numYvals];

        // read in the y coords
        for(int i=0; i<numYvals; i++)
            fscanf(infile, "%f", &yc[i]);

        fscanf(infile, "%s %d %s", junk, &numZvals, junk); 

        zc = new float[numZvals];

        // read in the z coords
        for(int i=0; i<numZvals; i++)
            fscanf(infile, "%f", &zc[i]);

        fclose(infile);

        // {{{ sanity check
#ifdef DEBUG_ON
        REQUIRE(numXvals>=mWidth&&numYvals>=mHeight && numZvals>=mDepth,
                "Something wrong with the grid file: size=[%d,%d,%d], "
                 "but numValues = [%d,%d,%d]",mWidth,mHeight,mDepth,
                 numXvals,numYvals,numZvals);
#endif
        // }}} end sanity check
        
        DEBUG("from grid file: size=[%d,%d,%d]",mWidth,mHeight,mDepth);
        DEBUG("array sizes: [%d,%d,%d]",numXvals,numYvals,numZvals);
        DEBUG("xc[0]=%f, [%d]=%f",xc[0],mWidth-1,xc[mWidth-1]);
    }
    else {
        ERROR("unable to open grid file '%s'", filename);
    }

    mXcoords = xc;
    mYcoords = yc;
    mZcoords = zc;

    return infile;
}

/* ========================================================================= */
/**
 *      Converts the topological row location to its coordinate 
 *      location in the geometry.  
 *
 *      @param col      column of a grid point in the topology.
 *      @return         Geometric location of the point.
 *
 **/

float MultiresGrid::xLocation(int col) const
{
    TRACE(col);

    float x = (float)col;      // default return values

    if(mXcoords != NULL) {
        if(col >= 0 and col < mWidth)
            x = mXcoords[col];
        else
            WARNING("xLocation: row=%d out of bounds=(0...%d)",col,mWidth-1);
    }

    return x;
}

/* ========================================================================= */
/**
 *      Converts the topological row location to its coordinate location
 *      in the geometry.  
 *
 *      @param row      row of a grid point in the topology.
 *      @return         Geometric location of the point.
 **/

float MultiresGrid::yLocation(int row) const
{
    TRACE(row);
    float y = (float)row;      // default return values

    if(mYcoords != NULL) {
        if(row >= 0 and row < mHeight)
            y = mYcoords[row];
        else
            WARNING("yLocation: col=%d out of bounds=(0...%d)",row,mHeight-1);
    }

    return y;
}

/* ========================================================================= */
/**
 *      Converts the topological slice location to its coordinate location
 *      in the geometry.  
 *
 *      @param slice      depth location of a grid point in the topology.
 *      @return           Geometric location of the point.
 **/

float MultiresGrid::zLocation(int slice) const
{
    TRACE(slice);
    float z = (float)slice;      // default return values

    if(mZcoords != NULL) {
        if(slice >= 0 and slice < mDepth)
            z = mZcoords[slice];
        else
            WARNING("geometryZ: slic=%d out of bounds=(0...%d)",slice,mDepth-1);
    }

    return z;
}

/* ========================================================================= */
/**
 *      Returns the distance between the 'left most' width
 *      location (index 0) and the 'right most' location
 *      along the width axis.
 **/

float MultiresGrid::geometryWidth() const
{
    TRACE();

    return mXcoords[mWidth-1] - mXcoords[0];
}

/* ========================================================================= */
/**
 *      Returns the distance between the 'left most' width
 *      location (index 0) and the 'right most' location.
 *      along the height axis.
 **/

float MultiresGrid::geometryHeight() const
{
    TRACE();

    return mYcoords[mHeight-1] - mYcoords[0];
}

/* ========================================================================= */
/**
 *      Returns the distance between the 'left most' width
 *      location (index 0) and the 'right most' location.
 *      along the depth axis.
 **/

float MultiresGrid::geometryDepth() const
{
    TRACE();

    return mZcoords[mDepth-1] - mZcoords[0];
}

/* ========================================================================= */
/**
 *      Returns the width of the topology.  This should be the same as
 *      the width of the data set that this grid is associated with!
 **/

int MultiresGrid::width() const
{
    TRACE();

    return mWidth;
}

/* ========================================================================= */
/**
 *      Returns the height of the topology.  This should be the same as
 *      the height of the data set that this grid is associated with!
 **/

int MultiresGrid::height() const
{
    TRACE();

    return mHeight;
}

/* ========================================================================= */
/**
 *      Returns the depth of the topology.  This should be the same as
 *      the depth of the data set that this grid is associated with!
 **/

int MultiresGrid::depth() const
{
    TRACE();

    return mDepth;
}

/* ========================================================================= */
/**
 *      Returns the filename used to read in the grid info.
 **/

const char* MultiresGrid::filename() const
{
    TRACE();

    return mFilename.c_str();
}

/* ========================================================================= */
/**
 *      Returns the grid location values for the lower resolution
 *      specified.  
 *
 *      The resolution is an integer parameter representing
 *      the level in a multiresolution hierarchy.  Resolution
 *      0 is the original resolution.  Resolution 1 is the
 *      first lower resolution, etc.
 **/

MultiresGrid* MultiresGrid::atResolution(int resolution) const
{
    TRACE(resolution);
    BOUNDS_CHECK(resolution,0,mGrids.size());

    if(resolution<0||(unsigned)resolution>mGrids.size())
    {
        EXCEPTION0(ImproperUseException);
#if 0
        exit(fprintf(stderr,"ERROR: resolution=%d, BOUNDS_CHECK didn't work\n",
                    resolution));
#endif
    }

    return mGrids[resolution];
}

/* ========================================================================= */

// FIXME -- these should be a separate class, that stores how many chunks
//          per axis so that it can figure out which chunk corresponds
//          to which set of coordinates

const float* MultiresGrid::xcoordsChunk(int whichChunk,
                                        int numXChunks,
                                        int numYChunks,
                                        int numZChunks)
const
{
    REQUIRE(numXChunks!=0,"Internal Error, whichChunk=%d, numXChunks=0,  "
             "will divide by 0\n", whichChunk);
    REQUIRE((mWidth/numXChunks)==((int)((float)mWidth/numXChunks)),
           "Something bad will happen.  Does not divide evenly.  "
            "chunkwidth=%d, mWidth=%d, numXChunks=%d, chunkwidth (as "
            "float)=%0.2f", (mWidth/numXChunks),mWidth,numXChunks,
            (float)mWidth/(float)numXChunks);

    DEBUG("whichChunk=%d, mWidth=%d, numChunks=[%d|%d|%d]",
            whichChunk, mWidth, numXChunks, numYChunks, numZChunks);

    int chunkwidth = mWidth / numXChunks;

    int index = whichChunk % numXChunks * chunkwidth;

    const float* ptr = &mXcoords[index];

    DEBUG("chunkwidth=%d, index=%d", chunkwidth, index);
    DEBUG("ptr[0..3]=[%0.2f,%0.2f,%0.2f,%0.2f}", ptr[0],ptr[1],ptr[2],ptr[3]);

    return ptr;
}

/* ========================================================================= */

const float* MultiresGrid::ycoordsChunk(int whichChunk,
                                           int numXChunks,
                                           int numYChunks,
                                           int numZChunks)
const
{
    REQUIRE(numXChunks!=0&&numYChunks!=0,"Internal Error, whichChunk=%d, "
                "numXChunks=%d, numYChunks=%d,  will divide by 0\n",
                whichChunk, numXChunks, numYChunks);
    REQUIRE((mHeight/numYChunks)==((int)((float)mHeight/numYChunks)),
            "Something bad will happen.  Does not divide evenly.  "
            "chunkheight=%d, mHeight=%d, numYChunks=%d, chunkwidth (as "
            "float)=%0.2f", (mHeight/numYChunks),mHeight,numYChunks,
            (float)mHeight/(float)numYChunks);

    int chunkheight = mHeight / numYChunks;

    int index = ((whichChunk/numXChunks)%numYChunks) * chunkheight;

    const float* ptr = &mYcoords[index];

    return ptr;
}

/* ========================================================================= */

const float* MultiresGrid::zcoordsChunk(int whichChunk,
                                           int numXChunks,
                                           int numYChunks,
                                           int numZChunks)
const
{
    REQUIRE(numXChunks!=0&&numYChunks!=0&&numZChunks!=0,
            "Internal Error, whichChunk=%d, numXYZChunks=[%d|%d|%d], "
            " will divide by 0\n",numXChunks,numYChunks,numZChunks,whichChunk);
    REQUIRE((mDepth/numZChunks)==((int)((float)mDepth/numZChunks)),
            "Something bad will happen.  Does not divide evenly.  "
            "chunkdepth=%d, mDepth=%d, numZChunks=%d, chunkwidth (as "
            "float)=%0.2f", (mDepth/numZChunks),mDepth,numZChunks,
            (float)mDepth/(float)numZChunks);

    int chunkdepth = mDepth / numZChunks;

    int index = ((whichChunk/(numXChunks*numYChunks))%numZChunks)*chunkdepth;

    const float* ptr = &mZcoords[index];

    return ptr;
}

/* ========================================================================= */
/**
 *      If there is no grid values for that resolution,
 *      they are generated using an averaging algorithm (Haar wavelets)
 *      and stored in the array of lower resolution grid values
 *      for future reference.
 **/
 
void MultiresGrid::generateLowRes(int numResolutions)
{
    TRACE(numResolutions);      // {{{ precondition
    REQUIRE(mResolution==0, "resolution=%d, but only can generate low "
                "resolutions if from the high resolution\n", mResolution);
    // }}}

    static bool firstTimeCalled = true;

    if(firstTimeCalled) {
        mGrids.push_back(this);

        for(int res=1; res<=numResolutions; res++) {
            // start with the previous resolution to create the next
            MultiresGrid* g = mGrids[res-1];

            REQUIRE(g->mResolution==res-1, "internal error, expecting grid "
                        "for resolution %d, but grid resolution is %d\n",
                        res-1,g->mResolution);

            int newwidth  = g->width()/2;
            int newheight = g->height()/2;
            int newdepth  = g->depth()/2;

            float* xcoords = new float[newwidth];
            float* ycoords = new float[newheight];
            float* zcoords = new float[newdepth];

            for(int i=0,j=0; i<newwidth; i++, j+=2)
                xcoords[i] = (g->xLocation(j)+g->xLocation(j+1))/2.0;

            for(int i=0, j=0; i<newheight; i++, j+=2)
                ycoords[i] = (g->yLocation(j)+g->yLocation(j+1))/2.0;

            for(int i=0, j=0; i<newdepth; i++, j+=2)
                zcoords[i] = (g->zLocation(j)+g->zLocation(j+1))/2.0;

            MultiresGrid* r = 
                new MultiresGrid(newwidth, newheight, newdepth, 
                                    xcoords, ycoords, zcoords, 
                                    res);
            mGrids.push_back(r);
        }
    }

    firstTimeCalled = false;
}

/* ========================================================================= */
/* ==                               PRIVATE                               == */
/* ========================================================================= */

/* ========================================================================= */

void MultiresGrid::outputToFile(const char* filename)
{
    FILE* outfile = fopen(filename, "w");

    if(outfile==NULL)
        ERROR("Unable to open file '%s' for writing", filename);
    else
        outputToFile(outfile);
}

void MultiresGrid::outputToFile(FILE* outfile)
{
    fprintf(outfile, "DATASET MULTIRES_GRID\n");
    fprintf(outfile, "DIMENSIONS %d %d %d\n", mWidth, mHeight, mDepth);
    fprintf(outfile, "X_COORDINATES %d float\n", mWidth);

    for(int j=0; j<mWidth; j++)
        fprintf(outfile, "%0.6f ", mXcoords[j]);

    fprintf(outfile, "\nY_COORDINATES %d float\n", mHeight);

    for(int j=0; j<mHeight; j++)
        fprintf(outfile, "%0.6f ", mYcoords[j]);

    fprintf(outfile, "\nZ_COORDINATES %d float\n", mDepth);

    for(int j=0; j<mDepth; j++)
        fprintf(outfile, "%0.6f ", mZcoords[j]);

    fprintf(outfile, "\n");
    fflush(outfile);
    fclose(outfile);
}

/* ========================================================================= */
/* ==                              DESTRUCTOR                             == */
/* ========================================================================= */

MultiresGrid::~MultiresGrid()
{
    TRACE();
    DEBUG("DESTROYING grid coordintes for %p", this);

    if(mResolution==0) {
        for(unsigned int i=1; i<mGrids.size(); i++)
            delete mGrids[i];
    }

    delete [] mXcoords;
    delete [] mYcoords;
    delete [] mZcoords;
}

/* ========================================================================= */

#ifdef STAND_ALONE_MULTIRES_GRID

//  To compile a stand alone block:
//      g++ -g -I.. -DSTAND_ALONE_MULTIRES_GRID MultiresGrid.cpp

int main(void)
{
    char* file="/local/data/mhd/jcd0004/multires/jcd0004.392x112x112.grid";
    StarObject::catchSegmentationFaults(true);
    Grid* g = NULL;
    float* xc = NULL, *yc=NULL, *zc=NULL;

    while(true) {
        FILE* infile = stdin;
        char line[1024] = {0};

        if(infile == stdin) printf("MultiresGrid: ");

        fgets(line, sizeof line, infile);

        if(strcmp(line,"\n")==0) continue;

        vector<string> tokens = StarObject::split(line);

        if(tokens[0] == "q") {
            break;
        }
        else if(tokens[0] == "new") {
            if(tokens.size()!=2&& fprintf(stderr,"PARSE ERROR\n")) continue;

            const char* filename = tokens[1].c_str();

            delete g;
            g = new MultiresGrid(filename);
            fprintf(stderr,"  Created new MultiresGrid for %s\n",filename);
        }
        else if(tokens[0] == "load") {
            delete g;
            g = new MultiresGrid(file);
            fprintf(stderr,"  Created new MultiresGrid for '%s'\n",file);
        }
        else if(tokens[0] == "test") {
            delete g;
            g = new MultiresGrid(8, 8, 8);
            fprintf(stderr,"  Created test grid 8x8x8\n");
        }
        else if(tokens[0] == "loc") {
            if(tokens.size()!=4&& fprintf(stderr,"PARSE ERROR\n")) continue;
            if(g==NULL&&fprintf(stderr,"grid is null\n")) continue;

            int col = StarObject::toInt(tokens[1]);
            int row = StarObject::toInt(tokens[2]);
            int slice = StarObject::toInt(tokens[3]);

            float x = g->xLocation(col);
            float y = g->yLocation(row);
            float z = g->zLocation(slice);

            fprintf(stderr,"  location for (%d,%d,%d) is (%0.2f,"
                    "%0.2f,%0.2f)\n", col, row, slice, x, y, z);
        }
        else if(tokens[0] == "print") {
            if(tokens.size()!=2&& fprintf(stderr,"PARSE ERROR\n")) continue;
            if(g==NULL&&fprintf(stderr,"grid is null\n")) continue;

            int n = StarObject::toInt(tokens[1]);

            fprintf(stderr, "x coords:\n");
            for(int i=0; i<n; i++) 
                fprintf(stderr, "%0.2f%c", g->xLocation(i), i==n-1?'\n':' ');

            fprintf(stderr, "y coords:\n");
            for(int i=0; i<n; i++) 
                fprintf(stderr, "%0.2f%c", g->yLocation(i), i==n-1?'\n':' ');

            fprintf(stderr, "z coords:\n");
            for(int i=0; i<n; i++) 
                fprintf(stderr, "%0.2f%c", g->zLocation(i), i==n-1?'\n':' ');
        }
        else if(tokens[0] == "size") {
            if(g==NULL&&fprintf(stderr,"grid is null\n")) continue;

            int width = g->width();
            int height = g->height();
            int depth = g->depth();

            float gwidth  = g->geometryWidth();
            float gheight = g->geometryHeight();
            float gdepth  = g->geometryDepth();

            fprintf(stderr,"  size: topology = [%d,%d,%d], geometry = "
                    "[%0.2f,%0.2f,%0.2f]\n", 
                    width,height,depth,gwidth,gheight,gdepth);
        }
        else if(tokens[0] == "atres") {
            if(g==NULL&&fprintf(stderr,"grid is null\n")) continue;

            if(tokens.size()!=2&& fprintf(stderr,"PARSE ERROR\n")) continue;

            int res = StarObject::toInt(tokens[1]);

            fprintf(stderr,"  Changing to resolution %d\n", res);

            g = g->atResolution(res);
        }
        else {
            fprintf(stderr,"commands:\n"
                           "  new GRIDFILENAME\n"
                           "  load (loads file '%s')\n"
                           "  test (loads small 8x8x8 test grid)\n"
                           "  loc COL ROW SLICE\n"
                           "  print N (show 1st N locations for each axis)\n"
                           "  size\n"
                           "  atres RES\n"
                           , file
                           );
        }
    }
}

//  To compile a stand alone block:
//      g++ -g -I.. -DSTAND_ALONE_MULTIRES_GRID MultiresGrid.cpp
//

#endif // STAND_ALONE_MULTIRES_GRID

/* ========================================================================= */
