#include "TetMIR.h"

#include <stdio.h>
#include <limits.h>
#include <math.h>

#include <string>
using std::string;
#include <vector>
using std::vector;

#include <Tetrahedralizer.h>
#include <Triangulator.h>

#include <vtkCell.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkUnstructuredGrid.h>

#include <avtMixedVariable.h>
#include <avtMaterial.h>
#include <avtSpecies.h>

#include <QuadraticHash.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>
#include <TimingsManager.h>


#define STDMIN(A, B) (((A)<(B)) ? (A) : (B))
#define STDMAX(A, B) (((A)>(B)) ? (A) : (B))


#include "VisItArray.h"
#include "Tri.h"
#include "Tet.h"
#include "Wedge.h"
#include "Face.h"
#include "Edge.h"
#include "ZoneCntAndVF.h"
#include "TriList.h"
#include "TetList.h"
#include "WedgeList.h"
#include "mat3d_tet.h"

// ----------------------------------------------------------------------------
//                             Static Data
// ----------------------------------------------------------------------------
float TetMIR::xGrid=1e6;
float TetMIR::yGrid=1e6;
float TetMIR::zGrid=1e6;


// ----------------------------------------------------------------------------
//                               Typedefs
// ----------------------------------------------------------------------------
typedef QuadraticHash<Face,ZoneCntAndVF> FaceHash;
typedef QuadraticHash<Edge,ZoneCntAndVF> EdgeHash;
typedef vector<ZoneCntAndVF>             NodeList;
typedef vector<bool>                     ZoneCleanList;


// ----------------------------------------------------------------------------
//                            Static Prototypes
// ----------------------------------------------------------------------------
static double FindIntersect(double,double,double,double);

static FaceHash* CreateFaceHash(MIRConnectivity &, int,
                                unsigned int (*)(Face&));
static EdgeHash* CreateEdgeHash(MIRConnectivity &, int,
                                unsigned int (*)(Edge&));
static ZoneCleanList *CreateZoneCleanList(MIRConnectivity &, int,
                              avtMaterial *, int, int*, NodeList*, int&, int&);

static void SubsampleVFsAndCreateNodeList(MIRConnectivity &, int,
                              avtMaterial*,int*,NodeList*,FaceHash*,EdgeHash*);
static void ExtractCellVFs(int, int, const int *, int, int, float *,
                                NodeList*, FaceHash*, EdgeHash*,
                                vector<float>&,
                                vector<float>*,
                                vector<float>*,
                                vector<float>*);
static void SetUpCoords(vtkDataSet *, vector<TetMIR::ReconstructedCoord> &);
static void AddFaces(int, const int *, FaceHash *, int, int=0, int* =NULL, float* =NULL);
static void AddEdges(int, const int *, EdgeHash *, int, int=0, int* =NULL, float* =NULL);
static void AddNodes(int, const int *, NodeList *, int, int=0, int* =NULL, float* =NULL);

// ----------------------------------------------------------------------------
//                             Class Methods
// ----------------------------------------------------------------------------


// ****************************************************************************
// ****************************************************************************
//                        class TetMIR::ReconstructedCoord
// ****************************************************************************
// ****************************************************************************


// ****************************************************************************
//  Method:  ReconstructedCoord::HashFunction
//
//  Purpose:
//    Hash function for a ReconstructedCoord.  
//    Map it to a 1e6 x 1e6 x 1e6 grid and then hash the resulting ints.
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 14:06:04 PST 2001
//    Made xGrid be the inverse of the old xGrid, so we can now multiply.
//
// ****************************************************************************
unsigned int
TetMIR::ReconstructedCoord::HashFunction(TetMIR::ReconstructedCoord &c)
{
    int gridx         = int(c.x * xGrid);
    int gridy         = int(c.y * yGrid);
    int gridz         = int(c.z * zGrid);
    unsigned char *cx = (unsigned char *)&gridx;
    unsigned char *cy = (unsigned char *)&gridy;
    unsigned char *cz = (unsigned char *)&gridz;

    return ((cx[0]^cy[1]^cz[2])<<24) +
           ((cx[1]^cy[2]^cz[3])<<16) +
           ((cx[2]^cy[3]^cz[0])<<8 ) +
           ((cx[3]^cy[0]^cz[1])    );
}

// ****************************************************************************
//  Method:  ReconstructedCoord::operator==
//
//  Purpose:
//    Comparison operator for ReconstructedCoord.
//    Map it to a 1e6 x 1e6 x 1e6 grid and then compare the resulting ints.
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 14:06:04 PST 2001
//    Made xGrid be the inverse of the old xGrid, so we can now multiply.
//
// ****************************************************************************
bool
TetMIR::ReconstructedCoord::operator==(const ReconstructedCoord &c) 
{
    return ((int(x * xGrid) == int(c.x * xGrid)) &&
            (int(y * yGrid) == int(c.y * yGrid)) &&
            (int(z * zGrid) == int(c.z * zGrid)));
}


// ****************************************************************************
// ****************************************************************************
//                               class MIR
// ****************************************************************************
// ****************************************************************************


// ****************************************************************************
//  Default Constructor:  TetMIR::TetMIR
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Jul  4 20:18:02 PDT 2002
//    Initialize mesh.
//
//    Hank Childs, Tue Sep  3 10:00:23 PDT 2002
//    Initialize outPts.
//
// ****************************************************************************
TetMIR::TetMIR()
{
    mesh   = NULL;
    outPts = NULL;
    coordsHash = NULL;
}

// ****************************************************************************
//  Destructor:  TetMIR::~TetMIR
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Jul  4 20:18:02 PDT 2002
//    Destruct mesh.
//
//    Hank Childs, Tue Sep  3 10:00:23 PDT 2002
//    Destruct outPts.
//
//    Hank Childs, Thu Sep 26 08:26:35 PDT 2002
//    Clear out the STL objects -- this shouldn't have to be done, but memory
//    bloats if we don't do it.
//
// ****************************************************************************
TetMIR::~TetMIR()
{
    if (mesh != NULL)
    {
        mesh->Delete();
        mesh = NULL;
    }
    if (outPts != NULL)
    {
        outPts->Delete();
        outPts = NULL;
    }
    if (coordsHash != NULL)
    {
        delete coordsHash;
        coordsHash = NULL;
    }
    coordsList.clear();
    zonesList.clear();
    indexList.clear();
}

// ****************************************************************************
//  Method:  TetMIR::Reconstruct3DMesh
//
//  Purpose:
//    Main method for interface reconstruction in 3d.
//
//  Arguments:
//    mesh       the mesh
//    mat        the material
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//    Jeremy Meredith, Fri Dec 15 17:30:17 PST 2000
//    Added detection of clean-zone-only meshes.
//
//    Jeremy Meredith, Thu Feb 15 14:06:04 PST 2001
//    Made xGrid be the inverse of the old xGrid, so we can now multiply.
//
//    Hank Childs, Tue Apr 10 16:46:10 PDT 2001
//    Reduced number of virtual function calls.
//
//    Jeremy Meredith, Thu May 31 17:13:58 PDT 2001
//    Big speedups: 1) pre-extract coordinate arrays
//    2) perform "clean reconstruction" on truly clean zones
//    3) call Tetrahedralizer constructor only once
//
//    Jeremy Meredith, Tue Jul 24 15:00:03 PDT 2001
//    Added support to pass along ghost zone info and original cell numbers.
//
//    Jeremy Meredith, Tue Sep 18 11:52:18 PDT 2001
//    Made it add the original nodes to the reconstructed coordinates
//    because they will all have to be added eventually, but this lets us
//    avoid hashing for them.
//
//    Eric Brugger, Wed Sep 19 11:53:29 PDT 2001
//    I moved some vector resizes of vf_zone, vf_node, vf_face and vf_edge
//    out of ExtractCellVFs into this routine to eliminate continually
//    constructing and resizing them.
//
//    Jeremy Meredith, Mon Oct  1 12:05:05 PDT 2001
//    Added support for mixed variable info.
//
//    Eric Brugger, Mon Nov  5 13:34:52 PST 2001
//    Modified to always compile the timing code.
//
//    Eric Brugger, Tue Dec 11 12:04:40 PST 2001
//    Reduced the amount of temporary memory used.  Also modified the timing
//    code to output timing information when material selecting an all clean
//    zone mesh.
//
//    Jeremy Meredith, Mon Feb  4 13:58:20 PST 2002
//    Moved the code to set the dimension above ReconstructCleanMesh.
//
//    Jeremy Meredith, Wed Feb  6 11:45:55 PST 2002
//    Changed a couple deletes to delete[]s.
//
//    Hank Childs, Thu Jul  4 20:18:02 PDT 2002
//    Make sure we are always keeping track of a valid mesh.
//
//    Jeremy Meredith, Tue Aug 13 14:44:23 PDT 2002
//    Added code to keep clean zones as clean when possible.
//    Added code to keep track of how many cells were subdivided.
//
//    Jeremy Meredith, Thu Aug 15 13:47:07 PDT 2002
//    Added ability to pack material so we do not operate on those
//    which don't exist for the current domain.
//
//    Jeremy Meredith, Fri Aug 30 17:19:49 PDT 2002
//    The number of materials is now an argument to the tetrahedralizer
//    constructor.
//
//    Hank Childs, Tue Sep  3 10:40:45 PDT 2002
//    Invalidate the output points as well as the mesh if it is changed.
//
//    Jeremy Meredith, Fri Oct  4 17:25:32 PDT 2002
//    Added more timings.  Added better size estimate for quadratic hash.
//    Combined two functions into one for a speedup.
//
//    Hank Childs, Mon Oct  7 14:40:50 PDT 2002
//    Use a routine optimized for performance to create the coordinates list.
//
//    Hank Childs, Mon Oct  7 15:30:53 PDT 2002
//    Took the dataset out of its VTK constructs to improve performance.
//
//    Jeremy Meredith, Fri Jun 13 16:56:43 PDT 2003
//    Added clean zones only.
//
//    Jeremy Meredith, Wed Jul  2 17:16:52 PDT 2003
//    Made it extract the max material count for each zone (i.e. real+fake)
//    and use it in the "is really clean" determination.  The reason is that
//    even if one material is dominant at all nodes of a "clean" cell, the
//    cell may still need to be split in many cases where there are more than
//    three materials in an adjacent zone.
//
//    Jeremy Meredith, Wed Oct 15 16:47:49 PDT 2003
//    Removed support for clean-zones-only.  Leaving it in would have required
//    more maintenance to this class than we would like to support.
//
//    Hank Childs, Fri Jan 28 15:39:23 PST 2005
//    Use exception macros.
//
// ****************************************************************************
bool
TetMIR::Reconstruct3DMesh(vtkDataSet *mesh, avtMaterial *mat_orig)
{
    // check that Reconstruct hasn't already been called
    if (!coordsList.empty())
    {
        debug1 << "Reconstruct has already been called!";
        EXCEPTION0(ImproperUseException);
    }
    int timerHandle = visitTimer->StartTimer();
    allClean = false;
    someClean = false;

    // Set the dimensionality
    dimension   = 3;

    int timerHandle0 = visitTimer->StartTimer();
    MIRConnectivity conn;
    conn.SetUpConnectivity(mesh);
    visitTimer->StopTimer(timerHandle0, "MIR: Setting up connectivity array");

    // see if we can perform a clean-zone-only algorithm
    if (mat_orig->GetMixlen() <= 0 || mat_orig->GetMixMat() == NULL)
    {
        allClean = true;
        bool status = ReconstructCleanMesh(mesh, mat_orig, conn);

        visitTimer->StopTimer(timerHandle, "MIR: Reconstructing clean mesh");
        visitTimer->DumpTimings();

        return status;
    }

    // Pack the material
    avtMaterial *mat = mat_orig->CreatePackedMaterial();
    mapMatToUsedMat = mat_orig->GetMapMatToUsedMat();
    mapUsedMatToMat = mat_orig->GetMapUsedMatToMat();

    // Set some attributes
    nMaterials  = mat->GetNMaterials();

    MIROptions::SubdivisionLevel subdivisionLevel = options.subdivisionLevel;

    int nmat    = nMaterials;
    int nCells  = mesh->GetNumberOfCells();
    int nPoints = mesh->GetNumberOfPoints();

    // Store the mesh for use later.
    if (this->mesh != NULL)
    {
        this->mesh->Delete();
    }
    this->mesh = mesh;
    this->mesh->Register(NULL);
    if (outPts != NULL)
    {
        outPts->Delete();
        outPts = NULL;
    }

    // extract coordinate arrays
    int timerHandle1 = visitTimer->StartTimer();
    SetUpCoords(mesh, coordsList);
    visitTimer->StopTimer(timerHandle1, "MIR: Copying coordinate list");

    // create the faces' zonecnt/vf list
    FaceHash *face_hash = NULL;
    if (subdivisionLevel == MIROptions::Med || 
        subdivisionLevel == MIROptions::High)
    {
        int timerHandle2 = visitTimer->StartTimer();
        face_hash = CreateFaceHash(conn, nmat, Face::HashFunction);
        visitTimer->StopTimer(timerHandle2, "MIR: Creating face hash");
    }

    // create the edges' zonecnt/vf list
    EdgeHash *edge_hash = NULL;
    if (subdivisionLevel == MIROptions::High)
    {
        int timerHandle3 = visitTimer->StartTimer();
        edge_hash = CreateEdgeHash(conn, nmat, Edge::HashFunction);
        visitTimer->StopTimer(timerHandle3, "MIR: Creating edge hash");
    }

    // extract zonal v.f.s to nodes, faces, edges
    // create the nodes' zonecnt/vf list
    // get the (max) material count for each zone, including faked ones
    int           *mat_cnt   = new int[nCells];
    NodeList      *node_list = new NodeList(nPoints, ZoneCntAndVF(0,nmat));
    int timerHandle4 = visitTimer->StartTimer();
    SubsampleVFsAndCreateNodeList(conn, nPoints, mat, mat_cnt,
                                  node_list, face_hash, edge_hash);
    visitTimer->StopTimer(timerHandle4, "MIR: Subsampling VFs and creating node list");

    // count whether each zone is *truly* clean
    int timerHandle5 = visitTimer->StartTimer();
    ZoneCleanList *real_clean_zones = NULL;
    real_clean_zones = CreateZoneCleanList(conn, nPoints, mat, nmat, mat_cnt,
                                           node_list, nrealclean, nrealmixed);
    visitTimer->StopTimer(timerHandle5, "MIR: Creating zone clean list");

    // create the grid for the coordinate hash table
    float *bounds = mesh->GetBounds();
    xGrid = 1./((bounds[1]-bounds[0])/(1.0e6));
    yGrid = 1./((bounds[3]-bounds[2])/(1.0e6));
    zGrid = 1./((bounds[5]-bounds[4])/(1.0e6));
    coordsHash =
        new QuadraticHash<ReconstructedCoord,int>(nrealmixed*3,
                                                  ReconstructedCoord::HashFunction);

    // for each zone
    //   do each tet
    vector<float>  vf_zone;
    vector<float>  vf_node[MAX_NODES_PER_ZONE];
    vector<float>  vf_face[MAX_FACES_PER_ZONE];
    vector<float>  vf_edge[MAX_EDGES_PER_ZONE];

    vf_zone.resize(nmat);
    int i;
    for (i = 0; i < MAX_NODES_PER_ZONE; i++)
        vf_node[i].resize(nmat);
    for (i = 0; i < MAX_FACES_PER_ZONE; i++)
        vf_face[i].resize(nmat);
    for (i = 0; i < MAX_EDGES_PER_ZONE; i++)
        vf_edge[i].resize(nmat);

    int timerHandle6 = visitTimer->StartTimer();
    int *mix_index = new int[nmat];
    float *zone_vf = new float[nmat];
    Tetrahedralizer tetrahedralizer(nmat);
    const int *c_ptr = conn.connectivity;
    for (int c=0; c<nCells; c++,  c_ptr += (*c_ptr)+1)
    {
        bool clean       = (*real_clean_zones)[c];
        int  clean_matno = mat->GetMatlist()[c];

        if (options.leaveCleanZonesWhole && clean)
        {
            someClean = true;
            ReconstructCleanCell(clean_matno, c, *c_ptr, c_ptr+1,
                                 conn.celltype[c]);
            continue;
        }

        mat->ExtractCellMatInfo(c, zone_vf, mix_index);

        ExtractCellVFs(c, *c_ptr, c_ptr+1, conn.celltype[c], nmat,
                       zone_vf, node_list, face_hash, edge_hash,
                       vf_zone, vf_node, vf_face, vf_edge);

        tetrahedralizer.Tetrahedralize(subdivisionLevel, conn.celltype[c],
                          *c_ptr, c_ptr+1, vf_zone, vf_node, vf_face, vf_edge);
        for (int t=0; t<tetrahedralizer.GetNumberOfTets(); t++)
        {
            if (clean)
            {
                ReconstructCleanTet(clean_matno, c, *c_ptr, c_ptr+1,
                                    tetrahedralizer.GetTet(t));
            }
            else
            {
                ReconstructTet(c, *c_ptr, c_ptr+1, tetrahedralizer.GetTet(t),
                               vf_zone, mix_index, nmat);
            }
        }
    }
    visitTimer->StopTimer(timerHandle6, "MIR: Tetrahedron based cell reconstruction");

    delete[] zone_vf;
    delete[] mix_index;
    delete   coordsHash;
    coordsHash = NULL;
    delete   real_clean_zones;
    delete   edge_hash;
    delete   face_hash;
    node_list->clear();
    delete   node_list;
    delete   mat;
    delete[] mat_cnt;

    visitTimer->StopTimer(timerHandle, "MIR: Reconstructing mixed mesh");
    visitTimer->DumpTimings();

    return true;
}

// ****************************************************************************
//  Method:  TetMIR::Reconstruct2DMesh
//
//  Purpose:
//    Main method for interface reconstruction in 2d.
//
//  Arguments:
//    mesh       the mesh
//    mat        the material
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//    Jeremy Meredith, Fri Dec 15 17:30:17 PST 2000
//    Added detection of clean-zone-only meshes.
//
//    Jeremy Meredith, Thu Feb 15 14:06:04 PST 2001
//    Made xGrid be the inverse of the old xGrid, so we can now multiply.
//
//    Hank Childs, Tue Apr 10 16:46:10 PDT 2001
//    Reduced number of virtual function calls.
//
//    Jeremy Meredith, Thu May 31 17:13:58 PDT 2001
//    Big speedups: 1) pre-extract coordinate arrays
//    2) perform "clean reconstruction" on truly clean zones
//    3) call Triangulator constructor only once
//
//    Jeremy Meredith, Tue Jul 24 15:00:03 PDT 2001
//    Added support to pass along ghost zone info and original cell numbers.
//
//    Jeremy Meredith, Tue Sep 18 11:52:18 PDT 2001
//    Made it add the original nodes to the reconstructed coordinates
//    because they will all have to be added eventually, but this lets us
//    avoid hashing for them.
//
//    Eric Brugger, Wed Sep 19 11:53:29 PDT 2001
//    I moved some vector resizes of vf_zone, vf_node, vf_face and vf_edge
//    out of ExtractCellVFs into this routine to eliminate continually
//    constructing and resizing them.
//
//    Jeremy Meredith, Mon Oct  1 12:05:05 PDT 2001
//    Added support for mixed variable info.
//
//    Eric Brugger, Tue Dec 11 12:04:40 PST 2001
//    Reduced the amount of temporary memory used.  Also added timing code
//    to match the 3d version of this method.
//
//    Jeremy Meredith, Mon Feb  4 13:58:20 PST 2002
//    Added code to set this->mesh.
//    Moved the code to set the dimension above ReconstructCleanMesh.
//
//    Jeremy Meredith, Wed Feb  6 11:45:55 PST 2002
//    Changed a couple deletes to delete[]s.
//
//    Hank Childs, Thu Jul  4 20:18:02 PDT 2002
//    Make sure we are always keeping track of a valid mesh.
//
//    Jeremy Meredith, Tue Aug 13 14:44:23 PDT 2002
//    Added code to keep clean zones as clean when possible.
//    Added code to keep track of how many cells were subdivided.
//
//    Jeremy Meredith, Thu Aug 15 13:47:11 PDT 2002
//    Added ability to pack material so we do not operate on those
//    which don't exist for the current domain.
//
//    Jeremy Meredith, Fri Aug 30 17:19:49 PDT 2002
//    The number of materials is now an argument to the triangulator
//    constructor.
//
//    Hank Childs, Tue Sep  3 10:40:45 PDT 2002
//    Invalidate the output points as well as the mesh if it is changed.
//
//    Jeremy Meredith, Fri Oct  4 17:25:32 PDT 2002
//    Added more timings.  Added better size estimate for quadratic hash.
//    Combined two functions into one for a speedup.
//
//    Hank Childs, Mon Oct  7 14:40:50 PDT 2002
//    Use a routine optimized for performance to create the coordinates list.
//
//    Jeremy Meredith, Fri Jun 13 16:56:43 PDT 2003
//    Added clean zones only.
//
//    Jeremy Meredith, Wed Jul  2 17:18:10 PDT 2003
//    Made it extract the max material count for each zone (i.e. real+fake)
//    and use it in the "is really clean" determination.  The reason is that
//    even if one material is dominant at all nodes of a "clean" cell, the
//    cell may still need to be split in many cases where there are more than
//    three materials in an adjacent zone.
//
//    Jeremy Meredith, Wed Oct 15 16:49:47 PDT 2003
//    Removed support for clean-zones-only.  Leaving it in would have required
//    more maintenance to this class than we would like to support.
//
//    Hank Childs, Fri Jan 28 15:36:03 PST 2005
//    Use exception macros.
//
// ****************************************************************************
bool
TetMIR::Reconstruct2DMesh(vtkDataSet *mesh, avtMaterial *mat_orig)
{
    // check that Reconstruct hasn't already been called
    if (!coordsList.empty())
    {
        debug1 << "Reconstruct has already been called!";
        EXCEPTION0(ImproperUseException);
    }

    int timerHandle = visitTimer->StartTimer();
    allClean = false;
    someClean = false;

    // Set the dimensionality
    dimension   = 2;

    int timerHandle0 = visitTimer->StartTimer();
    MIRConnectivity conn;
    conn.SetUpConnectivity(mesh);
    visitTimer->StopTimer(timerHandle0, "MIR: Setting up connectivity array");

    // see if we can perform a clean-zone-only algorithm
    if (mat_orig->GetMixlen() <= 0 || mat_orig->GetMixMat() == NULL)
    {
        allClean = true;
        bool status = ReconstructCleanMesh(mesh, mat_orig, conn);

        visitTimer->StopTimer(timerHandle, "MIR: Reconstructing clean mesh");
        visitTimer->DumpTimings();

        return status;
    }

    // Pack the material
    avtMaterial *mat = mat_orig->CreatePackedMaterial();
    mapMatToUsedMat = mat_orig->GetMapMatToUsedMat();
    mapUsedMatToMat = mat_orig->GetMapUsedMatToMat();

    // Set some attributes
    nMaterials  = mat->GetNMaterials();

    MIROptions::SubdivisionLevel subdivisionLevel = options.subdivisionLevel;
    int nmat    = nMaterials;
    int nCells  = mesh->GetNumberOfCells();
    int nPoints = mesh->GetNumberOfPoints();

    // Store the mesh for use later.
    if (this->mesh != NULL)
    {
        this->mesh->Delete();
    }
    this->mesh = mesh;
    this->mesh->Register(NULL);
    if (outPts != NULL)
    {
        outPts->Delete();
        outPts = NULL;
    }

    // extract coordinate arrays
    int timerHandle1 = visitTimer->StartTimer();
    SetUpCoords(mesh, coordsList);
    visitTimer->StopTimer(timerHandle1, "MIR: Copying coordinate list");

    // create the edges' zonecnt/vf list
    EdgeHash *edge_hash = NULL;
    if (subdivisionLevel == MIROptions::High)
    {
        int timerHandle3 = visitTimer->StartTimer();
        edge_hash = CreateEdgeHash(conn, nmat, Edge::HashFunction);
        visitTimer->StopTimer(timerHandle3, "MIR: Creating edge hash");
    }

    // extract zonal v.f.s to nodes, faces, edges
    // create the nodes' zonecnt/vf list
    // get the (max) material count for each zone, including faked ones
    int           *mat_cnt   = new int[nCells];
    NodeList      *node_list = new NodeList(nPoints, ZoneCntAndVF(0,nmat));
    int timerHandle4 = visitTimer->StartTimer();
    SubsampleVFsAndCreateNodeList(conn, nPoints, mat, mat_cnt,
                                  node_list, NULL, edge_hash);
    visitTimer->StopTimer(timerHandle4, "MIR: Subsampling VFs and creating node list");

    // count whether each zone is *truly* clean
    int timerHandle5 = visitTimer->StartTimer();
    ZoneCleanList *real_clean_zones = NULL;
    real_clean_zones = CreateZoneCleanList(conn, nPoints, mat, nmat, mat_cnt,
                                           node_list, nrealclean, nrealmixed);
    visitTimer->StopTimer(timerHandle5, "MIR: Creating zone clean list");

    // create the grid for the coordinate hash table
    float *bounds = mesh->GetBounds();
    xGrid = 1./((bounds[1]-bounds[0])/(1.0e6));
    yGrid = 1./((bounds[3]-bounds[2])/(1.0e6));
    zGrid = 1;
    coordsHash =
        new QuadraticHash<ReconstructedCoord,int>(nrealmixed*3,
                                                  ReconstructedCoord::HashFunction);

    // for each zone
    //   do each triangle
    vector<float>  vf_zone;
    vector<float>  vf_node[MAX_NODES_PER_POLY];
    vector<float>  vf_edge[MAX_EDGES_PER_POLY];

    vf_zone.resize(nmat);
    int i;
    for (i = 0; i < MAX_NODES_PER_POLY; i++)
        vf_node[i].resize(nmat);
    for (i = 0; i < MAX_EDGES_PER_POLY; i++)
        vf_edge[i].resize(nmat);

    int timerHandle6 = visitTimer->StartTimer();
    int *mix_index = new int[nmat];
    float *zone_vf = new float[nmat];
    Triangulator triangulator(nmat);
    const int *c_ptr = conn.connectivity;
    for (int c=0; c<nCells; c++,  c_ptr += (*c_ptr)+1)
    {
        bool clean       = (*real_clean_zones)[c];
        int  clean_matno = mat->GetMatlist()[c];

        if (options.leaveCleanZonesWhole && clean)
        {
            someClean = true;
            ReconstructCleanCell(clean_matno, c, *c_ptr, c_ptr+1,
                                 conn.celltype[c]);
            continue;
        }

        mat->ExtractCellMatInfo(c, zone_vf, mix_index);

        ExtractCellVFs(c, *c_ptr, c_ptr+1, conn.celltype[c], nmat,
                       zone_vf, node_list, NULL, edge_hash,
                       vf_zone, vf_node, NULL, vf_edge);

        triangulator.Triangulate(subdivisionLevel, conn.celltype[c],
                                 *c_ptr, c_ptr+1, vf_zone, vf_node, vf_edge);

        for (int t=0; t<triangulator.GetNumberOfTris(); t++)
        {
            if (clean)
                ReconstructCleanTri(clean_matno, c, *c_ptr, c_ptr+1,
                                    triangulator.GetTri(t));
            else
                ReconstructTri(c, *c_ptr, c_ptr+1, triangulator.GetTri(t),
                               vf_zone, mix_index, nmat);
        }
    }
    visitTimer->StopTimer(timerHandle6,
                          "MIR: Triangle based cell reconstruction");

    delete[] zone_vf;
    delete[] mix_index;
    delete   coordsHash;
    coordsHash = NULL;
    delete   real_clean_zones;
    delete   edge_hash;
    node_list->clear();
    delete   node_list;
    delete   mat;
    delete[] mat_cnt;

    visitTimer->StopTimer(timerHandle, "MIR: Reconstructing mixed mesh");
    visitTimer->DumpTimings();

    return true;
}


// ****************************************************************************
//  Method:  TetMIR::ReconstructCleanMesh
//
//  Purpose:
//    Main loop for interface reconstruction for any clean mesh.
//
//  Arguments:
//    mesh       the mesh
//    mat        the material
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//    Hank Childs, Tue Apr 10 14:44:48 PDT 2001
//    Reduced number of virtual function calls.
//
//    Jeremy Meredith, Tue Jul 24 15:00:03 PDT 2001
//    Added support to pass along ghost zone info and original cell numbers.
//
//    Kathleen Bonnell, Thu Nov  8 09:20:50 PST 2001 
//    Changed ids to vtkIdType to match VTK 4.0 API.
//
//    Eric Brugger, Tue Dec 11 12:04:40 PST 2001
//    Reduced the amount of temporary memory used.
//
//    Jeremy Meredith, Mon Feb  4 14:07:49 PST 2002
//    Let the caller set the number of dimensions, materials.
//
//    Jeremy Meredith, Tue Aug 13 14:47:02 PDT 2002
//    Pulled the ReconstructedCoord out of the loop.
//
//    Jeremy Meredith, Thu Aug 15 13:47:48 PDT 2002
//    Added faking of packed materials.  It buys us nothing to do so
//    unless we are doing mixed reconstruction.
//
//    Jeremy Meredith, Thu Aug 15 18:11:59 PDT 2002
//    Added setting of nMaterials into here since it's not done before
//    calling this function anymore.
//
//    Hank Childs, Mon Oct  7 16:05:35 PDT 2002
//    Remove costly VTK calls.
//
// ****************************************************************************
bool
TetMIR::ReconstructCleanMesh(vtkDataSet *mesh, avtMaterial *mat,
                             MIRConnectivity &conn)
{
    // no need to pack, so fake that part
    nMaterials = mat->GetNMaterials();
    mapMatToUsedMat.resize(mat->GetNMaterials(), -1);
    mapUsedMatToMat.resize(mat->GetNMaterials(), -1);
    for (int m=0; m<mat->GetNMaterials(); m++)
    {
        mapMatToUsedMat[m] = m;
        mapUsedMatToMat[m] = m;
    }

    // extract coords
    SetUpCoords(mesh, coordsList);

    // extract cells
    int        nCells  = conn.ncells;
    const int *matlist = mat->GetMatlist();
    int *conn_ptr = conn.connectivity;
    zonesList.resize(nCells);
    for (int c=0; c<nCells; c++)
    {
        int        nIds = *conn_ptr;
        const int *ids  = conn_ptr+1;

        ReconstructedZone &zone = zonesList[c];
        zone.origzone   = c;
        zone.mat        = matlist[c];
        zone.celltype   = conn.celltype[c];
        zone.nnodes     = nIds;
        zone.startindex = indexList.size();
        zone.mix_index  = -1;

        for (int n=0; n<nIds; n++)
            indexList.push_back(ids[n]);
        conn_ptr += nIds+1;
    }

    return true;
}

// ****************************************************************************
//  Method: TetMIR::GetDataset
//
//  Purpose:
//      Get the reconstructured mesh (for possibly a subset of materials).
//      Also reconstruct all nodal and zonal variable, including mixed 
//      variables if appropriate.  Finally, add an array indicating material
//      number if requested.
//
//  Arguments:
//      mats      The materials request.
//      ds        The dataset before reconstruction.
//      mixvars   A vector of mixed variables.
//      doMats    True if we should create a zonal array of which material
//                each cell came from.
//
//  Returns:    The reconstructed mesh with the appropriate materials and all
//              of the variables (nodal and zonal) in tact.
//
//  Programmer: Hank Childs
//  Creation:   July 4, 2002
//
//  Modifications:
//    Jeremy Meredith, Thu Aug 15 13:49:34 PDT 2002
//    Added translation of the material numbers to/from the packed
//    representation used in the reconstruction.
//
//    Hank Childs, Tue Sep  3 10:01:27 PDT 2002
//    Store off the points structure because it remains the same across
//    repeated calls.
//
//    Jeremy Meredith, Fri Jun 13 16:56:43 PDT 2003
//    Added clean-zone-only support.
//
//    Hank Childs, Fri Sep 12 17:40:50 PDT 2003
//    Formally tell Subset array how many tuples it has.
//
//    Jeremy Meredith, Wed Oct 15 16:47:49 PDT 2003
//    Added space for a material to get passed.  This is not used here;
//    it is only used for the new ZooMIR algorithm with clean-zones-only.
//
//    Jeremy Meredith, Wed Oct 22 13:02:12 PDT 2003
//    Added a check to make sure the requested material is not the
//    "clean-zones-only" mixed material index before looking it up.
//
// ****************************************************************************

vtkDataSet *
TetMIR::GetDataset(vector<int> mats, vtkDataSet *ds, 
                   vector<avtMixedVariable *> mixvars, bool doMats,
                   avtMaterial *)
{
    int i, j, timerHandle = visitTimer->StartTimer();

    //
    // Start off by determining which materials we should reconstruct and
    // which we should leave out.
    //
    bool *matFlag = new bool[nMaterials];
    if (!mats.empty())
    {
        for (i = 0; i < nMaterials; i++)
            matFlag[i] = false;
        for (i = 0; i < mats.size(); i++)
        {
            int origmatno = mats[i];
            if (origmatno < mapMatToUsedMat.size())
            {
                int usedmatno = mapMatToUsedMat[origmatno];
                if (usedmatno != -1)
                    matFlag[usedmatno] = true;
            }
        }
    }
    else
    {
        for (i = 0; i < nMaterials; i++)
            matFlag[i] = true;
    }

    //
    // Now count up the total number of cells we will have.
    //
    int ntotalcells = zonesList.size();
    int ncells = 0;
    int *cellList = new int[ntotalcells];
    for (int c = 0; c < ntotalcells; c++)
    {
        if (zonesList[c].mat >= 0 &&
            matFlag[zonesList[c].mat])
        {
            cellList[ncells] = c;
            ncells++;
        }
    }

    //
    // Instantiate the output dataset (-> VTK magic).
    //
    vtkUnstructuredGrid   *rv  = vtkUnstructuredGrid::New();
    rv->GetFieldData()->PassData(ds->GetFieldData());

    //
    // Set up the coordinate array.
    //
    int npoints = coordsList.size();
    if (outPts == NULL)
    {
        outPts = vtkPoints::New();
        outPts->SetNumberOfPoints(npoints);
        float *pts_buff = (float *) outPts->GetVoidPointer(0);
        for (i=0; i<npoints; i++)
        {
            pts_buff[3*i+0] = coordsList[i].x;
            pts_buff[3*i+1] = coordsList[i].y;
            pts_buff[3*i+2] = coordsList[i].z;
        }
    }
    rv->SetPoints(outPts);

    //
    // Now insert the connectivity array.
    //
    rv->Allocate(ncells);
    for (i=0; i<ncells; i++)
    {
        int c = cellList[i];
        rv->InsertNextCell(zonesList[c].celltype, zonesList[c].nnodes,
                           &indexList[zonesList[c].startindex]);
    }

    //
    // Copy over all node-centered data.
    //
    vtkPointData *outpd = rv->GetPointData();
    vtkPointData *inpd  = ds->GetPointData();
    if (inpd->GetNumberOfArrays() > 0)
    {
        outpd->CopyAllocate(inpd, npoints);
        for (i=0; i<npoints; i++)
        {
            //
            // For each point, copy over the original point data if the
            // reconstructed coordinate corresponds to a point in the dataset.
            // If the point is new, interpolate the values from the zone it
            // comes from.
            //
            ReconstructedCoord &coord = coordsList[i];
            if (coord.orignode >= 0)
            {
                outpd->CopyData(inpd, coord.orignode, i);
            }
            else
            {
                vtkCell *cell = mesh->GetCell(coord.origzone);
                outpd->InterpolatePoint(inpd, i, cell->GetPointIds(),
                                        coord.weight);
            }
        }
    }

    //
    // Copy over all the cell-centered data.  The logic gets awfully confusing
    // when using the VTK convenience methods *and* we have mixed variables,
    // so just do as normal and we will copy over the mixed values later.
    //
    vtkCellData *outcd = rv->GetCellData();
    vtkCellData *incd  = ds->GetCellData();
    if (incd->GetNumberOfArrays() > 0)
    {
        outcd->CopyAllocate(incd, ncells);
        for (i=0; i<ncells; i++)
        {
            int c = cellList[i];
            int origzone = zonesList[c].origzone;
            outcd->CopyData(incd, origzone, i);
        }
    }

    //
    // Now go and write over the mixed part of the mixed variables.  The non-
    // mixed part was already copied over in the last operation.
    //
    for (i=0; i<mixvars.size(); i++)
    {
        avtMixedVariable *mv = mixvars[i];
        if (mv == NULL)
        {
            continue;
        }
        vtkDataArray *arr = outcd->GetArray(mv->GetVarname().c_str());
        if (arr == NULL)
        {
            debug1 << "INTERNAL ERROR IN MIR.  Asked to reconstruct a variable"
                   << " with mixed elements,\nbut could not find the original "
                   << "variable array." << endl;
            debug1 << "The mixed variable is " << mv->GetVarname().c_str() << endl;
            debug1 << "Variables in the VTK dataset are: ";
            for (j = 0 ; j < outcd->GetNumberOfArrays() ; j++)
            {
                debug1 << outcd->GetArray(j)->GetName() << ", ";
            }
            debug1 << endl;
            continue;
        }
        if (arr->GetNumberOfComponents() != 1)
        {
            debug1 << "Can not operate on mixed vars that aren't scalars."
                   << endl;
            continue;
        }
        const float *buffer = mv->GetBuffer();
        float *outBuff = (float *) arr->GetVoidPointer(0);
        debug4 << "Overwriting mixed values for " << arr->GetName() << endl;
        int nvals = 0;
        for (j=0; j<ncells; j++)
        {
            int mix_index = zonesList[cellList[j]].mix_index;
            if (mix_index >= 0)
            {
                outBuff[j] = buffer[mix_index];
                nvals++;
            }
        }
        debug4 << "Overwrote " << nvals << " values (by tet, not necessarily "
               << "by original zone)" << endl;
    }

    if (doMats)
    {
        //
        // Add an array that contains the material for each zone (which is now
        // clean after reconstruction).
        //
        vtkIntArray *outmat = vtkIntArray::New();
        outmat->SetName("avtSubsets");
        outmat->SetNumberOfTuples(ncells);
        int *buff = outmat->GetPointer(0);
        for (i=0; i<ncells; i++)
        {
            buff[i] = mapUsedMatToMat[zonesList[cellList[i]].mat];
        }
        rv->GetCellData()->AddArray(outmat);
        outmat->Delete();
    }

    delete [] matFlag;
    delete [] cellList;

    visitTimer->StopTimer(timerHandle, "MIR: Getting clean dataset");
    visitTimer->DumpTimings();

    return rv;
}


// ****************************************************************************
//  Method:  TetMIR::IndexTetNode
//
//  Purpose:
//    1) Create a coordinate value from the weights of the node
//    2) Insert this coordinate in the list if it is unique
//       else find a matching coordinate in the list
//    3) Store the index to this coordinate in the node.
//
//  Arguments:
//    node   : the node to index
//    c      : the current cellid
//    cell   : the current cell
//    mattet : the current mattet
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 13, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 14:09:13 PST 2001
//    Small speed improvements.
//
//    Jeremy Meredith, Thu May 31 17:16:15 PDT 2001
//    Use the pre-extracted coordinate arrays.  Speed improvement.
//
//    Jeremy Meredith, Tue Sep 18 11:50:31 PDT 2001
//    Made it check to see if a node was already indexed.
//
//    Eric Brugger, Tue Dec 11 12:04:40 PST 2001
//    Reduced the amount of temporary memory used.
//
//    Jeremy Meredith, Fri Feb 15 13:42:03 PST 2002
//    Went to double precision in one critical spot.
//
//    Jeremy Meredith, Fri Aug 30 18:02:46 PDT 2002
//    Made use of the pre-extracted coordinates for a big speedup.
//
//    Hank Childs, Mon Oct  7 17:43:41 PDT 2002 
//    Remove costly VTK calls.
//
// ****************************************************************************
void
TetMIR::IndexTetNode(Tet::Node &node, int c, int npts, const int *c_ptr,
                  const MaterialTetrahedron &mattet)
{
    if (node.index != -1)
        return;

    // create original node weights from the weight of the mattet nodes
    ReconstructedCoord coord;
    int w;
    for (w=0; w<npts; w++)
        coord.weight[w] = 
            (node.weight[0] * mattet.node[0].weight[w]) +
            (node.weight[1] * mattet.node[1].weight[w]) +
            (node.weight[2] * mattet.node[2].weight[w]) +
            (node.weight[3] * mattet.node[3].weight[w]);

    for (w=0; w<npts; w++)
    {
        int id = c_ptr[w];
        coord.x += float(double(coordsList[id].x) * double(coord.weight[w]));
        coord.y += float(double(coordsList[id].y) * double(coord.weight[w]));
        coord.z += float(double(coordsList[id].z) * double(coord.weight[w]));
    }

    // find the old index or create a new one
    int index;
    if (coordsHash->Find(coord))
    {
        index = coordsHash->Get();
    }
    else
    {
        index = coordsList.size();
        coord.origzone = c;
        coordsHash->Insert(index);
        coordsList.push_back(coord);
    }
    node.index = index;
}


// ****************************************************************************
//  Method:  TetMIR::IndexTriNode
//
//  Purpose:
//    1) Create a coordinate value from the weights of the node
//    2) Insert this coordinate in the list if it is unique
//       else find a matching coordinate in the list
//    3) Store the index to this coordinate in the node.
//
//  Arguments:
//    node   : the node to index
//    c      : the current cellid
//    cell   : the current cell
//    mattri : the current mattri
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 13, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 14:09:13 PST 2001
//    Small speed improvements.
//
//    Jeremy Meredith, Thu May 31 17:16:15 PDT 2001
//    Use the pre-extracted coordinate arrays.  Speed improvement.
//
//    Jeremy Meredith, Tue Sep 18 11:50:31 PDT 2001
//    Made it check to see if a node was already indexed.
//
//    Eric Brugger, Tue Dec 11 12:04:40 PST 2001
//    Reduced the amount of temporary memory used.
//
//    Jeremy Meredith, Fri Feb 15 13:41:48 PST 2002
//    Went to double precision in one critical spot.
//
//    Jeremy Meredith, Fri Aug 30 18:02:46 PDT 2002
//    Made use of the pre-extracted coordinates for a big speedup.
//
//    Hank Childs, Mon Oct  7 17:43:41 PDT 2002 
//    Remove costly VTK calls.
//
// ****************************************************************************
void
TetMIR::IndexTriNode(Tri::Node &node, int c, int npts, const int *c_ptr,
                  const MaterialTriangle &mattri)
{
    if (node.index != -1)
        return;

    // create original node weights from the weight of the mattet nodes
    ReconstructedCoord coord;
    int w;
    for (w=0; w<npts; w++)
        coord.weight[w] = 
            (node.weight[0] * mattri.node[0].weight[w]) +
            (node.weight[1] * mattri.node[1].weight[w]) +
            (node.weight[2] * mattri.node[2].weight[w]);

    for (w=0; w<npts; w++)
    {
        int id = c_ptr[w];
        coord.x += float(double(coordsList[id].x) * double(coord.weight[w]));
        coord.y += float(double(coordsList[id].y) * double(coord.weight[w]));
    }

    // find the old index or create a new one
    int index;
    if (coordsHash->Find(coord))
    {
        index = coordsHash->Get();
    }
    else
    {
        index = coordsList.size();
        coord.origzone = c;
        coordsHash->Insert(index);
        coordsList.push_back(coord);
    }
    node.index = index;
}


// ****************************************************************************
//  Method:  TetMIR::ReconstructCleanCell
//
//  Purpose:
//    Perform reconstruction on an entire known-clean cell.
//
//  Arguments:
//    matno    the clean material number
//    c        the cell number
//    cell     the cell
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2002
//
//  Modifications:
//
//    Hank Childs, Mon Oct  7 17:30:53 PDT 2002
//    Removed costly VTK calls.
//
// ****************************************************************************
void
TetMIR::ReconstructCleanCell(int matno, int c, int nIds, const int *ids,
                          int celltype)
{
    ReconstructedZone zone;
    zone.origzone   = c;
    zone.mat        = matno;
    zone.celltype   = celltype;
    zone.nnodes     = nIds;
    zone.startindex = indexList.size();
    zone.mix_index  = -1;
        
    zonesList.push_back(zone);

    for (int n=0; n<nIds; n++)
        indexList.push_back(ids[n]);
}


// ****************************************************************************
//  Method:  TetMIR::ReconstructTet
//
//  Purpose:
//    Perform reconstruction for a single tet.
//
//  Arguments:
//    c        the cell number
//    cell     the cell
//    mattet   the tet with all material information
//    vf       the vf's for this cell
//    nmat     the number of materials
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//    Jeremy Meredith, Fri Dec 15 17:31:07 PST 2000
//    Added capability to have multiple zone types in output mesh.
//
//    Jeremy Meredith, Wed Feb 14 14:28:31 PST 2001
//    Made it index the nodes during the reconstruction process, not
//    afterwards.  This allows consistent wedge tetrahedralization when
//    merging two tets.
//
//    Jeremy Meredith, Thu Feb 15 14:09:13 PST 2001
//    Small speed improvements.
//    Also, split clean zones as if they were mixed, then make sure
//    they wind up clean at the end.  This removes all internal polygons.
//
//    Jeremy Meredith, Thu May 31 17:17:34 PDT 2001
//    Big speedup -- use static TetLists and Clear them every time instead
//    of reallocating a new one each time.
//
//    Jeremy Meredith, Wed Jun  6 22:44:20 PDT 2001
//    Added check to remove degenerate (3-unique-node) tetrahedrons.
//
//    Jeremy Meredith, Tue Sep 18 14:07:11 PDT 2001
//    Made it use TetList::Swap.  Possible speedups.
//
//    Eric Brugger, Tue Dec 11 12:04:40 PST 2001
//    Reduced the amount of temporary memory used and made some performance
//    improvements.
//
//    Jeremy Meredith, Fri Dec 21 13:30:27 PST 2001
//    Added smoothing option.
//
//    Jeremy Meredith, Tue Aug 13 14:46:47 PDT 2002
//    Changed the way options work.
//
//    Jeremy Meredith, Wed Dec 11 10:08:31 PST 2002
//    Changed the logic for providing correct connectivity and
//    allowing artificial smoothing of interfaces by allowing
//    materials to enter adjacent zones when it will look better.
//    This is to make the option that disables interface smoothing
//    for clean zones also disable smoothing for mixed zones, which
//    is the correct behavior.
//
//    Jeremy Meredith, Fri Dec 13 15:51:43 PST 2002
//    Changed the new two-pass algorithm back to a static ordering and added
//    logic to allow for any ordering of materials despite the change I made
//    two days ago.  A different ordering between neighboring cells could
//    cause different subdividisions, and thus internal polygons.
//
//    Jeremy Meredith, Tue Jan 14 14:34:17 PST 2003
//    Added support for leaving pices of split cells as wedges when possible.
//
// ****************************************************************************
void
TetMIR::ReconstructTet(int c, int npts, const int *c_ptr,
                     const MaterialTetrahedron &mattet,
                     const vector<float> &vf, int *mix_index, int nmat)
{
    static TetList tetlist;
    static WedgeList wedgelist;
    tetlist.Clear();
    wedgelist.Clear();

    //
    // Right now, we will perform the reconstruction on clean
    // zones as if they were mixed, then change the material
    // number when we put the new tets into the reconstructed mesh.
    //
    // This splits polygons appropriately where material interfaces
    // run directly into clean zones so that the duplicate polygons
    // can be removed.
    //
    // This also allows artificial smoothing of materials, e.g.
    // allowing interfaces in clean zones where it looks better.
    //
    // If we want to go back to the old way, set this to false.
    //
    bool need_good_connectivity = true;

    // create the reconstructed tets
    for (int m=0; m<nmat; m++)
    {
        bool newmat_is_fake = (vf[m] == 0);

        // We only need to worry about materials which are not "supposed"
        // to be in this zone but still show up if we are concerned about
        // getting good connectivity or smoothing material interfaces
        if (!need_good_connectivity && newmat_is_fake)
        {
            continue;
        }

        //
        // If the volume fraction at all the nodes for the current material
        // are all 0, then we can skip it.  This may not hold true if we
        // change to an iteration approach.
        //
        if (mattet.node[0].matvf[m] == 0. && mattet.node[1].matvf[m] == 0. &&
            mattet.node[2].matvf[m] == 0. && mattet.node[3].matvf[m] == 0.)
            continue;

        if (tetlist.Empty() && wedgelist.Empty())
        {
            Tet tet(c,mattet,m);
            for (int n=0; n<4; n++)
                IndexTetNode(tet.node[n], c, npts, c_ptr, mattet);

            tetlist.Add(tet,-1);
        }
        else
        {
            static TetList new_tetlist;
            static WedgeList new_wedgelist;
            new_tetlist.Clear();
            new_wedgelist.Clear();

            // Merge onto the wedges
            int nwedge = wedgelist.Size();
            for (int w=0; w<nwedge; w++)
            {
                int forcedMat = -1;
                if (!options.smoothing)
                {
                    bool oldmat_is_fake = (vf[wedgelist[w].mat] == 0);
                    // If we're overlaying a real mat on a fake mat, force new
                    // If we're overlaying a fake mat on a real mat, force old
                    if      (newmat_is_fake && !oldmat_is_fake)
                        forcedMat = wedgelist[w].mat;
                    else if (!newmat_is_fake && oldmat_is_fake)
                        forcedMat = m;
                }

                MergeWedges(new_tetlist, new_wedgelist, c, npts, c_ptr, mattet,
                            wedgelist[w], Wedge(c,mattet,m,wedgelist[w]),
                            forcedMat);
            }

            // Merge onto the tets
            int ntet = tetlist.Size();
            for (int t=0; t<ntet; t++)
            {
                int forcedMat = -1;
                if (!options.smoothing)
                {
                    bool oldmat_is_fake = (vf[tetlist[t].mat] == 0);
                    // If we're overlaying a real mat on a fake mat, force new
                    // If we're overlaying a fake mat on a real mat, force old
                    if      (newmat_is_fake && !oldmat_is_fake)
                        forcedMat = tetlist[t].mat;
                    else if (!newmat_is_fake && oldmat_is_fake)
                        forcedMat = m;
                }

                MergeTets(new_tetlist, new_wedgelist, c, npts, c_ptr, mattet,
                          tetlist[t], Tet(c,mattet,m,tetlist[t]),
                          forcedMat);
            }

            // Make the new lists the current ones
            WedgeList::Swap(wedgelist, new_wedgelist);
            TetList::Swap(tetlist, new_tetlist);
        }
    }

    // We've got the final lists now.

    // Add the wedges to the output mesh
    int t, nwedge = wedgelist.Size();
    for (t=0; t<nwedge; t++)
    {
        ReconstructedZone zone;
        zone.celltype   = VTK_WEDGE;
        zone.nnodes     = 6;
        zone.startindex = indexList.size();
        zone.mat        = wedgelist[t].mat;
        zone.origzone   = c;
        zone.mix_index  = mix_index[zone.mat];

        // Handle degenerate wedges here:
        if (wedgelist[t].node[0].index == wedgelist[t].node[1].index ||
            wedgelist[t].node[0].index == wedgelist[t].node[2].index ||
            wedgelist[t].node[0].index == wedgelist[t].node[3].index ||
            wedgelist[t].node[0].index == wedgelist[t].node[4].index ||
            wedgelist[t].node[0].index == wedgelist[t].node[5].index ||
            wedgelist[t].node[1].index == wedgelist[t].node[2].index ||
            wedgelist[t].node[1].index == wedgelist[t].node[3].index ||
            wedgelist[t].node[1].index == wedgelist[t].node[4].index ||
            wedgelist[t].node[1].index == wedgelist[t].node[5].index ||
            wedgelist[t].node[2].index == wedgelist[t].node[3].index ||
            wedgelist[t].node[2].index == wedgelist[t].node[4].index ||
            wedgelist[t].node[2].index == wedgelist[t].node[5].index ||
            wedgelist[t].node[3].index == wedgelist[t].node[4].index ||
            wedgelist[t].node[3].index == wedgelist[t].node[5].index ||
            wedgelist[t].node[4].index == wedgelist[t].node[5].index)
        {
            // We've got a degenerate wedge.  Split it into its tets now.
            Wedge &wedge = wedgelist[t];

            // get the tets
            int nodes[6] = {wedge.node[0].index,
                            wedge.node[1].index,
                            wedge.node[2].index,
                            wedge.node[3].index,
                            wedge.node[4].index,
                            wedge.node[5].index};

            static int indices[12];
            int ntets = Tetrahedralizer::GetLowTetNodesForWdg(6,nodes, indices);

            for (int t=0; t<ntets; t++)
            {
                // indices for the nodes of the new tet
                int ax = indices[t*4 + 0];
                int bx = indices[t*4 + 1];
                int cx = indices[t*4 + 2];
                int dx = indices[t*4 + 3];

                tetlist.AddTet(wedge.zone_num, wedge.mat,
                               wedge.node[ax],
                               wedge.node[bx],
                               wedge.node[cx],
                               wedge.node[dx],
                               -1);
            }
            
            continue;
        }

        // It's a good wedge; add it to the list
        zonesList.push_back(zone);

        for (int n=0; n<6; n++)
            indexList.push_back(wedgelist[t].node[n].index);
    }

    // Add the tets to the output mesh
    int ntet = tetlist.Size();
    for (t=0; t<ntet; t++)
    {
        ReconstructedZone zone;
        zone.celltype   = VTK_TETRA;
        zone.nnodes     = 4;
        zone.startindex = indexList.size();
        zone.mat        = tetlist[t].mat;
        zone.origzone   = c;
        zone.mix_index  = mix_index[zone.mat];

        // Handle degenerate tets here:
        if (tetlist[t].node[0].index == tetlist[t].node[1].index ||
            tetlist[t].node[0].index == tetlist[t].node[2].index ||
            tetlist[t].node[0].index == tetlist[t].node[3].index ||
            tetlist[t].node[1].index == tetlist[t].node[2].index ||
            tetlist[t].node[1].index == tetlist[t].node[3].index ||
            tetlist[t].node[2].index == tetlist[t].node[3].index)
        {
            // Just skip degenerate tets.
            continue;
        }

        // It's a good tet; add it to the list
        zonesList.push_back(zone);

        for (int n=0; n<4; n++)
            indexList.push_back(tetlist[t].node[n].index);
    }
}

// ****************************************************************************
//  Method:  TetMIR::ReconstructCleanTet
//
//  Purpose:
//    Perform reconstruction on a known-clean tetrahedron.
//
//  Arguments:
//    matno    the clean material number
//    c        the cell number
//    cell     the cell
//    mattet   the tet with all material information
//
//  Programmer:  Jeremy Meredith
//  Creation:    May 31, 2001
//
//  Modifications:
//    Eric Brugger, Tue Dec 11 12:04:40 PST 2001
//    Reduced the amount of temporary memory used and made some performance
//    improvements.
//
// ****************************************************************************
void
TetMIR::ReconstructCleanTet(int matno, int c, int npts, const int *c_ptr,
                         const MaterialTetrahedron &mattet)
{
    ReconstructedZone zone;
    zone.celltype   = VTK_TETRA;
    zone.nnodes     = 4;
    zone.startindex = indexList.size();
    zone.mat        = matno;
    zone.origzone   = c;
    zone.mix_index  = -1;
        
    zonesList.push_back(zone);

    //
    // The following preprocessor directive must be flipped when higher
    // subdivisision levels are implemented since the current code only
    // works with subdivision level low.
    //
#if 0
    Tet tet(c,mattet,matno);
    for (int n=0; n<4; n++)
        IndexTetNode(tet.node[n], c, npts, c_ptr, mattet);

    for (int n=0; n<4; n++)
        indexList.push_back(tet.node[n].index);
#else
    for (int n=0; n<4; n++)
        indexList.push_back(mattet.node[n].origindex);
#endif
}

// ****************************************************************************
//  Method:  TetMIR::ReconstructTri
//
//  Purpose:
//    Perform reconstruction for a single tri.
//
//  Arguments:
//    c        the cell number
//    cell     the cell
//    mattri   the tri with all material information
//    vf       the vf's for this cell
//    nmat     the number of materials
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//    Jeremy Meredith, Fri Dec 15 17:31:07 PST 2000
//    Added capability to have multiple zone types in output mesh.
//
//    Jeremy Meredith, Wed Feb 14 14:28:31 PST 2001
//    Made it index the nodes during the reconstruction process, not
//    afterwards.
//
//    Jeremy Meredith, Thu Feb 15 14:09:13 PST 2001
//    Small speed improvements.
//
//    Jeremy Meredith, Thu May 31 17:17:34 PDT 2001
//    Big speedup -- use static TriLists and Clear them every time instead
//    of reallocating a new one each time.
//
//    Jeremy Meredith, Tue Sep 18 14:07:11 PDT 2001
//    Made it use TriList::Swap.  Possible speedups.
//
//    Eric Brugger, Tue Dec 11 12:04:40 PST 2001
//    Reduced the amount of temporary memory used.
//
//    Jeremy Meredith, Wed Mar 13 15:10:57 PST 2002
//    Started splitting possibly-mixed clean zones like they were mixed.
//    This matches the way we have been doing it in 3D, and it became
//    necessary so that a feature edges filter would not find clean-mixed
//    boundary edges.
//
//    Jeremy Meredith, Tue Aug 13 14:46:39 PDT 2002
//    Changed the way options work.
//
//    Hank Childs, Tue Oct  8 15:50:04 PDT 2002
//    Removed costly VTK calls.
//
//    Jeremy Meredith, Wed Dec 11 10:08:31 PST 2002
//    Changed the logic for providing correct connectivity and
//    allowing artificial smoothing of interfaces by allowing
//    materials to enter adjacent zones when it will look better.
//    This is to make the option that disables interface smoothing
//    for clean zones also disable smoothing for mixed zones, which
//    is the correct behavior.
//
//    Jeremy Meredith, Fri Dec 13 15:51:43 PST 2002
//    Changed the new two-pass algorithm back to a static ordering and added
//    logic to allow for any ordering of materials despite the change I made
//    two days ago.  A different ordering between neighboring cells could
//    cause different subdividisions, and thus internal polygons.   While this
//    doesn't affect the 2D case, this keeps it aligned with the 3D case.
//
// ****************************************************************************
void
TetMIR::ReconstructTri(int c, int npts, const int *c_ptr,
                     const MaterialTriangle &mattri,
                     const vector<float> &vf, int *mix_index, int nmat)
{
    static TriList trilist;
    trilist.Clear();

    //
    // Right now, we will perform the reconstruction on clean
    // zones as if they were mixed, then change the material
    // number when we put the new tris into the reconstructed mesh.
    //
    // This splits polygons appropriately where material interfaces
    // run directly into clean zones so that the duplicate polygons
    // can be removed.
    //
    // This also allows artificial smoothing of materials, e.g.
    // allowing interfaces in clean zones where it looks better.
    //
    // If we want to go back to the old way, set this to false.
    //
    bool need_good_connectivity = true;

    for (int m=0; m<nmat; m++)
    {
        bool newmat_is_fake = (vf[m] == 0);

        // We only need to worry about materials which are not "supposed"
        // to be in this zone but still show up if we are concerned about
        // getting good connectivity or smoothing material interfaces
        if (!need_good_connectivity && newmat_is_fake)
        {
            continue;
        }

        //
        // If the volume fraction at all the nodes for the current material
        // are all 0. then we can skip it.  This may not hold true if we
        // change to an iteration approach.
        //
        if (mattri.node[0].matvf[m] == 0. &&
            mattri.node[1].matvf[m] == 0. &&
            mattri.node[2].matvf[m] == 0.)
            continue;

        if (trilist.Empty())
        {
            Tri tri(c,mattri,m);
            for (int n=0; n<3; n++)
                IndexTriNode(tri.node[n], c, npts, c_ptr, mattri);

            trilist.Add(tri,-1);
        }
        else
        {
            static TriList new_trilist;
            new_trilist.Clear();

            int ntri = trilist.Size();
            for (int t=0; t<ntri; t++)
            {
                int forcedMat = -1;
                if (!options.smoothing)
                {
                    bool oldmat_is_fake = (vf[trilist[t].mat] == 0);
                    // If we're overlaying a real mat on a fake mat, force new
                    // If we're overlaying a fake mat on a real mat, force old
                    if      (newmat_is_fake && !oldmat_is_fake)
                        forcedMat = trilist[t].mat;
                    else if (!newmat_is_fake && oldmat_is_fake)
                        forcedMat = m;
                }
                MergeTris(new_trilist, c, npts, c_ptr, mattri,
                          trilist[t], Tri(c,mattri,m,trilist[t]),
                          forcedMat);
            }

            TriList::Swap(trilist, new_trilist);
        }
    }

    // shove them into the reconstructed mesh
    int ntri = trilist.Size();
    for (int t=0; t<ntri; t++)
    {
        ReconstructedZone zone;
        zone.celltype   = VTK_TRIANGLE;
        zone.nnodes     = 3;
        zone.startindex = indexList.size();
        zone.mat        = trilist[t].mat;
        zone.origzone   = c;
        zone.mix_index  = mix_index[zone.mat];

        // Handle degenerate tris here:
        if (trilist[t].node[0].index == trilist[t].node[1].index ||
            trilist[t].node[0].index == trilist[t].node[2].index ||
            trilist[t].node[1].index == trilist[t].node[2].index)
        {
            continue;
        }

        zonesList.push_back(zone);

        for (int n=0; n<3; n++)
            indexList.push_back(trilist[t].node[n].index);
    }
}

// ****************************************************************************
//  Method:  TetMIR::ReconstructCleanTri
//
//  Purpose:
//    Perform reconstruction on a known-clean triangle.
//
//  Arguments:
//    matno    the clean material number
//    c        the cell number
//    cell     the cell
//    mattri   the tri with all material information
//
//  Programmer:  Jeremy Meredith
//  Creation:    May 31, 2001
//
//  Modifications:
//    Eric Brugger, Tue Dec 11 12:04:40 PST 2001
//    Reduced the amount of temporary memory used.
//
//    Hank Childs, Tue Oct  8 15:50:04 PDT 2002
//    Removed costly VTK calls.
//
// ****************************************************************************
void
TetMIR::ReconstructCleanTri(int matno, int c, int npts, const int *c_ptr,
                         const MaterialTriangle &mattri)
{
    Tri tri(c,mattri,matno);
    int n;
    for (n=0; n<3; n++)
        IndexTriNode(tri.node[n], c, npts, c_ptr, mattri);

    ReconstructedZone zone;
    zone.celltype   = VTK_TRIANGLE;
    zone.nnodes     = 3;
    zone.startindex = indexList.size();
    zone.mat        = matno;
    zone.origzone   = c;
    zone.mix_index  = -1;
        
    zonesList.push_back(zone);

    for (n=0; n<3; n++)
        indexList.push_back(tri.node[n].index);
}





// ----------------------------------------------------------------------------
//                            Static Functions
// ----------------------------------------------------------------------------





// ****************************************************************************
//  Method:  FindIntersect
//
//  Purpose:
//    Find the x-intersection betweens lines (0,a1)-(1,b1) and (0,a2)-(1,b2)
//
//  Arguments:
//    a1,b1      y coordinates of first  line at x(a)==0 and x(b)==1
//    a2,b2      y coordinates of second line at x(a)==0 and x(b)==1
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//    Jeremy Meredith, Wed Feb 14 14:30:00 PST 2001
//    Made it use all doubles.
//
//    Jeremy Meredith, Tue Jan 14 14:38:37 PST 2003
//    Simplified the logic.  The assertion was never called.
//
//    Hank Childs, Fri Jan 28 15:36:03 PST 2005
//    Use exception macros.
//
// ****************************************************************************
static double
FindIntersect(double a1, double b1, double a2, double b2)
{
    double s1=(b1-a1);
    double s2=(b2-a2);
    double c;

    if (s2==s1)       /* parallel */
        EXCEPTION0(VisItException)
    else if (a1==a2)  /* meet at c=0 */
        return 0;
    else if (b1==b2)  /* meet at c=1 */
        return 1;

    c = (a1 - a2) / (s2 - s1);

    return c;
}

// ****************************************************************************
//  Method:  MergeTetsHelper
//
//  Purpose:
//    merge tets when they need to be split
//
//  Arguments:
//    tetlist    the list to which to append the new tetrahedrons
//    c          the cell number
//    cell       the current cell
//    mattet     the current material tetrahedron
//    maxmat     maximum-vf material for each node
//    tet1,tet2  the two tets to merge
//
//  Note:        Assumes maxmat[0]==tet1.mat!
//               Also, see modification comment by JSM on 2/4/02
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//    Jeremy Meredith, Wed Feb 14 14:30:17 PST 2001
//    Made it index nodes during reconstruction.
//    Made it use doubles for intersection calculations.
//    Made it a class method.
//    Made it use a TetList to insert reconstructed shapes so that it could
//    handle proper tetrahedralization (ie diagonals from lowest index verts).
//
//    Jeremy Meredith, Mon Feb  4 14:08:43 PST 2002
//    Made the intersection points use the original index if the intersection
//    falls upon one of the original nodes.  Note -- a cleaner way to do this
//    may be, for example, to never claim that there is an intersection 
//    between nodes 0 and 1 if there is an intersection *at* node 0.  In this
//    case, maxmat[0] should not have been either m1 or m2, but some other
//    value to indicate that there is an intersection here.  This will,
//    however, expand the number of cases beyond 6 because we must separate
//    out the degenerate cases.  Also, the same repairs were not made to the
//    2D case yet because it is not critical.
//
//    Jeremy Meredith, Fri Feb 15 13:45:33 PST 2002
//    Moved some variables to a more local scope.
//
//    Jeremy Meredith, Wed Mar 13 15:10:34 PST 2002
//    Fixed node ordering on two cases.
//
//    Jeremy Meredith, Wed Dec 11 10:10:31 PST 2002
//    Added a "forced material" where if it is >=0, any added tet will have
//    the forced material instead of the normal requested material.
//
//    Jeremy Meredith, Mon Jan  6 10:06:49 PST 2003
//    VTK tets are inverted from Silo tets, so I flipped the ordering of 
//    the created wedge nodes to correct this.
//
//    Jeremy Meredith, Tue Jan 14 14:37:46 PST 2003
//    Leave any created wedges whole.  We will split them into tets only if
//    they need to be.
//
//    Hank Childs, Fri Jan 28 15:36:03 PST 2005
//    Use exception macros.
//
// ****************************************************************************
void
TetMIR::MergeTetsHelper(TetList &tetlist, WedgeList &wedgelist,
                     int c, int npts, const int *c_ptr,
                     const MaterialTetrahedron &mattet, int *maxmat,
                     const Tet &tet1, const Tet &tet2,
                     int forcedMat)
{
    const Tet::Node &v1_0 = tet1.node[0];
    const Tet::Node &v1_1 = tet1.node[1];
    const Tet::Node &v1_2 = tet1.node[2];
    const Tet::Node &v1_3 = tet1.node[3];

    const Tet::Node &v2_1 = tet2.node[1];
    const Tet::Node &v2_2 = tet2.node[2];
    const Tet::Node &v2_3 = tet2.node[3];

    Tet::Node  i_01, i_02, i_03, i_12, i_13, i_23;

    int m1 = tet1.mat;
    int m2 = tet2.mat;

    // Calculate needed intersections
    if (maxmat[0] != maxmat[1])
    {
        double T01 = FindIntersect(tet1.node[0].vf,tet1.node[1].vf, tet2.node[0].vf,tet2.node[1].vf);
        if (T01 == 0.0)
            i_01 = v1_0;
        else if (T01 == 1.0)
            i_01 = v1_1;
        else
        {
            i_01 = Tet::Node(T01, tet1.node[0], tet1.node[1]);
            IndexTetNode(i_01, c, npts, c_ptr, mattet);
        }
    }
    if (maxmat[0] != maxmat[2])
    {
        double T02 = FindIntersect(tet1.node[0].vf,tet1.node[2].vf, tet2.node[0].vf,tet2.node[2].vf);
        if (T02 == 0.0)
            i_02 = v1_0;
        else if (T02 == 1.0)
            i_02 = v1_2;
        else
        {
            i_02 = Tet::Node(T02, tet1.node[0], tet1.node[2]);
            IndexTetNode(i_02, c, npts, c_ptr, mattet);
        }
    }
    if (maxmat[0] != maxmat[3])
    {
        double T03 = FindIntersect(tet1.node[0].vf,tet1.node[3].vf, tet2.node[0].vf,tet2.node[3].vf);
        if (T03 == 0.0)
            i_03 = v1_0;
        else if (T03 == 1.0)
            i_03 = v1_3;
        else
        {
            i_03 = Tet::Node(T03, tet1.node[0], tet1.node[3]);
            IndexTetNode(i_03, c, npts, c_ptr, mattet);
        }
    }
    if (maxmat[1] != maxmat[2])
    {
        double T12 = FindIntersect(tet1.node[1].vf,tet1.node[2].vf, tet2.node[1].vf,tet2.node[2].vf);
        if (T12 == 0.0)
            i_12 = v1_1;
        else if (T12 == 1.0)
            i_12 = v1_2;
        else
        {
            i_12 = Tet::Node(T12, tet1.node[1], tet1.node[2]);
            IndexTetNode(i_12, c, npts, c_ptr, mattet);
        }
    }
    if (maxmat[1] != maxmat[3])
    {
        double T13 = FindIntersect(tet1.node[1].vf,tet1.node[3].vf, tet2.node[1].vf,tet2.node[3].vf);
        if (T13 == 0.0)
            i_13 = v1_1;
        else if (T13 == 1.0)
            i_13 = v1_3;
        else
        {
            i_13 = Tet::Node(T13, tet1.node[1], tet1.node[3]);
            IndexTetNode(i_13, c, npts, c_ptr, mattet);
        }
    }
    if (maxmat[2] != maxmat[3])
    {
        double T23 = FindIntersect(tet1.node[2].vf,tet1.node[3].vf, tet2.node[2].vf,tet2.node[3].vf);
        if (T23 == 0.0)
            i_23 = v1_2;
        else if (T23 == 1.0)
            i_23 = v1_3;
        else
        {
            i_23 = Tet::Node(T23, tet1.node[2], tet1.node[3]);
            IndexTetNode(i_23, c, npts, c_ptr, mattet);
        }
    }

    // Use the intersections and create the new tetrahedrons
    if      (maxmat[1]==m1 && maxmat[2]==m1 && maxmat[3]==m1)
    {
        // throw "Internal Error in MergeTetsHelper()"
        // This shouldn't happen, but it is.  For now, do the right thing:
        tetlist.Add(tet1, forcedMat);
    }
    else if (maxmat[1]==m2 && maxmat[2]==m1 && maxmat[3]==m1)
    {
        tetlist.AddTet(c,     m2, v2_1, i_12, i_01, i_13,             forcedMat);
        wedgelist.AddWedge(c, m1, v1_2, v1_0, v1_3, i_12, i_01, i_13, forcedMat);
    }
    else if (maxmat[1]==m1 && maxmat[2]==m2 && maxmat[3]==m1)
    {
        tetlist.AddTet(c,     m2, v2_2, i_02, i_12, i_23,             forcedMat);
        wedgelist.AddWedge(c, m1, v1_1, v1_3, v1_0, i_12, i_23, i_02, forcedMat);
    }
    else if (maxmat[1]==m1 && maxmat[2]==m1 && maxmat[3]==m2)
    {
        tetlist.AddTet(c,     m2, v2_3, i_13, i_03, i_23,             forcedMat);
        wedgelist.AddWedge(c, m1, i_03, i_13, i_23, v1_0, v1_1, v1_2, forcedMat);
    }
    else if (maxmat[1]==m1 && maxmat[2]==m2 && maxmat[3]==m2)
    {
        wedgelist.AddWedge(c, m1, i_12, i_13, v1_1, i_02, i_03, v1_0, forcedMat);
        wedgelist.AddWedge(c, m2, i_03, i_13, v2_3, i_02, i_12, v2_2, forcedMat);
    }
    else if (maxmat[1]==m2 && maxmat[2]==m1 && maxmat[3]==m2)
    {
        wedgelist.AddWedge(c, m1, i_23, i_12, v1_2, i_03, i_01, v1_0, forcedMat);
        wedgelist.AddWedge(c, m2, i_23, i_03, v2_3, i_12, i_01, v2_1, forcedMat);
    }
    else if (maxmat[1]==m2 && maxmat[2]==m2 && maxmat[3]==m1)
    {
        wedgelist.AddWedge(c, m1, i_13, i_23, v1_3, i_01, i_02, v1_0, forcedMat);
        wedgelist.AddWedge(c, m2, i_02, i_23, v2_2, i_01, i_13, v2_1, forcedMat);
    }
    else if (maxmat[1]==m2 && maxmat[2]==m2 && maxmat[3]==m2)
    {
        tetlist.AddTet(c,     m1, v1_0, i_01, i_02, i_03,             forcedMat);
        wedgelist.AddWedge(c, m2, v2_1, v2_2, v2_3, i_01, i_02, i_03, forcedMat);
    }
    else
    {
        // Internal Error in MergeTetsHelper() 
        EXCEPTION0(VisItException);
    }

}


// ****************************************************************************
//  Method:  MergeTrisHelper
//
//  Purpose:
//    merge tris when they need to be split
//
//  Arguments:
//    trilist    the list to which to append the new triangles
//    c          the cell number
//    cell       the current cell
//    mattri     the current material triangle
//    maxmat     maximum-vf material for each node
//    tri1,tri2  the two tris to merge
//
//  Note:        Assumes maxmat[0]==tri1.mat!
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//    Jeremy Meredith, Wed Feb 14 14:30:17 PST 2001
//    Made it index nodes during reconstruction.
//    Made it use doubles for intersection calculations.
//    Made it a class method.
//    Made it use a TetList to insert reconstructed shapes so that it could
//    handle proper tetrahedralization (ie diagonals from lowest index verts).
//
//    Jeremy Meredith, Wed Mar 13 15:12:41 PST 2002
//    Made it use double precision for the intersection calculations, and
//    made the point use the original index if the intersection fell upon
//    an original node.  See MergeTetsHelper for more detail.
//
//    Hank Childs, Tue Oct  8 15:50:04 PDT 2002
//    Removed costly VTK calls.
//
//    Jeremy Meredith, Wed Dec 11 10:10:31 PST 2002
//    Added a "forced material" where if it is >=0, any added tet will have
//    the forced material instead of the normal requested material.
//
//    Hank Childs, Fri Jan 28 15:36:03 PST 2005
//    Use exception macros.
//
// ****************************************************************************
void
TetMIR::MergeTrisHelper(TriList &trilist, int c, int npts, const int *c_ptr,
                     const MaterialTriangle &mattri, int *maxmat,
                     const Tri &tri1, const Tri &tri2,
                     int forcedMat)
{
    Tri::Node  v1_0, v1_1, v1_2;
    Tri::Node  v2_0, v2_1, v2_2;

    Tri::Node  i_01, i_02, i_12;

    int m1 = tri1.mat;
    int m2 = tri2.mat;

    v1_0 = Tri::Node(tri1.node[0]);
    v1_1 = Tri::Node(tri1.node[1]);
    v1_2 = Tri::Node(tri1.node[2]);

    v2_0 = Tri::Node(tri2.node[0]);
    v2_1 = Tri::Node(tri2.node[1]);
    v2_2 = Tri::Node(tri2.node[2]);

    // Calculate needed intersections
    if (maxmat[0] != maxmat[1])
    {
        double T01 = FindIntersect(tri1.node[0].vf,tri1.node[1].vf, tri2.node[0].vf,tri2.node[1].vf);
        if (T01 == 0.0)
            i_01 = v1_0;
        else if (T01 == 1.0)
            i_01 = v1_1;
        else
        {
            i_01 = Tri::Node(T01, tri1.node[0], tri1.node[1]);
            IndexTriNode(i_01, c, npts, c_ptr, mattri);
        }
    }
    if (maxmat[0] != maxmat[2])
    {
        double T02 = FindIntersect(tri1.node[0].vf,tri1.node[2].vf, tri2.node[0].vf,tri2.node[2].vf);
        if (T02 == 0.0)
            i_02 = v1_0;
        else if (T02 == 1.0)
            i_02 = v1_2;
        else
        {
            i_02 = Tri::Node(T02, tri1.node[0], tri1.node[2]);
            IndexTriNode(i_02, c, npts, c_ptr, mattri);
        }
    }
    if (maxmat[1] != maxmat[2])
    {
        double T12 = FindIntersect(tri1.node[1].vf,tri1.node[2].vf, tri2.node[1].vf,tri2.node[2].vf);
        if (T12 == 0.0)
            i_12 = v1_1;
        else if (T12 == 1.0)
            i_12 = v1_2;
        else
        {
            i_12 = Tri::Node(T12, tri1.node[1], tri1.node[2]);
            IndexTriNode(i_12, c, npts, c_ptr, mattri);
        }
    }


    // Use the intersections and create the new triangles
    if      (maxmat[1]==m1 && maxmat[2]==m1)
    {
        // throw "Internal Error in MergeTrisHelper()"
        // This shouldn't happen, but it is.  For now, do the right thing:
        trilist.Add(tri1, forcedMat);
    }
    else if (maxmat[1]==m2 && maxmat[2]==m1)
    {
        trilist.AddTri(c,  m2, v2_1, i_12, i_01,       forcedMat);
        trilist.AddQuad(c, m1, v1_2, v1_0, i_01, i_12, forcedMat);
    }
    else if (maxmat[1]==m1 && maxmat[2]==m2)
    {
        trilist.AddTri(c,  m2, v2_2, i_02, i_12,       forcedMat);
        trilist.AddQuad(c, m1, v1_0, v1_1, i_12, i_02, forcedMat);
    }
    else if (maxmat[1]==m2 && maxmat[2]==m2)
    {
        trilist.AddTri(c,  m1, v1_0, i_01, i_02,       forcedMat);
        trilist.AddQuad(c, m2, v2_1, v2_2, i_02, i_01, forcedMat);
    }
    else
    {
        // Internal Error in MergeTrisHelper() 
        EXCEPTION0(VisItException);
    }
}

// ****************************************************************************
//  Method:  MergeWedges
//
//  Purpose:
//    Merge two wedges.
//
//  Arguments:
//    tetlist        the list to which to append the new tetrahedrons
//    wedgelist      the list to which to append the new wedges
//    c              the cell number
//    cell           the current cell
//    matwedge       the current material wedges
//    wedge1,wedge2  the two wedges to merge
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 13, 2002
//
//  Modifications:
//
// ****************************************************************************
void
TetMIR::MergeWedges(TetList &tetlist, WedgeList &wedgelist,
                 int c, int npts, const int *c_ptr,
                 const MaterialTetrahedron &mattet,
                 const Wedge &wedge1, const Wedge &wedge2,
                 int forcedMat)
{
    float wedge1max = wedge1.node[0].vf;
    float wedge1min = wedge1.node[0].vf;
    float wedge2max = wedge2.node[0].vf;
    float wedge2min = wedge2.node[0].vf;
    for (int i=1; i<6; i++)
    {
        if (wedge1.node[i].vf > wedge1max)
            wedge1max = wedge1.node[i].vf;
        if (wedge1.node[i].vf < wedge1min)
            wedge1min = wedge1.node[i].vf;
        if (wedge2.node[i].vf > wedge2max)
            wedge2max = wedge2.node[i].vf;
        if (wedge2.node[i].vf < wedge2min)
            wedge2min = wedge2.node[i].vf;
    }

    if (wedge1min >= wedge2max)
    {
        wedgelist.Add(wedge1, forcedMat);
    }
    else if (wedge2min >= wedge1max)
    {
        wedgelist.Add(wedge2, forcedMat);
    }
    else
    {
        // get the tets
        int nodes[6] = {wedge1.node[0].index,
                        wedge1.node[1].index,
                        wedge1.node[2].index,
                        wedge1.node[3].index,
                        wedge1.node[4].index,
                        wedge1.node[5].index};

        static int indices[12];
        int ntets = Tetrahedralizer::GetLowTetNodesForWdg(6,nodes, indices);

        for (int t=0; t<ntets; t++)
        {
            // indices for the nodes of the new tet
            int ax = indices[t*4 + 0];
            int bx = indices[t*4 + 1];
            int cx = indices[t*4 + 2];
            int dx = indices[t*4 + 3];

            Tet tet1(c, wedge1.node[ax], wedge1.node[bx],
                        wedge1.node[cx], wedge1.node[dx], wedge1.mat);
            Tet tet2(c, wedge2.node[ax], wedge2.node[bx],
                        wedge2.node[cx], wedge2.node[dx], wedge2.mat);

            MergeTets(tetlist, wedgelist, c, npts, c_ptr, mattet,
                      tet1, tet2, forcedMat);
        }
    }
}

// ****************************************************************************
//  Method:  MergeTets
//
//  Purpose:
//    Merge two tets.
//
//  Arguments:
//    tetlist    the list to which to append the new tetrahedrons
//    wedgelist  the list to which to append the new wedges
//    c          the cell number
//    cell       the current cell
//    mattet     the current material tetrahedron
//    tet1,tet2  the two tets to merge
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//    Jeremy Meredith, Wed Feb 14 14:32:32 PST 2001
//    Changed some interfaces for other reasons.
//
//    Hank Childs, Tue Oct  8 15:50:04 PDT 2002
//    Removed costly VTK calls.
//
//    Jeremy Meredith, Wed Dec 11 10:10:31 PST 2002
//    Added a "forced material" where if it is >=0, any added tet will have
//    the forced material instead of the normal requested material.
//
//    Jeremy Meredith, Tue Jan 14 14:40:41 PST 2003
//    Added the ability to output pieces of split cells into a wedgelist.
//
// ****************************************************************************
void
TetMIR::MergeTets(TetList &tetlist, WedgeList &wedgelist,
               int c, int npts, const int *c_ptr,
               const MaterialTetrahedron &mattet,
               const Tet &tet1, const Tet &tet2,
               int forcedMat)
{
    float tet1max = STDMAX(STDMAX(tet1.node[0].vf,tet1.node[1].vf),STDMAX(tet1.node[2].vf,tet1.node[3].vf));
    float tet1min = STDMIN(STDMIN(tet1.node[0].vf,tet1.node[1].vf),STDMIN(tet1.node[2].vf,tet1.node[3].vf));
    float tet2max = STDMAX(STDMAX(tet2.node[0].vf,tet2.node[1].vf),STDMAX(tet2.node[2].vf,tet2.node[3].vf));
    float tet2min = STDMIN(STDMIN(tet2.node[0].vf,tet2.node[1].vf),STDMIN(tet2.node[2].vf,tet2.node[3].vf));

    if (tet1min >= tet2max)
    {
        tetlist.Add(tet1, forcedMat);
    }
    else if (tet2min >= tet1max)
    {
        tetlist.Add(tet2, forcedMat);
    }
    else
    {
        int maxmat[4];
        for (int n=0; n<4; n++)
            maxmat[n] = (tet1.node[n].vf > tet2.node[n].vf) ? tet1.mat : tet2.mat;

        if (maxmat[0] == tet1.mat)
            MergeTetsHelper(tetlist, wedgelist, c, npts, c_ptr,mattet,maxmat,
                            tet1, tet2, forcedMat);
        else
            MergeTetsHelper(tetlist, wedgelist, c, npts, c_ptr,mattet,maxmat,
                            tet2, tet1, forcedMat);
    }
}

// ****************************************************************************
//  Method:  MergeTris
//
//  Purpose:
//    Merge two tris.
//
//  Arguments:
//    trilist    the list to which to append the new triangles
//    c          the cell number
//    cell       the current cell
//    mattri     the current material triangles
//    tri1,tri2  the two tris to merge
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//    Jeremy Meredith, Wed Feb 14 14:32:32 PST 2001
//    Changed some interfaces for other reasons.
//
//    Hank Childs, Tue Oct  8 15:50:04 PDT 2002
//    Removed costly VTK calls.
//
//    Jeremy Meredith, Wed Dec 11 10:10:31 PST 2002
//    Added a "forced material" where if it is >=0, any added tet will have
//    the forced material instead of the normal requested material.
//
// ****************************************************************************
void
TetMIR::MergeTris(TriList &trilist, int c, int npts, const int *c_ptr,
               const MaterialTriangle &mattri, const Tri &tri1,const Tri &tri2,
               int forcedMat)
{
    float tri1max = STDMAX(STDMAX(tri1.node[0].vf,tri1.node[1].vf),tri1.node[2].vf);
    float tri1min = STDMIN(STDMIN(tri1.node[0].vf,tri1.node[1].vf),tri1.node[2].vf);
    float tri2max = STDMAX(STDMAX(tri2.node[0].vf,tri2.node[1].vf),tri2.node[2].vf);
    float tri2min = STDMIN(STDMIN(tri2.node[0].vf,tri2.node[1].vf),tri2.node[2].vf);

    if (tri1min >= tri2max)
    {
        trilist.Add(tri1, forcedMat);
    }
    else if (tri2min >= tri1max)
    {
        trilist.Add(tri2, forcedMat);
    }
    else
    {
        int maxmat[3];
        for (int n=0; n<3; n++)
            maxmat[n] = (tri1.node[n].vf > tri2.node[n].vf) ? tri1.mat : tri2.mat;

        if (maxmat[0] == tri1.mat)
            MergeTrisHelper(trilist, c, npts,c_ptr,mattri, maxmat, tri1, tri2, forcedMat);
        else
            MergeTrisHelper(trilist, c, npts,c_ptr,mattri, maxmat, tri2, tri1, forcedMat);
    }
}

// ****************************************************************************
//  Method:  CreateFaceHash
//
//  Purpose:
//    Create a hash for ZoneCntAndVF's containing every zone's faces
//
//  Arguments:
//    conn       the MIR connectivity object.
//    nmat       the number of materials
//    hashfunc   the hash function for Face
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Apr 10 16:46:10 PDT 2001
//    Reduced number of virtual function calls.
//
//    Hank Childs, Mon Oct  7 16:15:56 PDT 2002
//    Removed costly VTK calls.
//
// ****************************************************************************
static FaceHash *
CreateFaceHash(MIRConnectivity &conn, int nmat, 
               unsigned int (*hashfunc)(Face&))
{
    FaceHash *face_hash = new FaceHash(conn.ncells*3, hashfunc);

    int nCells = conn.ncells;
    int *c_ptr = conn.connectivity;
    for (int c=0; c<nCells; c++)
    {
        AddFaces(conn.celltype[c], c_ptr+1, face_hash, nmat);
        c_ptr += *c_ptr+1;
    }

    return face_hash;
}

// ****************************************************************************
//  Method:  AddFaces
//
//  Purpose:
//      Adds all of the faces for a specific cell to the face hash.
//
//  Arguments:
//      celltype    The type of the cell as a VTK enumerated type.
//      cellids     The connectivity list for the cell.
//      face_hash   The hash to add the faces to.
//      nmat        The max number of materials
//      nRealMat    The number of materials that exist in this cell
//      materials   The material numbers that exist in this cell
//      vf          The volume fractions of the mats that exist in this cell
//
//  Notes:  This routine assumes that the face hash is populated for
//          the first time before volume fractions are being subsampled.
//
//  Programmer:  Hank Childs
//  Creation:    October 7, 2002
//
//  Modifications:
//    Jeremy Meredith, Wed Dec 18 12:17:30 PST 2002
//    Added subsampling of volume fractions and fixed some assumptions.
//
// ****************************************************************************
static void
AddFaces(int celltype, const int *cellids, FaceHash *face_hash, int nmat,
         int nRealMat, int *materials, float *vf)
{
    static int hexfaces[6][4] = { {0,4,7,3}, {1,2,6,5}, {0,1,5,4}, {3,7,6,2},
                           {0,3,2,1}, {4,5,6,7} };
    static int tetfaces[4][3] = { {0,1,3}, {1,2,3}, {2,0,3}, {0,2,1} };
    static int wedgetrifaces[2][3]  = { {0,1,2}, {3,5,4} };
    static int wedgequadfaces[3][4] = { {0,3,4,1}, {1,4,5,2}, {2,5,3,0} };
    static int pyramidtrifaces[4][3] = { {0,1,4}, {1,2,4}, {2,3,4}, {3,0,4} };
    static int pyramidquadfaces[1][4] = { {0,3,2,1} };

    int (*trifaces)[3]  = NULL;
    int (*quadfaces)[4] = NULL;
    int ntrifaces = 0;
    int nquadfaces = 0;
    switch (celltype)
    {
      case VTK_TETRA:
        trifaces = tetfaces;
        ntrifaces = 3;
        break;
      case VTK_HEXAHEDRON:
        quadfaces = hexfaces;
        nquadfaces = 6;
        break;
      case VTK_WEDGE:
        trifaces = wedgetrifaces;
        ntrifaces = 2;
        quadfaces = wedgequadfaces;
        nquadfaces = 3;
        break;
      case VTK_PYRAMID:
        trifaces = pyramidtrifaces;
        ntrifaces = 4;
        quadfaces = pyramidquadfaces;
        nquadfaces = 1;
        break;
    }
    int f;
    int ids[4];
    for (f=0; f<ntrifaces; f++)
    {
        ids[0] = cellids[trifaces[f][0]];
        ids[1] = cellids[trifaces[f][1]];
        ids[2] = cellids[trifaces[f][2]];
        Face face(3, ids);
        if (face_hash->Find(face))
        {
            ZoneCntAndVF &F = face_hash->Get();
            F.zonecnt++;
            for (int m=0; m<nRealMat; m++)
            {
                F.vf[materials[m]] += vf[m];
            }
        }
        else
        {
            ZoneCntAndVF F(1,nmat);
            for (int m=0; m<nRealMat; m++)
            {
                F.vf[materials[m]] = vf[m];
            }
            face_hash->Insert(F);
        }
    }

    for (f=0; f<nquadfaces; f++)
    {
        ids[0] = cellids[quadfaces[f][0]];
        ids[1] = cellids[quadfaces[f][1]];
        ids[2] = cellids[quadfaces[f][2]];
        ids[3] = cellids[quadfaces[f][3]];
        Face face(4, ids);
        if (face_hash->Find(face))
        {
            ZoneCntAndVF &F = face_hash->Get();
            F.zonecnt++;
            for (int m=0; m<nRealMat; m++)
            {
                F.vf[materials[m]] += vf[m];
            }
        }
        else
        {
            ZoneCntAndVF F(1,nmat);
            for (int m=0; m<nRealMat; m++)
            {
                F.vf[materials[m]] = vf[m];
            }
            face_hash->Insert(F);
        }
    }
}

// ****************************************************************************
//  Method:  CreateEdgeHash
//
//  Purpose:
//    Create a hash for ZoneCntAndVF's containing every zone's edges
//
//  Arguments:
//    mesh       the mesh
//    nmat       the number of materials
//    hashfunc   the hash function for edge
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Apr 10 16:46:10 PDT 2001
//    Reduced number of virtual function calls.
//
//    Hank Childs, Mon Oct  7 16:47:29 PDT 2002
//    Removed costly VTK calls.
//
// ****************************************************************************
static EdgeHash *
CreateEdgeHash(MIRConnectivity &conn, int nmat, 
               unsigned int (*hashfunc)(Edge&))
{
    int nCells = conn.ncells;
    EdgeHash *edge_hash = new EdgeHash(nCells*3, hashfunc);

    const int *c_ptr = conn.connectivity;
    for (int c=0; c<nCells; c++)
    {
        AddEdges(conn.celltype[c], c_ptr+1, edge_hash, nmat);
        c_ptr += *c_ptr+1;
    }

    return edge_hash;
}

// ****************************************************************************
//  Method:  AddEdges
//
//  Purpose:
//      Adds all of the edges for a specific cell to the edge hash.
//
//  Arguments:
//      celltype    The type of the cell as a VTK enumerated type.
//      cellids     The connectivity list for the cell.
//      edge_hash   The hash to add the edges to.
//      nmat        The max number of materials
//      nRealMat    The number of materials that exist in this cell
//      materials   The material numbers that exist in this cell
//      vf          The volume fractions of the mats that exist in this cell
//
//  Notes:  This routine assumes that the edge hash is populated for
//          the first time before volume fractions are being subsampled.
//
//  Programmer:  Hank Childs
//  Creation:    October 7, 2002
//
//  Modifications:
//    Jeremy Meredith, Wed Dec 18 12:17:30 PST 2002
//    Added subsampling of volume fractions and fixed some assumptions.
//
// ****************************************************************************
static void
AddEdges(int celltype, const int *cellids, EdgeHash *edge_hash, int nmat,
         int nRealMat, int *materials, float *vf)
{
    static int tetra_edges[6][2] = { {0,1}, {1,2}, {2,0}, {0,3}, {1,3}, {2,3}};
    static int pyramid_edges[8][2] = { {0,1}, {1,2}, {2,3}, {3,0}, {0,4}, 
                                       {1,4}, {2,4}, {3,4} };
    static int hexahedron_edges[12][2] =  { {0,1}, {1,2}, {3,2}, {0,3}, {4,5},
                                            {5,6}, {7,6}, {4,7}, {0,4}, {1,5},
                                            {3,7}, {2,6} };
    static int wedge_edges[9][2] = { {0,1}, {1,2}, {2,0}, {3,4}, {4,5}, {5,3},
                                     {0,3}, {1,4}, {2,5} };
    static int tri_edges[3][2] = { {0,1}, {1,2}, {2,0} };
    static int quad_edges[4][2] = { {0,1}, {1,2}, {3,2}, {0,3} };

    int nEdges = 0;
    int (*edgelist)[2] = NULL;
    switch (celltype)
    {
      case VTK_TETRA:
        edgelist = tetra_edges;
        nEdges = 6;
        break;
      case VTK_WEDGE:
        edgelist = wedge_edges;
        nEdges = 9;
        break;
      case VTK_PYRAMID:
        edgelist = pyramid_edges;
        nEdges = 8;
        break;
      case VTK_HEXAHEDRON:
        edgelist = hexahedron_edges;
        nEdges = 12;
        break;
      case VTK_TRIANGLE:
        edgelist = tri_edges;
        nEdges = 3;
        break;
      case VTK_QUAD:
        edgelist = quad_edges;
        nEdges = 4;
        break;
    }
    int ids[2];
    for (int e=0; e<nEdges; e++)
    {
        ids[0] = cellids[edgelist[e][0]];
        ids[1] = cellids[edgelist[e][1]];
        Edge edge(ids);
        if (edge_hash->Find(edge))
        {
            ZoneCntAndVF &E = edge_hash->Get();
            E.zonecnt++;
            for (int m=0; m<nRealMat; m++)
            {
                E.vf[materials[m]] += vf[m];
            }
        }
        else
        {
            ZoneCntAndVF E(1,nmat);
            for (int m=0; m<nRealMat; m++)
            {
                E.vf[materials[m]] = vf[m];
            }
            edge_hash->Insert(E);
        }
    }
}

// ****************************************************************************
//  Method:  AddNodes
//
//  Purpose:
//      Adds all of the vfs for a specific cell to the node list.
//
//  Arguments:
//      nPts        The number of points in the cell.
//      cellids     The connectivity list for the cell.
//      node_list   The list to add the nodes to.
//      nmat        The max number of materials
//      nRealMat    The number of materials that exist in this cell
//      materials   The material numbers that exist in this cell
//      vf          The volume fractions of the mats that exist in this cell
//
//  Programmer:  Jeremy Meredith
//  Creation:    July  3, 2003
//
// ****************************************************************************
static void
AddNodes(int nPts, const int *cellids, NodeList *node_list, int nmat,
         int nRealMat, int *materials, float *vf)
{
    for (int n=0; n<nPts; n++)
    {
        ZoneCntAndVF &N = (*node_list)[cellids[n]];
        N.zonecnt++;
        for (int m=0; m<nRealMat; m++)
        {
            N.vf[materials[m]] += vf[m];
        }
    }
}

// ****************************************************************************
//  Method:  CreateZoneCleanList
//
//  Purpose:
//    Create a list determining if a zone is truly clean.
//    "Truly" clean means it would be clean even if we performed the
//    mixed reconstruction step on it.
//
//  Arguments:
//    mesh          the data set
//    mat           the material
//    nmat          the number of materials
//    node_list     the list of vf's for the nodes of the mesh
//
//  Programmer:  Jeremy Meredith
//  Creation:    May 31, 2001
//
//  Modifications:
//    Kathleen Bonnell, Thu Nov  8 09:20:50 PST 2001 
//    Changed ids to vtkIdType to match VTK 4.0 API.
//
//    Eric Brugger, Tue Dec 11 12:04:40 PST 2001
//    Reduced the amount of temporary memory used.
//
//    Jeremy Meredith, Fri Oct  4 11:08:58 PDT 2002
//    Added count of real mixed and real clean zones.
//
//    Hank Childs, Mon Oct  7 15:10:01 PDT 2002
//    Added an optimization for large material cases.
//
//    Hank Childs, Tue Oct  8 14:45:46 PDT 2002
//    Remove costly VTK calls.
//
//    Jeremy Meredith, July  2, 2003
//    Made use of mat_cnt when determining if a zone is "really clean".
//    The reason is that if there are more than two materials in an adjacent
//    zone, we may still need to split clean zones that don't fit the
//    "one material dominant at all nodes" criterion.
//
// ****************************************************************************
ZoneCleanList *
CreateZoneCleanList(MIRConnectivity &conn, int nPts, avtMaterial *mat, 
                    int nmat, int *mat_cnt, NodeList *node_list,
                    int &nrealclean, int &nrealmixed)
{
    nrealclean = 0;
    nrealmixed = 0;
    int nCells = conn.ncells;
    ZoneCleanList *real_clean_zones = new ZoneCleanList(nCells, false);

    int *dom_mat = new int[nPts];
    for (int i = 0 ; i < nPts ; i++)
    {
        dom_mat[i] = -1;
    }
    const int *c_ptr = conn.connectivity;
    for (int c=0; c<nCells; c++)
    {
        if (mat->GetMatlist()[c] < 0)
        {
            nrealmixed++;
            c_ptr += *c_ptr+1;
            continue;
        }
        int        clean_mat = mat->GetMatlist()[c];
        int        nPts = *c_ptr;
        const int *ids  = c_ptr+1;

        bool       real_clean = mat_cnt[c] < 3;
        for (int n=0; n<nPts && real_clean; n++)
        {
            int   id = ids[n];
            const float *vf_list = &((*node_list)[id].vf[0]);
            float clean_vf = vf_list[clean_mat];
            if (dom_mat[id] == -1)
            {
                float vf = 0.;
                for (int m=0; m<nmat ; m++)
                {
                    if (vf_list[m] > vf)
                    {
                        vf = vf_list[m];
                        dom_mat[id] = m;
                    }
                }
            }
            if (vf_list[dom_mat[id]] > clean_vf)
                real_clean = false;
        }
        c_ptr += *c_ptr+1;

        (*real_clean_zones)[c] = real_clean;
        if (real_clean)
            nrealclean++;
        else
            nrealmixed++;
    }

    delete [] dom_mat;
    return real_clean_zones;
}

// ****************************************************************************
//  Method:  SubsampleVFsAndCreateNodeList
//
//  Purpose:
//    Using the zonal vf's, average them to the nodes, faces, and edges.
//
//  Arguments:
//    conn       the connectivity information.
//    npts       the number of points in the mesh.
//    mat        the material
//    node_list  the node list of ZoneCntAndVF's
//    face_hash  the face hash of ZoneCntAndVF's
//    edge_hash  the edge hash of ZoneCntAndVF's
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//    Hank Childs, Tue Apr 10 16:46:10 PDT 2001
//    Reduced number of virtual function calls.
//
//    Kathleen Bonnell, Thu Nov  8 09:20:50 PST 2001 
//    Changed ids to vtkIdType to match VTK 4.0 API.
//
//    Eric Brugger, Tue Dec 11 12:04:40 PST 2001
//    Made some performance enhancements.
//
//    Hank Childs, Wed Sep 25 15:16:13 PDT 2002
//    Clean up memory leak.
//
//    Hank Childs, Mon Oct  7 17:05:48 PDT 2002
//    Remove costly VTK calls.
//
//    Jeremy Meredith, July  2, 2003
//    Restructured to clean some things up and to fix support for faces
//    and edges.  Added code to extract the number of how many materials
//    have nonzero volume fractions at any of the cells nodes into mat_cnt.
//
// ****************************************************************************
static void
SubsampleVFsAndCreateNodeList(MIRConnectivity      &conn,
                              int                        npts,
                              avtMaterial               *mat,
                              int                       *mat_cnt,
                              NodeList                  *node_list,
                              FaceHash                  *face_hash,
                              EdgeHash                  *edge_hash)
{
    int          nCells  = conn.ncells;

    int          nMat    = mat->GetNMaterials();
    const int   *matlist = mat->GetMatlist();
    const int   *mixMat  = mat->GetMixMat();
    const int   *mixNext = mat->GetMixNext();
    const float *mixVF   = mat->GetMixVF();

    int          nRealMat;
    int         *materials       = new int[nMat];
    float       *volumeFractions = new float[nMat];

    //
    // Go through each cell, extract the materials from the avtMaterial,
    // and subsample the zone volume fractions to the nodes and the faces.
    //
    const int *c_ptr = conn.connectivity;
    int c;
    for (c=0; c<nCells; c++)
    {
        int        nPts = *c_ptr;
        const int *ids  = c_ptr+1;

        //
        // Create a list of materials and their corresponding volume
        // fractions for the cell for the materials actually used.
        //
        int matno = matlist[c];

        if (matno < 0)
        {
            nRealMat = 0;
            int  mixIndex  = -matno - 1;

            while (mixIndex >= 0)
            {
                materials[nRealMat]       = mixMat[mixIndex];
                volumeFractions[nRealMat] = mixVF[mixIndex];
                nRealMat++;
                mixIndex = mixNext[mixIndex] - 1;
            }
        }
        else
        {
            nRealMat = 1;
            materials[0]       = matno;
            volumeFractions[0] = 1.;
        }

        // sample to the nodes
        AddNodes(nPts, ids, node_list, nMat,
                 nRealMat, materials, volumeFractions);

        // sample to the faces
        if (face_hash)
        {
            AddFaces(conn.celltype[c], ids, face_hash, nMat,
                     nRealMat, materials, volumeFractions);
        }

        // sample to the edges
        if (edge_hash)
        {
            AddEdges(conn.celltype[c], ids, edge_hash, nMat,
                     nRealMat, materials, volumeFractions);
        }
        c_ptr += *c_ptr+1;
    }


    //
    // Divide by the number of incident zones to renormalize the averaged vf
    //
    for (int n=0; n<npts; n++)
    {
        for (int m=0; m<nMat; m++)
            (*node_list)[n].vf[m] /= (*node_list)[n].zonecnt;
    }

    if (face_hash)
    {
        ZoneCntAndVF *f = face_hash->GetFirstValidValue();
        while (f)
        {
            for (int m=0; m<nMat; m++)
                f->vf[m] /= f->zonecnt;
            f = face_hash->GetNextValidValue();
        }
    }

    if (edge_hash)
    {
        ZoneCntAndVF *e = edge_hash->GetFirstValidValue();
        while (e)
        {
            for (int m=0; m<nMat; m++)
                e->vf[m] /= e->zonecnt;
            e = edge_hash->GetNextValidValue();
        }
    }

    //
    // Go through each cell and determine how many non-zero material
    // fractions it has at any node.
    //
    c_ptr = conn.connectivity;
    for (c=0; c<nCells; c++)
    {
        int        nPts = *c_ptr;
        const int *ids  = c_ptr+1;

        mat_cnt[c] = 0;
        for (int m=0; m<nMat; m++)
        {
            for (int n=0; n<nPts; n++)
            {
                if ((*node_list)[ids[n]].vf[m] > 0)
                {
                    mat_cnt[c]++;
                    break;
                }
            }
        }
        c_ptr += *c_ptr+1;
    }

    delete [] materials;
    delete [] volumeFractions;
}

// ****************************************************************************
//  Method:  ExtractCellVFs
//
//  Purpose:
//    Given a cell, extracts its globally indexed VFs to locally indexed arrays
//
//  Arguments:
//    c          the cell number                          (i)
//    cell       the cell                                 (i)
//    nmat       the number of materials                  (i)
//    zone_vf    the vf's for each mat                    (i)
//    node_list  the node list of ZoneCntAndVF's          (i)
//    face_hash  the face hash of ZoneCntAndVF's          (i)
//    edge_hash  the edge hash of ZoneCntAndVF's          (i)
//    vf_zone    the local vf array for the zone center   (o)
//    vf_node    the local vf array for the nodes         (o)
//    vf_face    the local vf array for the faces         (o)
//    vf_edge    the local vf array for the edges         (o)
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Apr 10 16:46:10 PDT 2001
//    Reduced number of virtual function calls.
//
//    Eric Brugger, Wed Sep 19 11:53:29 PDT 2001
//    I removed some vector resizes of vf_zone, vf_node, vf_face and vf_edge
//    assuming they are sized properly before being called.
//
//    Kathleen Bonnell, Thu Nov  8 09:20:50 PST 2001 
//    Changed ids to vtkIdType to match VTK 4.0 API.
//
//    Eric Brugger, Tue Dec 11 12:04:40 PST 2001
//    Reduced the amount of temporary memory used.
//
//    Hank Childs, Mon Oct  7 17:34:24 PDT 2002
//    Remove costly VTK calls.
//
// ****************************************************************************
static void
ExtractCellVFs(int c, int nPts, const int *ids, int celltype, int nmat,
               float                     *zone_vf,
               NodeList                  *node_list,
               FaceHash                  *face_hash,
               EdgeHash                  *edge_hash,
               vector<float>             &vf_zone,
               vector<float>             *vf_node,
               vector<float>             *vf_face,
               vector<float>             *vf_edge)
{
    // zones
    for (int m=0; m<nmat; m++)
        vf_zone[m] = zone_vf[m];

    // nodes
    for (int n=0; n<nPts; n++)
    {
        int id = ids[n];
        for (int m=0; m<nmat; m++)
            vf_node[n][m] = (*node_list)[id].vf[m];
    }

    // faces
    if (face_hash)
    {
        AddFaces(celltype, ids, face_hash, nmat);
    }

    // edges
    if (edge_hash)
    {
        AddEdges(celltype, ids, edge_hash, nmat);
    }
}


// ****************************************************************************
//  Function: SetUpCoords
//
//  Purpose:
//      Sets up the coordinates array.  Avoid using any VTK calls in its inner
//      loop.
//
//  Programmer: Hank Childs
//  Creation:   October 5, 2002
//
// ****************************************************************************

void
SetUpCoords(vtkDataSet *mesh, vector<TetMIR::ReconstructedCoord> &coordsList)
{
    int nPoints = mesh->GetNumberOfPoints();
    int i, j, k;

    coordsList.resize(nPoints);
    
    int dstype = mesh->GetDataObjectType();
    if (dstype == VTK_RECTILINEAR_GRID)
    {
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) mesh;
        vtkDataArray *xc = rgrid->GetXCoordinates();
        int nx = xc->GetNumberOfTuples();
        float *x = new float[nx];
        for (i = 0 ; i < nx ; i++)
        {
            x[i] = xc->GetTuple1(i);
        }
        vtkDataArray *yc = rgrid->GetYCoordinates();
        int ny = yc->GetNumberOfTuples();
        float *y = new float[ny];
        for (i = 0 ; i < ny ; i++)
        {
            y[i] = yc->GetTuple1(i);
        }
        vtkDataArray *zc = rgrid->GetZCoordinates();
        int nz = zc->GetNumberOfTuples();
        float *z = new float[nz];
        for (i = 0 ; i < nz ; i++)
        {
            z[i] = zc->GetTuple1(i);
        }

        int pt = 0;
        for (k = 0 ; k < nz ; k++)
        {
            for (j = 0 ; j < ny ; j++)
            {
                for (i = 0 ; i < nx ; i++)
                {
                    TetMIR::ReconstructedCoord &c = coordsList[pt];
                    c.orignode = pt;
                    c.x = x[i];
                    c.y = y[j];
                    c.z = z[k];
                    pt++;
                }
            }
        }
        delete [] x;
        delete [] y;
        delete [] z;
    }
    else
    {
        vtkPointSet *ps = (vtkPointSet *) mesh;
        float *ptr = (float *) ps->GetPoints()->GetVoidPointer(0);
        for (int n=0; n<nPoints; n++)
        {
            TetMIR::ReconstructedCoord &c = coordsList[n];
            c.orignode = n;
            c.x = *ptr++;
            c.y = *ptr++;
            c.z = *ptr++;
        }
    }
}
