#ifndef EDGE_H
#define EDGE_H
#include <mir_exports.h>

#include <limits.h>

// ****************************************************************************
//  Class:  Edge
//
//  Purpose:
//    Way to indentify an edge within a mesh (uses two lowest node id's)
//
//  Note:   
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
class MIR_API Edge
{
  public:
    Edge();
    Edge(int *);
    void operator=(const Edge &rhs);
    bool operator==(const Edge &rhs);
    static unsigned int HashFunction(Edge &edge);
  private:
    int a,b;
};


// ----------------------------------------------------------------------------
//                             Inline Methods
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Default Constructor:  Edge::Edge
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline
Edge::Edge()
{
    a=-1;
    b=-1;
}

// ****************************************************************************
//  Constructor:  Edge::Edge
//
//  Purpose:
//    Create yourself for a vtk edge
//
//  Arguments:
//    cell       the vtk edge
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Oct  8 16:29:24 PDT 2002
//    Removed costly VTK calls.
//
// ****************************************************************************
inline
Edge::Edge(int *edges)
{
    a = INT_MAX;
    b = INT_MAX;

    for (int i=0; i<2; i++)
    {
        int node = edges[i];
        if (node < a)     { b=a; a=node; }
        else if (node < b)     { b=node; }
    }
}

// ****************************************************************************
//  Method:  Edge::operator=
//
//  Purpose:
//    Assignment operator
//
//  Arguments:
//    rhs        the source edge
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline void
Edge::operator=(const Edge &rhs)
{
    a=rhs.a;
    b=rhs.b;
}

// ****************************************************************************
//  Method:  Edge::operator==
//
//  Purpose:
//    comparison operator
//
//  Arguments:
//    rhs        the edge to compare with
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline bool
Edge::operator==(const Edge &rhs)
{
    return (a==rhs.a && b==rhs.b);
}



#endif
