dnl                            Copyright 2000 - 2008
dnl                 The Regents of the University of California.
dnl                             All rights reserved.
dnl
dnl This work was produced at the University of California, Lawrence Livermore
dnl National Laboratory (UC LLNL)  under contract no.  W-7405-ENG-48 (Contract
dnl 48) between  the U.S. Department of Energy  (DOE)  and The Regents  of the
dnl University  of  California  (University)  for  the  operation of  UC LLNL.
dnl Copyright   is  reserved to  the   University for  purposes  of controlled
dnl dissemination,   commercialization  through  formal licensing,   or  other
dnl disposition under  terms of Contract   48; DOE policies,  regulations  and
dnl orders; and  U.S. statutes.  The    rights of the  Federal Government  are
dnl reserved under Contract 48 subject to  the restrictions agreed upon by DOE
dnl and University.
dnl
dnl                                 DISCLAIMER
dnl
dnl This software was prepared as an account of work sponsored by an agency of
dnl the United States Government. Neither the United States Government nor the
dnl University  of California nor any of  their employees, makes any warranty,
dnl express or implied,  or assumes  any liability  or responsiblity  for  the
dnl accuracy, completeness, or    usefullness of any  information,  apparatus,
dnl product, or  process   disclosed, or represents that  its   use would  not
dnl infringe privately    owned  rights. Reference   herein  to  any  specific
dnl commercial  products, process,  or    service by trade   name,  trademark,
dnl manufacturer, or otherwise, does  not necessarily constitute or imply  its
dnl endorsement, recommendation, or  favoring by the United  States Government
dnl or the   University  of  California.  The views   and  opinions of authors
dnl expressed herein do not  necessarily state or  reflect those of the United
dnl States Government  or the University of  California, and shall not be used
dnl for advertising or product endorsement purposes.
dnl
dnl Macro: VAC_ARG_WITH3RD
dnl
dnl Programmer: Mark C. Miller
dnl Created:    Tue Oct 21 09:18:59 PDT 2008
dnl
dnl A very helpful third party library macro. What does this macro do?
dnl
dnl A. Prints a nice help message without the caller having to worry about
dnl    formatting. The help message is 'integrated' with a header help
dnl    message that details how to format the arg to --with-xxx options.
dnl B. Unifies and sanitizes the naming of symbols for Make and
dnl    C Pre-Processor. For Makefiles, it defines...
dnl        XXX_INCLUDE (which should probably really be called XXX_CPPFLAGS)
dnl        XXX_LIB     (which should probably really be called XXX_LDFLAGS)
dnl        XXX_TARGET  (used to identify make targets that depend on lib)
dnl    For C Pre-Processor, it defines...
dnl        HAVE_LIBXXX
dnl    For internal configure logic, it defines...
dnl        XXX_LIBFILE (the full path to actual lib file it selected)
dnl C. Handles a variety of ways of formatting the argument to a
dnl    --with-xxx=<withval> command line option to configure where <withval>
dnl    can be any of the following...
dnl       no                      totally disable use of the specified lib
dnl       yes                     use the lib as provided by the platform
dnl       builtin                 use the lib in third party builtin 
dnl       <DIR>[:<DEPLIBS>]       single directory above inc[lude] and lib 
dnl       <IDIR>,<LDIR>[:<DEPLIBS>] separate paths to inc[lude] and lib dirs
dnl    where <DEPLIBS> is an optional set of dependent library paths and flags.
dnl D. Confirms existence and readability of key header file(s) for library.
dnl E. Confirms existence and readability of key library files for library.
dnl F. Handles variance of names of the lib (e.g. silo/siloh5)
dnl G. Handles variance in library extension (e.g. .a/.so/.dylib)
dnl H. Creates symbolic links in VisIt's lib dir for the lib and its
dnl    dependent libs, if any. We may want to use AC_CONFIG_LINKS for this
dnl    instead of how it is currently being handled.
dnl I. Handles defaults specified in config-site files using the symbols
dnl    DEFAULT_XXX_LIBLOC
dnl    DEFAULT_XXX_LIBDEP (optional)
dnl
dnl The arguments are...
dnl $1 = [REQ] library (short) name (e.g. 'hdf5' or 'zlib')
dnl $2 = [OPT] name of (key) header file including its extension (default is $1.h)
dnl $3 = [OPT] name(s) of (key) library file NOT including leading 'lib' or trailing
dnl      extension (default is lib$1.{dylib|so|a})
dnl $4 = [OPT] name of (key) function in library 
dnl $5 = [OPT] help string; the macro itself generates a default
dnl
dnl I would regard this as an m4-minimal/sh-maximal implementation of this
dnl macro. All string processing that can actually be done at configure time
dnl by sh is done so. Only the absolute minimum that has to be done by m4 at
dnl autoconf time is done using m4. In addition, it relies very little on
dnl other, pre-defined autoconf macros. This is primarily due to the
dnl implementor's lack of experience with m4 ;).
dnl
dnl Modififications:
dnl   Mark C. Miller, Thu Nov 20 09:29:37 PST 2008
dnl   Added '0-9' characters to m4_translit invokation. In theory, this should
dnl   NOT be necessary but m4_translit was NOT always getting the '5' in HDF5
dnl   and this change appears to have fixed that. Also, instead of actually
dnl   creating the links to libs here, we now stick the commands for doing that
dnl   into a configure variable that gets substituted into lib/Makefile.in.
dnl   Made it initialize withval to 'no' and initialize with_package. Added
dnl   logic to the action-if-not-given part of AC_ARG_WITH to change withval
dnl   only if DEFAULT specifications actually exist.
dnl
AC_DEFUN(VAC_ARG_WITH3RD,
[
    withval=no
    AC_ARG_WITH($1, AC_HELP_STRING([--with-$1],ifelse(,[$5],[use $1; build related plugin(s)/code],[$5])),,
        if test -n "$DEFAULT_[]m4_translit([$1],[a-z0-9-],[A-Z0-9_])_LIBLOC"; then
            withval=$DEFAULT_[]m4_translit([$1],[a-z0-9-],[A-Z0-9_])_LIBLOC:$DEFAULT_[]m4_translit([$1],[a-z0-9-],[A-Z0-9_])_LIBDEP
        fi)

    with_[]m4_translit([$1],[A-Z0-9_],[a-z0-9-])=$withval
    m4_translit([$1],[a-z0-9-],[A-Z0-9_])[]_INCLUDE=""
    m4_translit([$1],[a-z0-9-],[A-Z0-9_])[]_LIB=""
    m4_translit([$1],[a-z0-9-],[A-Z0-9_])[]_TARGET=""
    ifelse(,[$2],incfile=[$1].h,incfile=[$2])
    ifelse(,[$3],libtags="[$1]",libtags="[$3]")
    incdirs="include inc ."
    libdirs="libso lib ."
    libexts="dylib so a"
    lib_links=""
    deplib_links=""
    incdir=""
    libdir=""
    libtag=""
    libfile=""

    if test "$withval" = yes; then
        AC_CHECK_HEADERS($incfile)
        ifelse(,[$4],,
            [for t in $libtags; do
                 AC_CHECK_LIB($t,[$4])
            done])
    else
        AC_MSG_CHECKING(if using $1)
    fi
    if test "$withval" = builtin; then

        libs=""
        for t in $libtags; do
            libs="$libs -l${t}"
        done
        m4_translit([$1],[a-z0-9-],[A-Z0-9_])[]_LIB="$libs"
        m4_translit([$1],[a-z0-9-],[A-Z0-9_])[]_TARGET="$""(m4_translit([$1],[a-z0-9-],[A-Z0-9_])[]_TARGET)"
        AC_DEFINE(HAVE_LIB[]m4_translit([$1],[a-z0-9-],[A-Z0-9_]), [1], [Define if you have $1.])
        AC_MSG_RESULT(builtin)

    elif test -n "$withval" && test $withval != no && test $withval != yes; then

        #
        # Handle various forms of the with argument
        #     <DIR> single dir below which inc[lude]/lib subdirs can be found 
        #     <I-DIR>,<L-DIR> separate include/lib dirs
        #     <DIR>:<ADDL_LDFLAGS> same as <DIR> but with additional LDFLAGS needed to link with this lib
        #     <I-DIR>,<L-DIR>:<ADDL_LDFLAGS> same as <I-DIR>,<L-DIR> but with additional LDFLAGS needed to link with the lib
        #
        if test -n "`echo $withval | cut -d':' -f1 | grep ','`"; then
            #
            # confirm location of $1 header file and its existance and readability
            #
            incdir="`echo $withval | cut -d':' -f1 | cut -d',' -f1`"
            if test ! -r $incdir/$incfile; then
                incdir=""
            fi

            #
            # confirm location of $1 lib file and its existance and readability
            #
            libdir="`echo $withval | cut -d':' -f1 | cut -d',' -f2`"
            for t in $libtags; do
                for e in $libexts; do
                    if test -r $libdir/lib$t.$e; then
                        libfile=lib$t.$e
                        libtag=$t
                        break 2
                    fi
                done
            done
            if test -z "$libfile"; then
                libdir=""
            fi 

        else
            locdir="`echo $withval | cut -d':' -f1`"

            #
            # confirm location of $1 header file and its existance and readability
            #
            for i in $incdirs; do
                if test -r $locdir/$i/$incfile; then
                    incdir=$locdir/$i
                    break
                fi
            done

            #
            # confirm location of $1 lib file and its existance and readability
            #
            for l in $libdirs; do
                for t in $libtags; do
                    for e in $libexts; do
                        if test -r $locdir/$l/lib$t.$e; then
                            libdir=$locdir/$l
                            libfile=lib$t.$e
                            libtag=$t
                            break 3
                        fi
                    done
                done
            done
        fi
        if test -z "$incdir"; then
            AC_MSG_ERROR(cannot find or read header $incfile from info in $withval)
        fi
        if test -z "$libdir"; then
            AC_MSG_ERROR(cannot find or read lib file(s) $libtags from info in $withval)
        fi

        m4_translit([$1],[a-z0-9-],[A-Z0-9_])[]_LIBFILE=$libdir/$libfile

        #
        # Make links for this lib in the lib dir
        #
        rm -rf lib/${libfile}*
        for f in $libdir/${libfile}*; do
            fb=`basename $f`
            if test -z "$VAC_CV_LIBDIR_CREATE_LINKS"; then
                VAC_CV_LIBDIR_CREATE_LINKS="ln -sf $f $fb"
                VAC_CV_LIBDIR_DESTROY_LINKS="rm -f $fb"
            else
                VAC_CV_LIBDIR_CREATE_LINKS="$VAC_CV_LIBDIR_CREATE_LINKS; ln -sf $f $fb"
                VAC_CV_LIBDIR_DESTROY_LINKS="$VAC_CV_LIBDIR_DESTROY_LINKS; rm -f $fb"
            fi
            lib_links="$lib_links $f"
        done

        #
        # Handle dependent libs for this library
        #
        deplib_ldflags="`echo $withval | cut -d':' -f2 | tr ',' ' '`"
        deplib_dirs=$libdir
        deplib_libs=""
        deplib_flags=""
        if test -n "`echo $withval | grep ':'`"; then

            #
            # Separate into -L terms and -l terms
            #
            for f in $deplib_ldflags; do
                if test "`echo $f | cut -d'L' -f1`" = "-"; then
                    deplib_dirs="$deplib_dirs `echo $f | cut -d'L' -f2-`"
                elif test "`echo $f | cut -d'l' -f1`" = "-"; then
                    deplib_libs="$deplib_libs `echo $f | cut -d'l' -f2-`"
                    deplib_flags="$deplib_flags $f"
                else
                    AC_MSG_ERROR(unrecognized dependent lib flag "$f" in "$deplib_ldflags")
                fi
            done

            #
            # Create links in lib dir
            #
            deplib_found="$deplib_libs"
            for l in $deplib_libs; do
                for d in $deplib_dirs; do
                    for e in $libexts; do
                        libfiles="`ls -1 $d/lib${l}.${e}* 2>/dev/null`"
                        if test -n "$libfiles"; then
                            rm -rf lib/lib${l}.${e}*
                            for lf in $libfiles; do
                                lfb=`basename $lf`
                                if test -z "$VAC_CV_LIBDIR_CREATE_LINKS"; then
                                    VAC_CV_LIBDIR_CREATE_LINKS="ln -sf $lf $lfb"
                                    VAC_CV_LIBDIR_DESTROY_LINKS="rm -f $lfb"
                                else
                                    VAC_CV_LIBDIR_CREATE_LINKS="$VAC_CV_LIBDIR_CREATE_LINKS; ln -sf $lf $lfb"
                                    VAC_CV_LIBDIR_DESTROY_LINKS="$VAC_CV_LIBDIR_DESTROY_LINKS; rm -f $lfb"
                                fi
                                deplib_links="$deplib_links $lf"
                                deplib_found="`echo $deplib_found | tr ' ' '\n' | grep -xve ${l}`"
                            done
                            continue 3
                        fi
                    done # loop over e (shared lib extensions)
                done # loop over d (lib directories)
            done # loop over l (lib names)

            if test -n "`echo $deplib_found | tr -d '[:space:]'`"; then
                AC_MSG_ERROR(unable to resolve dependent liraries "$deplib_found" for $1)
            fi

        fi

        deplib_flags="`echo $deplib_flags | tr -d '\n'`"
        m4_translit([$1],[a-z0-9-],[A-Z0-9_])[]_INCLUDE="-I$incdir"
        m4_translit([$1],[a-z0-9-],[A-Z0-9_])[]_LIB="-l${libtag} $deplib_flags"
        m4_translit([$1],[a-z0-9-],[A-Z0-9_])[]_TARGET="$""(m4_translit([$1],[a-z0-9-],[A-Z0-9_])[]_TARGET)"
        AC_DEFINE(HAVE_LIB[]m4_translit([$1],[a-z0-9-],[A-Z0-9_]), [1], [Define if you have $1.])

        # if these strings are all blank, then make them really so
        if test -z "`echo $lib_links | tr -d '[:space:]'`"; then
            lib_links=
        fi
        if test -z "`echo $deplib_links | tr -d '[:space:]'`"; then
            deplib_links=
        fi

        if test -z "$deplib_links"; then
            AC_MSG_RESULT([
    m4_translit([$1],[a-z0-9-],[A-Z0-9_])[]_INCLUDE=$m4_translit([$1],[a-z0-9-],[A-Z0-9_])[]_INCLUDE
    m4_translit([$1],[a-z0-9-],[A-Z0-9_])[]_LIB=$m4_translit([$1],[a-z0-9-],[A-Z0-9_])[]_LIB
    $lib_links])
        else
            AC_MSG_RESULT([
    m4_translit([$1],[a-z0-9-],[A-Z0-9_])[]_INCLUDE=$m4_translit([$1],[a-z0-9-],[A-Z0-9_])[]_INCLUDE
    m4_translit([$1],[a-z0-9-],[A-Z0-9_])[]_LIB=$m4_translit([$1],[a-z0-9-],[A-Z0-9_])[]_LIB
    $lib_links
    $deplib_links])
        fi

    else

        AC_MSG_RESULT(no)

    fi

    AC_SUBST(m4_translit([$1],[a-z0-9-],[A-Z0-9_])[]_INCLUDE)
    AC_SUBST(m4_translit([$1],[a-z0-9-],[A-Z0-9_])[]_LIB)
    AC_SUBST(m4_translit([$1],[a-z0-9-],[A-Z0-9_])[]_TARGET)

]
)
