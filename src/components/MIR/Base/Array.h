#ifndef ARRAY_H
#define ARRAY_H
#include <mir_exports.h>

#include <string.h>

// ****************************************************************************
//  Class:  Array
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
// ****************************************************************************
template<class T>
class MIR_API Array
{
  private:
    T     *d;
    int    cp;
    int    sz;
    void operator=(const Array<T> &rhs) { };
  public:
    Array()
    {
        d = new T[10];
        cp = 10;
        sz = 0;
    }
    Array(int initSize)
    {
        d = new T[initSize];
        cp = initSize;
        sz = 0;
    }
    ~Array()
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

    static void Swap(Array<T> &a1, Array<T> &a2)
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
