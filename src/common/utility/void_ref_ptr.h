// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                void_ref_ptr.h                             //
// ************************************************************************* //

#ifndef VOID_REF_PTR_H
#define VOID_REF_PTR_H

#include <utility_exports.h>

#include <visitstream.h>
#include "VisItAtomic.h"

// ****************************************************************************
//  Class: void_ref_ptr
//
//  Purpose:
//      Reference counted void pointer.  
//      This is similar to ref_ptr, but it has slightly different semantics
//      to ensure that we know how to delete it.
//
//  Programmer: Hank Childs
//  Creation:   September 24, 2002
//
//  Modifications:
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added operator==
// ****************************************************************************

typedef void (*DestructorFunction)(void *);

class UTILITY_API void_ref_ptr
{
 public:
    void_ref_ptr();
    void_ref_ptr(void *p_, DestructorFunction, VISIT_ATOMIC_TYPE * = NULL);
    void_ref_ptr(const void_ref_ptr &rhs);
    ~void_ref_ptr();
    void Assign(void *rhs, DestructorFunction);
    void operator=(const void_ref_ptr &rhs);
    bool operator==(const void_ref_ptr &rhs) const;
    void *operator*();
    VISIT_ATOMIC_TYPE *GetN() const;
    void Print(ostream&);
 private:
    void    *p;
    VISIT_ATOMIC_TYPE     *n;
    DestructorFunction destruct;

    void   AddReference(void *, VISIT_ATOMIC_TYPE *);
    void   RemoveReference(void);
};

struct UTILITY_API VoidRefList
{
  public:
              VoidRefList();
    virtual  ~VoidRefList();

  public:
    void_ref_ptr  *list;
    int            nList;
};

// ****************************************************************************
//  Inline Methods
// ****************************************************************************
inline void
void_ref_ptr::AddReference(void *p_, VISIT_ATOMIC_TYPE *n_)
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

inline void
void_ref_ptr::RemoveReference(void)
{
    if (p)
    {
        VISIT_ATOMIC_TYPE a = AtomicDec(*n);
        if (a == 0)
        {
            if (destruct != NULL)
            {
                destruct(p);
            }
            delete n;
        }
    }
}

inline
void_ref_ptr::void_ref_ptr()
{
    AddReference(NULL, NULL);
    destruct = NULL;
}

inline
void_ref_ptr::void_ref_ptr(void *p_, DestructorFunction df, VISIT_ATOMIC_TYPE *n_)
{
    AddReference(p_, n_);
    destruct = df;
}

inline
void_ref_ptr::void_ref_ptr(const void_ref_ptr &rhs)
{
    AddReference(rhs.p, rhs.n);
    destruct = rhs.destruct;
}

inline
void_ref_ptr::~void_ref_ptr()
{
    RemoveReference();
}

inline void
void_ref_ptr::Assign(void *rhs, DestructorFunction df)
{
    RemoveReference();
    AddReference(rhs, NULL);
    destruct = df;
}

inline void
void_ref_ptr::operator=(const void_ref_ptr &rhs)
{
    if (p == rhs.p && n == rhs.n)
        return;
    RemoveReference();
    AddReference(rhs.p, rhs.n);
    destruct = rhs.destruct;
}

inline void *
void_ref_ptr::operator*()
{
    return p;
}

inline bool
void_ref_ptr::operator==(const void_ref_ptr &rhs) const
{
    return (p == rhs.p);
}

inline VISIT_ATOMIC_TYPE *
void_ref_ptr::GetN() const
{
    return n;
}

inline void
void_ref_ptr::Print(ostream &out)
{
    out << "<" << *n << ", " << p << ">";
}



#endif
