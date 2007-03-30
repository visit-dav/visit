#ifndef TETRAHEDRALIZER_H
#define TETRAHEDRALIZER_H
#include <database_exports.h>

#include <vector>
#include <MIROptions.h>
#include "MaterialTetrahedron.h"


// ****************************************************************************
//  Class:  Tetrahedralizer
//
//  Purpose:
//    Given a vtkCell, a subdivisionlevel, and volume fractions for the cell's
//    zone, nodes, faces, and edges, create a tetrahedralization for the cell.
//
//  Note:   
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//    Jeremy Meredith, Thu May 31 17:01:03 PDT 2001
//    Made the work routine a method, not the constructor.
//
//    Jeremy Meredith, Fri Aug 30 17:08:33 PDT 2002
//    Moved nmat initialization into constructor and made the output
//    tets a normal array.
//
//    Hank Childs, Mon Oct  7 17:42:02 PDT 2002
//    Remove costly VTK calls.
//
//    Jeremy Meredith, Tue Jan 14 14:42:04 PST 2003
//    Split 
//
// ****************************************************************************
class DATABASE_API Tetrahedralizer
{
  public:
    Tetrahedralizer(int nmat_);
    void Tetrahedralize(MIROptions::SubdivisionLevel, int, int, const int *,
                        std::vector<float>,
                        std::vector<float>*,
                        std::vector<float>*,
                        std::vector<float>*);

    int GetNumberOfTets() {return ntet;};
    MaterialTetrahedron &GetTet(int t) {return tet[t];}

  public:
    static int GetLowTetNodesForTet(int, const int *, int *);
    static int GetLowTetNodesForHex(int, const int *, int *);
    static int GetLowTetNodesForWdg(int, const int *, int *);
    static int GetLowTetNodesForPyr(int, const int *, int *);
    static int GetLowTetNodesForVox(int, const int *, int *);

  private:
    void calc_hex_low();
    void calc_tet_low();
    void calc_wdg_low();
    void calc_pyr_low();
    void calc_vox_low();
    void fill_tets_low();

  private:
    int         subdiv;
    int         nnodes;
    const int  *nodes;

    int nmat;
    std::vector<float>   vf_zone;
    std::vector<float>  *vf_node;
    std::vector<float>  *vf_face;
    std::vector<float>  *vf_edge;

    int ntet;
    MaterialTetrahedron tet[MAX_TETS_PER_CELL];

    int tetnodes_low[4*MAX_TETS_PER_CELL];
};

#endif
