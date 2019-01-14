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
//                         vtkPointGlyphMapper.h                             //
// ************************************************************************* //


#ifndef vtkPointGlyphMapper_h
#define vtkPointGlyphMapper_h

#include <vtkPolyDataMapper.h>
#include <vtkNew.h>

#include <plotter_exports.h>

#include <string>
#include <GlyphTypes.h>

class vtkLookupTable;
class vtkPointGlyphMapperHelper;
class vtkPointMapper;
class vtkTrivialProducer;
class vtkVisItGlyph3D;
class vtkVisItPolyDataNormals;

// ****************************************************************************
// Class: vtkPointGlyphMapper
//
// Purpose:
//   Subclass of vtkPolyDataMapper that can draw points as glyphs, points,
//   or sphere imposters.
//
//  Programmer: Kathleen Biagas
//  Creation:   August 17, 2016
//
//  Modifications:
//
// ****************************************************************************

class PLOTTER_API vtkPointGlyphMapper : public vtkPolyDataMapper
{
public:
  static vtkPointGlyphMapper *New();
  vtkTypeMacro(vtkPointGlyphMapper,vtkPolyDataMapper);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void Render(vtkRenderer *, vtkActor *) override;
  void ReleaseGraphicsResources(vtkWindow *) override;

  // To pass on to underlying mappers
  void SetScalarRange(double, double) override;
  void SetLookupTable(vtkLookupTable *lut);

  vtkGetMacro(SpatialDimension, int);
  vtkSetMacro(SpatialDimension, int);

  void SetGlyphType(GlyphType);
  GlyphType  GetGlyphType(void);
  void SetScale(double);
  void DataScalingOff(void);
  void DataScalingOn(const std::string &, int);
  void ColorByScalarOn(const std::string &);
  void ColorByScalarOff(void);
  bool SetFullFrameScaling(bool useScale, const double *s);

protected:
  vtkPointGlyphMapper();
  ~vtkPointGlyphMapper();

  void RenderPiece(vtkRenderer *, vtkActor *) override;

  // Description:
  bool        UseImposters;
  bool        PointDataInitialized;
  int         SpatialDimension;


  // Cached variables
  vtkNew<vtkPolyData> PointPolyData;
  vtkNew<vtkTrivialProducer> PointOutput;
  vtkNew<vtkVisItGlyph3D>         GlyphFilter;
  vtkNew<vtkVisItPolyDataNormals> NormalsFilter;
  vtkNew<vtkPolyDataMapper>       GlyphMapper;
  vtkNew<vtkPointMapper>          PointMapper;

  void UpdatePointData();

private:
  vtkPointGlyphMapper(const vtkPointGlyphMapper&);
  void operator=(const vtkPointGlyphMapper&);

  vtkPointGlyphMapperHelper *Helper;
};

#endif
