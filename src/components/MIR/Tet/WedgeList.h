#ifndef WEDGELIST_H
#define WEDGELIST_H
#include <mir_exports.h>

#include "Wedge.h"
#include "Array.h"
#include "mat3d_tet.h"

// ****************************************************************************
// ****************************************************************************
//                             class WedgeList
// ****************************************************************************
// ****************************************************************************

// ****************************************************************************
//  Class:  WedgeList
//
//  Purpose:
//    Encapsulation of a list of Wedges.
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 13, 2002
//
//  Modifications:
//
// ****************************************************************************
class MIR_API WedgeList
{
  private:
    Array<Wedge> list;
    void operator=(const WedgeList &rhs) { };
  public:
    void       Clear()                 {list.clear();}
    bool       Empty()                 {return list.empty();}
    const int &Size()            const {return list.size();}
    Wedge     &operator[](const int i) {return list[i];}
    void Add(const Wedge&, int);
    void AddWedge(int, int, const Tet::Node&,const Tet::Node&,const Tet::Node&,
                            const Tet::Node&,const Tet::Node&,const Tet::Node&, int);
    static void Swap(WedgeList &a, WedgeList &b) {Array<Wedge>::Swap(a.list, b.list);}
};

// ****************************************************************************
//  Method:  WedgeList::Add
//
//  Purpose:
//    Add a wedge directly to the list.
//
//  Arguments:
//    w      : the Wedge
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 13, 2002
//
// ****************************************************************************
void
WedgeList::Add(const Wedge &t, int forcedMat)
{
    list.push_back(t);

    if (forcedMat >= 0)
        list[list.size()-1].mat = forcedMat;
}

// ****************************************************************************
//  Method:  WedgeList::AddWedge
//
//  Purpose:
//    Construct a wedge from its nodes and add it to the list.
//
//  Arguments:
//    id     : the cell id
//    mat    : the mat number
//    n0..n5 : the nodes of the wedge
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 13, 2002
//
// ****************************************************************************
void
WedgeList::AddWedge(int id,int mat,
                    const Tet::Node &n0, const Tet::Node &n1, const Tet::Node &n2,
                    const Tet::Node &n3, const Tet::Node &n4, const Tet::Node &n5,
                    int forcedMat)
{
    if (forcedMat >= 0)
        mat = forcedMat;

    list.push_back(Wedge(id, n0, n1, n2, n3, n4, n5, mat));
}


#endif
