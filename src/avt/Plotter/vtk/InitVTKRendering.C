/*****************************************************************************
*
* Copyright (c) 2000 - 2018, Lawrence Livermore National Security, LLC
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

#include <InitVTKRendering.h>

#include <vtkToolkits.h>
#include <vtkVisItRectilinearGrid.h>
#include <vtkVisItStructuredGrid.h>
#include <vtkOpenGLPointMapper.h>

#include <vtkObjectFactory.h>
#include <vtkVersion.h>


#include <vtk/InitVTKRenderingConfig.h>

#ifdef VISIT_OSPRAY
#include <vtkOSPRayObjectFactory.h>
#endif

#include <avtCallback.h>
#include <DebugStream.h>

//
// The following calls initialize VTK modules.  They are built-in factories
// similar to what is done below, but are standard VTK, providing a lot of
// OpenGL overrides for rendering classes.
//
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkInteractionStyle)
VTK_MODULE_INIT(vtkRenderingFreeType)
VTK_MODULE_INIT(vtkRenderingOpenGL2)

//
// A factory that will allow VisIt to override any vtkObject
// with a sub-class of that object.
//
class vtkVisItGraphicsFactory : public vtkObjectFactory
{
  public:
    static vtkVisItGraphicsFactory * New();
    vtkTypeMacro(vtkVisItGraphicsFactory, vtkObjectFactory)

    const char* GetDescription() override { return "vtkVisItGraphicsFactory"; };
    const char* GetVTKSourceVersion() override;

  protected:
    vtkVisItGraphicsFactory();

  private:
    vtkVisItGraphicsFactory(const vtkVisItGraphicsFactory&);
    void operator=(const vtkVisItGraphicsFactory&);
};

vtkStandardNewMacro(vtkVisItGraphicsFactory)

//
// Necessary for each object that will override a vtkObject.
//
VTK_CREATE_CREATE_FUNCTION(vtkVisItRectilinearGrid);
VTK_CREATE_CREATE_FUNCTION(vtkVisItStructuredGrid);
VTK_CREATE_CREATE_FUNCTION(vtkOpenGLPointMapper);

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
//    Disable my previous change.  It causes a problem I don't understand yet,
//    seemingly related to reading a vtkRectilinearGrid or vtkStructuredGrid
//    from a file.
//
//    Dave Bremer, Mon Jun 18 17:44:43 PDT 2007
//    Reinstantiated use of vtkVisItStructuredGrid and vtkVisItRectilinearGrid.
//
//    Brad Whitlock, Thurs Nov 15 16:25 PST 2007
//    Added override of vtkOpenGLTexture that is compatible with our
//    vtkQtRenderWindow.
//
//    Tom Fogal, Tue Apr 27 11:42:46 MDT 2010
//    Fix Mesa special case: our OGL classes can handle that.
//
//    Brad Whitlock, Wed Aug 10 11:52:31 PDT 2011
//    Use VTK's new polydata mapper by eliminating our override of
//    vtkOpenGLPolyDataMapper.
//
//    Kathleen Biagas, Mon Jul 18 10:02:30 PDT 2016
//    Remove vtkVisItDataSetMapper
//
//    Kathleen Biagas, Wed Aug 17 09:27:12 PDT 2016
//    Add vtkOpenGLPointMapper.
//

vtkVisItGraphicsFactory::vtkVisItGraphicsFactory()
{
  this->RegisterOverride("vtkRectilinearGrid", "vtkVisItRectilinearGrid",
                         "vtkVisItRectilinearGrid override vtkRectilinearGrid",
                         1,
                         vtkObjectFactoryCreatevtkVisItRectilinearGrid);
  this->RegisterOverride("vtkStructuredGrid", "vtkVisItStructuredGrid",
                         "vtkVisItStructuredGrid override vtkStructuredGrid",
                         1,
                         vtkObjectFactoryCreatevtkVisItStructuredGrid);
  this->RegisterOverride("vtkPointMapper",
    "vtkOpenGLPointMapper",
    "vtkOpenGLPointMapper override vtkPointMapper",
    1,
    vtkObjectFactoryCreatevtkOpenGLPointMapper);
}

// ****************************************************************************
//  Method: InitVTKRendering::Initialize
//
//  Purpose:
//      Initialize the VTK portion of the program.  Separated from Init so
//      there would not be a VTK dependence on the GUI.
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
//    Carson Brownlee, Sun May  6 16:25:28 PDT 2012
//    Add support for Manta.
//
// ****************************************************************************

void
InitVTKRendering::Initialize(void)
{
#ifdef VISIT_OSPRAY
    if (avtCallback::UseOSPRay())
    {
      debug1 << "InitVTKRendering::Initializing turning on OSPRay" << endl;
      vtkOSPRayObjectFactory* ofactory = vtkOSPRayObjectFactory::New();
      vtkObjectFactory::RegisterFactory(ofactory);
      ofactory->Delete();
    }
    else
    {
      debug1 << "InitVTKRendering::Initializing not turning on OSPRay" << endl;
    }
#endif

    // Register the factory that allows VisIt objects to override vtk objects.
    vtkVisItGraphicsFactory *factory = vtkVisItGraphicsFactory::New();
    vtkObjectFactory::RegisterFactory(factory);
    factory->Delete();
}
