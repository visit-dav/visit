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
//  File:        MultiresGrid.h                                              //
//  Programmer:  Andrew Foulks <rafoulks@cs.unh.edu>                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////



#ifndef _MULTIRES_GRID_H_
#define _MULTIRES_GRID_H_

#include "StarObject.h"

/**
 *      MultiresGrid is used to store the locations of the 
 *      data values stored in a block of data.  This is used for 
 *      structured grid data sets.  The format is that
 *      of a Rectilinear Grid, one for each resolution.  There
 *      are 3 arrays of locations, one for the x-coordinates,
 *      one for the y-coordinates and one for the z-coordinates.
 *
 *      The highest resolution grid locations are read in from
 *      an ascii text file (@see parseGridFile()).  Low 
 *      resolutions can be made by calling @see generateLowRes(), 
 *      which performs a haar wavelet transform on the high 
 *      resolution locations.  The low res points are then 
 *      accessed by calling @see atResolution().
 *
 *      @author Andrew Foulks
 **/

class MultiresGrid : public StarObject
{
public:  // constants

public:  // constructors
            MultiresGrid                     ();
            MultiresGrid                     (const char* filename);
            MultiresGrid                     (int width, int height, int depth);
            MultiresGrid                     (int width, int height, int depth,
                                              const float* xcoords,
                                              const float* ycoords,
                                              const float* zcoords,
                                              int resolution=0);
    virtual ~MultiresGrid();

public:  // Grid interface
    virtual int             width               () const;
    virtual int             height              () const;
    virtual int             depth               () const;

    virtual float           xLocation           (int col) const;
    virtual float           yLocation           (int row) const;
    virtual float           zLocation           (int slice) const;

    virtual float           geometryWidth       () const;
    virtual float           geometryHeight      () const;
    virtual float           geometryDepth       () const;

    virtual const float*    xcoords             () const { return mXcoords; }
    virtual const float*    ycoords             () const { return mYcoords; }
    virtual const float*    zcoords             () const { return mZcoords; }
    virtual int             resolution          () const {return mResolution;}

    // for chunked data
    virtual const float*    xcoordsChunk        (int whichChunk,
                                                 int numXChunks,
                                                 int numYChunks,
                                                 int numZChunks) const;

    virtual const float*    ycoordsChunk        (int whichChunk,
                                                 int numXChunks,
                                                 int numYChunks,
                                                 int numZChunks) const;

    virtual const float*    zcoordsChunk        (int whichChunk,
                                                 int numXChunks,
                                                 int numYChunks,
                                                 int numZChunks) const;

    virtual MultiresGrid* atResolution          (int resolution) const;
    void                    generateLowRes      (int numResolutions);
    void                    outputToFile        (FILE* outfile);
    void                    outputToFile        (const char* filename);

public:  // api
    virtual bool            parseGridFile       (const char* filename);
    virtual const char*     filename            () const;

    
protected:  // members
    int                             mResolution;    // resolution (0 is high)
    string                          mFilename;
    static vector<MultiresGrid*>    mGrids;         // array of resolutions
    const float*                    mXcoords;
    const float*                    mYcoords;
    const float*                    mZcoords;
    int                             mWidth;
    int                             mHeight;
    int                             mDepth;

private: // disable copy constructor and operator= 
    /** copy constructor disabled by making it private */
    MultiresGrid(MultiresGrid&);
    /** operator= disabled by making it private */
    MultiresGrid& operator=(MultiresGrid&);
};

#endif // _MULTIRES_GRID_H_

