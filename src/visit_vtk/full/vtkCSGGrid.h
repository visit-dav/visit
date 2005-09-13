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

#include "vtkDataArray.h"
#include "vtkIdTypeArray.h"
#include "vtkDataSet.h"
#include "vtkImplicitFunctionCollection.h"
#include "vtkPlanes.h"
#include "vtkStructuredData.h"

class vtkPolyData;

using std::map;

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
  float *GetPoint(vtkIdType ptId);
  float *GetBoundary(vtkIdType bndId) const;
  void GetPoint(vtkIdType id, float x[3]);

  vtkCell *GetCell(vtkIdType cellId);
  void GetCell(vtkIdType cellId, vtkGenericCell *cell);
  void GetCellBounds(vtkIdType cellId, float bounds[6]);
  int FindPoint(float x, float y, float z) { return this->vtkDataSet::FindPoint(x, y, z);};
  vtkIdType FindPoint(float x[3]);
  vtkIdType FindCell(float x[3], vtkCell *cell, vtkIdType cellId, float tol2,
                     int& subId, float pcoords[3], float *weights);
  vtkIdType FindCell(float x[3], vtkCell *cell, vtkGenericCell *gencell,
                     vtkIdType cellId, float tol2, int& subId, 
                     float pcoords[3], float *weights);
  vtkCell *FindAndGetCell(float x[3], vtkCell *cell, vtkIdType cellId, 
                          float tol2, int& subId, float pcoords[3],
                          float *weights);
  int GetCellType(vtkIdType cellId);
  void GetCellPoints(vtkIdType cellId, vtkIdList *ptIds);
  void GetPointCells(vtkIdType ptId, vtkIdList *cellIds);
  void ComputeBounds();
  int GetMaxCellSize();
  void GetCellNeighbors(vtkIdType cellId, vtkIdList *ptIds,
                        vtkIdList *cellIds);


  //
  // A discretize method that returns the surfaces only
  //
  vtkPolyData  *DiscretizeSurfaces(int specificZone = -1,
                                   double minX = -10.0, double maxX = 10.0, int nX = 100,
                                   double minY = -10.0, double maxY = 10.0, int nY = 100,
                                   double minZ = -10.0, double maxZ = 10.0, int nZ = 100);

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

private:
  float tmpFloats[32];                       // temporary storage to help satisfy interface
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
