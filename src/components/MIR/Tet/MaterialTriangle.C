#include "MaterialTriangle.h"

// ****************************************************************************
//  Default Constructor:  MaterialTriangle::MaterialTriangle
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 11, 2000
//
// ****************************************************************************
MaterialTriangle::MaterialTriangle()
{
}

// ****************************************************************************
//  Constructor:  MaterialTriangle::MaterialTriangle
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
MaterialTriangle::MaterialTriangle(int nmat_)
{
    nmat = nmat_;
    node[0].matvf.resize(nmat);
    node[1].matvf.resize(nmat);
    node[2].matvf.resize(nmat);
}

// ****************************************************************************
//  Copy Constructor:  MaterialTriangle::MaterialTriangle
//
//  Arguments:
//    rhs        the soruce MaterialTriangle
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 11, 2000
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 18 11:54:51 PDT 2001
//    Made it copy the origindex.
//
// ****************************************************************************
MaterialTriangle::MaterialTriangle(const MaterialTriangle &rhs)
{
    for (int n=0; n<3; n++)
    {
        node[n].origindex = rhs.node[n].origindex;
        node[n].matvf     = rhs.node[n].matvf;
        for (int i=0; i<MAX_NODES_PER_POLY; i++)
            node[n].weight[i] = rhs.node[n].weight[i];
    }
}
