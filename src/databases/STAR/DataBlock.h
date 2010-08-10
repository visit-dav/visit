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
//  File:        DataBlock.h                                                 //
//  Programmer:  Andrew Foulks <rafoulks@cs.unh.edu>                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _DATA_BLOCK_H_
#define _DATA_BLOCK_H_

/**
 *      The DataBlock is a lightweight wrapper around raw data
 *      pointers.  It gives access to a 1D subset of the data 
 *      from within in a larger context.  This is not the same 
 *      as an arbitrary subset, it is more like a slice of
 *      a 1D array.
 *
 *      @author Andrew Foulks
 **/

class DataBlock
{
public:  // constructors
            DataBlock(): mData(NULL), mWidth(0), mHeight(0), mDepth(0) {} 
            DataBlock(float* data, int width, int height, int depth) :
                mData(data), mWidth(width), mHeight(height), mDepth(depth) {}
    virtual ~DataBlock() { mData = NULL; }

public:  // api 
    virtual attach(float* data, int width, int height, int depth) {
        mData = data;
        mWidth = width;
        mHeight = height;
        mDepth = depth;
    }
    virtual detach() { 
        mData = NULL; 
        mWidth = mHeight = mDepth = 0;
    }
    virtual const float*    rawData         () const { return mData; }
    virtual int             width           () const { return mWidth; }
    virtual int             height          () const { return mHeight; }
    virtual int             depth           () const { return mDepth; }

private: // members
    float*                  mData;          // ptr to data
    int                     mWidth;
    int                     mHeight;
    int                     mDepth;

private: // disable copy constructor and operator=
    DataBlock(DataBlock&);
    DataBlock& operator=(DataBlock&);
};

#endif // _DATA_BLOCK_H_

