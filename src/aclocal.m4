# generated automatically by aclocal 1.10 -*- Autoconf -*-

# Copyright (C) 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
# 2005, 2006  Free Software Foundation, Inc.
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

m4_include([acinclude.m4])

# Macro: VAC_ARG_WITH3RD
#
# Programmer: Mark C. Miller
# Created:    Tue Oct 21 09:18:59 PDT 2008
#
# A very helpful third party library macro. What does this macro do?
#
# A. Prints a nice help message without the caller having to worry about
#    formatting. The help message is 'integrated' with a header help
#    message that details how to format the arg to --with-xxx options.
# B. Unifies and sanitizes the naming of symbols for Make and
#    C Pre-Processor. For Makefiles, it defines...
#        XXX_INCLUDE (which should probably really be called XXX_CPPFLAGS)
#        XXX_LIB     (which should probably really be called XXX_LDFLAGS)
#        XXX_TARGET  (used to identify make targets that depend on lib)
#    For C Pre-Processor, it defines...
#        HAVE_LIBXXX
# C. Handles a variety of ways of formatting the argument to a
#    --with-xxx=<withval> command line option to configure where <withval>
#    can be any of the following...
#       no                      totally disable use of the specified lib
#       yes                     use the lib as provided by the platform
#       builtin                 use the lib in third party builtin 
#       <DIR>[:<DEPLIBS>]       single directory above inc[lude] and lib 
#       <IDIR>,<LDIR>[:<DEPLIBS>] separate paths to inc[lude] and lib dirs
#    where <DEPLIBS> is an optional set of dependent library paths and flags.
# D. Confirms existence and readability of key header file(s) for library.
# E. Confirms existence and readability of key library files for library.
# F. Handles variance of names of the lib (e.g. silo/siloh5)
# G. Handles variance in library extension (e.g. .a/.so/.dylib)
# H. Creates symbolic links in VisIt's lib dir for the lib and its
#    dependent libs, if any. We may want to use AC_CONFIG_LINKS for this
#    instead of how it is currently being handled.
# I. Handles defaults specified in config-site files using the symbols
#    DEFAULT_XXX_LIBLOC
#    DEFAULT_XXX_LIBDEP (optional)
#
# The arguments are...
# $1 = [REQ] library (short) name (e.g. 'hdf5' or 'zlib')
# $2 = [OPT] name of (key) header file including its extension (default is $1.h)
# $3 = [OPT] name of (key) library file NOT including leading 'lib' or trailing
#      extension (default is lib$1.{dylib|so|a})
# $4 = [OPT] name of (key) function in library 
# $5 = [OPT] help sub-string; the macro itself generates a pretty good default
#      help string
#
# I would regard this as an m4-minimal/sh-maximal implementation of this
# macro. All string processing that can actually be done at configure time
# by sh is done so. Only the absolute minimum that has to be done by m4 at
# autoconf time is done using m4. In addition, it relies very little on
# other, pre-defined autoconf macros.
#
AC_DEFUN(VAC_ARG_WITH3RD,
[
    AC_ARG_WITH($1, AC_HELP_STRING([--with-$1],ifelse(,[$5],[use $1; build related plugin(s)/code],[$5])),,
        withval=$DEFAULT_[]m4_translit([$1],[a-z-],[A-Z_])_LIBLOC:$DEFAULT_[]m4_translit([$1],[a-z-],[A-Z_])_LIBDEP)

    m4_translit([$1],[a-z-],[A-Z_])[]_INCLUDE=""
    m4_translit([$1],[a-z-],[A-Z_])[]_LIB=""
    m4_translit([$1],[a-z-],[A-Z_])[]_TARGET=""
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
        m4_translit([$1],[a-z-],[A-Z_])[]_LIB="$libs"
        m4_translit([$1],[a-z-],[A-Z_])[]_TARGET="$""(m4_translit([$1],[a-z-],[A-Z_])[]_TARGET)"
        AC_DEFINE(HAVE_LIB[]m4_translit([$1],[a-z-],[A-Z_]), [1], [Define if you have $1.])
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

        m4_translit([$1],[a-z-],[A-Z_])[]_LIBFILE=$libdir/$libfile

        #
        # Make links for this lib in the lib dir
        #
        rm -rf lib/${libfile}*
        for f in $libdir/${libfile}*; do
            fb=`basename $f`
            ln -s $f lib/$fb
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
                                ln -s $lf lib/$lfb
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
        m4_translit([$1],[a-z-],[A-Z_])[]_INCLUDE="-I$incdir"
        m4_translit([$1],[a-z-],[A-Z_])[]_LIB="-l${libtag} $deplib_flags"
        m4_translit([$1],[a-z-],[A-Z_])[]_TARGET="$""(m4_translit([$1],[a-z-],[A-Z_])[]_TARGET)"
        AC_DEFINE(HAVE_LIB[]m4_translit([$1],[a-z-],[A-Z_]), [1], [Define if you have $1.])

        # if these strings are all blank, then make them really so
        if test -z "`echo $lib_links | tr -d '[:space:]'`"; then
            lib_links=
        fi
        if test -z "`echo $deplib_links | tr -d '[:space:]'`"; then
            deplib_links=
        fi

        if test -z "$deplib_links"; then
            AC_MSG_RESULT([
    m4_translit([$1],[a-z-],[A-Z_])[]_INCLUDE=$m4_translit([$1],[a-z-],[A-Z_])[]_INCLUDE
    m4_translit([$1],[a-z-],[A-Z_])[]_LIB=$m4_translit([$1],[a-z-],[A-Z_])[]_LIB
    $lib_links])
        else
            AC_MSG_RESULT([
    m4_translit([$1],[a-z-],[A-Z_])[]_INCLUDE=$m4_translit([$1],[a-z-],[A-Z_])[]_INCLUDE
    m4_translit([$1],[a-z-],[A-Z_])[]_LIB=$m4_translit([$1],[a-z-],[A-Z_])[]_LIB
    $lib_links
    $deplib_links])
        fi

    else

        AC_MSG_RESULT(no)

    fi

    AC_SUBST(m4_translit([$1],[a-z-],[A-Z_])[]_INCLUDE)
    AC_SUBST(m4_translit([$1],[a-z-],[A-Z_])[]_LIB)
    AC_SUBST(m4_translit([$1],[a-z-],[A-Z_])[]_TARGET)

]
)
