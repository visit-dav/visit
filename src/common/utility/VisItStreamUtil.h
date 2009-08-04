/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#ifndef VISIT_STREAM_UTILITY_H
#define VISIT_STREAM_UTILITY_H

#include <visitstream.h>
#include <utility_exports.h>
#include <vector>
#include <list>
#include <map>

// ****************************************************************************
//  Method: ostream operator<<
//
//  Purpose:
//      Output vectors, lists and maps to a stream.
//
//  Returns:    the stream
//
//  Programmer: Dave Pugmire
//  Creation:   June 8, 2009
//
// ****************************************************************************

template<class T>
inline std::ostream& operator<<(std::ostream& out,
                                const std::vector<T> &v)
{
    typename std::vector<T>::const_iterator b = v.begin();
    typename std::vector<T>::const_iterator e = v.end();

    out<<"[";
    while (b != e)
    {
        out<<*b;
        std::advance(b,1);
        if (b != e)
            out<<" ";
    }
    out<<"]";
    return out;
}

template<class T>
inline std::ostream& operator<<(std::ostream& out,
                                const std::list<T> &l)
{
    typename std::list<T>::const_iterator b = l.begin();
    typename std::list<T>::const_iterator e = l.end();

    out<<"[";
    while (b != e)
    {
        out<<*b;
        std::advance(b,1);
        if (b != e)
            out<<" ";
    }
    out<<"]";
    return out;
}

template<class S, class T>
inline std::ostream& operator<<(std::ostream& out,
                                const std::map<S,T> &m)
{
    typename std::map<S,T>::const_iterator b = m.begin();
    typename std::map<S,T>::const_iterator e = m.end();

    out<<"[";
    while (b != e)
    {
        out<<"("<<b->first<<" "<<b->second<<")";
        std::advance(b,1);
        if (b != e)
            out<<" ";
    }
    out<<"]";
    return out;
}
#endif
