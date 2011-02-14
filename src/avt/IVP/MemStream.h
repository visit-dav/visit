/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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
//                                  MemStream.h                              //
// ************************************************************************* //

#ifndef MEM_STREAM_H
#define MEM_STREAM_H

#include <ivp_exports.h>
#include <string>
#include <vector>
#include <list>
#include <cstring>
#include <avtVector.h>
#include <vector>
#include <DebugStream.h>
#include <VisItException.h>

class vtkDataSet;

// ****************************************************************************
//  Struct: MemStream
//
//  Purpose:
//      A helper class for serializing a streamline.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
//  Modifications:
//
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
//   Dave Pugmire, Fri Sep 10 13:35:33 EDT 2010
//   Add new members.
//
//   Dave Pugmire, Mon Sep 20 15:03:27 EDT 2010
//   Rename methods, and member data.
//
//   Dave Pugmire, Fri Jan  7 13:59:48 EST 2011
//   Read/writing of vtkDataSet
//
// ****************************************************************************

struct IVP_API MemStream
{
  public:
    enum Mode {
        READ = 0,
        WRITE
    };
    
    MemStream(size_t sz0= 32);
    MemStream(size_t sz, const unsigned char *buff);
    MemStream(const MemStream &s);
    ~MemStream();

    void           rewind() {_pos = 0;}
    size_t         pos() const {return _pos;}
    void           setPos(size_t p);
    size_t         len() const { return _len; }
    size_t         capacity() const { return _maxLen; }
    unsigned char *data() const { return _data; }

    // General read/write routines.
    template <typename T> void io(Mode mode, T *pt, size_t num) {return (mode == READ ? read(pt,num) : write(pt,num));}
    template <typename T> void io(Mode mode, T& t) {size_t s=1; io( mode, &t, s );}
    template <typename T> void io(Mode mode, std::vector<T> &v)  {return (mode == READ ? read(v) : write(v));}
    void                       io(Mode mode, avtVector &v) {return (mode == READ ? read(v) : write(v));}

    //Read from buffer.
    template <typename T> void read(T *pt, const size_t &num);
    template <typename T> void read(T& t) { read(&t, 1);}
    template <typename T> void read(std::vector<T> &v);
    void read(avtVector &v);
    void read(std::string &str);
    void read(vtkDataSet **ds);

    //Write to buffer.
    template <typename T> void write(const T& t) { write( &t, 1 );}
    template <typename T> void write(const T *const pt, size_t num);
    template <typename T> void write(const std::vector<T> &v);
    void write(const avtVector &v);
    void write(const std::string &str);
    void write(vtkDataSet *ds);

  private:
    // data members
    unsigned char *_data;
    size_t _len, _maxLen, _pos;

    void CheckSize(size_t sz);

    friend std::ostream& operator<<(std::ostream &out, const MemStream &m)
    {
        out<<" MemStream(p= "<<m.pos()<<", l= "<<m.len()<<"["<<m.capacity()<<"]) data=[";
        for (size_t i=0; i < m.len(); i++)
            out<<(int)(m._data[i])<<" ";
        out<<"]";
        return out;
    }
};


inline void MemStream::setPos(size_t p)
{
    _pos = p;
    if (_pos > len())
        EXCEPTION0(ImproperUseException);
}

template <typename T> inline void MemStream::read(T *pt, const size_t &num)
{
    size_t nBytes = sizeof(T)*num;
    memcpy(pt, &_data[_pos], nBytes);
    _pos += nBytes;
}

inline void MemStream::read(avtVector &v)
{
    read(v.x);
    read(v.y);
    read(v.z);
}

inline void MemStream::read(std::string &str)
{
    size_t sz;
    read(sz);
    str.resize(sz);
    read(&str[0], sz);
}

template <typename T> inline void MemStream::read(std::vector<T> &v)
{
    size_t sz;
    read(sz);
    v.resize(sz);
    for ( size_t i = 0; i < sz; i++ )
        read(v[i]);
}

template <typename T> inline void MemStream::write(const T *const pt, size_t num)
{
    size_t nBytes = sizeof(T)*num;
    CheckSize(nBytes);
    memcpy(&_data[_pos], pt, nBytes );
    _pos += nBytes;
    
    if (_pos > _len)
        _len = _pos;
}

template <typename T> inline void MemStream::write(const std::vector<T> &v)
{
    write(v.size());
    for (size_t i = 0; i < v.size(); i++)
        write(v[i]);
}

inline void MemStream::write(const avtVector &v)
{
    write(v.x);
    write(v.y);
    write(v.z);
}

inline void MemStream::write(const std::string &str)
{
    size_t sz = str.size();
    write(sz);
    write(str.data(), sz);
}







#endif


