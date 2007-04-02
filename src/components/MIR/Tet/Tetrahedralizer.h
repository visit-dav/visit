/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef TETRAHEDRALIZER_H
#define TETRAHEDRALIZER_H
#include <mir_exports.h>

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
class MIR_API Tetrahedralizer
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
