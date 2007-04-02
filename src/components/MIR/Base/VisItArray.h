/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef VISITARRAY_H
#define VISITARRAY_H
#include <mir_exports.h>

#include <string.h>

// ****************************************************************************
//  Class:  VisItArray
//
//  Purpose:
//    Lightweight implementation of the STL vector template class.
//
//  Note:   It's missing much of the functionality, but it should perform
//          better for push_back operations.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 17, 2001
//
//  Modifications:
//    Jeremy Meredith, Mon Sep 15 09:47:58 PDT 2003
//    Updated to allow reserving memory.
//
//    Mark C. Miller, Thu Feb  9 20:29:57 PST 2006
//    Renamed to VisItArray
//
// ****************************************************************************
template<class T>
class MIR_API VisItArray
{
  private:
    T     *d;
    int    cp;
    int    sz;
    void operator=(const VisItArray<T> &rhs) { };
  public:
    VisItArray()
    {
        d = new T[10];
        cp = 10;
        sz = 0;
    }
    VisItArray(int initSize)
    {
        d = new T[initSize];
        cp = initSize;
        sz = 0;
    }
    ~VisItArray()
    {
        delete[] d;
    }

    bool       empty()                  const { return sz==0; }
    const int &size()                   const { return sz;    }
    void       clear()                        { sz = 0;       }
    const T   &operator[](const int &i) const { return d[i];  }
          T   &operator[](const int &i)       { return d[i];  }

    void resize(int n)
    {
        reserve(n);
        sz = n;
    }
    void reserve(int n)
    {
        if (n > cp)
        {
            T *d2 = new T[n];
            memcpy(d2, d, sizeof(T)*sz);
            delete[] d;
            d = d2;
            cp = n;
        }
    }

    void push_back(const T& t)
    {
        if (sz+1 > cp)
        {
            reserve(int(float(cp)*2));
        }
        d[sz++] = t;
    }

    static void Swap(VisItArray<T> &a1, VisItArray<T> &a2)
    {
        T  *td = a1.d;
        a1.d   = a2.d;
        a2.d   = td;
        int tc = a1.cp;
        a1.cp  = a2.cp;
        a2.cp  = tc;
        int ts = a1.sz;
        a1.sz  = a2.sz;
        a2.sz  = ts;
    }
};

#endif
