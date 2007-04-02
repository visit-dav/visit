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

#ifndef TETMIR_H
#define TETMIR_H

#include <mir_exports.h>

#include <MIR.h>

#include <MIRConnectivity.h>
#include <MIROptions.h>
#include <QuadraticHash.h>
#include <Tet.h>
#include <Tri.h>
#include <Wedge.h>

#include <vtkSystemIncludes.h>  // for vtkIdType

#include <vector>
#include <deque>

struct MaterialTetrahedron;
struct MaterialTriangle;
class TetList;
class WedgeList;
class TriList;
class vtkDataArray;
class vtkDataSet;
class vtkPoints;
class avtMixedVariable;
class avtMaterial;
class avtSpecies;

// ****************************************************************************
//  Class:  TetMIR
//
//  Purpose:
//    Encapsulation of material interface reconstruction
//
//  Note:   
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//    Jeremy Meredith, Fri Dec 15 17:31:07 PST 2000
//    Added capability to have multiple zone types in output mesh.
//    Added new->old node index mapping (when possible).
//
//    Jeremy Meredith, Wed Feb 14 14:12:14 PST 2001
//    Added method to index a Tri/Tet node.
//    Moved MergeXXXX functions inside the class.
//
//    Jeremy Meredith, Thu May 31 17:24:30 PDT 2001
//    Added pre-extracted coordinate arrays.
//    Added ReconstructCleanTet/Tri.
//
//    Jeremy Meredith, Tue Jul 24 14:17:01 PDT 2001
//    Added support for ghost zones and original zone numbers.
//
//    Jeremy Meredith, Mon Oct  1 11:36:33 PDT 2001
//    Added support for mixed variables.
//
//    Kathleen Bonnell, Thu Nov  8 09:20:50 PST 2001 
//    Made indexList a vector of vtkIdType to match VTK 4.0 API
//    requirements for cell insertion and retrieval of ids.
//
//    Eric Brugger, Tue Dec 11 11:56:13 PST 2001
//    I reduced the amount of temporary memory used.  I also modified the
//    MIR to leave all the cells in a single dataset so that internal
//    material boundaries would be eliminated.
//
//    Jeremy Meredith, Tue Dec 18 13:40:54 PST 2001
//    Added support for species selection.
//
//    Jeremy Meredith, Fri Dec 21 13:30:27 PST 2001
//    Added smoothing option.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Thu Jul  4 16:44:44 PDT 2002
//    Added GetDataset method.  Removed GetMesh, GetNodalVariable, 
//    GetZonalVariable, and GetMaterial.
//
//    Jeremy Meredith, Mon Aug 12 16:20:42 PDT 2002
//    Added ReconstructCleanCell.  Changed the way options work.
//    Added some reconstruction information flags about whether
//    all cells were reconstructed as clean or not.
//
//    Jeremy Meredith, Thu Aug 15 13:50:42 PDT 2002
//    Added ability to pack materials so we do not operate on those which
//    don't exist for the current domain.
//
//    Jeremy Meredith, Fri Oct  4 11:08:58 PDT 2002
//    Added count of real mixed and real clean zones.
//
//    Jeremy Meredith, Fri Oct 25 10:40:16 PDT 2002
//    Added clean zones only.
//
//    Jeremy Meredith, Wed Dec 11 10:08:31 PST 2002
//    Added a "forcedMaterial" option to the tet and tri merging routines.
//    This is to make the option that disables interface smoothing for clean
//    zones also disable smoothing for mixed zones, which is more correct.
//
//    Jeremy Meredith, Tue Jan 14 14:30:37 PST 2003
//    Added support for leaving pieces of split zones as wedges, only
//    tetrahedralizing the wedges if needed (i.e. to split again or if
//    they are degenerate).
//
//    Jeremy Meredith, Wed Aug 20 09:55:02 PDT 2003
//    Refactored much of this into a new base MIR class.
//
//    Jeremy Meredith, Mon Sep 15 09:51:19 PDT 2003
//    Did some more refactoring.
//
//    Brad Whitlock, Tue Sep 23 09:37:43 PDT 2003
//    Made ReconstructedCoord be a friend class so it builds on Windows.
//
//    Jeremy Meredith, Wed Oct 15 16:47:49 PDT 2003
//    Added space for a material to get passed in to GetDataset.
//
// ****************************************************************************
class MIR_API TetMIR : public MIR
{
  public:
    TetMIR();
    virtual ~TetMIR();
    
    // do the processing
    virtual bool Reconstruct3DMesh(vtkDataSet *, avtMaterial *);
    virtual bool Reconstruct2DMesh(vtkDataSet *, avtMaterial *);

    // material select everything -- all variables, the mesh, and the material
    // if requested.
    virtual vtkDataSet *GetDataset(std::vector<int>, vtkDataSet *, 
                                   std::vector<avtMixedVariable *>, bool,
                                   avtMaterial * = NULL);

    virtual bool SubdivisionOccurred()   { return !allClean; }
    virtual bool NotAllCellsSubdivided() { return someClean; }

    // This is the reconstructed output and supporting data structures
    struct ReconstructedCoord
    {
        double x,y,z;
        double weight[MAX_NODES_PER_ZONE];
        int origzone;
        int orignode;

        ReconstructedCoord() : x(0),y(0),z(0),orignode(-1) {};
        bool operator==(const ReconstructedCoord &c);
        static unsigned int HashFunction(ReconstructedCoord&);
    };

    struct ReconstructedZone
    {
        int origzone;
        int mat;
        int celltype;
        int nnodes;
        int startindex;
        int mix_index;
    };

  private:
    friend struct ReconstructedCoord;

    static float xGrid;
    static float yGrid;
    static float zGrid;

    bool         allClean;
    bool         someClean;

    vtkDataSet                             *mesh;
    vtkPoints                              *outPts;

    int                                     nrealclean;
    int                                     nrealmixed;

    QuadraticHash<ReconstructedCoord,int>  *coordsHash;
    std::vector<ReconstructedCoord>         coordsList;
    std::vector<ReconstructedZone>          zonesList;
    std::vector<vtkIdType>                  indexList;

    int                                     dimension;
    int                                     nMaterials;

    std::vector<int>                        mapMatToUsedMat;
    std::vector<int>                        mapUsedMatToMat;

    // These are the helper functions and supporting data structures
    void ReconstructCleanCell(int, int, int, const int *, int);
    bool ReconstructCleanMesh(vtkDataSet *, avtMaterial *, MIRConnectivity &);
    void ReconstructTet(int, int, const int *, const MaterialTetrahedron &,
                        const std::vector<float>&, int *, int);
    void ReconstructCleanTet(int, int, int, const int *, 
                             const MaterialTetrahedron &);
    void ReconstructTri(int, int, const int *, const MaterialTriangle &,
                        const std::vector<float>&, int *, int);
    void ReconstructCleanTri(int, int, int, const int *,
                             const MaterialTriangle &);

    void IndexTetNode(Tet::Node&, int, int, const int *,
                      const MaterialTetrahedron&);
    void IndexTriNode(Tri::Node&, int, int, const int *,
                      const MaterialTriangle&);

    void  MergeWedges(TetList&, WedgeList&, int, int, const int *,
                      const MaterialTetrahedron&,
                      const Wedge&, const Wedge&, int forcedMat);
    void  MergeTets(TetList&, WedgeList&, int, int, const int *,
                    const MaterialTetrahedron&,
                    const Tet&, const Tet&, int forcedMat);
    void  MergeTetsHelper(TetList&, WedgeList&, int, int, const int *, 
                          const MaterialTetrahedron&,
                          int*, const Tet&, const Tet&, int forcedMat);

    void  MergeTris(TriList&, int, int, const int *, const MaterialTriangle&,
                    const Tri&, const Tri&, int forcedMat);
    void  MergeTrisHelper(TriList&, int, int, const int *,
                          const MaterialTriangle&,
                          int*, const Tri&, const Tri&, int forcedMat);
};

#endif
