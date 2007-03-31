#ifndef MATERIAL_TRIANGLE_H
#define MATERIAL_TRIANGLE_H

#include <MIROptions.h>
#include <vector>

// ****************************************************************************
//  Class:  MaterialTriangle
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
struct MaterialTriangle
{
    struct Node
    {
        int                   origindex;
        float                 weight[MAX_NODES_PER_POLY];
        std::vector<float>    matvf;
      public:
        Node() 
        {
            reset();
        }
        void reset()
        {
            origindex=-1;
            for (int i=0; i<MAX_NODES_PER_POLY; i++)
                weight[i]=0.0; 
        }
    };

    int  nmat;
    Node node[3];

  public:
    MaterialTriangle();
    MaterialTriangle(int);
    MaterialTriangle(const MaterialTriangle&);
    void reset()
    {
        node[0].reset();
        node[1].reset();
        node[2].reset();
    }
};

#endif
