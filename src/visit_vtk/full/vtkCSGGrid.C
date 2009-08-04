/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <visit-config.h>
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

#include <DebugStream.h>

#ifdef HAVE_BILIB
#include <boost/numeric/interval.hpp>
using boost::numeric::interval;
#endif

#include <deque>
#include <map>
#include <vector>

using std::deque;
using std::map;
using std::vector;

//
// Since we're passing Silo's CSG rep directly to this
// class, we need to know these definitions. Eventually,
// we should go through some kind of conversion from Silo's
// CSG rep to one specific to vtkCSGGrid class.
//
// The reasone these are defined in decimal format instead
// of hexidecimal has to do with fact that these ints are
// also used by Silo's Fortran interface.
//
// These definitions were copied directly from silo.h
//
#define DBCSG_QUADRIC_G         16777216   // 0x01000000
#define DBCSG_SPHERE_PR         33619968   // 0x02010000
#define DBCSG_ELLIPSOID_PRRR    33685504   // 0x02020000
#define DBCSG_PLANE_G           50331648   // 0x03000000
#define DBCSG_PLANE_X           50397184   // 0x03010000
#define DBCSG_PLANE_Y           50462720   // 0x03020000
#define DBCSG_PLANE_Z           50528256   // 0x03030000
#define DBCSG_PLANE_PN          50593792   // 0x03040000
#define DBCSG_PLANE_PPP         50659328   // 0x03050000
#define DBCSG_CYLINDER_PNLR     67108864   // 0x04000000
#define DBCSG_CYLINDER_PPR      67174400   // 0x04010000
#define DBCSG_BOX_XYZXYZ        83886080   // 0x05000000
#define DBCSG_CONE_PNLA         100663296  // 0x06000000
#define DBCSG_CONE_PPA          100728832  // 0x06010000
#define DBCSG_POLYHEDRON_KF     117440512  // 0x07000000
#define DBCSG_HEX_6F            117506048  // 0x07010000
#define DBCSG_TET_4F            117571584  // 0x07020000
#define DBCSG_PYRAMID_5F        117637120  // 0x07030000
#define DBCSG_PRISM_5F          117702656  // 0x07040000

// Definitions for 2D CSG boundary types
#define DBCSG_QUADRATIC_G       134217728  // 0x08000000
#define DBCSG_CIRCLE_PR         150994944  // 0x09000000
#define DBCSG_ELLIPSE_PRR       151060480  // 0x09010000
#define DBCSG_LINE_G            167772160  // 0x0A000000
#define DBCSG_LINE_X            167837696  // 0x0A010000
#define DBCSG_LINE_Y            167903232  // 0x0A020000
#define DBCSG_LINE_PN           167968768  // 0x0A030000
#define DBCSG_LINE_PP           168034304  // 0x0A040000
#define DBCSG_BOX_XYXY          184549376  // 0x0B000000
#define DBCSG_ANGLE_PNLA        201326592  // 0x0C000000
#define DBCSG_ANGLE_PPA         201392128  // 0x0C010000
#define DBCSG_POLYGON_KP        218103808  // 0x0D000000
#define DBCSG_TRI_3P            218169344  // 0x0D010000
#define DBCSG_QUAD_4P           218234880  // 0x0D020000

// Definitions for CSG Region operators
#define DBCSG_INNER             2130706432 // 0x7F000000
#define DBCSG_OUTER             2130771968 // 0x7F010000
#define DBCSG_ON                2130837504 // 0x7F020000
#define DBCSG_UNION             2130903040 // 0x7F030000
#define DBCSG_INTERSECT         2130968576 // 0x7F040000
#define DBCSG_DIFF              2131034112 // 0x7F050000
#define DBCSG_COMPLIMENT        2131099648 // 0x7F060000
#define DBCSG_XFORM             2131165184 // 0x7F070000
#define DBCSG_SWEEP             2131230720 // 0x7F080000


#ifdef _WIN32
#define M_PI 3.14159265358979323846
#endif

#define NUM_QCOEFFS 10 // # coefficients in a quadric rep

vtkCxxRevisionMacro(vtkCSGGrid, "$Revision: 1.53 $");
vtkStandardNewMacro(vtkCSGGrid);

// internal type used only for implementation
typedef enum {
    FUNC_BOOLEAN,
    FUNC_CONE,
    FUNC_CYLINDER,
    FUNC_PLANE,
    FUNC_MULTIPLANE,
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
        return FUNC_MULTIPLANE;
    else if (strcmp(className, "vtkQuadric") == 0)
        return FUNC_QUADRIC;
    else if (strcmp(className, "vtkSphere") == 0)
        return FUNC_SPHERE;
    else
        return FUNC_UNKNOWN_IMPLICIT; 
}

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

bool
vtkCSGGrid::Box::IsFlatEnough2(const double *const gridBoundaries,
    int boundaryId, double tol)
{
#ifdef HAVE_BILIB
    const double *const a = &gridBoundaries[boundaryId * NUM_QCOEFFS]; 

    // compute spatial box
    interval<double> X(x0,x1);
    interval<double> Y(y0,y1);
    interval<double> Z(z0,z1);

    // quick check for planar functions. They're always flat
    if (a[0] == 0.0 && a[1] == 0.0 && a[2] == 0.0 &&
        a[3] == 0.0 && a[4] == 0.0 && a[5] == 0.0)
        return true;

    // compute gradient box components (e.g. intervals of grad components)
    interval<double> gradX = 2*a[0]*X + a[3]*Y + a[5]*Z + a[6];
    interval<double> gradY = 2*a[1]*Y + a[3]*X + a[4]*Z + a[7];
    interval<double> gradZ = 2*a[2]*Z + a[4]*Y + a[5]*X + a[8];

    //
    // see if the grad-box contains the origin. If so, it
    // means that gradient direction varies over entire 360 deg
    //
    if (lower(gradX)<0.0 && upper(gradX)>0.0 &&
        lower(gradY)<0.0 && upper(gradY)>0.0 &&
        lower(gradZ)<0.0 && upper(gradZ)>0.0)
        return false;   

    // compute vector to center of grad box
    double vgx = (lower(gradX) + upper(gradX)) / 2.0;
    double vgy = (lower(gradY) + upper(gradY)) / 2.0;
    double vgz = (lower(gradZ) + upper(gradZ)) / 2.0;

    // find diagonal of grad box most orthogonal to vector vg 
    double mindotp = DBL_MAX;
    int dmxmin, dmymin;
    for (int d = 0; d < 4; d++)
    {
        int dmx = (d & 0x01) ? -1 : 1;
        int dmy = (d & 0x02) ? -1 : 1;
        double dvx = dmx * (upper(gradX) - lower(gradX));
        double dvy = dmy * (upper(gradY) - lower(gradY));
        double dvz =       (upper(gradZ) - lower(gradZ));

        double dotp = vgx * dvx + vgy * dvy + vgz * dvz;
        if (dotp < 0.0) dotp = -dotp;
        if (dotp < mindotp)
        {
            mindotp = dotp;
            dmxmin = dmx;
            dmymin = dmy;
        }
    }

    // build vectors spanning maximum range of angles of grad box
    double ax = (dmxmin ==  1) ? upper(gradX) : lower(gradX);
    double ay = (dmymin ==  1) ? upper(gradY) : lower(gradY);
    double az =                  upper(gradZ);
    double bx = (dmxmin == -1) ? upper(gradX) : lower(gradX);
    double by = (dmymin == -1) ? upper(gradY) : lower(gradY);
    double bz =                               lower(gradZ);
    double maga = sqrt(ax*ax + ay*ay + az*az);
    double magb = sqrt(bx*bx + by*by + bz*bz);
    double cos_theta = (ax*bx + ay*by + az*bz) / (maga * magb);
    if (cos_theta < 0.0) cos_theta = -cos_theta;
    double theta = acos(cos_theta);

    // compute length of spatial box diagonal
    double db =  sqrt((upper(X)-lower(X)) * (upper(X)-lower(X)) +
                      (upper(Y)-lower(Y)) * (upper(Y)-lower(Y)) +
                      (upper(Z)-lower(Z)) * (upper(Z)-lower(Z)));

    //if (((1-cos(theta/2)) / (2*sin(theta/2))) < tol)
    //    return true;
    if (2*sin(theta/2) < tol)
        return true;
    return false;

#if 0
    // compute estimate of radius of curvature of this surface
    double r = (db / 2.0) / sin(theta / 2.0);

    if (r > tol)
        return true;
    return false;
#endif

#if 0
    // square of length of box diagonal 
    double db =  (upper(gradX)-lower(gradX)) * (upper(gradX)-lower(gradX)) +
                 (upper(gradY)-lower(gradY)) * (upper(gradY)-lower(gradY)) +
                 (upper(gradZ)-lower(gradZ)) * (upper(gradZ)-lower(gradZ));

    // square of distance to center of box 
    double dc = ((upper(gradX)+lower(gradX)) * (upper(gradX)+lower(gradX)) +
                 (upper(gradY)+lower(gradY)) * (upper(gradY)+lower(gradY)) +
                 (upper(gradZ)+lower(gradZ)) * (upper(gradZ)+lower(gradZ))) / 4.0;

    if (dc < 0.0) dc = -dc;
    if (dc > 0.0)
    {
        if (db / dc < tol)
            return true;
    }
    return false;
#endif

#else
    return false;
#endif
}

bool
vtkCSGGrid::Box::CanBeCut2(const double *const gridBoundaries,
    map<int,int> boundaryToStateMap, double tol)
{
    //
    // Check to see if any of the EQ_ZERO boundaries are not flat enough
    //
    map<int,int>::const_iterator it;
    for (it = boundaryToStateMap.begin(); it != boundaryToStateMap.end(); it++)
    {
        if (it->second == EQ_ZERO && !IsFlatEnough2(gridBoundaries, it->first, tol))
            return false;
    }

    //
    // All the EQ_ZERO boundaries are flat enough 
    //
    return true;
}

vtkCSGGrid::Box::FuncState
vtkCSGGrid::Box::EvalBoxStateOfBoundary(const double *const a, double tol) const
{
#ifdef HAVE_BILIB
    interval<double> X(x0,x1);
    interval<double> Y(y0,y1);
    interval<double> Z(z0,z1);

    interval<double> v = a[0]*square(X) + a[1]*square(Y) + a[2]*square(Z) + 
                 a[3]*X*Y + a[4]*Y*Z + a[5]*X*Z + 
                 a[6]*X + a[7]*Y + a[8]*Z + a[9];

    if ((lower(v) < 0) && (upper(v) < 0))
        return LT_ZERO;
    else if ((lower(v) > 0) && (upper(v) > 0))
        return GT_ZERO;
    else
        return EQ_ZERO;
#else
    return EQ_ZERO;
#endif
}

// curBox and tol are unused args here
int
vtkCSGGrid::EvalBoxStateOfRegion(const Box *const curBox, int zoneId,
    map<int,int>& boundaryToStateMap, double tol)
{
    int bndId, left, right;
    switch (regTypeFlags[zoneId])
    {
        case DBCSG_INNER:
            bndId = leftIds[zoneId];
            return boundaryToStateMap[bndId];
        case DBCSG_OUTER:
            bndId = leftIds[zoneId];
            return -boundaryToStateMap[bndId];
        case DBCSG_ON:
            bndId = leftIds[zoneId];
            if (boundaryToStateMap[bndId] == 0)
                return -1; // INNER
            else
                return +1; // OUTER
        case DBCSG_UNION:
        {
            const int unionMatrix[][3]={ {-1, -1, -1},
                                         {-1,  0,  0},
                                         {-1,  0, +1} };
            left  = EvalBoxStateOfRegion(curBox, leftIds[zoneId], boundaryToStateMap, tol); 
            right = EvalBoxStateOfRegion(curBox, rightIds[zoneId], boundaryToStateMap, tol); 
            return unionMatrix[left+1][right+1];
        }
        case DBCSG_INTERSECT:
        {
            const int intersectMatrix[][3]={ {-1,  0, +1},
                                             { 0,  0, +1},
                                             {+1, +1, +1} };
            left  = EvalBoxStateOfRegion(curBox, leftIds[zoneId], boundaryToStateMap, tol); 
            right = EvalBoxStateOfRegion(curBox, rightIds[zoneId], boundaryToStateMap, tol); 
            return intersectMatrix[left+1][right+1];
        }
        case DBCSG_DIFF:
        {
            const int diffMatrix[][3]={ {+1,  0, -1},
                                        {+1,  0,  0},
                                        {+1, +1, +1} };
            left  = EvalBoxStateOfRegion(curBox, leftIds[zoneId], boundaryToStateMap, tol); 
            right = EvalBoxStateOfRegion(curBox, rightIds[zoneId], boundaryToStateMap, tol); 
            return diffMatrix[left+1][right+1];
        }
        case DBCSG_XFORM:
            break;
        default:
            break;
    }
    return 0;
}

//----------------------------------------------------------------------------
vtkCSGGrid::vtkCSGGrid()
{
  this->Boundaries = vtkImplicitFunctionCollection::New();
  this->Regions    = vtkImplicitFunctionCollection::New();
  this->CellRegionIds = vtkIdTypeArray::New();

  // setup the Universe set
  this->Universe = vtkPlanes::New();
  this->Universe->SetBounds(-FLT_MAX, FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX, FLT_MAX);
  funcMap[Universe] = -1;

  gridBoundaries = 0;
  leftIds = 0;
  rightIds = 0;
  regTypeFlags = 0;
  gridZones = 0;
}

//----------------------------------------------------------------------------
vtkCSGGrid::~vtkCSGGrid()
{
  this->Initialize();
  this->Universe->Delete();
  if (gridBoundaries) delete [] gridBoundaries;
  gridBoundaries = 0;
  if (leftIds) delete [] leftIds;
  leftIds = 0;
  if (rightIds) delete [] rightIds;
  rightIds = 0;
  if (regTypeFlags) delete [] regTypeFlags;
  regTypeFlags = 0;
  if (gridZones) delete [] gridZones;
  gridZones = 0;
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
  int i;
  vtkCSGGrid *csgGrid=(vtkCSGGrid *)ds;
  this->Initialize();

  this->SetBoundaries(csgGrid->GetBoundaries());
  this->SetRegions(csgGrid->GetRegions());
  this->SetCellRegionIds(csgGrid->GetCellRegionIds());

  this->numBoundaries = csgGrid->numBoundaries;
  this->gridBoundaries = new double[NUM_QCOEFFS*(this->numBoundaries+6)];
  for (i = 0; i < NUM_QCOEFFS*(this->numBoundaries+6); i++)
    this->gridBoundaries[i] = csgGrid->gridBoundaries[i];

  this->numRegions = csgGrid->numRegions;
  this->leftIds = new int[this->numRegions];
  this->rightIds = new int[this->numRegions];
  this->regTypeFlags = new int[this->numRegions];
  for (i = 0; i < this->numRegions; i++)
    {
    this->leftIds[i] = csgGrid->leftIds[i];
    this->rightIds[i] = csgGrid->rightIds[i];
    this->regTypeFlags[i] = csgGrid->regTypeFlags[i];
    }
  
  this->numZones = csgGrid->numZones;
  this->gridZones = new int[this->numZones];
  for (i = 0; i < this->numZones; i++)
    this->gridZones[i] = csgGrid->gridZones[i];

  for (i = 0; i < 6; i++)
    this->Bounds[i] = csgGrid->Bounds[i];
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
        case FUNC_MULTIPLANE:
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
    // There is no shallow copy
    DeepCopy(dataObject);
}

//----------------------------------------------------------------------------
void vtkCSGGrid::DeepCopy(vtkDataObject *srcObject)
{
  vtkCSGGrid *grid = vtkCSGGrid::SafeDownCast(srcObject);

  if ( grid != NULL )
    {
      int i;
      this->numBoundaries = grid->numBoundaries;
      this->gridBoundaries = new double[NUM_QCOEFFS*(this->numBoundaries+6)];
      for (i = 0; i < NUM_QCOEFFS*(this->numBoundaries+6); i++)
        this->gridBoundaries[i] = grid->gridBoundaries[i];

      this->numRegions = grid->numRegions;
      this->leftIds = new int[this->numRegions];
      this->rightIds = new int[this->numRegions];
      this->regTypeFlags = new int[this->numRegions];
      for (i = 0; i < this->numRegions; i++)
        {
          this->leftIds[i] = grid->leftIds[i];
          this->rightIds[i] = grid->rightIds[i];
          this->regTypeFlags[i] = grid->regTypeFlags[i];
        }

      this->numZones = grid->numZones;
      this->gridZones = new int[this->numZones];
      for (i = 0; i < this->numZones; i++)
        this->gridZones[i] = grid->gridZones[i];

      for (i = 0; i < 6; i++)
        this->Bounds[i] = grid->Bounds[i];
    }

  // Do superclass
  this->vtkDataSet::DeepCopy(srcObject);
}

//----------------------------------------------------------------------------
// Modifications:
//
//   Hank Childs, Fri Jun  9 12:54:36 PDT 2006
//   Add "default" to switch statement
//
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
            default:
            {
                os << ", is an unexpected implicit function type" << endl;
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
                        default:
                        {
                            os << "is an unexpected implicit function type"
                               << endl;
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
            default:
            {
                os << "is an unexpected implicit function type"
                   << endl;
                break;
            }
        }
    }
}

//
// Quadric equation coefficient indices...
//
// x^2   y^2   z^2    xy    yz    xz    x    y    z    1
//  0     1     2     3     4     5     6    7    8    9
//
static void SpherePRToQuadric(const double *const sphere, double *quadric)
{
    quadric[0] = 1.0; // x^2 term
    quadric[1] = 1.0; // y^2 term
    quadric[2] = 1.0; // z^2 term
    quadric[6] = -2.0 * sphere[0]; // x term 
    quadric[7] = -2.0 * sphere[1]; // x term 
    quadric[8] = -2.0 * sphere[2]; // x term 
    quadric[9] = sphere[0]*sphere[0] + // constant term
                 sphere[1]*sphere[1] +
                 sphere[2]*sphere[2] -
                 sphere[3]*sphere[3];
}
static void PlaneXToQuadric(const double *const plane, double *quadric)
{
    quadric[6] = 1.0; 
    quadric[9] = -plane[0];
}
static void PlaneYToQuadric(const double *const plane, double *quadric)
{
    quadric[7] = 1.0;
    quadric[9] = -plane[0];
}
static void PlaneZToQuadric(const double *const plane, double *quadric)
{
    quadric[8] = 1.0;
    quadric[9] = -plane[0];
}
static void PlaneGToQuadric(const double *const plane, double *quadric)
{
    quadric[6] = plane[0]; 
    quadric[7] = plane[1]; 
    quadric[8] = plane[2]; 
    quadric[9] = plane[3]; 
}
static void PlanePNToQuadric(const double *const plane, double *quadric)
{
    quadric[6] = plane[3];
    quadric[7] = plane[4];
    quadric[8] = plane[5];
    quadric[9] = -plane[0]*plane[3] -
                  plane[1]*plane[4] -
                  plane[2]*plane[5];
}
static void PlanePPPToQuadric(const double *const plane, double *quadric)
{
    // The three points are 'a', 'b' and 'c', the middle point being the
    // vertex used to compute vectors and serve as point on the plane
    const double *a = &plane[0];
    const double *b = &plane[3];
    const double *c = &plane[6];
    double ab[3];
    double cb[3];
    for (int i = 0; i < 3; i++)
    {
        ab[i] = a[i] - b[i];
        cb[i] = c[i] - b[i];
    }

    // compute normalized cross-product of ab and cb vectors
    double xprod[3];
    xprod[0] = ab[1]*cb[2] - ab[2]*cb[1];
    xprod[1] = ab[0]*cb[2] - ab[2]*cb[0];
    xprod[2] = ab[0]*cb[1] - ab[1]*cb[0];
    double d = xprod[0]*xprod[0] + xprod[1]*xprod[1] + xprod[2]*xprod[2];
    if (d > 0.0)
    {
        xprod[0] /= d;
        xprod[1] /= d;
        xprod[2] /= d;
    }
    
    // use the PlanePN routine to finish it off
    double coeffs[6];
    coeffs[0] = b[0];
    coeffs[1] = b[1];
    coeffs[2] = b[2];
    coeffs[3] = xprod[0];
    coeffs[4] = xprod[1];
    coeffs[5] = xprod[2];
    PlanePNToQuadric(coeffs, quadric);
}
static void BoxXYZXYZToQuadric(const double *const box, double *quadric)
{
    PlaneXToQuadric(&box[0], &quadric[0*NUM_QCOEFFS]);
    PlaneYToQuadric(&box[1], &quadric[1*NUM_QCOEFFS]);
    PlaneZToQuadric(&box[2], &quadric[2*NUM_QCOEFFS]);
    PlaneXToQuadric(&box[3], &quadric[3*NUM_QCOEFFS]);
    PlaneYToQuadric(&box[4], &quadric[4*NUM_QCOEFFS]);
    PlaneZToQuadric(&box[5], &quadric[5*NUM_QCOEFFS]);
}
static void CylinderPNLRToQuadric(const double *const cyl, double *quadric)
{
    // point
    double px = cyl[0];
    double py = cyl[1];
    double pz = cyl[2];

    // normal
    double nx = cyl[3];
    double ny = cyl[4];
    double nz = cyl[5];

    // angle of rotation about x
    double phi  = asin(ny);

    // angle of rotation about y
    double theta;
    if (nx == 0 && nz == 0)
        theta = 0.0;
    else
        theta = asin(nx / sqrt(nx*nx + nz*nz));

    double a = cos(theta);
    double b = sin(theta);
    double q = cos(-phi);
    double r = sin(-phi);
    double a2 = a*a;
    double b2 = b*b;
    double q2 = q*q;
    double r2 = r*r;
    double R = cyl[7];
    quadric[0] = a2+r2*b2;
    quadric[1] = q2;
    quadric[2] = b2+r2*a2;
    quadric[3] = 2*q*r*b;
    quadric[4] = 2*q*r*a;
    quadric[5] = 2*a*b*r2-2*a*b;
    quadric[9] = -R*R;
}
static void CylinderPPRToQuadric(const double *const cyl, double *quadric)
{
    double dx = cyl[3] - cyl[0];
    double dy = cyl[4] - cyl[1];
    double dz = cyl[5] - cyl[2];
    double l = sqrt(dx*dx + dy*dy + dz*dz);
    double coeffs[8];
    coeffs[0] = cyl[0];
    coeffs[1] = cyl[1];
    coeffs[2] = cyl[2];
    coeffs[3] = dx / l;
    coeffs[4] = dy / l;
    coeffs[5] = dz / l;
    coeffs[6] = l;
    coeffs[7] = cyl[6];
    CylinderPNLRToQuadric(coeffs, quadric);
}
static void ConePNLAToQuadric(const double *const cone, double *quadric)
{
    // point
    double px = cone[0];
    double py = cone[1];
    double pz = cone[2];

    // normal
    double nx = cone[3];
    double ny = cone[4];
    double nz = cone[5];

    // cone angle
    double alpha = cone[7];

    // angle of rotation about x
    double phi  = asin(ny);

    // angle of rotation about y
    double theta;
    if (nx == 0 && nz == 0)
        theta = 0.0;
    else
        theta = asin(nx / sqrt(nx*nx + nz*nz));

    double a = cos(theta);
    double b = sin(theta);
    double q = cos(-phi);
    double r = sin(-phi);
    double u = tan(alpha*M_PI/180.0);
    double a2 = a*a;
    double b2 = b*b;
    double q2 = q*q;
    double r2 = r*r;
    double u2 = u*u;

    quadric[0] = a2+r2*b2-u2*q2*b2;
    quadric[1] = q2-u2*r2;
    quadric[2] = b2+r2*a2-u2*q2*a2;
    quadric[3] = 2*q*r*b+2*u2*q*r*b;
    quadric[4] = 2*q*r*a+2*u2*q*r*a;
    quadric[5] = 2*a*b*r2-2*a*b-2*u2*q2*a*b;
}
static void ConePPAToQuadric(const double *const cone, double *quadric)
{
    double dx = cone[3] - cone[0];
    double dy = cone[4] - cone[1];
    double dz = cone[5] - cone[2];
    double l = sqrt(dx*dx + dy*dy + dz*dz);
    double coeffs[8];
    coeffs[0] = cone[0];
    coeffs[1] = cone[1];
    coeffs[2] = cone[2];
    coeffs[3] = dx / l;
    coeffs[4] = dy / l;
    coeffs[5] = dz / l;
    coeffs[6] = l;
    coeffs[7] = cone[6];
    ConePNLAToQuadric(coeffs, quadric);
}
static void QuadricToQuadric(const double *const inquad, double *quadric)
{
    for (int i = 0; i < NUM_QCOEFFS; i++)
        quadric[i] = inquad[i];
}

void vtkCSGGrid::AddBoundaries(int nbounds,
    const int *const typeflags, int lcoeffs, const double *const coeffs)
{
    int i;

    // the '+6' is so we can define the 6 faces of the "universe" box
    gridBoundaries = new double[NUM_QCOEFFS * (nbounds + 6)];
    for (i = 0; i < NUM_QCOEFFS * (nbounds + 6); i++)
        gridBoundaries[i] = 0.0;

    int coeffidx = 0;
    int quadidx = 0;
    for (i = 0; i < nbounds; i++)
    {
        switch (typeflags[i])
        {
            case DBCSG_SPHERE_PR:
                SpherePRToQuadric(&coeffs[coeffidx], &gridBoundaries[quadidx]); coeffidx+=4; break;
            case DBCSG_CYLINDER_PNLR:
                CylinderPNLRToQuadric(&coeffs[coeffidx], &gridBoundaries[quadidx]); coeffidx+=8; break;
            case DBCSG_CYLINDER_PPR:
                CylinderPPRToQuadric(&coeffs[coeffidx], &gridBoundaries[quadidx]); coeffidx+=7; break;
            case DBCSG_PLANE_X:
                PlaneXToQuadric(&coeffs[coeffidx], &gridBoundaries[quadidx]); coeffidx+=1; break;
            case DBCSG_PLANE_Y:
                PlaneYToQuadric(&coeffs[coeffidx], &gridBoundaries[quadidx]); coeffidx+=1; break;
            case DBCSG_PLANE_Z:
                PlaneZToQuadric(&coeffs[coeffidx], &gridBoundaries[quadidx]); coeffidx+=1; break;
            case DBCSG_PLANE_PN:
                PlanePNToQuadric(&coeffs[coeffidx], &gridBoundaries[quadidx]); coeffidx+=6; break;
            case DBCSG_PLANE_G:
                PlaneGToQuadric(&coeffs[coeffidx], &gridBoundaries[quadidx]); coeffidx+=4; break;
            case DBCSG_PLANE_PPP:
                PlanePPPToQuadric(&coeffs[coeffidx], &gridBoundaries[quadidx]); coeffidx+=9; break;
#if 0
            // we don't handle compound surfaces yet. They'll break indexing of gridBoundaries
            case DBCSG_BOX_XYZXYZ:
                BoxXYZXYZToQuadric(&coeffs[coeffidx], &gridBoundaries[quadidx]); coeffidx+=6; quadidx += 5*NUM_QCOEFFS; break;
#endif
            case DBCSG_CONE_PNLA:
                ConePNLAToQuadric(&coeffs[coeffidx], &gridBoundaries[quadidx]); coeffidx+=8; break;
            case DBCSG_CONE_PPA:
                ConePPAToQuadric(&coeffs[coeffidx], &gridBoundaries[quadidx]); coeffidx+=7; break;
            case DBCSG_QUADRIC_G:
                QuadricToQuadric(&coeffs[coeffidx], &gridBoundaries[quadidx]); coeffidx+=NUM_QCOEFFS; break;
        }
        quadidx += NUM_QCOEFFS;
    }

    numBoundaries = nbounds + 6;

}
void vtkCSGGrid::AddBoundaries(int nbounds,
    const int *const typeflags, int lcoeffs, const float *const coeffs)
{
    double *tmpcoeffs = new double[lcoeffs];
    for (int i = 0; i < lcoeffs; i++)
        tmpcoeffs[i] = coeffs[i];

    AddBoundaries(nbounds, typeflags, lcoeffs, tmpcoeffs);

    delete [] tmpcoeffs;
}
void vtkCSGGrid::AddRegions(int nregions,
    const int *const lids, const int *const rids,
    const int *const typeflags, int lxforms, const double *const xforms)
{
    // the '+11' is so we can define the universe (bounding box of whole mesh) 
    leftIds = new int[nregions+11];
    rightIds = new int[nregions+11];
    regTypeFlags = new int[nregions+11];

    const int universeId = nregions + 11 - 1; 
    for (int i = 0; i < nregions; i++)
    {
        leftIds[i] = lids[i];
        rightIds[i] = rids[i];
        regTypeFlags[i] = typeflags[i];

        // replace all compliment operations with diffs with "universe"
        if (regTypeFlags[i] == DBCSG_COMPLIMENT)
        {
            regTypeFlags[i] = DBCSG_DIFF;
            rightIds[i] = leftIds[i];
            leftIds[i] = universeId;
        }
    }

    // add the universe
    int bndidx = numBoundaries - 6;
    int quadidx = NUM_QCOEFFS * bndidx; 
    int regidx = nregions;

    // add the min,max X plane boundaries
    PlaneXToQuadric(&Bounds[0], &gridBoundaries[quadidx]); quadidx += NUM_QCOEFFS;
    leftIds[regidx] = bndidx++; rightIds[regidx] = -1; regTypeFlags[regidx++] = DBCSG_OUTER;
    PlaneXToQuadric(&Bounds[1], &gridBoundaries[quadidx]); quadidx += NUM_QCOEFFS;
    leftIds[regidx] = bndidx++; rightIds[regidx] = -1; regTypeFlags[regidx++] = DBCSG_INNER;

    // add the min,max Y plane boundaries
    PlaneYToQuadric(&Bounds[2], &gridBoundaries[quadidx]); quadidx += NUM_QCOEFFS;
    leftIds[regidx] = bndidx++; rightIds[regidx] = -1; regTypeFlags[regidx++] = DBCSG_OUTER;
    PlaneYToQuadric(&Bounds[3], &gridBoundaries[quadidx]); quadidx += NUM_QCOEFFS;
    leftIds[regidx] = bndidx++; rightIds[regidx] = -1; regTypeFlags[regidx++] = DBCSG_INNER;

    // add the min,max Z plane boundaries
    PlaneZToQuadric(&Bounds[4], &gridBoundaries[quadidx]); quadidx += NUM_QCOEFFS;
    leftIds[regidx] = bndidx++; rightIds[regidx] = -1; regTypeFlags[regidx++] = DBCSG_OUTER;
    PlaneZToQuadric(&Bounds[5], &gridBoundaries[quadidx]); quadidx += NUM_QCOEFFS;
    leftIds[regidx] = bndidx++; rightIds[regidx] = -1; regTypeFlags[regidx++] = DBCSG_INNER;

    // add region between X planes
    int xplanes = regidx;
    leftIds[regidx] = nregions + 0;
    rightIds[regidx] = nregions + 1;
    regTypeFlags[regidx++] = DBCSG_INTERSECT;

    // add region between Y planes
    int yplanes = regidx;
    leftIds[regidx] = nregions + 2;
    rightIds[regidx] = nregions + 3;
    regTypeFlags[regidx++] = DBCSG_INTERSECT;

    // add region between Z planes
    int zplanes = regidx;
    leftIds[regidx] = nregions + 4;
    rightIds[regidx] = nregions + 5;
    regTypeFlags[regidx++] = DBCSG_INTERSECT;

    // add region between X & Y planes
    int xyplanes = regidx;
    leftIds[regidx] = xplanes; 
    rightIds[regidx] = yplanes; 
    regTypeFlags[regidx++] = DBCSG_INTERSECT;

    // add region between X, Y & Z planes
    leftIds[regidx] = xyplanes;
    rightIds[regidx] = zplanes; 
    regTypeFlags[regidx++] = DBCSG_INTERSECT;

    numRegions = regidx;
}

void vtkCSGGrid::AddZones(int nzones, const int *const zoneIds)
{
    gridZones = new int[nzones];
    for (int i = 0; i < nzones; i++)
        gridZones[i] = zoneIds[i];

    numZones = nzones;
}

//----------------------------------------------------------------------------
// Programmer: Mark C. Miller, Tue Feb 17 17:54:04 PST 2009
//
// Modifications:
//    Mark C. Miller, Tue Feb 17 20:38:14 PST 2009
//    Fixed bug where NUM_QCOEFFS multiplier was missing from loop over
//    boundaries.
//----------------------------------------------------------------------------
bool vtkCSGGrid::operator==(const vtkCSGGrid &grid) const
{
    int i;

    if (numBoundaries != grid.numBoundaries ||
        numRegions != grid.numRegions ||
        numZones != grid.numZones)
        return false;

    for (i = 0; i < numZones; i++)
    {
        if (gridZones[i] != grid.gridZones[i])
            return false;
    }
    for (i = 0; i < numRegions; i++)
    {
        if (regTypeFlags[i] != grid.regTypeFlags[i] ||
            leftIds[i] != grid.leftIds[i] ||
            rightIds[i] != grid.rightIds[i])
            return false;
    }
    for (i = 0; i < NUM_QCOEFFS * numBoundaries; i++)
    {
        if (gridBoundaries[i] != grid.gridBoundaries[i])
            return false;
    }

    return true;
}

//----------------------------------------------------------------------------
// Modifications:
//
//   Hank Childs, Fri Jun  9 12:54:36 PDT 2006
//   Add "default" to switch statement
//
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

            cylinder->SetCenter((float) coeffs[0] + coeffs[3] * coeffs[6]/2.0,
                                (float) coeffs[1] + coeffs[4] * coeffs[6]/2.0,
                                (float) coeffs[2] + coeffs[5] * coeffs[6]/2.0);
            cylinder->SetRadius((float) coeffs[7]);

            // if the desired cylinder is y-axis aligned, we don't need xform
            if (coeffs[3] != 0.0 || coeffs[5] != 0.0 ||
               (coeffs[4] != 1.0 && coeffs[4] != -1.0))
            {
                double Nx = coeffs[3], Ny = coeffs[4];
                double Nz = coeffs[5];
                double Nl = sqrt(Nx*Nx + Ny*Ny + Nz*Nz);

                if (Nl != 0.0)
                {
                    if (Nl != 1.0) // make sure its normalized
                    {
                        Nx /= Nl; Ny /= Nl; Nz /= Nl;
                    }

                    double rotz = acos(Ny) * 180.0 / M_PI;
                    double roty = acos(Nx/sqrt(1-Ny*Ny)) * 180.0 / M_PI;

                    vtkTransform *xform = vtkTransform::New();
                    xform->RotateZ(-rotz);
                    xform->RotateY(-roty);
                    xform->Translate(-coeffs[0], -coeffs[1], -coeffs[2]);

                    cylinder->SetTransform(xform);
                    xform->Delete();
                }
            }

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

        case PLANE_PN:
        {
            vtkPlane *plane = vtkPlane::New();

            plane->SetOrigin((float) coeffs[0], (float) coeffs[1], (float) coeffs[2]);
            plane->SetNormal((float) coeffs[3], (float) coeffs[4], (float) coeffs[5]);

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

        default:
        {
            // No-op, logic below will handle.
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

        default:
        {
            // No-op, logic below will handle
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
// Modifications:
//
//   Hank Childs, Fri Jun  9 12:54:36 PDT 2006
//   Add "default" to switch statement
//
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
        default:        break;  // Avoid compiler warning
    }

    boolReg->AddFunction(left);
    boolReg->AddFunction(right);

    Regions->AddItem(boolReg);
    boolReg->Delete();
    funcMap[boolReg] = Regions->GetNumberOfItems()-1;
    return funcMap[boolReg]; 
}

//----------------------------------------------------------------------------
// Modifications:
//
//   Hank Childs, Fri Jun  9 12:54:36 PDT 2006
//   Add "default" to switch statement
//
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

        default:
        {
            // No-op, logic below will handle.
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

void
vtkCSGGrid::BuildVTKImplicitFunction(int zoneId, vtkImplicitFunction **func) const
{
    switch (regTypeFlags[zoneId])
    {
        case DBCSG_INNER:
        {
            vtkQuadric *quadric = vtkQuadric::New();
            quadric->SetCoefficients(&gridBoundaries[NUM_QCOEFFS*leftIds[zoneId]]);
            *func = quadric;
            break;
        }
        case DBCSG_OUTER:
        {
            vtkQuadric *quadric = vtkQuadric::New();
            double coeffs[NUM_QCOEFFS];
            for (int i = 0; i < NUM_QCOEFFS; i++) // invert sign on all coeffs
                coeffs[i] = -gridBoundaries[NUM_QCOEFFS*leftIds[zoneId]+i];
            quadric->SetCoefficients(coeffs);
            *func = quadric;
            break;
        }
        case DBCSG_COMPLIMENT:
        {
            vtkImplicitFunction *left;
            BuildVTKImplicitFunction(leftIds[zoneId], &left);
            vtkImplicitBoolean *newfunc = vtkImplicitBoolean::New();
            newfunc->AddFunction(Universe);
            newfunc->AddFunction(left);
            left->Delete();
            newfunc->SetOperationTypeToDifference();
            *func = newfunc;
            break;
        }
        case DBCSG_UNION:
        case DBCSG_INTERSECT:
        case DBCSG_DIFF:
        {
            vtkImplicitFunction *left, *right;
            BuildVTKImplicitFunction(leftIds[zoneId], &left);
            BuildVTKImplicitFunction(rightIds[zoneId], &right);
            vtkImplicitBoolean *newfunc = vtkImplicitBoolean::New();
            newfunc->AddFunction(left);
            newfunc->AddFunction(right);
            switch (regTypeFlags[zoneId])
            {
                case DBCSG_UNION: newfunc->SetOperationTypeToUnion(); break;
                case DBCSG_INTERSECT: newfunc->SetOperationTypeToIntersection(); break;
                case DBCSG_DIFF: newfunc->SetOperationTypeToDifference(); break;
            }
            left->Delete();
            right->Delete();
            *func = newfunc;
            break;
        }
        default:
            break;
    }
}

// This removed code needs to be re-implemented using new class
double epsTol;
#if 1
vtkPolyData  *vtkCSGGrid::DiscretizeSurfaces(
    int specificZone, double tol,
    double minX, double maxX,
    double minY, double maxY,
    double minZ, double maxZ)
{
    vtkAppendPolyData *appender = vtkAppendPolyData::New();

    // fudge the bounds a bit
    minX -= minX * (minX < 0.0 ? -tol : tol);
    minY -= minY * (minY < 0.0 ? -tol : tol);
    minZ -= minZ * (minZ < 0.0 ? -tol : tol);
    minX += minX * (minX < 0.0 ? -tol : tol);
    minY += minY * (minY < 0.0 ? -tol : tol);
    minZ += minZ * (minZ < 0.0 ? -tol : tol);

    //
    // Turn relative tolerance into an absolute tolerance 
    //
    tol = ComputeRelativeTol(tol, minX, maxX, minY, maxY, minZ, maxZ);
    epsTol = tol;
    int nX = (int) ((maxX - minX) / tol);
    int nY = (int) ((maxY - minY) / tol);
    int nZ = (int) ((maxZ - minZ) / tol);

    int startZone = specificZone;
    int endZone = startZone + 1; 

    for (int i = startZone; i < endZone; i++)
    {
        vtkImplicitFunction *reg;
        BuildVTKImplicitFunction(gridZones[i], &reg);

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
        reg->Delete();
    }

    vtkPolyData *rv = appender->GetOutput();
    rv->Update();
    rv->Register(0);
    appender->Delete();
    return rv;
}

// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Fri Jun  9 12:54:36 PDT 2006
//    Remove unused variables.
//
// ****************************************************************************
vtkUnstructuredGrid *vtkCSGGrid::DiscretizeSpace(
    int specificZone, double tol,
    double minX, double maxX,
    double minY, double maxY,
    double minZ, double maxZ)
{
    vtkAppendFilter *appender = vtkAppendFilter::New();

    // fudge the bounds a bit
    minX -= minX * (minX < 0.0 ? -tol : tol);
    minY -= minY * (minY < 0.0 ? -tol : tol);
    minZ -= minZ * (minZ < 0.0 ? -tol : tol);
    minX += minX * (minX < 0.0 ? -tol : tol);
    minY += minY * (minY < 0.0 ? -tol : tol);
    minZ += minZ * (minZ < 0.0 ? -tol : tol);

    tol = ComputeRelativeTol(tol, minX, maxX, minY, maxY, minZ, maxZ);
    epsTol = tol;
    int nX = (int) ((maxX - minX) / tol);
    int nY = (int) ((maxY - minY) / tol);
    int nZ = (int) ((maxZ - minZ) / tol);

    int startZone = specificZone;
    int endZone = startZone + 1;

    for (int i = startZone; i < endZone; i++)
    {
        vtkImplicitFunction *reg;
        BuildVTKImplicitFunction(gridZones[i], &reg);

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
        reg->Delete();
    }

    vtkUnstructuredGrid *rv = appender->GetOutput();
    rv->Update();
    rv->Register(0);
    appender->Delete();
    return rv;
}
#endif


bool
vtkCSGGrid::AddCutZones(vtkUnstructuredGrid *cutBox,
    vtkPoints *points, vtkUnstructuredGrid *ugrid,
    map<float, map<float, map<float, int> > >& nodemap)
{
    bool addedAPiece = false;
    for (int i = 0; i < cutBox->GetNumberOfCells(); i++)
    {
        vtkCell *newCell = cutBox->GetCell(i);
        vtkPoints *cellPoints = newCell->GetPoints();
        int *pointIds = new int[cellPoints->GetNumberOfPoints()]; 

        //
        // Add the *unique* points for this cell
        //
        for (int j = 0; j < cellPoints->GetNumberOfPoints(); j++)
        {
            double *pt = cellPoints->GetPoint(j);
            double x = pt[0], y = pt[1], z = pt[2];

            int Ix = (int) (x / epsTol * 10000.0 + 0.5);
            float fx = Ix * epsTol / 10000.0;
            int Iy = (int) (y / epsTol * 10000.0 + 0.5);
            float fy = Iy * epsTol / 10000.0;
            int Iz = (int) (z / epsTol * 10000.0 + 0.5);
            float fz = Iz * epsTol / 10000.0;

            int mapId = nodemap[fx][fy][fz];
            if (mapId == 0)
            {
                points->InsertNextPoint(fx,fy,fz);
                nodemap[fx][fy][fz] = points->GetNumberOfPoints()-1;
                pointIds[j] = points->GetNumberOfPoints()-1;
            }
            else
            {
                pointIds[j] = mapId;
            }
        }

        //
        // Add this cell
        //
        ugrid->InsertNextCell(newCell->GetCellType(),
               cellPoints->GetNumberOfPoints(), pointIds);
        addedAPiece = true;

        delete [] pointIds;
    }
    return addedAPiece;
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

        int Ix = (int) (x / epsTol * 10000.0 + 0.5);
        float fx = Ix * epsTol / 10000.0;
        int Iy = (int) (y / epsTol * 10000.0 + 0.5);
        float fy = Iy * epsTol / 10000.0;
        int Iz = (int) (z / epsTol * 10000.0 + 0.5);
        float fz = Iz * epsTol / 10000.0;

        int mapId = nodemap[fx][fy][fz];
        if (mapId == 0)
        {
            points->InsertNextPoint(fx,fy,fz);
            nodemap[fx][fy][fz] = points->GetNumberOfPoints()-1;
            pointIds[i] = points->GetNumberOfPoints()-1;
        }
        else
        {
            pointIds[i] = mapId; 
        }
    }
    ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, pointIds);
}

void
vtkCSGGrid::AddBoundariesForZone2(int zoneId, vector<int> *bnds, vector<int> *senses)
{
    switch (regTypeFlags[zoneId])
    {
        case DBCSG_INNER:
            bnds->push_back(leftIds[zoneId]);
            senses->push_back((int)INNER);
            break;
        case DBCSG_OUTER:
            bnds->push_back(leftIds[zoneId]);
            senses->push_back((int)OUTER);
            break;
        case DBCSG_ON:
            bnds->push_back(leftIds[zoneId]);
            senses->push_back((int)ON);
            break;
        case DBCSG_UNION:
        case DBCSG_INTERSECT:
            AddBoundariesForZone2(leftIds[zoneId], bnds, senses);
            AddBoundariesForZone2(rightIds[zoneId], bnds, senses);
            break;
        case DBCSG_DIFF:
            AddBoundariesForZone2(leftIds[zoneId], bnds, senses);
            AddBoundariesForZone2(rightIds[zoneId], bnds, senses);
            if ((*senses)[senses->size()-1] == INNER)
                (*senses)[senses->size()-1] = OUTER; 
            else if ((*senses)[senses->size()-1] == OUTER)
                (*senses)[senses->size()-1] = INNER; 
            break;
        case DBCSG_XFORM:
            AddBoundariesForZone2(leftIds[zoneId], bnds, senses);
            break;
        default:
            break;
    }
}

bool
vtkCSGGrid::MakeMeshZonesByCuttingBox4(const Box *theBox,
    const map<int,int>& boundaryToStateMap,
    map<int,int>& boundaryToSenseMap, int zoneId,
    vtkPoints *points, vtkUnstructuredGrid *ugrid,
    map<float, map<float, map<float, int> > >& nodemap)
{
    //
    // Start by making an initial vtkUnstructuredGrid from the box
    //
    vtkPoints *boxPoints = vtkPoints::New();
    boxPoints->InsertNextPoint(0.0,0.0,0.0); // nodemap entry 0 used for "not set"
    vtkUnstructuredGrid *boxUgrid = vtkUnstructuredGrid::New();
    boxUgrid->SetPoints(boxPoints);
    boxPoints->Delete();
    map<float, map<float, map<float, int> > > dummyNodemap;
    MakeMeshZone(theBox, boxPoints, boxUgrid, dummyNodemap);

    //
    // Set up two "buffers" for pieces that result from repeated clips and
    // the boundaryToState maps for those pieces
    //
    vector<map<int, int> > pieceBoundaryToStateMapsA;
    vector<vtkUnstructuredGrid *> piecesA;
    vector<map<int, int> > pieceBoundaryToStateMapsB;
    vector<vtkUnstructuredGrid *> piecesB;

    //
    // Setup "current" and "next" pointers to point to the appropriate buffers
    //
    vector<vtkUnstructuredGrid *> *piecesCurrent = &piecesA;
    vector<map<int, int> > *pieceBoundaryToStateMapsCurrent = &pieceBoundaryToStateMapsA;
    vector<vtkUnstructuredGrid *> *piecesNext = &piecesB;
    vector<map<int, int> > *pieceBoundaryToStateMapsNext = &pieceBoundaryToStateMapsB;

    //
    // Initialize the current list of pieces to clip (e.g. the initial box)
    //
    piecesCurrent->push_back(boxUgrid);
    map<int, int> dummyMap;
    pieceBoundaryToStateMapsCurrent->push_back(dummyMap);

    //
    // Iterate over boundaries for this zoneId. For each boundary whose state is not
    // already LT_ZERO or GT_ZERO, use that boundary to clip the current list of
    // pieces.
    //
    int bndNum = 1;
    map<int,int>::const_iterator it;
    for (it = boundaryToStateMap.begin(); it != boundaryToStateMap.end(); it++)
    {
        const int bndId = it->first;
        const int boxState = it->second;
        const int sense = boundaryToSenseMap[bndId];

        //
        // Clear out the next "buffers"
        //
        piecesNext->clear();
        pieceBoundaryToStateMapsNext->clear();

        //
        // Estimate size of final result based on current trend and fall back
        // to approximate method if we expect to use too much memory here
        //
        float finalPieceCountEstimate =
            (float) boundaryToStateMap.size() / bndNum * (piecesCurrent->size());
        if (finalPieceCountEstimate > 50000.0)
        {
            // free up all the memory we've used so far
            for (size_t i = 0; i < piecesCurrent->size(); i++)
                (*piecesCurrent)[i]->Delete();

            debug1 << "vtkCSGGrid: Predicting too much memory for cutter4; "
                      "Falling back to cutter2" << endl;

            // call the approximate method
            return MakeMeshZonesByCuttingBox2(theBox, boundaryToStateMap,
                boundaryToSenseMap, zoneId, points, ugrid, nodemap);
        }

        //
        // If the current boundary's state is EQ_ZERO, that means it cut the
        // original box we started with. So, now clip all the pieces we 
        // currently have with it. If the current boundary's state is already
        // known, then just apply its state to all the pieces.
        //
        for (size_t i = 0; i < piecesCurrent->size(); i++)
        {
            // WE HAVE INEFFICIENCY HERE IN THAT WE WIND UP COPYING BUFFERS FOR NON-EQ_ZERO
            // BOUNDARIES BUT WE DO NEED TO APPEND TO THE MAPS FOR ALL THE PIECES
            if (boxState == Box::EQ_ZERO)
            {
                vtkQuadric *quadric = vtkQuadric::New();
                quadric->SetCoefficients(&gridBoundaries[NUM_QCOEFFS*bndId]);
                vtkVisItClipper *pieceCutter = vtkVisItClipper::New();
                pieceCutter->SetInput((*piecesCurrent)[i]);
                pieceCutter->SetUseZeroCrossings(true);
                pieceCutter->SetClipFunction(quadric);

                // THIS LOOP COULD BE REPLACED WITH SINGLE CALL TO VTKVISITCLIPPER
                // TO RETURN BOTH INNER AND OUTER PIECES
                for (int k = 0; k < 2; k++)
                {
                    // k==0 ==> in piece ; k==1 ==> out piece
                    if (sense == (int) INNER)
                        pieceCutter->SetInsideOut(k==0 ? true : false);
                    else
                        pieceCutter->SetInsideOut(k==0 ? false: true);
                    pieceCutter->Modified();
                    //pieceCutter->Update();

                    pieceCutter->GetOutput()->Update();
                    vtkUnstructuredGrid *thePiece = vtkUnstructuredGrid::New();
                    thePiece->ShallowCopy(pieceCutter->GetOutput());

                    if (thePiece->GetNumberOfCells() == 0)
                    {
                        thePiece->Delete();
                    }
                    else
                    {
                        piecesNext->push_back(thePiece);
                        pieceBoundaryToStateMapsNext->push_back((*pieceBoundaryToStateMapsCurrent)[i]);
                        if (sense == (int) INNER)
                        {
                            (*pieceBoundaryToStateMapsNext)[pieceBoundaryToStateMapsNext->size()-1][bndId] = 
                                k == 0 ? Box::LT_ZERO : Box::GT_ZERO;
                        }
                        else
                        {
                            (*pieceBoundaryToStateMapsNext)[pieceBoundaryToStateMapsNext->size()-1][bndId] = 
                                k == 0 ? Box::GT_ZERO : Box::LT_ZERO;
                        }
                    }
                }

                quadric->Delete();
                pieceCutter->Delete();
                (*piecesCurrent)[i]->Delete();

            }
            else
            {
                piecesNext->push_back((*piecesCurrent)[i]);
                pieceBoundaryToStateMapsNext->push_back((*pieceBoundaryToStateMapsCurrent)[i]);
                (*pieceBoundaryToStateMapsNext)[pieceBoundaryToStateMapsNext->size()-1][bndId] = boxState;
            }
        }

        //
        // Update the "buffer" pointers for the next iteration
        //
        if (piecesCurrent == &piecesA)
        {
            piecesCurrent = &piecesB;
            pieceBoundaryToStateMapsCurrent = &pieceBoundaryToStateMapsB;
            piecesNext = &piecesA;
            pieceBoundaryToStateMapsNext = &pieceBoundaryToStateMapsA;
        }
        else
        {
            piecesCurrent = &piecesA;
            pieceBoundaryToStateMapsCurrent = &pieceBoundaryToStateMapsA;
            piecesNext = &piecesB;
            pieceBoundaryToStateMapsNext = &pieceBoundaryToStateMapsB;
        }
        bndNum++;
    }

    //
    // Ok, we've now cut the original box into pieces by recursively
    // cutting by all the EQ_ZERO boundaries, resulting in a long list
    // of pieces whose states are all now either LT_ZERO or GT_ZERO for
    // the boundaries that were originally EQ_ZERO. Now, iterate over
    // all these pieces and decide if each is really "in" or "out" and
    // add them as necessary. Delete the pieces too.
    //
    bool addedAPiece = false;
    for (size_t i = 0; i < piecesCurrent->size(); i++)
    {
        Box::FuncState pieceState = (Box::FuncState) EvalBoxStateOfRegion(0, zoneId,
            (*pieceBoundaryToStateMapsCurrent)[i], 0);
        if (pieceState == Box::LT_ZERO)
        {
            addedAPiece = true;
            AddCutZones((*piecesCurrent)[i], points, ugrid, nodemap);
        }
        (*piecesCurrent)[i]->Delete();
    }

    return addedAPiece;
}

bool
vtkCSGGrid::MakeMeshZonesByCuttingBox2(const Box *theBox,
    const map<int,int>& boundaryToStateMap,
    map<int,int>& boundaryToSenseMap, int zoneId,
    vtkPoints *points, vtkUnstructuredGrid *ugrid,
    map<float, map<float, map<float, int> > >& nodemap)
{
    //
    // Start by making an initial vtkUnstructuredGrid from the box
    //
    vtkPoints *boxPoints = vtkPoints::New();
    boxPoints->InsertNextPoint(0.0,0.0,0.0); // nodemap entry 0 used for "not set"
    vtkUnstructuredGrid *boxUgrid = vtkUnstructuredGrid::New();
    boxUgrid->SetPoints(boxPoints);
    boxPoints->Delete();
    map<float, map<float, map<float, int> > > dummyNodemap;
    MakeMeshZone(theBox, boxPoints, boxUgrid, dummyNodemap);

    //
    // Ok, now iterate over boundary functions cutting and re-cutting
    // the vtkUnstructuredGrid
    //
    map<int,int>::const_iterator it;
    for (it = boundaryToStateMap.begin(); it != boundaryToStateMap.end(); it++)
    {
        if (it->second == Box::EQ_ZERO)
        {
            const int theInt = it->first;

            vtkVisItClipper *boxCutter = vtkVisItClipper::New();
            boxCutter->SetInput(boxUgrid);
            boxCutter->SetUseZeroCrossings(true);

            vtkQuadric *quadric = vtkQuadric::New();
            quadric->SetCoefficients(&gridBoundaries[NUM_QCOEFFS*theInt]);
            boxCutter->SetClipFunction(quadric);

            const int sense = boundaryToSenseMap[theInt]; 
            boxCutter->SetInsideOut(sense == (int) INNER ? true : false);
            boxCutter->Update();

            vtkUnstructuredGrid *cutBox = boxCutter->GetOutput();
            cutBox->Update();
            cutBox->Register(NULL);

            quadric->Delete();
            boxCutter->Delete();
            boxUgrid->Delete();

            if (cutBox->GetNumberOfCells() == 0)
            {
                cutBox->Delete();
                return false;
            }
            else
            {
                boxUgrid = cutBox;
            }
        }
    }

    bool result = AddCutZones(boxUgrid, points, ugrid, nodemap);
    boxUgrid->Delete();
    return result;
}

// ****************************************************************************
//  Modifications:
//
//    Mark C. Miller, Thu Mar 22 19:09:43 PST 2007 
//    Changed logic for flatness test. Flatness test is approximate. So, when
//    a surface is deemed flat, the cutter can still wind up NOT producing
//    any pieces for it. Before, we assumed cutter would produce pieces. When
//    it doesn't we loose pieces of the mesh. Now, if flatness test passes but
//    cutter yields nothing, we subdivide. This is expected to result in at
//    most 1-2 levels more of subdivision for the cases where cutter did not
//    agree with flatness test. 
//
// ****************************************************************************
vtkUnstructuredGrid *vtkCSGGrid::DiscretizeSpace3(
    int specificZone, int rank, int nprocs,
    double discTol, double flatTol,
    double minX, double maxX,
    double minY, double maxY,
    double minZ, double maxZ)
{
    deque<Box*> boxDeque;

    // for building unstructured grid
    vtkPoints *points = vtkPoints::New();
    points->InsertNextPoint(0.0,0.0,0.0); // nodemap entry 0 used for "not set"
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(points);
    points->Delete();
    map<float, map<float, map<float, int> > > nodemap;

    //
    // Convert list of zones to list of boundaries (leaves in
    // the region)
    //
    vector<int> boundariesToCheck;
    vector<int> sensesOfBoundariesInRegion; 
    AddBoundariesForZone2(gridZones[specificZone], &boundariesToCheck,
                          &sensesOfBoundariesInRegion);

    //
    // Create list of boundary ids to index into boundaries collection
    //
    vector<int> boundaryStates;
    for (size_t i = 0; i < boundariesToCheck.size(); i++)
        boundaryStates.push_back((int)Box::EQ_ZERO);

    // fudge the bounds a bit
    minX -= minX * (minX < 0.0 ? -discTol : discTol);
    minY -= minY * (minY < 0.0 ? -discTol : discTol);
    minZ -= minZ * (minZ < 0.0 ? -discTol : discTol);
    minX += minX * (minX < 0.0 ? -discTol : discTol);
    minY += minY * (minY < 0.0 ? -discTol : discTol);
    minZ += minZ * (minZ < 0.0 ? -discTol : discTol);

    Box* boxZero = new Box(minX, maxX, minY, maxY, minZ, maxZ, boundaryStates,
                           DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX,
                           DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX);

    discTol = ComputeRelativeTol(discTol, minX, maxX, minY, maxY, minZ, maxZ);
    epsTol = discTol;

    //
    // Process the deque
    //
    boxDeque.push_back(boxZero);
    while (boxDeque.size() > 0)
    {
        Box* curBox = boxDeque.front();
        boxDeque.pop_front();

        vector<int>  curBoxBoundaryStates = curBox->zids;
        map<int, int> boundaryToStateMap;
        map<int, int> boundaryToSenseMap;
        for (size_t i = 0; i < curBoxBoundaryStates.size(); i++)
        {
            if (curBoxBoundaryStates[i] == (int) Box::EQ_ZERO)
            {
                curBoxBoundaryStates[i] =
                    curBox->EvalBoxStateOfBoundary(&gridBoundaries[NUM_QCOEFFS*boundariesToCheck[i]], discTol);
            }
            boundaryToStateMap[boundariesToCheck[i]] = curBoxBoundaryStates[i];
            boundaryToSenseMap[boundariesToCheck[i]] = sensesOfBoundariesInRegion[i];
        }

        Box::FuncState boxStateRelativeToWholeRegion =
            (Box::FuncState) EvalBoxStateOfRegion(curBox,
                                                  gridZones[specificZone],
                                                  boundaryToStateMap, discTol);

        if (boxStateRelativeToWholeRegion == Box::LT_ZERO)
        {
            MakeMeshZone(curBox, points, ugrid, nodemap);
            delete curBox;
        }
        else if (boxStateRelativeToWholeRegion == Box::GT_ZERO)
        {
            delete curBox;
        }
        else if (curBox->Resolution() > discTol)
        {
            bool flatNessHandledIt = false;
            if (curBox->CanBeCut2(gridBoundaries, boundaryToStateMap, flatTol))
            {
                flatNessHandledIt =
                    MakeMeshZonesByCuttingBox4(curBox, boundaryToStateMap,
                         boundaryToSenseMap, gridZones[specificZone],
                         points, ugrid, nodemap);
                if (!flatNessHandledIt)
                    debug1 << "vtkCSGGrid: Flatness passed; Cutter4 failed. Subdividing..." << endl; 
            }

            if (flatNessHandledIt)
                delete curBox;
            else
            {
                //
                // Subdivide this box and add boxes to deque
                //
                vector<Box*> newBoxes;
                newBoxes = curBox->Subdivide();
                for (size_t j = 0; j < newBoxes.size(); j++)
                {
                    newBoxes[j]->zids = curBoxBoundaryStates;
                    boxDeque.push_back(newBoxes[j]);
                }
                delete curBox;
            }
        }
        else
        {
            MakeMeshZonesByCuttingBox4(curBox, boundaryToStateMap,
                boundaryToSenseMap, gridZones[specificZone],
                points, ugrid, nodemap);
            delete curBox;
        }
    }

    return ugrid;
}
