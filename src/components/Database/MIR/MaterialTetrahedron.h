#ifndef MATERIAL_TETRAHEDRON_H
#define MATERIAL_TETRAHEDRON_H

#include <MIROptions.h>
#include <vector>

// ****************************************************************************
//  Class:  MaterialTetrahedron
//
//  Purpose:
//    A triangle created from a cell, with VF's for every material at each node
//    Weights are percentages of each of the cell's original nodes.
//
//  Note:   
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 18 11:55:48 PDT 2001
//    Added an origindex field so we can avoid using the weights when possible.
//
//    Jeremy Meredith, Fri Aug 30 17:04:55 PDT 2002
//    Added a reset() method.
//
// ****************************************************************************
struct MaterialTetrahedron
{
    struct Node
    {
        int                   origindex;
        float                 weight[MAX_NODES_PER_ZONE];
        std::vector<float>    matvf;
      public:
        Node()
        {
            reset();
        }
        void reset()
        {
            origindex=-1;
            for (int i=0; i<MAX_NODES_PER_ZONE; i++)
                weight[i]=0.0;
        }
    };

    int  nmat;
    Node node[4];

  public:
    MaterialTetrahedron();
    MaterialTetrahedron(int);
    MaterialTetrahedron(const MaterialTetrahedron&);
    void reset()
    {
        node[0].reset();
        node[1].reset();
        node[2].reset();
        node[3].reset();
    }
};

#endif
