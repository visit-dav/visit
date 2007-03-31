#ifndef TRIANGULATOR_H
#define TRIANGULATOR_H
#include <mir_exports.h>

#include <vector>
#include <MIROptions.h>
#include "MaterialTriangle.h"

// ****************************************************************************
//  Class:  Triangulator
//
//  Purpose:
//    Given a vtkCell, a subdivisionlevel, and volume fractions for the cell's
//    zone, nodes, and edges, create a triangulation for the cell.
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
//    tris a normal array.
//
// ****************************************************************************
class MIR_API Triangulator
{
  public:
    Triangulator(int nmat_);
    void Triangulate(MIROptions::SubdivisionLevel, int, int, const int *,
                     std::vector<float>,
                     std::vector<float>*,
                     std::vector<float>*);

    int GetNumberOfTris() {return ntri;}
    MaterialTriangle &GetTri(int t) {return tri[t];}

  private:
    void calc_poly_low();
    void calc_poly_med();
    void calc_poly_high();
    void calc_pixel_low();
    void calc_pixel_med();
    void calc_pixel_high();

  private:
    int        subdiv;
    int        celltype;
    int        npts;
    const int *c_ptr;

    int nmat;
    std::vector<float>   vf_zone;
    std::vector<float>  *vf_node;
    std::vector<float>  *vf_edge;

    int ntri;
    MaterialTriangle tri[MAX_TRIS_PER_CELL];
};

#endif
