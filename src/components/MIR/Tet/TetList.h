#ifndef TETLIST_H
#define TETLIST_H
#include <mir_exports.h>

#include "Tet.h"
#include "VisItArray.h"
#include "mat3d_tet.h"

// ****************************************************************************
// ****************************************************************************
//                             class TetList
// ****************************************************************************
// ****************************************************************************

// ****************************************************************************
//  Class:  TetList
//
//  Purpose:
//    Encapsulation of a list of Tets.
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 14, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 14:09:13 PST 2001
//    Made it use a vector, since there is no current need to pop off the
//    front of the list.
//
//    Jeremy Meredith, Thu May 31 17:13:39 PDT 2001
//    Added the Clear method.
//
//    Jeremy Meredith, Mon Sep 17 15:33:35 PDT 2001
//    Made it use an Array instead of a vector, for faster push_backs.
//
//    Jeremy Meredith, Wed Dec 11 10:10:31 PST 2002
//    Added a "forced material" where if it is >=0, any added tet will have
//    the forced material instead of the normal requested material.
//
//    Mark C. Miller, Thu Feb  9 21:06:10 PST 2006
//    Renamed Array class to VisItArray to avoid name collisions with
//    third-party libs
//
// ****************************************************************************
class MIR_API TetList
{
  private:
    VisItArray<Tet> list;
    void operator=(const TetList &rhs) { };
  public:
    void       Clear()                 {list.clear();}
    bool       Empty()                 {return list.empty();}
    const int &Size()            const {return list.size();}
    Tet       &operator[](const int i) {return list[i];}
    void Add(const Tet&, int);
    void AddTet(int, int,   const Tet::Node&,const Tet::Node&,const Tet::Node&,const Tet::Node&, int);
    static void Swap(TetList &a, TetList &b) {VisItArray<Tet>::Swap(a.list, b.list);}
};

// ****************************************************************************
//  Method:  TetList::Add
//
//  Purpose:
//    Add a tet directly to the list.
//
//  Arguments:
//    t      : the Tet
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 14, 2001
//
// ****************************************************************************
void
TetList::Add(const Tet &t, int forcedMat)
{
    list.push_back(t);

    if (forcedMat >= 0)
        list[list.size()-1].mat = forcedMat;
}

// ****************************************************************************
//  Method:  TetList::AddTet
//
//  Purpose:
//    Construct a tet from its nodes and add it to the list.
//
//  Arguments:
//    id     : the cell id
//    mat    : the mat number
//    n0..n3 : the nodes of the tet
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 14, 2001
//
// ****************************************************************************
void
TetList::AddTet(int id,int mat,
                const Tet::Node &n0, const Tet::Node &n1, const Tet::Node &n2, const Tet::Node &n3,
                int forcedMat)
{
    if (forcedMat >= 0)
        mat = forcedMat;

    list.push_back(Tet(id, n0, n1, n2, n3, mat));
}

#endif
