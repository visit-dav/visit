#define vtkRenderingCore_AUTOINIT 1(vtkRenderingOpenGL)
#include <vtkSetGet.h>
#include <vtkRenderWindow.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkgl.h> 
#if defined(TEST_MPI_CAPS)
# include <mpi.h>
#endif
#if defined(TEST_PY_CAPS)
# include <patchlevel.h>
#endif
#if defined(TEST_MESA_CAPS)
//# include "vtkOSMesaRenderWindow.h"
#include <GL/osmesa.h>
#include <GL/gl.h>
#endif
#include <vtksys/SystemInformation.hxx>
#include <string>
#include <sstream>

using std::string;
using std::ostringstream;

// never use NULL pointer with stl
#define safes(arg) (arg?arg:"NULL")

// Description:
// Get python version
#if defined(TEST_PY_CAPS)
string GetPythonVersion()
{
    ostringstream oss;
#if defined(PY_VERSION)
    oss << PY_VERSION;
#else
    oss << "unknown";
#endif
    return oss.str();
}
#endif

// Description:
// Get the version of the standard implemented by this
// MPI
#if defined(TEST_MPI_CAPS)
string GetMPIVersion()
{
    ostringstream oss;
    int major=-1, minor=-1;
#if defined(MPI_VERSION)
    major = MPI_VERSION;
#endif
#if defined(MPI_SUBVERSION)
    minor = MPI_SUBVERSION;
#endif
  //MPI_Get_version(&major, &minor);
    oss << major << "." << minor;
    return oss.str();
}
#endif

// Description:
// Get the implementor name and release info
#if defined(TEST_MPI_CAPS)
string GetMPILibraryVersion()
{
    ostringstream oss;
#if defined(MPI_VERSION) && (MPI_VERSION >= 3)
    char libVer[MPI_MAX_LIBRARY_VERSION_STRING] = {'\0'};
    int libVerLen = MPI_MAX_LIBRARY_VERSION_STRING;
    MPI_Get_library_version(libVer, &libVerLen);
    libVer[libVerLen] = '\0';
    oss << libVer;
#else
      // Open MPI
#if defined(OPEN_MPI)
    oss << "Open MPI";
#if defined(OMPI_MAJOR_VERSION)
    oss << " " << OMPI_MAJOR_VERSION;
#endif
#if defined(OMPI_MINOR_VERSION)
    oss << "." << OMPI_MINOR_VERSION;
#endif
#if defined(OMPI_RELEASE_VERSION)
    oss << "." << OMPI_RELEASE_VERSION;
#endif
  // MPICH
#elif defined(MPICH2)
    oss << "MPICH2";
#if defined(MPICH2_VERSION)
    oss << " " << MPICH2_VERSION;
#endif
#elif defined(MSMPI_VER)
    oss << "Microsoft MPI " << MSMPI_VER;
#else
    oss << "unknown";
#endif
#endif
    return oss.str();
}
#endif

#if defined(TEST_MESA_CAPS)
// Description:
// Gather info about Mesa OpenGL
void GetMesaOpenGLInfo(
      string &renWinName,
      string &oglVersion,
      string &oglVendor,
      string &oglRenderer,
      string &oglExtensions)
{
    /*
    vtkOSMesaRenderWindow *rwin = vvtkOSMesaRenderWindow::New();
    renWinName = rwin->GetClassName();
    rwin->Render();*/
    unsigned char colorBuffer[4] = {'\0'};
    OSMesaContext context
        = OSMesaCreateContext(OSMESA_RGBA, NULL);
    if (!(context
       && OSMesaMakeCurrent(
          context,
          colorBuffer,
          GL_UNSIGNED_BYTE,
          1,
          1)))
    {
      return;
    }

    oglVersion
      = safes(reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    oglVendor
       = safes(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
    oglRenderer
       = safes(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    oglExtensions
       = safes(reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)));

    OSMesaDestroyContext(context);
    //rwin->Delete();
}
#else
  // Description:
  // Gather info from VTK about which OpenGL
  // it will use be defualt
void GetVTKOpenGLInfo(
      string &renWinName,
      string &oglVersion,
      string &oglVendor,
      string &oglRenderer,
      string &oglExtensions)
{
    vtkRenderWindow *rwin = vtkRenderWindow::New();
    renWinName = rwin->GetClassName();
    rwin->Render();

    oglVersion
        = safes(reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    oglVendor
         = safes(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
    oglRenderer
         = safes(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    oglExtensions
         = safes(reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)));

    rwin->Delete();
}
#endif

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
#if defined(TEST_MESA_CAPS)
    string mRenWinName,
        mOglVersion,
        mOglVendor,
        mOglRenderer,
        mOglExtensions;
    GetMesaOpenGLInfo(
        mRenWinName,
        mOglVersion,
        mOglVendor,
        mOglRenderer,
        mOglExtensions);
#else
    string vRenWinName,
        vOglVersion,
        vOglVendor,
        vOglRenderer,
        vOglExtensions;
    GetVTKOpenGLInfo(
        vRenWinName,
        vOglVersion,
        vOglVendor,
        vOglRenderer,
        vOglExtensions);
#endif

    // for info about the host
    vtksys::SystemInformation sysinfo;
    sysinfo.RunCPUCheck();
    sysinfo.RunOSCheck();

    // make the report
    cout << "CTEST_FULL_OUTPUT (Avoid ctest truncation of output)" << endl
      << endl
      << "Host System:" << endl
      << "OS = " << sysinfo.GetOSDescription() << endl
      << "CPU = " << sysinfo.GetCPUDescription() << endl
      << "RAM = " << sysinfo.GetMemoryDescription() << endl
      << endl
#if defined(TEST_MPI_CAPS)
      << "MPI:" << endl
      << "Version = " << GetMPIVersion() << endl
      << "Library = " << GetMPILibraryVersion() << endl
      << endl
#endif
#if defined(TEST_PY_CAPS)
      << "Python:" << endl
      << "Version = " << GetPythonVersion() << endl
      << endl
#endif
#if defined(TEST_MESA_CAPS)
      << "Mesa OpenGL:" << endl
      << "RenderWindow = " << mRenWinName << endl
      << "DriverGLVendor = " << mOglVersion << endl
      << "DriverGLVersion = " << mOglVendor << endl
      << "DriverGLRenderer = " << mOglRenderer << endl
      << "Extensions = " << mOglExtensions << endl
#else
      << "VTK OpenGL:" << endl
      << "RenderWindow = " << vRenWinName << endl
      << "DriverGLVendor = " << vOglVersion << endl
      << "DriverGLVersion = " << vOglVendor << endl
      << "DriverGLRenderer = " << vOglRenderer << endl
      << "Extensions = " << vOglExtensions << endl
#endif
      << endl;

    // always pass
    return 0;
}
