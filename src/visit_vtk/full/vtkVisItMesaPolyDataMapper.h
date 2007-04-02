/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItMesaPolyDataMapper.h,v $
  Language:  C++
  Date:      $Date: 2002/08/22 18:39:30 $
  Version:   $Revision: 1.20 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItMesaPolyDataMapper - a PolyDataMapper for the Mesa library
// .SECTION Description
// vtkVisItMesaPolyDataMapper is a subclass of vtkPolyDataMapper.
// vtkVisItMesaPolyDataMapper is a geometric PolyDataMapper for the Mesa 
// rendering library.

#ifndef __vtkVisItMesaPolyDataMapper_h
#define __vtkVisItMesaPolyDataMapper_h

#include "vtkPolyDataMapper.h"
#include <visit_vtk_exports.h>

class vtkProperty;
class vtkRenderWindow;
class vtkMesaRenderer;
class vtkTimerLog;

#define SPHERE_TEX_W 64
#define SPHERE_TEX_H 64

// ****************************************************************************
//  Modifications:
//  
//    Hank Childs, Tue May 25 10:04:36 PDT 2004
//    Break display lists up into smaller display lists.
//
//    Brad Whitlock, Thu Aug 24 16:06:03 PST 2006
//    I added support for color texturing for point data.
//
// ****************************************************************************

class VISIT_VTK_API vtkVisItMesaPolyDataMapper : public vtkPolyDataMapper
{
public:
  static vtkVisItMesaPolyDataMapper *New();
  vtkTypeRevisionMacro(vtkVisItMesaPolyDataMapper,vtkPolyDataMapper);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Implement superclass render method.
  virtual void RenderPiece(vtkRenderer *ren, vtkActor *a);

  // Description:
  // Release any graphics resources that are being consumed by this mapper.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Draw method for Mesa.
  virtual int Draw(vtkRenderer *ren, vtkActor *a);

  typedef enum {TEXTURE_NO_POINTS,
                TEXTURE_USING_POINTSPRITES
                // room for more methods such as shaders
               } PointTextureMode;

  // Description:
  // Sets/Gets the point texturing method. 
  vtkSetMacro(PointTextureMethod, PointTextureMode);
  vtkGetMacro(PointTextureMethod, PointTextureMode);

  // Description:
  // Sets/Gets the whether color texturing is enabled.
  vtkSetMacro(EnableColorTexturing, bool);
  vtkGetMacro(EnableColorTexturing, bool);
  
protected:
  vtkVisItMesaPolyDataMapper();
  ~vtkVisItMesaPolyDataMapper();

  int ListStart;
  int CurrentList;
  int nLists;
  bool doingDisplayLists;
  int  primsInCurrentList;

  // Description:
  // Method for texturing the points as spheres.
  PointTextureMode PointTextureMethod; 

  // Description:
  // Whether the texture data has been created. Used with PointTextureMethod
  // equal to TEXTURE_USING_POINTSPRITES.
  bool SphereTexturesDataCreated;

  // Description:
  // Whether the texture data has been loaded. Used with PointTextureMethod
  // equal to TEXTURE_USING_POINTSPRITES.
  bool SphereTexturesLoaded;

  // Description:
  // Contains the sphere texture that we use when the point texturing mode
  // is set to TEXTURE_USING_POINTSPRITES.
  unsigned char SphereTexture[SPHERE_TEX_H][SPHERE_TEX_W][2];

  // Description:
  // Contains the names of the textures. Used with PointTextureMethod
  // equal to TEXTURE_USING_POINTSPRITES.
  unsigned int  TextureName;

  // Description:
  // Contains the GL state for alpha testing and blending so we can restore
  // if if we do point texturing.
  struct TextureState
  {
    int   isBlendEnabled;
    int   blendFunc0;
    int   blendFunc1;
    int   needAlphaTest;
    int   isAlphaTestEnabled;
    int   alphaTestFunc;
    float alphaTestRef;
  };

  // Description:
  // Called to set up textures, etc when we want to draw textured points.
  void StartFancyPoints(TextureState &atts);

  // Description:
  // Called to restore previous OpenGL state after drawing textured points.
  void EndFancyPoints(TextureState &atts);

  // Description:
  // Makes the sphere textures used when PointTextureMethod is
  // equal to TEXTURE_USING_POINTSPRITES.
  void MakeTextures();

  bool          EnableColorTexturing;
  bool          ColorTexturingAllowed;
  bool          ColorTextureLoaded;
  bool          ColorTextureLooksDiscrete;
  unsigned int  ColorTextureName;
  float        *ColorTexture;
  int           ColorTextureSize;
  bool          OpenGLSupportsVersion1_2;

  bool MapScalarsWithTextureSupport(double);
  void BeginColorTexturing();
  void EndColorTexturing();
  bool UsesPointData(vtkDataSet *input, int scalarMode,
                     int arrayAccessMode, int arrayId, const char *arrayName,
                     int& offset);

  vtkRenderWindow *RenderWindow;   // RenderWindow used for the previous render
private:
  // Description:
  // True if the point sprite extension is supported.
  bool PointSpriteSupported;

  vtkVisItMesaPolyDataMapper(const vtkVisItMesaPolyDataMapper&);  // Not implemented.
  void operator=(const vtkVisItMesaPolyDataMapper&);  // Not implemented.
};

#endif
