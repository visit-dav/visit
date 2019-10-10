# --------------------------------------------------------------------------- #
#   checking compiler minimum version                                         #
# --------------------------------------------------------------------------- #
function vercomp ()
{
    if [[ $1 == $2 ]]
    then
        return 0
    fi
    local IFS=.
    local i ver1=($1) ver2=($2)
    # fill empty fields in ver1 with zeros
    for ((i=${#ver1[@]}; i<${#ver2[@]}; i++))
    do
        ver1[i]=0
    done
    for ((i=0; i<${#ver1[@]}; i++))
    do
        if [[ -z ${ver2[i]} ]]
        then
            # fill empty fields in ver2 with zeros
            ver2[i]=0
        fi
        if [[ 10#${ver1[i]} > 10#${ver2[i]} ]]
        then
            return 1
        fi
        if [[ 10#${ver1[i]} < 10#${ver2[i]} ]]
        then
            return 2
        fi
    done
    return 0
}

function testvercomp ()
{
    vercomp $1 $2
    case $? in
        0) op='=';;
        1) op='>';;
        2) op='<';;
    esac
    if [[ $op != $3 ]]
    then
        return 1
    else
        return 0
    fi
}

function check_minimum_compiler_version()
{
   if [[ "$CXX_COMPILER" == "g++" ]] ; then
        VERSION=$(g++ -v 2>&1 | grep "gcc version" | cut -d' ' -f3 )
        echo "g++ version $VERSION"
        testvercomp $VERSION 4.8 '<'
        if [[ $? == 0 ]] ; then
            echo "Need g++ version >= 4.8"
            exit 1
        fi
    elif [[ "$OPSYS" == "Darwin"  &&  "$CXX_COMPILER" == "clang++" ]] ; then 
        VERSION=$(clang++ -v 2>&1 | grep "clang version" | cut -d' ' -f3 )
        echo "apple clang version $VERSION"
        testvercomp $VERSION 5.0 '<'
        if [[ $? == 0 ]] ; then
            echo "Need clang++ version >= 0.0"
            exit 1
        fi
    elif [[ "$CXX_COMPILER" == "clang++" ]] ; then 
        VERSION=$(clang++ -v 2>&1 | grep "clang version" | cut -d' ' -f3 )
        echo "clang version $VERSION"
        testvercomp $VERSION 3.3 '<'
        if [[ $? == 0 ]] ; then
            echo "Need clang++ version >= 3.3"
            exit 1
        fi
    elif [[ "$CXX_COMPILER" == "icpc" ]] ; then 
        VERSION=$(icpc -v 2>&1 | grep "icpc version" | cut -d' ' -f3 )
        if [[ $VERSION == "" ]] ; then
            VERSION=$(icpc -v 2>&1 | grep "icpc.orig version" | cut -d' ' -f3 )
        fi
        echo "icpc version $VERSION"
        testvercomp $VERSION 14.0 '<'
        if [[ $? == 0 ]] ; then
            echo "Need icpc version >= 14.0"
            exit 1
        fi
    fi
}

# --------------------------------------------------------------------------- #
#   checking opengl context creation (minimum required is 3.2)
# --------------------------------------------------------------------------- #

function check_opengl_context()
{
    # Check if we can create a 3.2 context with system gl
    echo "#include <GL/gl.h>" >> checkogl.cpp
    echo "#include <GL/glx.h>" >> checkogl.cpp
    echo "#include <cstring>" >> checkogl.cpp
    echo "#include <cstdlib>" >> checkogl.cpp
    echo "#define GLX_CONTEXT_MAJOR_VERSION_ARB       0x2091" >> checkogl.cpp
    echo "#define GLX_CONTEXT_MINOR_VERSION_ARB       0x2092" >> checkogl.cpp
    echo "typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);" >> checkogl.cpp
    echo "static bool isExtensionSupported(const char *extList, const char *extension) {" >> checkogl.cpp
    echo "  const char *start;" >> checkogl.cpp
    echo "  const char *where, *terminator;" >> checkogl.cpp
    echo "  where = strchr(extension, ' ');" >> checkogl.cpp
    echo "  if (where || *extension == '\0')" >> checkogl.cpp
    echo "    return false;" >> checkogl.cpp
    echo "  for (start=extList;;) {" >> checkogl.cpp
    echo "    where = strstr(start, extension);" >> checkogl.cpp
    echo "    if (!where)" >> checkogl.cpp
    echo "      break;" >> checkogl.cpp
    echo "    terminator = where + strlen(extension);" >> checkogl.cpp
    echo "    if ( where == start || *(where - 1) == ' ' )" >> checkogl.cpp
    echo "      if ( *terminator == ' ' || *terminator == '\0' )" >> checkogl.cpp
    echo "        return true;" >> checkogl.cpp
    echo "    start = terminator;" >> checkogl.cpp
    echo "  }" >> checkogl.cpp
    echo "  return false;" >> checkogl.cpp
    echo "}" >> checkogl.cpp
    echo "static bool ctxErrorOccurred = false;" >> checkogl.cpp
    echo "static int ctxErrorHandler( Display *dpy, XErrorEvent *ev )" >> checkogl.cpp
    echo "{" >> checkogl.cpp
    echo "    ctxErrorOccurred = true;" >> checkogl.cpp
    echo "    return 0;" >> checkogl.cpp
    echo "}" >> checkogl.cpp
    echo "int main(int argc, char* argv[])" >> checkogl.cpp
    echo "{" >> checkogl.cpp
    echo "  Display *display = XOpenDisplay(NULL);" >> checkogl.cpp
    echo "  if (!display)" >> checkogl.cpp
    echo " {" >> checkogl.cpp
    echo "    exit(1);" >> checkogl.cpp
    echo "  }" >> checkogl.cpp
    echo "  static int visual_attribs[] =" >> checkogl.cpp
    echo "    {" >> checkogl.cpp
    echo "      GLX_X_RENDERABLE    , True," >> checkogl.cpp
    echo "      GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT," >> checkogl.cpp
    echo "      GLX_RENDER_TYPE     , GLX_RGBA_BIT," >> checkogl.cpp
    echo "      GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR," >> checkogl.cpp
    echo "      GLX_RED_SIZE        , 8," >> checkogl.cpp
    echo "      GLX_GREEN_SIZE      , 8," >> checkogl.cpp
    echo "      GLX_BLUE_SIZE       , 8," >> checkogl.cpp
    echo "      GLX_ALPHA_SIZE      , 8," >> checkogl.cpp
    echo "      GLX_DEPTH_SIZE      , 24," >> checkogl.cpp
    echo "      GLX_STENCIL_SIZE    , 8," >> checkogl.cpp
    echo "      GLX_DOUBLEBUFFER    , True," >> checkogl.cpp
    echo "      //GLX_SAMPLE_BUFFERS  , 1," >> checkogl.cpp
    echo "      //GLX_SAMPLES         , 4," >> checkogl.cpp
    echo "      None" >> checkogl.cpp
    echo "    };" >> checkogl.cpp
    echo "  int glx_major, glx_minor;" >> checkogl.cpp
    echo "  if ( !glXQueryVersion( display, &glx_major, &glx_minor ) || ( ( glx_major == 1 ) && ( glx_minor < 3 ) ) || ( glx_major < 1 ) )" >> checkogl.cpp
    echo "  {" >> checkogl.cpp
    echo "    exit(1);" >> checkogl.cpp
    echo "  }" >> checkogl.cpp
    echo "  int fbcount;" >> checkogl.cpp
    echo "  GLXFBConfig* fbc = glXChooseFBConfig(display, DefaultScreen(display), visual_attribs, &fbcount);" >> checkogl.cpp
    echo "  if (!fbc)" >> checkogl.cpp
    echo "  {" >> checkogl.cpp
    echo "    exit(1);" >> checkogl.cpp
    echo "  }" >> checkogl.cpp
    echo "  int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;" >> checkogl.cpp
    echo "  int i;" >> checkogl.cpp
    echo "  for (i=0; i<fbcount; ++i)" >> checkogl.cpp
    echo "  {" >> checkogl.cpp
    echo "    XVisualInfo *vi = glXGetVisualFromFBConfig( display, fbc[i] );" >> checkogl.cpp
    echo "    if ( vi )" >> checkogl.cpp
    echo "    {" >> checkogl.cpp
    echo "      int samp_buf, samples;" >> checkogl.cpp
    echo "      glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf );" >> checkogl.cpp
    echo "      glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLES       , &samples  );" >> checkogl.cpp
    echo "      if ( best_fbc < 0 || samp_buf && samples > best_num_samp )" >> checkogl.cpp
    echo "        best_fbc = i, best_num_samp = samples;" >> checkogl.cpp
    echo "      if ( worst_fbc < 0 || !samp_buf || samples < worst_num_samp )" >> checkogl.cpp
    echo "        worst_fbc = i, worst_num_samp = samples;" >> checkogl.cpp
    echo "    }" >> checkogl.cpp
    echo "    XFree( vi );" >> checkogl.cpp
    echo "  }" >> checkogl.cpp
    echo "  GLXFBConfig bestFbc = fbc[ best_fbc ];" >> checkogl.cpp
    echo "  XFree( fbc );" >> checkogl.cpp
    echo "  XVisualInfo *vi = glXGetVisualFromFBConfig( display, bestFbc );" >> checkogl.cpp
    echo "  XSetWindowAttributes swa;" >> checkogl.cpp
    echo "  Colormap cmap;" >> checkogl.cpp
    echo "  swa.colormap = cmap = XCreateColormap( display, RootWindow( display, vi->screen ), vi->visual, AllocNone );" >> checkogl.cpp
    echo "  swa.background_pixmap = None ;" >> checkogl.cpp
    echo "  swa.border_pixel      = 0;" >> checkogl.cpp
    echo "  swa.event_mask        = StructureNotifyMask;" >> checkogl.cpp
    echo "  Window win = XCreateWindow( display, RootWindow( display, vi->screen ), 0, 0, 100, 100, 0, vi->depth, InputOutput, vi->visual, CWBorderPixel|CWColormap|CWEventMask, &swa );" >> checkogl.cpp
    echo "  if ( !win )" >> checkogl.cpp
    echo "  {" >> checkogl.cpp
    echo "    exit(1);" >> checkogl.cpp
    echo " }" >> checkogl.cpp
    echo "  XFree( vi );" >> checkogl.cpp
    echo "  XStoreName( display, win, \"GL 3.0 Window\" );" >> checkogl.cpp
    echo "  XMapWindow( display, win );" >> checkogl.cpp
    echo "  const char *glxExts = glXQueryExtensionsString( display, DefaultScreen( display ) );" >> checkogl.cpp
    echo "  glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;" >> checkogl.cpp
    echo "  glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc) glXGetProcAddressARB( (const GLubyte *) \"glXCreateContextAttribsARB\" );" >> checkogl.cpp
    echo "  GLXContext ctx = 0;" >> checkogl.cpp
    echo "  ctxErrorOccurred = false;" >> checkogl.cpp
    echo "  int (*oldHandler)(Display*, XErrorEvent*) = XSetErrorHandler(&ctxErrorHandler);" >> checkogl.cpp
    echo "  if ( !isExtensionSupported( glxExts, \"GLX_ARB_create_context\" ) || !glXCreateContextAttribsARB )" >> checkogl.cpp
    echo "  {" >> checkogl.cpp
    echo "      exit(1);" >> checkogl.cpp
    echo "  }" >> checkogl.cpp
    echo "  else" >> checkogl.cpp
    echo "  {" >> checkogl.cpp
    echo "    int context_attribs[] =" >> checkogl.cpp
    echo "      {" >> checkogl.cpp
    echo "        GLX_CONTEXT_MAJOR_VERSION_ARB, 3," >> checkogl.cpp
    echo "        GLX_CONTEXT_MINOR_VERSION_ARB, 2," >> checkogl.cpp
    echo "        //GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB," >> checkogl.cpp
    echo "        None};" >> checkogl.cpp
    echo "    ctx = glXCreateContextAttribsARB( display, bestFbc, 0, True, context_attribs );" >> checkogl.cpp
    echo "    XSync( display, False );" >> checkogl.cpp
    echo "    if ( ctxErrorOccurred || !ctx ){" >> checkogl.cpp
    echo "        exit(1);" >> checkogl.cpp
    echo "    }" >> checkogl.cpp
    echo "  }" >> checkogl.cpp
    echo "  XCloseDisplay( display );" >> checkogl.cpp
    echo "  return 0;" >> checkogl.cpp
    echo "}" >> checkogl.cpp

    $CXX_COMPILER checkogl.cpp -Wl,-lGL -lSM -lICE -lX11 -lXext
    if [[ $? != 0 ]]; then
        echo "failed to compile checkogl.cpp"
        rm -f checkogl.cpp
        rm -f a.out
        exit 1
    fi
    ./a.out 
    if [[ $? != 0 ]]; then
        echo "Could not obtain a 3.2 context with system GL."
        echo "You may want to add --mesagl to the command line."
        echo "To disable this check use --skip-opengl-context-check"
        rm -f checkogl.cpp
        rm -f a.out
        exit 1
    fi
    rm -f checkogl.cpp
    rm -f a.out
}


# ************************************************************************** #
#                       Section 1, setting up inputs                          #
# --------------------------------------------------------------------------- #
# This section sets up the inputs to the VisIt script.  This is where you can #
# specify which compiler to use, which versions of the third party libraries, #
# etc.  Note that this script is really only known to work with gcc.          #
# *************************************************************************** #


function initialize_build_visit()
{
    # This env. variable is NOT to be overriden by user. It is intended to
    # contain user's env. just prior to running build_visit.
    export BUILD_VISIT_ENV=$(env | cut -d'=' -f1 | sort | uniq)

    # allow users to set an external hostname for output filename
    export EXTERNAL_HOSTNAME=""

    # Can cause problems in some build systems.
    unset CDPATH

    # Some systems tar command does not support the deep directory hierarchies
    # used in Qt, such as AIX. Gnu tar is a good alternative.
    ### export TAR=/usr/local/bin/tar # Up and Purple
    export TAR=tar

    #
    # we have logic that assumes lib dirs are named "lib", not "lib64". On 
    # openSuSe some of the autoconf based 3rd party libs will install to "lib64"
    # unless the "CONFIG_SITE" env var is cleared  
    #
    export CONFIG_SITE=""


    # Determine if gfortran is present. This overly complex coding is to prevent
    # the "which" command from echoing failure to the user.
    which gfortran >& /dev/null
    if [[ $? == 0 ]]; then
        export GFORTRAN=`which gfortran | grep '^/'`
    else
        export GFORTRAN=""
    fi

    export OPSYS=${OPSYS:-$(uname -s)}
    export PROC=${PROC:-$(uname -p)}
    export REL=${REL:-$(uname -r)}
    # Determine architecture
    if [[ "$OPSYS" == "Darwin" ]]; then
        export ARCH=${ARCH:-"${PROC}-apple-darwin${REL%%.*}"}
        #  export VISITARCH=${VISITARCH-${ARCH}}
        export SO_EXT="dylib"
        VER=$(uname -r)
        # Check for Panther, because MACOSX_DEPLOYMENT_TARGET will
        # default to 10.1
	
        # Used http://en.wikipedia.org/wiki/Darwin_(operating_system)
        # to map Darwin Kernel versions to OSX version numbers.  Other
        # options for dealing with MACOSX_DEPLOYMENT_TARGET didn't
        # work See issue #1499 (https://visitbugs.ornl.gov/issues/1499)

        # use gcc for 10.9 & earlier

	VER_MAJOR=${VER%%.*}

	# bash script educational note:
	# The less than sign "<" is an arithmetic expression and
	# as such one must use parenthesis (( .. )) and not square brackets.
	# i.e. if (( ${VER_MAJOR} < 8 )) ; then

	# Square brackets are for contionals only. To make it a
	# conditional one must use "-lt"
        # i.e. if [[ ${VER_MAJOR} -lt 8 ]] ; then
	    
        if [[ ${VER_MAJOR} -lt 8 ]] ; then
            export MACOSX_DEPLOYMENT_TARGET=10.3
        elif [[ ${VER_MAJOR} == 8 ]] ; then
            export MACOSX_DEPLOYMENT_TARGET=10.4
        elif [[ ${VER_MAJOR} == 9 ]] ; then
            export MACOSX_DEPLOYMENT_TARGET=10.5
        elif [[ ${VER_MAJOR} == 10 ]] ; then
            export MACOSX_DEPLOYMENT_TARGET=10.6
        elif [[ ${VER_MAJOR} == 11 ]] ; then
            export MACOSX_DEPLOYMENT_TARGET=10.7
        elif [[ ${VER_MAJOR} == 12 ]] ; then
            export MACOSX_DEPLOYMENT_TARGET=10.8
        elif [[ ${VER_MAJOR} == 13 ]] ; then
            export MACOSX_DEPLOYMENT_TARGET=10.9
            export C_COMPILER=${C_COMPILER:-"clang"}
            export CXX_COMPILER=${CXX_COMPILER:-"clang++"}
        elif [[ ${VER_MAJOR} == 14 ]] ; then
            export MACOSX_DEPLOYMENT_TARGET=10.10
            export C_COMPILER=${C_COMPILER:-"clang"}
            export CXX_COMPILER=${CXX_COMPILER:-"clang++"}
        elif [[ ${VER_MAJOR} == 15 ]] ; then
            export MACOSX_DEPLOYMENT_TARGET=10.11
            export C_COMPILER=${C_COMPILER:-"clang"}
            export CXX_COMPILER=${CXX_COMPILER:-"clang++"}
        elif [[ ${VER_MAJOR} == 16 ]] ; then
            export MACOSX_DEPLOYMENT_TARGET=10.12
            export C_COMPILER=${C_COMPILER:-"clang"}
            export CXX_COMPILER=${CXX_COMPILER:-"clang++"}
        elif [[ ${VER_MAJOR} == 17 ]] ; then
            export MACOSX_DEPLOYMENT_TARGET=10.13
            export C_COMPILER=${C_COMPILER:-"clang"}
            export CXX_COMPILER=${CXX_COMPILER:-"clang++"}
        else
            export MACOSX_DEPLOYMENT_TARGET=10.13
            export C_COMPILER=${C_COMPILER:-"clang"}
            export CXX_COMPILER=${CXX_COMPILER:-"clang++"}
        fi

        export C_COMPILER=${C_COMPILER:-"gcc"}
        export CXX_COMPILER=${CXX_COMPILER:-"g++"}
        export FC_COMPILER=${FC_COMPILER:-$GFORTRAN}
        export C_OPT_FLAGS=${C_OPT_FLAGS:-"-O2"}
        export CFLAGS=${CFLAGS:-"-fno-common -fexceptions"}
        export CXX_OPT_FLAGS=${CXX_OPT_FLAGS:-"-O2"}
        export CXXFLAGS=${CXXFLAGS:-"-fno-common -fexceptions"}
        export FCFLAGS=${FCFLAGS:-$CFLAGS}
        export MESA_TARGET=${MESA_TARGET:-"darwin"}
	
    elif [[ "$OPSYS" == "Linux" ]]; then
        export ARCH=${ARCH:-"linux-$(uname -m)"} # You can change this to say RHEL, SuSE, Fedora.
        export SO_EXT="so"
        if [[ "$(uname -m)" == "i386" ]] ; then
            ###   export MESA_TARGET=${MESA_TARGET:-"linux-x86"} # Mesa-6.x
            export MESA_TARGET=${MESA_TARGET:-"linux"}
        elif [[ "$(uname -m)" == "i686" ]] ; then
            ###   export MESA_TARGET=${MESA_TARGET:-"linux-x86"} # Mesa-6.x
            export MESA_TARGET=${MESA_TARGET:-"linux"}
        elif [[ "$(uname -m)" == "x86_64" ]] ; then
            CFLAGS="$CFLAGS -m64 -fPIC"
            FCFLAGS="$FCFLAGS -m64 -fPIC"
            if [[ "$C_COMPILER" == "gcc" || "$C_COMPILER" == "" ]]; then
                C_OPT_FLAGS="$C_OPT_FLAGS -O2"
            fi
            CXXFLAGS="$CXXFLAGS -m64 -fPIC"
            if [[ "$CXX_COMPILER" == "g++" || "$CXX_COMPILER" == "" ]]; then
                CXX_OPT_FLAGS="$CXX_OPT_FLAGS -O2"
            fi
            ###   export MESA_TARGET=${MESA_TARGET:-"linux-x86-64"} # Mesa-6.x
            export MESA_TARGET=${MESA_TARGET:-"linux"}
        elif [[ "$(uname -m)" == "ppc64" ]] ; then
            if [[ "$C_COMPILER" == "xlc" ]] ; then
                CFLAGS="$CFLAGS -qpic"
                FCFLAGS="$FCFLAGS -qpic"
                CXXFLAGS="$CXXFLAGS -qpic"
                export CXX_COMPILER=${CXX_COMPILER-"xlC"}
                export MESA_TARGET=${MESA_TARGET-"linux"}
            elif [[ "$C_COMPILER" == "bgxlc" ]] ; then
                export CXX_COMPILER=${CXX_COMPILER-"bgxlC"}
            else
                CFLAGS="$CFLAGS -fPIC"
                FCFLAGS="$FCFLAGS -fPIC"
                if [[ "$C_COMPILER" == "gcc" || "$C_COMPILER" == "" ]]; then
                    C_OPT_FLAGS="$C_OPT_FLAGS -O2"
                fi
                CXXFLAGS="$CXXFLAGS -fPIC"
                if [[ "$CXX_COMPILER" == "g++" || "$CXX_COMPILER" == "" ]]; then
                    CXX_OPT_FLAGS="$CXX_OPT_FLAGS -O2"
                fi
                export MESA_TARGET=${MESA_TARGET-"linux"}
            fi
        elif [[ "$(uname -m)" == "ppc64le" ]] ; then
            if [[ "$C_COMPILER" == "xlc" ]] ; then
                CFLAGS="$CFLAGS -qpic"
                FCFLAGS="$FCFLAGS -qpic"
                CXXFLAGS="$CXXFLAGS -qpic"
                export CXX_COMPILER=${CXX_COMPILER-"xlC"}
                export MESA_TARGET=${MESA_TARGET-"linux"}
                QT_PLATFORM="linux-xlc" #aix-xlc"
            else
                CFLAGS="$CFLAGS -fPIC"
                FCFLAGS="$FCFLAGS -fPIC"
                if [[ "$C_COMPILER" == "gcc" || "$C_COMPILER" == "" ]]; then
                    C_OPT_FLAGS="$C_OPT_FLAGS -O2"
                fi
                CXXFLAGS="$CXXFLAGS -fPIC"
                if [[ "$CXX_COMPILER" == "g++" || "$CXX_COMPILER" == "" ]]; then
                    CXX_OPT_FLAGS="$CXX_OPT_FLAGS -O2"
                fi
                export MESA_TARGET=${MESA_TARGET-"linux"}
                QT_PLATFORM="linux-g++"
            fi
        elif [[ "$(uname -m)" == "ia64" ]] ; then
            CFLAGS="$CFLAGS -fPIC"
            FCFLAGS="$FCFLAGS -fPIC"
            if [[ "$C_COMPILER" == "gcc" || "$C_COMPILER" == "" ]]; then
                C_OPT_FLAGS="$C_OPT_FLAGS -O2"
            fi
            CXXFLAGS="$CXXFLAGS -fPIC"
            if [[ "$CXX_COMPILER" == "g++" || "$CXX_COMPILER" == "" ]]; then
                CXX_OPT_FLAGS="$CXX_OPT_FLAGS -O2"
            fi
        fi
        export C_COMPILER=${C_COMPILER:-"gcc"}
        export CXX_COMPILER=${CXX_COMPILER:-"g++"}
        export FC_COMPILER=${FC_COMPILER:-$GFORTRAN}
        export C_OPT_FLAGS=${C_OPT_FLAGS:-"-O2"}
        export CXX_OPT_FLAGS=${CXX_OPT_FLAGS:-"-O2"}
        export MESA_TARGET=${MESA_TARGET:-"linux"}
    elif [[ "$OPSYS" == "AIX" ]]; then
        export ARCH="aix" # You can change this to say RHEL, SuSE, Fedora, etc.
        export SO_EXT="a"
        export C_COMPILER=${C_COMPILER:-"xlc"}
        export FC_COMPILER=${FC_COMPILER:-$(which xlf | grep '^/')}
        export CXX_COMPILER=${CXX_COMPILER:-"xlC"}
        export C_OPT_FLAGS=${C_OPT_FLAGS:-"-O2"}
        export CXX_OPT_FLAGS=${CXX_OPT_FLAGS:-"-O2"}
        export MAKE=${MAKE:-"gmake"}
        export MESA_TARGET=${MESA_TARGET:-"aix"}
    elif [[ "$OPSYS" == "IRIX64" ]]; then
        export ARCH="irix64" # You can change this to say RHEL, SuSE, Fedora, etc.
        export SO_EXT="so"
        export C_COMPILER=${C_COMPILER:-"gcc"}
        export FC_COMPILER=${FC_COMPILER:-$GFORTRAN}
        export CXX_COMPILER=${CXX_COMPILER:-"g++"}
        export C_OPT_FLAGS=${C_OPT_FLAGS:-"-O2"}
        export CXX_OPT_FLAGS=${CXX_OPT_FLAGS:-"-O2"}
        export MAKE=${MAKE:-"gmake"}
        export MESA_TARGET=${MESA_TARGET:-"irix6-64-dso"}
    elif [[ "$OPSYS" == "SunOS" ]]; then
        export ARCH=${ARCH:-"sunos5"}
        export SO_EXT="so"
        export C_COMPILER=${C_COMPILER:-"gcc"}
        export FC_COMPILER=${FC_COMPILER:-$GFORTRAN}
        export CXX_COMPILER=${CXX_COMPILER:-"g++"}
        export C_OPT_FLAGS=${C_OPT_FLAGS:-"-O2"}
        export CXX_OPT_FLAGS=${CXX_OPT_FLAGS:-"-O2"}
        export MAKE=${MAKE:-"make"}
        export MESA_TARGET=${MESA_TARGET:-"sunos5-gcc"}
    else
        export ARCH=${ARCH:-"linux-$(uname -m)"} # You can change this to say RHEL, SuSE, Fedora.
        export SO_EXT="so"
        if [[ "$(uname -m)" == "x86_64" ]] ; then
            CFLAGS="$CFLAGS -m64 -fPIC"
            FCFLAGS="$FCFLAGS -m64 -fPIC"
            if [[ "$C_COMPILER" == "gcc" || "$C_COMPILER" == "" ]]; then
                C_OPT_FLAGS="$C_OPT_FLAGS -O2"
            fi
            CXXFLAGS="$CXXFLAGS -m64 -fPIC"
            if [[ "$CXX_COMPILER" == "g++" || "$CXX_COMPILER" == "" ]]; then
                CXX_OPT_FLAGS="$CXX_OPT_FLAGS -O2"
            fi
        fi
        if [[ "$(uname -m)" == "ia64" ]] ; then
            CFLAGS="$CFLAGS -fPIC"
            FCFLAGS="$FCFLAGS -fPIC"
            if [[ "$C_COMPILER" == "gcc" || "$C_COMPILER" == "" ]]; then
                C_OPT_FLAGS="$C_OPT_FLAGS -O2"
            fi
            CXXFLAGS="$CXXFLAGS -fPIC"
            if [[ "$CXX_COMPILER" == "g++" || "$CXX_COMPILER" == "" ]]; then
                CXX_OPT_FLAGS="$CXX_OPT_FLAGS -O2"
            fi
        fi
        export C_COMPILER=${C_COMPILER:-"gcc"}
        export FC_COMPILER=${FC_COMPILER:-$GFORTRAN}
        export CXX_COMPILER=${CXX_COMPILER:-"g++"}
        export C_OPT_FLAGS=${C_OPT_FLAGS:-"-O2"}
        export CXX_OPT_FLAGS=${CXX_OPT_FLAGS:-"-O2"}
    fi

    export MAKE=${MAKE:-"make"}
    export THIRD_PARTY_PATH=${THIRD_PARTY_PATH:-"./third_party"}
    export GROUP=${GROUP:-"visit"}
    #export LOG_FILE=${LOG_FILE:-"${0##*/}_log"}
    export GITREVISION=${GITREVISION:-"HEAD"}
    # Created a temporary value because the user can override most of
    # the components, which for the GUI happens at a later time.
    # the tmp value is useful for user feedback.
    if [[ $VISITARCH == "" ]] ; then
        export VISITARCHTMP=${ARCH}_${C_COMPILER}
        if [[ "$CXX_COMPILER" == "g++" ]] ; then
            VERSION=$(g++ -v 2>&1 | grep "gcc version" | cut -d' ' -f3 | cut -d'.' -f1-2)
            if [[ ${#VERSION} == 3 ]] ; then
                VISITARCHTMP=${VISITARCHTMP}-${VERSION}
            fi
        fi
    else
        # use environment variable value
        export VISITARCHTMP=$VISITARCH
    fi

    REDIRECT_ACTIVE="no"
    ANY_ERRORS="no"

    #initialize VisIt
    bv_visit_initialize

    #
    # OPTIONS
    #
    #initialize required libraries..

    for (( bv_i=0; bv_i<${#reqlibs[*]}; ++bv_i ))
    do
        initializeFunc="bv_${reqlibs[$bv_i]}_initialize"
        $initializeFunc
    done

    for (( bv_i=0; bv_i<${#optlibs[*]}; ++bv_i ))
    do
        initializeFunc="bv_${optlibs[$bv_i]}_initialize"
        $initializeFunc
    done

    export DO_HOSTCONF="yes"
    export DO_QT_SILENT="yes"

    export DO_DEBUG="no"
    export DO_GROUP="no"
    export DO_LOG="no"
    parallel="no"
    export DO_GIT="no"
    export DO_GIT_ANON="no"
    export DO_REVISION="no"
    USE_VISIT_FILE="no"
    export DO_PATH="no"
    export DO_VERSION="no"
    export DO_VERBOSE="no"
    export DO_JAVA="no"
    export DO_FORTRAN="no"
    export DO_PARADIS="no"
    export PREVENT_ICET="no"
    verify="no"
    export DO_OPTIONAL="yes"
    export DO_OPTIONAL2="no"
    export DO_MORE="no"
    export DO_DBIO_ONLY="no"
    export DO_ENGINE_ONLY="no"
    export DO_SERVER_COMPONENTS_ONLY="no"
    export DO_STATIC_BUILD="no"
    export DO_THREAD_BUILD="no"
    export USE_VISIBILITY_HIDDEN="no"
    export VISIT_INSTALL_PREFIX=""
    export VISIT_BUILD_MODE="Release"
    export VISIT_SELECTED_DATABASE_PLUGINS=""
    export DO_XDB="no"
    export CREATE_RPM="no"
    export DO_CONTEXT_CHECK="yes"
    export VISIT_INSTALL_NETWORK=""
    DOWNLOAD_ONLY="no"


    if [[ "$CXX_COMPILER" == "g++" ]] ; then
        VERSION=$(g++ -v 2>&1 | grep "gcc version" | cut -d' ' -f3 | cut -d'.' -f1-1)
        if [[ ${VERSION} -ge 4 ]] ; then
            export USE_VISIBILITY_HIDDEN="yes"
        fi
    fi


    # Setup git path
    export GIT_ANON_ROOT_PATH="http://github.com/visit-dav/visit.git"
    export GIT_REPO_ROOT_PATH="ssh://git@github.com/visit-dav/visit.git"


    if [[ "$OPSYS" != "Darwin" ]]; then
        WGET_MINOR_VERSION=$(wget --version| head -n 1|cut -d. -f 2)
        # version 1.7 pre-dates ssl integration
        if [[ "${WGET_MINOR_VERSION}" == "8" ]] ; then
            export WGET_OPTS=${WGET_OPTS=""}
        elif [[ "${WGET_MINOR_VERSION}" == "9" ]] ; then
            export WGET_OPTS=${WGET_OPTS:="--sslcheckcert=0"}
        else
            export WGET_OPTS=${WGET_OPTS:-"--no-check-certificate"}
        fi
    fi


    #get visit information..
    bv_visit_info

    #
    # TARBALL LOCATIONS AND VERSIONS
    #
    if [[ "$VISIT_FILE" != "" ]] ; then
        USE_VISIT_FILE="yes"
    fi
    export VISIT_FILE=${VISIT_FILE:-"visit${VISIT_VERSION}.tar.gz"}

    for (( bv_i=0; bv_i<${#reqlibs[*]}; ++bv_i ))
    do
        initializeFunc="bv_${reqlibs[$bv_i]}_info"
        $initializeFunc
    done

    for (( bv_i=0; bv_i<${#optlibs[*]}; ++bv_i ))
    do
        initializeFunc="bv_${optlibs[$bv_i]}_info"
        $initializeFunc
    done


    WRITE_UNIFIED_FILE=""
    VISIT_INSTALLATION_BUILD_DIR=""
    VISIT_DRY_RUN="no"
    DO_SUPER_BUILD="no"
    DO_MANGLED_LIBRARIES="no"
}




# *************************************************************************** #
# Function: starts_with_quote                                                 #
#                                                                             #
# Purpose: Meant to be used in `if $(starts_with_quote "$var") ; then`        #
#          conditionals.                                                      #
#                                                                             #
# Programmer: Tom Fogal                                                       #
# Date: Thu Oct  9 15:24:04 MDT 2008                                          #
#                                                                             #
# *************************************************************************** #

function starts_with_quote
{
    if test "${1:0:1}" = "\""; then #"
        return 0
    fi
    if test "${1:0:1}" = "'" ; then
        return 0
    fi
    return 1
}

# *************************************************************************** #
# Function: ends_with_quote                                                   #
#                                                                             #
# Purpose: Meant to be used `if $(ends_with_quote "$var") ; then`             #
#          conditionals.                                                      #
#                                                                             #
# Programmer: Tom Fogal                                                       #
# Date: Thu Oct  9 15:24:13 MDT 2008                                          #
#                                                                             #
# *************************************************************************** #

function ends_with_quote
{
    if test "${1: -1:1}" = "\""; then #"
        return 0
    fi
    if test "${1: -1:1}" = "'"; then
        return 0
    fi
    return 1
}

# *************************************************************************** #
# Function: strip_quotes                                                      #
#                                                                             #
# Purpose: Removes all quotes from the given argument.  Meant to be used in   #
#          $(strip_quotes "$some_string") expressions.                        #
#                                                                             #
# Programmer: Tom Fogal                                                       #
# Date: Thu Oct  9 16:04:25 MDT 2008                                          #
#                                                                             #
# *************************************************************************** #

function strip_quotes
{
    local arg="$@"
    str=""
    while test -n "$arg" ; do
        if test "${arg:0:1}" != "\"" ; then
            str="${str}${arg:0:1}"
        fi
        arg="${arg:1}"
    done
    echo "${str}"
}

function bv_enable_group
{
    local name=${1/--}
    local match=0

    for (( bv_i=0; bv_i < ${#grouplibs_name[*]}; ++bv_i ))
    do
        #replace | with space
        group_flag=${grouplibs_name[$bv_i]}
        group_flag=${group_flag//\|/ }
        for group in `echo $group_flag`;
        do
            if [[ "$group" == "$name" ]]; then
                echo "executing group $name"
                if [[ "$group" == "dbio-only" ]]; then
                    DO_DBIO_ONLY="yes"
                fi
                match=1
                for group_dep in `echo ${grouplibs_deps[$bv_i]}`;
                do
                    if [[ "$group_dep" == no-* ]]; then
                        group_dep=${group_dep/no-}
                        #info "disabling $group_dep"
                        initializeFunc="bv_${group_dep}_disable"
                        $initializeFunc
                    else
                        #info "enabling $group_dep"
                        initializeFunc="bv_${group_dep}_enable"
                        $initializeFunc
                    fi
                done
            fi
        done
    done

    return $match
}

function enable_dependent_libraries
{
    local depends_on=""

    for (( bv_i=0; bv_i<${#reqlibs[*]}; ++bv_i ))
    do
        $"bv_${reqlibs[$bv_i]}_is_enabled"

        #if not enabled then skip
        if [[ $? == 0 ]]; then
            continue
        fi

        #enabled library, check dependencies..
        depends_on=$("bv_${reqlibs[$bv_i]}_depends_on")

        #replace commas with spaces if there are any..
        depends_on=${depends_on//,/ }

        for depend_lib in `echo $depends_on`;
        do
            $"bv_${depend_lib}_is_enabled"
            if [[ $? == 0 ]]; then
                error "ERROR: library ${depend_lib} was not set ${reqlibs[$bv_i]} depends on it, please enable"
                #echo "library ${depend_lib} was not set but another library depends on it, enabling it"
                #$"bv_${depend_lib}_enable"
            fi
        done
    done

    for (( bv_i=0; bv_i<${#optlibs[*]}; ++bv_i ))
    do
        $"bv_${optlibs[$bv_i]}_is_enabled"

        #if not enabled then skip
        if [[ $? == 0 ]]; then
            continue
        fi

        #enabled library, check dependencies..
        depends_on=$("bv_${optlibs[$bv_i]}_depends_on")

        #replace commas with spaces if there are any..
        depends_on=${depends_on//,/ }

        for depend_lib in `echo $depends_on`;
        do
            $"bv_${depend_lib}_is_enabled"
            if [[ $? == 0 ]]; then
                error "ERROR: library ${depend_lib} was not set ${optlibs[$bv_i]} depends on it, please enable"
                echo "library ${depend_lib} was not set but another library depends on it, enabling it"
                $"bv_${depend_lib}_enable"
            fi
        done
    done
}

#TODO: enable this feature and remove this from ensure..
function initialize_module_variables
{
    info "initializing module variables"
    for (( bv_i=0; bv_i<${#reqlibs[*]}; ++bv_i ))
    do
        $"bv_${reqlibs[$bv_i]}_is_enabled"

        #if not enabled then skip
        if [[ $? == 0 ]]; then
            continue
        fi

        declare -F "bv_${reqlibs[$bv_i]}_initialize_vars" &>/dev/null

        if [[ $? == 0 ]]; then
            info "initialize module variables for ${reqlibs[$bv_i]}"
            $"bv_${reqlibs[$bv_i]}_initialize_vars"
        fi
    done

    for (( bv_i=0; bv_i<${#optlibs[*]}; ++bv_i ))
    do
        $"bv_${optlibs[$bv_i]}_is_enabled"

        #if not enabled then skip
        if [[ $? == 0 ]]; then
            continue
        fi

        declare -F "bv_${optlibs[$bv_i]}_initialize_vars" &>/dev/null

        if [[ $? == 0 ]]; then
            info "initialize module variables for ${optlibs[$bv_i]}"
            $"bv_${optlibs[$bv_i]}_initialize_vars"
        fi
    done
}


function build_library
{
    local build_lib=$1
    local depends_on=""

    #check if library is already installed..
    $"bv_${build_lib}_is_installed"

    if [[ $? == 1 ]]; then
        info "$build_lib is already installed, skipping"
        return
    fi

    #Make sure that the recursive enable feature is working properly
    $"bv_${build_lib}_is_enabled"

    if [[ $? == 0 ]]; then
        error "$build_lib was disabled, but seems that another library requires it "
    fi

    depends_on=$("bv_${build_lib}_depends_on")

    if [[ $depends_on != "" ]]; then
        info "library $build_lib depends on $depends_on"
    fi

    #replace commas with spaces if there are any..
    depends_on=${depends_on//,/ }

    for depend_lib in `echo $depends_on`;
    do
        build_library $depend_lib
    done

    #build ..
    $"bv_${build_lib}_build"
}

function build_libraries_serial
{
    info "building required libraries"
    for (( bv_i=0; bv_i<${#reqlibs[*]}; ++bv_i ))
    do
        $"bv_${reqlibs[$bv_i]}_is_enabled"

        if [[ $? == 0 ]]; then
            continue
        fi

        $"bv_${reqlibs[$bv_i]}_is_installed"

        if [[ $? == 0 ]]; then
            cd "$START_DIR"
            build_library ${reqlibs[$bv_i]}
        else
            info "${reqlibs[$bv_i]} already installed, skipping"
        fi
    done

    info "building optional libraries"
    for (( bv_i=0; bv_i<${#optlibs[*]}; ++bv_i ))
    do
        $"bv_${optlibs[$bv_i]}_is_enabled"

        if [[ $? == 0 ]]; then
            continue
        fi

        $"bv_${optlibs[$bv_i]}_is_installed"

        if [[ $? == 0 ]]; then
            cd "$START_DIR"
            build_library ${optlibs[$bv_i]}
        else
            info "${optlibs[$bv_i]} already installed, skipping"
        fi
    done
}

function build_libraries_parallel
{
    #launch all non dependent libraries in parallel..
    info "building parallel required libraries"
    for (( bv_i=0; bv_i<${#reqlibs[*]}; ++bv_i ))
    do
        $"bv_${reqlibs[$bv_i]}_is_enabled"

        if [[ $? == 0 ]]; then
            continue
        fi

        $"bv_${reqlibs[$bv_i]}_is_installed"
        if [[ $? == 0 ]]; then

            depends_on=$("bv_${reqlibs[$bv_i]}_depends_on")
            if [[ "$depends_on" == "" ]]; then
                (cd "$START_DIR" && build_library ${reqlibs[$bv_i]}) &
            fi
        else
            info "${reqlibs[$bv_i]} already installed, skipping"
        fi
    done

    wait

    #load the serial ones..
    for (( bv_i=0; bv_i<${#reqlibs[*]}; ++bv_i ))
    do
        $"bv_${reqlibs[$bv_i]}_is_enabled"

        if [[ $? == 0 ]]; then
            continue
        fi

        $"bv_${reqlibs[$bv_i]}_is_installed"

        if [[ $? == 0 ]]; then
            cd "$START_DIR"
            build_library ${reqlibs[$bv_i]}
        else
            info "${reqlibs[$bv_i]} already installed, skipping"
        fi
    done

    info "building parallel optional libraries"
    for (( bv_i=0; bv_i<${#optlibs[*]}; ++bv_i ))
    do
        $"bv_${optlibs[$bv_i]}_is_enabled"

        if [[ $? == 0 ]]; then
            continue
        fi
        $"bv_${optlibs[$bv_i]}_is_installed"
        if [[ $? == 0 ]]; then

            depends_on=$("bv_${optlibs[$bv_i]}_depends_on")
            if [[ "$depends_on" == "" ]]; then
                (cd "$START_DIR" && build_library ${optlibs[$bv_i]}) &
            fi
        else
            info "${optlibs[$bv_i]} already installed, skipping"
        fi
    done

    wait

    #load serial
    for (( bv_i=0; bv_i<${#optlibs[*]}; ++bv_i ))
    do
        $"bv_${optlibs[$bv_i]}_is_enabled"

        if [[ $? == 0 ]]; then
            continue
        fi

        $"bv_${optlibs[$bv_i]}_is_installed"

        if [[ $? == 0 ]]; then
            cd "$START_DIR"
            build_library ${optlibs[$bv_i]}
        else
            info "${optlibs[$bv_i]} already installed, skipping"
        fi
    done

}

# *************************************************************************** #
#                       Section 2, building VisIt                             #
# --------------------------------------------------------------------------- #
# This section does some set up for building VisIt, and then calls the        #
# functions to build the third party libraries and VisIt itself.              #
# *************************************************************************** #
function run_build_visit()
{
    declare -a arguments

    # Will be set if the next argument is an argument to an argument (I swear that
    # makes sense).  Make sure to unset it after pulling the argument!
    next_arg=""
    # If the user gives any deprecated options, we'll append command line options
    # we think they should use here.
    deprecated=""
    # A few options require us to perform some action before we start building
    # things, but we'd like to finish option parsing first.  We'll set this
    # variable in those cases, and test it when we finish parsing.
    next_action=""

    #handle groups first since they affect multiple libraries..
    for arg in "$@" ;
    do
        bv_enable_group "$arg"
        #not part of a group, add to argument list..
        if [[ $? == 0 ]]; then
            local match=0

            #suppress licenses from argument list
            for license in `echo $defaultLicenses`
            do
                if [[ "${arg/--}" == "$license" ]]; then
                    match=1
                    break
                fi
            done

            #suppress licenses as well..
            if [[ $match == 0 ]]; then
                arguments[${#arguments[*]}]="$arg"
            fi
        fi
    done

    for arg in "${arguments[@]}" ; do

        # Was the last option something that took an argument?
        if test -n "$next_arg" ; then
            # Yep.  Which option was it?
            case $next_arg in
                extra_commandline_arg) $EXTRA_COMMANDLINE_ARG_CALL "$arg";;
                visit-build-hostname) EXTERNAL_HOSTNAME="$arg";;
                installation-build-dir) VISIT_INSTALLATION_BUILD_DIR="$arg";;
                write-unified-file) WRITE_UNIFIED_FILE="$arg";;
                append-cflags) C_OPT_FLAGS="${C_OPT_FLAGS} ${arg}";;
                append-cxxflags) CXX_OPT_FLAGS="${CXX_OPT_FLAGS} ${arg}";;
                arch) VISITARCH="${arg}";;
                build-mode) VISIT_BUILD_MODE="${arg}";;
                cflags) C_OPT_FLAGS="${arg}";;
                cxxflags) CXX_OPT_FLAGS="${arg}";;
                cc) C_COMPILER="${arg}";;
                cxx) CXX_COMPILER="${arg}";;
                database-plugins) VISIT_SELECTED_DATABASE_PLUGINS="${arg}";;
                fc) FC_COMPILER="${arg}"; DO_FORTRAN="yes";;
                log-file) LOG_FILE="${arg}";;
                makeflags) MAKE_OPT_FLAGS="${arg}";;
                prefix) VISIT_INSTALL_PREFIX="${arg}";;
                install-network) VISIT_INSTALL_NETWORK="${arg}";;
                group) GROUP="${arg}";;
                git) GITREVISION="${arg}";;
                tarball) VISIT_FILE="${arg}";;
                thirdparty-path) THIRD_PARTY_PATH="${arg}";;
                version) VISIT_VERSION="${arg}"
                         VISIT_FILE="visit${VISIT_VERSION}.tar.gz";;
                *) error "Unknown next_arg value '$next_arg'!"
            esac
            # Make sure we process the next option as an option and not an
            # argument to an option.
            next_arg=""
            continue
        fi

        if [[ ${#arg} -gt 2 ]] ; then #has --

            #one module at a time
            resolve_arg=${arg:2} #remove --
            declare -F "bv_${resolve_arg}_enable" &>/dev/null

            if [[ $? == 0 ]] ; then
                #echo "enabling $resolve_arg"
                initializeFunc="bv_${resolve_arg}_enable"
                #argument is being explicitly set by the user so add a "force" flag
                $initializeFunc force
                continue
            elif [[ ${#resolve_arg} -gt 3 ]] ; then #in case it is --no-
                resolve_arg_no_opt=${resolve_arg:3}
                #disable library if it does not exist..
                declare -F "bv_${resolve_arg_no_opt}_disable" &>/dev/null
                if [[ $? == 0 ]] ; then
                    #echo "disabling ${resolve_arg_no_opt}"
                    initializeFunc="bv_${resolve_arg_no_opt}_disable"
                    $initializeFunc
                    #if disabling icet, prevent it as well
                    if [[ ${resolve_arg_no_opt} == "icet" ]]; then
                        echo "preventing icet from starting"
                        PREVENT_ICET="yes"
                    fi
                    continue
                fi
            fi

            #command line arguments created by modules
            #checking to see if additional command line arguments were requested
            resolve_arg=${arg:2} #remove --
            local match=0
            for (( bv_i=0; bv_i<${#extra_commandline_args[*]}; bv_i += 5 ))
            do
                local module_name=${extra_commandline_args[$bv_i]}
                local command=${extra_commandline_args[$bv_i+1]}
                local args=${extra_commandline_args[$bv_i+2]}
                local comment=${extra_commandline_args[$bv_i+3]}
                local fp=${extra_commandline_args[$bv_i+4]}
                if [[ "$command" == "$resolve_arg" ]]; then
                    if [ $args -eq 0 ] ; then
                        #call function immediately
                        $fp
                    else
                        #call function with next argument
                        next_arg="extra_commandline_arg"
                        EXTRA_COMMANDLINE_ARG_CALL="$fp"
                    fi
                    match=1
                    break;
                fi
            done

            #found a match in the modules..
            if [[ $match -eq 1 ]]; then
                continue
            fi
        fi


        case $arg in
            --visit-build-hostname) next_arg="visit-build-hostname";;
            --installation-build-dir) next_arg="installation-build-dir";;
            --write-unified-file) next_arg="write-unified-file";;
            --parallel-build) DO_SUPER_BUILD="yes";;
            --dry-run) VISIT_DRY_RUN="yes";;
            --arch) next_arg="arch";;
            --build-mode) next_arg="build-mode";;
            --cflag) next_arg="append-cflags";;
            --cflags) next_arg="cflags";;
            --cxxflag) next_arg="append-cxxflags";;
            --cxxflags) next_arg="cxxflags";;
            --cc) next_arg="cc";;
            --cxx) next_arg="cxx";;
            --create-rpm) CREATE_RPM="yes";;
            --log-file) next_arg="log-file";;
            --database-plugins) next_arg="database-plugins";;
            --debug) C_OPT_FLAGS="${C_OPT_FLAGS} -g"; CXX_OPT_FLAGS="${CXX_OPT_FLAGS} -g"; VISIT_BUILD_MODE="Debug";;
            --bv-debug) set -vx;;
            --download-only) DOWNLOAD_ONLY="yes";;
            --engine-only) DO_ENGINE_ONLY="yes";;
            --flags-debug) C_OPT_FLAGS="${C_OPT_FLAGS} -g"; CXX_OPT_FLAGS="${CXX_OPT_FLAGS} -g"; VISIT_BUILD_MODE="Debug";;
            --gdal) DO_GDAL="yes";;
            --fc) next_arg="fc";;
            --fortran) DO_FORTRAN="yes";;
            --group) next_arg="group"; DO_GROUP="yes";;
            -h|--help) next_action="help";;
            --install-network) next_arg="install-network";;
            --java) DO_JAVA="yes";;
            --makeflags) next_arg="makeflags";;
            --no-hostconf) DO_HOSTCONF="no";;
            --no-boost) DO_BOOST="no";;
            --no-qt-silent) DO_QT_SILENT="no";;
            --parallel) parallel="yes"; DO_ICET="yes";;
            --prefix) next_arg="prefix";;
            --print-vars) next_action="print-vars";;
            --server-components-only) DO_SERVER_COMPONENTS_ONLY="yes";;
            --paradis) DO_PARADIS="yes";;
            --static) DO_STATIC_BUILD="yes"
                      export USE_VISIBILITY_HIDDEN="no"
                      CXXFLAGS=$(echo $CXXFLAGS | sed "s/-fPIC//g")
                      CFLAGS=$(echo $CFLAGS | sed "s/-fPIC//g")
                      ;;
            --thread) DO_THREAD_BUILD="yes";;
            --stdout) LOG_FILE="/dev/tty";;
            --git) DO_GIT="yes"; export GIT_ROOT_PATH=$GIT_REPO_ROOT_PATH;;
            --git-anon) DO_GIT="yes"; DO_GIT_ANON="yes" ; export GIT_ROOT_PATH=$GIT_ANON_ROOT_PATH ;;
            --git-anonymous) DO_GIT="yes"; DO_GIT_ANON="yes" ; export GIT_ROOT_PATH=$GIT_ANON_ROOT_PATH ;;
            --git-revision) next_arg="git"; DO_GIT="yes"; DO_REVISION="yes"; DO_GIT_ANON="yes" ; export GIT_ROOT_PATH=$GIT_ANON_ROOT_PATH ;;
            --tarball) next_arg="tarball"
                       USE_VISIT_FILE="yes";;
            --thirdparty-path) next_arg="thirdparty-path";;
            --version) next_arg="version";;
            --xdb) DO_XDB="yes";;
            --console) ;;
            --skip-opengl-context-check) DO_CONTEXT_CHECK="no";;
            *)
                echo "Unrecognized option '${arg}'."
                ANY_ERRORS="yes";;
        esac
    done

    #error check to make sure that next arg is not left blank..
    if [[ $next_arg != "" ]] ; then
        echo "command line arguments are used incorrectly: argument $next_arg not fullfilled"
        exit 1
    fi

    if [[ "$ANY_ERRORS" == "yes" ]] ; then
        echo "command line arguments are used incorrectly. unrecognized options..."
        exit 1
    fi

    if test -n "${deprecated}" ; then
        summary="You are using some deprecated options to $0.  Please re-run"
        summary="${summary} $0 with a command line similar to:"
        echo "$summary"
        echo ""
        echo "$0 ${deprecated}"
        exit 1
    fi

    if test -n "${next_action}" ; then
        case ${next_action} in
            print-vars) printvariables; exit 2;;
            help) usage; exit 2;;
        esac
    fi

    #
    # Echo the current invocation command line to the log file
    #
    info "[build_visit invocation arguments] $@"


    #
    # Write a unified file
    #
    if [[ $WRITE_UNIFIED_FILE != "" ]] ; then
        bv_write_unified_file $WRITE_UNIFIED_FILE
        exit 0
    fi

    #
    # If we doing a trunk or RC build then make sure we are using GIT
    #
    if [[ "$TRUNK_BUILD" == "yes" || "$RC_BUILD" == "yes" ]]; then
        if [[ "$DO_GIT" == "no" ]]; then
            DO_GIT="yes"
            DO_GIT_ANON="yes"
            export GIT_ROOT_PATH=$GIT_ANON_ROOT_PATH
        fi
    fi

    check_minimum_compiler_version

    if [[ "$OPSYS" != "Darwin" && $DO_MESAGL == "no" && $DO_CONTEXT_CHECK != "no"  && $DO_DBIO_ONLY == "no" ]] ; then 
        if [[ $DO_VTK == "yes" || $DO_VISIT == "yes" ]] ; then
            check_opengl_context
        fi
    fi

    #
    # If we are AIX, make sure we are using GNU tar.
    #
    if [[ "$OPSYS" == "AIX" ]]; then
        TARVERSION=$($TAR --version >/dev/null 2>&1)
        if [[ $? != 0 ]] ; then
            echo "Error in build process. You are using the system tar on AIX."
            echo "Change the TAR variable in the script to the location of the"
            echo "GNU tar command."
            exit 1
        fi
    fi

    # Disable fortran support unless --fortran specified and a fortran compiler
    # was specified or found.
    if [[ $DO_FORTRAN == "no" || $FC_COMPILER == "" ]]; then
        export FC_COMPILER="no";
        warn "Fortran support for thirdparty libraries disabled."
    fi

    # make all VisIt related builds in its own directory..
    if [[ $VISIT_INSTALLATION_BUILD_DIR != "" ]] ; then
        if [[ -d $VISIT_INSTALLATION_BUILD_DIR ]]; then
            echo "Using already existing directory: $VISIT_INSTALLATION_BUILD_DIR"
        else
            mkdir -p $VISIT_INSTALLATION_BUILD_DIR
        fi

        if [[ ! -d $VISIT_INSTALLATION_BUILD_DIR ]]; then
            echo "Directory does not exist or I do not have permission to create it. Quitting"
            exit 0
        fi
        cd $VISIT_INSTALLATION_BUILD_DIR
    fi

    #
    # Log build_visit invocation w/ arguments & the start time.
    # Especially helpful if there are multiple starts dumped into the
    # same log.
    #
    LINES="------------------------------------------------------------"
    log $LINES
    log $0 $@
    log "Started:" $(date)
    log $LINES

    if [[ "$DO_GIT" == "yes" ]] ; then
        check_git_client
        if [[ $? != 0 ]]; then
            error "Fatal Error: GIT mode selected, but git client is not available."
        fi
    fi

    #enabling any dependent libraries, handles both dependers and dependees..
    #TODO: handle them seperately
    info "enabling any dependent libraries"
    enable_dependent_libraries

    # At this point we are after the command line and the visual selection
    # dry run, don't execute anything just run the enabled stuff..
    # happens before any downloads have taken place..
    if [[ $VISIT_DRY_RUN == "yes" ]]; then
        for (( bv_i=0; bv_i<${#reqlibs[*]}; ++bv_i ))
        do
            initializeFunc="bv_${reqlibs[$bv_i]}_dry_run"
            $initializeFunc
        done

        for (( bv_i=0; bv_i<${#optlibs[*]}; ++bv_i ))
        do
            initializeFunc="bv_${optlibs[$bv_i]}_dry_run"
            $initializeFunc
        done

        bv_visit_dry_run
        exit 0
    fi

    START_DIR="$PWD"

    if [[ "$DOWNLOAD_ONLY" == "no" ]] ; then
        if [[ ! -d "$THIRD_PARTY_PATH" ]] ; then
            if [[ "$THIRD_PARTY_PATH" == "./visit" ]] ; then
                mkdir "$THIRD_PARTY_PATH"
                if [[ $? != 0 ]] ; then
                    error "Unable to write files to the third party library location." \
                          "Bailing out."
                fi
            else
                info "The third party library location does not exist. Create it?"
                read RESPONSE
                if [[ "$RESPONSE" != "yes" ]] ; then
                    error "The third party library location does not exist." \
                          "Bailing out."
                else
                    mkdir "$THIRD_PARTY_PATH"
                    if [[ $? != 0 ]] ; then
                        error "Unable to write files to the third party library location." \
                              "Bailing out."
                    fi
                fi
            fi
        fi

        cd "$THIRD_PARTY_PATH"
        if [[ $? != 0 ]] ; then
            error "Unable to access the third party location. Bailing out."
        fi
    fi

    if [[ $VISITARCH == "" ]] ; then
        C_COMPILER_BASENAME=$(basename ${C_COMPILER})
        CXX_COMPILER_BASENAME=$(basename ${CXX_COMPILER})
        export VISITARCH=${ARCH}_${C_COMPILER_BASENAME}
        if [[ "$CXX_COMPILER_BASENAME" == "g++" ]] ; then
            VERSION=$(${CXX_COMPILER} -v 2>&1 | grep "gcc version" | cut -d' ' -f3 | cut -d'.' -f1-2)
            if [[ ${#VERSION} == 3 ]] ; then
                VISITARCH=${VISITARCH}-${VERSION}
            fi
        elif [[ "$CXX_COMPILER_BASENAME" == "icpc" ]] ; then
            VERSION=$(${CXX_COMPILER} --version | cut -d' ' -f3 | head -n1)
            VISITARCH=${VISITARCH}-${VERSION}
        fi
    fi

    export VISITDIR=${VISITDIR:-$(pwd)}
    cd "$START_DIR"

    #
    # See if the user wants to build a parallel version.
    #
    check_parallel
    if [[ $? != 0 ]] ; then
        error "Stopping build because necessary parallel options are not set."
    fi

    if [[ "$DO_ICET" == "yes" && "$PREVENT_ICET" != "yes" ]] ; then
        DO_CMAKE="yes"
    fi

    # initialize module variables, since all of VisIt's variables should
    # be set by now..
    initialize_module_variables

    #
    # Disable qt,qwt if it is not needed
    #
    if [[ "$DO_ENGINE_ONLY" == "yes" || "$DO_DBIO_ONLY" == "yes" || "$DO_SERVER_COMPONENTS_ONLY" == "yes" ]] ; then
        if [[ "$DO_ENGINE_ONLY" == "yes" ]] ; then
           info "disabling qt, qwt because --engine-only used"
        elif [[ "$DO_DBIO_ONLY" == "yes" ]] ; then
           info "disabling qt, qwt because --dbio-only used"
        elif [[ "$DO_SERVER_COMPONENTS_ONLY" == "yes" ]] ; then
           info "disabling qt, qwt because --server-components-only used"
        fi
        bv_qt_disable
        bv_qwt_disable
    fi

    #
    # Later we will build Qt.  We are going to bypass their licensing agreement,
    # so echo it here.
    #
    if [[ "$USE_SYSTEM_QT" != "yes" && "$DO_QT" == "yes" ]]; then
        BYPASS_QT_LICENSE="no"
        check_if_installed "qt" $QT_VERSION
        if [[ $? == 0 ]] ; then
            BYPASS_QT_LICENSE="yes"
        fi

        if [[ "$BYPASS_QT_LICENSE" == "no" && "$DOWNLOAD_ONLY" == "no" ]] ; then
            qt_license_prompt
            if [[ $? != 0 ]] ;then
                error "Qt4 Open Source Edition License Declined. Bailing out."
            fi
        fi
    fi

    #
    # Save stdout as stream 3, redirect stdout and stderr to the log file.
    # After this maks sure to use the info/warn/error functions to display
    # messages to the user
    #

    if [[ "${LOG_FILE}" != "/dev/tty" ]] ; then
        exec 3>&1 >> ${LOG_FILE} 2>&1
        REDIRECT_ACTIVE="yes"
    else
        exec 2>&1
    fi

    #
    #
    # Now make sure that we have everything we need to build VisIt, so we can bail
    # out early if we are headed for failure.
    #
    check_files
    if [[ $? != 0 ]] ; then
        error "Stopping build because necessary files aren't available."
    fi

    #
    # Exit if we were told to only download the files.
    #
    if [[ "$DOWNLOAD_ONLY" == "yes" ]] ; then
        info "Successfully downloaded the specified files."
        exit 0
    fi

    if [[ $DO_MANGLED_LIBRARIES == "yes" ]]; then
        info "Mangling libraries while building"
        info "Any libraries that support mangling will do so"
    fi

    if [[ "$DO_SUPER_BUILD" == "yes" ]]; then
        build_libraries_parallel
    else
        build_libraries_serial
    fi

    #
    # Create the host.conf file
    #

    if [[ "$DO_HOSTCONF" == "yes" ]] ; then
        info "Creating host.conf"
        build_hostconf
    fi

    #build visit itself..
    bv_visit_build
}
