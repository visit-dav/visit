// ************************************************************************* //
//                           avtUserDefinedMapper.h                          //
// ************************************************************************* //

#ifndef AVT_USER_DEFINED_MAPPER_H
#define AVT_USER_DEFINED_MAPPER_H

#include <plotter_exports.h>


#include <avtCustomRenderer.h>
#include <avtMapper.h>

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
//    Adde virtual methods in support of lighting. 
//    
// ****************************************************************************

class PLOTTER_API avtUserDefinedMapper : public avtMapper
{
  public:
                               avtUserDefinedMapper(avtCustomRenderer_p);
    virtual                   ~avtUserDefinedMapper() {;};

    bool                       GetDataRange(float &, float &);    
    bool                       GetCurrentDataRange(float &, float &);    

    virtual void               GlobalLightingOn(void);
    virtual void               GlobalLightingOff(void);
    virtual void               GlobalSetAmbientCoefficient(const float);

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
// ****************************************************************************

class PLOTTER_API vtkUserDefinedMapperBridge : public vtkDataSetMapper
{
  public:
    vtkTypeMacro(vtkUserDefinedMapperBridge, vtkDataSetMapper);
    static vtkUserDefinedMapperBridge *New();
  
    void                     SetRenderer(avtCustomRenderer_p);
  
    virtual void             Render(vtkRenderer *ren, vtkActor *a);

  protected:
                             vtkUserDefinedMapperBridge();
                            ~vtkUserDefinedMapperBridge() {};

                             vtkUserDefinedMapperBridge(const
                                               vtkUserDefinedMapperBridge&) {};
    void                     operator=(const vtkUserDefinedMapperBridge&) {};

    avtCustomRenderer_p      ren;
};


#endif


