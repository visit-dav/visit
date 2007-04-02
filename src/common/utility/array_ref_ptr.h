/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

#ifndef ARRAY_REF_PTR_H
#define ARRAY_REF_PTR_H

#include <visitstream.h>

// ****************************************************************************
//  Class: array_ref_ptr
//
//  Purpose:
//      Reference counted pointer for array variables.
//      This is similar to the STL auto_ptr, but it is actually useful.
//
//  Notes:      Copied from ref_ptr class.
//
//  Programmer: Hank Childs
//  Creation:   September 17, 2001
//
// ****************************************************************************

template <class T>
class array_ref_ptr
{
 public:
    array_ref_ptr();
    array_ref_ptr(T *p_, int * = NULL);
    array_ref_ptr(const array_ref_ptr<T> &rhs);
    ~array_ref_ptr();
    void operator=(T *rhs);
    void operator=(const array_ref_ptr<T> &rhs);
    T *operator*();
    const T *operator*() const;
    T *operator->();
    const T *operator->() const;
    template <class S>  void CopyTo(array_ref_ptr<S> &);
    void Print(ostream&);
 private:
    T    *p;
    int  *n;

    void   AddReference(T *, int *);
    void   RemoveReference(void);
};

// ****************************************************************************
//  Inline Methods
// ****************************************************************************
template <class T>
inline
array_ref_ptr<T>::array_ref_ptr()
{
    AddReference(NULL, NULL);
}

template <class T>
inline
array_ref_ptr<T>::array_ref_ptr(T *p_, int *n_)
{
    AddReference(p_, n_);
}

template <class T>
inline
array_ref_ptr<T>::array_ref_ptr(const array_ref_ptr<T> &rhs)
{
    AddReference(rhs.p, rhs.n);
}

template <class T>
inline
array_ref_ptr<T>::~array_ref_ptr()
{
    RemoveReference();
}

template <class T>
inline void
array_ref_ptr<T>::operator=(T *rhs)
{
    RemoveReference();
    AddReference(rhs, NULL);
}

template <class T>
inline void
array_ref_ptr<T>::operator=(const array_ref_ptr<T> &rhs)
{
    if (p == rhs.p && n == rhs.n)
        return;
    RemoveReference();
    AddReference(rhs.p, rhs.n);
}

template <class T>
inline T *
array_ref_ptr<T>::operator*()
{
    return p;
}

template <class T>
inline const T *
array_ref_ptr<T>::operator*() const
{
    return p;
}

template <class T>
inline T *
array_ref_ptr<T>::operator->()
{
    return p;
}

template <class T>
inline const T *
array_ref_ptr<T>::operator->() const
{
    return p;
}

template <class T>
void
array_ref_ptr<T>::AddReference(T *p_, int *n_)
{
    p = p_;
    if (p != NULL)
    {
        if (n_ == NULL)
        {
            n = new int(1); 
        }
        else
        {
            n = n_;
            (*n)++;
        }
    }
    else
    {
        n = NULL;
    }
}

template <class T>
void
array_ref_ptr<T>::RemoveReference(void)
{
    if (p)
    {
        (*n)--;
        if (*n == 0)
        {
            delete [] p;
            delete n;
        }
    }
}

#if defined(_WIN32) && defined(USING_MSVC6)
template <class T, class S>
#else
template <class T>
template <class S>
#endif
void
array_ref_ptr<T>::CopyTo(array_ref_ptr<S> &rhs)
{
    array_ref_ptr<S>  newone((S *) p, n);
    rhs = newone;
}

template <class T>
inline void
array_ref_ptr<T>::Print(ostream &out)
{
    out << "<" << *n << ", " << (void*) p << ">";
}

typedef array_ref_ptr<char> CharStrRef;


#endif
