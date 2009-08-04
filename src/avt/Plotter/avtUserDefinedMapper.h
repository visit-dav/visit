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
  
    virtual void             Render(vtkRenderer *ren, vtkActor *a);

    virtual void             ImmediateModeRenderingOn(void);
    virtual void             ImmediateModeRenderingOff(void);
    virtual void             SetImmediateModeRendering(int );
    virtual int              GetImmediateModeRendering(void);

  protected:
                             vtkUserDefinedMapperBridge();
                            ~vtkUserDefinedMapperBridge() {};

                             vtkUserDefinedMapperBridge(const
                                               vtkUserDefinedMapperBridge&) {};
    void                     operator=(const vtkUserDefinedMapperBridge&) {};

    avtCustomRenderer_p      ren;
};


#endif


