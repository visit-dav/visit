// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          vtkVisItMoleculeMapper.cxx                       //
// ************************************************************************* //


#include <vtkVisItMoleculeMapper.h>

// LLNL
#include <AtomicProperties.h> // common/utility
#include <avtColorTables.h>
#include <visit-config.h> // For LIB_VERSION_GE
// a VisIt class
#include <vtkPointMapper.h>


#include <vtkActor.h>
#include <vtkCellArray.h>
#if LIB_VERSION_GE(VTK,9,1,0)
#include <vtkCellArrayIterator.h>
#endif
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>
#include <vtkGlyph3DMapper.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkSphereSource.h>
#include <vtkTriangle.h>
#include <vtkTrivialProducer.h>
#include <vtkUnsignedCharArray.h>

#include <vector>
using std::string;
using std::vector;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_DETAIL_LEVELS 4

static int cylinder_quality_levels[4] = {
    3,
    5,
    9,
    17
};

static  bool ct_is_discrete = false;

class MoleculeMapperHelper
{
public:
  MoleculeMapperHelper();
 ~MoleculeMapperHelper();

  int CreateRectangleBetweenTwoPoints(double *, double *, float ,
              vtkPoints *, vtkCellArray *);
  int CreateCylinderBetweenTwoPoints(double *, double *, float, int,
              vtkPoints *, vtkCellArray *, vtkFloatArray *);
  int CreateCylinderCap(double *, double *, int, float, int,
              vtkPoints *, vtkCellArray *, vtkFloatArray *);
private:
  void CalculateCylPts(void);

  float *cyl_pts[MAX_DETAIL_LEVELS];
  bool cylinders_calculated;
};

MoleculeMapperHelper::MoleculeMapperHelper()
{
  cylinders_calculated = false;
}

MoleculeMapperHelper::~MoleculeMapperHelper()
{
  for (int i = 0; i < MAX_DETAIL_LEVELS; ++i)
    {
    if (cylinders_calculated)
        delete [] cyl_pts[i];
    }
}


// ****************************************************************************
//  Method:  MoleculeMapperHelper::CalculateCylPts
//
//  Notes:  Extracted from avtOpenGLMoleculeRenderer
//
//  Purpose:
//    Precalculate points for cylinder geometry.
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 10, 2006
//
//  Modifications:
//
// ****************************************************************************

void
MoleculeMapperHelper::CalculateCylPts()
{
  if (cylinders_calculated)
    return;

  cylinders_calculated = true;

  for (int detail=0; detail<4; detail++)
    {
    int cdetail = cylinder_quality_levels[detail];
    cyl_pts[detail] = new float[(cdetail+1)*4];

    for (int b=0; b<=cdetail; b++)
      {
      float theta = 2*M_PI * float(b) / float(cdetail);

      float dx = cos(theta);
      float dy = sin(theta);
      float dz = 0;

      cyl_pts[detail][b*4+0] = dx;
      cyl_pts[detail][b*4+1] = dy;
      cyl_pts[detail][b*4+2] = dz;
      cyl_pts[detail][b*4+3] = 0;
      }
    }
}


// ****************************************************************************
//  Method:  MoleculeMapperHelper::CreateRectangleBetweenTwoPoints
//
//  Notes:  extracted from
//          avtOpenGLMoleculeRenderer::DrawRectangleBetweenTwoPoints
//
//  Purpose:
//    Make the OpenGL calls to draw a rectangle with the given begin
//    and end points and radius.  Assumes drawing in the z==0 plane.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 22, 2010
//
//  Modifications:
//
// ****************************************************************************

int
MoleculeMapperHelper::CreateRectangleBetweenTwoPoints(double *p0, double *p1,
    float r, vtkPoints *pts, vtkCellArray *cells)
{
  float v[3] = {(float)(p0[1]-p1[1]), (float)(p1[0]-p0[0]), 0.f};
  float v_len = vtkMath::Normalize(v);
  if (v_len == 0)
    return 0;

  vtkNew<vtkIdList> ids;
  ids->InsertNextId(pts->InsertNextPoint(p0[0] + r*v[0], p0[1] + r*v[1], 0.));
  ids->InsertNextId(pts->InsertNextPoint(p1[0] + r*v[0], p1[1] + r*v[1], 0.));
  ids->InsertNextId(pts->InsertNextPoint(p1[0] - r*v[0], p1[1] - r*v[1], 0.));
  ids->InsertNextId(pts->InsertNextPoint(p0[0] - r*v[0], p0[1] - r*v[1], 0.));
  cells->InsertNextCell(ids.GetPointer());
  return 1;
}


// ****************************************************************************
//  Method:  MoleculeMapperHelper::CreateCylinderBetweenTwoPoints
//
//  Notes:  extracted from
//          avtOpenGLMoleculeRenderer::DrawCylinderBetweenTwoPoints
//
//  Purpose:
//    Make the OpenGL calls to draw a cylinder with the given begin
//    and end points, radius, and detail level.
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 10, 2006
//
//  Modifications:
//    Jeremy Meredith, Mon Aug 28 18:25:02 EDT 2006
//    Point locations are now doubles, not floats.
//
// ****************************************************************************

int
MoleculeMapperHelper::CreateCylinderBetweenTwoPoints(double *p0, double *p1,
    float r, int detail, vtkPoints *pts, vtkCellArray *cells,
    vtkFloatArray *normals)
{
  this->CalculateCylPts();

  int ncells = 0;
  float vc[3] = {static_cast<float>(p1[0]-p0[0]),
                 static_cast<float>(p1[1]-p0[1]),
                 static_cast<float>(p1[2]-p0[2])
                };
  float va[3];
  float vb[3];

  float vc_len = vtkMath::Normalize(vc);
  if (vc_len == 0)
    return ncells;

  vtkMath::Perpendiculars(vc, va,vb, 0);

  float v0[4];
  float v1[4];
  int cdetail = cylinder_quality_levels[detail];
  for (int b=0; b<cdetail; b++, ncells++)
    {
    float *u0 = &(cyl_pts[detail][b*4]);
    float *u1 = &(cyl_pts[detail][(b+1)*4]);

    v0[0] = va[0]*u0[0] + vb[0]*u0[1];
    v0[1] = va[1]*u0[0] + vb[1]*u0[1];
    v0[2] = va[2]*u0[0] + vb[2]*u0[1];

    v1[0] = va[0]*u1[0] + vb[0]*u1[1];
    v1[1] = va[1]*u1[0] + vb[1]*u1[1];
    v1[2] = va[2]*u1[0] + vb[2]*u1[1];

    normals->InsertNextTypedTuple(v0);
    normals->InsertNextTypedTuple(v0);
    normals->InsertNextTypedTuple(v1);
    normals->InsertNextTypedTuple(v1);

    vtkNew<vtkIdList> ids;
    ids->InsertNextId(pts->InsertNextPoint(
         p1[0] + r*v0[0], p1[1] + r*v0[1], p1[2] + r*v0[2]));

    ids->InsertNextId(pts->InsertNextPoint(
         p0[0] + r*v0[0], p0[1] + r*v0[1], p0[2] + r*v0[2]));

    ids->InsertNextId(pts->InsertNextPoint(
         p0[0] + r*v1[0], p0[1] + r*v1[1], p0[2] + r*v1[2]));

    ids->InsertNextId(pts->InsertNextPoint(
         p1[0] + r*v1[0], p1[1] + r*v1[1], p1[2] + r*v1[2]));
    cells->InsertNextCell(ids.GetPointer());
    }
  return ncells;
}


// ****************************************************************************
//  Method:  MoleculeMapperHelper::CreateCylinderCap
//
//  Notes:  extracted from avtOpenGLMoleculeRenderer::DrawCylinderCap
//
//  Purpose:
//    Make the OpenGL calls to draw a cylinder cap with the given begin
//    and end points, radius, and detail level.  The cap is drawn at the
//    second point; the first point is used for orientation.
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 25, 2010
//
//  Modifications:
//    Kathleen Biagas, June 16, 2021
//    Add normals for all inserted ids.
//
// ****************************************************************************

int
MoleculeMapperHelper::CreateCylinderCap(double *p0, double *p1, int half,
    float r, int detail, vtkPoints *pts, vtkCellArray *cells,
    vtkFloatArray *normals)
{
  CalculateCylPts();

  int ncells = 0;
  float vc[3] = {static_cast<float>(p1[0]-p0[0]),
                 static_cast<float>(p1[1]-p0[1]),
                 static_cast<float>(p1[2]-p0[2])
                };
  float va[3];
  float vb[3];

  float vc_len = vtkMath::Normalize(vc);
  if (vc_len == 0)
    return ncells;

  vtkMath::Perpendiculars(vc, va,vb, 0);

  float v0[4];

  vtkNew<vtkIdList> ids;
  if (half==0)
    {
    normals->InsertNextTypedTuple(vc);
    ids->InsertNextId(pts->InsertNextPoint(p1));
    }
  else
    {
    //normals->InsertNextTypedTuple(-vc[0],-vc[1],-vc[2]);
    normals->InsertNextTypedTuple(vc);
    ids->InsertNextId(pts->InsertNextPoint(p0));
    }

  int cdetail = cylinder_quality_levels[detail];
  for (int b=0; b<=cdetail; b++)
    {
    int b0 = b;

    float *u0;
    u0 = &(cyl_pts[detail][b0*4]);

    v0[0] = va[0]*u0[0] + vb[0]*u0[1];
    v0[1] = va[1]*u0[0] + vb[1]*u0[1];
    v0[2] = va[2]*u0[0] + vb[2]*u0[1];

    if (half==0)
      {
      ids->InsertNextId(pts->InsertNextPoint(p1[0] + r*v0[0], p1[1] + r*v0[1], p1[2] + r*v0[2]));
      normals->InsertNextTypedTuple(vc);
      }
    else
      {
      ids->InsertNextId(pts->InsertNextPoint(p0[0] + r*v0[0], p0[1] + r*v0[1], p0[2] + r*v0[2]));
      normals->InsertNextTypedTuple(vc);
      }
    }

  for (int i = 1; i < ids->GetNumberOfIds()-1; ++i)
    {
    vtkNew<vtkTriangle> tri;
    tri->GetPointIds()->SetId(0, ids->GetId(0));
    tri->GetPointIds()->SetId(0, ids->GetId(i));
    tri->GetPointIds()->SetId(0, ids->GetId(i+1));
    cells->InsertNextCell(tri.GetPointer());
    ncells++;
    }
    // close the fan
    vtkNew<vtkTriangle> tri;
    tri->GetPointIds()->SetId(0, ids->GetId(0));
    tri->GetPointIds()->SetId(0, ids->GetId(ids->GetNumberOfIds()-1));
    tri->GetPointIds()->SetId(0, ids->GetId(1));
    cells->InsertNextCell(tri.GetPointer());
    ncells++;
    return ncells;
}



vtkStandardNewMacro(vtkVisItMoleculeMapper)

//----------------------------------------------------------------------------
// Modifications:
//   Kathleen Biagas, Fri Jun 18 2021
//   Register 'AtomPolyData' to this class to prevent strange crash under
//   certain conditions when plot attributes update. (Bug #5794)

vtkVisItMoleculeMapper::vtkVisItMoleculeMapper()
  : RenderAtoms(true),
    DrawAtomsAs(Spheres),
    SphereQuality(Medium),
    RadiusType(Fixed),
    RadiusScaleFactor(1.),
    RadiusFixed(0.3),
    RadiusVariable(),
    RenderBonds(true),
    DrawBondsAs(Lines),
    CylinderQuality(Medium),
    BondRadius(0.12),
    BondColorMode(ColorByAtom),
    ElementCTName(),
    ResSeqCTName(),
    ResTypeCTName(),
    ContinuousCTName(),
    ColorTableName()
{
  // Initialize ivars:
  this->BondColor[0] = this->BondColor[1] = this->BondColor[2] = 50;

  // from avtOpenGLMoleculeRenderer
  this->levelsLUT = NULL;
  this->NumColors = 0;
  this->MolColors = NULL;
  this->VarMin = this->VarMax = 0.;
  this->Is2D = false;

  this->Helper = new MoleculeMapperHelper();

  // ATOMS
  // Setup glyph sources
  this->sphere = vtkSphereSource::New();
  this->sphere->LatLongTessellationOn();
  this->sphere->SetRadius(1.0);
  this->sphere->SetThetaResolution(12);
  this->sphere->SetPhiResolution(7);
  this->sphere->Update();

  this->AtomMapper = vtkGlyph3DMapper::New();
  this->AtomMapper->SetSourceConnection(sphere->GetOutputPort());
  this->AtomMapper->SetScaleModeToScaleByMagnitude();

  this->AtomPolyData = vtkPolyData::New();
  this->AtomPolyData->Register(this);
  // Connect the trivial producers to forward the glyph polydata

  this->AtomOutput = vtkTrivialProducer::New();
  this->AtomOutput->SetOutput(this->AtomPolyData);
  this->AtomMapper->SetInputConnection (this->AtomOutput->GetOutputPort());

  this->ImposterMapper = vtkPointMapper::New();
  this->ImposterMapper->SetColorModeToMapScalars();
  this->ImposterMapper->SetScalarModeToUsePointData();
  this->ImposterMapper->UseImpostersOn();
  this->ImposterMapper->SetInputConnection(this->AtomOutput->GetOutputPort());

  // Connect the trivial producers to forward the glyph polydata
  this->BondLinesPolyData = vtkPolyData::New();
  this->BondCylsPolyData = vtkPolyData::New();

  this->BondOutput = vtkTrivialProducer::New();
  this->BondOutput->SetOutput(this->BondLinesPolyData);

  this->BondMapper = vtkPolyDataMapper::New();
  this->BondMapper->SetInputConnection
    (this->BondOutput->GetOutputPort());

  // Force the atom and bond data to be generated on the next render:
  this->AtomDataInitialized = false;
  this->BondDataInitialized = false;
}

//----------------------------------------------------------------------------
vtkVisItMoleculeMapper::~vtkVisItMoleculeMapper()
{
  if (this->MolColors)
    delete[] this->MolColors;
  this->MolColors = NULL;
  delete this->Helper;

  this->sphere->Delete();
  this->AtomPolyData->Delete();
  this->AtomOutput->Delete();
  this->BondLinesPolyData->Delete();
  this->BondCylsPolyData->Delete();
  this->BondOutput->Delete();
  this->AtomMapper->Delete();
  this->ImposterMapper->Delete();
  this->BondMapper->Delete();
}


//----------------------------------------------------------------------------
void vtkVisItMoleculeMapper::Render(vtkRenderer *ren, vtkActor *act )
{
  // Update cached polydata if needed
  this->UpdatePolyData();

  // Pass rendering call on
  if (this->RenderAtoms)
    {
    if (this->DrawAtomsAs == Spheres)
      this->AtomMapper->Render(ren, act);
    else
      this->ImposterMapper->Render(ren, act);
    }

  if (this->RenderBonds)
    {
    this->BondMapper->Render(ren, act);
    }
}


//----------------------------------------------------------------------------
void vtkVisItMoleculeMapper::ReleaseGraphicsResources(vtkWindow *w)
{
  this->AtomMapper->ReleaseGraphicsResources(w);
  this->ImposterMapper->ReleaseGraphicsResources(w);
  this->BondMapper->ReleaseGraphicsResources(w);
}


//----------------------------------------------------------------------------
double *vtkVisItMoleculeMapper::GetBounds()
{
  vtkDataSet *input = this->GetInput();
  if (!input)
    {
    vtkMath::UninitializeBounds(this->Bounds);
    }
  else
    {
    if (!this->Static)
      {
      this->Update();
      }
    input->GetBounds(this->Bounds);
    // Pad bounds by 3 Angstrom to contain spheres, etc
    this->Bounds[0] -= 3.0;
    this->Bounds[1] += 3.0;
    this->Bounds[2] -= 3.0;
    this->Bounds[3] += 3.0;
    this->Bounds[4] -= 3.0;
    this->Bounds[5] += 3.0;
    }
  return this->Bounds;
}


// from avtOpenGLMoleculeRenderer::SetColors
void
vtkVisItMoleculeMapper::SetColors()
{
  vtkDataSet *data = GetInput();
  int new_numcolors = -1;

  avtColorTables *ct = avtColorTables::Instance();
  const int opacity = 255;

  //
  // Detect if we're using the default color table or a color table
  // that does not exist anymore.
  //
  string new_colortablename;
  string varName = "";
  if (data->GetPointData()->GetScalars())
    {
    varName = data->GetPointData()->GetScalars()->GetName();
    }
    else if (data->GetCellData()->GetScalars())
    {
    varName = data->GetCellData()->GetScalars()->GetName();
    }

  if (varName == "")
    {
    new_colortablename = "";
    new_numcolors = 0;
    }
  else if (varName == "element" ||
          (varName.length()>7 && varName.substr(0,7)=="element") ||
          (varName.length()>8 && varName.substr(varName.length()-8)=="/element"))
    {
    new_colortablename = this->ElementCTName;
    if (new_colortablename == "Default")
      new_colortablename = string(ct->GetDefaultDiscreteColorTable());

    new_numcolors = 110;
    }
  else if (varName == "resseq" ||
          (varName.length()>7 && varName.substr(varName.length()-7)=="/resseq"))
    {
    new_colortablename = this->ResSeqCTName;
    if (new_colortablename == "Default")
      new_colortablename = string(ct->GetDefaultDiscreteColorTable());

    new_numcolors = 256;
    }
  else if (varName == "restype" ||
          (varName.length()>8 && varName.substr(varName.length()-8)=="/restype"))
    {
    new_colortablename = this->ResTypeCTName;
    if (new_colortablename == "Default")
      new_colortablename = string(ct->GetDefaultDiscreteColorTable());

    new_numcolors = NumberOfKnownResidues();
    }
  else
    {
    new_colortablename = this->ContinuousCTName;
    if (new_colortablename == "Default")
      new_colortablename = string(ct->GetDefaultContinuousColorTable());

    new_numcolors = 256;
    }
  //
  // Skip the work if we don't need to do it!
  //
  if (new_numcolors      == this->NumColors   &&
      new_colortablename == this->ColorTableName)
    {
    return;
    }

  this->NumColors      = new_numcolors;
  this->ColorTableName = new_colortablename;

  if (this->MolColors)
    delete[] this->MolColors;

  this->MolColors = new unsigned char[this->NumColors * 4];
  unsigned char *cptr = this->MolColors;
  //
  // Add a color for each subset name.
  //
  if (ct->IsDiscrete(this->ColorTableName.c_str()))
    {
    // The CT is discrete, get its color color control points.
    ct_is_discrete = true;
    for (int i = 0; i < this->NumColors; ++i)
      {
      unsigned char rgb[3] = {0,0,0};
      ct->GetControlPointColor(this->ColorTableName.c_str(), i, rgb);
      *cptr++ = rgb[0];
      *cptr++ = rgb[1];
      *cptr++ = rgb[2];
      *cptr++ = opacity;
      }
    }
  else
    {
    // The CT is continuous, sample the CT so we have a unique color
    // for each element.
    ct_is_discrete = false;
    unsigned char *rgb = ct->GetSampledColors(this->ColorTableName.c_str(), this->NumColors);
    if (rgb)
      {
      for (int i = 0; i < this->NumColors; ++i)
        {
        int j = i * 3;
        *cptr++ = rgb[j];
        *cptr++ = rgb[j+1];
        *cptr++ = rgb[j+2];
        *cptr++ = opacity;
        }
      delete [] rgb;
      }
    }
}


// ****************************************************************************
//  Method:  vtkVisItMoleculeMapper::InvalidateColors
//
//  Purpose:
//
//  Arguments:
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
// ****************************************************************************
void
vtkVisItMoleculeMapper::InvalidateColors()
{
    this->NumColors = 0;
    if (this->MolColors)
        delete[] this->MolColors;
    this->MolColors = NULL;
    this->ColorTableName = "";
    this->AtomPolyData->GetPointData()->RemoveArray("Colors");
    this->AtomMapper->ClearColorArrays();
    this->ImposterMapper->ClearColorArrays();
    this->BondLinesPolyData->GetPointData()->RemoveArray("Colors");
    this->BondCylsPolyData->GetPointData()->RemoveArray("Colors");
    this->BondMapper->ClearColorArrays();
    // need to update the whole poly-data objects
    this->AtomDataInitialized = false;
    this->BondDataInitialized = false;
}


//----------------------------------------------------------------------------
void vtkVisItMoleculeMapper::UpdatePolyData()
{
  vtkDataSet *input = this->GetInput();

  if (!this->AtomDataInitialized || (
        (input->GetMTime() > this->AtomPolyData->GetMTime() ||
         this->GetMTime() > this->AtomPolyData->GetMTime()) &&
        this->RenderAtoms))
    {
    this->UpdateAtomPolyData();
    this->AtomDataInitialized = true;
    }

  if (!this->BondDataInitialized || (
        (input->GetMTime() > this->BondLinesPolyData->GetMTime() ||
         this->GetMTime() > this->BondLinesPolyData->GetMTime()) &&
        this->RenderBonds))
    {
    this->UpdateBondPolyData();
    this->BondDataInitialized = true;
    }

}

//----------------------------------------------------------------------------
// Generate scale and position information for each atom sphere
//
// Modifications:
//   Kathleen Biagas, Fri Jun 18 2021
//   Register 'scol' array to this class to prevent strange crash under
//   certain conditions when plot attributes update. (Bug #5794)
//
//    Kathleen Biagss, Thu Aug 11, 2022
//    Support VTK9: use vtkCellArrayIterator.
//

void vtkVisItMoleculeMapper::UpdateAtomPolyData()
{
  this->AtomPolyData->Initialize();

  this->SetColors();

  switch (this->SphereQuality)
    {
    case Low:
      sphere->SetThetaResolution(6);
      sphere->SetPhiResolution(4);
      break;
    case High:
      sphere->SetThetaResolution(24);
      sphere->SetPhiResolution(13);
      break;
    case Super:
      sphere->SetThetaResolution(45);
      sphere->SetPhiResolution(25);
      break;
    case Medium:
    default:
      sphere->SetThetaResolution(12);
      sphere->SetPhiResolution(7);
      break;
    }
  sphere->Update();

  vtkPolyData *input = vtkPolyData::SafeDownCast(this->GetInput());
  vtkPoints *points = input->GetPoints();
  int numpoints = input->GetNumberOfPoints();
  int numverts = input->GetNumberOfVerts();

  vtkDataArray *primary = input->GetPointData()->GetScalars();
  // TODO: allow cell-centered variables to (probably as an option)
  //       be drawn on atoms; some use cases -- like connected components
  //       -- do viably label both atoms and bonds.
  if (!primary)
    {
    // Let's just assume we don't want to plot the spheres for
    // a cell-centered variable
    return;
    }
  float *scalar = (float*)primary->GetVoidPointer(0);
  vector<float> scalar_storage;
  if (!primary->IsA("vtkFloatArray"))
    {
    int n = numpoints;
    scalar_storage.resize(n);
    for (int i=0; i<n; i++)
      scalar_storage[i] = primary->GetComponent(i,0);
    scalar = &(scalar_storage[0]);
    }

  string primaryname = primary->GetName();
  bool primary_is_element = (primaryname == "element" ||
                  (primaryname.length() > 7 &&
                   primaryname.substr(0,7) == "element") ||
                  (primaryname.length() > 8 &&
                   primaryname.substr(primaryname.length()-8) == "/element"));
  bool primary_is_resseq = (primaryname == "resseq" ||
                  (primaryname.length() > 7 &&
                   primaryname.substr(primaryname.length()-7) == "/resseq"));
  bool primary_is_restype = (primaryname == "restype" ||
                  (primaryname.length() > 8 &&
                   primaryname.substr(primaryname.length()-8) == "/restype"));

  vtkDataArray *element = primary_is_element ? primary :
                                 input->GetPointData()->GetArray("element");

  if (element && !element->IsA("vtkFloatArray"))
    {
    vtkWarningMacro(<<"vtkVisItMoleculeMapper: found a non-float array\n");
    return;
    }
  float *elementnos = element ? (float*)element->GetVoidPointer(0) : NULL;


  bool color_by_levels = (primary_is_element ||
                          primary_is_restype ||
                          primary_is_resseq)     && ct_is_discrete;
  bool color_by_element = color_by_levels && primary_is_element;

  float *radiusvar = NULL;
  bool sbv  = this->RadiusType == Variable;
  bool sbar = this->RadiusType == Atomic;
  bool sbcr = this->RadiusType == Covalent;
  float radiusscale = this->RadiusScaleFactor;

  if (sbv)
    {
    if (this->RadiusVariable == "default")
      radiusvar = scalar;
    else
      {
      vtkDataArray *radius_array = input->GetPointData()->GetArray(
                                    this->RadiusVariable.c_str());
      if (!radius_array)
        {
        // This shouldn't have gotten this far if it couldn't
        // read the variable like we asked.
        vtkErrorMacro(<<"Couldn't read radius variable.\n");
        }
      if (radius_array && !radius_array->IsA("vtkFloatArray"))
        {
        vtkWarningMacro(<<"vtkVisItMoleculeMapper: found a non-float array\n");
        return;
        }
        radiusvar = (float*)radius_array->GetVoidPointer(0);
      }
    }

  vtkNew<vtkUnsignedCharArray> scol;
  scol->SetName("Colors");
  scol->SetNumberOfComponents(4);
  scol->Allocate(numverts*3);
  scol->Register(this);

  vtkPoints *pts = points->NewInstance();
  pts->Allocate(numverts*4);

  vtkNew<vtkCellArray> cells;
  cells->Allocate(numverts);

  vtkNew<vtkFloatArray> normals;
  normals->Allocate(numverts*4);

  // for imposters
  vtkNew<vtkFloatArray> scaleFactors;
//  if (this->DrawAtomsAs == Imposters)
    {
    scaleFactors->SetName("ScaleFactors");
    scaleFactors->Allocate(numverts);
    }

#if LIB_VERSION_LE(VTK,8,1,0)
  vtkIdType *vertptr = input->GetVerts()->GetPointer();
  for (int ix=0; ix<numverts; ix++, vertptr += (1+*vertptr))
    {
    if (*vertptr != 1)
      continue;

    int atom = *(vertptr+1);
#else
  auto verts = vtk::TakeSmartPointer(input->GetVerts()->NewIterator());
  for (verts->GoToFirstCell(); !verts->IsDoneWithTraversal(); verts->GoToNextCell())
    {
    vtkIdList *ids = verts->GetCurrentCell();
    if (ids->GetNumberOfIds() != 1)
      continue;

    vtkIdType atom = ids->GetId(0);
#endif

    int element_number = 0;
    if (element)
      element_number = int(elementnos[atom]);

    if (element_number < 0 || element_number > MAX_ELEMENT_NUMBER)
      element_number = 0;

    // Determine radius
    float radius = this->RadiusFixed;
    if (element && sbar)
      radius = atomic_radius[element_number] * radiusscale;
    else if (element && sbcr)
      radius = covalent_radius[element_number] * radiusscale;
    else if (radiusvar && sbv)
      radius = radiusvar[atom] * radiusscale;


    int npts = 1;
    // Create spheres
    double *pt = points->GetPoint(atom);
    vtkIdType id = pts->InsertNextPoint(pt);
    cells->InsertNextCell(1, &id);
    scaleFactors->InsertNextValue(radius);
    // Determine color
    if (color_by_element)
      {
      int level = element_number % this->NumColors;
      for (int i = 0; i < npts; ++i)
        scol->InsertNextTypedTuple(&MolColors[4*level]);
      }
    else if (color_by_levels)
      {
      int level = int(scalar[atom]) - (primary_is_resseq ? 1 : 0);
      if(levelsLUT != 0)
        {
        const unsigned char *rgb =
              levelsLUT->MapValue(level);
        for (int i = 0; i < npts; ++i)
          scol->InsertNextTypedTuple(rgb);
        }
      else
        {
        level = level % this->NumColors;
        for (int i = 0; i < npts; ++i)
          scol->InsertNextTypedTuple(&MolColors[4*level]);
        }
      }
    else
      {
      float alpha;
      if (this->VarMax == this->VarMin)
        alpha = 0.5;
      else
        alpha = (scalar[atom] - this->VarMin) / (this->VarMax - this->VarMin);

      int color = int((float(this->NumColors)-.01) * alpha);
      if (color < 0)
        color = 0;
      if (color > this->NumColors-1)
        color = this->NumColors-1;
      for (int i = 0; i < npts; ++i)
        scol->InsertNextTypedTuple(&MolColors[4*color]);
      }
    }
  pts->Squeeze();
  cells->Squeeze();
  scol->Squeeze();
  this->AtomPolyData->SetPoints(pts);
  this->AtomPolyData->GetPointData()->SetScalars(scol.GetPointer());

  this->AtomPolyData->SetVerts(cells.GetPointer());
  this->AtomPolyData->GetPointData()->AddArray(scaleFactors.GetPointer());
  this->AtomMapper->SetScaleArray("ScaleFactors");
  this->ImposterMapper->SetImposterScaleArray("ScaleFactors");
}

//----------------------------------------------------------------------------
// Generate position, scale, and orientation vectors for each bond cylinder
//
//  Modifications:
//    Kathleen Biagas, June 16, 2021
//    Use separate color arrays for lines and cylinders as they have different
//    number of cells.
//
//    Kathleen Biagss, Thu Aug 11, 2022
//    Added logic to check for radiusvar.
//    Support VTK9: use vtkCellArrayIterator.
//

void vtkVisItMoleculeMapper::UpdateBondPolyData()
{
  this->BondLinesPolyData->Initialize();
  this->BondCylsPolyData->Initialize();

  vtkPolyData *input = vtkPolyData::SafeDownCast(this->GetInput());
  vtkPoints *points = input->GetPoints();
  int numpoints = input->GetNumberOfPoints();
  int numverts = input->GetNumberOfVerts();
  int numlines = input->GetNumberOfLines();

  vtkPoints *linePoints = points->NewInstance();
  vtkNew<vtkCellArray> lineLines;
  vtkPoints *cylPoints = points->NewInstance();
  vtkNew<vtkCellArray> cylPolys;
  vtkNew<vtkFloatArray> cylNorms;
  cylNorms->SetNumberOfComponents(3);
  vtkNew<vtkUnsignedCharArray> cylinderBondColors;
  cylinderBondColors->SetName("Colors");
  cylinderBondColors->SetNumberOfComponents(3);
  vtkNew<vtkUnsignedCharArray> lineBondColors;
  lineBondColors->SetName("Colors");
  lineBondColors->SetNumberOfComponents(3);

  bool primary_is_cell_centered = false;
  vtkDataArray *primary = input->GetPointData()->GetScalars();
  if (!primary)
    {
    primary = input->GetCellData()->GetScalars();
    primary_is_cell_centered = true;
    }
  if (!primary)
    {
    // eh? no variable at all?  that's a logic error....
    return;
    }

  float *scalar = (float*)primary->GetVoidPointer(0);
  vector<float> scalar_storage;
  if (!primary->IsA("vtkFloatArray"))
    {
    int n = primary_is_cell_centered ? (numlines+numverts) : numpoints;
    scalar_storage.resize(n);
    for (int i=0; i<n; i++)
      scalar_storage[i] = primary->GetComponent(i,0);
    scalar = &(scalar_storage[0]);
    }

  string primaryname = primary->GetName();
  bool primary_is_element = (primaryname == "element" ||
                  (primaryname.length() > 7 &&
                   primaryname.substr(0,7) == "element") ||
                  (primaryname.length() > 8 &&
                   primaryname.substr(primaryname.length()-8) == "/element"));
  bool primary_is_resseq = (primaryname == "resseq" ||
                  (primaryname.length() > 7 &&
                   primaryname.substr(primaryname.length()-7) == "/resseq"));
  bool primary_is_restype = (primaryname == "restype" ||
                  (primaryname.length() > 8 &&
                   primaryname.substr(primaryname.length()-8) == "/restype"));

  vtkDataArray *element = primary_is_element ? primary :
                                 input->GetPointData()->GetArray("element");
  if (element && !element->IsA("vtkFloatArray"))
    {
    vtkWarningMacro(<< "vtkVisItMoleculeMapper: found a non-float array\n");
    return;
    }
  float *elementnos = element ? (float*)element->GetVoidPointer(0) : NULL;

  bool color_by_levels = (primary_is_element ||
                          primary_is_restype ||
                          primary_is_resseq)     && ct_is_discrete;
  bool color_by_element = color_by_levels && primary_is_element;

  //
  // Get radius variable
  //

  float *radiusvar = NULL;
  bool sbv  = this->RadiusType == Variable;
  bool sbar = this->RadiusType == Atomic;
  bool sbcr = this->RadiusType == Covalent;
  float radiusscale = this->RadiusScaleFactor;

  if (sbv)
    {
    if (this->RadiusVariable == "default")
      radiusvar = scalar;
    else
      {
      vtkDataArray *radius_array = input->GetPointData()->GetArray(
                                    this->RadiusVariable.c_str());
      if (!radius_array)
        {
        // This shouldn't have gotten this far if it couldn't
        // read the variable like we asked.
        vtkErrorMacro(<<"Couldn't read radius variable.\n");
        }
      if (radius_array && !radius_array->IsA("vtkFloatArray"))
        {
        vtkWarningMacro(<<"vtkVisItMoleculeMapper: found a non-float array\n");
        return;
        }
        radiusvar = (float*)radius_array->GetVoidPointer(0);
      }
    }

  // We only want to draw a bond-half if its adjacent atom is a "real" atom.
  vector<bool> hasVertex(numpoints,false);
#if LIB_VERSION_LE(VTK,8,1,0)
  vtkCellArray *lines = input->GetLines();
  vtkIdType *vertptr = input->GetVerts()->GetPointer();
  for (int i=0; i<input->GetNumberOfVerts(); i++, vertptr += (1+*vertptr))
    {
    int atom = *(vertptr+1);
    hasVertex[atom] = true;
    }
  vtkIdType *segments = lines->GetPointer();
  vtkIdType *segptr = segments;
  int lineIndex = 0;
  for (int i=0; i<input->GetNumberOfLines(); i++)
    {
    if (*segptr == 2)
      {
      int v0 = *(segptr+1);
      int v1 = *(segptr+2);
#else
  auto verts = vtk::TakeSmartPointer(input->GetVerts()->NewIterator());
  for(verts->GoToFirstCell(); !verts->IsDoneWithTraversal(); verts->GoToNextCell())
    {
    vtkIdList *ids = verts->GetCurrentCell();
    hasVertex[ids->GetId(0)] = true;
    }

  int lineIndex = 0;
  auto lines = vtk::TakeSmartPointer(input->GetLines()->NewIterator());
  for(lines->GoToFirstCell(); !lines->IsDoneWithTraversal(); lines->GoToNextCell(), ++lineIndex)
    {
    vtkIdList *lineIds = lines->GetCurrentCell();
    if (lineIds->GetNumberOfIds() == 2)
      {
      vtkIdType v0 = lineIds->GetId(0);
      vtkIdType v1 = lineIds->GetId(1);
#endif

      double pt_0[3];
      double pt_1[3];
      points->GetPoint(v0, pt_0);
      points->GetPoint(v1, pt_1);

      double pt_mid[3] = {(pt_0[0]+pt_1[0])/2.,
                          (pt_0[1]+pt_1[1])/2.,
                          (pt_0[2]+pt_1[2])/2.};


      double dpt[3] = {pt_1[0]-pt_0[0], pt_1[1]-pt_0[1], pt_1[2]-pt_0[2]};
      double dptlen = sqrt(dpt[0]*dpt[0]+dpt[1]*dpt[1]+dpt[2]*dpt[2]);
      vtkMath::Normalize(dpt);


      for (int half=0; half<=1; half++)
        {
        vtkIdType atom     = (half==0) ? v0 : v1;
        vtkIdType otherAtom= (half==0) ? v1 : v0;
        double *pt_a = (half==0) ? pt_0 : pt_mid;
        double *pt_b = (half==0) ? pt_mid : pt_1;

        if (!hasVertex[atom])
          continue;

        int element_number = 0;
        if (element)
          {
          element_number = int(elementnos[atom]);
          }

        if (element_number < 0 || element_number > 109) // MAX_ELEMENT_NUMBER)
          element_number = 0;


        if (!primary_is_cell_centered && this->DrawAtomsAs == Imposters)
          {
          // Determine radius
          float atom_radius = this->RadiusFixed;
          if (element && sbar)
            atom_radius = atomic_radius[element_number] * radiusscale;
          else if (element && sbcr)
            atom_radius = covalent_radius[element_number] * radiusscale;
          else if (radiusvar && sbv)
            atom_radius = radiusvar[atom] * radiusscale;

          if (atom_radius > dptlen/2.)
            continue;

          const float fudge = 0.9;

          if (half == 0)
            {
            pt_a[0] += atom_radius * dpt[0] * fudge;
            pt_a[1] += atom_radius * dpt[1] * fudge;
            pt_a[2] += atom_radius * dpt[2] * fudge;
            }
          else
            {
            pt_b[0] -= atom_radius * dpt[0] * fudge;
            pt_b[1] -= atom_radius * dpt[1] * fudge;
            pt_b[2] -= atom_radius * dpt[2] * fudge;
            }
          }
        float radius = this->BondRadius;

        int ncells = 1;
        // do the cylinders
        {
        if (this->Is2D)
          {
          this->Helper->CreateRectangleBetweenTwoPoints(pt_a, pt_b, radius,
                                          cylPoints, cylPolys.GetPointer());
          }
        else
          {
          ncells = this->Helper->CreateCylinderBetweenTwoPoints(pt_a, pt_b,
                                 radius, this->CylinderQuality, cylPoints,
#if LIB_VERSION_LE(VTK,8,1,0)
                                 cylPolys.GetPointer(), cylNorms.GetPointer());
#else
                                 cylPolys, cylNorms);
#endif
          if (!hasVertex[otherAtom])
            {
            ncells += this->Helper->CreateCylinderCap(pt_a, pt_b, half, radius,
                                this->CylinderQuality, cylPoints,
#if LIB_VERSION_LE(VTK,8,1,0)
                                cylPolys.GetPointer(), cylNorms.GetPointer());
#else
                                cylPolys, cylNorms);
#endif
            }
          // TODO: modify this next test if we allow drawing
          //       atoms with a primary cell-centered variable
          if (primary_is_cell_centered)
            {
            ncells += this->Helper->CreateCylinderCap(pt_0, pt_1, 1-half,
                                radius, this->CylinderQuality, cylPoints,
#if LIB_VERSION_LE(VTK,8,1,0)
                                cylPolys.GetPointer(), cylNorms.GetPointer());
#else
                                cylPolys, cylNorms);
#endif
            }
          } // 3D
        } // do the cylinder
        // do the lines
        {
        vtkNew<vtkIdList> ids;
        ids->InsertNextId(linePoints->InsertNextPoint(pt_a));
        ids->InsertNextId(linePoints->InsertNextPoint(pt_b));
        lineLines->InsertNextCell(ids.GetPointer());
        }


        // Determine color
        if (this->BondColorMode == SingleColor)
          {
          unsigned char bc[3] = {255,0, 0};
          for (int i = 0;i < ncells; ++i)
            cylinderBondColors->InsertNextTypedTuple(this->BondColor);
          lineBondColors->InsertNextTypedTuple(this->BondColor);
          }
        else // (this->BondColorMode == ColorByAtom)
          {
          float scalarval;
          if (primary_is_cell_centered)
            scalarval = scalar[lineIndex + numverts];
          else
            scalarval = scalar[atom];

          if (color_by_element)
            {
            int level = element_number % this->NumColors;
            for (int i = 0;i < ncells; ++i)
              cylinderBondColors->InsertNextTypedTuple(&this->MolColors[4*level]);
            lineBondColors->InsertNextTypedTuple(&this->MolColors[4*level]);
            }
          else if (color_by_levels)
            {
            int level = int(scalarval) - (primary_is_resseq ? 1 : 0);
            if(levelsLUT != 0)
              {
              const unsigned char *rgb =
              levelsLUT->MapValue(level);
              for (int i = 0;i < ncells; ++i)
                cylinderBondColors->InsertNextTypedTuple(rgb);
              lineBondColors->InsertNextTypedTuple(rgb);
              }
            else
              {
              level = level % this->NumColors;
              for (int i = 0;i < ncells; ++i)
                cylinderBondColors->InsertNextTypedTuple(&this->MolColors[4*level]);
              lineBondColors->InsertNextTypedTuple(&this->MolColors[4*level]);
              }
            }
          else
            {
            float alpha;
            if (this->VarMax == this->VarMin)
              alpha = 0.5;
            else
              alpha = (scalarval - this->VarMin) / (this->VarMax - this->VarMin);

            int color = int((float(this->NumColors)-.01) * alpha);
            if (color < 0)
              color = 0;
            if (color > this->NumColors-1)
              color = this->NumColors-1;
            for (int i = 0;i < ncells; ++i)
              cylinderBondColors->InsertNextTypedTuple(&this->MolColors[4*color]);
            lineBondColors->InsertNextTypedTuple(&this->MolColors[4*color]);
            }
          } // color by atom
        } // for half
      }
#if LIB_VERSION_LE(VTK,8,1,0)
      segptr += (*segptr) + 1;
#endif
    }
  this->BondLinesPolyData->SetPoints(linePoints);
  this->BondLinesPolyData->SetLines(lineLines.GetPointer());
  this->BondLinesPolyData->GetCellData()->SetScalars(lineBondColors.GetPointer());
  this->BondCylsPolyData->SetPoints(cylPoints);
  this->BondCylsPolyData->SetPolys(cylPolys.GetPointer());
  this->BondCylsPolyData->GetCellData()->SetScalars(cylinderBondColors.GetPointer());
  this->BondCylsPolyData->GetPointData()->SetNormals(cylNorms.GetPointer());
}


//----------------------------------------------------------------------------
void vtkVisItMoleculeMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "AtomMapper:\n";
  this->AtomMapper->PrintSelf(os, indent.GetNextIndent());

  os << indent << "ImposterMapper:\n";
  this->ImposterMapper->PrintSelf(os, indent.GetNextIndent());

  os << indent << "BondMapper:\n";
  this->BondMapper->PrintSelf(os, indent.GetNextIndent());
}

void vtkVisItMoleculeMapper::SetDrawBondsAs(int type)
{
  DrawBondsAs = type;
  if (DrawBondsAs == vtkVisItMoleculeMapper::Lines)
    this->BondOutput->SetOutput(this->BondLinesPolyData);
  else
    this->BondOutput->SetOutput(this->BondCylsPolyData);
  this->BondMapper->Modified();
}


