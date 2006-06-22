/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

#include "avtOpenGLAtomTexturer.h"
#include <vtkConfigure.h>
#include <math.h>
#include <DebugStream.h>

//#ifndef TextureModeData
//#define GLEW_SUPPORTED
//#endif

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

    void SetHint(int,int) {; };

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

    void SetHint(int,int);

    bool ModeAvailable();
private:
    void MakeTextures();
    void FreeResources();

    // Shader-related data.
    static const char *GLSL_sphere_fragment_program_source;
    static const char *GLSL_sphere_vertex_program_source;
    static const char *GLSL_spheredepth_fragment_program_source;
    static const char *GLSL_spheredepth_vertex_program_source;
    bool GLSL_set_depth;
    bool GLSL_init;
    bool GLSL_shaders_setup;
    int  GLSL_screen_w, GLSL_screen_h;
    GLhandleARB f, v, p;
};


//
// Just do basic, flat depth, no lighting.
//
const char *ShaderModeData::GLSL_sphere_vertex_program_source = 
"varying vec4 C;"
"varying float tx, ty;"
"void main()"
"{"
"    C = gl_Color;"
"    tx = gl_MultiTexCoord0.x;"
"    ty = gl_MultiTexCoord0.y;"
"    float R = gl_Normal[0];"

"    vec4 z0pt = gl_ModelViewProjectionMatrix * gl_Vertex;"

"    vec4 pt = vec4(-R, R, 0., 1.);"
"    if(tx == 0. && ty == 0.)"
"       pt = vec4(-R, -R, 0., 1.);"
"    else if(tx == 1. && ty == 0.)"
"       pt = vec4(R, -R, 0., 1.);"
"    else if(tx == 1. && ty == 1.)"
"       pt = vec4(R, R, 0., 1.);"
"    vec4 pt2 = gl_ModelViewMatrixInverse * pt + gl_Vertex;"
"    vec4 pt3 = gl_ModelViewProjectionMatrix * pt2;"
"    pt3[2] = z0pt[2];"
"    gl_Position = pt3;"
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

//
// These programs try setting the depth of the fragment as well as 
// the color. 
//
// Note: They currently do not create the same depth values as would
//       be produced by the fixed functionality pipeline. I don't know why.
//

const char *ShaderModeData::GLSL_spheredepth_vertex_program_source = 
"varying vec4 C;"
"varying float tx, ty;"
"varying float R;"
"void main()"
"{"
"    C = gl_Color;"
"    tx = gl_MultiTexCoord0.x;"
"    ty = gl_MultiTexCoord0.y;"
"    R = gl_Normal[0];"

"    vec4 z0pt = gl_ModelViewProjectionMatrix * gl_Vertex;"

"    vec4 pt = vec4(-R, R, 0., 1.);"
"    if(tx == 0. && ty == 0.)"
"       pt = vec4(-R, -R, 0., 1.);"
"    else if(tx == 1. && ty == 0.)"
"       pt = vec4(R, -R, 0., 1.);"
"    else if(tx == 1. && ty == 1.)"
"       pt = vec4(R, R, 0., 1.);"
"    vec4 pt2 = gl_ModelViewMatrixInverse * pt + gl_Vertex;"
"    vec4 pt3 = gl_ModelViewProjectionMatrix * pt2;"
"    pt3[2] = z0pt[2];"
"    gl_Position = pt3;"
"}";

const char *ShaderModeData::GLSL_spheredepth_fragment_program_source = 
"uniform vec2 two_over_screen;"
"varying vec4 C;"
"varying float tx, ty;"
"varying float R;"
"void main()"
"{"
"    float x = tx * 2. - 1;"
"    float y = ty * 2. - 1;"
"    float x2y2 = x*x + y*y;"
"    float z = 0.;"
"    if(x2y2 > 1.)"
"        discard;"
"    z = sqrt(1. - x2y2);"
"    vec4 mc = C * z;"
"    mc.a = 1.;"
"    gl_FragColor = mc;"

"    float px = gl_FragCoord.x * two_over_screen[0] - 1.;"
"    float py = gl_FragCoord.y * two_over_screen[1] - 1.;"
"    float pz = gl_FragCoord.z * 2. - 1;"
"    vec4 world = gl_ModelViewProjectionMatrixInverse * vec4(px,py,pz,1.);"
"    world /= world.w;"
"    world.z += z * R;"
"    vec4 proj = gl_ModelViewProjectionMatrix * world;"
"    gl_FragDepth = (proj.z / proj.w) / 2. + 0.5;"
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
    GLSL_set_depth = true;
    GLSL_screen_w = GLSL_screen_h = 1;
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
    FreeResources();
}

// ****************************************************************************
// Method: ShaderModeData::FreeResources
//
// Purpose: 
//   Frees the resources allocated to the shaders.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 7 10:53:22 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
ShaderModeData::FreeResources()
{
    // Free the program.
    if(GLSL_shaders_setup)
    {
        glDetachObjectARB(p, v);
        glDetachObjectARB(p, f);
        glDeleteObjectARB(v);
        glDeleteObjectARB(f);
        glDeleteObjectARB(p);
        GLSL_shaders_setup = false;
    }
}

// ****************************************************************************
// Method: ShaderModeData::SetHint
//
// Purpose: 
//   Sets hints for the renderer.
//
// Arguments:
//   hint : The hint to set.
//   val  : The value to use for the hint.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 7 10:52:47 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
ShaderModeData::SetHint(int hint, int val)
{
    if(hint == avtOpenGLAtomTexturer::HINT_SET_DEPTH)
    {
        bool d = val != 0;

        // If the depth setting mode does not equal the one that we're
        // currently using, free the shader so we will recreate it
        // using the right shader programs.
        if(GLSL_set_depth != d)
            FreeResources();
        GLSL_set_depth = d;
    }
    else if(GLSL_set_depth)
    {
        if(hint == avtOpenGLAtomTexturer::HINT_SET_SCREEN_WIDTH)
            GLSL_screen_w = val;
        else if(hint == avtOpenGLAtomTexturer::HINT_SET_SCREEN_WIDTH)
            GLSL_screen_h = val;
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
//   Brad Whitlock, Fri Apr 7 11:18:26 PDT 2006
//   Added code to use alternate programs that also set the depth.
//
// ****************************************************************************

bool
ShaderModeData::BeginSphereTexturing()
{
    if(!ModeAvailable())
        return false;

    if(!GLSL_shaders_setup)
    {
        GLsizei loglen = 0;
#define MAX_LOG_SIZE 2000
        char *log = new char[MAX_LOG_SIZE];
        memset(log, 0, MAX_LOG_SIZE);

        int maxvarying = 0;
        glGetIntegerv(GL_MAX_VARYING_FLOATS_ARB, &maxvarying);
        debug1 << "Maximum # of varying floats:" << maxvarying << endl;

        // Create the shader program handle.
        p = glCreateProgramObjectARB();

        // Create the vertex program and link it to the program.
        v = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
        if(GLSL_set_depth)
            glShaderSourceARB(v, 1, &GLSL_spheredepth_vertex_program_source, NULL);
        else
            glShaderSourceARB(v, 1, &GLSL_sphere_vertex_program_source, NULL);
        glCompileShaderARB(v);
        int vc = 0;
        glGetObjectParameterivARB(v, GL_OBJECT_COMPILE_STATUS_ARB, &vc);
        debug1 << "Vertex program "
               << ((vc==1)?" compiled":" did not compile")
               << endl;
        glAttachObjectARB(p, v);
        memset(log, 0, MAX_LOG_SIZE);
        glGetInfoLogARB(v, MAX_LOG_SIZE, &loglen, log);
        debug1 << "Vertex log:\n" << log << endl;
    
        // Create the fragment program and link it to the program.
        f = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
        if(GLSL_set_depth)
            glShaderSourceARB(f, 1, &GLSL_spheredepth_fragment_program_source, NULL);
        else
            glShaderSourceARB(f, 1, &GLSL_sphere_fragment_program_source, NULL);
        glCompileShaderARB(f);
        int fc = 0;
        glGetObjectParameterivARB(f, GL_OBJECT_COMPILE_STATUS_ARB, &fc);
        debug1 << "Fragment program "
               << ((fc==1)?" compiled":" did not compile")
               << endl;
        glAttachObjectARB(p, f);
        memset(log, 0, MAX_LOG_SIZE);
        glGetInfoLogARB(f, MAX_LOG_SIZE, &loglen, log);
        debug1 << "Fragment log:\n" << log << endl;

        glLinkProgramARB(p);
 
        int pls = 0;
        glGetObjectParameterivARB(p, GL_OBJECT_LINK_STATUS_ARB, &pls);
        debug1 << "Program "
               << ((pls==1)?" linked":" did not link")
               << endl;

        memset(log, 0, MAX_LOG_SIZE);
        glGetInfoLogARB(p, MAX_LOG_SIZE, &loglen, log);
        debug1 << "Program log:\n" << log << endl;
        delete [] log;

        GLSL_shaders_setup = true;

        if(vc == 0 || fc == 0 || pls == 0)
        {
            debug1 << "One or more of the required shader programs is "
                      "not supported." << endl;
            return false;
        }
    }

    if(GLSL_set_depth)
    {
        // Pass the 2./screen_size to the fragment program via uniform
        // vec2.
        float two_over_screen[2];
        two_over_screen[0] = 2. / float(GLSL_screen_w);
        two_over_screen[1] = 2. / float(GLSL_screen_h);
        GLint u = glGetUniformLocationARB(p, "two_over_screen");
        glUniform1fvARB(u, 2, two_over_screen);
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
    void SetHint(int,int) { };
};
#endif

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
///
/// avtOpenGLAtomTexturer CLASS
///
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

const int avtOpenGLAtomTexturer::HINT_SET_DEPTH = 0;
const int avtOpenGLAtomTexturer::HINT_SET_SCREEN_WIDTH = 1;
const int avtOpenGLAtomTexturer::HINT_SET_SCREEN_HEIGHT = 2;

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
// Method: avtOpenGLAtomTexturer::SetHint
//
// Purpose: 
//   Sets hints into the different shading implementations.
//
// Arguments:
//   hint  : The hint to set.
//   value : The value to use for the hint.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 7 11:25:36 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtOpenGLAtomTexturer::SetHint(int hint, int value)
{
    ((TextureModeData *)tData)->SetHint(hint, value);
    ((ShaderModeData *)sData)->SetHint(hint, value);
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
