// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtUserDefinedMapper.h                          //
// ************************************************************************* //

#ifndef AVT_USER_DEFINED_MAPPER_H
#define AVT_USER_DEFINED_MAPPER_H

#include <plotter_exports.h>


#include <avtCustomRenderer.h>
#include <avtMapper.h>

class     ColorAttribute;
class     vtkUserDefinedMapperBridge;


// ****************************************************************************
//  Class: avtUserDefinedMapper
//
//  Purpose:
//      A mapper that allows vtkMappers to be bypassed.  It allows someone
//      write custom rendering calls directly.
//      
//  Programmer: Hank Childs
//  Creation:   March 26, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Oct 4 13:36:54 PDT 2001
//    Added GetDataRange, GetCurrentDataRange. 
//
//    Hank Childs, Mon May 20 10:41:32 PDT 2002
//    Made the custom renderer a ref_ptr, to prevent problems with referencing
//    a deleted object on redraws.
//
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002    
//    Added virtual methods in support of lighting. 
//    
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Kathleen Bonnell, Thu Sep  2 11:44:09 PDT 2004 
//    Added SetSurfaceRepresentation and SetSpecularProperties.
//
//    Brad Whitlock, Wed Feb 7 16:03:22 PST 2007
//    Added SetAlternateDisplay.
//
// ****************************************************************************

class PLOTTER_API avtUserDefinedMapper : public avtMapper
{
  public:
                               avtUserDefinedMapper(avtCustomRenderer_p);
    virtual                   ~avtUserDefinedMapper();

    bool                       GetDataRange(double &, double &);    
    bool                       GetCurrentDataRange(double &, double &);    

    virtual void               GlobalLightingOn(void);
    virtual void               GlobalLightingOff(void);
    virtual void               GlobalSetAmbientCoefficient(const double);
    virtual void               SetSurfaceRepresentation(int rep);
    virtual void               SetSpecularProperties(bool,double,double,
                                                      const ColorAttribute&);

    virtual void               SetAlternateDisplay(void *);

    virtual void               ReducedDetailModeOn();
    virtual bool               ReducedDetailModeOff();
  protected:
    avtCustomRenderer_p        renderer;
    vtkDataSetMapper          *CreateMapper(void);

    virtual void               CustomizeMappers(void);
};


#include <vtkDataSetMapper.h>


// ****************************************************************************
//  Class: vtkUserDefinedMapperBridge
//
//  Purpose:
//      A vtk module that can be inserted where a vtkDataSetMapper would
//      normally go, but calls an avtCustomRenderer instead of doing the
//      mapping.
//
//  Programmer: Hank Childs
//  Creation:   March 26, 2001
//
//  Modifications:
//    Kathleen Bonnell, Mon Aug  4 11:35:53 PDT 2003
//    Added methods for setting ImmediateModeRendering.
//
// ****************************************************************************

class PLOTTER_API vtkUserDefinedMapperBridge : public vtkDataSetMapper
{
  public:
    vtkTypeMacro(vtkUserDefinedMapperBridge, vtkDataSetMapper);
    static vtkUserDefinedMapperBridge *New();
  
    void                     SetRenderer(avtCustomRenderer_p);
  
    virtual void             Render(vtkRenderer *ren, vtkActor *a) override;

  protected:
                             vtkUserDefinedMapperBridge();
    virtual                 ~vtkUserDefinedMapperBridge();

                             vtkUserDefinedMapperBridge(const vtkUserDefinedMapperBridge&);
    void                     operator=(const vtkUserDefinedMapperBridge&);

    avtCustomRenderer_p      ren;
};


#endif
