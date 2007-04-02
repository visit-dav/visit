#include "avtOpenGLAtomTexturer.h"
#include <math.h>
#include <DebugStream.h>

#ifdef GLEW_SUPPORTED
#include <GL/glew.h>
#endif

#ifndef VTK_IMPLEMENT_MESA_CXX
  #if defined(__APPLE__) && (defined(VTK_USE_CARBON) || defined(VTK_USE_COCOA))
    #include <OpenGL/gl.h>
  #else
    #if defined(_WIN32)
       #include <windows.h>
    #endif
    #include <GL/gl.h>
  #endif
#else
  #include <GL/gl.h>
#endif

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
///
/// BEGIN TEXTURE-BASED SPHERE IMPLEMENTATION
///
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#define SPHERE_TEX_W 256
#define SPHERE_TEX_H 256

// ****************************************************************************
// Class: TextureModeData
//
// Purpose:
//   Shades imposter quads using a texture that looks like a sphere.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 27 17:22:06 PST 2006
//
// Modifications:
//   
// ****************************************************************************

class TextureModeData
{
public:
    TextureModeData();
    virtual ~TextureModeData();
    bool BeginSphereTexturing();
    void EndSphereTexturing();

    bool ModeAvailable() { return true; }
private:
    void MakeTextures();

    // Texture-related data.
    bool sphereTexturesDataCreated;
    bool sphereTexturesLoaded;
    unsigned int  textureName;
    unsigned char sphereTexture[SPHERE_TEX_H][SPHERE_TEX_W][2];

    // Keep track of OpenGL state
    int   isBlendEnabled;
    int   blendFunc0;
    int   blendFunc1;
    int   needAlphaTest;
    int   isAlphaTestEnabled;
    int   alphaTestFunc;
    float alphaTestRef;
};

// ****************************************************************************
// Method: TextureModeData::TextureModeData
//
// Purpose: Constructor
//   
// Programmer: Brad Whitlock
// Creation:   Mon Mar 27 17:22:27 PST 2006
//
// Modifications:
//   
// ****************************************************************************

TextureModeData::TextureModeData()
{
    sphereTexturesDataCreated = false;
    sphereTexturesLoaded = false;
    textureName = 0;

    isBlendEnabled = 0;
    blendFunc0 = 0;
    blendFunc1 = 0;
    needAlphaTest = 0;
    isAlphaTestEnabled = 0;
    alphaTestFunc = 0;
    alphaTestRef = 0.;
}

// ****************************************************************************
// Method: TextureModeData::~TextureModeData
//
// Purpose: Destructor
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 27 17:22:46 PST 2006
//
// Modifications:
//   
// ****************************************************************************

TextureModeData::~TextureModeData()
{
    if(sphereTexturesLoaded)
        glDeleteTextures(1, &textureName);
}

// ****************************************************************************
// Method: TextureModeData::BeginSphereTexturing
//
// Purpose: 
//   Sets up a sphere texture that we will apply to all of the imposter quads.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 27 17:18:28 PST 2006
//
// Modifications:
//   
// ****************************************************************************

bool
TextureModeData::BeginSphereTexturing()
{
    // Create the rextures
    if(!sphereTexturesDataCreated)
    {
        MakeTextures();
        sphereTexturesDataCreated = true;
    }

    // Create and bind the textures if we have not done that yet.
    if(!sphereTexturesLoaded)
    {
        glGenTextures(1, &textureName);

        // Set up the first texture
        glBindTexture(GL_TEXTURE_2D, textureName);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, 2, SPHERE_TEX_W, SPHERE_TEX_H,
                     0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, sphereTexture);

        sphereTexturesLoaded = true;
    }


    //
    // Get whether GL_BLEND is enabled.
    //
    glGetIntegerv(GL_BLEND, &isBlendEnabled);
    if(isBlendEnabled == 0)
        glEnable(GL_BLEND);

    glGetIntegerv(GL_BLEND_SRC, &blendFunc0);
    glGetIntegerv(GL_BLEND_DST, &blendFunc1);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //
    // Get the AlphaTest mode to restore it later.
    //
    int dt = 0;
    glGetIntegerv(GL_DEPTH_TEST, &dt);
    if(dt == 1)
    {
        // Get the current alpha test function
        glGetIntegerv(GL_ALPHA_TEST, &isAlphaTestEnabled);
        if(isAlphaTestEnabled)
        {
            glGetIntegerv(GL_ALPHA_TEST_FUNC, &alphaTestFunc);
            glGetFloatv(GL_ALPHA_TEST_REF, &alphaTestRef);
        }
        else
            glEnable(GL_ALPHA_TEST);

        // Set the alpha testing mode and function.
        glAlphaFunc(GL_GREATER, 0.7);

        needAlphaTest = 1;
    }
    else
        needAlphaTest = 0;

    //
    // Turn on the texture
    //
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureName);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);

    return true;
}

// ****************************************************************************
// Method: TextureModeData::EndSphereTexturing
//
// Purpose: 
//   Disables texuring for the imposter quads.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 27 17:19:08 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
TextureModeData::EndSphereTexturing()
{
    if(needAlphaTest)
    {
        if(isAlphaTestEnabled)
            glAlphaFunc(alphaTestFunc, alphaTestRef);
        else
            glDisable(GL_ALPHA_TEST);
    }

    if(isBlendEnabled == 0)
        glDisable(GL_BLEND);

    // Restore the old blend function.
    glBlendFunc(blendFunc0, blendFunc1);

    glDisable(GL_TEXTURE_2D);
}

// ****************************************************************************
// Method: TextureModeData::MakeTextures
//
// Purpose: 
//   Makes a sphere texture that we'll apply to imposter quads.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 27 17:19:31 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
TextureModeData::MakeTextures()
{
   int i, j;

   float dx = SPHERE_TEX_H * 0.5f;
   float dy = SPHERE_TEX_H * 0.5f;
   float R = dx-2;

   float minT = 0.;
   float maxT = 0.;

   GLubyte texture[SPHERE_TEX_H][SPHERE_TEX_H][2];

   for (j = 0; j < SPHERE_TEX_H; j++)
   {
      float y = (float(j) / float(SPHERE_TEX_H-1)) * 2. - 1.;
      for (i = 0; i < SPHERE_TEX_W; i++)
      {
         float x = (float(i) / float(SPHERE_TEX_W-1)) * 2. - 1.;
         float x2y2 = sqrt(x*x + y*y);
         if(x2y2 < 1.)
         {
             float z = sqrt(1. - x2y2);
             GLubyte rc = (GLubyte)(z * 255.);

             texture[j][i][0] = (GLubyte) 255 - rc;
             texture[j][i][1] = (GLubyte) 255;
         }
         else
         {
             texture[j][i][0] = (GLubyte) 0;
             texture[j][i][1] = (GLubyte) 0;
         }
      }
   }

   //
   // Blur the texture a little
   //
   float kernel[3][3] = {
       {0.125, 0.2, 0.125},
       {0.200, 1.0, 0.200},
       {0.125, 0.2, 0.125}};
   float kernelSum = kernel[0][0] + kernel[0][1] + kernel[0][2] + 
                     kernel[1][0] + kernel[1][1] + kernel[1][2] + 
                     kernel[2][0] + kernel[2][1] + kernel[2][2];
   for (j = 0; j < SPHERE_TEX_H; j++)
   {
       for (i = 0; i < SPHERE_TEX_W; i++)
       {
           if(i >= 1 && i < SPHERE_TEX_W-1 &&
              j >= 1 && j < SPHERE_TEX_H-1)
           {
               for(int c = 0; c < 2; ++c)
               {
                   float t = kernel[0][0] * float(texture[j-1][i-1][c]) + 
                             kernel[0][1] * float(texture[j-1][i][c]) + 
                             kernel[0][2] * float(texture[j-1][i+1][c]) + 
                             kernel[1][0] * float(texture[j][i-1][c]) + 
                             kernel[1][1] * float(texture[j][i][c]) + 
                             kernel[1][2] * float(texture[j][i+1][c]) + 
                             kernel[2][0] * float(texture[j+1][i-1][c]) + 
                             kernel[2][1] * float(texture[j+1][i][c]) + 
                             kernel[2][2] * float(texture[j+1][i+1][c]);
                   t /= kernelSum;
                   sphereTexture[j][i][c] = (GLubyte)t;
               }
           }
           else
           {
               sphereTexture[j][i][0] = (GLubyte)0;
               sphereTexture[j][i][1] = (GLubyte)0;
           }
       }
   }
}

#ifdef GLEW_SUPPORTED
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
///
/// BEGIN GLSL SHADER-BASED SPHERE IMPLEMENTATION
///
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Class: ShaderModeData
//
// Purpose:
//   Shades imposter quads so they look like spheres.
//
// Notes:      The shaders used by this class do not currently set a fragment's
//             depth. We need to fix that.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 27 17:21:38 PST 2006
//
// Modifications:
//   
// ****************************************************************************

class ShaderModeData
{
public:
    ShaderModeData();
    virtual ~ShaderModeData();
    bool BeginSphereTexturing();
    void EndSphereTexturing();

    bool ModeAvailable();
private:
    void MakeTextures();

    // Shader-related data.
    static const char *GLSL_sphere_fragment_program_source;
    static const char *GLSL_sphere_vertex_program_source;
    bool GLSL_init;
    bool GLSL_shaders_setup;
    GLhandleARB f, v, p;
};

const char *ShaderModeData::GLSL_sphere_vertex_program_source = 
"varying vec4 C;"
"varying float tx, ty;"
"void main()"
"{"
"    C = gl_Color;"
"    tx = gl_MultiTexCoord0.x;"
"    ty = gl_MultiTexCoord0.y;"
"    vec4 pt = vec4(0.,0.,0., 1.);"
"    if(tx == 0. && ty == 0.)"
"       pt = vec4(-1., -1., 0., 1.);"
"    else if(tx == 1. && ty == 0.)"
"       pt = vec4(1., -1., 0., 1.);"
"    else if(tx == 1. && ty == 1.)"
"       pt = vec4(1., 1., 0., 1.);"
"    else"
"       pt = vec4(-1., 1., 0., 1.);"
"    mat4 imv = gl_ModelViewMatrixInverse;"
"    float radius = gl_Normal[0];"
"    vec4 pt2 = imv * (pt * radius);"
"    vec4 pt3 = pt2 + gl_Vertex;"
"    gl_Position = gl_ModelViewProjectionMatrix * pt3;"
"}";

const char *ShaderModeData::GLSL_sphere_fragment_program_source =
"varying vec4 C;"
"varying float tx, ty;"
"void main()"
"{"
"    float x = tx * 2. - 1;"
"    float y = ty * 2. - 1;"
"    float x2y2 = x*x + y*y;"
"    float z = 0.;"
"    if(x2y2 > 1.)"
"        discard;"
"    else"
"        z = sqrt(1. - x2y2);"
"    vec4 mc = C * z;"
"    mc.a = 1.;"
"    gl_FragColor = mc;"
"}";

// ****************************************************************************
// Method: ShaderModeData::ShaderModeData
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 28 10:46:31 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

ShaderModeData::ShaderModeData()
{
    GLSL_shaders_setup = false;
    GLSL_init = false;
}

// ****************************************************************************
// Method: ShaderModeData::~ShaderModeData
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 28 10:46:10 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

ShaderModeData::~ShaderModeData()
{
    // Free the program.
    if(GLSL_shaders_setup)
    {
        glDetachObjectARB(p, v);
        glDetachObjectARB(p, f);
        glDeleteObjectARB(v);
        glDeleteObjectARB(f);
        glDeleteObjectARB(p);
    }
}

// ****************************************************************************
// Method: ShaderModeData::ModeAvailable
//
// Purpose: 
//   Returns whether or not the shader mode is available.
//
// Returns:    True if we can use GLSL; False otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 28 10:45:03 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
ShaderModeData::ModeAvailable()
{
    if(!GLSL_init)
    {
        glewInit();
        GLSL_init = true;
    }
    return GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader;
}

// ****************************************************************************
// Method: ShaderModeData::BeginSphereTexturing
//
// Purpose: 
//   Set up a fragment shader that will texture the imposter quads so they
//   look like spheres.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 27 17:20:02 PST 2006
//
// Modifications:
//   
// ****************************************************************************

bool
ShaderModeData::BeginSphereTexturing()
{
    if(!ModeAvailable())
        return false;

    if(!GLSL_shaders_setup)
    {
        // Create the shader program handle.
        p = glCreateProgramObjectARB();

        // Create the vertex program and link it to the program.
        v = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
        glShaderSourceARB(v, 1, &GLSL_sphere_vertex_program_source, NULL);
        glCompileShaderARB(v);
        int vc = 0;
        glGetObjectParameterivARB(v, GL_OBJECT_COMPILE_STATUS_ARB, &vc);
        debug1 << "Vertex program "
               << ((vc==1)?" compiled":" did not compile")
               << endl;
        glAttachObjectARB(p, v);
    
        // Create the fragment program and link it to the program.
        f = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
        glShaderSourceARB(f, 1, &GLSL_sphere_fragment_program_source, NULL);
        glCompileShaderARB(f);
        int fc = 0;
        glGetObjectParameterivARB(f, GL_OBJECT_COMPILE_STATUS_ARB, &fc);
        debug1 << "Fragment program "
               << ((fc==1)?" compiled":" did not compile")
               << endl;
        glAttachObjectARB(p, f);

        glLinkProgramARB(p);
 
        int pls = 0;
        glGetObjectParameterivARB(p, GL_OBJECT_LINK_STATUS_ARB, &pls);
        debug1 << "Program "
               << ((pls==1)?" linked":" did not link")
               << endl;

        GLSL_shaders_setup = true;

        if(vc == 0 || fc == 0 || pls == 0)
        {
            debug1 << "One or more of the required shader programs is "
                      "not supported." << endl;
            return false;
        }
    }

    // Start using the shader.
    glUseProgramObjectARB(p);

    return true;
}

// ****************************************************************************
// Method: ShaderModeData::EndSphereTexturing
//
// Purpose: 
//   Turn off the shader for imposter quads.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 27 17:20:43 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
ShaderModeData::EndSphereTexturing()
{
    // Quit using the shader.
    glUseProgramObjectARB(0);
}
#else
//
// Dummy implementation for when we don't define GLEW_SUPPORTED
//
class ShaderModeData
{
public:
    ShaderModeData() { };
    virtual ~ShaderModeData() { };
    bool ModeAvailable() { return false; }
    bool BeginSphereTexturing() { return false; };
    void EndSphereTexturing() { };
};
#endif

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
///
/// avtOpenGLAtomTexturer CLASS
///
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

avtOpenGLAtomTexturer::avtOpenGLAtomTexturer()
{
    modeDetermined = false;
    tData = (void *)new TextureModeData;
    sData = (void *)new ShaderModeData;
}

avtOpenGLAtomTexturer::~avtOpenGLAtomTexturer()
{
    delete (TextureModeData *) tData;
    delete (ShaderModeData *) sData;
}

// ****************************************************************************
// Method: avtOpenGLAtomTexturer::BeginSphereTexturing
//
// Purpose: 
//   Starts sphere texturing using the best available mode.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 28 10:19:16 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtOpenGLAtomTexturer::BeginSphereTexturing()
{
    GetMode();

    if(mode == ShaderMode)
        if(!((ShaderModeData *)sData)->BeginSphereTexturing())
        {
            debug1 << "BeginSphereTexturing: Reverting back to texture "
                      "method from shading method." << endl;
            mode = TextureMode;
        }

    if(mode == TextureMode)
        ((TextureModeData *)tData)->BeginSphereTexturing();
}

// ****************************************************************************
// Method: avtOpenGLAtomTexturer::EndSphereTexturing
//
// Purpose: 
//   Stops sphere texturing.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 28 10:19:37 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtOpenGLAtomTexturer::EndSphereTexturing()
{
    GetMode();

    if(mode == ShaderMode)
        ((ShaderModeData *)sData)->EndSphereTexturing();
    else if(mode == TextureMode)
        ((TextureModeData *)tData)->EndSphereTexturing();
}

// ****************************************************************************
// Method: avtOpenGLAtomTexturer::Mode
//
// Purpose: 
//   Returns the sphere shading mode that we'll use, determining the best
//   available mode if we have not already done so.
//
// Returns:    The best available texturing mode.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 28 10:19:59 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

avtOpenGLAtomTexturer::TexMode_t
avtOpenGLAtomTexturer::GetMode()
{ 
    if(!modeDetermined)
    {
        if( ((ShaderModeData *)sData)->ModeAvailable() )
            mode = ShaderMode;
        else
            mode = TextureMode;
        modeDetermined = true;
    }

    return mode;
}
