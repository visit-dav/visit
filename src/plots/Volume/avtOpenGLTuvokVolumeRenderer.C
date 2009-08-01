/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
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

#include <vtkCamera.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>
#include <vtkMatrix4x4.h>
#include <vtkRectilinearGrid.h>

#include <tuvok/../VisItDebugOut.h>
#include <tuvok/Controller/Controller.h>
#include <tuvok/IO/UnbrickedDataset.h>
#include <tuvok/IO/UnbrickedDSMetadata.h>
#include <tuvok/Renderer/AbstrRenderer.h>
#include <tuvok/Renderer/GL/GLFrameCapture.h>

#include <avtCallback.h>
#include <avtGLEWInitializer.h>
#include <avtParallel.h>
#include <avtViewInfo.h>
#include <DebugStream.h>
#include <FileFunctions.h>
#include <ImproperUseException.h>
#include <InstallationFunctions.h>
#include <RuntimeSetting.h>
#include <VolumeAttributes.h>

// Don't warn if a function is unused.  Useful for keeping a function static
// even if it's only used while debugging.
#if defined(__GNUC__) && __GNUC__ >= 4
    // GNUC >= 3 is probably safe, but I can't test easily.
#   define FQN_UNUSED __attribute__((unused))
#else
#   define FQN_UNUSED /* nothing */
#endif

static AbstrRenderer* create_renderer(const VolumeAttributes &);
static tuvok::UnbrickedDSMetadata *create_dataset_info(vtkRectilinearGrid *);

FQN_UNUSED static void debug_vtk_array(vtkDataArray *);
FQN_UNUSED static void debug_view(const avtViewInfo &);
static void tuvok_set_data(AbstrRenderer *, vtkRectilinearGrid *,
                           vtkDataArray *, float *, size_t);
static void tuvok_set_transfer_fqn(AbstrRenderer &, const VolumeAttributes &);
static void tuvok_set_view(AbstrRenderer &, const avtViewInfo &);

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
//    Tom Fogal, Tue Jul  7 12:01:34 MDT 2009
//    Use initializer list.
//
// ****************************************************************************

avtOpenGLTuvokVolumeRenderer::avtOpenGLTuvokVolumeRenderer()
  : renderer(NULL)
{
    Controller::Instance().AddDebugOut(new VisItDebugOut());
    // enable tuvok logging output -- very slow, do not leave enabled!
    Controller::Debug::Out().SetOutput(true, true, true, true);
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
//   Tom Fogal, Sun Jul 26 15:22:47 MDT 2009
//   Second pass; take most of Brad's code and work it into the
//   UnbrickedDataset methodology of setting data.
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
    avt::glew::initialize();

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
    avt::glew::initialize();

    // Bit of a hack.  Tuvok changes some sort of texture state, which is doing
    // Bad Things (tm) to the rest of VisIt's rendering code.  We'll need to
    // track down and fix Tuvok at some point, but in the meantime pushing and
    // popping our texture state will workaround the issue.
    glPushAttrib(GL_TEXTURE_BIT);

    if(NULL == this->renderer)
    {
        this->renderer = create_renderer(props.atts);
    }
    debug_view(props.view);

    tuvok_set_data(this->renderer, volume.grid, volume.data.data, volume.gmn,
                   volume.data.data->GetNumberOfTuples());

    tuvok_set_transfer_fqn(*this->renderer, props.atts);

    tuvok_set_view(*this->renderer, props.view);

    this->renderer->SetGlobalBBox(true);
    this->renderer->SetLocalBBox(true);
    this->renderer->SetRenderCoordArrows(true);

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

#if 0
    // Works well in the UI, but messes up testing via the CLI.  Hrm.
    GLfloat rmat[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, rmat);
    // Null out the translation components
    rmat[12] = 0.f;
    rmat[13] = 0.f;
    rmat[14] = 0.f;
    rmat[15] = 1.f;
    this->renderer->SetRotation(rmat);
#endif

    this->renderer->Paint();

    glPopAttrib(); // fix texture state; see comment above PushAttrib.
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
//    Tom Fogal, Fri May  1 18:11:20 MDT 2009
//    Updated for revised Tuvok API.  Use RuntimeSettings to lookup shader dir.
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
    const bool no_clip_planes = false; // mac long shader workaround.
    // Don't assume bit width relations between dataset and TFqns.
    const bool bias_tfqn_scaling = true;

    MasterController &mc = Controller::Instance();
    AbstrRenderer *ren = mc.RequestNewVolumeRenderer(
                            MasterController::OPENGL_SBVR,
                            use_only_PoT_textures, downsample,
                            disable_border, no_clip_planes, bias_tfqn_scaling
                         );
    // We need to know where Tuvok stores its shaders, since it must load them
    // at runtime.  They should be placed relative to the VisIt binary, but use
    // a RuntimeSetting to allow overrides.
    const std::string shader_dir = RuntimeSetting::lookups("tuvok-shader-dir");
    debug5 << "Adding shader path: " << shader_dir << std::endl;
    { // Make sure the shader path makes sense.
      VisItStat_t statbuf; // ignored, just want the return val.
      if(VisItStat(shader_dir.c_str(), &statbuf) != 0)
      {
        std::ostringstream dir_error;
        dir_error << "Tuvok cannot find its shaders in '" << shader_dir << "'"
                  << "!  Try using the --tuvoks-shaders command line option, "
                  << "or setting the VISIT_TUVOK_SHADER_DIR environment "
                  << "variable.";
        EXCEPTION1(ImproperUseException, dir_error.str().c_str());
      }
    }
    ren->AddShaderPath(shader_dir.c_str());
    ren->SetDataset(new tuvok::UnbrickedDataset());
    ren->SetGlobalBBox(true);
    // Tuvok needs to know how big to make its FBOs.  We'll resize it when we
    // actually render, but make sure we have something for now.
    ren->Resize(UINTVECTOR2(300,300));
    ren->Initialize();
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
//  Returns: An object representing metadata as tuvok understands it.
//
//  Programmer: Tom Fogal
//  Creation:   Fri Mar  6 11:23:52 MST 2009
//
//  Modifications:
//
//    Tom Fogal, Wed Mar 18 21:47:31 MST 2009
//    Rename so statics follow a consistent + distinct convention.
//
//    Tom Fogal, Fri May  1 20:23:13 MDT 2009
//    Updated for revised Tuvok API.
//
// ****************************************************************************
static tuvok::UnbrickedDSMetadata *
create_dataset_info(vtkRectilinearGrid *grid)
{
    tuvok::UnbrickedDSMetadata *vds_info = new tuvok::UnbrickedDSMetadata();
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
//    Tom Fogal, Fri May  1 20:25:02 MDT 2009
//    Updated for revised Tuvok API.  Get rid of 8bit case, no longer relevant.
//
// ****************************************************************************
static void
tuvok_set_data(AbstrRenderer *ren, vtkRectilinearGrid *grid,
               vtkDataArray *data, float *gr_mag, size_t n_mag)
{
    // base class doesn't have these `Set' methods; we can only set this kind
    // of thing manually from the subclass we'll be using.
    tuvok::UnbrickedDataset &vol = dynamic_cast<tuvok::UnbrickedDataset&>
                                               (ren->GetDataset());
    vol.SetMetadata(create_dataset_info(grid));
    vol.SetGradientMagnitude(gr_mag, n_mag);
    debug_vtk_array(data);

    // This will break if we're not given float data, but that's guaranteed
    // for now!
    // We could also send down unsigned char data at this point.  Anything else
    // would require some simple but non-zero amount of work inside Tuvok.
    vol.SetData(static_cast<float*>(data->GetVoidPointer(0)),
                data->GetNumberOfTuples());
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
//    Tom Fogal, Sun Jul 26 15:30:33 MDT 2009
//    New, simpler API.  Disable (very) verbose debugging.
//
// ****************************************************************************
static void
tuvok_set_transfer_fqn(AbstrRenderer &ren, const VolumeAttributes &atts)
{
    std::vector<unsigned char> rgba(256*4);
    atts.GetTransferFunction(&rgba.at(0));

    ren.Set1DTrans(rgba);
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

    // The arguments this API expects are:
    //      field of view
    //      near plane Z value
    //      far plane Z value
    //      eye (camera) location in world coordinates
    //      reference point in world coordinates
    //      view up vector
    // The simplest way to think of this data is `the stuff one would pass to
    // "gluLookAt" and "gluPerspective"' in any other app.
    ren.SetViewParameters(
        static_cast<float>(v.viewAngle),
        static_cast<float>(v.nearPlane),
        static_cast<float>(v.farPlane),
        eye, ref, vup
    );
#if 1
    eye[0] = 0.f; eye[1] = 0.f; eye[2] = 1.6f;
    ref[0] = 0.f; ref[1] = 0.f; ref[2] = 0.f;
    vup[0] = 0.f; vup[1] = 1.f; vup[2] = 0.f;
    ren.SetViewParameters(50.0f, 0.1f, 100.0f, eye,ref,vup);
#endif
}

#endif // USE_TUVOK
