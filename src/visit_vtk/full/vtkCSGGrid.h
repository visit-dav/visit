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

// .NAME vtkCSGGrid - Constructive Solid Geometry representation of a grid 
// .SECTION Description
// vtkCSGGrid is a derived class of vtkDataSet used to represent variables
// on a Constructive Solid Geometry "grid." The word "grid" is used loosely
// here because, of course, a CSG representation is continuous as in not
// discrete. So, the notion of a "grid" is a bit misleading. Nontheless,
// one of the important methods of this class, Discretize, produces a 
// vtkUnstructuredGrid discrete representation from the CSG
// representation.
//
// A further potential confusion here is that the base class we are
// deriving from, vtkDataSet, is pre-disposed to think in discrete
// terms. For example, it has a method to GetNumberOfPoints() which
// returns the number of discrete points in the representation.
//
// Nonetheless, since all of VTK's useful grid types are derived from
// vtkDataSet, it is still the best place to add the new CSG grid type.
// Also, since the CSG grid type will need to be discretized before it is
// used in most pipelines, that makes it even a better candidate to be
// tucked under vtkDataSet. Lastly, since VisIt's interface to get meshes 
// from database plugins expects a vtkDataSet, that pretty much clinches
// the need to define the vtkCSGGrid class in this way.
//
// There are multiple ways around vtkDataSet's discrete notions of points.
// One is to accept the analogy that in a CSG grid, the analytic functions
// play a role similar to "nodes" of an unstructured grid. For example,
// in an unstructured grid, a list of points is specified along with a 
// a list of cells. The list of cells defines how points are combined to
// form each of the cells in the grid. In a CSG grid, a list of analytic
// functions defining boundaries is specified. Then, cells are constructed
// from boolean, point-set expressions involving the boundaries. So, the
// boundaries of a CSG grid play a role similar to the nodes of an
// unstructured grid. Likewise, just as we have cell-oriented or point-
// oriented variables in an unstructured grid, we could have cell-oriented
// or boundary-oriented variables in a CSG grid. If we take this approach,
// any method in vtkDataSet's interface that refers to points can be,
// by analogy, viewed as refering to the analytic bounaries of a CSG grid.
//
// Another way around vtkDataSet's discrete notion of points is to argue
// that a CSG grid involves an infinity of points. It is the union of all
// the points satisfying all the analytic equations of the boundaries.
// What should a method like GetNumberOfPoints() return in this case?
// We can't return infinity even though that might be the correct. This
// approach has more difficulty. So, we take the former approach and treat
// the word "point" in vtkDataSet's interface as "boundary" in vtkCSGGrid's
// interface.

// .SECTION See Also
// vtkImplicitFunction, vtkQuadric, vtkUnstructuredGrid, vtkDataSet
#ifndef __vtkCSGGrid_h
#define __vtkCSGGrid_h
#include <visit_vtk_exports.h>

#include <map>
#include <vector>

#include "vtkDataArray.h"
#include "vtkIdTypeArray.h"
#include "vtkDataSet.h"
#include "vtkImplicitFunctionCollection.h"
#include "vtkPlanes.h"
#include "vtkStructuredData.h"

#include <float.h>

class vtkPolyData;
class vtkUnstructuredGrid;

using std::map;
using std::vector;

#define VTK_CSG_GRID 20

class VISIT_VTK_API vtkCSGGrid : public vtkDataSet
{
public:
  static vtkCSGGrid *New();

  vtkTypeRevisionMacro(vtkCSGGrid,vtkDataSet);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create a similar type object.
  vtkDataObject *MakeObject() {return vtkCSGGrid::New();};

  // Description:
  // Return what type of dataset this is.
  int GetDataObjectType() {return VTK_CSG_GRID;};
  virtual const char *GetClassName() {return "vtkCSGGrid";};

  // Description:
  // Copy the geometric and topological structure of an input rectilinear grid
  // object.
  void CopyStructure(vtkDataSet *ds);

  // Description:
  // Restore object to initial state. Release memory back to system.
  void Initialize();

  // Description:
  // Standard vtkDataSet API methods. See vtkDataSet for more information.
  vtkIdType GetNumberOfCells();
  vtkIdType GetNumberOfPoints();
  vtkIdType GetNumberOfBoundaries() const;
  double *GetPoint(vtkIdType ptId);
  double *GetBoundary(vtkIdType bndId) const;
  void GetPoint(vtkIdType id, double x[3]);

  vtkCell *GetCell(vtkIdType cellId);
  void GetCell(vtkIdType cellId, vtkGenericCell *cell);
  void GetCellBounds(vtkIdType cellId, double bounds[6]);
  int FindPoint(double x, double y, double z) { return this->vtkDataSet::FindPoint(x, y, z);};
  vtkIdType FindPoint(double x[3]);
  vtkIdType FindCell(double x[3], vtkCell *cell, vtkIdType cellId, double tol2,
                     int& subId, double pcoords[3], double *weights);
  vtkIdType FindCell(double x[3], vtkCell *cell, vtkGenericCell *gencell,
                     vtkIdType cellId, double tol2, int& subId, 
                     double pcoords[3], double *weights);
  vtkCell *FindAndGetCell(double x[3], vtkCell *cell, vtkIdType cellId, 
                          double tol2, int& subId, double pcoords[3],
                          double *weights);
  int GetCellType(vtkIdType cellId);
  void GetCellPoints(vtkIdType cellId, vtkIdList *ptIds);
  void GetPointCells(vtkIdType ptId, vtkIdList *cellIds);
  void ComputeBounds();
  void SetBounds(double minX, double maxX,
                 double minY, double maxY,
                 double minZ, double maxZ)
      {Bounds[0] = minX; Bounds[1] = maxX;
       Bounds[2] = minY; Bounds[3] = maxY;
       Bounds[4] = minZ; Bounds[5] = maxZ;};
  int GetMaxCellSize();
  void GetCellNeighbors(vtkIdType cellId, vtkIdList *ptIds,
                        vtkIdList *cellIds);

  void BuildVTKImplicitFunction(int zoneId, vtkImplicitFunction **func) const;

#if 1
  //
  // A discretize method that returns the surfaces only
  //
  vtkPolyData  *DiscretizeSurfaces(int specificZone = -1, double tol = 0.01,
                                   double minX = -10.0, double maxX = 10.0,
                                   double minY = -10.0, double maxY = 10.0,
                                   double minZ = -10.0, double maxZ = 10.0);

  //
  // A discretize method that returns the volumetric mesh, uniformally
  // sampled to a specific number of samples in x, y and z
  //
  vtkUnstructuredGrid *DiscretizeSpace(int specificZone = -1, double tol = 0.01,
                                   double minX = -10.0, double maxX = 10.0,
                                   double minY = -10.0, double maxY = 10.0,
                                   double minZ = -10.0, double maxZ = 10.0);
#endif

  //
  // A discretize method that returns the entire spatial bounding
  // box, meshed adaptively, though discontinuously to a specified
  // tolerance (smallest edge length)
  //
  vtkUnstructuredGrid *DiscretizeSpace3(int specificZone = -1, int rank=0, int nprocs=1,
                                       double discTol = 0.01, double flatTol = 0.01,
                                       double minX = -10.0, double maxX = 10.0,
                                       double minY = -10.0, double maxY = 10.0,
                                       double minZ = -10.0, double maxZ = 10.0);

  // Description:
  // Return the actual size of the data in kilobytes. This number
  // is valid only after the pipeline has updated. The memory size
  // returned is guaranteed to be greater than or equal to the
  // memory required to represent the data (e.g., extra space in
  // arrays, etc. are not included in the return value). THIS METHOD
  // IS THREAD SAFE.
  unsigned long GetActualMemorySize();

  // Description:
  // Shallow and Deep copy.
  void ShallowCopy(vtkDataObject *src);  
  void DeepCopy(vtkDataObject *src);

  typedef enum {
    QUADRIC_G,
    SPHERE_G,
    SPHERE_PR,
    PLANE_GEN,
    PLANE_X,
    PLANE_Y,
    PLANE_Z,
    PLANE_PN,
    PLANE_3PT,
    PLANE_PTLINE,
    PLANE_2LINE,
    CYLINDER_GEN,
    CYLINDER_PNLR,
    CYLINDER_2PT,
    BOX
  } BoundaryType;

  typedef enum {
    INNER, // replace '=' with '<' in boundary equation
    OUTER, // replace '=' with '>' in boundary equation
    ON,    // use '=' in boundary equation
    UNION,
    INTERSECT,
    DIFF,
    XFORM
  } RegionOp;

  //
  // Methods unique to CSG Description
  //

  // Add an analytic boundary
  vtkIdType AddBoundary(BoundaryType type, int numcoeffs, const double *coeffs);

  // Get a boundary description
  void GetBoundary(vtkIdType id, int *type, int *numcoeffs, double **coeffs) const;

  // Construct and add a region from a boundary
  vtkIdType AddRegion(vtkIdType bndId, RegionOp op);

  // Construct and add a region as a binary op of other regions
  vtkIdType AddRegion(vtkIdType regIdLeft, vtkIdType regIdRight, RegionOp op);

  // Construct and add a region as transform of another region 
  vtkIdType AddRegion(vtkIdType regId, const double *xform);

  // Get a region description
  void GetRegion(vtkIdType id, vtkIdType *id1, vtkIdType *id2,
                 RegionOp *op, double **xform) const;

  // Add a cell (really just a complete region expression)
  vtkIdType AddCell(vtkIdType regId);
  vtkIdType GetCellRegionId(vtkIdType cellId) const;

  //
  // Silo convenience functions
  //
  void AddBoundaries(int nbounds, const int *const typeflags, int lcoeffs,
                const double *const coeffs);
  void AddBoundaries(int nbounds, const int *const typeflags, int lcoeffs,
                const float *const coeffs);
  void AddRegions(int nregions, const int *const lids, const int *const rids,
                  const int *const typeflags,
                  int lxforms, const double *const xforms);
#if 0
  void AddRegions(int nregions, const int *const lids, const int *const rids,
                  const int *const typeflags,
                  int lxforms, const float *const xforms);
#endif

  void AddZones(int nzones, const int *const zoneIds);

protected:
  vtkCSGGrid();
  ~vtkCSGGrid();

  //
  // We put this in the protected part of the interface because
  // we want to force clients of the class to use the AddBroundary/
  // GetBoundary and AddRegion/GetRegion methods.
  //
  vtkSetObjectMacro(Boundaries,vtkImplicitFunctionCollection);
  vtkGetObjectMacro(Boundaries,vtkImplicitFunctionCollection);

  vtkSetObjectMacro(Regions,vtkImplicitFunctionCollection);
  vtkGetObjectMacro(Regions,vtkImplicitFunctionCollection);

  vtkSetObjectMacro(CellRegionIds,vtkIdTypeArray);
  vtkGetObjectMacro(CellRegionIds,vtkIdTypeArray);

  vtkImplicitFunctionCollection *Boundaries; // Just the equations, f(x,y) or f(x,y,z) = 0
                                             // In other words, just the analytic boundaries.
  vtkImplicitFunctionCollection *Regions;    // Regions created by picking sides ('<' or '>') of
                                             // boundaries and binary expressions of other regions 
  vtkIdTypeArray *CellRegionIds;             // Indices into Regions of the "completed" regions


  int numBoundaries;
  double *gridBoundaries;
  int numRegions;
  int *leftIds, *rightIds, *regTypeFlags;
  int numZones;
  int *gridZones;
private:


// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Fri Jun  9 12:54:36 PDT 2006
//    Re-order arguments to constructor to match declaration order (addresses
//    compiler warning).
//
//    Mark C. Miller, Thu Mar 22 19:09:43 PST 2007
//    Made AddCutZones return bool indicating if it indeed added anything.
//
// ****************************************************************************

class Box
{
public:
    typedef enum {
        LT_ZERO = -1,
        EQ_ZERO =  0,
        GT_ZERO = +1
    } FuncState;

    Box(double x, double X,
        double y, double Y,
        double z, double Z,
        const vector<int>& _zids,
        double g000, double g001, double g010, double g011,
        double g100, double g101, double g110, double g111)
        : x0(x),y0(y),z0(z),x1(X),y1(Y),z1(Z),
          f000(g000), f001(g001), f010(g010), f011(g011),
          f100(g100), f101(g101), f110(g110), f111(g111), zids(_zids) {};

    FuncState EvalBoxStateOfBoundary(const double *const a, double tol) const;

    bool IsFlatEnough2(const double *const a, int bndId, double tol);
    bool CanBeCut2(const double *const a, map<int,int>, double tol);

    static FuncState ValState2(double val)
        { return val > 0.0 ? GT_ZERO :  LT_ZERO; };
    static FuncState ValState3(double val)
        { return val > 0.0 ? GT_ZERO : val < 0.0 ? LT_ZERO : EQ_ZERO; };

    static bool SameState2(FuncState s1, FuncState s2)
    {
        if (s1 == LT_ZERO || s1 == EQ_ZERO)
        {
            if (s2 == LT_ZERO || s2 == EQ_ZERO)
                return true;
            return false;
        }
        else
        {
            if (s2 == GT_ZERO || s2 == EQ_ZERO)
                return true;
            return false;
        }
    }
    static bool SameState3(FuncState s1, FuncState s2)
        { return s1 == s2; } ;

    double Resolution() const
    {
        double xres = x1 - x0; 
        double yres = y1 - y0; 
        double zres = z1 - z0; 

        if (xres < yres)
        {
            if (xres < zres)
                return xres;
            return zres;
        }
        else
        {
            if (yres < zres)
                return yres;
            return zres;
        }
    };

    vector<Box*> Subdivide() const
    {
        vector<Box*> retval;
        double halfx = (x0 + x1) / 2.0;
        double halfy = (y0 + y1) / 2.0;
        double halfz = (z0 + z1) / 2.0;

        Box* box000 = new Box(x0, halfx, y0, halfy, z0, halfz, zids,
                                 f000, DBL_MAX, DBL_MAX, DBL_MAX,
                              DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX);
        Box* box001 = new Box(halfx, x1, y0, halfy, z0, halfz, zids,
                              DBL_MAX,    f001, DBL_MAX, DBL_MAX,
                              DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX);
        Box* box010 = new Box(x0, halfx, halfy, y1, z0, halfz, zids,
                              DBL_MAX, DBL_MAX,    f010, DBL_MAX,
                              DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX);
        Box* box011 = new Box(halfx, x1, halfy, y1, z0, halfz, zids,
                              DBL_MAX, DBL_MAX, DBL_MAX,    f011,
                              DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX);
        Box* box100 = new Box(x0, halfx, y0, halfy, halfz, z1, zids,
                              DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX,
                                 f100, DBL_MAX, DBL_MAX, DBL_MAX);
        Box* box101 = new Box(halfx, x1, y0, halfy, halfz, z1, zids,
                              DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX,
                              DBL_MAX,    f101, DBL_MAX, DBL_MAX);
        Box* box110 = new Box(x0, halfx, halfy, y1, halfz, z1, zids,
                              DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX,
                              DBL_MAX, DBL_MAX,    f110, DBL_MAX);
        Box* box111 = new Box(halfx, x1, halfy, y1, halfz, z1, zids,
                              DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX,
                              DBL_MAX, DBL_MAX, DBL_MAX,    f111);

        retval.push_back(box000);
        retval.push_back(box001);
        retval.push_back(box010);
        retval.push_back(box011);
        retval.push_back(box100);
        retval.push_back(box101);
        retval.push_back(box110);
        retval.push_back(box111);

        return retval;
    }
    vector<Box*> SubdivideX() const
    {
        vector<Box*> retval;
        double halfx = (x0 + x1) / 2.0;

        Box* box0 = new Box(x0, halfx, y0, y1, z0, z1, zids,
                            f000, DBL_MAX, f010, DBL_MAX,
                            f100, DBL_MAX, f110, DBL_MAX);
        Box* box1 = new Box(halfx, x1, y0, y1, z0, z1, zids,
                            DBL_MAX, f001, DBL_MAX, f011,
                            DBL_MAX, f101, DBL_MAX, f111);

        retval.push_back(box0);
        retval.push_back(box1);

        return retval;
    }
    vector<Box*> SubdivideY() const
    {
        vector<Box*> retval;
        double halfy = (y0 + y1) / 2.0;

        Box* box0 = new Box(x0, x1, y0, halfy, z0, z1, zids,
                            f000, f001, DBL_MAX, DBL_MAX,
                            f100, f101, DBL_MAX, DBL_MAX);
        Box* box1 = new Box(x0, x1, halfy, y1, z0, z1, zids,
                            DBL_MAX, DBL_MAX, f010, f011,
                            DBL_MAX, DBL_MAX, f110, f111);

        retval.push_back(box0);
        retval.push_back(box1);

        return retval;
    }
    vector<Box*> SubdivideZ() const
    {
        vector<Box*> retval;
        double halfz = (z0 + z1) / 2.0;

        Box* box0 = new Box(x0, x1, y0, y1, z0, halfz, zids,
                               f000,    f001,    f010,    f011,
                            DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX);
        Box* box1 = new Box(x0, x1, y0, y1, halfz, z1, zids,
                            DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX,
                               f100,    f101,    f110,    f111);

        retval.push_back(box0);
        retval.push_back(box1);

        return retval;
    }

    double *GetPoint(int i)
    {
        tmp[0] = (i & 01) ? x1 : x0;
        tmp[1] = (i & 02) ? y1 : y0;
        tmp[2] = (i & 04) ? z1 : z0;
        return tmp;
    }

    double GetFunctionValue(vtkImplicitFunction *func, int i)
    {
        switch (i)
        {
            case 0:
            {
                if (f000 != DBL_MAX)
                    return f000;
                f000 = func->FunctionValue(GetPoint(i));
                return f000;
            }
            case 1:
            {
                if (f001 != DBL_MAX)
                    return f001;
                f001 = func->FunctionValue(GetPoint(i));
                return f001;
            }
            case 2:
            {
                if (f010 != DBL_MAX)
                    return f010;
                f010 = func->FunctionValue(GetPoint(i));
                return f010;
            }
            case 3:
            {
                if (f011 != DBL_MAX)
                    return f011;
                f011 = func->FunctionValue(GetPoint(i));
                return f011;
            }
            case 4:
            {
                if (f100 != DBL_MAX)
                    return f100;
                f100 = func->FunctionValue(GetPoint(i));
                return f100;
            }
            case 5:
            {
                if (f101 != DBL_MAX)
                    return f101;
                f101 = func->FunctionValue(GetPoint(i));
                return f101;
            }
            case 6:
            {
                if (f110 != DBL_MAX)
                    return f110;
                f110 = func->FunctionValue(GetPoint(i));
                return f110;
            }
            case 7:
            {
                if (f111 != DBL_MAX)
                    return f111;
                f111 = func->FunctionValue(GetPoint(i));
                return f111;
            }
        }
        return 0.0;
    }

    double x0,y0,z0,x1,y1,z1;
    double f000,f001,f010,f011,f100,f101,f110,f111;
    double tmp[3];
    vector<int> zids;
};

  static bool AddCutZones(vtkUnstructuredGrid *cutBox, vtkPoints *points,
                           vtkUnstructuredGrid *ugrid,
                           map<float, map<float, map<float, int> > >& nodemap);
  static void MakeMeshZone(const Box *aBox, vtkPoints *points,
                           vtkUnstructuredGrid *ugrid,
                           map<float, map<float, map<float, int> > >& nodemap);
  bool MakeMeshZonesByCuttingBox4(const Box *aBox,
                           const map<int,int>& boundaryToStateMap,
                           map<int,int>& boundaryToSenseMap, int zoneId,
                           vtkPoints *points, vtkUnstructuredGrid *ugrid,
                           map<float, map<float, map<float, int> > >& nodemap);
  bool MakeMeshZonesByCuttingBox2(const Box *aBox,
                           const map<int,int>& boundaryToStateMap,
                           map<int,int>& boundaryToSenseMap, int zoneId,
                           vtkPoints *points, vtkUnstructuredGrid *ugrid,
                           map<float, map<float, map<float, int> > >& nodemap);
  static void MakeMeshZonesByCuttingBox(const Box *aBox,
                           map<vtkImplicitFunction*,Box::FuncState> funcToStateMap,
                           vector<RegionOp> senses,
                           vtkPoints *points, vtkUnstructuredGrid *ugrid,
                           map<float, map<float, map<float, int> > >& nodemap);
  void AddBoundariesForZone2(int, vector<int> *bnds, vector<int> *senses);
  void AddBoundariesForZone(vtkImplicitFunction *func,
                                   vector<vtkImplicitFunction*> *bnds,
                                   vector<RegionOp> *senses);
  int EvalBoxStateOfRegion(const Box *const curBox, int regId,
        map<int,int>& boundaryToStateMap, double tol);

  double tmpFloats[32];                       // temporary storage to help satisfy interface
                                             //    requirements of vtkDataSet

  vtkPlanes *Universe;                       // The "universe" set (a maximally sized box)

  map<vtkImplicitFunction *, vtkIdType> funcMap;

  vtkImplicitFunction *GetBoundaryFunc(vtkIdType id) const;
  vtkImplicitFunction *GetRegionFunc(vtkIdType id) const;

  vtkCSGGrid(const vtkCSGGrid&);             // Not implemented.
  void operator=(const vtkCSGGrid&);         // Not implemented.
};


inline vtkIdType vtkCSGGrid::GetNumberOfPoints()
{
  vtkErrorMacro("GetNumberOfPoints() means GetNumberOfBoundaries()");
  vtkErrorMacro("Use GetNumberOfBoundaries() to avoid this message");
  return GetNumberOfBoundaries();
};

inline vtkIdType vtkCSGGrid::GetNumberOfBoundaries() const
{
  return (vtkIdType) this->Boundaries->GetNumberOfItems();
};

inline vtkIdType vtkCSGGrid::GetNumberOfCells() 
{
  return CellRegionIds->GetNumberOfTuples();
}

#endif
