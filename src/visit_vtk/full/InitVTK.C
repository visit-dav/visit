/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                                 InitVTK.C                                 //
// ************************************************************************* //

#include <InitVTK.h>
#include <InitVTKNoGraphics.h>
#include <vtkVisItCellDataToPointData.h>
#include <vtkVisItDataSetMapper.h>
#include <vtkVisItOpenGLPolyDataMapper.h>
#include <vtkVisItMesaPolyDataMapper.h>
#include <vtkVisItRectilinearGrid.h>
#include <vtkVisItStructuredGrid.h>
#if defined(__APPLE__)
#include <vtkOSMesaRenderWindow.h>
#endif

#include <vtkObjectFactory.h>
#include <vtkVersion.h>
#if !defined(_WIN32)
#include <vtkGraphicsFactory.h>
#include <vtkImagingFactory.h>
#endif


//
// A factory that will allow VisIt to override any vtkObject
// with a sub-class of that object.
//
class vtkVisItGraphicsFactory : public vtkObjectFactory
{
  public:
    vtkVisItGraphicsFactory();
    static vtkVisItGraphicsFactory* New() { return new vtkVisItGraphicsFactory;};
    virtual const char* GetVTKSourceVersion();
    const char* GetDescription() { return "vtkVisItGraphicsFactory"; };

  protected:
    vtkVisItGraphicsFactory(const vtkVisItGraphicsFactory&);
    void operator=(const vtkVisItGraphicsFactory&);
};

//
// Necessary for each object that will override a vtkObject.
//
VTK_CREATE_CREATE_FUNCTION(vtkVisItOpenGLPolyDataMapper);
VTK_CREATE_CREATE_FUNCTION(vtkVisItMesaPolyDataMapper);
VTK_CREATE_CREATE_FUNCTION(vtkVisItCellDataToPointData);
VTK_CREATE_CREATE_FUNCTION(vtkVisItDataSetMapper);
VTK_CREATE_CREATE_FUNCTION(vtkVisItRectilinearGrid);
VTK_CREATE_CREATE_FUNCTION(vtkVisItStructuredGrid);

#if defined(__APPLE__)
VTK_CREATE_CREATE_FUNCTION(vtkOSMesaRenderWindow);
#endif


const char*
vtkVisItGraphicsFactory::GetVTKSourceVersion()
{
    return VTK_SOURCE_VERSION;
}

//
//  Create the overrides so that VTK will use VisIt's class instead.
//
//  Modifications:
//    Hank Childs, Fri Oct 22 07:13:35 PDT 2004
//    Added vtkVisItCellDataToPointData.
//
//    Brad Whitlock, Wed Jul 13 15:11:53 PST 2005
//    Added vtkOSMesaRenderWindow on MacOS X.
//
//    Brad Whitlock, Fri Aug 26 10:29:26 PDT 2005
//    Added override for vtkDataSetMapper.
//
//    Dave Bremer, Fri May 25 11:48:48 PDT 2007
//    Added override for vtkVisItStructuredGrid and vtkVisItRectilinearGrid
//    (our versions are much more memory friendly).  Note: since 
//    vtkVisItCellDataToPointData is in this module, this isn't really a
//    "graphics" factory any more.  Maybe it should be renamed.  I decided
//    to pile on instead.
//
//    Dave Bremer, Thu Jun 14 16:47:37 PDT 2007
//    Disable my previous change.  It causes a problem I don't understand
//    yet, seemingly related to reading a vtkRectilinearGrid or vtkStructuredGrid
//    from a file.
//
//    Dave Bremer, Mon Jun 18 17:44:43 PDT 2007
//    Reinstantiated use of vtkVisItStructuredGrid and vtkVisItRectilinearGrid.
//    
vtkVisItGraphicsFactory::vtkVisItGraphicsFactory()
{
  this->RegisterOverride("vtkOpenGLPolyDataMapper", "vtkVisItOpenGLPolyDataMapper",
                         "vtkVisItOpenGLPolyDataMapper override vtkOpenGLPolyDataMapper",
                         1,
                         vtkObjectFactoryCreatevtkVisItOpenGLPolyDataMapper);
  this->RegisterOverride("vtkMesaPolyDataMapper", "vtkVisItMesaPolyDataMapper",
                         "vtkVisItMesaPolyDataMapper override vtkMesaPolyDataMapper",
                         1,
                         vtkObjectFactoryCreatevtkVisItMesaPolyDataMapper);
  this->RegisterOverride("vtkCellDataToPointData", "vtkVisItCellDataToPointData",
                         "vtkVisItCellDataToPointData override vtkCellDataToPointData",
                         1,
                         vtkObjectFactoryCreatevtkVisItCellDataToPointData);
  this->RegisterOverride("vtkDataSetMapper", "vtkVisItDataSetMapper",
                         "vtkVisItDataSetMapper override vtkDataSetMapper",
                         1,
                         vtkObjectFactoryCreatevtkVisItDataSetMapper);
  this->RegisterOverride("vtkRectilinearGrid", "vtkVisItRectilinearGrid",
                         "vtkVisItRectilinearGrid override vtkRectilinearGrid",
                         1,
                         vtkObjectFactoryCreatevtkVisItRectilinearGrid);
  this->RegisterOverride("vtkStructuredGrid", "vtkVisItStructuredGrid",
                         "vtkVisItStructuredGrid override vtkStructuredGrid",
                         1,
                         vtkObjectFactoryCreatevtkVisItStructuredGrid);
#if defined(__APPLE__)
  this->RegisterOverride("vtkCarbonRenderWindow", "vtkOSMesaRenderWindow",
                         "vtkOSMesaRenderWindow override vtkCarbonRenderWindow",
                         1,
                         vtkObjectFactoryCreatevtkOSMesaRenderWindow);
#endif
}

// ****************************************************************************
//  Method: InitVTK::Initialize
//
//  Purpose:
//      Initialize the VTK portion of the program.  Separated from Init so
//      there would not be a VTK dependence on the GUI.  The only 
//      initialization that happens currently is the vtk print statements are
//      re-routed to the debug stream.
//
//  Programmer: Hank Childs
//  Creation:   April 24, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Apr 10 18:27:54 PDT 2003   
//    Register the factory that will allow VisIt to override vtkObjects.
//
//    Kathleen Bonnell, Wed Nov 12 16:51:56 PST 2003 
//    Comment out the VisItFactory until it is actually needed again.
//
//    Hank Childs, Thu Jan 22 17:31:23 PST 2004
//    Use the InitVTKNoGraphics Initialize routine to minimize duplication.
//
//    Hank Childs, Wed May  5 10:15:48 PDT 2004
//    Use the VisIt graphics factory to override the standard polydata mapper.
//
// ****************************************************************************

void
InitVTK::Initialize(void)
{
    InitVTKNoGraphics::Initialize();

    // Register the factory that allows VisIt objects to override vtk objects.
    vtkVisItGraphicsFactory *factory = vtkVisItGraphicsFactory::New();
    vtkObjectFactory::RegisterFactory(factory);
    factory->Delete();
}


// ****************************************************************************
//  Method: InitVTK::Force/Unforce Mesa
//
//  Purpose:
//      Forces all VTK rendering objects to be instantiated as Mesa object.
//
//  Programmer: Hank Childs
//  Creation:   January 28, 2002
//
//  Modifications:
//    Kathleen Bonnell, Wed Feb  5 09:09:08 PST 2003
//    Use vtkGrahpicsFactory and vtkImagingFactory static variables to
//    handle Mesa objects.
//
// ****************************************************************************

void
InitVTK::ForceMesa(void)
{
#if !defined(_WIN32)
    vtkGraphicsFactory::SetUseMesaClasses(1);
    vtkImagingFactory::SetUseMesaClasses(1);
#endif
}


void
InitVTK::UnforceMesa(void)
{
#if !defined(_WIN32)
    vtkGraphicsFactory::SetUseMesaClasses(0);
    vtkImagingFactory::SetUseMesaClasses(0);
#endif
}


