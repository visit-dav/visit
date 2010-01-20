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
//                              avtGLSLProgram.C                             //
// ************************************************************************* //

#include <avtGLSLProgram.h>
#include <DebugStream.h>

#include <cstring>

// ****************************************************************************
//  Method: avtGLSLProgram constructor
//
//  Programmer: Christoph Garth
//  Creation:   Mon Jan 11 16:25:07 PST 2010 
//
// ****************************************************************************

avtGLSLProgram::avtGLSLProgram( const std::string& _name ) : 
    name(_name), program(0), previous(-1), supported(-1)
{
}

// ****************************************************************************
//  Method: avtGLSLProgram destructor
//
//  Programmer: Christoph Garth
//  Creation:   Mon Jan 11 16:25:07 PST 2010 
//
// ****************************************************************************

avtGLSLProgram::~avtGLSLProgram()
{
    Destroy();
}

// ****************************************************************************
//  Method: avtGLSLProgram::AttachShaderFromString
//
//  Purpose: 
//      Records source code for a new shader, to be compiled and attached on
//      the next call to Enable() or Create().
//     
//  Programmer: Christoph Garth
//  Creation:   Mon Jan 11 16:25:07 PST 2010 
//
// ****************************************************************************

void avtGLSLProgram::AttachShaderFromString( GLenum type, const std::string& source )
{
    shaders.push_back( std::make_pair(type,source) );
}
    
// ****************************************************************************
//  Method: avtGLSLProgram::Enable
//
//  Purpose: 
//      Tell OpenGL to begin using the program. Create() is called to make 
//      sure all outstanding shader sources have been attached. The previously
//      bound shader is stored and reset on the following call of Disable(). 
//      If the program is not supported by the current context or an error
//      occurred during compilation, the fixed function pipeline is activated;
//      this is indicated by a false return value. 
//
//  Returns:
//      True if the shader was activated without error, false otherwise.
//     
//  Programmer: Christoph Garth
//  Creation:   Mon Jan 11 16:26:32 PST 2010 
//
// ****************************************************************************

bool avtGLSLProgram::Enable()
{
    if( !IsSupported() )        
        return false;

    glGetIntegerv( GL_CURRENT_PROGRAM, &previous );

    bool ok = Create();

    if( ok )
        glUseProgram( ok ? program : 0 );
        
    return ok;
}
    
// ****************************************************************************
//  Method: avtGLSLProgram::Disable
//
//  Purpose: 
//      Tell OpenGL to stop using the program. If a program was active at
//      the time of the last Enable(), that program is restored.
//
//  Programmer: Christoph Garth
//  Creation:   Mon Jan 11 16:26:32 PST 2010 
//
// ****************************************************************************
    
void avtGLSLProgram::Disable()
{
    if( !IsSupported() )
        return;

    if( previous >= 0 )
        glUseProgram( previous );
        
    previous = -1;
}                              
  
// ****************************************************************************
//  Method: avtGLSLProgram::Create
//
//  Purpose: 
//      Create the current program, i.e. compile and attach all 
//      previously attached shader sources and link the program.
// 
//  Returns:
//      True upon successful creation, false otherwise.
//
//  Programmer: Christoph Garth
//  Creation:   Mon Jan 11 16:28:51 PST 2010 
//
//  Modifications:
//
//    Tom Fogal, Wed Jan 20 11:48:50 MST 2010
//    Formatting, bail if program creation fails.
//    Delete shader logs.
//
// ****************************************************************************

bool avtGLSLProgram::Create()
{
    if( !IsSupported() )
        return false;

    // Create the program if it's not already there
    if( program == 0 )            
        program = glCreateProgram();

    if( program == 0 )
    {
        debug1 << "avtGLSLProgram \"" << name << "\": "
                  "GLSL program could not be created\n";
        return false;
    }

    bool shadersOK = true;

    // Go through the list of shaders, and compile and attach each of them

    for( std::vector<Shader>::const_iterator si=shaders.begin();
         si!=shaders.end(); ++si )
    {
        GLuint shader = glCreateShader( si->first );
        
        if( !shader )
        {
            debug1 << "avtGLSLProgram \"" << name << "\": "
                      "GLSL shader could not be created\n";
        }
        
        const char* source = si->second.c_str();
        
        glShaderSource( shader, 1, &source, NULL );
        glCompileShader( shader );

        GLint length = 0;
        glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &length );
        
        if( length )
        {
            GLchar* log = new GLchar[length];
            glGetShaderInfoLog( shader, length, NULL, log );
            
            debug1 << "avtGLSLProgram \"" << name << "\": "
                   << "GLSL shader compile log:\n" << log << '\n';
            delete[] log;
        }
        
        GLint compiled;
        glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );
        
        if( compiled != GL_TRUE )
        {
            debug1 << "avtGLSLProgram \"" << name << "\": "
                      "GLSL shader failed to compile\n";
            shadersOK = false;
        }
            
        glAttachShader( program, shader );
        glDeleteShader( shader );
    }
    
    shaders.clear();
    
    // Link the program
    glLinkProgram( program );
        
    GLint length = 0;
    glGetProgramiv( program, GL_INFO_LOG_LENGTH, &length );
        
    if( length )
    {
        GLchar* log = new GLchar[length];
        glGetProgramInfoLog( program, length, NULL, log );
            
        debug1 << "avtGLSLProgram \"" << name << "\": GLSL program link log:\n"
                << log << '\n';
        delete[] log;
    }
        
    GLint linked;
    glGetProgramiv( program, GL_LINK_STATUS, &linked );

    if( linked != GL_TRUE )
        debug1 << "avtGLSLProgram \""<< name << "\": "
                  "GLSL program failed to link\n";

    if( shadersOK && linked )
        return true;
        
    glDeleteProgram( program );
    program = 0;
    
    return false;
}
   
  
// ****************************************************************************
//  Method: avtGLSLProgram::Destroy
//
//  Purpose: 
//      Destroy the program and free all resources.
//
//  Programmer: Christoph Garth
//  Creation:   Mon Jan 11 16:29:02 PST 2010 
//
// ****************************************************************************
    
void avtGLSLProgram::Destroy()
{
    if( !IsSupported() )
        return;
        
    if( program )
    {
        glDeleteProgram( program );
        program = 0;
    }
}

// ****************************************************************************
//  Method: avtGLSLProgram::IsSupported
//
//  Purpose: 
//      Determine whether GLSL programs are supported by the OpenGL context.
//
//  Returns:
//      True if all required extensions are present, false otherwise.
//
//  Programmer: Christoph Garth
//  Creation:   Mon Jan 11 16:32:22 PST 2010 
//
// ****************************************************************************
   
bool avtGLSLProgram::IsSupported()
{
    if( supported >= 0 )
        return supported;

    if( !avt::glew::initialize() )
        return supported = 0;
        
    return supported = 
        glewIsSupported( "GL_VERSION_2_0" ) &&
        glewIsSupported( "GL_ARB_shading_language_100" );
}

