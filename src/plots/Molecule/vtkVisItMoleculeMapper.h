/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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


// ************************************************************************* //
//                         vtkVisItMoleculeMapper.h                          //
// ************************************************************************* //


#ifndef vtkVisItMoleculeMapper_h
#define vtkVisItMoleculeMapper_h

#include <vtkDataSetMapper.h>
#include <vtkNew.h> // For vtkNew


#include <string> 

class vtkGlyph3DMapper;
class vtkLookupTable;
class vtkPointMapper;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkSphereSource;
class vtkTrivialProducer;
class MoleculeMapperHelper;


// ****************************************************************************
// Class: vtkVisItMoleculeMapper
//
// Purpose:
//   Subclass of vtkPolyDataMapper that glyphs polydata representing
//   molecular data.
//       Points(Atoms) are glyphed as spheres.
//       Lines (Bonds) may be glyphed as cylinders. 
//
// Notes:
//   Taken partly from vtkMoleculeMapper, modified to accept vtkPolyData as
//   input instead of vtkMolecule.
//
//   Also taken partly from avtOpenGLMoleculeRenderer.
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 22, 2016 
//
//  Modifications:
//
// **************************************************************************** 

class vtkVisItMoleculeMapper : public vtkDataSetMapper
{
public:
  static vtkVisItMoleculeMapper *New();
  vtkTypeMacro(vtkVisItMoleculeMapper,vtkDataSetMapper);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Reimplemented from base class
  void Render(vtkRenderer *, vtkActor *) override;
  void ReleaseGraphicsResources(vtkWindow *) override;
  double * GetBounds() override;
  void GetBounds(double bounds[6]) override { vtkAbstractMapper3D::GetBounds(bounds); }
  bool GetSupportsSelection() override {return false;}


  enum AtomicRadiusType {
    Fixed = 0,
    Covalent,
    Atomic,
    Variable
  };

  enum ColorType {
    ColorByAtom = 0,
    SingleColor
  };

  enum AtomType {
    Spheres = 0,
    Imposters
  };

  enum BondType {
    Lines = 0,
    Cylinders
  };

  enum Quality{
    Low = 0,     
    Medium,
    High,
    Super
  };


  // Description:
  // Get/Set whether or not to render atoms. Default: On.
  vtkGetMacro(RenderAtoms, bool);
  vtkSetMacro(RenderAtoms, bool);
  vtkBooleanMacro(RenderAtoms, bool);

  // Description:
  // Get/Set how to draw atoms: as sphere geometry or sphere imposters
  // Default: Spheres
  vtkGetMacro(DrawAtomsAs, int);
  vtkSetClampMacro(DrawAtomsAs, int, 0,1);
  void DrawAtomsAsSpheres()
    {
    this->SetDrawAtomsAs(Spheres);
    }
  void DrawAtomsAsImposters()
    {
    this->SetDrawAtomsAs(Imposters);
    }

  // Description:
  // Get/Set the quality for sphere atoms
  // Default: Spheres
  vtkGetMacro(SphereQuality, int);
  vtkSetClampMacro(SphereQuality, int, 0,3);
  void SetSphereQualityToLow()
    {
    this->SetSphereQuality(Low);
    }
  void SetSphereQualityToMedium()
    {
    this->SetSphereQuality(Medium);
    }
  void SetSphereQualityToHigh()
    {
    this->SetSphereQuality(High);
    }
  void SetSphereQualityToSuper()
    {
    this->SetSphereQuality(Super);
    }


  // Description:
  // Get/Set the type of radius used to generate the atoms. Default:
  //  Fixed
  vtkGetMacro(RadiusType, int);
  vtkSetClampMacro(RadiusType, int, 0, 3);
  void SetRadiusTypeToFixed()
    {
    this->SetRadiusType(Fixed);
    }
  void SetRadiusTypeToCovalent()
    {
    this->SetRadiusType(Covalent);
    }
  void SetRadiusTypeToAtomic()
    {
    this->SetRadiusType(Atomic);
    }
  void SetRadiusTypeToVariable()
    {
    this->SetRadiusType(Variable);
    }

  // Description:
  // Get/Set the uniform scaling factor applied to the atoms.
  // Default: 1.0.
  vtkGetMacro(RadiusScaleFactor, float);
  vtkSetMacro(RadiusScaleFactor, float);


  // Description:
  // Get/Set the fixed scaling applied to the atoms.
  // Default: 0.3.
  vtkGetMacro(RadiusFixed, float);
  vtkSetMacro(RadiusFixed, float);

  // Description:
  // Get/Set the variable name to be used for radius scaling
  // Default: None (empty)
  vtkGetMacro(RadiusVariable, std::string);
  vtkSetMacro(RadiusVariable, std::string);


  // Description:
  // Get/Set whether or not to render bonds. Default: On.
  vtkGetMacro(RenderBonds, bool);
  vtkSetMacro(RenderBonds, bool);
  vtkBooleanMacro(RenderBonds, bool);

  // Description:
  // Get/Set how the bonds are drawn.
  // If 'Lines', the bonds will be drawn as simple lines.
  // If 'Cylinders' is selected, each bond will be drawn with cylinder.
  vtkGetMacro(DrawBondsAs, int);
  void SetDrawBondsAs(int);
  void DrawBondsAsLines()
    {
    this->SetDrawBondsAs(Lines);
    }
  void DrawBondsAsCylinders()
    {
    this->SetDrawBondsAs(Cylinders);
    }


  // Description:
  // Get/Set the quality for sphere atoms
  // Default: Spheres
  vtkGetMacro(CylinderQuality, int);
  vtkSetClampMacro(CylinderQuality, int, 0,3);
  void SetCylinderQualityToLow()
    {
    this->SetCylinderQuality(Low);
    }
  void SetCylinderQualityToMedium()
    {
    this->SetCylinderQuality(Medium);
    }
  void SetCylinderQualityToHigh()
    {
    this->SetCylinderQuality(High);
    }
  void SetCylinderQualityToSuper()
    {
    this->SetCylinderQuality(Super);
    }

  // Description:
  // Get/Set the radius of the bond cylinders. Default: 0.12
  vtkGetMacro(BondRadius, float);
  vtkSetMacro(BondRadius, float);


  // Description:
  // Get/Set the method by which bonds are colored.
  // If 'SingleColor' is used, all bonds will be the same color. Use
  // SetBondColor to set the rgb values used.
  // If 'ColorByAtom' is selected, each bond is colored using the
  // same lookup table as the atoms at each end, with a sharp color
  // boundary at the bond center.
  vtkGetMacro(BondColorMode, int);
  vtkSetClampMacro(BondColorMode, int, 0 ,1);
  void SetBondColorModeToColorByAtom()
    {
    this->SetBondColorMode(ColorByAtom);
    }
  void SetBondColorModeToSingleColor()
    {
    this->SetBondColorMode(SingleColor);
    }

  // Description:
  // Get/Set the color of the bonds as an rgb tuple.
  // Default: {50, 50, 50} (dark grey)
  vtkGetVector3Macro(BondColor, unsigned char);
  vtkSetVector3Macro(BondColor, unsigned char);



  // ColorTable Stuff
  virtual void InvalidateColors(void);
  void SetLevelsLUT(vtkLookupTable *lut)
    {
    this->levelsLUT = lut;
    }

  vtkGetMacro(ElementCTName, std::string);
  vtkSetMacro(ElementCTName, std::string);

  vtkGetMacro(ResSeqCTName, std::string);
  vtkSetMacro(ResSeqCTName, std::string);

  vtkGetMacro(ResTypeCTName, std::string);
  vtkSetMacro(ResTypeCTName, std::string);

  vtkGetMacro(ContinuousCTName, std::string);
  vtkSetMacro(ContinuousCTName, std::string);

  void SetRange(double m, double x) { VarMin = m; VarMax = x;}

  vtkGetMacro(Is2D, bool);
  vtkSetMacro(Is2D, bool);

protected:
  vtkVisItMoleculeMapper();
  ~vtkVisItMoleculeMapper();


  // Description:
  // Customize atom rendering
  bool        RenderAtoms;
  int         DrawAtomsAs;
  int         SphereQuality;
  int         RadiusType;
  float       RadiusScaleFactor;
  float       RadiusFixed;
  std::string RadiusVariable;



  // Description:
  // Customize bond rendering
  bool          RenderBonds;
  int           DrawBondsAs;
  int           CylinderQuality;
  float         BondRadius;
  int           BondColorMode;
  unsigned char BondColor[3];


  // Description:
  // Internal render methods
  void GlyphRender(vtkRenderer *ren, vtkActor *act);

  // Description:
  // Cached variables and update methods
  vtkNew<vtkSphereSource> sphere;
  vtkNew<vtkPolyData> AtomPolyData;
  vtkNew<vtkTrivialProducer> AtomOutput;
  vtkNew<vtkPolyData> BondLinesPolyData;
  vtkNew<vtkPolyData> BondCylsPolyData;
  vtkNew<vtkTrivialProducer> BondOutput;
  bool AtomDataInitialized;
  bool BondDataInitialized;
  virtual void UpdatePolyData();
  virtual void UpdateAtomPolyData();
  virtual void UpdateBondPolyData();

  // Description:
  // Internal mappers
  vtkNew<vtkGlyph3DMapper>  AtomMapper;
  vtkNew<vtkPointMapper>    ImposterMapper;
  vtkNew<vtkPolyDataMapper> BondMapper;


private:
  vtkVisItMoleculeMapper(const vtkVisItMoleculeMapper&); 
  void operator=(const vtkVisItMoleculeMapper&);

  void SetColors(void);

  std::string ElementCTName;
  std::string ResSeqCTName;
  std::string ResTypeCTName;
  std::string ContinuousCTName;
  std::string ColorTableName;

  vtkLookupTable *levelsLUT;

  int NumColors;
  unsigned char *MolColors;
  double VarMin;
  double VarMax;
  bool   Is2D;

  MoleculeMapperHelper *Helper;
};

#endif
