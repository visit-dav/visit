#ifndef FACE_H
#define FACE_H
#include <database_exports.h>

#include <limits.h>

// ****************************************************************************
//  Class:  Face
//
//  Purpose:
//    Way to indentify a face within a mesh (uses three lowest node id's)
//
//  Note:   
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
class DATABASE_API Face
{
  public:
    Face();
    Face(int, const int *);
    void operator=(const Face &rhs);
    bool operator==(const Face &rhs);
    static unsigned int HashFunction(Face &face);
  private:
    int a,b,c;
};


// ----------------------------------------------------------------------------
//                             Inline Methods
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Default Constructor:  Face::Face
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline
Face::Face()
{
    a=-1;
    b=-1;
    c=-1;
}


// ****************************************************************************
//  Constructor:  Face::Face
//
//  Purpose:
//    create yourself from a vtk face
//
//  Arguments:
//    cell       the vtk face
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Oct  8 17:02:05 PDT 2002
//    Removed costly VTK calls.
//
// ****************************************************************************
inline
Face::Face(int n, const int *ids)
{
    a = INT_MAX;
    b = INT_MAX;
    c = INT_MAX;

    for (int i=0; i<n; i++)
    {
        int node = ids[i];
        if (node < a)     { c=b; b=a; a=node; }
        else if (node < b)     { c=b; b=node; }
        else if (node < c)          { c=node; }
    }
}

// ****************************************************************************
//  Method:  Face::operator=
//
//  Purpose:
//    Assignment operator
//
//  Arguments:
//    rhs        the source face
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline void
Face::operator=(const Face &rhs)
{
    a=rhs.a;
    b=rhs.b;
    c=rhs.c;
}

// ****************************************************************************
//  Method:  Face::operator==
//
//  Purpose:
//    comparison operator
//
//  Arguments:
//    rhs        the face to compare with
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline bool
Face::operator==(const Face &rhs)
{
    return (a==rhs.a && b==rhs.b && c==rhs.c);
}

#endif
