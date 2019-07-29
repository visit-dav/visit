// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef TET_H
#define TET_H

#include "MaterialTetrahedron.h"

// ****************************************************************************
//  Class:  Tet
//
//  Purpose:
//    A tetrahedron used in the intermediate stages of MIR.
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
struct Tet
{
    struct Node
    {
        float vf;
        float weight[4];
        int   index;
      public:
        Node() {};
        Node(const Node&);
        Node(double, const Node&, const Node&);
    };

    Node  node[4];
    int   zone_num;
    int   mat;
  public:
    Tet() {};
    Tet(int c,const MaterialTetrahedron &t,int m);
    Tet(int c,const MaterialTetrahedron &t,int m,const Tet&);
    Tet(int,const Node&,const Node&,const Node&,const Node&,int);
};


// ----------------------------------------------------------------------------
//                             Inline Methods
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Constructor:  Tet::Tet
//
//  Purpose:
//    Create this tetrahedron from material m of a MaterialTetrahedron.
//
//  Arguments:
//    c      the cell number
//    t      the composite MaterialTetrahedron
//    m      the material to extract
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 18 12:01:19 PDT 2001
//    Added setting of index from the MaterialTetrahedron's origindex
//    so we can avoid using the weights when possible.
//
// ****************************************************************************
inline
Tet::Tet(int c,const MaterialTetrahedron &t,int m)
{
    zone_num = c;
    mat = m;
    for (int n=0; n<4; n++)
    {
        node[n].vf = t.node[n].matvf[m];
        node[n].index = t.node[n].origindex;
        for (int w=0; w<4; w++)
            node[n].weight[w] = (w==n ? 1.0 : 0.0);
    }
}

// ****************************************************************************
//  Constructor:  Tet::Tet
//
//  Purpose:
//    Create this tetrahedron from material m of a MaterialTetrahedron, but
//    conform it to another Tet.
//
//  Arguments:
//    c         the cell number
//    t         the composite MaterialTetrahedron
//    m         the material to extract
//    conform   the Tet to conform this new one to
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline
Tet::Tet(int c,const MaterialTetrahedron &t,int m, const Tet &conform)
{
    zone_num = c;
    mat = m;
    for (int n=0; n<4; n++)
    {
        node[n].vf = 0.;
        node[n].index = conform.node[n].index;
        for (int w=0; w<4; w++)
        {
            node[n].weight[w] = conform.node[n].weight[w];
            node[n].vf += node[n].weight[w] * t.node[w].matvf[m];
        }
    }
}

// ****************************************************************************
//  Constructor:  Tet::Tet
//
//  Purpose:
//    Create this tetrahedron from its constituent parts.
//
//  Arguments:
//    c          the cell number
//    n1..n4     the nodes
//    m          the material number
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline
Tet::Tet(int c,
         const Node &n1,
         const Node &n2,
         const Node &n3,
         const Node &n4,
         int m)
{
    node[0] = n1;
    node[1] = n2;
    node[2] = n3;
    node[3] = n4;
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
Tet::Node::Node(const Node &rhs)
{
    vf = rhs.vf;
    index = rhs.index;
    weight[0] = rhs.weight[0];
    weight[1] = rhs.weight[1];
    weight[2] = rhs.weight[2];
    weight[3] = rhs.weight[3];
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
Tet::Node::Node(double t, const Node &n1, const Node &n2)
{
    vf = float(double(n1.vf) + t*(double(n2.vf) - double(n1.vf)));
    index = -1;
    for (int w=0; w<4; w++)
        weight[w] = float(double(n1.weight[w]) + 
                          t*(double(n2.weight[w]) - double(n1.weight[w])));
}

#endif
