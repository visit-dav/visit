#ifndef ARRAY_REF_PTR_H
#define ARRAY_REF_PTR_H
#include <utility_exports.h>

#include <iostream.h>

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
class UTILITY_API array_ref_ptr
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

#if defined(_WIN32)
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
    out << "<" << *n << ", " << p << ">";
}

typedef array_ref_ptr<char> CharStrRef;


#endif
