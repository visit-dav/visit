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
dnl Any macros related to the IceT image compositor.
dnl
dnl Modifications:
dnl    Tom Fogal, Sun Jun  8 15:14:21 EDT 2008
dnl    Split up into AX_ICET_OPTIONS and AX_CHECK_ICET.  This allows a user to
dnl    check whether or not the enable option was specified without doing the
dnl    full check for IceT.
dnl

dnl provide --enable-icet and --with-icet-(include|lib)dir=... options.  These
dnl values will be picked up later by the AX_CHECK_ICET macro.
AC_DEFUN([AX_ICET_OPTIONS], [
AC_ARG_ENABLE([icet],
    [AS_HELP_STRING([--enable-icet],
        [Use the ICE-T parallel image compositor])],
    [enable_icet=yes],
    [enable_icet=no]
)

dnl `with' options to specify header/library locations.
AC_ARG_WITH([icet-includedir],
    [AS_HELP_STRING([--with-icet-includedir=/path],
        [Directory where ICE-T include files can be found.])],
    [with_icet_includedir=$withval],
    [with_icet_includedir=no]
)
AC_ARG_WITH([icet-libdir],
    [AS_HELP_STRING([--with-icet-libdir=/path],
        [Directory where ICE-T libraries can be found.])],
    [with_icet_libdir=$withval],
    [with_icet_libdir=no]
)

ICET_LIBS=
ICET_ENGINE_MAIN_OBJ=
ax_ICET_LIB=
ax_ICET_LIB_MPI=
ax_ICET_LIB_STRATEGIES=

])

dnl Make sure we can find/use the IceT libary specified.
dnl You can set ICET_INCLUDEDIR and ICET_LIBDIR to setup paths sans
dnl ./configure's --with-icet options, but (if specified) --with-icet options
dnl will override those settings.
dnl If we can/should use IceT:
dnl    define HAVE_ICET in the preprocessor.
dnl    substitute ICET_LIBS to be available to makefiles as the `-l' options
dnl       needed to link in IceT.
dnl    substitute ICET_CXXFLAGS to be available to makefiles
dnl    define ICET_ENGINE_MAIN_OBJ to be `$(ICET_PAROBJ)'
dnl Otherwise:
dnl    define ICET_ENGINE_MAIN_OBJ to be `'
AC_DEFUN([AX_CHECK_ICET], [

AS_IF([test "x$enable_icet" != xno],
    [
        dnl IceT is really a C library, but we do our checks with C++ because
        dnl that's how we'll use it in VisIt
        AC_LANG_PUSH([C++])
        ax_save_LDFLAGS="${LDFLAGS}"
        ax_save_CXXFLAGS="${CXXFLAGS}"

        dnl Did they specify --with-icet options?  If they didn't, default to
        dnl use anything they might have had in the environment.
        AS_IF([test "x$with_icet_includedir" != xno],
            [ICET_INCLUDEDIR="${with_icet_includedir}"]
        )
        AS_IF([test "x$with_icet_libdir" != xno],
            [ICET_LIBDIR="${with_icet_libdir}"]
        )
        dnl Those are just directories, not compiler flags.

        dnl Now that we've got include/library directories (somehow, who cares
        dnl how at this point) we can set up appopriate compiler/linker flags.
        dnl
        dnl Note we require IceT to use OS mesa, so we force USE_MGL_NAMESPACE!
        ICET_CXXFLAGS="-DUSE_MGL_NAMESPACE"
        ICET_LDFLAGS=
        AS_IF([test -n "${ICET_INCLUDEDIR}"],
            [
                ICET_CXXFLAGS="${ICET_CXXFLAGS} -I${ICET_INCLUDEDIR}"
            ]
        )
        CXXFLAGS="${CXXFLAGS} ${ICET_CXXFLAGS}"
        AS_IF([test -n "${ICET_LIBDIR}"], [
            ICET_LDFLAGS="-L${ICET_LIBDIR}"
            LDFLAGS="${LDFLAGS} ${ICET_LDFLAGS}"
        ])

        dnl IceT is really three libraries: `icet', where most everything is;
        dnl `icet_mpi' which has basic functions for wrapping the MPI context;
        dnl and `icet_strategies' which has backend functions for implementing
        dnl different methods for composition.
        AC_CHECK_LIB([icet], [icetGetColorBuffer],
            [ax_ICET_LIB="-licet"],
            [AC_MSG_FAILURE([
--enable-icet was given, but I could not use IceT.  Perhaps you need to set
some custom LDFLAGS?])],
            dnl This isn't the greatest setup.  We rely on a previous macro
            dnl setting up the MESA_DIR and MESA_LIBS variables here, but the
            dnl checks for those are simply done inline (not in an autoconf
            dnl macro).  Thus we cannot AC_REQUIRE it and ensure that Mesa has
            dnl been checked for before we run this macro.
            dnl
            dnl That's the long way of saying `don't use AX_CHECK_ICET until
            dnl you've checked for / setup Mesa'.
            [-L$MESA_DIR/lib $MESA_LIBS -lm]
        )
        AC_CHECK_LIB([icet_mpi], [icetCreateMPICommunicator],
            [ax_ICET_LIB_MPI="-licet_mpi"],
            [AC_MSG_FAILURE([
--enable-icet was given, but I could not use IceT's MPI wrappers.  Perhaps you
need to set some custom LDFLAGS?])],
            [-L$MESA_DIR/lib $MESA_LIBS -lm -licet -lmpi]
        )
        dnl This is tricky.  We test to make sure it works, but really I don't
        dnl think any of the functions in this library are designed to be
        dnl public.  They should really be rolled into `libicet', it seems, but
        dnl I'm not the IceT developers...
        AC_CHECK_LIB([icet_strategies], [icetTreeCompose],
            [ax_ICET_LIB_STRATEGIES="-licet_strategies"],
            [AC_MSG_FAILURE([
--enable-icet was given, but I could not make use of it's strategies!  This
might mean the library was updated and VisIt's IceT test simply needs to be
updated, or it might mean your IceT install is not quite right.  You may need
to set some custom LDFLAGS.])],
            [-L$MESA_DIR/lib $MESA_LIBS -lm -licet -lmpi]
        )
        ICET_LIBS="${ax_ICET_LIB} ${ax_ICET_LIB_MPI} ${ax_ICET_LIB_STRATEGIES}"
        ICET_ENGINE_MAIN_OBJ='$(ICET_PAROBJ)'
        AC_SUBST(ICET_CXXFLAGS)
        AC_SUBST(ICET_LDFLAGS)
        AC_SUBST(ICET_LIBS)
        AC_DEFINE([HAVE_ICET], [1], [Define if you have the IceT library])
        LDFLAGS=${ax_save_LDFLAGS}
        CXXFLAGS=${ax_save_CXXFLAGS}
        AC_LANG_POP([C++])
    ],
    [
        ICET_ENGINE_MAIN_OBJ='$(NO_ICET_PAROBJ)'
        AC_DEFINE([HAVE_ICET], [0], [Define if you have the IceT library])

        dnl If they gave us options to say where IceT is, but did not say they
        dnl wanted IceT... why did they bother?  Yell at them.
        AS_IF([test "x$with_icet_includedir" != xno -o \
                    "x$with_icet_libdir" != xno], [
            AC_MSG_WARN([
--with-icet-... option[s] specified without --enable-icet.  Did you mean to
enable IceT?])
        ])
    ]
)

AC_SUBST(ICET_ENGINE_MAIN_OBJ)

]) dnl end AC_DEFUN for AX_CHECK_ICET
