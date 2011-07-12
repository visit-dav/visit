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
//  File:        ResolutionMap.h                                             //
//  Programmer:  Andrew Foulks <rafoulks@cs.unh.edu>                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _RESOLUTION_MAP_H_
#define _RESOLUTION_MAP_H_

#include "StarObject.h"

#include <vector>

/**
 *      An adaptive resolution map assumes a chunked data format
 *      and maps each chunk int he dataset to a resolution.
 *      The resolution is determined by an error tolerance
 *      and error data.  The resolution returned for a given
 *      chunk is the lowest possible that still falls below the
 *      error tolernace threshhold.
 **/

class ResolutionMap : public StarObject
{
public:  // constructors
            ResolutionMap                      (int numchunks, int numres);
    virtual ~ResolutionMap                     ();

public:  // api
    virtual void        loadMapFromFile         (const char* filename);
    virtual void        loadRandomMap           ();
    virtual void        loadUniformResMap       (int resolution);
    virtual void        setMap                  (int resolutions[]);
    virtual int         resolutionForChunk      (int chunkIndex);
    virtual int         numResolutions          ();
    virtual int         numChunks               ();
    virtual void        outputToFile            (FILE* outfile);

private:
    int                 mNumChunks;
    int                 mNumResolutions;
    std::vector<int>    mResolutionMap;         // index is chunk number

private: // disable copy constructor and operator=
    ResolutionMap(ResolutionMap&);
    ResolutionMap& operator=(ResolutionMap&);
};

#endif // _RESOLUTION_MAP_H_

