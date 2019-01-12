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
//  File:        avtResolutionInfo.h                                         //
//  Programmer:  Andrew Foulks <rafoulks@cs.unh.edu>                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _AVT_RESOLUTION_INFO_H_
#define _AVT_RESOLUTION_INFO_H_

#include "avtDataSelection.h"

#include <vector>

//   This class is used to record information about how 
//   many resolutions are available, and the size of each 
//   resolution.

class avtResolutionInfo : public avtDataSelection {
public:
                         avtResolutionInfo()  {mSizes.reserve(16);}
    virtual              ~avtResolutionInfo() {}
    // base class api
    virtual const char*  GetType() const { return "avtResolutionInfo"; }

    // added functionality
    struct DataSize { 
        int width; int height; int depth; 
        DataSize(int w=0, int h=0, int d=0) : width(w), height(h), depth(d) {;}
    };

    // the resolution is an index into the array of sizes
    virtual void addSize(int width, int height, int depth, int resolution) {
        if((int)mSizes.size() <= resolution)
            mSizes.resize(resolution+1);

        mSizes[resolution] = DataSize(width, height, depth);
    }
    virtual std::vector<DataSize>& sizes() { return mSizes; }
    virtual int               numResolutions() { return (int)mSizes.size(); }

private:
    std::vector<DataSize> mSizes;
};

#endif // _AVT_RESOLUTION_INFO_H_

