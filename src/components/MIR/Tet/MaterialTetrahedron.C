#include "MaterialTetrahedron.h"

// ****************************************************************************
//  Default Constructor:  MaterialTetrahedron::MaterialTetrahedron
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 11, 2000
//
// ****************************************************************************
MaterialTetrahedron::MaterialTetrahedron()
{
}

// ****************************************************************************
//  Constructor:  MaterialTetrahedron::MaterialTetrahedron
//
//  Purpose:
//    Initialize the object and make sure there is enough room at each
//    node to store the v.f.'s.
//
//  Arguments:
//    nmat_      the number of materials
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 11, 2000
//
// ****************************************************************************
MaterialTetrahedron::MaterialTetrahedron(int nmat_)
{
    nmat = nmat_;
    node[0].matvf.resize(nmat);
    node[1].matvf.resize(nmat);
    node[2].matvf.resize(nmat);
    node[3].matvf.resize(nmat);
}

// ****************************************************************************
//  Copy Constructor:  MaterialTetrahedron::MaterialTetrahedron
//
//  Arguments:
//    rhs        the source MaterialTetrahedron
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 11, 2000
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 18 11:54:51 PDT 2001
//    Made it copy the origindex.
//
// ****************************************************************************
MaterialTetrahedron::MaterialTetrahedron(const MaterialTetrahedron &rhs)
{
    for (int n=0; n<4; n++)
    {
        node[n].origindex = rhs.node[n].origindex;
        node[n].matvf     = rhs.node[n].matvf;
        for (int i=0; i<MAX_NODES_PER_ZONE; i++)
            node[n].weight[i] = rhs.node[n].weight[i];
    }
}
