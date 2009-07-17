/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                      avtOpenGLTuvokVolumeRenderer.C                       //
// ************************************************************************* //
#include <visit-config.h>

#ifdef USE_TUVOK

#include "avtOpenGLTuvokVolumeRenderer.h"

#include <GL/glew.h>
#include <vtkCamera.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>
#include <vtkMatrix4x4.h>
#include <vtkRectilinearGrid.h>

#include <tuvok/../VisItDebugOut.h>
#include <tuvok/Controller/Controller.h>
#include <tuvok/IO/CoreVolume.h>
#include <tuvok/IO/CoreVolumeInfo.h>
#include <tuvok/Renderer/GL/ImmediateGLSBVR.h>

#include <avtViewInfo.h>
#include <Environment.h>
#include <InstallationFunctions.h>
#include <ImproperUseException.h>
#include <VolumeAttributes.h>
#include <DebugStream.h>

// Don't warn if a function is unused.  Useful for keeping a function static
// even if it's only used while debugging.
#if defined(__GNUC__) && __GNUC__ >= 4
    // GNUC >= 3 is probably safe, but I can't test easily.
#   define FQN_UNUSED __attribute__((unused))
#else
#   define FQN_UNUSED /* nothing */
#endif

static AbstrRenderer* create_renderer(const VolumeAttributes &);
static VolumeDatasetInfo *create_dataset_info(vtkRectilinearGrid *);

FQN_UNUSED static void debug_vtk_array(vtkDataArray *);
FQN_UNUSED static void debug_transfer_function(const std::vector<unsigned char>& rgba);
FQN_UNUSED static void debug_view(const avtViewInfo &);
static void initialize_glew();
static void tuvok_set_data(AbstrRenderer *, vtkRectilinearGrid *,
                           vtkDataArray *, float *, size_t);
static void tuvok_set_transfer_fqn(AbstrRenderer &, const VolumeAttributes &);
static void tuvok_set_view(AbstrRenderer &, const avtViewInfo &);
static std::vector<unsigned char> float_to_8bit(float *, size_t);

static bool glew_initialized = false;

// ****************************************************************************
//  Method: avtOpenGLTuvokVolumeRenderer::avtOpenGLTuvokVolumeRenderer
//
//  Purpose:
//
//  Programmer:  Josh Stratton
//  Creation:    Wed Dec 17 15:00:34 MST 2008
//
//  Modifications:
//
//    Tom Fogal, Thu Mar  5 14:31:42 MST 2009
//    Connect the appropriate type of debug output.
//    NULL out our renderer, until we know what kind of one to make.
//
// ****************************************************************************

avtOpenGLTuvokVolumeRenderer::avtOpenGLTuvokVolumeRenderer()
{
    Controller::Instance().AddDebugOut(new VisItDebugOut());
    // enable tuvok logging output -- very slow, do not leave enabled!
    Controller::Debug::Out().SetOutput(true, true, true, true);
    this->renderer = NULL;
}

// ****************************************************************************
//  Method: avtOpenGLTuvokVolumeRenderer::~avtOpenGLTuvokVolumeRenderer
//
//  Purpose:
//
//  Programmer:  Josh Stratton
//  Creation:    Wed Dec 17 15:00:34 MST 2008
//
//  Tom Fogal, Thu Mar  5 14:35:43 MST 2009
//  Add renderer instance.
//  Tidy up after our renderer.
//
// ****************************************************************************
avtOpenGLTuvokVolumeRenderer::~avtOpenGLTuvokVolumeRenderer()
{
    if(this->renderer)
    {
        debug1 << "Cleaning up renderer.." << std::endl;
        this->renderer->Cleanup();
        Controller::Instance().ReleaseVolumerenderer(this->renderer);
        this->renderer = NULL;
    }
}

// ****************************************************************************
//  Method:  avtOpenGLTuvokVolumeRenderer::Render
//
//  Purpose: Renders the dataset using Tuvok.
//
//  Arguments:
//    grid      : the data set to render
//    data,opac : the color/opacity variables
//    view      : the viewing information
//    atts      : the current volume plot attributes
//    vmin/max/size : the min/max/range of the color variable
//    omin/max/size : the min/max/range of the opacity variable
//    gx/gy/gz      : the gradient of the opacity variable
//    gmn           : the gradient magnitude, normalized to the max grad mag
//
//  Programmer:  Josh Stratton
//  Creation:    Wed Dec 17 15:00:34 MST 2008
//
//  Modifications:
//
//   Tom Fogal, Thu Mar  5 15:57:43 MST 2009
//   Create the underlying renderer.
//
//   Tom Fogal, Thu Mar 19 00:14:16 MST 2009
//   First pass at an implementation; camera settings are a bit off right now.
//
// ****************************************************************************

#if 0
// This is Brad's render code, which sets up the renderer much as it gets set
// up in ImageVis3D, at least for now. This was the only way I could get it
// to work with a version of Tuvok on my Mac. I didn't see obvious *explicit*
// ways to set the data into the renderer so I didn't try doing that. ImageVis3D
// does everything through calls to LoadDataset.
void
avtOpenGLTuvokVolumeRenderer::Render(
    const avtVolumeRendererImplementation::RenderProperties &props,
    const avtVolumeRendererImplementation::VolumeData &volume)
{
    initialize_glew();

    if(NULL == this->renderer)
    {
        bool bUseOnlyPowerOfTwo = true, bDownSampleTo8Bits = false, bDisableBorder = false, simple=false;
        MasterController &masterController = Controller::Instance();
        this->renderer = masterController.RequestNewVolumerenderer(MasterController::OPENGL_SBVR,
            bUseOnlyPowerOfTwo, bDownSampleTo8Bits, bDisableBorder, simple);

        std::string shaderDir;
        if(Environment::exists("TUVOK_SHADER_DIR"))
            shaderDir = Environment::get("TUVOK_SHADER_DIR");
        else
        {
#ifdef WIN32
            shaderDir = GetVisItArchitectureDirectory() + "/shaders";
#else
            shaderDir = GetVisItArchitectureDirectory() + "/bin/shaders";
#endif
        }

        this->renderer->AddShaderPath(shaderDir);
        this->renderer->LoadDataset("/Users/whitlock2/data/SCIData/engine.uvf");
        this->renderer->ScheduleCompleteRedraw();
        this->renderer->SetBlendPrecision(AbstrRenderer::BP_8BIT);

        this->renderer->Initialize();

        this->renderer->SetGlobalBBox(true);
        this->renderer->SetLocalBBox(true);
        this->renderer->SetRenderCoordArrows(true);
    }

    //  Resize the renderer's buffer so it matches the window size.
    this->renderer->Resize(UINTVECTOR2(props.windowSize[0],props.windowSize[1]));

    // Set the background color
    FLOATVECTOR3 bg[2];
    bg[0] = FLOATVECTOR3(
        props.backgroundColor[0],
        props.backgroundColor[1],
        props.backgroundColor[2]);
    bg[1] = FLOATVECTOR3(
        props.backgroundColor[0],
        props.backgroundColor[1],
        props.backgroundColor[2]);
    this->renderer->SetBackgroundColors(bg);

    GLfloat rmat[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, rmat);
    // Null out the translation components
    rmat[12] = 0.f;
    rmat[13] = 0.f;
    rmat[14] = 0.f;
    rmat[15] = 1.f;
    this->renderer->SetRotation(rmat);

    // Tell the renderer to draw the scene.
    this->renderer->ScheduleCompleteRedraw();
    this->renderer->Paint();
}
#else
// This is Tom's original Render code.
void
avtOpenGLTuvokVolumeRenderer::Render(
    const avtVolumeRendererImplementation::RenderProperties &props,
    const avtVolumeRendererImplementation::VolumeData &volume)
{
    initialize_glew();

    if(NULL == this->renderer)
    {
        this->renderer = create_renderer(props.atts);
    }
    debug_view(props.view);

    tuvok_set_data(this->renderer, volume.grid, volume.data.data, volume.gmn,
                   volume.data.data->GetNumberOfTuples());

    tuvok_set_transfer_fqn(*this->renderer, props.atts);

    tuvok_set_view(*this->renderer, props.view);

    this->renderer->Paint();
}
#endif

// ****************************************************************************
//  Function: create_renderer
//
//  Purpose: Gets the appropriate type of renderer considering the information
//           given in the VolumeAttributes.
//
//  Returns:    The requested renderer, or NULL if request is nonsensical.
//
//  Programmer: Tom Fogal
//  Creation:   Thu Mar  5 14:55:14 MST 2009
//
//  Modifications:
//
//    Tom Fogal, Thu Mar  5 20:15:10 MST 2009
//    Initialize the renderer before returning it.
//
//    Tom Fogal, Fri Mar  6 10:39:04 MST 2009
//    Set an empty `CoreVolume' to the dataset.
//
//    Tom Fogal, Wed Mar 18 22:29:17 MST 2009
//    Rename so statics follow a consistent + distinct convention.  Make a more
//    compatible renderer.
//
// ****************************************************************************
static AbstrRenderer *
create_renderer(const VolumeAttributes &)
{
    // Hack!  Eventually we'll need to add some state to the VolumeAttributes
    // to describe these.  For now, we'll just automatically select a renderer
    // which will be compatible on a wide variety of platforms.
    const bool use_only_PoT_textures = true;
    // switch downsample to true if OpenGL crashes for you.
    const bool downsample = false;
    const bool disable_border = false;
    const bool simple = true;

    MasterController &mc = Controller::Instance();
    AbstrRenderer *ren = mc.RequestNewVolumerenderer(
                            MasterController::OPENGL_SBVR,
                            use_only_PoT_textures, downsample,
                            disable_border, simple);

    // We need to know where Tuvok stores its shaders, since it must load them
    // at runtime.  They're in our source tree, but of course we can't even
    // assume that's present.  We'll need to coordinate with copying the
    // shaders around at install time, but for now we'll just use an
    // environment variable.
    if(!Environment::exists("TUVOK_SHADER_DIR"))
    {
        EXCEPTION1(ImproperUseException,
                   "Don't know where to find Tuvok Shaders!  Please set the "
                   "TUVOK_SHADER_DIR environment variable.");
    }
    ren->AddShaderPath(Environment::get("TUVOK_SHADER_DIR").c_str());
    ren->SetDataSet(new CoreVolume());
    ren->Initialize();
    // Tuvok needs to know how big to make its FBOs.  We'll need to modify
    // VisIt to somehow pass the information of the view window size down to
    // here, but we need something for now.  Make it huge to ensure our window
    // doesn't exceed the FBO size.
    ren->Resize(UINTVECTOR2(1200,1200));  // HACK!
    ren->SetRendermode(AbstrRenderer::RM_1DTRANS);
    ren->SetUseLighting(false);
    ren->SetBlendPrecision(AbstrRenderer::BP_8BIT);
    {
        FLOATVECTOR3 bg[2] = { FLOATVECTOR3(1,1,1),
                               FLOATVECTOR3(1,1,1) };
        ren->SetBackgroundColors(bg);
    }
    return ren;
}

// ****************************************************************************
//  Function: create_dataset_info
//
//  Purpose: Translates our/vtk's metadata into the object Tuvok wants it as.
//
//  Returns: An object which should be given to a VolumeDataset to replace its
//           existing metadata.
//
//  Programmer: Tom Fogal
//  Creation:   Fri Mar  6 11:23:52 MST 2009
//
//  Modifications:
//
//    Tom Fogal, Wed Mar 18 21:47:31 MST 2009
//    Rename so statics follow a consistent + distinct convention.
//
// ****************************************************************************
static VolumeDatasetInfo *
create_dataset_info(vtkRectilinearGrid *grid)
{
    CoreVolumeInfo *vds_info = new CoreVolumeInfo();
    {
        int dims[3];
        grid->GetDimensions(dims);
        vds_info->SetDomainSize(static_cast<UINT64>(dims[0]),
                                static_cast<UINT64>(dims[1]),
                                static_cast<UINT64>(dims[2]));
        debug1 << "dims: "
               << dims[0] << ", "
               << dims[1] << ", "
               << dims[2] << std::endl;
    }
    return vds_info;
}

// ****************************************************************************
//  Function: initialize_glew
//
//  Purpose: Does a one-time GLEW initialization.
//
//  Programmer: Tom Fogal
//  Creation:   Fri Mar  6 14:03:22 MST 2009
//
//  Modifications:
//
// ****************************************************************************
static void
initialize_glew()
{
    if(!glew_initialized) {
#if 1
        GLenum err = glewInit();
#else
        // Lets one use HW rendering in serial w/ -nowin.
        GLenum err = glewInitLibrary("/usr/lib/libGL.so",
                                     GLEW_NAME_CONVENTION_GL);
#endif
        if(GLEW_OK != err) {
            debug1 << "GLEW initialization failed: " << glewGetErrorString(err)
                   << std::endl;
        } else {
            glew_initialized = true;
        }
    }
}

/// prints VTK array information to the debug stream.
static void
debug_vtk_array(vtkDataArray *arr)
{
    vtkFloatArray *fA = vtkFloatArray::SafeDownCast(arr);
    double range[2];
    fA->GetRange(range);
    debug3 << "Array: " << arr->GetName() << std::endl
           << "\ttype: " << arr->GetDataType() << "("
              << vtkImageScalarTypeNameMacro(arr->GetDataType()) << ")"
              << std::endl
           << "\tcomponents: " << arr->GetNumberOfComponents() << std::endl
           << "\telement size: " << arr->GetDataTypeSize() << std::endl
           << "\ttuples: " << arr->GetNumberOfTuples() << std::endl
           << "\trange: [" << range[0] << ", " << range[1] << "]" << std::endl;
}

template<typename in, typename out>
static inline out
lerp(in value, in imin, in imax, out omin, out omax)
{
  out ret = omin + (value-imin) * (static_cast<double>(omax-omin) /
                                                      (imax-imin));
  return ret;
}

static void
debug_transfer_function(const std::vector<unsigned char>& rgba)
{
#define UC(x) static_cast<unsigned char>(x)
    for(size_t i=0; i < rgba.size(); i+=4)
    {
        debug5 << "tf(" << setw(3) << i/4 << "): "
               << setw(5) << setprecision(3) << setfill(' ')
               << setiosflags(std::ios_base::right)
               << lerp(rgba[i+0], UC(0),UC(255), 0.f,1.f) << ", "
               << lerp(rgba[i+1], UC(0),UC(255), 0.f,1.f) << ", "
               << lerp(rgba[i+2], UC(0),UC(255), 0.f,1.f) << ", "
               << lerp(rgba[i+3], UC(0),UC(255), 0.f,1.f) << std::endl;
    }
}

static void
dbg_4x4_matrix(const char *pfx, const GLfloat m[16])
{
    debug5 << pfx << ":" << std::endl
        << setw(3) << setprecision(3) << setfill(' ')
        << "[" << m[ 0] << "," << m[ 1] << "," << m[ 2] << "," << m[ 3] << "]\n"
        << "[" << m[ 4] << "," << m[ 5] << "," << m[ 6] << "," << m[ 7] << "]\n"
        << "[" << m[ 8] << "," << m[ 9] << "," << m[10] << "," << m[11] << "]\n"
        << "[" << m[12] << "," << m[13] << "," << m[14] << "," << m[15] << "]\n";
    // should restore the old stream state here ...
}

// Modifications:
//   Tom Fogal, Thu Apr 23 14:48:21 MDT 2009
//   Updated for recent change w.r.t. accessing debug streams.
static void
debug_view(const avtViewInfo &v)
{
    vtkCamera *cam = vtkCamera::New();
    v.SetCameraFromView(cam);

    cam->Print(DebugStream::Stream1());
    debug5 << "imagepan: [" << v.imagePan[0] << ", " << v.imagePan[1] << "]"
           << std::endl << "imagezoom: " << v.imageZoom << std::endl;

    { // Get current OpenGL matrices
        GLfloat matrix[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
        dbg_4x4_matrix("modelview", matrix);

        glGetFloatv(GL_PROJECTION_MATRIX, matrix);
        dbg_4x4_matrix("projection", matrix);
    }

    debug5 << "parallel scale: " << v.parallelScale << std::endl
           << "eye: [ "
           << v.camera[0] << ", " << v.camera[1] << ", " << v.camera[2]
           << " ]" << std::endl
           << "ref: [ "
           << v.focus[0] << ", " << v.focus[1] << ", " << v.focus[2]
           << " ]" << std::endl;
}

// ****************************************************************************
//  Function: tuvok_set_data
//
//  Purpose: Sets the dataset and gradient magnitude within tuvok.
//
//  Programmer: Tom Fogal
//  Creation:   Wed Mar 18 22:09:34 MST 2009
//
//  Modifications:
//
// ****************************************************************************
static void
tuvok_set_data(AbstrRenderer *ren, vtkRectilinearGrid *grid,
               vtkDataArray *data, float *gr_mag, size_t n_mag)
{
    // base class doesn't have these `Set' methods; we can only set this kind
    // of thing manually from the subclass we'll be using (for now).
    CoreVolume &vol = dynamic_cast<CoreVolume&>(ren->GetDataSet());

    vol.SetInfo(create_dataset_info(grid));
    vol.SetGradientMagnitude(gr_mag, n_mag);
    debug_vtk_array(data);

    // This will break if we're not given float data, but that's guaranteed
    // for now!
#if 0
    vol.SetData(static_cast<float*>(data->GetVoidPointer(0)),
                data->GetNumberOfTuples());
#else
    debug5 << "Converting to 32bit data to 8bit data." << std::endl;
    std::vector<unsigned char> eight_bit_data;
    eight_bit_data = float_to_8bit(
                         static_cast<float*>(data->GetVoidPointer(0)),
                         data->GetNumberOfTuples()
                     );
    vol.SetData(&eight_bit_data.at(0), data->GetNumberOfTuples());
#endif
}

// ****************************************************************************
//  Function: tuvok_set_transfer_fqn
//
//  Purpose: Uploads VisIt's TF into tuvok.
//
//  Programmer: Tom Fogal
//  Creation:   Wed Mar 18 22:09:21 MST 2009
//
//  Modifications:
//
// ****************************************************************************
static void
tuvok_set_transfer_fqn(AbstrRenderer &ren, const VolumeAttributes &atts)
{
    std::vector<unsigned char> rgba(256*4);
    atts.GetTransferFunction(&rgba.at(0));

    // TF seems to be working; don't spam the logs right now.
    //debug_transfer_function(rgba);

    TransferFunction1D *tf = ren.Get1DTrans();
    tf->Set(rgba);
    // Ensure tuvok knows that it must re-upload the TF to the GPU.
    ren.Changed1DTrans();
}

// ****************************************************************************
//  Function: tuvok_set_view
//
//  Purpose: Copies VisIt's view parameters into Tuvok.
//
//  Programmer: Tom Fogal
//  Creation:   Wed Mar 18 22:22:15 MST 2009
//
//  Modifications:
//
// ****************************************************************************
static void
tuvok_set_view(AbstrRenderer &ren, const avtViewInfo &v)
{
    float eye[3];
    float ref[3];
    float vup[3];
    eye[0] = v.camera[0]; eye[1] = v.camera[1]; eye[2] = v.camera[2];
    ref[0] =  v.focus[0]; ref[1] =  v.focus[1]; ref[2] =  v.focus[2];
    vup[0] = v.viewUp[0]; vup[1] = v.viewUp[1]; vup[2] = v.viewUp[2];

    // There's a `CameraModel' page on the wiki which explains this in detail.
    FLOATVECTOR3 Eye(eye);
    FLOATVECTOR3 Ref(ref);
    FLOATVECTOR3 viewNormal(Ref - Eye);
    viewNormal.normalize();
    // Unfortunately the `Eye' calculated here is equivalent to what is stored
    // in v.camera.  Neither is what Tuvok expects as reasonable eye points.
    Eye = viewNormal * ((v.parallelScale) / tan(v.viewAngle/360)) + Ref;
    eye[0] = Eye[0];
    eye[1] = Eye[1];
    eye[2] = Eye[2];

    // This API is going to change, as you might guess from the code.  But the
    // information given will remain the same; only how we grab the object &&
    // the function name will differ.
    // The arguments this API expects are:
    //      field of view
    //      near plane Z value
    //      far plane Z value
    //      eye (camera) location in world coordinates
    //      reference point in world coordinates
    //      view up vector
    // The simplest way to think of this data is `the stuff one would pass to
    // "gluLookAt" and "gluPerspective"' in any other app.  VisIt's notion of a
    // camera seems to be a bit different ...
    ImmediateGLSBVR& glren =
        dynamic_cast<ImmediateGLSBVR&>(ren);
    glren.Hack(
        static_cast<float>(v.viewAngle),
        static_cast<float>(v.nearPlane),
        static_cast<float>(v.farPlane),
        eye, ref, vup
    );
}

// ****************************************************************************
//  Function: float_to_8bit
//
//  Purpose: Temporary hack to convert an FP dataset to an 8bit dataset.
//           Does so in an absolutely terrible way.
//
//  Programmer: Tom Fogal
//  Creation:   Wed Mar 25 12:33:53 MST 2009
//
//  Modifications:
//
// ****************************************************************************
struct lerpf_8 : std::unary_function<float, unsigned char> {
  unsigned char operator()(float f) const {
    return lerp(f, std::numeric_limits<float>::min(),
                   std::numeric_limits<float>::max(),
                   std::numeric_limits<unsigned char>::min(),
                   std::numeric_limits<unsigned char>::max());
  }
};

static std::vector<unsigned char>
float_to_8bit(float *data, size_t v)
{
  std::vector<unsigned char> ret(v);
  std::transform(data, data + v, ret.begin(), lerpf_8());
  return ret;
}

#endif // USE_TUVOK
