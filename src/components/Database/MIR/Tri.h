#ifndef TRI_H
#define TRI_H

#include "MaterialTriangle.h"

// ****************************************************************************
//  Class:  Tri
//
//  Purpose:
//    A triangle used in the intermediate stages of MIR.
//
//  Note:   
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//    Jeremy Meredith, Wed Feb 14 13:50:57 PST 2001
//    Added an index to the node so that each coordinate could be indexed
//    before interface reconstruction was complete.  Also, changed Node
//    interpolation constructor to use doubles for calculation.
//
// ****************************************************************************
struct Tri
{
    struct Node
    {
        float vf;
        float weight[3];
        int   index;
      public:
        Node() {};
        Node(const Node&);
        Node(double, const Node&, const Node&);
    };

    Node  node[3];
    int   zone_num;
    int   mat;
  public:
    Tri() {};
    Tri(int c,const MaterialTriangle &t,int m);
    Tri(int c,const MaterialTriangle &t,int m,const Tri&);
    Tri(int,const Node&,const Node&,const Node&,int);
};


// ----------------------------------------------------------------------------
//                             Inline Methods
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Method:  Tri::Tri
//
//  Purpose:
//    Create this tetrahedron from material m of a MaterialTriangle.
//
//  Arguments:
//    c      the cell number
//    t      the composite MaterialTriangle
//    m      the material to extract
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 18 12:01:19 PDT 2001
//    Added setting of index from the MaterialTriangle's origindex
//    so we can avoid using the weights when possible.
//
// ****************************************************************************
inline
Tri::Tri(int c,const MaterialTriangle &t,int m)
{
    zone_num = c;
    mat = m;
    for (int n=0; n<3; n++)
    {
        node[n].vf = t.node[n].matvf[m];
        node[n].index = t.node[n].origindex;
        for (int w=0; w<3; w++)
            node[n].weight[w] = (w==n ? 1.0 : 0.0);
    }
}

// ****************************************************************************
//  Constructor:  Tri::Tri
//
//  Purpose:
//    Create this triangle from material m of a MaterialTriangle, but
//    conform it to another Tri.
//
//  Arguments:
//    c         the cell number
//    t         the composite MaterialTriangle
//    m         the material to extract
//    conform   the Tri to conform this new one to
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline
Tri::Tri(int c,const MaterialTriangle &t,int m, const Tri &conform)
{
    zone_num = c;
    mat = m;
    for (int n=0; n<3; n++)
    {
        node[n].vf = 0.;
        node[n].index = conform.node[n].index;
        for (int w=0; w<3; w++)
        {
            node[n].weight[w] = conform.node[n].weight[w];
            node[n].vf += node[n].weight[w] * t.node[w].matvf[m];
        }
    }
}

// ****************************************************************************
//  Constructor:  Tri::Tri
//
//  Purpose:
//    Create this triangle from its constituent parts.
//
//  Arguments:
//    c          the cell number
//    n1..n3     the nodes
//    m          the material number
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline
Tri::Tri(int c,
         const Node &n1,
         const Node &n2,
         const Node &n3,
         int m)
{
    node[0] = n1;
    node[1] = n2;
    node[2] = n3;
    zone_num = c;
    mat = m;
}

// ****************************************************************************
//  Copy Constructor:  Node::Node
//
//  Arguments:
//    rhs        the source Node
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//    Jeremy Meredith, Thu May 31 16:56:49 PDT 2001
//    Manually unrolled the loop.
//
// ****************************************************************************
inline
Tri::Node::Node(const Node &rhs)
{
    vf = rhs.vf;
    index = rhs.index;
    weight[0] = rhs.weight[0];
    weight[1] = rhs.weight[1];
    weight[2] = rhs.weight[2];
}

// ****************************************************************************
//  Constructor:  Node::Node
//
//  Purpose:
//    Create this node by interpolating t percent between n1 and n2.
//
//  Arguments:
//    t          percentage (in range of 0.0 to 1.0)
//    n1         resultant if t==0
//    n2         resultant if t==1
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline
Tri::Node::Node(double t, const Node &n1, const Node &n2)
{
    vf = float(double(n1.vf) + t*(double(n2.vf) - double(n1.vf)));
    index = -1;
    for (int w=0; w<3; w++)
        weight[w] = float(double(n1.weight[w]) + 
                          t*(double(n2.weight[w]) - double(n1.weight[w])));
}

#endif
