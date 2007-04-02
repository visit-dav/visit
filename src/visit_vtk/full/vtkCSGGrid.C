#include "vtkCSGGrid.h"
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

#include <float.h>

vtkCxxRevisionMacro(vtkCSGGrid, "$Revision: 1.53 $");
vtkStandardNewMacro(vtkCSGGrid);

// internal type used only for implementation
typedef enum {
    BOOLEAN,
    CONE,
    CYLINDER,
    PLANE,
    MULTIPLANE,
    QUADRIC,
    SPHERE,
    UNKNOWN_IMPLICIT
} ImplicitFuncType;

static ImplicitFuncType GetImplicitFuncType(const vtkObject *obj)
{
    const char *className = obj->GetClassName();
    if      (strcmp(className, "vtkImplicitBoolean") == 0)
        return BOOLEAN;
    else if (strcmp(className, "vtkCone") == 0)
        return CONE;
    else if (strcmp(className, "vtkCylinder") == 0)
        return CYLINDER;
    else if (strcmp(className, "vtkPlane") == 0)
        return PLANE;
    else if (strcmp(className, "vtkPlanes") == 0)
        return MULTIPLANE;
    else if (strcmp(className, "vtkQuadric") == 0)
        return QUADRIC;
    else if (strcmp(className, "vtkSphere") == 0)
        return SPHERE;
    else
        return UNKNOWN_IMPLICIT; 
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
  int i;
  this->Initialize();

  this->SetBoundaries(csgGrid->GetBoundaries());
  this->SetRegions(csgGrid->GetRegions());
  this->SetCellRegionIds(csgGrid->GetCellRegionIds());
}

//----------------------------------------------------------------------------
vtkCell *vtkCSGGrid::GetCell(vtkIdType cellId)
{
#warning GetCell NOT IMPLEMENTED
  return NULL;
}

//----------------------------------------------------------------------------
void vtkCSGGrid::GetCell(vtkIdType cellId, vtkGenericCell *cell)
{
#warning GetCell NOT IMPLEMENTED
  cell->SetCellTypeToEmptyCell();
}

int vtkCSGGrid::GetCellType(vtkIdType cellId)
{
#warning GetCellType NOT IMPLEMENTED
  return VTK_EMPTY_CELL;
}

int vtkCSGGrid::GetMaxCellSize()
{
#warning GetMaxCellSize NOT IMPLEMENTED
  return -1;
}

//----------------------------------------------------------------------------
float *vtkCSGGrid::GetPoint(vtkIdType ptId)
{
  vtkErrorMacro("For a vtkCSGGrid, GetPoint() means GetBoundary()");
  vtkErrorMacro("Use GetBoundary() to avoid this message");
  int dummy, n;
  double *p = 0;
  this->GetBoundary(ptId, &dummy, &n, &p);
  const int k = sizeof(tmpFloats) / sizeof(float);
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
void vtkCSGGrid::GetCellBounds(vtkIdType cellId, float bounds[6])
{
#warning GetCellBounds NOT IMPLEMENTED
  return;
}

void vtkCSGGrid::GetPoint(vtkIdType ptId, float x[3])
{
  vtkErrorMacro("Requesting a point[3] from a vtkCSGGrid");
  vtkErrorMacro("Use GetBoundary() to avoid this message");
  x[0] = x[1] = x[2] = 0.0;
  return;
}

//----------------------------------------------------------------------------
vtkIdType vtkCSGGrid::FindPoint(float x[3])
{
  vtkErrorMacro("Finding a point on a vtkCSGGrid not yet implemented.");
  return -1;
}

vtkIdType vtkCSGGrid::FindCell(float x[3], vtkCell *vtkNotUsed(cell), 
                                       vtkGenericCell *vtkNotUsed(gencell),
                                       vtkIdType vtkNotUsed(cellId), 
                                       float vtkNotUsed(tol2), 
                                       int& subId, float pcoords[3], 
                                       float *weights)
{
  return
    this->FindCell( x, (vtkCell *)NULL, 0, 0.0, subId, pcoords, weights );
}

//----------------------------------------------------------------------------
vtkIdType vtkCSGGrid::FindCell(float x[3], vtkCell *vtkNotUsed(cell), 
                                       vtkIdType vtkNotUsed(cellId),
                                       float vtkNotUsed(tol2), 
                                       int& subId, float pcoords[3],
                                       float *weights)
{
#warning FindCell NOT IMPLEMENTED
  return -1;
}

//----------------------------------------------------------------------------
vtkCell *vtkCSGGrid::FindAndGetCell(float x[3],
                                            vtkCell *vtkNotUsed(cell), 
                                            vtkIdType vtkNotUsed(cellId),
                                            float vtkNotUsed(tol2),
                                            int& subId, 
                                            float pcoords[3], float *weights)
{
#warning FindAndGetCell NOT IMPLEMENTED
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
#warning CellPoints NOT IMPLEMENTED
}

//----------------------------------------------------------------------------
void vtkCSGGrid::GetPointCells(vtkIdType ptId, vtkIdList *cellIds)
{
#warning GetPointCells NOT IMPLEMENTED
}


//----------------------------------------------------------------------------
void vtkCSGGrid::ComputeBounds()
{
  this->Bounds[0] = this->Bounds[1] = this->Bounds[2] = this->Bounds[3] =
  this->Bounds[4] = this->Bounds[5] = 0.0;
#warning ComputeBounds NOT IMPLEMENTED
}

static unsigned long GetActualMemorySizeOfImplicitFunc(vtkImplicitFunction *func)
{
    unsigned long size = 0;

    switch (GetImplicitFuncType(func))
    {
        case BOOLEAN:
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
        case CONE:     size += sizeof(vtkCone);     break;
        case CYLINDER: size += sizeof(vtkCylinder); break;
        case PLANE:    size += sizeof(vtkPlane);    break;
        case SPHERE:   size += sizeof(vtkSphere);   break;
        case QUADRIC:  size += sizeof(vtkQuadric);  break;
        case MULTIPLANE:
        {
            vtkPlanes *planes = vtkPlanes::SafeDownCast(func);
            size += planes->GetPoints()->GetActualMemorySize();
            size += planes->GetNormals()->GetActualMemorySize();
            size += planes->GetNumberOfPlanes() * sizeof(vtkPlane);
            break;
        }
        case UNKNOWN_IMPLICIT:
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

#warning GetCellNeighbors NOT IMPLEMENTED
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
            case CYLINDER:
            {
                os << ", is a cylinder" << endl;
                vtkCylinder *cylinder = vtkCylinder::SafeDownCast(func);
                  cylinder->PrintSelf(os, indent2.GetNextIndent());
                break;
            }
            case PLANE:
            {
                os << ", is a plane" << endl;
                vtkPlane *plane = vtkPlane::SafeDownCast(func);
                  plane->PrintSelf(os, indent2.GetNextIndent());
                break;
            }
            case SPHERE:
            {
                os << ", is a sphere" << endl;
                vtkSphere *sphere = vtkSphere::SafeDownCast(func);
                  sphere->PrintSelf(os, indent2.GetNextIndent());
                break;
            }
            case QUADRIC:
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
            case BOOLEAN:
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
                        case CYLINDER:
                        {
                            os << ", is the OUTER of a cylinder" << endl;
                            vtkCylinder *cylinder = vtkCylinder::SafeDownCast(rightFunc);
                              cylinder->PrintSelf(os, indent2.GetNextIndent());
                            break;
                        }
                        case SPHERE:
                        {
                            os << ", is the OUTER of a sphere" << endl;
                            vtkSphere *sphere = vtkSphere::SafeDownCast(rightFunc);
                              sphere->PrintSelf(os, indent2.GetNextIndent());
                            break;
                        }
                        case QUADRIC:
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

                    cerr << "of items " << leftId << " and " << rightId << endl;
                }
                break;
            }
            case CYLINDER:
            {
                os << ", is the INNER of a cylinder" << endl;
                vtkCylinder *cylinder = vtkCylinder::SafeDownCast(func);
                  cylinder->PrintSelf(os, indent2.GetNextIndent());
                break;
            }
            case PLANE:
            {
                os << ", is the INNER of a plane" << endl;
                vtkPlane *plane = vtkPlane::SafeDownCast(func);
                  plane->PrintSelf(os, indent2.GetNextIndent());
                break;
            }
            case SPHERE:
            {
                os << ", is the INNER of a sphere" << endl;
                vtkSphere *sphere = vtkSphere::SafeDownCast(func);
                  sphere->PrintSelf(os, indent2.GetNextIndent());
                break;
            }
            case QUADRIC:
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

            sphere->SetCenter((float) coeffs[0], (float) coeffs[1], (float) coeffs[2]);
            sphere->SetRadius((float) coeffs[3]);

            newBoundary = sphere;
            break;
        }

        case CYLINDER_PNLR:
        {
            vtkCylinder *cylinder = vtkCylinder::New();

            // if the desired cylinder is y-axis aligned, we don't need xform
            if (coeffs[3] == 0.0 && coeffs[4] == 1.0 && coeffs[5] == 0.0)
            {
                cylinder->SetCenter((float) coeffs[0], (float) coeffs[1], (float) coeffs[2]);
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
#warning were not setting length

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

            plane->SetOrigin((float) coeffs[0], 0.0, 0.0);
            plane->SetNormal(1.0, 0.0, 0.0);

            newBoundary = plane;
            break;
        }

        case PLANE_Y:
        {
            vtkPlane *plane = vtkPlane::New();

            plane->SetOrigin(0.0, (float) coeffs[0], 0.0);
            plane->SetNormal(0.0, 1.0, 0.0);

            newBoundary = plane;
            break;
        }

        case PLANE_Z:
        {
            vtkPlane *plane = vtkPlane::New();

            plane->SetOrigin(0.0, 0.0, (float) coeffs[0]);
            plane->SetNormal(0.0, 0.0, 1.0);

            newBoundary = plane;
            break;
        }

        case QUADRIC_G:
        {
            vtkQuadric *quadric = vtkQuadric::New();
            quadric->SetCoefficients((float) coeffs[0], // x^2 term
                                     (float) coeffs[1], // y^2 term
                                     (float) coeffs[2], // z^2 term
                                     (float) coeffs[3], // xy term
                                     (float) coeffs[4], // yz term
                                     (float) coeffs[5], // xz term
                                     (float) coeffs[6], // x^1 term
                                     (float) coeffs[7], // y^1 term
                                     (float) coeffs[8], // z^1 term
                                     (float) coeffs[9]); // constant term

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
#warning GetBoundary NOT IMPLEMENTED
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
        case SPHERE:
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
        case PLANE:
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
        case CYLINDER:
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

        case QUADRIC:
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
        case SPHERE:
        {
            vtkSphere *oldSphere = vtkSphere::SafeDownCast(oldReg);
            vtkSphere *newSphere = vtkSphere::New();
            newSphere->SetCenter(oldSphere->GetCenter());
            newSphere->SetRadius(oldSphere->GetRadius());

            newRegion = newSphere;
            break;
        }
        case PLANE:
        {
            vtkPlane *oldPlane = vtkPlane::SafeDownCast(oldReg);
            vtkPlane *newPlane = vtkPlane::New();
            newPlane->SetOrigin(oldPlane->GetOrigin());
            newPlane->SetNormal(oldPlane->GetNormal());

            newRegion = newPlane;
            break;
        }
        case CYLINDER: 
        {
            vtkCylinder *oldCylinder = vtkCylinder::SafeDownCast(oldReg);
            vtkCylinder *newCylinder = vtkCylinder::New();
            newCylinder->SetCenter(oldCylinder->GetCenter());
            newCylinder->SetRadius(oldCylinder->GetRadius());

            newRegion = newCylinder;
            break;
        }
        case BOOLEAN:
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
        case QUADRIC:
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
#warning GetRegion NOT IMPLEMENTED
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

vtkPolyData  *vtkCSGGrid::DiscretizeSurfaces(int specificZone,
    double minX, double maxX, int nX,
    double minY, double maxY, int nY,
    double minZ, double maxZ, int nZ)
{
    vtkAppendPolyData *appender = vtkAppendPolyData::New();

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
        // We restrict each region (zone) in the mesh to the
        // spatial bounding box so that upon contouring using the
        // algorithm below, the surfaces will be closed off
        //
        vtkPlanes           *box = vtkPlanes::New();
        box->SetBounds(minX, maxX, minY, maxY, minZ, maxZ);

        vtkImplicitBoolean *boxedReg = vtkImplicitBoolean::New();
        boxedReg->SetOperationTypeToIntersection();
        boxedReg->AddFunction(box);
        boxedReg->AddFunction(reg);

        //
        // Resample onto a regular grid and iso-contour
        //
        vtkSampleFunction *regSample = vtkSampleFunction::New();
        regSample->SetImplicitFunction(boxedReg);
        regSample->SetModelBounds(minX, maxX, minY, maxY, minZ, maxZ);
        regSample->SetSampleDimensions(nX, nY, nZ);
        regSample->ComputeNormalsOff();

        vtkContourFilter *regContour = vtkContourFilter::New();
        regContour->SetInput((vtkDataSet*)regSample->GetOutput());
        regContour->SetValue(0, 0.0);

        boxedReg->Delete();
        box->Delete();

        appender->AddInput(regContour->GetOutput());
    }
#warning FIX LEAKS HERE
    appender->Update();
    //this->PrintSelf(cerr, 0);

    return appender->GetOutput();
}
