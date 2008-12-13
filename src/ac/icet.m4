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
dnl    Tom Fogal, Fri Jun 20 13:43:48 EDT 2008
dnl    Added an include path option for IceT to parallel c-pre-processor flags.
dnl    Was also using AC_ARG_ENABLE incorrectly.
dnl
dnl    Tom Fogal, Wed Jun 25 11:49:27 EDT 2008
dnl    AC_DEFINE should only be used once per variable, else it will just
dnl    use the last one!  Used a variable to define HAVE_ICET at the end of the
dnl    macro, instead of embedded in an AS_IF.
dnl
dnl    Tom Fogal, Mon Jun 30 16:39:14 EDT 2008
dnl    Set ICET_ENG_MAIN_OBJ to the list of object files in `engine/main/'
dnl    which should be built when IceT is enabled.  Did this in a new macro;
dnl    everything not in AX_VISIT_ICET should be non-VisIt-specific, I hope.
dnl
dnl    Tom Fogal, Tue Jul  1 13:33:34 EDT 2008
dnl    Fixed a `test' conditional which would produce a warning when not using
dnl    IceT.
dnl
dnl    Tom Fogal, Mon Aug  4 10:52:41 EDT 2008
dnl    Don't force USE_MGL_NAMESPACE; not required, and may do strange things
dnl    in the HW rendering case.
dnl
dnl    Tom Fogal, Fri Dec  5 09:14:33 MST 2008
dnl    Add the ICET_ENABLE variable as a synonym for --enable-icet.
dnl    Make sure there is always a message about IceT status.

dnl provide --enable-icet and --with-icet-(include|lib)dir=... options.  These
dnl values will be picked up later by the AX_CHECK_ICET macro.
AC_DEFUN([AX_ICET_OPTIONS], [
AC_ARG_ENABLE([icet],
    [AS_HELP_STRING([--enable-icet],
        [Use the ICE-T parallel image compositor])]
)
if [[ "$DEFAULT_ICET_INCLUDE" == "" ]] ; then
   DEFAULT_ICET_INCLUDE="no"
fi
if [[ "$DEFAULT_ICET_LIB" == "" ]] ; then
   DEFAULT_ICET_LIB="no"
fi
dnl `with' options to specify header/library locations.
AC_ARG_WITH([icet-includedir],
    [AS_HELP_STRING([--with-icet-includedir=/path],
        [Directory where ICE-T include files can be found.])],
    [with_icet_includedir=$withval],
    [with_icet_includedir=$DEFAULT_ICET_INCLUDE]
)
AC_ARG_WITH([icet-libdir],
    [AS_HELP_STRING([--with-icet-libdir=/path],
        [Directory where ICE-T libraries can be found.])],
    [with_icet_libdir=$withval],
    [with_icet_libdir=$DEFAULT_ICET_LIB]
)

ICET_LIBS=
ax_ICET_LIB=
ax_ICET_LIB_MPI=
ax_ICET_LIB_STRATEGIES=
AC_MSG_CHECKING([if IceT should be used])
if test -n "${ICET_ENABLE}" ; then
    enable_icet="yes"
fi
if test -z "${enable_icet}" ; then
   enable_icet="no"
   if test -n "${DEFAULT_ICET_INCLUDE}" ; then
      if [[ "$UseParallel" == "yes" ]] ; then
         enable_icet="yes"
      fi
   fi
fi
AC_MSG_RESULT([$enable_icet])

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
AC_DEFUN([AX_CHECK_ICET], [

ax_have_icet=0
AS_IF([test "x$enable_icet" != "xno"],
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
            [-L$MESA_DIR/lib $MESA_LIBS $X_LIBS -lXext -lm $PTHREAD_LIB]
        )
        AC_CHECK_LIB([icet_mpi], [icetCreateMPICommunicator],
            [ax_ICET_LIB_MPI="-licet_mpi"],
            [AC_MSG_FAILURE([
--enable-icet was given, but I could not use IceT's MPI wrappers.  Perhaps you
need to set some custom LDFLAGS?])],
            [-L$MESA_DIR/lib $MESA_LIBS $X_LIBS -lXext -lm $PTHREAD_LIB -licet $MPI_LIBS]
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
            [-L$MESA_DIR/lib $MESA_LIBS $X_LIBS -lXext -lm $PTHREAD_LIB -licet $MPI_LIBS]
        )
        ICET_LIBS="${ax_ICET_LIB} ${ax_ICET_LIB_MPI} ${ax_ICET_LIB_STRATEGIES}"
        PARALLEL_CPPFLAGS="${PARALLEL_CPPFLAGS} -I${ICET_INCLUDEDIR}"
        AC_SUBST(ICET_CXXFLAGS)
        AC_SUBST(ICET_LDFLAGS)
        AC_SUBST(ICET_LIBS)
        LDFLAGS=${ax_save_LDFLAGS}
        CXXFLAGS=${ax_save_CXXFLAGS}
        AC_LANG_POP([C++])
        ax_have_icet=1
    ],
    [
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
AC_DEFINE_UNQUOTED([HAVE_ICET], ["$ax_have_icet"],
                   [Define if you have the IceT library])

]) dnl end AC_DEFUN for AX_CHECK_ICET

dnl This doesn't really deserve its own macro, but it's VisIt-specific code.
dnl It substitutes the ICET_ENG_MAIN_OBJ variable, for use in Makefile.in's.
dnl This will expand to all of the IceT object files if IceT is enabled, or
dnl nothing if IceT is not enabled.
AC_DEFUN([AX_VISIT_ICET], [
    AC_REQUIRE([AX_CHECK_ICET])
    dnl The `0' looks strange there, but is important; if the variable is unset
    dnl then we would otherwise expand to an empty string, which the shell
    dnl would warn about.  Since `test' does not seem to use short-circuit
    dnl evaluation (or at least does semantic analysis before any evaluation),
    dnl we can't just throw in an earlier clause which requires ax_have_icet to
    dnl be nonempty.
    AS_IF([test "0$ax_have_icet" -eq "1"],
        [
            ax_it_obj="IceTNetworkManager_icet.o"
        ],
        [
            ax_it_obj=""
        ]
    )
    AC_SUBST(ICET_ENG_MAIN_OBJ, $ax_it_obj)
])
