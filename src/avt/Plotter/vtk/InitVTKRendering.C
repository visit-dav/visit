/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

#include <vtkVisItDataSetMapper.h>
#include <vtkToolkits.h>
#include <vtkVisItRectilinearGrid.h>
#include <vtkVisItStructuredGrid.h>

#include <vtkObjectFactory.h>
#include <vtkVersion.h>
#if !defined(_WIN32)
#include <vtkGraphicsFactory.h>
//FIX_ME_VTK6.0
//#include <vtkImagingFactory.h>
#endif


#include <vtk/InitVTKRenderingConfig.h>

#ifdef VISIT_MANTA
#include <vtkMantaObjectFactory.h>
#endif

#include <avtCallback.h>

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
VTK_CREATE_CREATE_FUNCTION(vtkVisItDataSetMapper);
VTK_CREATE_CREATE_FUNCTION(vtkVisItRectilinearGrid);
VTK_CREATE_CREATE_FUNCTION(vtkVisItStructuredGrid);

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
//    Brad Whitlock, Thurs Nov 15 16:25 PST 2007
//    Added override of vtkOpenGLTexture that is compatible with our vtkQtRenderWindow.
//
//    Tom Fogal, Tue Apr 27 11:42:46 MDT 2010
//    Fix Mesa special case: our OGL classes can handle that.
//
//    Brad Whitlock, Wed Aug 10 11:52:31 PDT 2011
//    Use VTK's new polydata mapper by eliminating our override of vtkOpenGLPolyDataMapper.
//
vtkVisItGraphicsFactory::vtkVisItGraphicsFactory()
{
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
    // Register the factory that allows VisIt objects to override vtk objects.
    vtkVisItGraphicsFactory *factory = vtkVisItGraphicsFactory::New();
    vtkObjectFactory::RegisterFactory(factory);
    factory->Delete();

#ifdef VISIT_MANTA
    if (avtCallback::UseManta())
    {
      printf("Initializing Manta\n");
      //register manta overrides.  This factory overrides actors, lights and other state to pass of polygonal data to manta.
      vtkMantaObjectFactory* mfactory = vtkMantaObjectFactory::New();
      vtkObjectFactory::RegisterFactory(mfactory);
      mfactory->Delete();
    }
#endif
}
