/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

// ************************************************************************* //
//                                  MemStream.C                              //
// ************************************************************************* //

#include <MemStream.h>


// ****************************************************************************
//  Method: MemStream constructor
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

MemStream::MemStream(size_t sz0)
{
    _pos = 0;
    _len = 0;
    _maxLen = _len;
    _data = NULL;
    CheckSize(sz0);
}


// ****************************************************************************
//  Method: MemStream constructor
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

MemStream::MemStream(size_t sz, const unsigned char *buff)
{
    _pos = 0;
    _len = sz;
    _maxLen = _len;
    
    _data = new unsigned char[_len];
    memcpy(_data, buff, _len);
}

// ****************************************************************************
// Method:  MemStream constructor
//
// Programmer:  Dave Pugmire
// Creation:    September 10, 2010
//
// ****************************************************************************

MemStream::MemStream(const MemStream &s)
{
    _pos = 0;
    _len = s.len();
    _maxLen = _len;
    _data = new unsigned char[_len];
    memcpy(_data, s.data(), _len);
}


// ****************************************************************************
//  Method: MemStream destructor
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

MemStream::~MemStream()
{
    if (_data)
        delete [] _data;
    _pos = 0;
    _len = 0;
    _maxLen = 0;
    _data = NULL;
}

// ****************************************************************************
//  Method: MemStream::CheckSize
//
//  Purpose:
//      Checks to see if the current size is sufficent and changes the size
//      of the underlying array if not.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

void
MemStream::CheckSize(size_t sz)
{
    size_t reqLen = _pos+sz;
    
    if (reqLen > _maxLen)
    {
        size_t newLen = 2*_maxLen; // double current size.
        if (newLen < reqLen)
            newLen = reqLen;
        
        unsigned char *newData = new unsigned char[newLen];
        
        if (_data)
        {
            memcpy(newData, _data, _len); // copy existing data to new buffer.
            delete [] _data;
        }
        _data = newData;
        _maxLen = newLen;
    }
}


