/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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

#ifndef REF_PTR_H
#define REF_PTR_H

#include <visitstream.h>
#include "VisItAtomic.h"

// ****************************************************************************
//  Class: ref_ptr
//
//  Purpose:
//      Reference counted pointer.  
//      This is similar to the STL auto_ptr, but it is actually useful.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 29, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Oct 17 13:52:17 PDT 2000
//    Fixed bug with assigning new pointer and not decreasing reference count
//    for previous pointer and added routines Add- and RemoveReference.  Also
//    added a conversion function (Copy) for downcasting and changed the
//    ostream operator to Print to avoid warnings.
//
//    Jeremy Meredith, Tue Jun  5 13:04:16 PDT 2001
//    Added a check for equality so that self-assignment is a no-op.
//
//    Brad Whitlock, Thu Apr 4 14:17:44 PST 2002
//    Added GetN method and removed CopyTo method to get around the Windows
//    compiler.
//
//    Hank Childs, Tue Sep 24 09:29:57 PDT 2002
//    Remove references to VoidRef, since it is getting its own class.
//
//    Brad Whitlock, Tue Aug 10 17:06:50 PST 2004
//    Removed some Windows API stuff that was not necessary.
//
//    Tom Fogal, Tue Jun 23 20:05:12 MDT 2009
//    Made `source' argument in CopyTo const.
//
//    David Camp, Tue Mar 12 13:19:37 PDT 2013
//    Made the code thread safe. Changed the int to be an atomic type.
//
//    Burlen Loring, Fri Sep  4 08:30:24 PDT 2015
//    added bool cast to better match normal pointer behavior and
//    to follow the convention used in other common shared pointer
//    implementations
//
// ****************************************************************************

template <class T>
class ref_ptr
{
 public:
    ref_ptr();
    ref_ptr(T *p_, VISIT_ATOMIC_TYPE * = NULL);
    ref_ptr(const ref_ptr<T> &rhs);
    ~ref_ptr();
    void operator=(T *rhs);
    void operator=(const ref_ptr<T> &rhs);
    T *operator*();
    const T *operator*() const;
    T *operator->();
    const T *operator->() const;
    VISIT_ATOMIC_TYPE *GetN() const;
    void Print(ostream&);
    void   SetReference(T *p_)
    {
        RemoveReference();
        AddReference(p_, NULL);
    }
    operator bool() { return p != NULL; }
 private:
    T                 *p;
    VISIT_ATOMIC_TYPE *n;

    void   AddReference(T *, VISIT_ATOMIC_TYPE *);
    void   RemoveReference(void);
};

// ****************************************************************************
//  Inline Methods
// ****************************************************************************
template <class T>
inline
ref_ptr<T>::ref_ptr()
{
    AddReference(NULL, NULL);
}

template <class T>
inline
ref_ptr<T>::ref_ptr(T *p_, VISIT_ATOMIC_TYPE *n_)
{
    AddReference(p_, n_);
}

template <class T>
inline
ref_ptr<T>::ref_ptr(const ref_ptr<T> &rhs)
{
    AddReference(rhs.p, rhs.n);
}

template <class T>
inline
ref_ptr<T>::~ref_ptr()
{
    RemoveReference();
}

template <class T>
inline void
ref_ptr<T>::operator=(T *rhs)
{
    RemoveReference();
    AddReference(rhs, NULL);
}

template <class T>
inline void
ref_ptr<T>::operator=(const ref_ptr<T> &rhs)
{
    if (p == rhs.p && n == rhs.n)
        return;
    RemoveReference();
    AddReference(rhs.p, rhs.n);
}

template <class T>
inline T *
ref_ptr<T>::operator*()
{
    return p;
}

template <class T>
inline const T *
ref_ptr<T>::operator*() const
{
    return p;
}

template <class T>
inline T *
ref_ptr<T>::operator->()
{
    return p;
}

template <class T>
inline const T *
ref_ptr<T>::operator->() const
{
    return p;
}

template <class T>
inline VISIT_ATOMIC_TYPE *
ref_ptr<T>::GetN() const
{
    return n;
}

template <class T>
void
ref_ptr<T>::AddReference(T *p_, VISIT_ATOMIC_TYPE *n_)
{
    p = p_;
    if (p != NULL)
    {
        if (n_ == NULL)
        {
            n = new VISIT_ATOMIC_TYPE(1); 
        }
        else
        {
            n = n_;
            AtomicInc(*n);
        }
    }
    else
    {
        n = NULL;
    }
}

template <class T>
void
ref_ptr<T>::RemoveReference(void)
{
    if (p)
    {
        VISIT_ATOMIC_TYPE a = AtomicDec(*n);
        if (a == 0)
        {
            delete p;
            delete n;
        }
    }
}

template <class T>
inline void
ref_ptr<T>::Print(ostream &out)
{
    if ((n!=NULL) && (p!=NULL))
        out << "<" << *n << ", " << (void*) p << ">";
    else
        out << "<NULL>";
}


template <class S, class T>
inline void
CopyTo(ref_ptr<S> &lhs, const ref_ptr<T> &rhs)
{
    ref_ptr<S> newone((S *)*rhs, rhs.GetN());
    lhs = newone;
}

#endif
