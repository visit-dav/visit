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

// Anonymous namespace to make sure our templates don't have external linkage.
namespace {

template<typename T>
struct Setting
{
    const char *key;     ///< key for identifying this setting in code.
    T default_value;     ///< compile-time default for the setting
    const char *cmdline; ///< command line option
    const char *envvar;  ///< which environment variable affects this?
    const char *helpstr; ///< short help string which describes what this does
    /// @todo const char* cfg; ///< for the key which sets this in the cfg file
};

// This essentially creates a unique namespace for every different kind of
// setting.  Each namespace essentially holds two `arrays' which give
// information on a setting.  The first gives basic, compile-time information:
// default values and how to influence this particular setting.  The second
// holds runtime overrides for variables, set for example when we parse command
// line options.
template<class T> struct s
{
    static const struct Setting<const T> settings[];

    typedef std::map<const char*, T> runtime_type;
    static runtime_type runtime_settings;
};

// MSVC's compiler (2005 + 2008) will ICE out if we declare our settings arrays
// properly.  A workaround is to not specify the null template list.  Such code
// isn't legal as per C++98 (ref. 14.5.1.3 temp.static; 14.7.3, temp.spec), so
// only do it on MS compilers.  A bug with FeedbackID 471170 has been filed
// with MS.
#ifdef _MSC_VER
# define TEMPLATE_NULL_OR_MSVC /* nothing */
#else
# define TEMPLATE_NULL_OR_MSVC template<>
#endif

// Keep this ordered by key (first field per entry).
// Strings are special: if they start with ".", they're assumed to be a
// relative path, and get our installation directory prepended.
TEMPLATE_NULL_OR_MSVC
const struct Setting<const std::string> s<std::string>::settings[] = {
#ifdef __APPLE__
    {"mesa-lib", "./lib/libOSMesa.dylib", "--mesa-lib", "VISIT_MESA_LIB",
     "Library with mangled offscreen mesa symbols."},
#elif defined(_WIN32)
    {"mesa-lib", "./lib/libOSMesa.dll", "--mesa-lib", "VISIT_MESA_LIB",
     "Library with mangled offscreen mesa symbols."},
#else
    {"mesa-lib", "./lib/libOSMesa.so", "--mesa-lib", "VISIT_MESA_LIB",
     "Library with mangled offscreen mesa symbols."},
#endif
#ifdef __APPLE__
    {"system-gl",
     "/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL",
     "--system-gl-lib", "VISIT_GL_LIB",
     "Library (dylib) which implements OpenGL."},
#elif defined(_WIN32)
    {"mesa-lib", "./lib/libOSMesa.dll", "--mesa-lib", "VISIT_MESA_LIB",
     "Library with mangled offscreen mesa symbols."},
#else
    {"system-gl", "/usr/lib/libGL.so", "--system-gl-lib",
     "VISIT_GL_LIB", "Library (so) which implements OpenGL."},
#endif
    {"tuvok-shader-dir", "./share/Shaders", "--tuvok-shaders",
     "VISIT_TUVOK_SHADER_DIR", "Directory which holds Tuvok's GLSL shaders."}
};

// Other statics; currently empty since they are unused.
TEMPLATE_NULL_OR_MSVC const struct Setting<const int> s<int>::settings[] = { };
TEMPLATE_NULL_OR_MSVC const struct Setting<const double> s<double>::settings[] = { };
TEMPLATE_NULL_OR_MSVC const struct Setting<const bool> s<bool>::settings[] = { };

// Runtime modification maps.  This is where we'll put settings we figure out
// at runtime; currently that means those obtained via parsing command lines.
TEMPLATE_NULL_OR_MSVC s<int>::runtime_type s<int>::runtime_settings =
    s<int>::runtime_type();
TEMPLATE_NULL_OR_MSVC s<double>::runtime_type s<double>::runtime_settings =
    s<double>::runtime_type();
TEMPLATE_NULL_OR_MSVC s<bool>::runtime_type s<bool>::runtime_settings =
    s<bool>::runtime_type();
TEMPLATE_NULL_OR_MSVC
s<std::string>::runtime_type s<std::string>::runtime_settings =
    s<std::string>::runtime_type();

// Thrown as an exception.
struct KeyNotFound {};

// ****************************************************************************
//  Function: RuntimeSetting::read_env
//
//  Purpose: Read a value from an environment variable and convert it to the
//           needed type.
//           This is templatized/extracted so that we can special case the
//           const char* case: one can't use the input operator on a const
//           char* (it's const!)
//
//  Programmer: Tom Fogal
//  Creation: June 27, 2009
//
// ****************************************************************************
template<typename T>
static void read_env(const std::string &s, T &t)
{
    std::string value = Environment::get(s.c_str());
    std::istringstream read_value(value);
    read_value >> t;
}
template<>
void read_env(const std::string &s, const char *&t)
{
    t = Environment::get(s.c_str()).c_str();
}
// ****************************************************************************
//  Function: RuntimeSetting::read_val
//
//  Purpose: Similar to read_env, for input from another const char*.
//
//  Programmer: Tom Fogal
//  Creation: June 27, 2009
//
// ****************************************************************************
template<typename T>
static void read_val(const char* s, T &t)
{
    std::istringstream read_value(s);
    read_value >> t;
}
template<>
void read_val(const char* s, const char *&t)
{
    t = s;
}

// ****************************************************************************
//  Function: RuntimeSetting::compile_time_default
//
//  Purpose: Looks up the compile time default for the given type.
//           Exists so we can specialize on string and detect paths.
//
//  Programmer: Tom Fogal
//  Creation: July 22, 2009
//
// ****************************************************************************
template <typename T>
T compile_time_default(int i)
{
    return s<T>::settings[i].default_value;
}

// String specialization: if the default starts with ".", assume it's a
// path and prepend our architecture directory.
template <>
std::string compile_time_default(int i)
{
    typedef std::string T;
    T retval;
    // If the setting starts with ".", assume it's a relative path and
    // prepend our arch dir.
    if(s<T>::settings[i].default_value[0] == '.')
    {
        retval = GetVisItInstallationDirectory() + "/";
    }
    retval += s<T>::settings[i].default_value;
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
// ****************************************************************************
template<typename T>
T find_setting(const char *key)
{
    int i;
    size_t len = strlen(key);
    const size_t n = sizeof(s<T>::settings) / sizeof(struct Setting<const T>);

    // lookup the default from our settings array.
    T retval;
    for(i=0; i < n; ++i)
    {
        if(strncmp(s<T>::settings[i].key, key, len) == 0)
        {
            retval = compile_time_default<T>(i);
            break;
        }
    }
    if(i == n)
    {
        throw KeyNotFound();
    }

    /// @todo FIXME Here we *should* lookup the value in a configuration file.

    // Now lookup from the environment variable, if it exists.
    if(Environment::exists(s<T>::settings[i].envvar))
    {
        read_env(s<T>::settings[i].envvar, retval);
    }

    // Finally, check any overrides from cmd line options.
    typename s<T>::runtime_type::const_iterator iter =
      s<T>::runtime_settings.find(key);
    if(iter != s<T>::runtime_settings.end())
    {
        retval = iter->second;
    }

    return retval;
}

// Values to return when we encounter an error.  This is mostly to satisfy the
// compiler, since the errors we can detect always indicate programmer error,
// but we also strive for obviously-bad values, so that a developer printing
// values out in a debugger will get suspicious.
  template<typename T> struct bad {};
  // Integers are easy: we can use an enum.
  template<> struct bad<int> { enum { value = -1 }; };
  // We need the same trick we used with settings for other types though.
  template<> struct bad<const char*> { static const char *value; };
  template<> struct bad<std::string> { static const std::string value; };
  template<> struct bad<double>      { static const double value; };
  template<> struct bad<bool>        { static const bool value; };
  const char *      bad<const char*>::value = "badness";
  const std::string bad<std::string>::value = "badness";
  const double      bad<double>::value      = -42.42424242;
  const bool        bad<bool>::value        = false;

// ****************************************************************************
//  Function: RuntimeSetting::internal_lookup
//
//  Purpose:  Looks up a value while handling bad keys.
//
//  Programmer: Tom Fogal
//  Creation: June 27, 2009
//
// ****************************************************************************
template<typename T>
static T internal_lookup(const char *key)
{
    // The exception is internal only; it would be a bad idea to
    // let the exception propagate out of here.  The reasoning is
    // that specifying a bad key is *always* a coding error: what a
    // developer really wants at that point is an abort and a stack
    // trace.  If we put an exception specification in our interface
    // here, developers might try to catch it, which is a terrible
    // idea: it would mask a real error.
    // The VisIt development community has voted on banning assertions;
    // returning a bad value is the next best thing.
    TRY
    {
        return find_setting<T>(key);
    }
    CATCH(KeyNotFound)
    {
        debug1 << "Key '" << key << "' not found!" << std::endl;
        return bad<T>::value;
    }
    ENDTRY
}

// ****************************************************************************
// Function: RuntimeSetting::create_clo_overrides
//
// Purpose: Create overrides from a given set of command line options (clo's)
//
//  Programmer: Tom Fogal
//  Creation: June 27, 2009
//
// ****************************************************************************
template<typename T>
void create_clo_overrides(int argc, const char *argv[])
{
    const size_t n = sizeof(s<T>::settings) / sizeof(struct Setting<const T>);
    for(int i=0; i < argc; ++i)
    {
        for(int j=0; j < n; ++j)
        {
            if(strcmp(s<T>::settings[j].cmdline, argv[i]) == 0)
            {
                if(i+1 >= argc)
                {
                    debug1 << argv[i] << " option given with no parameter!"
                           << "  Ignoring option." << std::endl;
                    return;
                }
                T override;
                read_val(argv[i], override);
                s<T>::runtime_settings[s<T>::settings[j].key] = override;
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
int lookupi(const char *key)         { return internal_lookup<int>(key); }
double lookupf(const char *key)      { return internal_lookup<double>(key); }
std::string lookups(const char *key) {
    return internal_lookup<std::string>(key);
}
bool lookupb(const char *key)        { return internal_lookup<bool>(key); }

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
    create_clo_overrides<int>(argc, argv);
    create_clo_overrides<std::string>(argc, argv);
    create_clo_overrides<double>(argc, argv);
    create_clo_overrides<bool>(argc, argv);
}

};  /* namespace RuntimeSetting */
