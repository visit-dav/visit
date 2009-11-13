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
//                              RuntimeSetting.C                             //
// ************************************************************************* //
#include <cstring>
#include <map>
#include <string>

#include <RuntimeSetting.h>
#include <DebugStream.h>
#include <Environment.h>
#include <InstallationFunctions.h>
#include <VisItException.h>

// Anonymous namespace
namespace {

struct Setting
{
    const char *key;           ///< key for identifying this setting in code.
    const char *default_value; ///< compile-time default for the setting
    bool        isFile;
    const char *cmdline;       ///< command line option
    const char *envvar;        ///< which environment variable affects this?
    const char *helpstr;       ///< short help string which describes what this does
    /// @todo const char* cfg; ///< for the key which sets this in the cfg file
};

// This essentially creates a unique namespace essentially holding two `arrays'
// which give information on a setting.  The first gives basic, compile-time information:
// default values and how to influence this particular setting.  The second
// holds runtime overrides for variables, set for example when we parse command
// line options.
struct s
{
    static const struct Setting settings[];

    typedef std::map<std::string, std::string> runtime_type;
    static runtime_type runtime_settings;
};

// Keep this ordered by key (first field per entry).
// Strings are special: if they start with ".", they're assumed to be a
// relative path, and get our installation directory prepended.
const struct Setting s::settings[] = {
#ifdef __APPLE__
    {"mesa-lib", "./lib/libOSMesa.dylib", true, "--mesa-lib", "VISIT_MESA_LIB",
     "Library with mangled offscreen mesa symbols."},
#elif defined(_WIN32)
    {"mesa-lib", "./lib/libOSMesa.dll", true, "--mesa-lib", "VISIT_MESA_LIB",
     "Library with mangled offscreen mesa symbols."},
#else
    {"mesa-lib", "./lib/libOSMesa.so", true, "--mesa-lib", "VISIT_MESA_LIB",
     "Library with mangled offscreen mesa symbols."},
#endif
#ifdef __APPLE__
    {"system-gl",
     "/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", true,
     "--system-gl-lib", "VISIT_GL_LIB",
     "Library (dylib) which implements OpenGL."},
#elif defined(_WIN32)
    {"mesa-lib", "./lib/libOSMesa.dll", true, "--mesa-lib", "VISIT_MESA_LIB",
     "Library with mangled offscreen mesa symbols."},
#else
    {"system-gl", "/usr/lib/libGL.so", true, "--system-gl-lib",
     "VISIT_GL_LIB", "Library (so) which implements OpenGL."},
#endif
    {"tuvok-shader-dir", "./share/Shaders", true, "--tuvok-shaders",
     "VISIT_TUVOK_SHADER_DIR", "Directory which holds Tuvok's GLSL shaders."}
};

// Runtime modification map.  This is where we'll put settings we figure out
// at runtime; currently that means those obtained via parsing command lines.
s::runtime_type s::runtime_settings;

// ****************************************************************************
//  Function: RuntimeSetting::compile_time_default
//
//  Purpose: Looks up the compile time default for the given type.
//
//  Programmer: Tom Fogal
//  Creation: July 22, 2009
//
// ****************************************************************************

static std::string compile_time_default(int i)
{
    std::string retval;

    // If the setting starts with ".", assume it's a relative path and
    // prepend our arch dir.
    if(s::settings[i].isFile && s::settings[i].default_value[0] == '.')
    {
        retval = GetVisItInstallationDirectory() + "/";
    }
    retval += s::settings[i].default_value;
    return retval;
}

// ****************************************************************************
//  Function: RuntimeSetting::find_setting
//
//  Purpose: searches through the settings list for the entry with the given
//           key.  Looks for overrides in environment variables and parsed
//           command line options.
//
//  Programmer: Tom Fogal
//  Creation: June 27, 2009
//
//  Modifications:
//    Brad Whitlock, Nov 13, 2009
//    I detemplated the code to simplify it and make it work on AIX.
//    
// ****************************************************************************

static bool
find_setting(const char *key, std::string &retval)
{
    size_t i;
    size_t len = strlen(key);
    const size_t n = sizeof(s::settings) / sizeof(struct Setting);

    // lookup the default from our settings array.
    for(i=0; i < n; ++i)
    {
        if(strncmp(s::settings[i].key, key, len) == 0)
        {
            retval = compile_time_default(i);
            break;
        }
    }
    if(i == n)
    {
        return false;
    }

    /// @todo FIXME Here we *should* lookup the value in a configuration file.

    // Now lookup from the environment variable, if it exists.
    if(Environment::exists(s::settings[i].envvar))
    {
        retval = Environment::get(s::settings[i].envvar);
    }

    // Finally, check any overrides from cmd line options.
    s::runtime_type::const_iterator iter =
      s::runtime_settings.find(std::string(key));
    if(iter != s::runtime_settings.end())
    {
        retval = iter->second;
    }

    return true;
}

// ****************************************************************************
// Function: RuntimeSetting::create_clo_overrides
//
// Purpose: Create overrides from a given set of command line options (clo's)
//
//  Programmer: Tom Fogal
//  Creation: June 27, 2009
//
//  Modifications:
//    Brad Whitlock, Nov 13, 2009
//    I detemplated the code to simplify it and make it work on AIX.
//
// ****************************************************************************
static void
create_clo_overrides(int argc, const char *argv[])
{
    const size_t n = sizeof(s::settings) / sizeof(struct Setting);
    for(int i=0; i < argc; ++i)
    {
        for(size_t j=0; j < n; ++j)
        {
            if(strcmp(s::settings[j].cmdline, argv[i]) == 0)
            {
                if(i+1 >= argc)
                {
                    debug1 << argv[i] << " option given with no parameter!"
                           << "  Ignoring option." << std::endl;
                    return;
                }

                debug1 << "Storing override " << argv[i] << " = " << argv[i+1]
                       << endl;
                std::string override(argv[i+1]);
                s::runtime_settings[s::settings[j].key] = override;

                ++i;
                break;
            }
        }
    }
}

}; // end anon namespace.

namespace RuntimeSetting {

// ****************************************************************************
//  Function: RuntimeSetting::lookup
//
//  Purpose:  Looks up a value.
//
//  Programmer: Tom Fogal
//  Creation: June 27, 2009
//
// ****************************************************************************
int
lookupi(const char *key)
{
    int retval = 0;
    std::string value;
    if(find_setting(key, value))
    {
        std::istringstream read_value(value);
        read_value >> retval;
    }
    return retval;
}

double
lookupf(const char *key)
{
    double retval = 0.;
    std::string value;
    if(find_setting(key, value))
    {
        std::istringstream read_value(value);
        read_value >> retval;
    }
    return retval;
}

std::string
lookups(const char *key)
{
    std::string retval;
    std::string value;
    if(find_setting(key, value))
    {
        std::istringstream read_value(value);
        read_value >> retval;
    }
    return retval;
}

bool
lookupb(const char *key)
{
    bool retval = true;
    std::string value;
    if(find_setting(key, value))
    {
        std::istringstream read_value(value);
        read_value >> retval;
    }
    return retval;
}

// ****************************************************************************
//  Function: RuntimeSetting::parse_command_line
//
//  Purpose:  Creates overrides based on command line parameters.
//
//  Programmer: Tom Fogal
//  Creation: June 27, 2009
//
// ****************************************************************************
void parse_command_line(int argc, const char *argv[])
{
    create_clo_overrides(argc, argv);
}

};  /* namespace RuntimeSetting */
