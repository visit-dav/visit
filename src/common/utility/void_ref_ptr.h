// ************************************************************************* //
//                                void_ref_ptr.h                             //
// ************************************************************************* //

#ifndef VOID_REF_PTR_H
#define VOID_REF_PTR_H

#include <utility_exports.h>

#include <iostream.h>

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
// ****************************************************************************

typedef void (*DestructorFunction)(void *);

class UTILITY_API void_ref_ptr
{
 public:
    void_ref_ptr();
    void_ref_ptr(void *p_, DestructorFunction, int * = NULL);
    void_ref_ptr(const void_ref_ptr &rhs);
    ~void_ref_ptr();
    void Assign(void *rhs, DestructorFunction);
    void operator=(const void_ref_ptr &rhs);
    void *operator*();
    int *GetN() const;
    void Print(ostream&);
 private:
    void    *p;
    int     *n;
    DestructorFunction destruct;

    void   AddReference(void *, int *);
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
void_ref_ptr::AddReference(void *p_, int *n_)
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

inline void
void_ref_ptr::RemoveReference(void)
{
    if (p)
    {
        (*n)--;
        if (*n == 0)
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
void_ref_ptr::void_ref_ptr(void *p_, DestructorFunction df, int *n_)
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

inline int *
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
