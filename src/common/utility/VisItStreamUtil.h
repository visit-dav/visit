// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_STREAM_UTILITY_H
#define VISIT_STREAM_UTILITY_H

#include <visitstream.h>
#include <utility_exports.h>
#include <vector>
#include <list>
#include <map>
#include <tuple>

// ****************************************************************************
//  Method: ostream operator<<
//
//  Purpose:
//      Output vectors, lists, maps and pairs to a stream.
//
//  Returns:    the stream
//
//  Programmer: Dave Pugmire
//  Creation:   June 8, 2009
//
//   Dave Pugmire, Wed Jun 13 14:21:26 EDT 2012
//   Add code for pairs.
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

template<class S, class T>
inline std::ostream& operator<<(std::ostream& out,
                                const std::pair<S,T> &p)
{
    out<<"("<<p.first<<", "<<p.second<<")";
    return out;
}

template<class S, class T, class U>
inline std::ostream& operator<<(std::ostream& out,
                                const std::tuple<S,T,U> &t)
{
    out<<"("<<std::get<0>(t)<<", "<<std::get<1>(t)<<", "<<std::get<2>(t)<<")";
    return out;
}

#endif
