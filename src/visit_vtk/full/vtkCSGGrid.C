#include "vtkCSGGrid.h"
#include <vtkAppendFilter.h>
#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCone.h>
#include <vtkContourFilter.h>
#include <vtkCylinder.h>
#include <vtkExtentTranslator.h>
#include <vtkIdTypeArray.h>
#include <vtkImplicitBoolean.h>
#include <vtkGenericCell.h>
#include <vtkImplicitBoolean.h>
#include <vtkImplicitFunctionCollection.h>
#include <vtkLine.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPlanes.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkQuadric.h>
#include <vtkSampleFunction.h>
#include <vtkSphere.h>
#include <vtkTransform.h>
#include <vtkUnstructuredGrid.h>

#include <vtkVisItClipper.h>

#include <deque>
#include <map>
#include <vector>

using std::deque;
using std::map;
using std::vector;

#ifdef _WIN32
#define M_PI 3.14159265358979323846
#endif


static
double ComputeRelativeTol(double absTol,
                          double minX, double maxX,
                          double minY, double maxY,
                          double minZ, double maxZ)
{
    //
    // Turn relative tolerance into an absolute tolerance
    //
    if ((maxX - minX) > (maxY - minY))
    {
        if ((maxX - minX) > (maxZ - minZ))
            absTol *= (maxX - minX);
        else
            absTol *= (maxZ - minZ);
    }
    else
    {
        if ((maxY - minY) > (maxZ - minZ))
            absTol *= (maxY - minY);
        else
            absTol *= (maxZ - minZ);
    }
    return absTol;
}

vtkCxxRevisionMacro(vtkCSGGrid, "$Revision: 1.53 $");
vtkStandardNewMacro(vtkCSGGrid);

vtkCSGGrid::Box::FuncState
vtkCSGGrid::Box::EvalFuncState(vtkImplicitFunction *func, double tol)
{
    int i,j;

    const char *funcName = func->GetClassName();

    //
    // Acceleration code for spheres
    //
    if (strcmp(funcName, "vtkSphere") == 0)
    {
        vtkSphere *sphere = vtkSphere::SafeDownCast(func);

        const double *p = sphere->GetCenter();
        const double r = sphere->GetRadius();
        //float r2 = r*r;
        float r2 = r;

        float sx = x1 - x0;
        float sy = y1 - y0;
        float sz = z1 - z0;
        //float s2 = (sx*sx + sy*sy + sz*sz) / 4.0;
        float s2 = sqrt((sx*sx + sy*sy + sz*sz) / 2.0);

        float dx = (x1 + x0) / 2.0 - p[0];
        float dy = (y1 + y0) / 2.0 - p[1];
        float dz = (z1 + z0) / 2.0 - p[2];
        //float d2 = dx*dx + dy*dy + dz*dz;
        float d2 = sqrt(dx*dx + dy*dy + dz*dz);

        if (r2 > s2)
        {
            if (d2 > r2 + s2)
                return GT_ZERO;
            else if (d2 < r2 - s2)
                return LT_ZERO;
            else
                return EQ_ZERO;
        }
        else
        {
            if (d2 > s2 + r2)
                return GT_ZERO;
            else
                return EQ_ZERO;
        }
    }

    FuncState firstState = ValState2(GetFunctionValue(func, 0));

    //
    // First, lets examine function values at the 8 corners to see if
    // we've got a region boundary inside the box
    //
    for (i = 1; i < 8; i++)
    {
        if (!SameState2(firstState, ValState2(GetFunctionValue(func, i))))
            return EQ_ZERO;
    }

    if (Resolution() < tol)
        return firstState;

    //
    // If we reach here, then all 8 corners of the box have the same
    // state. So, now we examine the box's derivatives in each of
    // the coordinate axes' directions (e.g. the gradient)
    //
    double *firstGrad = func->FunctionGradient(GetPoint(0));
    FuncState firstStateVec[3];
    for (j = 0; j < 3; j++)
        firstStateVec[j] = ValState2(firstGrad[j]);
    for (i = 1; i < 8; i++)
    {
        double *grad = func->FunctionGradient(GetPoint(i));
        for (j = 0; j < 3; j++)
        {
            if (!SameState2(firstStateVec[j], ValState2(grad[j])))
                return EQ_ZERO;
        }
    }

    return firstState;

}

// internal type used only for implementation
typedef enum {
    FUNC_BOOLEAN,
    FUNC_CONE,
    FUNC_CYLINDER,
    FUNC_PLANE,
    FUNC_MULTIFUNC_PLANE,
    FUNC_QUADRIC,
    FUNC_SPHERE,
    FUNC_UNKNOWN_IMPLICIT
} ImplicitFuncType;

static ImplicitFuncType GetImplicitFuncType(const vtkObject *obj)
{
    const char *className = obj->GetClassName();
    if      (strcmp(className, "vtkImplicitBoolean") == 0)
        return FUNC_BOOLEAN;
    else if (strcmp(className, "vtkCone") == 0)
        return FUNC_CONE;
    else if (strcmp(className, "vtkCylinder") == 0)
        return FUNC_CYLINDER;
    else if (strcmp(className, "vtkPlane") == 0)
        return FUNC_PLANE;
    else if (strcmp(className, "vtkPlanes") == 0)
        return FUNC_MULTIFUNC_PLANE;
    else if (strcmp(className, "vtkQuadric") == 0)
        return FUNC_QUADRIC;
    else if (strcmp(className, "vtkSphere") == 0)
        return FUNC_SPHERE;
    else
        return FUNC_UNKNOWN_IMPLICIT; 
}

//----------------------------------------------------------------------------
vtkCSGGrid::vtkCSGGrid()
{
  this->Boundaries = vtkImplicitFunctionCollection::New();
  this->Regions    = vtkImplicitFunctionCollection::New();
  this->CellRegionIds = vtkIdTypeArray::New();

  // setup the Universe set
  this->Universe   = vtkPlanes::New();
  this->Universe->SetBounds(-FLT_MAX, FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX, FLT_MAX);
  funcMap[Universe] = -1;
}

//----------------------------------------------------------------------------
vtkCSGGrid::~vtkCSGGrid()
{
  this->Initialize();
  this->Universe->Delete();
}

//----------------------------------------------------------------------------
void vtkCSGGrid::Initialize()
{
  vtkDataSet::Initialize();

  if ( this->Boundaries ) 
    {
    this->Boundaries->UnRegister(this);
    this->Boundaries = NULL;
    }

  if ( this->Regions ) 
    {
    this->Regions->UnRegister(this);
    this->Regions = NULL;
    }

  if ( this->CellRegionIds ) 
    {
    this->CellRegionIds->UnRegister(this);
    this->CellRegionIds = NULL;
    }
}

//----------------------------------------------------------------------------
// Copy the geometric and topological structure of an input rectilinear grid
// object.
void vtkCSGGrid::CopyStructure(vtkDataSet *ds)
{
  vtkCSGGrid *csgGrid=(vtkCSGGrid *)ds;
  this->Initialize();

  this->SetBoundaries(csgGrid->GetBoundaries());
  this->SetRegions(csgGrid->GetRegions());
  this->SetCellRegionIds(csgGrid->GetCellRegionIds());
}

//----------------------------------------------------------------------------
vtkCell *vtkCSGGrid::GetCell(vtkIdType cellId)
{
//#warning GetCell NOT IMPLEMENTED
  return NULL;
}

//----------------------------------------------------------------------------
void vtkCSGGrid::GetCell(vtkIdType cellId, vtkGenericCell *cell)
{
//#warning GetCell NOT IMPLEMENTED
  cell->SetCellTypeToEmptyCell();
}

int vtkCSGGrid::GetCellType(vtkIdType cellId)
{
//#warning GetCellType NOT IMPLEMENTED
  return VTK_EMPTY_CELL;
}

int vtkCSGGrid::GetMaxCellSize()
{
//#warning GetMaxCellSize NOT IMPLEMENTED
  return -1;
}

//----------------------------------------------------------------------------
double *vtkCSGGrid::GetPoint(vtkIdType ptId)
{
  vtkErrorMacro("For a vtkCSGGrid, GetPoint() means GetBoundary()");
  vtkErrorMacro("Use GetBoundary() to avoid this message");
  int dummy, n;
  double *p = 0;
  this->GetBoundary(ptId, &dummy, &n, &p);
  const int k = sizeof(tmpFloats) / sizeof(double);
  int m = n;
  if (n >= k)
  {
      vtkErrorMacro("GetPoint() too many coefficients");
      m = k;
  }
  for (int i = 0; i < m; i++)
      tmpFloats[i] = p[i];
  delete [] p;
  return tmpFloats;
}

//----------------------------------------------------------------------------
// Fast implementation of GetCellBounds().  Bounds are calculated without
// constructing a cell.
void vtkCSGGrid::GetCellBounds(vtkIdType cellId, double bounds[6])
{
//#warning GetCellBounds NOT IMPLEMENTED
  return;
}

void vtkCSGGrid::GetPoint(vtkIdType ptId, double x[3])
{
  vtkErrorMacro("Requesting a point[3] from a vtkCSGGrid");
  vtkErrorMacro("Use GetBoundary() to avoid this message");
  x[0] = x[1] = x[2] = 0.0;
  return;
}

//----------------------------------------------------------------------------
vtkIdType vtkCSGGrid::FindPoint(double x[3])
{
  vtkErrorMacro("Finding a point on a vtkCSGGrid not yet implemented.");
  return -1;
}

vtkIdType vtkCSGGrid::FindCell(double x[3], vtkCell *vtkNotUsed(cell), 
                                       vtkGenericCell *vtkNotUsed(gencell),
                                       vtkIdType vtkNotUsed(cellId), 
                                       double vtkNotUsed(tol2), 
                                       int& subId, double pcoords[3], 
                                       double *weights)
{
  return
    this->FindCell( x, (vtkCell *)NULL, 0, 0.0, subId, pcoords, weights );
}

//----------------------------------------------------------------------------
vtkIdType vtkCSGGrid::FindCell(double x[3], vtkCell *vtkNotUsed(cell), 
                                       vtkIdType vtkNotUsed(cellId),
                                       double vtkNotUsed(tol2), 
                                       int& subId, double pcoords[3],
                                       double *weights)
{
//#warning FindCell NOT IMPLEMENTED
  return -1;
}

//----------------------------------------------------------------------------
vtkCell *vtkCSGGrid::FindAndGetCell(double x[3],
                                            vtkCell *vtkNotUsed(cell), 
                                            vtkIdType vtkNotUsed(cellId),
                                            double vtkNotUsed(tol2),
                                            int& subId, 
                                            double pcoords[3], double *weights)
{
//#warning FindAndGetCell NOT IMPLEMENTED
  return NULL;
#if 0
  int loc[3];
  vtkIdType cellId;
  
  subId = 0;
  if ( this->ComputeStructuredCoordinates(x, loc, pcoords) == 0 )
    {
    }
  //
  // Get the parametric coordinates and weights for interpolation
  //
  vtkVoxel::InterpolationFunctions(pcoords,weights);
  //
  // Get the cell
  //
  cellId = loc[2] * (this->Dimensions[0]-1)*(this->Dimensions[1]-1) +
           loc[1] * (this->Dimensions[0]-1) + loc[0];

  return vtkCSGGrid::GetCell(cellId);
#endif
}

//----------------------------------------------------------------------------
void vtkCSGGrid::GetCellPoints(vtkIdType cellId, vtkIdList *ptIds)
{
//#warning CellPoints NOT IMPLEMENTED
}

//----------------------------------------------------------------------------
void vtkCSGGrid::GetPointCells(vtkIdType ptId, vtkIdList *cellIds)
{
//#warning GetPointCells NOT IMPLEMENTED
}


//----------------------------------------------------------------------------
void vtkCSGGrid::ComputeBounds()
{
}

static unsigned long GetActualMemorySizeOfImplicitFunc(vtkImplicitFunction *func)
{
    unsigned long size = 0;

    switch (GetImplicitFuncType(func))
    {
        case FUNC_BOOLEAN:
        {
            vtkImplicitBoolean *boolFunc = vtkImplicitBoolean::SafeDownCast(func);
            vtkImplicitFunctionCollection *funcs = boolFunc->GetFunction();
            vtkImplicitFunction *leftFunc =
                vtkImplicitFunction::SafeDownCast(funcs->GetItemAsObject(0));
            vtkImplicitFunction *rightFunc =
                vtkImplicitFunction::SafeDownCast(funcs->GetItemAsObject(1));
            size += GetActualMemorySizeOfImplicitFunc(leftFunc);
            size += GetActualMemorySizeOfImplicitFunc(rightFunc);
            break;
        }
        case FUNC_CONE:     size += sizeof(vtkCone);     break;
        case FUNC_CYLINDER: size += sizeof(vtkCylinder); break;
        case FUNC_PLANE:    size += sizeof(vtkPlane);    break;
        case FUNC_SPHERE:   size += sizeof(vtkSphere);   break;
        case FUNC_QUADRIC:  size += sizeof(vtkQuadric);  break;
        case FUNC_MULTIFUNC_PLANE:
        {
            vtkPlanes *planes = vtkPlanes::SafeDownCast(func);
            size += planes->GetPoints()->GetActualMemorySize();
            size += planes->GetNormals()->GetActualMemorySize();
            size += planes->GetNumberOfPlanes() * sizeof(vtkPlane);
            break;
        }
        case FUNC_UNKNOWN_IMPLICIT:
        default:       size += 0; break;
    }
    return size;
}

//----------------------------------------------------------------------------
unsigned long vtkCSGGrid::GetActualMemorySize()
{
    int i;
    unsigned long size=this->vtkDataSet::GetActualMemorySize();

    if (this->Boundaries)
    {
        for (i = 0; i < this->Boundaries->GetNumberOfItems(); i++)
        {
            vtkImplicitFunction *func =
                vtkImplicitFunction::SafeDownCast(this->Boundaries->GetItemAsObject(i));
            size += GetActualMemorySizeOfImplicitFunc(func); 
        }
    }

    if (this->Regions)
    {
        for (i = 0; i < this->Regions->GetNumberOfItems(); i++)
        {
            vtkImplicitFunction *func =
                vtkImplicitFunction::SafeDownCast(this->Regions->GetItemAsObject(i));
            size += GetActualMemorySizeOfImplicitFunc(func); 
        }
    }

    if (this->CellRegionIds)
    {
        size += this->CellRegionIds->GetActualMemorySize();
    }

    return size;

}

//----------------------------------------------------------------------------
void vtkCSGGrid::GetCellNeighbors(vtkIdType cellId, vtkIdList *ptIds,
                                          vtkIdList *cellIds)
{

//#warning GetCellNeighbors NOT IMPLEMENTED
  cellIds->Reset();
  return;
}

//----------------------------------------------------------------------------
void vtkCSGGrid::ShallowCopy(vtkDataObject *dataObject)
{
  vtkCSGGrid *grid = vtkCSGGrid::SafeDownCast(dataObject);

  if ( grid != NULL )
    {
    this->SetBoundaries(grid->GetBoundaries());
    this->SetRegions(grid->GetRegions());
    this->SetCellRegionIds(grid->GetCellRegionIds());
    }

  // Do superclass
  this->vtkDataSet::ShallowCopy(dataObject);
}

//----------------------------------------------------------------------------
void vtkCSGGrid::DeepCopy(vtkDataObject *srcObject)
{
  vtkCSGGrid *grid = vtkCSGGrid::SafeDownCast(srcObject);

  if ( grid != NULL )
    {
    vtkImplicitFunction *next;
    vtkImplicitFunctionCollection *dstColl, *srcColl;
    
    // copy boundaries
    dstColl = vtkImplicitFunctionCollection::New();
    srcColl = grid->GetBoundaries();
    srcColl->InitTraversal();
    while ((next = srcColl->GetNextItem()) != NULL)
        dstColl->AddItem(next);
    this->SetBoundaries(dstColl);
    dstColl->Delete();

    // copy regions
    dstColl = vtkImplicitFunctionCollection::New();
    srcColl = grid->GetRegions();
    srcColl->InitTraversal();
    while ((next = srcColl->GetNextItem()) != NULL)
        dstColl->AddItem(next);
    this->SetRegions(dstColl);
    dstColl->Delete();

    // copy cell region ids
    vtkIdTypeArray *s = vtkIdTypeArray::New();
    s->DeepCopy(grid->GetCellRegionIds());
    this->SetCellRegionIds(s);
    s->Delete();
    }

  // Do superclass
  this->vtkDataSet::DeepCopy(srcObject);
}

//----------------------------------------------------------------------------
void vtkCSGGrid::PrintSelf(ostream& os, vtkIndent indent)
{
    int i;
    vtkImplicitFunction *func;

    this->Superclass::PrintSelf(os,indent);

    os << indent << "Contents of \"" << GetClassName() << "\"..." << endl;
    os << indent << "Number of Boundaries = " << Boundaries->GetNumberOfItems() << endl;
    os << indent << "Number of Regions    = " << Regions->GetNumberOfItems() << endl;
    os << indent << "Number of Cells      = " << CellRegionIds->GetNumberOfTuples() << endl;
    os << endl;

    //
    // Print the boundaries
    //
    os << indent << "Boundaries:" << endl;
    for (i = 0; i < Boundaries->GetNumberOfItems(); i++)
    {
        vtkIndent indent2 = indent.GetNextIndent();
        os << indent2 << "Item: " << i;

        func = GetBoundaryFunc(i);
        switch (GetImplicitFuncType(func))
        {
            case FUNC_CYLINDER:
            {
                os << ", is a cylinder" << endl;
                vtkCylinder *cylinder = vtkCylinder::SafeDownCast(func);
                  cylinder->PrintSelf(os, indent2.GetNextIndent());
                break;
            }
            case FUNC_PLANE:
            {
                os << ", is a plane" << endl;
                vtkPlane *plane = vtkPlane::SafeDownCast(func);
                  plane->PrintSelf(os, indent2.GetNextIndent());
                break;
            }
            case FUNC_SPHERE:
            {
                os << ", is a sphere" << endl;
                vtkSphere *sphere = vtkSphere::SafeDownCast(func);
                  sphere->PrintSelf(os, indent2.GetNextIndent());
                break;
            }
            case FUNC_QUADRIC:
            {
                os << ", is a quadric" << endl;
                vtkQuadric *quadric = vtkQuadric::SafeDownCast(func);
                  quadric->PrintSelf(os, indent2.GetNextIndent());
                break;
            }
        }
    }

    //
    // Print the regions
    //
    os << indent << "Regions:" << endl;
    for (i = 0; i < Regions->GetNumberOfItems(); i++)
    {
        vtkIndent indent2 = indent.GetNextIndent();
        os << indent2 << "Item: " << i;

        func = GetRegionFunc(i);
        switch (GetImplicitFuncType(func))
        {
            case FUNC_BOOLEAN:
            {
                vtkImplicitBoolean *boolFunc = vtkImplicitBoolean::SafeDownCast(func);
                vtkImplicitFunctionCollection *funcs = boolFunc->GetFunction();
                vtkImplicitFunction *leftFunc =
                    vtkImplicitFunction::SafeDownCast(funcs->GetItemAsObject(0));
                vtkImplicitFunction *rightFunc =
                    vtkImplicitFunction::SafeDownCast(funcs->GetItemAsObject(1));

                if (leftFunc == Universe)
                {
                    switch (GetImplicitFuncType(rightFunc))
                    {
                        case FUNC_CYLINDER:
                        {
                            os << ", is the OUTER of a cylinder" << endl;
                            vtkCylinder *cylinder = vtkCylinder::SafeDownCast(rightFunc);
                              cylinder->PrintSelf(os, indent2.GetNextIndent());
                            break;
                        }
                        case FUNC_SPHERE:
                        {
                            os << ", is the OUTER of a sphere" << endl;
                            vtkSphere *sphere = vtkSphere::SafeDownCast(rightFunc);
                              sphere->PrintSelf(os, indent2.GetNextIndent());
                            break;
                        }
                        case FUNC_QUADRIC:
                        {
                            os << ", is the OUTER of a quadric" << endl;
                            vtkQuadric *quadric = vtkQuadric::SafeDownCast(rightFunc);
                              quadric->PrintSelf(os, indent2.GetNextIndent());
                            break;
                        }
                    }
                }
                else
                {
                    os << ", is a boolean ";
                    switch (boolFunc->GetOperationType())
                    {
                        case VTK_INTERSECTION: os << "intersection "; break;
                        case VTK_UNION:        os << "union "; break;
                        case VTK_DIFFERENCE:   os << "difference "; break;
                    }

                    vtkIdType leftId  = funcMap[leftFunc];
                    vtkIdType rightId = funcMap[rightFunc];

                    os << "of items " << leftId << " and " << rightId << endl;
                }
                break;
            }
            case FUNC_CYLINDER:
            {
                os << ", is the INNER of a cylinder" << endl;
                vtkCylinder *cylinder = vtkCylinder::SafeDownCast(func);
                  cylinder->PrintSelf(os, indent2.GetNextIndent());
                break;
            }
            case FUNC_PLANE:
            {
                os << ", is the INNER of a plane" << endl;
                vtkPlane *plane = vtkPlane::SafeDownCast(func);
                  plane->PrintSelf(os, indent2.GetNextIndent());
                break;
            }
            case FUNC_SPHERE:
            {
                os << ", is the INNER of a sphere" << endl;
                vtkSphere *sphere = vtkSphere::SafeDownCast(func);
                  sphere->PrintSelf(os, indent2.GetNextIndent());
                break;
            }
            case FUNC_QUADRIC:
            {
                os << ", is the INNER of a quadric" << endl;
                vtkQuadric *quadric = vtkQuadric::SafeDownCast(func);
                  quadric->PrintSelf(os, indent2.GetNextIndent());
                break;
            }
        }
    }
}

//----------------------------------------------------------------------------
vtkIdType vtkCSGGrid::AddBoundary(BoundaryType type, int numcoeffs,
                                  const double *coeffs)
{
    vtkImplicitFunction *newBoundary = 0;

    switch (type)
    {
        case SPHERE_PR:
        {
            vtkSphere *sphere = vtkSphere::New();

            sphere->SetCenter(const_cast<double*>(coeffs));
            sphere->SetRadius(coeffs[3]);

            newBoundary = sphere;
            break;
        }

        case CYLINDER_PNLR:
        {
            vtkCylinder *cylinder = vtkCylinder::New();

            // if the desired cylinder is y-axis aligned, we don't need xform
            if ((coeffs[3] == 0.0 && coeffs[4] ==  1.0 && coeffs[5] == 0.0) ||
                (coeffs[3] == 0.0 && coeffs[4] == -1.0 && coeffs[5] == 0.0))
            {
                cylinder->SetCenter(coeffs[0],
                                    coeffs[1] + coeffs[4] * coeffs[6]/2.0,
                                    coeffs[2]);
            }
            else
            {
                double Nx = coeffs[3], Ny = coeffs[4], Nz = coeffs[5];
                double rotz = acos(Ny) * 180.0 / M_PI;
                double roty = acos(Nx/sqrt(1-Ny*Ny)) * 180.0 / M_PI;

                vtkTransform *xform = vtkTransform::New();
                xform->RotateZ(-rotz);
                xform->RotateY(-roty);
                xform->Translate(-coeffs[0], -coeffs[1], -coeffs[2]);
//#warning were not setting length

                cylinder->SetTransform(xform);
                xform->Delete();
            }
            cylinder->SetRadius((float) coeffs[7]);

            newBoundary = cylinder;
            break;
        }

        case PLANE_X:
        {
            vtkPlane *plane = vtkPlane::New();

            plane->SetOrigin(coeffs[0], 0.0, 0.0);
            plane->SetNormal(1.0, 0.0, 0.0);

            newBoundary = plane;
            break;
        }

        case PLANE_Y:
        {
            vtkPlane *plane = vtkPlane::New();

            plane->SetOrigin(0.0, coeffs[0], 0.0);
            plane->SetNormal(0.0, 1.0, 0.0);

            newBoundary = plane;
            break;
        }

        case PLANE_Z:
        {
            vtkPlane *plane = vtkPlane::New();

            plane->SetOrigin(0.0, 0.0, coeffs[0]);
            plane->SetNormal(0.0, 0.0, 1.0);

            newBoundary = plane;
            break;
        }

        case QUADRIC_G:
        {
            vtkQuadric *quadric = vtkQuadric::New();
            quadric->SetCoefficients( coeffs[0], // x^2 term
                                      coeffs[1], // y^2 term
                                      coeffs[2], // z^2 term
                                      coeffs[3], // xy term
                                      coeffs[4], // yz term
                                      coeffs[5], // xz term
                                      coeffs[6], // x^1 term
                                      coeffs[7], // y^1 term
                                      coeffs[8], // z^1 term
                                      coeffs[9]); // constant term

            newBoundary = quadric;
            break;
        }

    } // switch

    if (newBoundary)
    {
        Boundaries->AddItem(newBoundary);
        newBoundary->Delete();
        funcMap[newBoundary] = Boundaries->GetNumberOfItems()-1;
        return funcMap[newBoundary]; 
    }

    return -1;
}

//----------------------------------------------------------------------------
vtkImplicitFunction *vtkCSGGrid::GetBoundaryFunc(vtkIdType id) const
{
    if (id >= 0 && id < Boundaries->GetNumberOfItems())
        return vtkImplicitFunction::SafeDownCast(Boundaries->GetItemAsObject(id));
    cerr << "id \"" << id << "\" out of range \"" << Boundaries->GetNumberOfItems() << "\"" << endl;
    return 0;
}

//----------------------------------------------------------------------------
vtkImplicitFunction *vtkCSGGrid::GetRegionFunc(vtkIdType id) const
{
    if (id >= 0 && id < Regions->GetNumberOfItems())
        return vtkImplicitFunction::SafeDownCast(Regions->GetItemAsObject(id));
    cerr << "id \"" << id << "\" out of range \"" << Regions->GetNumberOfItems() << "\"" << endl;
    return 0;
}

//----------------------------------------------------------------------------
void vtkCSGGrid::GetBoundary(vtkIdType id, int *type, int *numcoeffs,
                             double **coeffs) const
{
//#warning GetBoundary NOT IMPLEMENTED
}

//----------------------------------------------------------------------------
vtkIdType vtkCSGGrid::AddRegion(vtkIdType bndId, RegionOp op)
{
    vtkImplicitFunction *newRegion = 0;
    vtkImplicitFunction *bnd = GetBoundaryFunc(bndId);

    //
    // INNER means replace the '=' in the equation of the boundary with '<'
    // OUTER means replace the '=' in the equation of the boundary with '>'
    // For planes, INNER means the side against the normal while OUTER 
    // means the side with the normal.
    //

    //
    // For closed objects like sphere and cylinder, OUTER means we need
    // to compliment it. For planes, we need to invert the normal.
    // To make a compliment, we do a diff with Universe.
    //

    bool doBoolDiff = false;
    switch (GetImplicitFuncType(bnd))
    {
        case FUNC_SPHERE:
        {
            //
            // Copy the sphere boundary
            //
            vtkSphere *sphereBnd = vtkSphere::SafeDownCast(bnd);
            vtkSphere *sphereReg = vtkSphere::New();
            sphereReg->SetCenter(sphereBnd->GetCenter());
            sphereReg->SetRadius(sphereBnd->GetRadius());

            if (op == OUTER)
                doBoolDiff = true;

            newRegion = sphereReg;
            break;
        }
        case FUNC_PLANE:
        {
            //
            // Copy the plane boundary
            //
            vtkPlane *planeBnd = vtkPlane::SafeDownCast(bnd);
            vtkPlane *planeReg = vtkPlane::New();
            planeReg->SetOrigin(planeBnd->GetOrigin());

            //
            // Invert the normal for the OUTER case
            //
            if (op == OUTER)
            {
                planeReg->SetNormal(-planeBnd->GetNormal()[0],
                                    -planeBnd->GetNormal()[1],
                                    -planeBnd->GetNormal()[2]);
            }
            else
            {
                planeReg->SetNormal(planeBnd->GetNormal());
            }

            newRegion = planeReg;
            break;
        }
        case FUNC_CYLINDER:
        {
            //
            // Copy the cylinder boundary
            //
            vtkCylinder *cylBnd = vtkCylinder::SafeDownCast(bnd);
            vtkCylinder *cylReg = vtkCylinder::New();
            cylReg->SetCenter(cylBnd->GetCenter());
            cylReg->SetRadius(cylBnd->GetRadius());

            if (op == OUTER)
                doBoolDiff = true;

            newRegion = cylReg;
            break;
        }

        case FUNC_QUADRIC:
        {
            //
            //
            //
            vtkQuadric *quadricBnd = vtkQuadric::SafeDownCast(bnd);
            vtkQuadric *quadricReg = vtkQuadric::New();
            quadricReg->SetCoefficients(quadricBnd->GetCoefficients());

            if (op == OUTER)
                doBoolDiff = true;

            newRegion = quadricReg;
            break;
        }

    } // switch

    if (newRegion)
    {
        //
        // Copy the xform if there is one 
            //
        vtkTransform *bndXform = vtkTransform::SafeDownCast(bnd->GetTransform());
        if (bndXform)
        {
            vtkTransform *regXform = vtkTransform::New();
            regXform->DeepCopy(bndXform);
            newRegion->SetTransform(regXform);
            regXform->Delete();
        }

        //
        // Diff with universe if necessary
        //
        if (doBoolDiff)
        {
            vtkImplicitBoolean *notReg = vtkImplicitBoolean::New();
            notReg->SetOperationTypeToDifference();
            notReg->AddFunction(Universe);
            notReg->AddFunction(newRegion);
            newRegion->Delete();
            newRegion = notReg;
        }

        Regions->AddItem(newRegion);
        newRegion->Delete();
        funcMap[newRegion] = Regions->GetNumberOfItems()-1;
        return funcMap[newRegion]; 
    }

    return -1;
}

//----------------------------------------------------------------------------
vtkIdType vtkCSGGrid::AddRegion(vtkIdType regIdLeft, vtkIdType regIdRight,
                                RegionOp op)
{
    vtkImplicitBoolean *boolReg = vtkImplicitBoolean::New();

    vtkImplicitFunction *left = GetRegionFunc(regIdLeft);
    vtkImplicitFunction *right = GetRegionFunc(regIdRight);

    switch (op)
    {
        case INTERSECT: boolReg->SetOperationTypeToIntersection(); break;
        case UNION:     boolReg->SetOperationTypeToUnion();        break;
        case DIFF:      boolReg->SetOperationTypeToDifference();   break;
    }

    boolReg->AddFunction(left);
    boolReg->AddFunction(right);

    Regions->AddItem(boolReg);
    boolReg->Delete();
    funcMap[boolReg] = Regions->GetNumberOfItems()-1;
    return funcMap[boolReg]; 
}

//----------------------------------------------------------------------------
vtkIdType vtkCSGGrid::AddRegion(vtkIdType regId, const double *coeffs)
{
    vtkImplicitFunction *newRegion = 0;
    vtkImplicitFunction *oldReg = GetRegionFunc(regId);

    switch (GetImplicitFuncType(oldReg))
    {
        case FUNC_SPHERE:
        {
            vtkSphere *oldSphere = vtkSphere::SafeDownCast(oldReg);
            vtkSphere *newSphere = vtkSphere::New();
            newSphere->SetCenter(oldSphere->GetCenter());
            newSphere->SetRadius(oldSphere->GetRadius());

            newRegion = newSphere;
            break;
        }
        case FUNC_PLANE:
        {
            vtkPlane *oldPlane = vtkPlane::SafeDownCast(oldReg);
            vtkPlane *newPlane = vtkPlane::New();
            newPlane->SetOrigin(oldPlane->GetOrigin());
            newPlane->SetNormal(oldPlane->GetNormal());

            newRegion = newPlane;
            break;
        }
        case FUNC_CYLINDER: 
        {
            vtkCylinder *oldCylinder = vtkCylinder::SafeDownCast(oldReg);
            vtkCylinder *newCylinder = vtkCylinder::New();
            newCylinder->SetCenter(oldCylinder->GetCenter());
            newCylinder->SetRadius(oldCylinder->GetRadius());

            newRegion = newCylinder;
            break;
        }
        case FUNC_BOOLEAN:
        {
            vtkImplicitBoolean *oldBool = vtkImplicitBoolean::SafeDownCast(oldReg);
            vtkImplicitBoolean *newBool = vtkImplicitBoolean::New();

            vtkImplicitFunctionCollection *oldColl = oldBool->GetFunction();
            newBool->SetOperationType(newBool->GetOperationType());
            newBool->AddFunction(vtkImplicitFunction::SafeDownCast(oldColl->GetItemAsObject(0)));
            newBool->AddFunction(vtkImplicitFunction::SafeDownCast(oldColl->GetItemAsObject(1)));

            newRegion = newBool;
            break;
        }
        case FUNC_QUADRIC:
        {
            vtkQuadric *oldQuadric = vtkQuadric::SafeDownCast(oldReg);
            vtkQuadric *newQuadric = vtkQuadric::New();
            newQuadric->SetCoefficients(oldQuadric->GetCoefficients());

            newRegion = newQuadric;
            break;
        }

    } // switch

    if (newRegion)
    {
        vtkTransform *oldXform = vtkTransform::SafeDownCast(oldReg->GetTransform());
        vtkTransform *newXform = vtkTransform::New();
        if (oldXform)
            newXform->SetMatrix(oldXform->GetMatrix());

        newXform->Concatenate(coeffs);
        newRegion->SetTransform(newXform);
        newXform->Delete();
        
        Regions->AddItem(newRegion);
        newRegion->Delete();
        funcMap[newRegion] = Regions->GetNumberOfItems()-1;
        return funcMap[newRegion]; 
    }

    return -1;
}

//----------------------------------------------------------------------------
void vtkCSGGrid::GetRegion(vtkIdType id, vtkIdType *id1, vtkIdType *id2,
                 RegionOp *op, double **xform) const
{
//#warning GetRegion NOT IMPLEMENTED
}

//----------------------------------------------------------------------------
vtkIdType vtkCSGGrid::AddCell(vtkIdType regId)
{
    if (regId >= 0 && regId < Regions->GetNumberOfItems())
        return CellRegionIds->InsertNextValue(regId);
    return -1;
}

//----------------------------------------------------------------------------
vtkIdType vtkCSGGrid::GetCellRegionId(vtkIdType cellId) const
{
    if (cellId >= 0 && cellId < CellRegionIds->GetNumberOfTuples())
        return CellRegionIds->GetValue(cellId);
    return -1;
}

vtkPolyData  *vtkCSGGrid::DiscretizeSurfaces(
    int specificZone, double tol,
    double minX, double maxX,
    double minY, double maxY,
    double minZ, double maxZ)
{
    vtkAppendPolyData *appender = vtkAppendPolyData::New();

    //
    // Turn relative tolerance into an absolute tolerance 
    //
    tol = ComputeRelativeTol(tol, minX, maxX, minY, maxY, minZ, maxZ);
    int nX = (int) ((maxX - minX) / tol);
    int nY = (int) ((maxY - minY) / tol);
    int nZ = (int) ((maxZ - minZ) / tol);

    // fudge the bounds a bit
    minX -= minX * (minX < 0.0 ? -tol : tol);
    minY -= minY * (minY < 0.0 ? -tol : tol);
    minZ -= minZ * (minZ < 0.0 ? -tol : tol);
    minX += minX * (minX < 0.0 ? -tol : tol);
    minY += minY * (minY < 0.0 ? -tol : tol);
    minZ += minZ * (minZ < 0.0 ? -tol : tol);

    int startZone = 0;
    int endZone = CellRegionIds->GetNumberOfTuples();
    if (specificZone >= 0)
    {
        startZone = specificZone;
        endZone = startZone+1;
    }

    for (int i = startZone; i < endZone; i++)
    {
        vtkImplicitFunction *reg = GetRegionFunc(CellRegionIds->GetValue(i));

        //
        // Resample onto a regular grid and iso-contour
        //
        vtkSampleFunction *regSample = vtkSampleFunction::New();
        regSample->SetImplicitFunction(reg);
        regSample->SetModelBounds(minX, maxX, minY, maxY, minZ, maxZ);
        regSample->SetSampleDimensions(nX, nY, nZ);
        regSample->SetCapValue(0.0);
        regSample->ComputeNormalsOff();
        regSample->CappingOn();

        vtkContourFilter *regContour = vtkContourFilter::New();
        regContour->SetInput((vtkDataSet*)regSample->GetOutput());
        regContour->SetValue(0, 0.0);

        appender->AddInput(regContour->GetOutput());
        regContour->Delete();
        regSample->Delete();
    }

    appender->Update();
    return appender->GetOutput();
}

vtkUnstructuredGrid *vtkCSGGrid::DiscretizeSpace(
    int specificZone, double tol,
    double minX, double maxX,
    double minY, double maxY,
    double minZ, double maxZ)
{
    vtkUnstructuredGrid *retval = 0;
    vtkAppendFilter *appender = vtkAppendFilter::New();

    //
    // Turn relative tolerance into an absolute tolerance 
    //
    tol = ComputeRelativeTol(tol, minX, maxX, minY, maxY, minZ, maxZ);
    int nX = (int) ((maxX - minX) / tol);
    int nY = (int) ((maxY - minY) / tol);
    int nZ = (int) ((maxZ - minZ) / tol);

    // fudge the bounds a bit
    minX -= minX * (minX < 0.0 ? -tol : tol);
    minY -= minY * (minY < 0.0 ? -tol : tol);
    minZ -= minZ * (minZ < 0.0 ? -tol : tol);
    minX += minX * (minX < 0.0 ? -tol : tol);
    minY += minY * (minY < 0.0 ? -tol : tol);
    minZ += minZ * (minZ < 0.0 ? -tol : tol);

    int startZone = 0;
    int endZone = CellRegionIds->GetNumberOfTuples();
    if (specificZone >= 0)
    {
        startZone = specificZone;
        endZone = startZone+1;
    }

    for (int i = startZone; i < endZone; i++)
    {
        vtkImplicitFunction *reg = GetRegionFunc(CellRegionIds->GetValue(i));

        //
        // Resample onto a regular grid and iso-contour
        //
        vtkSampleFunction *regSample = vtkSampleFunction::New();
        regSample->SetImplicitFunction(reg);
        regSample->SetModelBounds(minX, maxX, minY, maxY, minZ, maxZ);
        regSample->SetSampleDimensions(nX, nY, nZ);
        regSample->SetCapValue(0.0);
        regSample->ComputeNormalsOff();
        regSample->CappingOn();

        vtkVisItClipper *regClipper = vtkVisItClipper::New();
        regClipper->SetInput((vtkDataSet*)regSample->GetOutput());
        regClipper->SetClipFunction(reg);
        regClipper->SetInsideOut(true);

        appender->AddInput(regClipper->GetOutput());
        regClipper->Delete();
        regSample->Delete();
    }

    appender->Update();
    return appender->GetOutput();
}

void
vtkCSGGrid::MakeMeshZone(const Box *theBox,
    vtkPoints *points, vtkUnstructuredGrid *ugrid,
    map<float, map<float, map<float, int> > >& nodemap)
{
    // Add points first
    int pointIds[8] = {0,0,0,0,0,0,0,0};
    for (int i = 0; i < 8; i++)
    {
        double x, y, z;
        if (i==0 || i==1 || i==4 || i==5)
            x = theBox->x0;
        else
            x = theBox->x1;
        if (i==1 || i==2 || i==5 || i==6)
            y = theBox->y0;
        else
            y = theBox->y1;
        if (i==0 || i==1 || i==2 || i==3)
            z = theBox->z0; 
        else
            z = theBox->z1; 

        int mapId = nodemap[x][y][z];
        if (mapId == 0)
        {
            points->InsertNextPoint(x,y,z);
            nodemap[x][y][z] = points->GetNumberOfPoints()-1;
            pointIds[i] = points->GetNumberOfPoints()-1;
        }
        else
        {
            pointIds[i] = nodemap[x][y][z];
        }
    }
    ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, pointIds);
}

vtkUnstructuredGrid *vtkCSGGrid::DiscretizeSpace(
    int specificZone, int rank, int nprocs, double tol,
    double minX, double maxX,
    double minY, double maxY,
    double minZ, double maxZ)
{
    int i;
    deque<Box*> boxDeque;

    // for building unstructured grid
    vtkPoints *points = vtkPoints::New();
    points->InsertNextPoint(0.0,0.0,0.0); // nodemap entry 0 used for "not set"
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(points);
    map<float, map<float, map<float, int> > > nodemap;

    int startZone = 0;
    int endZone = CellRegionIds->GetNumberOfTuples();
    if (specificZone >= 0)
    {
        startZone = specificZone;
        endZone = startZone+1;
    }

    vector<int> zoneIds;
    for (i = startZone; i < endZone; i++)
        zoneIds.push_back(i);            

    // fudge the bounds a bit
    minX -= minX * (minX < 0.0 ? -tol : tol);
    minY -= minY * (minY < 0.0 ? -tol : tol);
    minZ -= minZ * (minZ < 0.0 ? -tol : tol);
    minX += minX * (minX < 0.0 ? -tol : tol);
    minY += minY * (minY < 0.0 ? -tol : tol);
    minZ += minZ * (minZ < 0.0 ? -tol : tol);

    Box* boxZero = new Box(minX, maxX, minY, maxY, minZ, maxZ, zoneIds,
                           DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX,
                           DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX);
    boxDeque.push_back(boxZero);

    //
    // Turn relative tolerance into an absolute tolerance 
    //
    tol = ComputeRelativeTol(tol, minX, maxX, minY, maxY, minZ, maxZ);

    int K = 100;

    for (int pass = 0; pass < 2; pass++)
    {
        // split the deque on 2nd pass
        if (pass == 1)
        {
            deque<Box*> tmpDeque;

            int numBoxes  = boxDeque.size() / nprocs;
            int nprocs1   = boxDeque.size() % nprocs;

            int next = nprocs1 ? numBoxes+1 : numBoxes;
            int proc = 0;
            for (int q = 0; q < boxDeque.size(); q++)
            {
                if (q == next)
                {
                    proc++;
                    next += numBoxes;
                    if (proc < nprocs1)
                        next++;
                }

                if (proc == rank)
                    tmpDeque.push_back(boxDeque[q]);
            }

            boxDeque = tmpDeque;
        }

    while (boxDeque.size() > 0 &&
           ((pass == 0 && boxDeque.size() < K * nprocs) ||
            (pass == 1 && boxDeque.size() > 0)))
    {
        Box* curBox = boxDeque.front();
        boxDeque.pop_front();

        vector<int>& idref = curBox->zids;
        vector<int>  idcpy = curBox->zids;

        for (i = 0; i < idcpy.size(); i++)
        {
            vtkImplicitFunction *reg = GetRegionFunc(CellRegionIds->GetValue(idcpy[i]));

            Box::FuncState boxState = curBox->EvalFuncState(reg, tol);

            if (boxState == Box::LT_ZERO)
            {
                if ((pass == 0 && rank == 0) || (pass == 1))
                    MakeMeshZone(curBox, points, ugrid, nodemap);
                break;
            }
            else if (boxState == Box::GT_ZERO)
            {
                std::vector<int>::iterator ii;
                for (ii = idref.begin(); ii != idref.end(); ii++)
                {
                    if (*ii == i)
                    {
                        idref.erase(ii);
                        break;
                    }
                }
                if (idref.size() == 0)
                {
                    //MakeMeshZone(curBox, points, ugrid, nodemap);
                }
            }
            else if (curBox->Resolution() > tol)
            {

               //
               // Subdivide this box
               //
               int j;
               int numX = 0, numY = 0, numZ = 0;
               vector<Box*> newBoxes, newXBoxes, newYBoxes, newZBoxes;

#if 0
               Box::FuncState newState;

               //
               // Try subdividing in X first 
               //
               newXBoxes = curBox->SubdivideX();
               for (j = 0; j < newXBoxes.size(); j++)
               {
                   newState = newXBoxes[j]->EvalFuncState(reg, tol); 
                   if (newState == Box::LT_ZERO || newState == Box::GT_ZERO)
                       numX++;
               }

               //
               // Now, try subidiving in Y
               //
               newYBoxes = curBox->SubdivideY();
               for (j = 0; j < newYBoxes.size(); j++)
               {
                   newState = newYBoxes[j]->EvalFuncState(reg, tol); 
                   if (newState == Box::LT_ZERO || newState == Box::GT_ZERO)
                       numY++;
               }

               //
               // Now, try subidiving in Z
               //
               newZBoxes = curBox->SubdivideZ();
               for (j = 0; j < newZBoxes.size(); j++)
               {
                   newState = newZBoxes[j]->EvalFuncState(reg, tol); 
                   if (newState == Box::LT_ZERO || newState == Box::GT_ZERO)
                       numZ++;
               }

               if (numX > numY)
               {
                   if (numZ > numX)
                       newBoxes = newZBoxes;
                   else if (numX > numZ)
                       newBoxes = newXBoxes;
                   else
                   {
                       int j=1+(int) (10.0*rand()/(RAND_MAX+1.0));
                       if (j <= 5)
                           newBoxes = newXBoxes;
                       else
                           newBoxes = newZBoxes;
                   }
               }
               else if (numY > numX)
               {
                   if (numZ > numY)
                       newBoxes = newZBoxes;
                   else if (numY > numZ)
                       newBoxes = newYBoxes;
                   else
                   {
                       int j=1+(int) (10.0*rand()/(RAND_MAX+1.0));
                       if (j <= 5)
                           newBoxes = newYBoxes;
                       else
                           newBoxes = newZBoxes;
                   }
               }
               else
               {
                   if (numZ > numY)
                       newBoxes = newZBoxes;
                   else if (numY > numZ)
                   {
                       int j=1+(int) (10.0*rand()/(RAND_MAX+1.0));
                       if (j <= 5)
                           newBoxes = newYBoxes;
                       else
                           newBoxes = newXBoxes;
                   }
                   else
                   {
                       int j=1+(int) (12.0*rand()/(RAND_MAX+1.0));
                       if (j <= 4)
                           newBoxes = newXBoxes;
                       else if (j <= 8)
                           newBoxes = newYBoxes;
                       else
                           newBoxes = newZBoxes;
                   }
               }
               if (newBoxes != newXBoxes)
               {
                   for (j = 0; j < newXBoxes.size(); j++)
                       delete newXBoxes[j];
               }
               if (newBoxes != newYBoxes)
               {
                   for (j = 0; j < newYBoxes.size(); j++)
                       delete newYBoxes[j];
               }
               if (newBoxes != newZBoxes)
               {
                   for (j = 0; j < newZBoxes.size(); j++)
                       delete newZBoxes[j];
               }
#else
               newBoxes = curBox->Subdivide();
#endif
               for (j = 0; j < newBoxes.size(); j++)
                   boxDeque.push_back(newBoxes[j]);

#if 0
               if (curBox->Resolution() <= 8 * tol)
                   MakeMeshZone(curBox, points, ugrid2, nodemap);

               if (curBox->Resolution() <= 64 * tol)
                   MakeMeshZone(curBox, points, ugrid1, nodemap);
#endif
                  
           }
           else
           {
               if ((pass == 0 && rank == 0) || (pass == 1))
                   MakeMeshZone(curBox, points, ugrid, nodemap);
           }
        }

        delete curBox;
    }


    } // for pass

    vtkVisItClipper *regClipper = vtkVisItClipper::New();
    regClipper->SetInput(ugrid);
    regClipper->SetClipFunction(GetRegionFunc(CellRegionIds->GetValue(specificZone)));
    regClipper->SetInsideOut(true);
    regClipper->Update();

    vtkUnstructuredGrid *clippedGrid = regClipper->GetOutput();
    clippedGrid->Update();
    clippedGrid->Register(NULL);
    regClipper->Delete();
    ugrid->Delete();

    return clippedGrid;
}
