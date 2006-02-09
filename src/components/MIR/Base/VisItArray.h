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
