#ifndef WEDGE_H
#define WEDGE_H

#include "Tet.h"

// ****************************************************************************
//  Class:  Wedge
//
//  Purpose:
//    A wedge used in the intermediate stages of MIR.
//
//  Note:   
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 13, 2002
//
//  Modifications:
//
// ****************************************************************************
struct Wedge
{
    Tet::Node  node[6];
    int        zone_num;
    int        mat;
  public:
    Wedge() {};
    Wedge(int c,const MaterialTetrahedron &t,int m,const Wedge&);
    Wedge(int,
          const Tet::Node&,const Tet::Node&,const Tet::Node&,
          const Tet::Node&,const Tet::Node&,const Tet::Node&,int);
};


// ----------------------------------------------------------------------------
//                             Inline Methods
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Constructor:  Wedge::Wedge
//
//  Purpose:
//    Create this wedge from material m of a MaterialTetrahedron, but
//    conform it to another Wedge.
//
//  Arguments:
//    c         the cell number
//    t         the composite MaterialTetrahedron
//    m         the material to extract
//    conform   the Wedge to conform this new one to
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 13, 2002
//
// ****************************************************************************
inline
Wedge::Wedge(int c,const MaterialTetrahedron &t,int m, const Wedge &conform)
{
    zone_num = c;
    mat = m;
    for (int n=0; n<6; n++)
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
//  Constructor:  Wedge::Wedge
//
//  Purpose:
//    Create this wedge from its constituent parts.
//
//  Arguments:
//    c          the cell number
//    n1..n6     the nodes
//    m          the material number
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 13, 2002
//
// ****************************************************************************
inline
Wedge::Wedge(int c,
             const Tet::Node &n1,
             const Tet::Node &n2,
             const Tet::Node &n3,
             const Tet::Node &n4,
             const Tet::Node &n5,
             const Tet::Node &n6,
             int m)
{
    node[0] = n1;
    node[1] = n2;
    node[2] = n3;
    node[3] = n4;
    node[4] = n5;
    node[5] = n6;
    zone_num = c;
    mat = m;
}

#endif
